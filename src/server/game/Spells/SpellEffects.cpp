
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
#include "SpellAuraEffects.h"
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
#include "Management/VMapFactory.h"
#include "Language.h"
#include "SocialMgr.h"
#include "Util.h"
#include "TemporarySummon.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ScriptMgr.h"
#include "GameObjectAI.h"
#include "InstanceScript.h"
#include "LogsDatabaseAccessor.h"
#include "SpellHistory.h"
#include "ReputationMgr.h"

SpellEffectHandlerFn SpellEffectHandlers[TOTAL_SPELL_EFFECTS]=
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
    &Spell::EffectLeap,                                     // 29 SPELL_EFFECT_LEAP
    &Spell::EffectEnergize,                                 // 30 SPELL_EFFECT_ENERGIZE
    &Spell::EffectWeaponDmg,                                // 31 SPELL_EFFECT_WEAPON_PERCENT_DAMAGE
    &Spell::EffectTriggerMissileSpell,                      // 32 SPELL_EFFECT_TRIGGER_MISSILE
    &Spell::EffectOpenLock,                                 // 33 SPELL_EFFECT_OPEN_LOCK
    &Spell::EffectSummonChangeItem,                         // 34 SPELL_EFFECT_SUMMON_CHANGE_ITEM
    &Spell::EffectUnused,                                   // 35 SPELL_EFFECT_APPLY_AREA_AURA_PARTY
    &Spell::EffectLearnSpell,                               // 36 SPELL_EFFECT_LEARN_SPELL
    &Spell::EffectUnused,                                   // 37 SPELL_EFFECT_SPELL_DEFENSE            one spell: SPELLDEFENSE (DND)
    &Spell::EffectDispel,                                   // 38 SPELL_EFFECT_DISPEL
    &Spell::EffectUnused,                                   // 39 SPELL_EFFECT_LANGUAGE
    &Spell::EffectDualWield,                                // 40 SPELL_EFFECT_DUAL_WIELD
    &Spell::EffectUnused,                                   // 41 SPELL_EFFECT_41 (old SPELL_EFFECT_SUMMON_WILD)
    &Spell::EffectUnused,                                   // 42 SPELL_EFFECT_42 (old SPELL_EFFECT_SUMMON_GUARDIAN)
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
    &Spell::EffectUnused,                                   // 73 SPELL_EFFECT_73 (old SPELL_EFFECT_SUMMON_POSSESSED
    &Spell::EffectUnused,                                   // 74 SPELL_EFFECT_74 (old SPELL_EFFECT_SUMMON_TOTEM)
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
    &Spell::EffectUnused,                                   // 87 SPELL_EFFECT_87 (old SPELL_EFFECT_SUMMON_TOTEM_SLOT1)
    &Spell::EffectUnused,                                   // 88 SPELL_EFFECT_88 (old SPELL_EFFECT_SUMMON_TOTEM_SLOT2)
    &Spell::EffectUnused,                                   // 89 SPELL_EFFECT_89 (old SPELL_EFFECT_SUMMON_TOTEM_SLOT2)
    &Spell::EffectUnused,                                   // 90 SPELL_EFFECT_90 (old SPELL_EFFECT_SUMMON_TOTEM_SLOT4)
    &Spell::EffectUnused,                                   // 91 SPELL_EFFECT_THREAT_ALL               one spell: zzOLDBrainwash
    &Spell::EffectEnchantHeldItem,                          // 92 SPELL_EFFECT_ENCHANT_HELD_ITEM
    &Spell::EffectUnused,                                   // 93 SPELL_EFFECT_93 (old SPELL_EFFECT_SUMMON_PHANTASM)
    &Spell::EffectSelfResurrect,                            // 94 SPELL_EFFECT_SELF_RESURRECT
    &Spell::EffectSkinning,                                 // 95 SPELL_EFFECT_SKINNING
    &Spell::EffectCharge,                                   // 96 SPELL_EFFECT_CHARGE
    &Spell::EffectUnused,                                   // 97 SPELL_EFFECT_97 (old SPELL_EFFECT_SUMMON_CRITTER)
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
    &Spell::EffectResurrectPet,                             //109 SPELL_EFFECT_RESURRECT_PET
    &Spell::EffectDestroyAllTotems,                         //110 SPELL_EFFECT_DESTROY_ALL_TOTEMS
    &Spell::EffectDurabilityDamage,                         //111 SPELL_EFFECT_DURABILITY_DAMAGE
    &Spell::EffectUnused,                                   //112 SPELL_EFFECT_112 (old SPELL_EFFECT_SUMMON_DEMON)
    &Spell::EffectResurrectNew,                             //113 SPELL_EFFECT_RESURRECT_NEW
    &Spell::EffectTaunt,                                    //114 SPELL_EFFECT_ATTACK_ME
    &Spell::EffectDurabilityDamagePCT,                      //115 SPELL_EFFECT_DURABILITY_DAMAGE_PCT
    &Spell::EffectSkinPlayerCorpse,                         //116 SPELL_EFFECT_SKIN_PLAYER_CORPSE       one spell: Remove Insignia, bg usage, required special corpse flags...
    &Spell::EffectSpiritHeal,                               //117 SPELL_EFFECT_SPIRIT_HEAL              one spell: Spirit Heal
    &Spell::EffectSkill,                                    //118 SPELL_EFFECT_SKILL                    professions and more
    &Spell::EffectUnused,                                   //119 SPELL_EFFECT_APPLY_AREA_AURA_PET
    &Spell::EffectUnused,                                   //120 SPELL_EFFECT_TELEPORT_GRAVEYARD       one spell: Graveyard Teleport Test
    &Spell::EffectWeaponDmg,                                //121 SPELL_EFFECT_NORMALIZED_WEAPON_DMG
    &Spell::EffectUnused,                                   //122 SPELL_EFFECT_122                      unused
    &Spell::EffectSendTaxi,                                 //123 SPELL_EFFECT_SEND_TAXI                taxi/flight related (misc value is taxi path id)
    &Spell::EffectPullTowards,                              //124 SPELL_EFFECT_PLAYER_PULL              opposite of knockback effect (pulls player twoard caster)
    &Spell::EffectModifyThreatPercent,                      //125 SPELL_EFFECT_MODIFY_THREAT_PERCENT
    &Spell::EffectStealBeneficialBuff,                      //126 SPELL_EFFECT_STEAL_BENEFICIAL_BUFF    spell steal effect?
    &Spell::EffectProspecting,                              //127 SPELL_EFFECT_PROSPECTING              Prospecting spell
    &Spell::EffectUnused,                                   //128 SPELL_EFFECT_APPLY_AREA_AURA_FRIEND
    &Spell::EffectUnused,                                   //129 SPELL_EFFECT_APPLY_AREA_AURA_ENEMY
    &Spell::EffectRedirectThreat,                           //130 SPELL_EFFECT_REDIRECT_THREAT
    &Spell::EffectPlaySound,                                //131 SPELL_EFFECT_PLAY_SOUND               used in some test spells
    &Spell::EffectPlayMusic,                                //132 SPELL_EFFECT_PLAY_MUSIC               sound id in misc value
    &Spell::EffectUnlearnSpecialization,                    //133 SPELL_EFFECT_UNLEARN_SPECIALIZATION   unlearn profession specialization
    &Spell::EffectKillCredit,                               //134 SPELL_EFFECT_KILL_CREDIT              misc value is creature entry
    &Spell::EffectNULL,                                     //135 SPELL_EFFECT_CALL_PET
    &Spell::EffectHealPct,                                  //136 SPELL_EFFECT_HEAL_PCT
    &Spell::EffectEnergizePct,                              //137 SPELL_EFFECT_ENERGIZE_PCT
    &Spell::EffectNULL,                                     //138 SPELL_EFFECT_138                      Leap
    &Spell::EffectUnused,                                   //139 SPELL_EFFECT_139                      unused
    &Spell::EffectForceCast,                                //140 SPELL_EFFECT_FORCE_CAST
    &Spell::EffectForceCastWithValue,                       //141 SPELL_EFFECT_FORCE_CAST_WITH_VALUE
    &Spell::EffectTriggerSpell,                             //142 SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE
    &Spell::EffectUnused,                                   //143 SPELL_EFFECT_APPLY_AREA_AURA_OWNER
    &Spell::EffectKnockBack,                                //144 SPELL_EFFECT_KNOCK_BACK_DEST             Spectral Blast
    &Spell::EffectPullTowards,                              //145 SPELL_EFFECT_PULL_TOWARDS_DEST        Black Hole Effect (only 46230 on BC)
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
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || unitTarget->IsAlive())
        return;

    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if(!unitTarget->IsInWorld())
        return;

    //sun: According to WoWWiki, silent fail on cross faction res
    if (m_caster->GetTypeId() == TYPEID_PLAYER && !sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP))
        if (m_caster->ToPlayer()->GetTeam() != unitTarget->ToPlayer()->GetTeam())
            return;

    Player* target = (unitTarget->ToPlayer());

    if(target->isRessurectRequested())       // already have one active request
        return;

    uint32 health = damage;
    uint32 mana = m_spellInfo->Effects[i].MiscValue;
    ExecuteLogEffectResurrect(i, target);
    target->setResurrectRequestData(m_caster->GetGUID(), m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest(target);
}

void Spell::EffectInstaKill(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsAlive())
        return;

    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
        if (unitTarget->ToPlayer()->GetCommandStatus(CHEAT_GOD))
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
                TC_LOG_ERROR("spells","EffectInstaKill: Unhandled creature entry (%u) case.",entry);
                return;
        }

        m_caster->CastSpell(m_caster,spellID, true);
    }

    if(m_caster==unitTarget)                                // prevent interrupt message
        finish();

    WorldPacket data(SMSG_SPELLINSTAKILLLOG, 8+8+4);
    data << uint64(m_caster->GetGUID());
    data << uint64(unitTarget->GetGUID());
    data << uint32(m_spellInfo->Id);
    m_caster->SendMessageToSet(&data, true);
    
    Unit::DealDamage(_unitCaster, unitTarget, unitTarget->GetHealth(), nullptr, NODAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
}

void Spell::EffectEnvironmentalDMG(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsAlive())
        return;

    // CalcAbsorbResist already in Player::EnvironmentalDamage
    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
        unitTarget->ToPlayer()->EnvironmentalDamage(DAMAGE_FIRE, damage);
    else
    {
        DamageInfo damageInfo(_unitCaster, unitTarget, damage, m_spellInfo, m_spellInfo->GetSchoolMask(), SPELL_DIRECT_DAMAGE, BASE_ATTACK);
        Unit::CalcAbsorbResist(damageInfo);

        uint32 absorb = damageInfo.GetAbsorb();
        uint32 resist = damageInfo.GetResist();
        if (_unitCaster)
            _unitCaster->SendSpellNonMeleeDamageLog(unitTarget, m_spellInfo->Id, damage, m_spellInfo->GetSchoolMask(), absorb, resist, false, 0, false);
    }
}

void Spell::EffectSchoolDMG(uint32 effect_idx)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    bool addBonusDamage = true;
    if (unitTarget && unitTarget->IsAlive())
    {
        switch (m_spellInfo->SpellFamilyName)
        {
        case SPELLFAMILY_GENERIC:
        {
            //Gore
            if (m_spellInfo->SpellIconID == 2269)
            {
                damage += (uint32)rand32() % 2 ? damage : 0;
            }

            // Meteor like spells (divided damage to targets)
            if (m_spellInfo->HasAttribute(SPELL_ATTR0_CU_SHARE_DAMAGE))
            {
                uint32 count = 0;
                for (auto& ihit : m_UniqueTargetInfo)
                {
                    if (ihit.MissCondition != SPELL_MISS_NONE)
                        continue;

                    if (ihit.EffectMask & (1 << effect_idx))
                        ++count;
                }

                if (count)
                    damage /= count;                    // divide to all targets
            }

            ///@todo: move those to scripts
            switch (m_spellInfo->Id)                     // better way to check unknown
            {
            case 35354: //Hand of Death
            {
                if (unitTarget && unitTarget->HasAura(38528)) //Protection of Elune
                {
                    damage = 0;
                }
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
                if (damage < 200)
                    damage = 200;
                break;
            }
            // arcane charge. must only affect demons (also undead?)
            case 45072:
            {
                if (unitTarget->GetCreatureType() != CREATURE_TYPE_DEMON
                    && unitTarget->GetCreatureType() != CREATURE_TYPE_UNDEAD)
                    return;
                break;
            }
            // gruul's shatter
            case 33671:
            {
                // don't damage self and only *ALIVE* players
                if (unitTarget->GetGUID() == m_caster->GetGUID() || unitTarget->GetTypeId() != TYPEID_PLAYER || !unitTarget->IsAlive())
                    return;

                float radius = m_spellInfo->Effects[0].CalcRadius(m_caster->GetSpellModOwner());
                if (!radius)
                    return;
                float distance = m_caster->GetDistance2d(unitTarget);
                damage = (distance > radius) ? 0 : (int32)(m_spellInfo->Effects[0].BasePoints*((radius - distance) / radius));
                break;
            }
            // Cataclysmic Bolt
            case 38441:
                damage = unitTarget->GetMaxHealth() / 2;
                break;
                // Illidan Flame Burst
            case 41126:
            {
                if (!unitTarget || !unitTarget->IsAlive())
                    return;

                break;
            }
            case 31944:
            {
                if (!unitTarget)
                    return;

                if (m_damage > unitTarget->GetHealth()) {
                    if (Creature* archimonde = unitTarget->FindNearestCreature(17968, 100.0f, true))
                        archimonde->AI()->KilledUnit(unitTarget);
                }
                break;
            }
            case 46768:
            {
                if (!unitTarget)
                    return;

                if (!unitTarget->HasAura(45345)) {
                    if (unitTarget->HasAura(45347)) {
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
                if (!unitTarget || !_unitCaster)
                    return;

                if (!_unitCaster->IsWithinMeleeRange(unitTarget))
                    return;

                break;
            }
            case 45779: //Felfire Fission
            {
                if (!unitTarget || !_unitCaster)
                    return;
                
                if(TempSummon* u = _unitCaster->ToTempSummon())
                    if (Unit* summoner = u->GetSummoner())
                        if (unitTarget == summoner)
                            return;

                break;
            }
            case 45664:
                damage = damage / 2;
                break;
            case 37433:
                if (!unitTarget)
                    return;

                if (unitTarget->IsPet())
                    return;

                break;
            }
            break;
        }

        case SPELLFAMILY_MAGE:
        {
            // Arcane Blast
            if (m_spellInfo->SpellFamilyFlags & 0x20000000LL)
            {
                m_caster->CastSpell(m_caster, 36032, true);
            }
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Bloodthirst
            if (m_spellInfo->SpellFamilyFlags & 0x40000000000LL)
            {
                damage = uint32(damage * (_unitCaster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget)) / 100);
            }
            // Shield Slam
            else if (m_spellInfo->SpellFamilyFlags & 0x100000000LL)
                damage += int32(_unitCaster->GetShieldBlockValue());
            // Victory Rush
            else if (m_spellInfo->SpellFamilyFlags & 0x10000000000LL)
            {
                damage = uint32(damage * _unitCaster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) / 100);
            }
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            if (!_unitCaster)
                break;

            // Incinerate Rank 1 & 2
            if ((m_spellInfo->SpellFamilyFlags & 0x00004000000000LL) && m_spellInfo->SpellIconID == 2128)
            {
                // Incinerate does more dmg (dmg*0.25) if the target is Immolated.
                if (unitTarget->HasAuraState(AURA_STATE_CONFLAGRATE))
                    damage += int32(damage*0.25);
            }

            // Conflagrate - consumes immolate 
            if (m_spellInfo->TargetAuraState == AURA_STATE_CONFLAGRATE)
            {
                bool found = false;
                // for caster applied auras only
                Unit::AuraEffectList const &mPeriodic = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
                for (auto i : mPeriodic)
                {
                    if (i->GetSpellInfo()->SpellFamilyName == SPELLFAMILY_WARLOCK && (i->GetSpellInfo()->SpellFamilyFlags & 4) &&
                        i->GetCasterGUID() == m_caster->GetGUID())
                    {
                        unitTarget->RemoveAura(i->GetId(), m_caster->GetGUID());
                        found = true;
                        break;
                    }
                }
                // FIXME - This check should be done earlier, at this point the effect is sent and the mana spent
                if (!found) {
                    SendCastResult(SPELL_FAILED_TARGET_AURASTATE);
                    if (m_caster->GetTypeId() == TYPEID_PLAYER) {
                        _unitCaster->GetSpellHistory()->ResetCooldown(m_spellInfo->Id, true);
                    }
                    return;
                }
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            if (!_unitCaster)
                break;

            // L5 Arcane Charge
            if (m_spellInfo->Id == 41360)
            {
                damage = unitTarget->GetMaxHealth();
                addBonusDamage = false;
                break;
            }
            // Ferocious Bite
            else if ((m_spellInfo->SpellFamilyFlags & 0x000800000) && m_spellInfo->SpellVisual == 6587)
            {
                // converts each extra point of energy into ($f1+$AP/630) additional damage
                float multiple = _unitCaster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) / 630 + m_spellInfo->Effects[effect_idx].DamageMultiplier;
                damage += int32(_unitCaster->GetPower(POWER_ENERGY) * multiple);
                _unitCaster->SetPower(POWER_ENERGY, 0);
            }
            // Rake
            else if (m_spellInfo->SpellFamilyFlags & 0x0000000000001000LL)
            {
                damage += int32(_unitCaster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) / 100);
            }
            // Starfire
            else if (m_spellInfo->SpellFamilyFlags & 0x0004LL)
            {
                Unit::AuraEffectList const& m_OverrideClassScript = _unitCaster->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                for (auto i : m_OverrideClassScript)
                {
                    // Starfire Bonus (caster)
                    switch (i->GetMiscValue())
                    {
                    case 5481:                      // Nordrassil Regalia - bonus
                    {
                        Unit::AuraEffectList const& m_periodicDamageAuras = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
                        for (auto m_periodicDamageAura : m_periodicDamageAuras)
                        {
                            // Moonfire or Insect Swarm (target debuff from any casters)
                            if (m_periodicDamageAura->GetSpellInfo()->SpellFamilyFlags & 0x00200002LL)
                            {
                                int32 mod = i->GetAmount();
                                damage += damage*mod / 100;
                                break;
                            }
                        }
                        break;
                    }
                    case 5148:                      //Improved Starfire - Ivory Idol of the Moongoddes Aura
                    {
                        damage += i->GetAmount();
                        break;
                    }
                    }
                }
            }
            //Mangle Bonus for the initial damage of Lacerate and Rake
            if ((m_spellInfo->SpellFamilyFlags == 0x0000000000001000LL && m_spellInfo->SpellIconID == 494) ||
                (m_spellInfo->SpellFamilyFlags == 0x0000010000000000LL && m_spellInfo->SpellIconID == 2246))
            {
                Unit::AuraEffectList const& mDummyAuras = unitTarget->GetAuraEffectsByType(SPELL_AURA_DUMMY);
                for (auto mDummyAura : mDummyAuras)
                    if (mDummyAura->GetSpellInfo()->SpellFamilyFlags & 0x0000044000000000LL && mDummyAura->GetSpellInfo()->SpellFamilyName == SPELLFAMILY_DRUID)
                    {
                        damage = int32(damage*(100.0f + mDummyAura->GetAmount()) / 100.0f);
                        break;
                    }
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            if (!_unitCaster)
                break;

            // Envenom
            if (_unitCaster->GetTypeId() == TYPEID_PLAYER && (m_spellInfo->SpellFamilyFlags & 0x800000000LL))
            {
                // consume from stack dozes not more that have combo-points
                if (uint32 combo = (_unitCaster->ToPlayer())->GetComboPoints())
                {
                    // count consumed deadly poison doses at target
                    uint32 doses = 0;

                    // remove consumed poison doses
                    Unit::AuraEffectList const& auras = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (auto aura : auras)
                    {
                        // Deadly poison (only attacker applied)
                        if (aura->GetSpellInfo()->SpellFamilyName == SPELLFAMILY_ROGUE && (aura->GetSpellInfo()->SpellFamilyFlags & 0x10000) &&
                            aura->GetSpellInfo()->SpellVisual == 5100 && aura->GetCasterGUID() == m_caster->GetGUID())
                        {
                            doses = aura->GetBase()->GetStackAmount();

                            if (doses > combo)
                                doses = combo;

                            for (int j = 0; j < doses; j++)
                                unitTarget->RemoveSingleAuraFromStack(aura->GetId());

                            break;
                        }
                    }

                    damage *= doses;
                    damage += int32((m_caster->ToPlayer())->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) * 0.03f * doses);

                    // Eviscerate and Envenom Bonus Damage (item set effect)
                    if (_unitCaster->GetDummyAura(37169))
                        damage += (_unitCaster->ToPlayer())->GetComboPoints() * 40;
                }
            }
            // Eviscerate
            else if ((m_spellInfo->SpellFamilyFlags & 0x00020000LL) && _unitCaster->GetTypeId() == TYPEID_PLAYER)
            {
                if (uint32 combo = (_unitCaster->ToPlayer())->GetComboPoints())
                {
                    damage += int32(_unitCaster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) * combo * 0.03f);

                    // Eviscerate and Envenom Bonus Damage (item set effect)
                    if (_unitCaster->GetDummyAura(37169))
                        damage += combo * 40;
                }
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            if (!_unitCaster)
                break;

            // Mongoose Bite
            if ((m_spellInfo->SpellFamilyFlags & 0x000000002) && m_spellInfo->SpellVisual == 342)
            {
                damage += int32(_unitCaster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget)*0.2);
            }
            // Arcane Shot
            else if ((m_spellInfo->SpellFamilyFlags & 0x00000800) && m_spellInfo->MaxLevel > 0)
            {
                damage += int32(_unitCaster->GetTotalAttackPowerValue(RANGED_ATTACK, unitTarget)*0.15);
            }
            // Steady Shot
            else if (m_spellInfo->SpellFamilyFlags & 0x100000000LL)
            {
                // Add Ammo and Weapon damage plus RAP * 0.1
                float dmg_min = 0.f;
                float dmg_max = 0.f;
                for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
                {
                    dmg_min += _unitCaster->GetWeaponDamageRange(RANGED_ATTACK, MINDAMAGE, i);
                    dmg_max += _unitCaster->GetWeaponDamageRange(RANGED_ATTACK, MAXDAMAGE, i);
                }

                int32 base;
                if (dmg_max == 0.0f && dmg_min > dmg_max)
                    base = int32(dmg_min);
                else
                    base = irand(int32(dmg_min), int32(dmg_max));

                damage += int32(float(base) / _unitCaster->GetAttackTime(RANGED_ATTACK) * 2800 + _unitCaster->GetTotalAttackPowerValue(RANGED_ATTACK, unitTarget)*0.2f);
                if (Player* caster = _unitCaster->ToPlayer())
                    damage += int32(caster->GetAmmoDPS() * _unitCaster->GetAttackTime(RANGED_ATTACK) * 0.001f);

                bool found = false;

                // check dazed affect
                Unit::AuraEffectList const& decSpeedList = unitTarget->GetAuraEffectsByType(SPELL_AURA_MOD_DECREASE_SPEED);
                for (auto iter : decSpeedList)
                {
                    if (iter->GetSpellInfo()->SpellIconID == 15 && iter->GetSpellInfo()->Dispel == 0)
                    {
                        found = true;
                        break;
                    }
                }

                //TODO: should this be put on taken but not done?
                if (found)
                    damage += m_spellInfo->Effects[1].BasePoints;
            }
            //Explosive Trap Effect
            else if (m_spellInfo->SpellFamilyFlags & 0x00000004)
            {
                damage += int32(_unitCaster->GetTotalAttackPowerValue(RANGED_ATTACK, unitTarget)*0.1);
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            if (!_unitCaster)
                break;

            //Judgement of Vengeance
            if ((m_spellInfo->SpellFamilyFlags & 0x800000000LL) && m_spellInfo->SpellIconID == 2292)
            {
                uint32 stacks = 0;
                Unit::AuraEffectList const& auras = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE);
                for (auto aura : auras)
                    if (aura->GetId() == 31803 && aura->GetCasterGUID() == _unitCaster->GetGUID())
                        stacks = aura->GetBase()->GetStackAmount();
                if (!stacks)
                    //No damage if the target isn't affected by this
                    damage = -1;
                else
                    damage *= stacks;
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            if (!_unitCaster)
                break;

            // Lightning and Chain Lightning
            if (m_spellInfo->SpellFamilyFlags & 0x0003LL)
            {
                Unit::AuraEffectList const& m_OverrideClassScript = _unitCaster->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                for (auto i : m_OverrideClassScript)
                {
                    // Increased Lightning Damage
                    switch (i->GetMiscValue())
                    {
                    case 4554:
                    case 5142:
                    case 6008:
                    {
                        damage += i->GetAmount();
                        break;
                    }
                    }
                }
            }
            break;
        }
        }

        if (_unitCaster && damage > 0 && addBonusDamage)
        {
            damage = _unitCaster->SpellDamageBonusDone(unitTarget, m_spellInfo, (uint32)damage, SPELL_DIRECT_DAMAGE, effect_idx, {});
            damage = unitTarget->SpellDamageBonusTaken(_unitCaster, m_spellInfo, damage, SPELL_DIRECT_DAMAGE);
        }

        m_damage += damage;
    }
}

void Spell::EffectDummy(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

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
                // Fel Reaver Sentinel Tag // quest 10612
                case 38020:
                {
                    if (!unitTarget)
                        break;
                   
                    Player* charmer = unitTarget->GetCharmerOrOwnerPlayerOrPlayerItself();
                    if (!charmer)
                        break;

                    charmer->KilledMonsterCredit(21959, ObjectGuid::Empty, 10612);
                    break;
                }
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
                    if(_unitCaster)
                        _unitCaster->RemoveAurasDueToSpell(7656);
                    break;
                }
                // Self Force Bow
                case 25716:
                {
                    if (_unitCaster)
                       _unitCaster->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                    break;
                }
                // Melodious Rapture (quest 6661)
                case 21050:
                {
                    if (_unitCaster && unitTarget && unitTarget->ToCreature() && unitTarget->GetEntry() == 13016)
                    {
                        if (m_caster->ToPlayer() && m_caster->ToPlayer()->GetQuestStatus(6661) == QUEST_STATUS_INCOMPLETE) 
                        {
                            unitTarget->ToCreature()->UpdateEntry(13017);
                            unitTarget->SetFaction(m_caster->GetFaction());
                            unitTarget->GetMotionMaster()->MoveFollow(_unitCaster, PET_FOLLOW_DIST, unitTarget->GetFollowAngle());
                            m_caster->ToPlayer()->KilledMonsterCredit(13017, unitTarget->GetGUID());
                        }
                    }
                    
                    break;
                }
                // Placing Smokey's Explosives (quest 6041)
                case 19250:
                {
                    if (gameObjTarget && m_caster->ToPlayer()) {
                        if (i == 0)
                            m_caster->ToPlayer()->KilledMonsterCredit(12247, ObjectGuid::Empty);
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
                    //(should be spell 13258)
                    if (Creature* bomb = m_caster->SummonCreature(8937, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000))
                        bomb->SetFaction(m_caster->GetFaction());
                    
                    break;
                }
                // Druid Signal
                case 38782:
                {
                    if (m_caster->GetAreaId() == 3831)
                        if (Creature *druid = m_caster->SummonCreature(22423, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 45000)) {
                            druid->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            druid->CastSpell(druid, 39158, TRIGGERED_NONE);
                        }
                    break;
                }
                // Expose Razorthorn Root
                case 44935:
                {
                    if (_unitCaster && _unitCaster->GetTypeId() == TYPEID_UNIT && _unitCaster->GetEntry() == 24922) {
                        if (GameObject* root = _unitCaster->FindNearestGameObject(187073, 15.0f)) {
                            if (!root->isSpawned())
                                break;
                            _unitCaster->GetMotionMaster()->MovePoint(0, root->GetPositionX(), root->GetPositionY(), root->GetPositionZ());
                            _unitCaster->SummonGameObject(187072, root->GetPosition(), G3D::Quat(), (root->GetRespawnTime() - GameTime::GetGameTime()));
                            root->SetLootState(GO_JUST_DEACTIVATED);
                        }
                        else
                            TC_LOG_ERROR("FIXME","EffectDummy: Spell 44935 not casted by the right creature.");
                        break;
                    }
                    break;
                }
                // Wrath of the Astromancer
                case 42784:
                {
                    uint32 count = 0;
                    for(auto & ihit : m_UniqueTargetInfo)
                        if(ihit.EffectMask & (1<<i))
                            ++count;

                    damage = 12000; // maybe wrong value
                    damage /= count;

                    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(42784);

                     // now deal the damage
                    for(auto & ihit : m_UniqueTargetInfo)
                        if(ihit.EffectMask & (1<<i))
                            {
                                Unit* casttarget = ObjectAccessor::GetUnit((*unitTarget), ihit.TargetGUID);
                                if(casttarget && _unitCaster)
                                    Unit::DealDamage(_unitCaster, casttarget, damage, nullptr, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_ARCANE, spellInfo, false);
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
                    if (!unitTarget || !unitTarget->IsAlive())
                        return;

                    m_caster->CastSpell(unitTarget, RAND(8401,8408,930,118,1680,10159) , true);
                }break;
                // Gnomish Shrink Ray
                case 13006:
                {
                    if (!unitTarget || !unitTarget->IsAlive())
                        return;

                    if(m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                        
                    // These rates are hella random; someone feel free to correct them
                    uint32 roll = urand(0, 99);
                    if (roll < 3)                                         // Whole party will grow
                        m_caster->CastSpell(m_caster, 13004, true);
                    else if (roll < 6)                                    // Whole party will shrink
                        m_caster->CastSpell(m_caster, 13010, true);
                    else if (roll < 9)                                    // Whole enemy 'team' will grow
                        m_caster->CastSpell(unitTarget, 13004, true);
                    else if (roll < 12)                                    // Whole enemy 'team' will shrink
                        m_caster->CastSpell(unitTarget, 13010, true);
                    else if (roll < 24)                                   // Caster will shrink
                        m_caster->CastSpell(m_caster, 13003, true);
                    else                                                  // Enemy target will shrink
                        m_caster->CastSpell(unitTarget, 13003, true);

                    return;
                }
                case 13180:                                 // Gnomish Mind Control Cap (ItemID: 10726)
                {
                    if (unitTarget && m_CastItem)
                    {
                        uint32 roll = urand(0, 9);
                        CastSpellExtraArgs args;
                        args.TriggerFlags = TRIGGERED_FULL_MASK;
                        args.SetCastItem(m_CastItem);

                        if (roll == 1 && unitTarget->GetTypeId() == TYPEID_PLAYER) //Backfire
                            unitTarget->CastSpell(m_caster, 13181, args);
                        else if (roll)
                            unitTarget->CastSpell(unitTarget, 13181, args);
                    }
                        
                    return;
                }
                // Gnomish Universal Remote
                case 8344:
                {
                    if (!unitTarget || !unitTarget->IsAlive())
                        return;

                    if(m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                        
                    switch (rand()%5) {
                    case 0:
                    case 1:
                    case 2:
                        m_caster->CastSpell(unitTarget, 8345, true); //Control the machine 
                        break;
                    case 3:
                        m_caster->CastSpell(unitTarget ,8346, true); //Malfunction the machin
                        break;
                    case 4:
                        m_caster->CastSpell(unitTarget, 8347, true); //Taunt
                        break;
                    }
                    
                    return;
                }
                // Encapsulate Voidwalker
                case 29364:
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || (unitTarget->ToCreature())->IsPet()) return;

                    Creature* creatureTarget = unitTarget->ToCreature();
                    auto  pGameObj = new GameObject;

                    if (!creatureTarget || !pGameObj) return;

                    if (!pGameObj->Create(creatureTarget->GetMap()->GenerateLowGuid<HighGuid::GameObject>(), 181574, creatureTarget->GetMap(), unitTarget->GetPhaseMask(),
                        creatureTarget->GetPosition(), G3D::Quat(), 255, GO_STATE_READY))
                    {
                        delete pGameObj;
                        return;
                    }

                    pGameObj->SetRespawnTime(0);
                    pGameObj->SetOwnerGUID(m_caster->GetGUID());
                    //pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->GetLevel());
                    pGameObj->SetSpellId(m_spellInfo->Id);

                    creatureTarget->GetMap()->AddToMap(pGameObj);

                    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
                    data << uint64(pGameObj->GetGUID());
                    m_caster->SendMessageToSet(&data,true);

                    return;
                }
                // Demon Broiled Surprise
                case 43723:
                {
                    if (!unitTarget || unitTarget->IsAlive() || unitTarget->GetTypeId() != TYPEID_UNIT ||
                        (unitTarget->ToCreature())->IsPet()) return;

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

                    uint32 _spell_id = 0;
                    switch(urand(1,5))
                    {
                        case 1: spell_id = 8064; break;     // Sleepy
                        case 2: spell_id = 8065; break;     // Invigorate
                        case 3: spell_id = 8066; break;     // Shrink
                        case 4: spell_id = 8067; break;     // Party Time!
                        case 5: spell_id = 8068; break;     // Healthy Spirit
                    }
                    m_caster->CastSpell(m_caster, _spell_id, true);
                    return;
                }
                case 8213:                                  // Savory Deviate Delight
                {
                    if(!_unitCaster || _unitCaster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    uint32 spell_id_ = 0;
                    switch(urand(1,2))
                    {
                        // Flip Out - ninja
                        case 1: spell_id_ = (_unitCaster->GetGender() == GENDER_MALE ? 8219 : 8220); break;
                        // Yaaarrrr - pirate
                        case 2: spell_id_ = (_unitCaster->GetGender() == GENDER_MALE ? 8221 : 8222); break;
                    }
                    _unitCaster->CastSpell(_unitCaster, spell_id_, true);
                    return;
                }
                case 8593:                                  // Symbol of life (restore creature to life)
                case 31225:                                 // Shimmering Vessel (restore creature to life)
                {
                    if(!unitTarget || unitTarget->GetTypeId()!=TYPEID_UNIT)
                        return;
                    (unitTarget->ToCreature())->SetDeathState(JUST_RESPAWNED);
                    return;
                }
                case 12162:                                 // Deep wounds
                case 12850:                                 // (now good common check for this spells)
                case 12868:
                {
                    if(!unitTarget || !_unitCaster)
                        return;

                    float localDamage;
                    // DW should benefit of attack power, damage percent mods etc.
                    // TODO: check if using offhand damage is correct and if it should be divided by 2
                    if (_unitCaster->HaveOffhandWeapon() && _unitCaster->GetAttackTimer(BASE_ATTACK) > _unitCaster->GetAttackTimer(OFF_ATTACK))
                        localDamage = (_unitCaster->GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE) + _unitCaster->GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE))/2;
                    else
                        localDamage = (_unitCaster->GetFloatValue(UNIT_FIELD_MINDAMAGE) + _unitCaster->GetFloatValue(UNIT_FIELD_MAXDAMAGE))/2;

                    switch (m_spellInfo->Id)
                    {
                        case 12850: localDamage *= 0.2f; break;
                        case 12162: localDamage *= 0.4f; break;
                        case 12868: localDamage *= 0.6f; break;
                        default:
                            TC_LOG_ERROR("FIXME","Spell::EffectDummy: Spell %u not handled in DW",m_spellInfo->Id);
                            return;
                    };

                    int32 deepWoundsDotBasePoints0 = int32(damage / 4);
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellBP0(int32(deepWoundsDotBasePoints0));
                    m_caster->CastSpell(unitTarget, 12721, args);
                    return;
                }
                case 12975:                                 //Last Stand
                {
                    if (!_unitCaster)
                        return;

                    int32 healthModSpellBasePoints0 = int32(_unitCaster->GetMaxHealth()*0.3);
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellBP0(int32(healthModSpellBasePoints0));
                    _unitCaster->CastSpell(_unitCaster, 12976, args);
                    return;
                }
                case 13120:                                 // net-o-matic
                {
                    if(!unitTarget)
                        return;

                    uint32 _spell_id = 0;

                    uint32 roll = urand(0, 99);

                    if(roll < 2)                            // 2% for 30 sec self root (off-like chance unknown)
                        _spell_id = 16566;
                    else if(roll < 4)                       // 2% for 20 sec root, charge to target (off-like chance unknown)
                        _spell_id = 13119;
                    else                                    // normal root
                        _spell_id = 13099;

                    m_caster->CastSpell(unitTarget, _spell_id, true);
                    return;
                }
                case 13280:                                 // Gnomish Death Ray
                {
                    if (!unitTarget)
                        return;
                        
                    if (urand(0, 99) < 15)
                        m_caster->CastSpell(m_caster, 13493, true);    // failure
                    else
                        m_caster->CastSpell(unitTarget, 13279, true);
                        
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
                        {
                            CastSpellExtraArgs args;
                            args.TriggerFlags = TRIGGERED_FULL_MASK;
                            args.AddSpellBP0(int32(damage));
                            m_caster->CastSpell(unitTarget, 26470, args);
                            break;
                        }
                        default:
                            TC_LOG_ERROR("FIXME","EffectDummy: Non-handled case for spell 13567 for triggered aura %u",m_triggeredByAuraSpell->Id);
                            break;
                    }
                    return;
                }
                case 15998:                                 // Capture Worg Pup
                case 29435:                                 // Capture Female Kaliri Hatchling
                {
                    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT)
                        return;

                    Creature* creatureTarget = unitTarget->ToCreature();
                    creatureTarget->SetDeathState(JUST_DIED);
                    creatureTarget->RemoveCorpse();
                    creatureTarget->SetHealth(0);           // just for nice GM-mode view
                    return;
                }
                case 16589:                                 // Noggenfogger Elixir
                {
                    if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    uint32 _spell_id = 0;
                    switch(urand(1,3))
                    {
                        case 1: spell_id = 16595; break;
                        case 2: spell_id = 16593; break;
                        default:spell_id = 16591; break;
                    }

                    m_caster->CastSpell(m_caster, _spell_id, true);
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
                        (m_originalCaster->ToPlayer())->SendDirectMessage( &data );
                    }
                    return;
                }
                case 17271:                                 // Test Fetid Skull
                {
                    if(!itemTarget && m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    uint32 spell_id_ = roll_chance_i(50) ? 17269 : 17270;

                    m_caster->CastSpell(m_caster, spell_id_, true);
                    return;
                }
                case 23019:                                 // Crystal Prison Dummy DND
                {
                    if(!m_caster->ToCreature() || (m_caster->ToCreature() && m_caster->ToCreature()->GetEntry() != 12396))
                        return;

                    Creature* creatureTarget = m_caster->ToCreature();
                    if(creatureTarget->IsPet())
                        return;

                    creatureTarget->SetDeathState(JUST_DIED);
                    creatureTarget->RemoveCorpse();
                    creatureTarget->SetHealth(0);                   // just for nice GM-mode view

                    GameObject* Crystal_Prison = m_caster->SummonGameObject(179644, creatureTarget->GetPosition(), G3D::Quat(), creatureTarget->GetRespawnTime() - GameTime::GetGameTime());
                    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
                    data << uint64(Crystal_Prison->GetGUID());
                    m_caster->SendMessageToSet(&data,true);

                    return;
                }
                case 23074:                                 // Arc. Dragonling
                {
                    if (!m_CastItem) return;

                    m_caster->CastSpell(m_caster, 19804, m_CastItem);
                    return;
                }
                case 23075:                                 // Mithril Mechanical Dragonling
                {
                    if (!m_CastItem) return;

                    m_caster->CastSpell(m_caster, 12749, m_CastItem);
                    return;
                }
                case 23076:                                 // Mechanical Dragonling
                {
                    if (!m_CastItem) return;

                    m_caster->CastSpell(m_caster, 4073, m_CastItem);
                    return;
                }
                case 23133:                                 // Gnomish Battle Chicken
                {
                    if (!m_CastItem) return;

                    m_caster->CastSpell(m_caster, 13166, m_CastItem);
                    return;
                }
                case 23138: //Gate of Shazzrah
                {
                    if(!unitTarget || !_unitCaster)
                        return;

                    float x = unitTarget->GetPositionX();
                    float y = unitTarget->GetPositionY();
                    float z = unitTarget->GetPositionZ();

                    _unitCaster->Relocate(x, y, z);
                    _unitCaster->MonsterMoveWithSpeed(x, y, z, 0);
                    _unitCaster->CastSpell(unitTarget, 19712, TRIGGERED_NONE);
                    if (_unitCaster->ToCreature())
                        if (_unitCaster->ToCreature()->AI())
                            _unitCaster->ToCreature()->GetThreatManager().ClearAllThreat();
                    return;
                }
                case 23448:                                 // Ultrasafe Transporter: Gadgetzan - backfires
                {
                  int32 r = irand(0, 119);
                    if ( r < 20 )                           // 1/6 polymorph
                        m_caster->CastSpell(m_caster,23444, true);
                    else if ( r < 100 )                     // 4/6 evil twin
                        m_caster->CastSpell(m_caster,23445, true);
                    else                                    // 1/6 miss the target
                        m_caster->CastSpell(m_caster,36902, true);
                    return;
                }
                case 23453:                                 // Ultrasafe Transporter: Gadgetzan
                    if ( roll_chance_i(50) )                // success
                        m_caster->CastSpell(m_caster,23441, true);
                    else                                    // failure
                        m_caster->CastSpell(m_caster,23446, true);
                    return;
                case 23645:                                 // Hourglass Sand
                    if(_unitCaster)
                        _unitCaster->RemoveAurasDueToSpell(23170);
                    return;
                case 23725:                                 // Gift of Life (warrior bwl trinket)
                    m_caster->CastSpell(m_caster,23782, true);
                    m_caster->CastSpell(m_caster,23783, true);
                    return;
                case 24930:                                 // Hallow's End Candy
                    if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    switch(irand(0,3))
                    {
                    case 0:
                        m_caster->CastSpell(m_caster,24927, true); // Ghost
                        break;
                    case 1:
                        if (!_unitCaster)
                            break;
                        _unitCaster->CastSpell(m_caster,24926, true); // Pirate
                        if(_unitCaster->GetGender() == GENDER_MALE)
                            _unitCaster->CastSpell(_unitCaster,44743, true);
                        else
                            _unitCaster->CastSpell(_unitCaster,44742, true);
                        break;
                    case 2:
                        m_caster->CastSpell(m_caster,24925, true); // Skeleton
                        break;
                    case 3:
                        m_caster->CastSpell(m_caster,24924, true); // Huge and Orange
                        break;
                    }
                    return;
                case 25860:                                 // Reindeer Transformation
                {
                    if (!_unitCaster || !_unitCaster->HasAuraType(SPELL_AURA_MOUNTED))
                        return;

                    float flyspeed = _unitCaster->GetSpeedRate(MOVE_FLIGHT);
                    float speed = _unitCaster->GetSpeedRate(MOVE_RUN);

                    _unitCaster->RemoveAurasByType(SPELL_AURA_MOUNTED);

                    //5 different spells used depending on mounted speed and if mount can fly or not
                    if (flyspeed >= 4.1f)
                        _unitCaster->CastSpell(_unitCaster, 44827, true); //310% flying Reindeer
                    else if (flyspeed >= 3.8f)
                        _unitCaster->CastSpell(_unitCaster, 44825, true); //280% flying Reindeer
                    else if (flyspeed >= 1.6f)
                        _unitCaster->CastSpell(_unitCaster, 44824, true); //60% flying Reindeer
                    else if (speed >= 2.0f)
                        _unitCaster->CastSpell(_unitCaster, 25859, true); //100% ground Reindeer
                    else
                        _unitCaster->CastSpell(_unitCaster, 25858, true); //60% ground Reindeer

                    return;
                }
                //case 26074:                               // Holiday Cheer
                //    return; -- implemented at client side
                case 28006:                                 // Arcane Cloaking
                {
                    if( unitTarget->GetTypeId() == TYPEID_PLAYER )
                        m_caster->CastSpell(unitTarget,29294, true);
                    return;
                }
                case 28730:                                 // Arcane Torrent (Mana)
                {
                    if (!_unitCaster)
                        break;
                    AuraEffect* dummy = _unitCaster->GetDummyAura(28734);
                    if (dummy)
                    {
                        int32 bp_ = damage * dummy->GetBase()->GetStackAmount();
                        CastSpellExtraArgs args;
                        args.TriggerFlags = TRIGGERED_FULL_MASK;
                        args.AddSpellBP0(int32(bp_));
                        _unitCaster->CastSpell(_unitCaster, 28733, args);

                        _unitCaster->RemoveAurasDueToSpell(28734);
                    }
                    return;
                }
                //this spell has no effect in dbc (?!), using item_script instead.
                case 29200:                                 // Purify Helboar Meat
                {
                    if( m_caster->GetTypeId() != TYPEID_PLAYER )
                        return;

                    uint32 spell_id_ = roll_chance_i(50) ? 29277 : 29278;

                    m_caster->CastSpell(m_caster, spell_id_, true);
                    return;
                }
                case 29858:                                 // Soulshatter
                    if (_unitCaster && unitTarget && unitTarget->CanHaveThreatList()
                        && unitTarget->GetThreatManager().GetThreat(_unitCaster) > 0.0f)
                        _unitCaster->CastSpell(unitTarget, 32835, true);
                    return;
                case 30458:                                 // Nigh Invulnerability
                {
                    if (!m_CastItem) return;
                    uint32 castSpellId;
                    if (roll_chance_i(86))                   // success
                        castSpellId = 30456;
                    else                                    // backfire in 14% casts
                        castSpellId = 30457;

                    m_caster->CastSpell(m_caster, castSpellId, m_CastItem);
                    return;
                }
                case 30507:                                 // Poultryizer
                {
                    if (!m_CastItem) return;
                    uint32 castSpellId;
                    if (roll_chance_i(80))                   // success
                        castSpellId = 30501;
                    else                                    // backfire 20%
                        castSpellId = 30504;

                    m_caster->CastSpell(unitTarget, castSpellId, m_CastItem);
                    return;
                }
                case 33060:                                         // Make a Wish
                {
                    if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    uint32 _spell_id = 0;

                    switch(urand(1,5))
                    {
                        case 1: _spell_id = 33053; break;
                        case 2: _spell_id = 33057; break;
                        case 3: _spell_id = 33059; break;
                        case 4: _spell_id = 33062; break;
                        case 5: _spell_id = 33064; break;
                    }

                    m_caster->CastSpell(m_caster, _spell_id, true);
                    return;
                }
                case 35686:                                 // Electro-Shock (Electro-Shock Goodness!)
                {
                    if (!unitTarget || !_unitCaster)
                        return;
                    
                    if (unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->GetEntry() == 20778)
                    {
                        Creature *cr = nullptr;
                        uint8 spawnNum = 6 + rand()%3;
                        for (uint8 idx = 0; idx < spawnNum; idx++)
                        {
                            cr = unitTarget->SummonCreature(20805, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0);
                            if (cr)
                                cr->EngageWithTarget(_unitCaster);
                        }
                        Unit::Kill(_unitCaster, unitTarget, false); // Just for the "burst" animation on death....
                        ( unitTarget->ToCreature() )->RemoveCorpse();
                    }
                    if (unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->GetEntry() == 20501)
                    {
                        Creature *cr = nullptr;
                        uint8 spawnNum = 6 + rand()%3;
                        for (uint8 idx = 0; idx < spawnNum; idx++)
                        {
                            cr = unitTarget->SummonCreature(20806, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 0);
                            if(cr)
                                cr->EngageWithTarget(_unitCaster);
                        }
                        Unit::Kill(_unitCaster, unitTarget, false); // Just for the "burst" animation on death....
                        unitTarget->ToCreature()->RemoveCorpse();
                    }
                    
                    return;
                }
                case 35745:
                {
                    uint32 _spell_id;
                    switch(m_caster->GetAreaId())
                    {
                        case 3900: _spell_id = 35743; break;
                        case 3742: _spell_id = 35744; break;
                        default: return;
                    }

                    m_caster->CastSpell(m_caster, _spell_id, TRIGGERED_FULL_MASK);
                    return;
                }
                case 37674:                                 // Chaos Blast
                {
                    if(!unitTarget)
                        return;

                    int32 basepoints0 = 100;
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellBP0(int32(basepoints0));
                    m_caster->CastSpell(unitTarget, 37675, args);
                    return;
                }
                case 40109:                                 // Knockdown Fel Cannon: The Bolt
                {
                    unitTarget->CastSpell(unitTarget, 40075, TRIGGERED_FULL_MASK);
                    return;
                }
                case 40802:                                 // Mingo's Fortune Generator (Mingo's Fortune Giblets)
                {
                    // selecting one from Bloodstained Fortune item
                    uint32 newitemid;
                    switch(urand(1,20))
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
                    if (i != 0 || !_unitCaster)
                        return;

                    float fDestX, fDestY, fDestZ;
                    _unitCaster->GetNearPoint(_unitCaster, fDestX, fDestY, fDestZ, 30.0f, 0.0f);
                    if (Creature* pWolf = _unitCaster->SummonCreature(25324, fDestX, fDestY, fDestZ, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000))
                        pWolf->GetMotionMaster()->MoveFollow(_unitCaster, PET_FOLLOW_DIST, pWolf->GetAbsoluteAngle(_unitCaster));
                    return;
                }
                case 40834: // Flammes déchirantes
                {
                    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
                        return;

                    m_caster->CastSpell(unitTarget,40932, true);
                    break;
                }
                // Demon Broiled Surprise
                /* FIX ME: Required for correct work implementing implicit target 7 (in pair (22,7))
                case 43723:
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;

                    (m_caster->ToPlayer())->CastSpell(unitTarget, 43753, TRIGGERED_FULL_MASK);
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
                    for (unsigned char & idx : rands)
                        idx = rand() % 2;
 
                    m_caster->CastSpell(unitTarget, Disease[rands[0]], true);
                    m_caster->CastSpell(unitTarget, Curse[rands[1]], true);
                    m_caster->CastSpell(unitTarget, Magic[rands[2]], true);
                    m_caster->CastSpell(unitTarget, Poison[rands[3]], true);
                   
                    // Flip the 4 previous random numbers
                    for (unsigned char & rand : rands)
                        rand = (rand + 1) % 2;
                   
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

                    creatureTarget->SetDeathState(JUST_DIED);
                    creatureTarget->RemoveCorpse();
                    creatureTarget->SetHealth(0);           // just for nice GM-mode view

                    //cast spell Raptor Capture Credit
                    if(Unit * pCaster = m_originalCaster)
                        pCaster->CastSpell(pCaster, 42337, true);
                    return;
                }
                case 37573:                                 //Temporal Phase Modulator
                {
                    if(!unitTarget || !_unitCaster)
                        return;

                    TempSummon* tempSummon = dynamic_cast<TempSummon*>(unitTarget);
                    if(!tempSummon)
                        return;

                    uint32 health = tempSummon->GetHealth();
                    const uint32 entry_list[6] = {21821, 21820, 21817};

                    float x = tempSummon->GetPositionX();
                    float y = tempSummon->GetPositionY();
                    float z = tempSummon->GetPositionZ();
                    float o = tempSummon->GetOrientation();

                    tempSummon->UnSummon();

                    Creature* pCreature = _unitCaster->SummonCreature(entry_list[urand(0, 2)], x, y, z, o,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,180000);
                    if (!pCreature)
                        return;

                    pCreature->SetHealth(health);
                    pCreature->EngageWithTarget(_unitCaster);
                    return;
                }
                case 34665:                                 //Administer Antidote
                {
                    if(!unitTarget || m_caster->GetTypeId() != TYPEID_PLAYER )
                        return;

                    if(!_unitCaster)
                        return;

                    TempSummon* tempSummon = dynamic_cast<TempSummon*>(unitTarget);
                    if(!tempSummon)
                        return;

                    uint32 health = tempSummon->GetHealth();

                    float x = tempSummon->GetPositionX();
                    float y = tempSummon->GetPositionY();
                    float z = tempSummon->GetPositionZ();
                    float o = tempSummon->GetOrientation();
                    tempSummon->UnSummon();

                    Creature* pCreature = _unitCaster->SummonCreature(16992, x, y, z, o,TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,180000);
                    if (!pCreature)
                        return;

                    pCreature->SetHealth(health);
                    (_unitCaster->ToPlayer())->KilledMonsterCredit(16992,pCreature->GetGUID());

                    pCreature->EngageWithTarget(_unitCaster);
                    return;
                }
                case 34063:                                 //Soul Mirror
                {
                    if(!unitTarget || !_unitCaster || m_caster->GetTypeId() != TYPEID_PLAYER )
                        return;

                    Creature* pCreature = _unitCaster->SummonCreature(19480, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), unitTarget->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN,180000);
                    if (!pCreature)
                        return;

                    Unit::Kill(unitTarget, unitTarget, false);
                    if (unitTarget->GetTypeId() == TYPEID_UNIT)
                        (unitTarget->ToCreature())->RemoveCorpse();

                    pCreature->EngageWithTarget(_unitCaster);
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

                    if(Battleground* bg = (m_caster->ToPlayer())->GetBattleground())
                        bg->EventPlayerDroppedFlag(m_caster->ToPlayer());

                    m_caster->CastSpell(m_caster, 30452, true);
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

                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellBP0(int32(heal));
                    m_caster->CastSpell(m_caster, 41249, args);
                    return;
                }
                case 38920:
                {
                    if (Player* player = m_caster->GetCharmerOrOwnerPlayerOrPlayerItself()) {
                        if (Creature* west = m_caster->FindNearestCreature(22348, 12.0f, true))
                            player->KilledMonsterCredit(22348, ObjectGuid::Empty);
                        else if (Creature* center = m_caster->FindNearestCreature(22350, 12.0f, true))
                            player->KilledMonsterCredit(22350, ObjectGuid::Empty);
                        else if (Creature* east = m_caster->FindNearestCreature(22351, 12.0f, true))
                            player->KilledMonsterCredit(22351, ObjectGuid::Empty);
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
                        m_caster->CastSpell(m_caster, 42288, TRIGGERED_NONE);
                        
                    return;
                }
                case 28441:
                {
                    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT)
                        return;

                    if (unitTarget->ToCreature()->IsPet())
                        return;
                        
                    if (unitTarget->ToCreature()->GetReactState() == REACT_PASSIVE)
                        return;
                    
                    if (unitTarget->GetEntry() != 3976 && unitTarget->GetEntry() != 4542) {
                        unitTarget->GetMotionMaster()->MoveIdle();
                        unitTarget->SetInFront(m_caster);
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
                    int32 bp_ = 45000 - (unitTarget->GetDistance(m_caster) * 2500);
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellBP0(int32(bp_));
                    m_caster->CastSpell(unitTarget, 5255, args);
                    return;
                }
            }

            //All IconID Check in there
            switch(m_spellInfo->SpellIconID)
            {
                // Berserking (troll racial traits)
                case 1661:
                {
                    if (!_unitCaster || !_unitCaster)
                        break;
                    uint32 healthPerc = uint32((float(_unitCaster->GetHealth())/ _unitCaster->GetMaxHealth())*100);
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

                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.AddSpellMod(SPELLVALUE_BASE_POINT0, int32(hasteModBasePoints0));
                    args.AddSpellMod(SPELLVALUE_BASE_POINT1, int32(hasteModBasePoints1));
                    args.AddSpellMod(SPELLVALUE_BASE_POINT2, int32(hasteModBasePoints2));
                    _unitCaster->CastSpell(_unitCaster, 26635, args);
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
                    if (!_unitCaster)
                        return;

                    if(_unitCaster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    // immediately finishes the cooldown on Frost spells
                    const PlayerSpellMap& sp_list = (_unitCaster->ToPlayer())->GetSpellMap();
                    for (const auto & itr : sp_list)
                    {
                        if (itr.second->state == PLAYERSPELL_REMOVED)
                            continue;

                        uint32 classspell = itr.first;
                        SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(classspell);
                        if (!spellInfo)
                            continue;

                        if( spellInfo->SpellFamilyName == SPELLFAMILY_MAGE &&
                            (spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST) &&
                            spellInfo->Id != 11958 && spellInfo->GetRecoveryTime() > 0 )
                        {
                            _unitCaster->GetSpellHistory()->ResetCooldown(classspell, true);
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
                        unitTarget->CastSpell( unitTarget, spell_list[urand(0, 5)], TRIGGERED_FULL_MASK);
                    }
                    return;
                }
                case 30610:
                {
                    if (!unitTarget)
                        return;
                        
                    /*unitTarget->CastSpell(unitTarget, RAND(30605, 30606, 30607, 30608, 30609), TRIGGERED_FULL_MASK);
                    return;*/
                    spell_id = RAND(30605, 30606, 30607, 30608, 30609);
                    break;
                }
            }
            break;
        case SPELLFAMILY_WARRIOR:
            if (!_unitCaster)
                break;

            // Charge
            if(m_spellInfo->SpellFamilyFlags & 0x1 && m_spellInfo->SpellVisual == 867)
            {
                int32 chargeBasePoints0 = damage;
                CastSpellExtraArgs args;
                args.TriggerFlags = TRIGGERED_FULL_MASK;
                args.AddSpellBP0(int32(chargeBasePoints0));
                _unitCaster->CastSpell(m_caster, 34846, args);
                return;
            }
            // Execute
            if(m_spellInfo->SpellFamilyFlags & 0x20000000)
            {
                if(!unitTarget)
                    return;

                spell_id = 20647;
                int32 rage = _unitCaster->GetPower(POWER_RAGE);
                if(rage < 0) 
                    rage = 0; //should not happen but let's be sure
                bp = damage + int32(rage * m_spellInfo->Effects[i].DamageMultiplier);
                _unitCaster->SetPower(POWER_RAGE,0);
                break;
            }
            if(m_spellInfo->Id==21977)                      //Warrior's Wrath
            {
                if(!unitTarget)
                    return;

                m_caster->CastSpell(unitTarget,21887, true); // spell mod
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
                            TC_LOG_ERROR("spells","Spell::EffectDummy: Spell 28598 triggered by unhandled spell %u",m_triggeredByAuraSpell->Id);
                            return;
                    }
                    m_caster->CastSpell(unitTarget, spellid, true);
                    return;
                }
            }
            break;
        case SPELLFAMILY_DRUID:
            if (!_unitCaster)
                break;
            switch(m_spellInfo->Id )
            {
                case 5420:                                  // Tree of Life passive
                {
                    // Tree of Life area effect
                    int32 health_mod = int32(_unitCaster->GetStat(STAT_SPIRIT)/4);
                    _unitCaster->CastSpell(_unitCaster, 34123, { SPELLVALUE_BASE_POINT0, health_mod } );
                    return;
                }
            }
            break;
        case SPELLFAMILY_ROGUE:
            switch(m_spellInfo->Id )
            {
                case 31231:                                 // Cheat Death
                {
                    m_caster->CastSpell(m_caster,45182, true);
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

                    // apply poison
                    if (uint32 enchant_id = item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                    {
                        if(SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id))
                        {
                            for (int s=0;s<3;s++)
                            {
                                if(pEnchant->type[s]!=ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL)
                                    continue;

                                SpellInfo const* combatEntry = sSpellMgr->GetSpellInfo(pEnchant->spellid[s]);
                                if(!combatEntry || combatEntry->Dispel != DISPEL_POISON)
                                    continue;

                                CastSpellExtraArgs args;
                                args.TriggerFlags = TRIGGERED_FULL_MASK;
                                args.SetCastItem(item);
                                m_caster->CastSpell(unitTarget, combatEntry->Id, args);
                            }
                        }
                    }

                    //dmg + combo point effect
                    m_caster->CastSpell(unitTarget, 5940, true);
                    return;
                }
            }
            break;
        case SPELLFAMILY_HUNTER:
            // Kill command
            if(m_spellInfo->SpellFamilyFlags & 0x00080000000000LL && _unitCaster)
            {
                if(_unitCaster->GetClass()!=CLASS_HUNTER)
                    return;

                // clear hunter crit aura state
                _unitCaster->ModifyAuraState(AURA_STATE_HUNTER_CRIT_STRIKE,false);

                // additional damage from pet to pet target
                Unit* pet = _unitCaster->GetGuardianPet();
                if(!pet || !pet->GetVictim())
                    return;

                uint32 _spell_id = 0;
                switch (m_spellInfo->Id)
                {
                case 34026: _spell_id = 34027; break;        // rank 1
                default:
                    TC_LOG_ERROR("spells","Spell::EffectDummy: Spell %u not handled in KC",m_spellInfo->Id);
                    return;
                }

                pet->CastSpell(pet->GetVictim(), _spell_id, true);
                return;
            }

            switch(m_spellInfo->Id)
            {
                case 781:                                 // Disengage
                {
                    if (m_caster->GetTypeId() != TYPEID_PLAYER)
                        return;
                        
                    //Todo: use Unit::KnockbackFrom instead
                    WorldPacket data(SMSG_MOVE_KNOCK_BACK, 50);
                    data << m_caster->GetPackGUID();
                    data << GetMSTime();
                    data << float(cosf(m_caster->GetOrientation()+M_PI));
                    data << float(sinf(m_caster->GetOrientation()+M_PI));
                    data << float(15);
                    data << float(-7.0f);
                    (m_caster->ToPlayer())->SendDirectMessage(&data);
                    return;
                }
                case 23989:                                 //Readiness talent
                {
                    if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    //immediately finishes the cooldown for hunter abilities
                    const PlayerSpellMap& sp_list = (m_caster->ToPlayer())->GetSpellMap();
                    for (const auto & itr : sp_list)
                    {
                        uint32 classspell = itr.first;
                        SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(classspell);
                        if (!spellInfo)
                            return;

                        if (spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER && spellInfo->Id != 23989 && spellInfo->GetRecoveryTime() > 0 )
                            _unitCaster->GetSpellHistory()->ResetCooldown(classspell, true);

                    }
                    return;
                }
                case 37506:                                 // Scatter Shot
                {
                    if (m_caster->GetTypeId()!=TYPEID_PLAYER)
                        return;

                    // break Auto Shot and autohit
                    _unitCaster->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
                    _unitCaster->AttackStop();
                    (_unitCaster->ToPlayer())->SendAttackSwingCancelAttack();
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
                            TC_LOG_ERROR("FIXME","Spell::EffectDummy: Spell %u not handled in HS",m_spellInfo->Id);
                            return;
                    }

                    if(m_caster->IsFriendlyTo(unitTarget))
                        m_caster->CastSpell(unitTarget, heal, true);
                    else
                        m_caster->CastSpell(unitTarget, hurt, true);

                    return;
                }
            }

            switch(m_spellInfo->Id)
            {
                case 31789:                                 // Righteous Defense (step 1)
                {
                    // 31989 -> dummy effect (step 1) + dummy effect (step 2) -> 31709 (taunt like spell for each target)

                    // non-standard cast requirement check
                    if (!unitTarget || unitTarget->GetAttackers().empty())
                    {
                        // clear cooldown at fail
                        if(m_caster->GetTypeId()==TYPEID_PLAYER && _unitCaster)
                            _unitCaster->GetSpellHistory()->ResetCooldown(m_spellInfo->Id, true);

                        SendCastResult(SPELL_FAILED_BAD_TARGETS);
                        return;
                    }

                    // Righteous Defense (step 2) (in old version 31980 dummy effect)
                    // Clear targets for eff 1
                    for(auto & ihit : m_UniqueTargetInfo)
                        ihit.EffectMask &= ~(1<<1);

                    // select up to 3 random targets
                    Unit::AttackerSet const& attackers = unitTarget->GetAttackers();
                    std::set<Unit*> targetSet (attackers);
                    size_t setSize = targetSet.size();
                    while (setSize > 3)
                    {
                        auto itr = targetSet.begin();
                        std::advance(itr, urand(0, setSize - 1));
                        targetSet.erase(itr);
                        --setSize;
                    }

                    for(auto itr : targetSet)
                        AddUnitTarget(itr, 1);

                    // now let next effect cast spell at each target.
                    return;
                }
                case 37877:                                 // Blessing of Faith
                {
                    if(!unitTarget)
                        return;

                    uint32 _spell_id = 0;
                    switch(unitTarget->GetClass())
                    {
                        case CLASS_DRUID:   _spell_id = 37878; break;
                        case CLASS_PALADIN: _spell_id = 37879; break;
                        case CLASS_PRIEST:  _spell_id = 37880; break;
                        case CLASS_SHAMAN:  _spell_id = 37881; break;
                        default: return;                    // ignore for not healing classes
                    }

                    m_caster->CastSpell(m_caster, _spell_id, true);
                    return;
                }
            }
            break;
        case SPELLFAMILY_SHAMAN:
            //Shaman Rockbiter Weapon
            if (m_spellInfo->SpellFamilyFlags == 0x400000)
            {
                uint32 _spell_id = 0;
                switch(m_spellInfo->Id)
                {
                    case  8017: _spell_id = 36494; break;    // Rank 1
                    case  8018: _spell_id = 36750; break;    // Rank 2
                    case  8019: _spell_id = 36755; break;    // Rank 3
                    case 10399: _spell_id = 36759; break;    // Rank 4
                    case 16314: _spell_id = 36763; break;    // Rank 5
                    case 16315: _spell_id = 36766; break;    // Rank 6
                    case 16316: _spell_id = 36771; break;    // Rank 7
                    case 25479: _spell_id = 36775; break;    // Rank 8
                    case 25485: _spell_id = 36499; break;    // Rank 9
                    default:
                        TC_LOG_ERROR("spells","Spell::EffectDummy: Spell %u not handled in RW", m_spellInfo->Id);
                        return;
                }

                SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(_spell_id);

                if(!spellInfo)
                {
                    TC_LOG_ERROR("spells","WORLD: unknown spell id %i\n", _spell_id);
                    return;
                }

                if(m_caster->GetTypeId() != TYPEID_PLAYER)
                    return;

                for(int idx = BASE_ATTACK; idx <= OFF_ATTACK; ++idx)
                {
                    if(Item* item = (m_caster->ToPlayer())->GetWeaponForAttack(WeaponAttackType(idx)))
                    {
                        if(item->IsFitToSpellRequirements(m_spellInfo))
                        {
                            auto spell = new Spell(m_caster, spellInfo, TRIGGERED_FULL_MASK);

                            // enchanting spell selected by calculated damage-per-sec in enchanting effect
                            // at calculation applied affect from Elemental Weapons talent
                            // real enchantment damage-1
                            spell->SetSpellValue(SPELLVALUE_BASE_POINT1, damage);

                            SpellCastTargets targets;
                            targets.SetItemTarget( item );
                            spell->prepare(targets);
                        }
                    }
                }
                return;
            }

            if(m_spellInfo->Id == 39610)                    // Mana-Tide Totem effect
            {
                if(!unitTarget || unitTarget->GetPowerType() != POWER_MANA)
                    return;

                // Regenerate 6% of Total Mana Every 3 secs
                int32 EffectBasePoints0 = unitTarget->GetMaxPower(POWER_MANA)  * damage / 100;
                CastSpellExtraArgs args;
                args.TriggerFlags = TRIGGERED_FULL_MASK;
                args.AddSpellBP0(int32(EffectBasePoints0));
                args.SetOriginalCaster(m_originalCasterGUID);
                m_caster->CastSpell(unitTarget, 39609, args);
                return;
            }

            // Flametongue Weapon Proc
            if(m_spellInfo->SpellFamilyFlags &0x0000000000200000LL && _unitCaster)
            {
                if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                    return;

                if(!m_CastItem || !m_CastItem->IsEquipped())
                    return;

                WeaponAttackType attType=BASE_ATTACK;
                if (m_CastItem->GetSlot() == EQUIPMENT_SLOT_OFFHAND)
                    attType=OFF_ATTACK;

                float wspeed = _unitCaster->GetAttackTime(attType)/1000.0f;
                if (wspeed > 4.0) wspeed = 4.0f;
                if (wspeed < 1.3) wspeed = 1.3f;
                int32 EffectBasePoints0 = uint32(damage*wspeed/100.f);
                _unitCaster->CastSpell(unitTarget, 10444, { SPELLVALUE_BASE_POINT0, EffectBasePoints0 });
                return;
            }
            // Flametongue Totem Proc
            if(m_spellInfo->SpellFamilyFlags &0x0000000400000000LL && _unitCaster)
            {
                if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                    return;

                float wspeed = _unitCaster->GetAttackTime(BASE_ATTACK)/1000.0f;
                if (wspeed > 4.0) wspeed = 4.0f;
                if (wspeed < 1.3) wspeed = 1.3f;
                int32 EffectBasePoints0 = uint32(damage*wspeed/100.f);
                _unitCaster->CastSpell(unitTarget, 16368, { SPELLVALUE_BASE_POINT0, EffectBasePoints0 });
                return;
            }

            break;
    }

    //spells triggered by dummy effect should not miss
    if(spell_id)
    {
        SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo( spell_id );

        if(!spellInfo)
        {
            TC_LOG_ERROR("FIXME","EffectDummy of spell %u: triggering unknown spell id %i\n", m_spellInfo->Id, spell_id);
            return;
        }

        auto spell = new Spell(m_caster, spellInfo, TRIGGERED_FULL_MASK, m_originalCasterGUID, nullptr, true);
        if(bp) 
            spell->SetSpellValue(SPELLVALUE_BASE_POINT0, bp);

        spell->SetForceHitResult(_forceHitResult); //sun: inherit force hit

        SpellCastTargets targets;
        targets.SetUnitTarget(unitTarget);
        spell->prepare(targets);
    }

    // pet auras
    if(PetAura const* petSpell = sSpellMgr->GetPetAura(m_spellInfo->Id))
    {
        if(_unitCaster)
            _unitCaster->AddPetAura(petSpell);
        return;
    }
    
    if (unitTarget && _unitCaster && unitTarget->ToCreature() && unitTarget->ToCreature()->IsAIEnabled())
        unitTarget->ToCreature()->AI()->sOnDummyEffect(_unitCaster, m_spellInfo->Id, i);
}

void Spell::EffectTriggerRitualOfSummoning(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    uint32 triggered_spell_id = m_spellInfo->Effects[i].TriggerSpell;
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo( triggered_spell_id );

    if (!spellInfo) {
        TC_LOG_ERROR("network.opcode","EffectTriggerRitualOfSummoning of spell %u: triggering unknown spell id %i", m_spellInfo->Id,triggered_spell_id);
        return;
    }

    m_caster->CastSpell(unitTarget, spellInfo->Id, TRIGGERED_CAST_DIRECTLY);
}

void Spell::EffectForceCast(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if( !unitTarget )
        return;

    uint32 triggered_spell_id = m_spellInfo->Effects[i].TriggerSpell;

    // normal case
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo( triggered_spell_id );

    if(!spellInfo)
    {
        TC_LOG_ERROR("FIXME","EffectForceCast of spell %u: triggering unknown spell id %i", m_spellInfo->Id,triggered_spell_id);
        return;
    }

    switch (m_spellInfo->Id)
    {
        case 45442: // KJ Soul Flay
        {
            if (!_unitCaster || !_unitCaster->GetVictim())
                return;

            _unitCaster->GetVictim()->CastSpell(m_caster, triggered_spell_id, m_originalCasterGUID);
            break;
        }
        case 45391:
        case 45388:
        {
            unitTarget->CastSpell(m_caster, triggered_spell_id, m_originalCasterGUID);
            break;
        }
        case 45782:
        {
            unitTarget->CastSpell((Unit*)nullptr, triggered_spell_id, m_originalCasterGUID);
            break;
        }
        default:
        {
            unitTarget->CastSpell(unitTarget, spellInfo->Id, m_originalCasterGUID);
            break;
        }
    }
}

void Spell::EffectTriggerSpell(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET
        && effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH)
        return;

    // Prevent triggering spells here if spell has a charge effect (handled in ChargeMovementGenerator)
    if ((m_spellInfo->Effects[0].Effect == SPELL_EFFECT_CHARGE || m_spellInfo->Effects[1].Effect == SPELL_EFFECT_CHARGE || m_spellInfo->Effects[2].Effect == SPELL_EFFECT_CHARGE))
        return;
    
    uint32 triggered_spell_id = m_spellInfo->Effects[effIndex].TriggerSpell;

    // special cases
    if (m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_TRIGGER_SPELL
        && effectHandleMode == SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        switch(triggered_spell_id)
        {
            // Enrage
            case 37975:
            {
                if(_unitCaster)
                    _unitCaster->CastSpell(unitTarget, triggered_spell_id, m_originalCasterGUID);
                return;
            }
            // Vanish
            case 18461:
            {
                if (!_unitCaster || !_unitCaster)
                    break;

                _unitCaster->RemoveAurasByType(SPELL_AURA_MOD_ROOT);
                _unitCaster->RemoveAurasByType(SPELL_AURA_MOD_DECREASE_SPEED);
                _unitCaster->RemoveAurasByType(SPELL_AURA_MOD_STALKED);

                // if this spell is given to NPC it must handle rest by it's own AI
                if ( m_caster->GetTypeId() != TYPEID_PLAYER )
                    return;

                // get highest rank of the Stealth spell
                bool found = false;
                SpellInfo const *spellInfo = nullptr;
                const PlayerSpellMap& sp_list = (m_caster->ToPlayer())->GetSpellMap();
                for (const auto & itr : sp_list)
                {
                    // only highest rank is shown in spell book, so simply check if shown in spell book
                    if(!itr.second->active || itr.second->disabled || itr.second->state == PLAYERSPELL_REMOVED)
                        continue;

                    spellInfo = sSpellMgr->GetSpellInfo(itr.first);
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

                //cast stealth
                if (spellInfo)
                {
                    // reset cooldown on it if needed

                    if (!_unitCaster->GetSpellHistory()->IsReady(spellInfo))
                        _unitCaster->GetSpellHistory()->ResetCooldown(m_spellInfo->Id, false);

                    _unitCaster->CastSpell(unitTarget, spellInfo->Id, true);
                }

                //also cast vanish dummy aura
                _unitCaster->CastSpell(m_caster, 18461, true);
                return;
            }
            // just skip
            case 23770:                                         // Sayge's Dark Fortune of *
                // not exist, common cooldown can be implemented in scripts if need.
                return;
            // Brittle Armor - (need add max stack of 24575 Brittle Armor)
            case 29284:
            {
                const SpellInfo *spell = sSpellMgr->GetSpellInfo(24575);
                if (!spell)
                    return;

                for (int idx = 0; idx < spell->StackAmount; ++idx)
                {
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.SetOriginalCaster(m_originalCasterGUID);
                    args.SetCastItem(m_CastItem);
                    m_caster->CastSpell(unitTarget, spell->Id, args);
                }
                return;
            }
            // Mercurial Shield - (need add max stack of 26464 Mercurial Shield)
            case 29286:
            {
                const SpellInfo *spell = sSpellMgr->GetSpellInfo(26464);
                if (!spell)
                    return;

                for (int idx = 0; idx < spell->StackAmount; ++idx)
                {
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.SetOriginalCaster(m_originalCasterGUID);
                    args.SetCastItem(m_CastItem);
                    m_caster->CastSpell(unitTarget, spell->Id, args);
                }
                return;
            }
            // Righteous Defense
            case 31980:
            {
                m_caster->CastSpell(unitTarget, 31790, m_originalCasterGUID);
                return;
            }
            // Cloak of Shadows
            case 35729 :
            {
                if (!_unitCaster)
                    break;

                uint32 dispelMask = GetDispellMask(DISPEL_ALL);
                auto& Auras = _unitCaster->GetAppliedAuras();
                for(auto iter = Auras.begin(); iter != Auras.end(); ++iter)
                {
                    // remove all harmful spells on you...
                    SpellInfo const* spell = iter->second->GetBase()->GetSpellInfo();
                    if((spell->DmgClass == SPELL_DAMAGE_CLASS_MAGIC // only affect magic spells
                        || ((1<<spell->Dispel) & dispelMask))
                        // ignore positive and passive auras
                        && !iter->second->IsPositive() && !iter->second->GetBase()->IsPassive())
                    {
                        _unitCaster->RemoveAurasDueToSpell(spell->Id);
                        iter = Auras.begin();
                    }
                }
                return;
            }
        }

    // normal case
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(triggered_spell_id);
    if(!spellInfo)
    {
        TC_LOG_ERROR("spells","EffectTriggerSpell of spell %u: triggering unknown spell id %i", m_spellInfo->Id, triggered_spell_id);
        return;
    }

    // some triggered spells require specific equipment
    if(spellInfo->EquippedItemClass >=0 && m_caster->GetTypeId()==TYPEID_PLAYER)
    {
        // main hand weapon required
        if(spellInfo->HasAttribute(SPELL_ATTR3_MAIN_HAND))
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
        if(spellInfo->HasAttribute(SPELL_ATTR3_REQ_OFFHAND))
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

    SpellCastTargets targets;
    if (effectHandleMode == SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
    {
        if (!spellInfo->NeedsToBeTriggeredByCaster(m_spellInfo))
            return;
        targets.SetUnitTarget(unitTarget);
    }
    else //if (effectHandleMode == SPELL_EFFECT_HANDLE_LAUNCH)
    {
        if (spellInfo->NeedsToBeTriggeredByCaster(m_spellInfo) && (m_spellInfo->Effects[effIndex].GetProvidedTargetMask() & TARGET_FLAG_UNIT_MASK))
            return;

        if (spellInfo->GetExplicitTargetMask() & TARGET_FLAG_DEST_LOCATION)
            targets.SetDst(m_targets);

        if (Unit* target = m_targets.GetUnitTarget())
            targets.SetUnitTarget(target);
        else
        {
            if (Unit* unit = m_caster->ToUnit())
                targets.SetUnitTarget(unit);
            else if (GameObject* go = m_caster->ToGameObject())
                targets.SetGOTarget(go);
        }
    }

    CastSpellExtraArgs args(m_originalCasterGUID);
    args.SetCastItem(m_CastItem);
    args.ForceHitResult = _forceHitResult; //sun: inherit force hit result
    // set basepoints for trigger with value effect
    if (m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE)
        for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            args.AddSpellMod(SpellValueMod(SPELLVALUE_BASE_POINT0 + i), damage);

    // original caster guid only for GO cast
    m_caster->CastSpell(targets, spellInfo->Id, args);
}

void Spell::EffectTriggerMissileSpell(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET
        && effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    uint32 triggered_spell_id = m_spellInfo->Effects[effIndex].TriggerSpell;

    // normal case
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo( triggered_spell_id );

    if(!spellInfo)
    {
        TC_LOG_ERROR("spells","EffectTriggerMissileSpell of spell %u (eff: %u): triggering unknown spell id %u",
            m_spellInfo->Id, effIndex, triggered_spell_id);
        return;
    }
    
    // some triggered spells require specific equipment
    if(spellInfo->EquippedItemClass >=0 && m_caster->GetTypeId()==TYPEID_PLAYER)
    {
        // main hand weapon required
        if(spellInfo->HasAttribute(SPELL_ATTR3_MAIN_HAND))
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
        if(spellInfo->HasAttribute(SPELL_ATTR3_REQ_OFFHAND))
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
        TC_LOG_DEBUG("spells","WORLD: cast Item spellId - %i", spellInfo->Id);

    SpellCastTargets targets;
    if (effectHandleMode == SPELL_EFFECT_HANDLE_HIT_TARGET)
    {
        if (!spellInfo->NeedsToBeTriggeredByCaster(m_spellInfo))
            return;
        targets.SetUnitTarget(unitTarget);
    }
    else //if (effectHandleMode == SPELL_EFFECT_HANDLE_HIT)
    {
        if (spellInfo->NeedsToBeTriggeredByCaster(m_spellInfo) && (m_spellInfo->Effects[effIndex].GetProvidedTargetMask() & TARGET_FLAG_UNIT_MASK))
            return;

        if (spellInfo->GetExplicitTargetMask() & TARGET_FLAG_DEST_LOCATION)
            targets.SetDst(m_targets);

        if (Unit* unit = m_caster->ToUnit())
            targets.SetUnitTarget(unit);
        else if (GameObject* go = m_caster->ToGameObject())
            targets.SetGOTarget(go);
    }

    CastSpellExtraArgs args(m_originalCasterGUID);
    args.SetTriggerFlags(TriggerCastFlags(TRIGGERED_IGNORE_GCD | TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD | TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_CAST_DIRECTLY));
    args.SetCastItem(m_CastItem);
    args.ForceHitResult = _forceHitResult; //sun: inherit force hit result

    m_caster->CastSpell(targets, spellInfo->Id, args);
}

void Spell::EffectTeleportUnits(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || unitTarget->IsInFlight())
        return;
        
    // HackCrashFix
    if (m_spellInfo->Id == 46020 && unitTarget->GetMapId() != 580)
        return;

    // If not exist data for dest location - return
    if(!m_targets.HasDst())
    {
        TC_LOG_ERROR( "spell","Spell::EffectTeleportUnits - does not have destination for spell ID %u\n", m_spellInfo->Id );
        return;
    }
    // Init dest coordinates
    WorldLocation targetDest(*destTarget);
    if (targetDest.GetMapId() == MAPID_INVALID)
        targetDest.m_mapId = unitTarget->GetMapId();

    if (!targetDest.GetOrientation() && m_targets.GetUnitTarget())
        targetDest.SetOrientation(m_targets.GetUnitTarget()->GetOrientation());
    // Teleport and Transform kj outro
    // Orientation exist in db but is not used?
    if (m_spellInfo->Id == 46473)
        targetDest.SetOrientation(6.22f);

    // Teleport
    if (targetDest.GetMapId() == unitTarget->GetMapId())
        unitTarget->NearTeleportTo(targetDest, unitTarget == m_caster);
    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        (unitTarget->ToPlayer())->TeleportTo(targetDest, (unitTarget == m_caster ? TELE_TO_SPELL : 0));
    else
    {
        TC_LOG_ERROR("spells", "Spell::EffectTeleportUnits - spellId %u attempted to teleport creature to a different map.", m_spellInfo->Id);
        return;
    }

    // post effects for TARGET_DEST_DB
    switch ( m_spellInfo->Id )
    {
        // Dimensional Ripper - Everlook
        case 23442:
        {
          int32 r = irand(0, 119);
            if ( r >= 70 )                                  // 7/12 success
            {
                if ( r < 100 )                              // 4/12 evil twin
                    m_caster->CastSpell(m_caster,23445, true);
                else                                        // 1/12 fire
                    m_caster->CastSpell(m_caster,23449, true);
            }
            return;
        }
        // Ultrasafe Transporter: Toshley's Station
        case 36941:
        {
            if ( roll_chance_i(50) )                        // 50% success
            {
              int32 rand_eff = urand(1,7);
                switch ( rand_eff )
                {
                    case 1:
                        // soul split - evil
                        m_caster->CastSpell(m_caster,36900, true);
                        break;
                    case 2:
                        // soul split - good
                        m_caster->CastSpell(m_caster,36901, true);
                        break;
                    case 3:
                        // Increase the size
                        m_caster->CastSpell(m_caster,36895, true);
                        break;
                    case 4:
                        // Decrease the size
                        m_caster->CastSpell(m_caster,36893, true);
                        break;
                    case 5:
                    // Transform
                    {
                        if ((m_caster->ToPlayer())->GetTeam() == ALLIANCE )
                            m_caster->CastSpell(m_caster,36897, true);
                        else
                            m_caster->CastSpell(m_caster,36899, true);
                        break;
                    }
                    case 6:
                        // chicken
                        m_caster->CastSpell(m_caster,36940, true);
                        break;
                    case 7:
                        // evil twin
                        m_caster->CastSpell(m_caster,23445, true);
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
              int32 rand_eff = urand(1,7);
                switch ( rand_eff )
                {
                    case 1:
                        // soul split - evil
                        m_caster->CastSpell(m_caster,36900, true);
                        break;
                    case 2:
                        // soul split - good
                        m_caster->CastSpell(m_caster,36901, true);
                        break;
                    case 3:
                        // Increase the size
                        m_caster->CastSpell(m_caster,36895, true);
                        break;
                    case 4:
                    // Transform
                    {
                        if ((m_caster->ToPlayer())->GetTeam() == ALLIANCE )
                            m_caster->CastSpell(m_caster,36897, true);
                        else
                            m_caster->CastSpell(m_caster,36899, true);
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

void Spell::EffectApplyAura(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    //HACKS (TODO: move those to scripts)
    {
        // Intervention shouldn't be used in a bg in preparation phase (possibility to get out of starting area with that spell)
        if (m_spellInfo->Id == 3411 && _unitCaster && _unitCaster->HasAura(44521))     // 44521 : bg preparation
            return;
        if (m_spellInfo->Id == 10803 || m_spellInfo->Id == 10804) //Summon Purple Tallstrider || Summon Turquoise Tallstrider
            unitTarget->RemoveAurasByType(SPELL_AURA_MOUNTED);

        if (unitTarget->GetCharmerOrOwnerPlayerOrPlayerItself())              // Negative buff should only be applied on players
        {
            uint32 spellId = 0;
            if (m_spellInfo->CasterAuraStateNot == AURA_STATE_WEAKENED_SOUL || m_spellInfo->TargetAuraStateNot == AURA_STATE_WEAKENED_SOUL)
                spellId = 6788;                                 // Weakened Soul
            else if (m_spellInfo->CasterAuraStateNot == AURA_STATE_FORBEARANCE || m_spellInfo->TargetAuraStateNot == AURA_STATE_FORBEARANCE)
                spellId = 25771;                                // Forbearance
            else if (m_spellInfo->CasterAuraStateNot == AURA_STATE_HYPOTHERMIA)
                spellId = 41425;                                // Hypothermia
            else if (m_spellInfo->Mechanic == MECHANIC_BANDAGE) // Bandages
                spellId = 11196;                                // Recently Bandaged
            else if ((m_spellInfo->HasAttribute(SPELL_ATTR1_NOT_BREAK_STEALTH)) && (m_spellInfo->HasAttribute(SPELL_ATTR2_NOT_RESET_AUTO_ACTIONS)))
                spellId = 23230;                                // Blood Fury - Healing Reduction

            if(spellId && _unitCaster)
                _unitCaster->AddAura(spellId, unitTarget);
        }

    }

    if (!_spellAura || !unitTarget)
        return;

    // register target/effect on aura
    AuraApplication* aurApp = _spellAura->GetApplicationOfTarget(unitTarget->GetGUID());
    if (!aurApp)
        aurApp = unitTarget->_CreateAuraApplication(_spellAura, 1 << effIndex);
    else
        aurApp->UpdateApplyEffectMask(aurApp->GetEffectsToApply() | 1 << effIndex);

    // Prayer of Mending (jump animation), we need formal caster instead original for correct animation
    if (m_spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && (m_spellInfo->SpellFamilyFlags & 0x00002000000000LL))
    {
        CastSpellExtraArgs args;
        args.TriggerFlags = TRIGGERED_FULL_MASK;
        args.SetTriggeringAura(_spellAura->GetEffect(EFFECT_0));
        args.SetOriginalCaster(m_originalCasterGUID);
        m_caster->CastSpell(unitTarget, 41637, args);
    }

    //remove stealth on hostile targets (still need to find the correct rule)
    if (_unitCaster && _unitCaster->IsHostileTo(unitTarget)
        && !(m_spellInfo->HasAttribute(SPELL_ATTR3_NO_INITIAL_AGGRO))
        && (m_caster->GetTypeId() == TYPEID_PLAYER || (!m_caster->GetOwner()) ) //not gameobjects spells like ice trap & earthbind and so on (these have owner) but still remove if wild trap
       )
        unitTarget->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
}

void Spell::EffectUnlearnSpecialization( uint32 i )
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = unitTarget->ToPlayer();
    uint32 spellToUnlearn = m_spellInfo->Effects[i].TriggerSpell;

    _player->RemoveSpell(spellToUnlearn);
}

void Spell::EffectPowerDrain(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(m_spellInfo->Effects[i].MiscValue < 0 || m_spellInfo->Effects[i].MiscValue >= MAX_POWERS)
        return;

    Powers powerType = Powers(m_spellInfo->Effects[i].MiscValue);

    if(!unitTarget)
        return;
    if(!unitTarget->IsAlive())
        return;
    if(unitTarget->GetPowerType() != powerType)
        return;
    if(damage < 0)
        return;

    //add spell damage bonus
    if (_unitCaster)
    {
        damage = _unitCaster->SpellDamageBonusDone(unitTarget, m_spellInfo, uint32(damage), SPELL_DIRECT_DAMAGE, i, {});
        damage = unitTarget->SpellDamageBonusTaken(_unitCaster, m_spellInfo, damage, SPELL_DIRECT_DAMAGE);
    }

    // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
    uint32 power = damage;
    if (powerType == POWER_MANA && unitTarget->GetTypeId() == TYPEID_PLAYER )
        power -= (unitTarget->ToPlayer())->GetSpellCritDamageReduction(power);

    int32 newDamage = -(unitTarget->ModifyPower(powerType, -int32(power)));
        
    if (_unitCaster && m_spellInfo->Id == 27526) {
        if (_unitCaster->GetTypeId() == TYPEID_PLAYER && _unitCaster->GetVictim())
            unitTarget = _unitCaster->GetVictim();
    }

    float gainMultiplier = 0.0f;
    // Don't restore from self drain
    if (_unitCaster && _unitCaster != unitTarget)
    {
        gainMultiplier = m_spellInfo->Effects[i].CalcValueMultiplier(_unitCaster, this);
        if (gainMultiplier == 0.0f)
            gainMultiplier = 1.0f;

        int32 gain = int32(newDamage * gainMultiplier);

        _unitCaster->EnergizeBySpell(_unitCaster, m_spellInfo, gain, powerType, false); //sun: don't send energize log, already handled in ExecuteLogEffectTakeTargetPower
    }
    ExecuteLogEffectTakeTargetPower(i, unitTarget, powerType, newDamage, gainMultiplier);
}

void Spell::EffectSendEvent(uint32 effIndex)
{
    // we do not handle a flag dropping or clicking on flag in battleground by sendevent system
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET
        && effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    //special cases TODO: Move them to instanceScript->ProcessEvent
    if (m_spellInfo->Id == 34140 && _unitCaster && _unitCaster->GetTypeId() == TYPEID_PLAYER && (m_caster->ToPlayer())->GetQuestStatus(10305) == QUEST_STATUS_INCOMPLETE)
        (m_caster->ToPlayer())->KilledMonsterCredit(19547, ObjectGuid::Empty);
    else if (m_spellInfo->Id == 30098 && m_caster->GetTypeId() == TYPEID_PLAYER && (m_caster->ToPlayer())->GetQuestStatus(9444) == QUEST_STATUS_INCOMPLETE)
        (m_caster->ToPlayer())->CompleteQuest(9444);
    else if (m_spellInfo->Id == 24325) {
        InstanceScript *pInstance = ((InstanceScript*)m_caster->GetInstanceScript());
        if (pInstance && (pInstance->GetData(29) == 1 || pInstance->GetData(29) == 3)) // Ghazranka has been down, don't spawn it another time
            return;
    }
    else if (m_spellInfo->Id == 32408 && (m_caster->ToPlayer()))
        (m_caster->ToPlayer())->KilledMonsterCredit(18395, ObjectGuid::Empty);
    else if (m_spellInfo->Id == 40328) {
        Creature *soulgrinder = m_caster->FindNearestCreature(23019, 10.0f, true);
        if (soulgrinder) {      // Fear all ghosts
            /*CellCoord pair(Trinity::ComputeCellCoord(m_caster->GetPositionX(), m_caster->GetPositionY()));
            Cell cell(pair);
            cell.SetNoCreate();
            std::list<Creature*> ogres;

            Trinity::AllCreaturesOfEntryInRange check(m_caster, 24039, 100.0f);
            Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(ogres, check);
            TypeContainerVisitor<Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange>, GridTypeMapContainer> visitor(searcher);

            cell.Visit(pair, visitor, *m_caster->GetMap());
            
            for (std::list<Creature*>::iterator itr = ogres.begin(); itr != ogres.end(); ++itr)
                (*itr)->CastSpell((*itr), 39914, true);*/
            m_caster->CastSpell(m_caster, 39914, TRIGGERED_NONE);
        }
        else                   // Summon Soulgrinder
            m_caster->SummonCreature(23019, 3535.181641, 5590.692871, 0.183175, 3.915725, TEMPSUMMON_DEAD_DESPAWN, 0);
    }
    else if (m_spellInfo->Id == 24706 && m_caster->ToPlayer())
        m_caster->ToPlayer()->KilledMonsterCredit(15415, ObjectGuid::Empty);
    else if (m_spellInfo->Id == 20737 && m_caster->GetTypeId() == TYPEID_PLAYER)
        m_caster->ToPlayer()->SummonCreature(12918, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
    else if (m_spellInfo->Id == 42338 && m_caster->GetTypeId() == TYPEID_PLAYER)
        m_caster->ToPlayer()->KilledMonsterCredit(23727, ObjectGuid::Empty);
    
    if (ZoneScript* zoneScript = m_caster->GetZoneScript())
        zoneScript->ProcessEvent(focusObject, m_spellInfo->Effects[effIndex].MiscValue);
    else if (InstanceScript* instanceScript = m_caster->GetInstanceScript())    // needed in case Player is the caster
        instanceScript->ProcessEvent(focusObject, m_spellInfo->Effects[effIndex].MiscValue);

    m_caster->GetMap()->ScriptsStart(sEventScripts, m_spellInfo->Effects[effIndex].MiscValue, m_caster, focusObject);
}

void Spell::EffectPowerBurn(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(m_spellInfo->Effects[i].MiscValue < 0 || m_spellInfo->Effects[i].MiscValue >= MAX_POWERS)
        return;

    Powers powertype = Powers(m_spellInfo->Effects[i].MiscValue);

    if(!unitTarget)
        return;
    if(!unitTarget->IsAlive())
        return;
    if(unitTarget->GetPowerType()!=powertype)
        return;
    if(damage < 0)
        return;

    int32 curPower = int32(unitTarget->GetPower(powertype));

    // resilience reduce mana draining effect at spell crit damage reduction (added in 2.4)
    uint32 power = damage;
    if (powertype == POWER_MANA && unitTarget->GetTypeId() == TYPEID_PLAYER)
        power -= (unitTarget->ToPlayer())->GetSpellCritDamageReduction(power);

    int32 new_damage = (curPower < power) ? curPower : power;

    unitTarget->ModifyPower(powertype,-new_damage);

    // NO - Not a typo - EffectPowerBurn uses effect value multiplier - not effect damage multiplier
    float multiplier = m_spellInfo->Effects[i].CalcValueMultiplier(_unitCaster, this);

    // add log data before multiplication (need power amount, not damage)
    ExecuteLogEffectTakeTargetPower(i, unitTarget, powertype, new_damage, 0.0f);

    new_damage = int32(new_damage*multiplier);
    
    m_damage += new_damage;
}

void Spell::EffectHeal(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    // Some level depends spells
    int multiplier = 0;
    int level_diff = 0;
    switch (m_spellInfo->Id)
    {
        // Holy Strength
        case 20007:
            if (_unitCaster)
            {
                level_diff = _unitCaster->GetLevel() - 60;
                multiplier = int(0.04*damage);
            }
            break;
        default:
            break;
    }

    if (level_diff > 0)
        damage -= multiplier * level_diff;

    if (!unitTarget || !unitTarget->IsAlive() || damage < 0)
        return;

    // Skip if m_originalCaster not available
    if (!_unitCaster)
        return;

    int32 addhealth = damage;

    // Sceau de lumiere proc
    if (m_spellInfo->Id == 20167)
    {
        float ap = _unitCaster->GetTotalAttackPowerValue(BASE_ATTACK);
        int32 holy = _unitCaster->SpellBaseHealingBonusDone(m_spellInfo->GetSchoolMask());

        if (holy < 0)
            holy = 0;

        addhealth += int32(ap * 0.15) + int32(holy * 15 / 100);
    }

    // Vessel of the Naaru (Vial of the Sunwell trinket)
    if (m_spellInfo->Id == 45064)
    {
        // Amount of heal - depends from stacked Holy Energy
        int damageAmount = 0;
        Unit::AuraEffectList const& mDummyAuras = _unitCaster->GetAuraEffectsByType(SPELL_AURA_DUMMY);
        for (auto mDummyAura : mDummyAuras)
            if (mDummyAura->GetId() == 45062)
                damageAmount += mDummyAura->GetAmount();
        if (damageAmount)
            _unitCaster->RemoveAurasDueToSpell(45062);

        addhealth += damageAmount;
    }
    // Swiftmend - consumes Regrowth or Rejuvenation
    else if (m_spellInfo->TargetAuraState == AURA_STATE_SWIFTMEND && unitTarget->HasAuraState(AURA_STATE_SWIFTMEND))
    {
        Unit::AuraEffectList const& RejorRegr = unitTarget->GetAuraEffectsByType(SPELL_AURA_PERIODIC_HEAL);
        // find most short by duration
        AuraEffect* targetAura = nullptr;
        for (auto i : RejorRegr)
        {
            if (i->GetSpellInfo()->SpellFamilyName == SPELLFAMILY_DRUID
                && (i->GetSpellInfo()->SpellFamilyFlags == 0x40 || i->GetSpellInfo()->SpellFamilyFlags == 0x10))
            {
                if (!targetAura || i->GetBase()->GetDuration() < targetAura->GetBase()->GetDuration())
                    targetAura = i;
            }
        }

        if (!targetAura)
        {
            TC_LOG_ERROR("spell", "Target(GUID: %s) has aurastate AURA_STATE_SWIFTMEND but no matching aura.", unitTarget->GetGUID().ToString().c_str());
            return;
        }

        int32 tickheal = targetAura->GetAmount();
        tickheal = unitTarget->SpellHealingBonusTaken(_unitCaster, targetAura->GetSpellInfo(), tickheal, DOT);

        //int32 tickheal = targetAura->GetSpellInfo()->Effects[idx].BasePoints + 1;
        //It is said that talent bonus should not be included
        //int32 tickheal = targetAura->GetAmount();
        int32 tickcount = 0;
        if (targetAura->GetSpellInfo()->SpellFamilyName == SPELLFAMILY_DRUID)
        {
            switch (targetAura->GetSpellInfo()->SpellFamilyFlags)//TODO: proper spellfamily for 3.3.5
            {
            case 0x10:  tickcount = 4;  break; // Rejuvenation
            case 0x40:  tickcount = 6;  break; // Regrowth
            }
        }
        addhealth += tickheal * tickcount;
        unitTarget->RemoveAura(targetAura->GetId(), targetAura->GetCasterGUID());
    }
    else
    {
        addhealth = _unitCaster->SpellHealingBonusDone(unitTarget, m_spellInfo, addhealth, HEAL, effIndex, {});
    }
    addhealth = unitTarget->SpellHealingBonusTaken(_unitCaster, m_spellInfo, addhealth, HEAL);

    m_healing += addhealth;
}

void Spell::EffectHealPct(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsAlive() || damage < 0)
        return;
                
    if (m_spellInfo->Id == 39703) //WR HACK
        unitTarget = _unitCaster;

    uint32 heal = unitTarget->CountPctFromMaxHealth(damage);
    if (_unitCaster)
    {
         heal = _unitCaster->SpellHealingBonusDone(unitTarget, m_spellInfo, heal, HEAL, effIndex, { });
         heal = unitTarget->SpellHealingBonusTaken(_unitCaster, m_spellInfo, heal, HEAL);
    }

    m_healing += heal;
}

void Spell::EffectHealMechanical(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsAlive() || damage < 0)
        return;

    uint32 heal = damage;
    if (_unitCaster)
    {
        //"Also, it seems to be affected by +healing as my lvl 48 shaman with +123 healing just used this for 811 heal to my mechanical dragonling."
        heal = _unitCaster->SpellHealingBonusDone(unitTarget, m_spellInfo, heal, HEAL, effIndex, {});
        heal = unitTarget->SpellHealingBonusTaken(_unitCaster, m_spellInfo, heal, HEAL);
    }

    m_healing += heal;
}

void Spell::EffectHealthLeech(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !unitTarget->IsAlive() || damage < 0)
        return;

    if (_unitCaster)
    {
        damage = _unitCaster->SpellDamageBonusDone(unitTarget, m_spellInfo, uint32(damage), SPELL_DIRECT_DAMAGE, effIndex);
        damage = unitTarget->SpellDamageBonusTaken(_unitCaster, m_spellInfo, uint32(damage), SPELL_DIRECT_DAMAGE);
    }

    float healMultiplier = m_spellInfo->Effects[effIndex].CalcValueMultiplier(_unitCaster, this);

    m_damage += damage;

    // get max possible damage, don't count overkill for heal
    uint32 healthGain = uint32(-unitTarget->GetHealthGain(-damage) * healMultiplier);
    if(_unitCaster && _unitCaster->IsAlive())
    {
        healthGain = _unitCaster->SpellHealingBonusDone(unitTarget, m_spellInfo, healthGain, HEAL, effIndex);
        healthGain = _unitCaster->SpellHealingBonusTaken(_unitCaster, m_spellInfo, healthGain, HEAL);

        HealInfo healInfo(_unitCaster, _unitCaster, healthGain, m_spellInfo, m_spellSchoolMask);
        _unitCaster->HealBySpell(healInfo);
    }
}

void Spell::DoCreateItem(uint32 i, uint32 itemtype)
{
    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = unitTarget->ToPlayer();

    uint32 newitemid = itemtype;
    ItemTemplate const *pProto = sObjectMgr->GetItemTemplate( newitemid );
    if(!pProto)
    {
        player->SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr );
        return;
    }

    uint32 num_to_add;

    // TODO: maybe all this can be replaced by using correct calculated `damage` value
    if(pProto->Class != ITEM_CLASS_CONSUMABLE || m_spellInfo->SpellFamilyName != SPELLFAMILY_MAGE)
        num_to_add = damage;
    else if (pProto->MaxCount == 1)
        num_to_add = 1;
    else if(player->GetLevel() >= m_spellInfo->SpellLevel)
        num_to_add = damage;
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
            player->SendEquipError(msg, nullptr, nullptr);
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
            player->SendEquipError( EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr );
            return;
        }

        // set the "Crafted by ..." property of the item
        if( pProto->Class != ITEM_CLASS_CONSUMABLE && pProto->Class != ITEM_CLASS_QUEST)
            pItem->SetUInt32Value(ITEM_FIELD_CREATOR,player->GetGUID().GetCounter());

        // send info to the client
        player->SendNewItem(pItem, num_to_add, true, true);

        // we succeeded in creating at least one item, so a levelup is possible
        player->UpdateCraftSkill(m_spellInfo->Id);
    }
}

void Spell::EffectCreateItem(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    DoCreateItem(effIndex, m_spellInfo->Effects[effIndex].ItemType);
    ExecuteLogEffectCreateItem(effIndex, m_spellInfo->Effects[effIndex].ItemType);
}

void Spell::EffectPersistentAA(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!_unitCaster)
        return;

    // only handle at last effect
    for (uint8 i = effIndex + 1; i < MAX_SPELL_EFFECTS; ++i)
        if (m_spellInfo->Effects[i].Effect == SPELL_EFFECT_PERSISTENT_AREA_AURA)
            return;

    ASSERT(!_dynObjAura);

    float radius = m_spellInfo->Effects[effIndex].CalcRadius(_unitCaster);

    // Caster not in world, might be spell triggered from aura removal
    if (!_unitCaster->IsInWorld())
        return;

    DynamicObject* dynObj = new DynamicObject(false);
    if (!dynObj->CreateDynamicObject(_unitCaster->GetMap()->GenerateLowGuid<HighGuid::DynamicObject>(), _unitCaster, m_spellInfo->Id, *destTarget, radius, DYNAMIC_OBJECT_AREA_SPELL))
    {
        delete dynObj;
        return;
    }

    AuraCreateInfo createInfo(m_spellInfo, MAX_EFFECT_MASK, dynObj);
    createInfo
        .SetCaster(_unitCaster)
        .SetBaseAmount(m_spellValue->EffectBasePoints)
        .SetForceHitResult(_forceHitResult);

    if (Aura* aura = Aura::TryCreate(createInfo))
    {
        _dynObjAura = aura->ToDynObjAura();
        _dynObjAura->_RegisterForTargets();
    }
    else
        return;

    ASSERT(_dynObjAura->GetDynobjOwner());
    _dynObjAura->_ApplyEffectForTargets(effIndex);
}

void Spell::EffectEnergize(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!_unitCaster || !unitTarget)
        return;

    if(!unitTarget->IsAlive())
        return;

    if(m_spellInfo->Effects[i].MiscValue < 0 || m_spellInfo->Effects[i].MiscValue >= MAX_POWERS)
        return;

    // Some level depends spells
    ///@todo: move this to scripts
    int multiplier = 0;
    int level_diff = 0;
    switch (m_spellInfo->Id)
    {
        // Restore Energy
        case 9512:
            level_diff = _unitCaster->GetLevel() - 40;
            multiplier = 2;
            break;
        // Blood Fury
        case 24571:
            level_diff = _unitCaster->GetLevel() - 60;
            multiplier = 10;
            break;
        // Burst of Energy
        case 24532:
            level_diff = _unitCaster->GetLevel() - 60;
            multiplier = 4;
            break;
        // Mana gems with item 30720
        case 5405:
        case 10052:
        case 10057:
        case 10058:
        case 27103:
            if (_unitCaster->HasAura(37447))
                _unitCaster->CastSpell(_unitCaster, 37445, TRIGGERED_FULL_MASK);
            break;
        //Elune's Touch (30% AP)
        case 33926:
            damage = _unitCaster->GetTotalAttackPowerValue(BASE_ATTACK, unitTarget) * 30 / 100;
            break;
        default:
            break;
    }

    if (level_diff > 0)
        damage -= multiplier * level_diff;

    if(damage < 0)
        return;

    Powers power = Powers(m_spellInfo->Effects[i].MiscValue);

    if(unitTarget->GetMaxPower(power) == 0)
        return;

    _unitCaster->EnergizeBySpell(unitTarget, m_spellInfo, damage, power);

    // Mad Alchemist's Potion
    if (m_spellInfo->Id == 45051)
    {
        // find elixirs on target
        uint32 elixir_mask = 0;
        auto& Auras = unitTarget->GetAppliedAuras();
        for(auto & aura : Auras)
        {
            uint32 spell_id = aura.second->GetBase()->GetId();
            if(uint32 mask = sSpellMgr->GetSpellElixirMask(spell_id))
                elixir_mask |= mask;
        }

        // get available elixir mask any not active type from battle/guardian (and flask if no any)
        elixir_mask = (elixir_mask & ELIXIR_FLASK_MASK) ^ ELIXIR_FLASK_MASK;

        // get all available elixirs by mask and spell level
        std::vector<uint32> elixirs;
        SpellElixirMap const& m_spellElixirs = sSpellMgr->GetSpellElixirMap();
        for(const auto & m_spellElixir : m_spellElixirs)
        {
            if (m_spellElixir.second & elixir_mask)
            {
                if (m_spellElixir.second & (ELIXIR_UNSTABLE_MASK | ELIXIR_SHATTRATH_MASK))
                    continue;

                SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(m_spellElixir.first);
                if (spellInfo && (spellInfo->SpellLevel < m_spellInfo->SpellLevel || spellInfo->SpellLevel > unitTarget->GetLevel()))
                    continue;

                elixirs.push_back(m_spellElixir.first);
            }
        }

        if (!elixirs.empty())
        {
            // cast random elixir on target
          uint32 rand_spell = urand(0,elixirs.size()-1);
          {
              CastSpellExtraArgs args;
              args.TriggerFlags = TRIGGERED_FULL_MASK;
              args.SetCastItem(m_CastItem);
              _unitCaster->CastSpell(unitTarget, elixirs[rand_spell]);
          }
        }
    }
}

void Spell::EffectEnergizePct(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!_unitCaster || !unitTarget || !unitTarget->IsAlive())
        return;

    if(m_spellInfo->Effects[i].MiscValue < 0 || m_spellInfo->Effects[i].MiscValue >= MAX_POWERS)
        return;

    Powers power = Powers(m_spellInfo->Effects[i].MiscValue);

#ifdef LICH_KING
    if (unitTarget->GetTypeId() == TYPEID_PLAYER && unitTarget->GetPowerType() != power && !m_spellInfo->HasAttribute(SPELL_ATTR7_CAN_RESTORE_SECONDARY_POWER))
        return;
#endif

    uint32 maxPower = unitTarget->GetMaxPower(power);
    if(!maxPower)
        return;

    uint32 const gain = CalculatePct(maxPower, damage);
    _unitCaster->EnergizeBySpell(unitTarget, m_spellInfo, gain, power);
}

void Spell::SendLoot(ObjectGuid guid, LootType loottype)
{
    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    if (gameObjTarget)
    {
        // Players shouldn't be able to loot gameobjects that are currently despawned
        if (!gameObjTarget->isSpawned() && !player->IsGameMaster())
        {
            TC_LOG_ERROR("entities.player.cheat", "Possible hacking attempt: Player %s [guid: %u] tried to loot a gameobject [entry: %u id: %u] which is on respawn timer without being in GM mode!",
                player->GetName().c_str(), player->GetGUID().GetCounter(), gameObjTarget->GetEntry(), gameObjTarget->GetGUID().GetCounter());
            return;
        }

        player->PlayerTalkClass->ClearMenus();
        if (gameObjTarget->AI()->GossipHello(player))
            return;

        switch (gameObjTarget->GetGoType())
        {
            case GAMEOBJECT_TYPE_DOOR:
            case GAMEOBJECT_TYPE_BUTTON:
                gameObjTarget->UseDoorOrButton(0, false, player);
                gameObjTarget->GetMap()->ScriptsStart(sGameObjectScripts, gameObjTarget->GetSpawnId(), player, gameObjTarget);
                return;

            case GAMEOBJECT_TYPE_QUESTGIVER:
                player->PrepareGossipMenu(gameObjTarget, gameObjTarget->GetGOInfo()->questgiver.gossipID, true);
                player->SendPreparedGossip(gameObjTarget);
                return;

            case GAMEOBJECT_TYPE_SPELL_FOCUS:
                // triggering linked GO
                if(uint32 trapEntry = gameObjTarget->GetGOInfo()->spellFocus.linkedTrapId)
                    gameObjTarget->TriggeringLinkedGameObject(trapEntry, player);
                return;

            case GAMEOBJECT_TYPE_GOOBER:
                // goober_scripts can be triggered if the player don't have the quest
                if (gameObjTarget->GetGOInfo()->goober.eventId)
                    gameObjTarget->GetMap()->ScriptsStart(sEventScripts, gameObjTarget->GetGOInfo()->goober.eventId, player, gameObjTarget);

                // cast goober spell
                if (gameObjTarget->GetGOInfo()->goober.questId)
                    ///Quest require to be active for GO using
                    if(player->GetQuestStatus(gameObjTarget->GetGOInfo()->goober.questId) != QUEST_STATUS_INCOMPLETE)
                        return;

                gameObjTarget->GetMap()->ScriptsStart(sGameObjectScripts, gameObjTarget->GetSpawnId(), player, gameObjTarget);

                gameObjTarget->AddUniqueUse(player);
                gameObjTarget->SetLootState(GO_JUST_DEACTIVATED);

                //TODO? Objective counting called without spell check but with quest objective check
                // if send spell id then this line will duplicate to spell casting call (double counting)
                // So we or have this line and not required in quest_template have reqSpellIdN
                // or must remove this line and required in DB have data in quest_template have reqSpellIdN for all quest using cases.
                player->CastedCreatureOrGO(gameObjTarget->GetEntry(), gameObjTarget->GetGUID(), 0);

                // triggering linked GO
                if(uint32 trapEntry = gameObjTarget->GetGOInfo()->goober.linkedTrapId)
                    gameObjTarget->TriggeringLinkedGameObject(trapEntry, player);

                return;

            case GAMEOBJECT_TYPE_CHEST:
                // TODO: possible must be moved to loot release (in different from linked triggering)
                if (gameObjTarget->GetGOInfo()->chest.eventId)
                    gameObjTarget->GetMap()->ScriptsStart(sEventScripts, gameObjTarget->GetGOInfo()->chest.eventId, player, gameObjTarget);

                // triggering linked GO
                if(uint32 trapEntry = gameObjTarget->GetGOInfo()->chest.linkedTrapId)
                    gameObjTarget->TriggeringLinkedGameObject(trapEntry, player);

                // Don't return, let loots been taken   
                break;
            default:
                break;
        }
    }

    // Send loot
    player->SendLoot(guid, loottype);
}

void Spell::EffectOpenLock(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER)
    {
        TC_LOG_ERROR("network.opcode", "WORLD: Open Lock - No Player Caster!");
        return;
    }

    Player* player = m_caster->ToPlayer();

    uint32 lockId = 0;
    ObjectGuid guid;
    
    //Hand of Iruxos HACK
    if (m_spellInfo->Id == 18762 && m_caster->GetTypeId() == TYPEID_PLAYER)
        m_caster->ToPlayer()->KilledMonsterCredit(11937, ObjectGuid::Empty);

    // Get lockId
    if(gameObjTarget)
    {
        GameObjectTemplate const* goInfo = gameObjTarget->GetGOInfo();
        // Arathi Basin banner opening !
        if( (goInfo->type == GAMEOBJECT_TYPE_BUTTON && goInfo->button.noDamageImmune) ||
            (goInfo->type == GAMEOBJECT_TYPE_GOOBER && goInfo->goober.losOK) )
        {
            //isAllowUseBattlegroundObject() already called in CanCast()
            // in battleground check
            if(Battleground *bg = player->GetBattleground())
            {
                // check if it's correct bg
                if(bg->GetTypeID() == BATTLEGROUND_AB || bg->GetTypeID() == BATTLEGROUND_AV)
                    bg->EventPlayerClickedOnFlag(player, gameObjTarget);
                return;
            }
        }
        else if (goInfo->type == GAMEOBJECT_TYPE_FLAGSTAND)
        {
            //isAllowUseBattlegroundObject() already called in CanCast()
            // in battleground check
            if(Battleground *bg = player->GetBattleground())
            {
                if(bg->GetTypeID() == BATTLEGROUND_EY)
                    bg->EventPlayerClickedOnFlag(player, gameObjTarget);
                return;
            }
        }
        // handle outdoor pvp object opening, return true if go was registered for handling
        // these objects must have been spawned by outdoorpvp!
        else if(gameObjTarget->GetGOInfo()->type == GAMEOBJECT_TYPE_GOOBER && sOutdoorPvPMgr->HandleOpenGo(player, gameObjTarget->GetGUID()))
            return;
        lockId = gameObjTarget->GetLockId();
        guid = gameObjTarget->GetGUID();
    }
    else if(itemTarget)
    {
        lockId = itemTarget->GetTemplate()->LockID;
        guid = itemTarget->GetGUID();
    }
    else
    {
        TC_LOG_ERROR("network.opcode", "WORLD: Open Lock - No GameObject/Item Target!");
        return;
    }

    LootType loottype = LOOT_CORPSE;

    if(!lockId)                                             // possible case for GO and maybe for items.
    {
        SendLoot(guid, loottype);
        return;
    }

    // Get LockInfo
    LockEntry const *lockInfo = sLockStore.LookupEntry(lockId);

    if (!lockInfo)
    {
        TC_LOG_ERROR("network", "Spell::EffectOpenLock: %s [guid = %u] has an unknown lockId: %u!",
            (gameObjTarget ? "gameobject" : "item"), guid.GetCounter(), lockId);
        SendCastResult(SPELL_FAILED_BAD_TARGETS);
        return;
    }

    // check key
    for(int i = 0; i < MAX_LOCK_CASE; ++i)
    {
        // type==1 This means lockInfo->key[i] is an item
        if(lockInfo->Type[i]==LOCK_KEY_ITEM && lockInfo->Index[i] && m_CastItem && m_CastItem->GetEntry()==lockInfo->Index[i])
        {
            if (gameObjTarget && gameObjTarget->GetEntry() == 184849) // Mechanar Cache of the Legion
                gameObjTarget->setManualUnlocked();

            SendLoot(guid, loottype);
            return;
        }
    }

    uint32 SkillId = 0;
    // Check and skill-up skill
    if( m_spellInfo->Effects[1].Effect == SPELL_EFFECT_SKILL )
        SkillId = m_spellInfo->Effects[1].MiscValue;
                                                            // pickpocketing spells
    else if( m_spellInfo->Effects[0].MiscValue == LOCKTYPE_PICKLOCK )
        SkillId = SKILL_LOCKPICKING;

    // skill bonus provided by casting spell (mostly item spells)
    uint32 spellSkillBonus = uint32(damage);

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

    if ( SkillId != SKILL_NONE )
    {
        loottype = LOOT_SKINNING;
        if ( player->GetSkillValue(SkillId) + spellSkillBonus < reqSkillValue )
        {
            SendCastResult(SPELL_FAILED_LOW_CASTLEVEL);
            return;
        }

        // update skill if really known
        uint32 pureSkillValue = player->GetPureSkillValue(SkillId);
        if(pureSkillValue)                                      // non only item base skill
        {
            if(gameObjTarget)
            {
                // Allow one skill-up until respawned
                if ( !gameObjTarget->IsInSkillupList( player->GetGUID().GetCounter() ) &&
                    player->UpdateGatherSkill(SkillId, pureSkillValue, reqSkillValue) )
                    gameObjTarget->AddToSkillupList( player->GetGUID().GetCounter() );
            }
            else if(itemTarget)
            {
                // Do one skill-up
                player->UpdateGatherSkill(SkillId, pureSkillValue, reqSkillValue);
            }
        }
    }

    SendLoot(guid, loottype);    

    ExecuteLogEffectOpenLock(effIndex, gameObjTarget ? (Object*)gameObjTarget : (Object*)itemTarget);
}

void Spell::EffectSummonChangeItem(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *player = m_caster->ToPlayer();

    // applied only to using item
    if(!m_CastItem)
        return;

    // ... only to item in own inventory/bank/equip_slot
    if(m_CastItem->GetOwnerGUID()!=player->GetGUID())
        return;

    uint32 newitemid = m_spellInfo->Effects[i].ItemType;
    if(!newitemid)
        return;

    uint16 pos = m_CastItem->GetPos();

    Item *pNewItem = Item::CreateItem( newitemid, 1, player);
    if( !pNewItem )
        return;

    for(uint8 idx = PERM_ENCHANTMENT_SLOT; idx<=TEMP_ENCHANTMENT_SLOT; ++idx)
    {
        if(m_CastItem->GetEnchantmentId(EnchantmentSlot(idx)))
            pNewItem->SetEnchantment(EnchantmentSlot(idx), m_CastItem->GetEnchantmentId(EnchantmentSlot(idx)), m_CastItem->GetEnchantmentDuration(EnchantmentSlot(idx)), m_CastItem->GetEnchantmentCharges(EnchantmentSlot(idx)));
    }

    if(m_CastItem->GetUInt32Value(ITEM_FIELD_DURABILITY) < m_CastItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY))
    {
        double loosePercent = 1 - m_CastItem->GetUInt32Value(ITEM_FIELD_DURABILITY) / double(m_CastItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY));
        player->DurabilityLoss(pNewItem, loosePercent);
    }

    if (player->IsInventoryPos(pos))
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanStoreItem( m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), dest, pNewItem, true );
        if( msg == EQUIP_ERR_OK )
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true);

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if(m_CastItem==m_targets.GetItemTarget())
                m_targets.SetItemTarget(nullptr);

            m_CastItem = nullptr;
            m_castItemGUID.Clear();
            m_castItemEntry = 0;

            player->StoreItem( dest, pNewItem, true);
            player->SendNewItem(pNewItem, 1, true, false);
            player->ItemAddedQuestCheck(newitemid, 1);
            return;
        }
    }
    else if (player->IsBankPos(pos))
    {
        ItemPosCountVec dest;
        uint8 msg = player->CanBankItem( m_CastItem->GetBagSlot(), m_CastItem->GetSlot(), dest, pNewItem, true );
        if( msg == EQUIP_ERR_OK )
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true);

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if(m_CastItem==m_targets.GetItemTarget())
                m_targets.SetItemTarget(nullptr);

            m_CastItem = nullptr;
            m_castItemGUID.Clear();
            m_castItemEntry = 0;

            player->BankItem( dest, pNewItem, true);
            return;
        }
    }
    else if (player->IsEquipmentPos(pos))
    {
        uint16 dest;
        uint8 msg = player->CanEquipItem( m_CastItem->GetSlot(), dest, pNewItem, true );
        if( msg == EQUIP_ERR_OK )
        {
            player->DestroyItem(m_CastItem->GetBagSlot(), m_CastItem->GetSlot(),true);

            // prevent crash at access and unexpected charges counting with item update queue corrupt
            if(m_CastItem==m_targets.GetItemTarget())
                m_targets.SetItemTarget(nullptr);

            m_CastItem = nullptr;
            m_castItemGUID.Clear();
            m_castItemEntry = 0;

            player->EquipItem(dest, pNewItem, true);
            player->AutoUnequipOffhandIfNeed();
            player->SendNewItem(pNewItem, 1, true, false);
            player->ItemAddedQuestCheck(newitemid, 1);
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
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *p_target = unitTarget->ToPlayer();

    uint32 subClassMask = m_spellInfo->EquippedItemSubClassMask;
    if(m_spellInfo->EquippedItemClass == ITEM_CLASS_WEAPON && !(p_target->GetWeaponProficiency() & subClassMask))
    {
        p_target->AddWeaponProficiency(subClassMask);
        if (p_target->IsInWorld()) //sun: else this is sent at character creation
            p_target->SendProficiency(uint8(ITEM_CLASS_WEAPON), p_target->GetWeaponProficiency());
    }

    if(m_spellInfo->EquippedItemClass == ITEM_CLASS_ARMOR && !(p_target->GetArmorProficiency() & subClassMask))
    {
        p_target->AddArmorProficiency(subClassMask);
        if (p_target->IsInWorld()) //sun: else this is sent at character creation
            p_target->SendProficiency(uint8(ITEM_CLASS_ARMOR), p_target->GetArmorProficiency());
    }
}

void Spell::EffectLearnSpell(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget)
        return;

    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            EffectLearnPetSpell(i);

        return;
    }

    Player *player = unitTarget->ToPlayer();

    uint32 spellToLearn = (m_spellInfo->Id==SPELL_ID_GENERIC_LEARN) ? damage : m_spellInfo->Effects[i].TriggerSpell;
    player->LearnSpell(spellToLearn, false);
    
 //   TC_LOG_DEBUG("spells", "Spell: Player %u has learned spell %u from NpcGUID: %u", player->GetGUID().GetCounter(), spellToLearn, m_caster->GetGUID().GetCounter());
}

void Spell::EffectDispel(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || !_unitCaster)
        return;

    if (!unitTarget->IsFriendlyTo(m_caster))
    {
        if (unitTarget->GetTypeId() == TYPEID_UNIT)
            unitTarget->GetThreatManager().AddThreat(_unitCaster, 10.0f);
    }

    // Create dispel mask by dispel type
    uint32 dispel_type = m_spellInfo->Effects[effIndex].MiscValue;
    uint32 dispelMask = SpellInfo::GetDispelMask(DispelType(dispel_type));

    DispelChargesList dispelList;
    unitTarget->GetDispellableAuraList(_unitCaster, dispelMask, dispelList, targetMissInfo == SPELL_MISS_REFLECT);
    if (dispelList.empty())
        return;

    size_t remaining = dispelList.size();

    // Ok if exist some buffs for dispel try dispel it
    uint32 failCount = 0;
    DispelChargesList successList;
    successList.reserve(damage);

    WorldPacket dataFail(SMSG_DISPEL_FAILED, 8 + 8 + 4 + 4 + damage * 4);
    // dispel N = damage buffs (or while exist buffs for dispel)
    for (int32 count = 0; count < damage && remaining > 0;)
    {
        // Random select buff for dispel (sunstrider: 100% confirmed on 2.4.3 arena videos)
        auto itr = dispelList.begin();
        std::advance(itr, urand(0, remaining - 1));

        if (itr->RollDispel())
        {
            auto successItr = std::find_if(successList.begin(), successList.end(), [&itr](DispelableAura& dispelAura) -> bool
            {
                if (dispelAura.GetAura()->GetId() == itr->GetAura()->GetId())
                    return true;

                return false;
            });

            if (successItr == successList.end())
                successList.emplace_back(itr->GetAura(), 0, uint8(1));
            else
                successItr->IncrementCharges();

            if (!itr->DecrementCharge())
            {
                --remaining;
                std::swap(*itr, dispelList[remaining]);
            }
        }
        else
        {
            if (!failCount)
            {
                // Failed to dispell
                dataFail << uint64(m_caster->GetGUID());            // Caster GUID
                dataFail << uint64(unitTarget->GetGUID());          // Victim GUID
                dataFail << uint32(m_spellInfo->Id);                // dispel spell id
            }
            ++failCount;
            dataFail << uint32(itr->GetAura()->GetId());            // Spell Id
        }
        ++count;
    }

    if (failCount)
        m_caster->SendMessageToSet(&dataFail, true);

    if (successList.empty())
        return;

    WorldPacket dataSuccess(SMSG_SPELLDISPELLOG, 8 + 8 + 4 + 1 + 4 + successList.size() * 5);
    // Send packet header
    dataSuccess << unitTarget->GetPackGUID();         // Victim GUID
    dataSuccess << m_caster->GetPackGUID();           // Caster GUID
    dataSuccess << uint32(m_spellInfo->Id);                // dispel spell id
    dataSuccess << uint8(0);                               // not used
    dataSuccess << uint32(successList.size());            // count
    for (DispelChargesList::iterator itr = successList.begin(); itr != successList.end(); ++itr)
    {
        // Send dispelled spell info
        dataSuccess << uint32(itr->GetAura()->GetId());         // Spell Id
        dataSuccess << uint8(0);                                // 0 - dispelled !=0 cleansed
        unitTarget->RemoveAurasDueToSpellByDispel(itr->GetAura()->GetId(), m_spellInfo->Id, itr->GetAura()->GetCasterGUID(), m_caster, itr->GetDispelCharges());
    }
    m_caster->SendMessageToSet(&dataSuccess, true);

    // On success dispel
    // Devour Magic
    if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && m_spellInfo->GetCategory() == SPELLCATEGORY_DEVOUR_MAGIC)
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
            TC_LOG_ERROR("spells", "Spell for Devour Magic %d not handled in Spell::EffectDispel", m_spellInfo->Id);
            break;
        }
        if (heal_spell)
            m_caster->CastSpell(m_caster, heal_spell, true);
#ifdef LICH_KING
        // Glyph of Felhunter
        if (Unit* owner = m_caster->GetOwner())
            if (owner->GetAura(56249))
                owner->CastSpell(owner, 19658, args);
#endif
    }
}

void Spell::EffectDualWield(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

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
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    // Check for possible target
    if(!unitTarget || unitTarget->IsInCombat())
        return;

    // target must be OK to do this
    if(unitTarget->HasUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_STUNNED | UNIT_STATE_FLEEING))
        return;

    //can be used on players as well
    unitTarget->GetMotionMaster()->MoveDistract(damage * IN_MILLISECONDS, unitTarget->GetAbsoluteAngle(destTarget));
}

void Spell::EffectPickPocket(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if( m_caster->GetTypeId() != TYPEID_PLAYER )
        return;

    // victim must be creature and attackable
    if(!_unitCaster || !unitTarget || unitTarget->GetTypeId() != TYPEID_UNIT || m_caster->IsFriendlyTo(unitTarget) )
        return;

    // victim have to be alive and humanoid or undead
    if (unitTarget->IsAlive() && (unitTarget->GetCreatureTypeMask() &CREATURE_TYPEMASK_HUMANOID_OR_UNDEAD) != 0)
    {
        // WoWWiki: "Failure rates depend on relative levels, but the base failure rate seems to be around 5%."
        int32 successChance = 95 + (int32(_unitCaster->GetLevel()) - int32(unitTarget->GetLevel())) * 2.0f; //rule out of my hat: 2% chance per lvl diff

        if(roll_chance_i(successChance))
        {
            // Stealing successful
            (m_caster->ToPlayer())->SendLoot(unitTarget->GetGUID(), LOOT_PICKPOCKETING);
        }
        else
        {
            // WoWWiki: "Sapping an enemy before pickpocketing them renders you immune to their aggro as long as you get out of their aggro radius once they resist. However, nearby allies will notice your stealth break and attack you."
            // Reveal action + get attack
            _unitCaster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TALK);
            if(!unitTarget->HasUnitState(UNIT_STATE_STUNNED) && !unitTarget->IsFriendlyTo(_unitCaster))
                unitTarget->EngageWithTarget(_unitCaster);
        }
    }
}

void Spell::EffectAddFarsight(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    // Caster not in world, might be spell triggered from aura removal
    if (!player->IsInWorld())
        return;

    float radius = m_spellInfo->Effects[effIndex].CalcRadius();
    int32 duration = m_spellInfo->GetDuration();

    player->StopCastingBindSight();
    player->RemoveAurasDueToSpell(6495); //sentry totem

    DynamicObject* dynObj = new DynamicObject(true);
    if (!dynObj->CreateDynamicObject(player->GetMap()->GenerateLowGuid<HighGuid::DynamicObject>(), player, m_spellInfo->Id, *destTarget, radius, DYNAMIC_OBJECT_FARSIGHT_FOCUS))
    {
        delete dynObj;
        return;
    }

    dynObj->SetUInt32Value(OBJECT_FIELD_TYPE, 65);
    dynObj->SetUInt32Value(DYNAMICOBJECT_BYTES, 0x80000002);

    dynObj->SetKeepActive(true);
    dynObj->SetDuration(duration);
    dynObj->SetCasterViewpoint();
}

void Spell::EffectTeleUnitsFaceCaster(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget)
        return;

    if(unitTarget->IsInFlight())
        return;

    uint32 mapid = m_caster->GetMapId();
    float dis = m_spellInfo->Effects[i].CalcRadius(m_originalCaster->GetSpellModOwner(), this);

    float fx,fy,fz;
    m_caster->GetClosePoint(fx,fy,fz,unitTarget->GetCombatReach(),dis);

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        (unitTarget->ToPlayer())->TeleportTo(mapid, fx, fy, fz, -m_caster->GetOrientation(), TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET | (unitTarget==m_caster ? TELE_TO_SPELL : 0));
    else
        unitTarget->GetMap()->CreatureRelocation(unitTarget->ToCreature(), fx, fy, fz, -m_caster->GetOrientation());
}

void Spell::EffectLearnSkill(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if(damage < 0)
        return;

    uint32 skillid = m_spellInfo->Effects[effIndex].MiscValue;
    SkillRaceClassInfoEntry const* rcEntry = GetSkillRaceClassInfo(skillid, unitTarget->GetRace(), unitTarget->GetClass());
    if (!rcEntry)
        return;

    SkillTiersEntry const* tier = sSkillTiersStore.LookupEntry(rcEntry->SkillTier);
    if (!tier)
        return;

    uint16 skillval = unitTarget->ToPlayer()->GetPureSkillValue(skillid);
    //TC unitTarget->ToPlayer()->SetSkill(skillid, m_spellInfo->Effects[effIndex].CalcValue(), std::max<uint16>(skillval, 1), tier->MaxSkill[damage - 1]);
    uint16 step = damage - 1;
    unitTarget->ToPlayer()->SetSkill(skillid, step, std::max<uint16>(skillval, 1), tier->MaxSkill[step]);
}

void Spell::EffectAddHonor(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    // 2.4.3 honor-spells don't scale with level and won't be casted by an item
    // also we must use damage+1 (spelldescription says +25 honor but damage is only 24)
    (unitTarget->ToPlayer())->RewardHonor(nullptr, 1, damage + 1);
}

void Spell::EffectTradeSkill(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    // uint32 skillid =  m_spellInfo->Effects[i].MiscValue;
    // uint16 skillmax = (unitTarget->ToPlayer())->(skillid);
    // (unitTarget->ToPlayer())->SetSkill(skillid,skillval?skillval:1,skillmax+75);
}

void Spell::EffectEnchantItemPerm(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if (!itemTarget)
        return;

    Player* p_caster = m_caster->ToPlayer();

    p_caster->UpdateCraftSkill(m_spellInfo->Id);

    if (m_spellInfo->Effects[i].MiscValue)
    {
        uint32 enchant_id = m_spellInfo->Effects[i].MiscValue;

        SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
        if(!pEnchant)
            return;

        // item can be in trade slot and have owner diff. from caster
        Player* item_owner = itemTarget->GetOwner();
        if(!item_owner)
            return;

        LogsDatabaseAccessor::Enchantment(p_caster, item_owner, itemTarget->GetGUID().GetCounter(), itemTarget->GetEntry(), enchant_id, true);

        // remove old enchanting before applying new if equipped
        item_owner->ApplyEnchantment(itemTarget,PERM_ENCHANTMENT_SLOT,false);

        itemTarget->SetEnchantment(PERM_ENCHANTMENT_SLOT, enchant_id, 0, 0);

        // add new enchanting if equipped
        item_owner->ApplyEnchantment(itemTarget,PERM_ENCHANTMENT_SLOT,true);

        // update trade window for show enchantment for caster in trade window     
        if (m_targets.GetTargetMask() & TARGET_FLAG_TRADE_ITEM)                       
            p_caster->GetSession()->SendUpdateTrade();
    }
}

void Spell::EffectEnchantItemTmp(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = m_caster->ToPlayer();

    if(!itemTarget)
        return;

    uint32 enchant_id = m_spellInfo->Effects[i].MiscValue;

    // Shaman Rockbiter Weapon
    if(i==0 && m_spellInfo->Effects[1].Effect==SPELL_EFFECT_DUMMY)
    {
        int32 enchnting_damage = CalculateDamage(EFFECT_1);//+1;

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
                TC_LOG_ERROR("FIXME","Spell::EffectEnchantItemTmp: Damage %u not handled in S'RW",enchnting_damage);
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
        TC_LOG_ERROR("FIXME","Spell %u Effect %u (SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) have 0 as enchanting id",m_spellInfo->Id,i);
        return;
    }

    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
    if(!pEnchant)
    {
        TC_LOG_ERROR("FIXME","Spell %u Effect %u (SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) have not existed enchanting id %u ",m_spellInfo->Id,i,enchant_id);
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
    else if (m_spellInfo->HasVisual(215))
        duration = 1800;                                    // 30 mins
    // some fishing pole bonuses
    else if (m_spellInfo->HasVisual(563))
        duration = 600;                                     // 10 mins
    // shaman rockbiter enchantments
    else if (m_spellInfo->HasVisual(0))
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

    LogsDatabaseAccessor::Enchantment(p_caster, item_owner, itemTarget->GetGUID().GetCounter(), itemTarget->GetEntry(), enchant_id, false);

    // remove old enchanting before applying new if equipped
    item_owner->ApplyEnchantment(itemTarget,TEMP_ENCHANTMENT_SLOT,false);

    itemTarget->SetEnchantment(TEMP_ENCHANTMENT_SLOT, enchant_id, duration*1000, 0);

    // add new enchanting if equipped
    item_owner->ApplyEnchantment(itemTarget,TEMP_ENCHANTMENT_SLOT,true);

    // update trade window for show enchantment for caster in trade window       
    if (m_targets.GetTargetMask() & TARGET_FLAG_TRADE_ITEM)               
        p_caster->GetSession()->SendUpdateTrade(); 
}

void Spell::EffectTameCreature(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!_unitCaster || _unitCaster->GetPetGUID())
        return;

    if(!unitTarget)
        return;

    if(unitTarget->GetTypeId() == TYPEID_PLAYER)
        return;

    Creature* creatureTarget = unitTarget->ToCreature();

    if(creatureTarget->IsPet())
        return;

    if(_unitCaster->GetClass() != CLASS_HUNTER)
        return;

    // cast finish successfully
    finish();

    Pet* pet = _unitCaster->CreateTamedPetFrom(creatureTarget, m_spellInfo->Id);
    if(!pet) 
        return;

    // kill original creature
    creatureTarget->DespawnOrUnsummon();

#ifdef LICH_KING
    uint8 level = (creatureTarget->GetLevel() < (_unitCaster->GetLevel() - 5)) ? (_unitCaster->GetLevel() - 5) : creatureTarget->GetLevel();
#else
    uint8 level = creatureTarget->GetLevel() - 1;
#endif

    // prepare visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, level);

    // add to world
    pet->GetMap()->AddToMap(pet->ToCreature(), true);

    // visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, creatureTarget->GetLevel());

    // caster have pet now
    _unitCaster->SetMinion(pet, true);

#ifdef LICH_KING
    pet->InitTalentForLevel();
#endif

    if(_unitCaster->GetTypeId() == TYPEID_PLAYER)
    {
        pet->SavePetToDB(PET_SAVE_AS_CURRENT);
        _unitCaster->ToPlayer()->PetSpellInitialize();
    }
}

void Spell::EffectSummonPet(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    Player* owner = nullptr;
    if(m_originalCaster)
    {
        if (m_originalCaster->GetTypeId() == TYPEID_PLAYER)
            owner = m_originalCaster->ToPlayer();
        else if ((m_originalCaster->ToCreature())->IsTotem())
            owner = m_originalCaster->GetCharmerOrOwnerPlayerOrPlayerItself();
    }

    uint32 petentry = m_spellInfo->Effects[effIndex].MiscValue;

    if(!owner)
    {
        SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(67);
        if (properties)
            SummonGuardian(effIndex, petentry, properties, 1);
        return;
    }

    Pet* OldSummon = owner->GetPet();

    // if pet requested type already exist
    if( OldSummon )
    {
        if(petentry == 0 || OldSummon->GetEntry() == petentry)
        {
            // pet in corpse state can't be summoned
            if(OldSummon->IsDead())
                return;

            if (OldSummon->GetMap() != owner->GetMap())
            {
                DEBUG_ASSERT(false);
                return;
            }

            float px, py, pz;
            owner->GetClosePoint(px, py, pz, OldSummon->GetCombatReach());

            OldSummon->NearTeleportTo(px, py, pz, OldSummon->GetOrientation());
            if (OldSummon->getPetType() == SUMMON_PET)
            {
                OldSummon->SetHealth(OldSummon->GetMaxHealth());
                OldSummon->SetPower(OldSummon->GetPowerType(), OldSummon->GetMaxPower(OldSummon->GetPowerType()));
            }

            if(owner->GetTypeId() == TYPEID_PLAYER && OldSummon->isControlled() )
                (owner->ToPlayer())->PetSpellInitialize();
            return;
        }

        if(owner->GetTypeId() == TYPEID_PLAYER)
            (owner->ToPlayer())->RemovePet(OldSummon,(OldSummon->getPetType()==HUNTER_PET ? PET_SAVE_AS_DELETED : PET_SAVE_NOT_IN_SLOT),false);
        else
            return;
    }

    float x, y, z;
    owner->GetClosePoint(x, y, z, owner->GetCombatReach());
    Pet* pet = owner->SummonPet(petentry, x, y, z, owner->GetOrientation(), SUMMON_PET, 0);
    if (!pet)
        return;

    if(m_caster->GetTypeId() == TYPEID_UNIT)
    {
        if ( (m_caster->ToCreature())->IsTotem() || owner->GetTypeId() == TYPEID_UNIT)
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
    pet->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, GameTime::GetGameTime());

    // generate new name for summon pet
    std::string new_name=sObjectMgr->GeneratePetName(petentry);
    if(!new_name.empty())
        pet->SetName(new_name);

    ExecuteLogEffectSummonObject(effIndex, pet);
}

void Spell::EffectLearnPetSpell(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = m_caster->ToPlayer();

    Pet *pet = _player->GetPet();
    if(!pet)
        return;
    if(!pet->IsAlive())
        return;

    SpellInfo const *learn_spellproto = sSpellMgr->GetSpellInfo(m_spellInfo->Effects[i].TriggerSpell);
    if(!learn_spellproto)
        return;

    pet->SetTP(pet->m_TrainingPoints - pet->GetTPForSpell(learn_spellproto->Id));
    pet->LearnSpell(learn_spellproto->Id);

    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    _player->PetSpellInitialize();
}

void Spell::EffectTaunt(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!_unitCaster)
        return;

    // this effect use before aura Taunt apply for prevent taunt already attacking target
    // for spell as marked "non effective at already attacking target"
    if(!unitTarget || !unitTarget->CanHaveThreatList())
    {
        SendCastResult(SPELL_FAILED_DONT_REPORT);
        return;
    }

    ThreatManager& mgr = unitTarget->GetThreatManager();
    if (mgr.GetCurrentVictim() == m_caster)
    {
        SendCastResult(SPELL_FAILED_DONT_REPORT);
        return;
    }

    if (!mgr.IsThreatListEmpty())
        // Set threat equal to highest threat currently on target
        mgr.MatchUnitThreatToHighestThreat(_unitCaster);
}

void Spell::EffectWeaponDmg(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    if (!_unitCaster)
        return;

    if (!unitTarget || !unitTarget->IsAlive())
        return;

    // multiple weapon dmg effect workaround
    // execute only the last weapon damage
    // and handle all effects at once
    for (uint8 j = 0; j < MAX_SPELL_EFFECTS; j++)
    {
        switch (m_spellInfo->Effects[j].Effect)
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
    float totalDamagePercentMod = 1.0f;                    // applied to final bonus+weapon damage
    int32 fixed_bonus = 0;
    int32 spell_bonus = 0;                                  // bonus specific for spell
    float meleeDamageModifier = 1.0f;

    switch (m_spellInfo->SpellFamilyName)
    {
    case SPELLFAMILY_WARRIOR:
    {
        // Devastate bonus and sunder armor refresh
        if (m_spellInfo->SpellVisual == 671 && m_spellInfo->SpellIconID == 1508)
        {
            uint32 stack = 0;

            Unit::AuraEffectList const& list = unitTarget->GetAuraEffectsByType(SPELL_AURA_MOD_RESISTANCE);
            for (auto itr : list)
            {
                SpellInfo const *proto = itr->GetSpellInfo();
                if (proto->SpellFamilyName == SPELLFAMILY_WARRIOR
                    && proto->SpellFamilyFlags == SPELLFAMILYFLAG_WARRIOR_SUNDERARMOR)
                {
                    int32 duration = proto->GetDuration();
                    itr->GetBase()->SetDuration(duration);
                    stack = itr->GetBase()->GetStackAmount();
                    break;
                }
            }

            for (uint8 j = 0; j < MAX_SPELL_EFFECTS; j++)
            {
                if (m_spellInfo->Effects[j].Effect == SPELL_EFFECT_NORMALIZED_WEAPON_DMG)
                {
                    fixed_bonus += (stack - 1) * CalculateDamage(j);
                    break;
                }
            }

            float threat = 14 * stack;
            m_targets.GetUnitTarget()->GetThreatManager().AddThreat(_unitCaster, threat, m_spellInfo);

            if (stack < 5)
            {
                // get highest rank of the Sunder Armor spell
                const PlayerSpellMap& sp_list = (m_caster->ToPlayer())->GetSpellMap();
                for (const auto & itr : sp_list)
                {
                    // only highest rank is shown in spell book, so simply check if shown in spell book
                    if (!itr.second->active || itr.second->disabled || itr.second->state == PLAYERSPELL_REMOVED)
                        continue;

                    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(itr.first);
                    if (!spellInfo)
                        continue;

                    if (spellInfo->SpellFamilyFlags == SPELLFAMILYFLAG_WARRIOR_SUNDERARMOR
                        && spellInfo->Id != m_spellInfo->Id
                        && spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR)
                    {
                        m_caster->CastSpell(unitTarget, spellInfo->Id, TRIGGERED_FULL_MASK);
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
        if (m_spellInfo->SpellFamilyFlags & 0x2000000)
        {
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
                (m_caster->ToPlayer())->AddComboPoints(unitTarget, 1);
        }
        // Mutilate (for each hand)
        else if (m_spellInfo->SpellFamilyFlags & 0x600000000LL)
        {
            //*2 damage for fixed bonus on offhand :
            if (m_attackType == OFF_ATTACK)
                meleeDamageModifier *= 2.0f;

            //150% damage if poisoned
            auto const& auras = unitTarget->GetAppliedAuras();
            for (const auto & aura : auras)
            {
                if (aura.second->GetBase()->GetSpellInfo()->Dispel == DISPEL_POISON)
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
        if (m_spellInfo->SpellFamilyFlags & 0x00000002000000LL)
        {
            spell_bonus += int32(0.20f * _unitCaster->SpellBaseDamageBonusDone(m_spellInfo->GetSchoolMask()));
            spell_bonus += int32(0.29f * unitTarget->SpellBaseDamageBonusTaken(m_spellInfo));
        }
        break;
    }
    case SPELLFAMILY_SHAMAN:
    {
        // Skyshatter Harness item set bonus
        // Stormstrike
        if (m_spellInfo->SpellFamilyFlags & 0x001000000000LL)
        {
            Unit::AuraEffectList const& m_OverrideClassScript = _unitCaster->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
            for (auto idx : m_OverrideClassScript)
            {
                // Stormstrike AP Buff
                if (idx->GetMiscValue() == 5634)
                {
                    CastSpellExtraArgs args;
                    args.SetCastItem(m_CastItem);
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.SetTriggeringAura(idx);
                    m_caster->CastSpell(m_caster, 38430, args);
                    break;
                }
            }
        }
        break;
    }
    case SPELLFAMILY_DRUID:
    {
        // Mangle (Cat): CP
        if (m_spellInfo->SpellFamilyFlags == 0x0000040000000000LL)
        {
            if (m_caster->GetTypeId() == TYPEID_PLAYER)
                (m_caster->ToPlayer())->AddComboPoints(unitTarget, 1);
        }
        break;
    }
    }

    bool normalized = false;
    float weaponDamagePercentMod = 1.0;
    for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
    {
        switch (m_spellInfo->Effects[j].Effect)
        {
        case SPELL_EFFECT_WEAPON_DAMAGE:
        case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            fixed_bonus += CalculateDamage(j);
            break;
        case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            fixed_bonus += CalculateDamage(j);
            normalized = true;
            break;
        case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
            weaponDamagePercentMod *= float(CalculateDamage(j)) / 100.0f;
            break;
        default:
            break;                                      // not weapon damage effect, just skip
        }
    }

    // if (addPctMods) { percent mods are added in Unit::CalculateDamage } else { percent mods are added in Unit::MeleeDamageBonusDone }
    // this distinction is neccessary to properly inform the client about his autoattack damage values from Script_UnitDamage
    //sun: changed m_spellSchoolMask -> info->GetSchoolMask() to include wands
    bool const addPctMods = !m_spellInfo->HasAttribute(SPELL_ATTR6_LIMIT_PCT_DAMAGE_MODS) && (m_spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_NORMAL);
    if (addPctMods)
    {
        UnitMods unitMod;
        switch (m_attackType)
        {
        default:
        case BASE_ATTACK:   unitMod = UNIT_MOD_DAMAGE_MAINHAND; break;
        case OFF_ATTACK:    unitMod = UNIT_MOD_DAMAGE_OFFHAND;  break;
        case RANGED_ATTACK: unitMod = UNIT_MOD_DAMAGE_RANGED;   break;
        }

        meleeDamageModifier *= _unitCaster->GetPctModifierValue(unitMod, TOTAL_PCT);

        if (fixed_bonus)
            fixed_bonus = int32(fixed_bonus * meleeDamageModifier);
        if (spell_bonus)
            spell_bonus = int32(spell_bonus * meleeDamageModifier);
    }

    int32 weaponDamage = _unitCaster->CalculateDamage(m_attackType, normalized, addPctMods);

    // Sequence is important
    for (const auto & Effect : m_spellInfo->Effects)
    {
        // We assume that a spell have at most one fixed_bonus
        // and at most one weaponDamagePercentMod
        switch (Effect.Effect)
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
    if (spell_bonus)
        weaponDamage += spell_bonus;

    // only for Mutilate
    if (totalDamagePercentMod != 1.0f)
        weaponDamage = int32(weaponDamage * totalDamagePercentMod);

    // apply spellmod to Done damage
    if (Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_DAMAGE, weaponDamage);

    // prevent negative damage
    weaponDamage = std::max(weaponDamage, 0);

    // Add melee damage bonuses (also check for negative)
    weaponDamage = _unitCaster->MeleeDamageBonusDone(unitTarget, weaponDamage, m_attackType, m_spellInfo);
    m_damage += unitTarget->MeleeDamageBonusTaken(_unitCaster, weaponDamage, m_attackType, m_spellInfo);
}

void Spell::EffectThreat(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!_unitCaster || !_unitCaster->IsAlive())
        return;

    if(!unitTarget)
        return;

    if(!unitTarget->CanHaveThreatList())
        return;

    unitTarget->GetThreatManager().AddThreat(_unitCaster, float(damage), m_spellInfo, true);
}

void Spell::EffectHealMaxHealth(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!_unitCaster)
        return;

    if(!unitTarget || !unitTarget->IsAlive())
        return;

    int32 addhealth = 0;
    // damage == 0 - heal for caster max health
    if (damage == 0)
        addhealth = _unitCaster->GetMaxHealth();
    else
        addhealth = unitTarget->GetMaxHealth() - unitTarget->GetHealth();

    m_healing += addhealth;
}

void Spell::EffectInterruptCast(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_LAUNCH_TARGET)
        return;

    if(!unitTarget)
        return;

    if(!unitTarget->IsAlive())
        return;

    // TODO: not all spells that used this effect apply cooldown at school spells
    // also exist case: apply cooldown to interrupted cast only and to all spells
    for (uint32 idx = CURRENT_FIRST_NON_MELEE_SPELL; idx < CURRENT_MAX_SPELL; idx++)
    {
        if (Spell* spell = unitTarget->GetCurrentSpell(idx))
        {
            SpellInfo const* curSpellInfo = spell->m_spellInfo;
            // check if we can interrupt spell
            if ( (unitTarget->m_currentSpells[idx]->getState() == SPELL_STATE_CASTING || (unitTarget->m_currentSpells[idx]->getState() == SPELL_STATE_PREPARING && unitTarget->m_currentSpells[idx]->GetCastTime() > 0.0f)) && unitTarget->m_currentSpells[idx]->m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT && unitTarget->m_currentSpells[idx]->m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE )
            {
                if(_unitCaster)
                {
                    int32 duration = m_spellInfo->GetDuration();
                    unitTarget->GetSpellHistory()->LockSpellSchool(curSpellInfo->GetSchoolMask(), unitTarget->ModSpellDuration(m_spellInfo, unitTarget, duration, false, 1 << effIndex));
                    Unit::ProcSkillsAndAuras(_unitCaster, unitTarget, PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG, PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG, PROC_SPELL_TYPE_MASK_ALL, PROC_SPELL_PHASE_HIT, PROC_HIT_INTERRUPT, nullptr, nullptr, nullptr);
                }
                ExecuteLogEffectInterruptCast(effIndex, unitTarget, curSpellInfo->Id);
                unitTarget->InterruptSpell(idx, false);
            }
        }
    }
}

void Spell::EffectSummonObjectWild(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    uint32 gameobject_id = m_spellInfo->Effects[i].MiscValue;

    auto  pGameObj = new GameObject;

    WorldObject* target = focusObject;
    if( !target )
        target = m_caster;

    float x,y,z;
    if(m_targets.HasDst())
    {
        destTarget->GetPosition(x, y, z);
    }
    else
        m_caster->GetClosePoint(x, y, z, DEFAULT_PLAYER_BOUNDING_RADIUS);

    Map *map = target->GetMap();

    if(!pGameObj->Create(map->GenerateLowGuid<HighGuid::GameObject>(), gameobject_id, map, target->GetPhaseMask(),
        Position(x, y, z, target->GetOrientation()), G3D::Quat(), 255, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    int32 duration = m_spellInfo->GetDuration();
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);

    ExecuteLogEffectSummonObject(i, pGameObj);

    if(pGameObj->GetGoType() != GAMEOBJECT_TYPE_FLAGDROP)   // make dropped flag clickable for other players (not set owner guid (created by) for this)...
    {
        if(m_originalCaster)
            m_originalCaster->AddGameObject(pGameObj);
        else if(_unitCaster)
            _unitCaster->AddGameObject(pGameObj);
    }
    map->AddToMap(pGameObj, true);

    if(pGameObj->GetMapId() == 489 && pGameObj->GetGoType() == GAMEOBJECT_TYPE_FLAGDROP)  //WS
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            Player *pl = m_caster->ToPlayer();
            Battleground* bg = (m_caster->ToPlayer())->GetBattleground();
            if(bg && bg->GetTypeID()==BATTLEGROUND_WS && bg->GetStatus() == STATUS_IN_PROGRESS)
            {
                 uint32 team = ALLIANCE;

                 if(pl->GetTeam() == team)
                     team = HORDE;

                ((BattlegroundWS*)bg)->SetDroppedFlagGUID(pGameObj->GetGUID(),team);
            }
        }
    }

    if(pGameObj->GetMapId() == 566 && pGameObj->GetGoType() == GAMEOBJECT_TYPE_FLAGDROP)  //EY
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            Battleground* bg = (m_caster->ToPlayer())->GetBattleground();
            if(bg && bg->GetTypeID()==BATTLEGROUND_EY && bg->GetStatus() == STATUS_IN_PROGRESS)
            {
                ((BattlegroundEY*)bg)->SetDroppedFlagGUID(pGameObj->GetGUID());
            }
        }
    }

    if(uint32 linkedEntry = pGameObj->GetLinkedGameObjectEntry())
    {
        auto  linkedGO = new GameObject;
        if(linkedGO->Create(map->GenerateLowGuid<HighGuid::GameObject>(), linkedEntry, map, pGameObj->GetPhaseMask(),
            Position(x, y, z, target->GetOrientation()), G3D::Quat(), 255, GO_STATE_READY))
        {
            linkedGO->SetRespawnTime(duration > 0 ? duration/1000 : 0);
            linkedGO->SetSpellId(m_spellInfo->Id);

            ExecuteLogEffectSummonObject(i, linkedGO);

            if (_unitCaster)
                _unitCaster->AddGameObject(linkedGO);
            map->AddToMap(linkedGO, true);
        }
        else
        {
            delete linkedGO;
            linkedGO = nullptr;
            return;
        }
    }
    
    if (m_spellInfo->Id == 19588)
        m_caster->SummonCreature(11560, m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 60000);
}

void Spell::EffectScriptEffect(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    // TODO: we must implement hunter pet summon at login there (spell 6962)

    // by spell id
    switch(m_spellInfo->Id)
    {
        // Power of the Blue Flight
        case 45833:
        {
            if (!unitTarget)
                return;

            CastSpellExtraArgs args;
            args.TriggerFlags = TRIGGERED_FULL_MASK;
            args.SetOriginalCaster(m_caster->GetGUID());
            unitTarget->CastSpell((Unit*)nullptr, 45836, args);
            return;
        }
        // Sinister Reflection
        case 45892:
        {
            Unit *target = nullptr;
            if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
            {
                if (m_caster->ToCreature())
                {
                    if (m_caster->ToCreature()->AI())
                    {
                        if (Unit *unit = m_caster->ToCreature()->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
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
                    target->CastSpell((Unit*)nullptr, 45891, m_caster->GetGUID());

                target->CastSpell((Unit*)nullptr, 45785, true);
            }
            return;
        }
        // Copy Weapon
        case 45785:
        {
            if (!unitTarget || unitTarget->GetEntry() != 25708)
                return;

            Player* pCaster = m_caster->ToPlayer();
            if(!pCaster)
                return;

            Item* mainItem = nullptr;
            Item* offItem = nullptr;
            Item* rangedItem = nullptr;
            if ((mainItem = pCaster->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND)))
                unitTarget->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 0, mainItem->GetTemplate()->DisplayInfoID);

            if ((offItem = pCaster->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND)))
                unitTarget->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 1, offItem->GetTemplate()->DisplayInfoID);

            if ((rangedItem = pCaster->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED)))
                unitTarget->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + 2, rangedItem->GetTemplate()->DisplayInfoID);

            break;
        }
        // Chilling burst
        case 46541:
            unitTarget->AddAura(46458, unitTarget);
            break;
        // Plant Kil'sorrow Banner (quest 9931)
        case 32314:
            if (Player *plr = m_caster->ToPlayer())
                plr->KilledMonsterCredit(18393, ObjectGuid::Empty);
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
            if( unitTarget->HasAura(26272)
             || unitTarget->HasAura(26157)
             || unitTarget->HasAura(26273)
             || unitTarget->HasAura(26274))
                return;

            uint32 iTmpSpellId=0;

            switch(urand(0,3))
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
            switch(urand(1,5))
            {
                case 1:  spell_id = 8854; break;
                default: spell_id = 8855; break;
            }

            m_caster->CastSpell(m_caster,spell_id, true);
            return;
        }
        /* Disabled: This is currently hardcoded in Nazan script
        case 34653:                                 // Nazan Fireball
        case 36920:                                 // Nazan Fireball (h)
        {
            if (!unitTarget)
                return;

            unitTarget->CastSpell(unitTarget, unitTarget->GetMap()->IsRegularDifficulty() ? 23971 : 30928, TRIGGERED_FULL_MASK, NULL, NULL, m_caster->GetObjectGuid());
            return;
        }
        */
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
            Unit::AuraEffectList const& mDummyAuras = unitTarget->GetAuraEffectsByType(SPELL_AURA_DUMMY);
            for(auto mDummyAura : mDummyAuras)
            {
                if(mDummyAura->GetId() == 18692)
                {
                    rank = 1;
                    break;
                }
                else if(mDummyAura->GetId() == 18693)
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
            unitTarget->RemoveSingleAuraFromStack(24575);
            return;
        // Mercurial Shield - need remove one 26464 Mercurial Shield aura
        case 26465:
            unitTarget->RemoveSingleAuraFromStack(26464);
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

            unitTarget->CastSpell(unitTarget,spellid, TRIGGERED_NONE);
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
            if(!unitTarget || !unitTarget->IsAlive())
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
            if(!unitTarget || !unitTarget->IsAlive())
                return;

            uint32 spellId = 0;
            switch((uint32)rand32()%7)
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
            if(!unitTarget || !unitTarget->IsAlive())
                return;

            if(unitTarget->GetGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24708, true);
            else
                m_caster->CastSpell(unitTarget,24709, true);
        }
        break;
        // Ninja Costume
        case 24718:
        {
            if(!unitTarget || !unitTarget->IsAlive())
                return;

            if(unitTarget->GetGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24711, true);
            else
                m_caster->CastSpell(unitTarget,24710, true);
        }
        break;
        // Leper Gnome Costume
        case 24719:
        {
            if(!unitTarget || !unitTarget->IsAlive())
                return;

            if(unitTarget->GetGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24712, true);
            else
                m_caster->CastSpell(unitTarget,24713, true);
        }
        break;
        // Ghost Costume
        case 24737:
        {
            if(!unitTarget || !unitTarget->IsAlive())
                return;

            if(unitTarget->GetGender() == GENDER_MALE)
                m_caster->CastSpell(unitTarget,24735, true);
            else
                m_caster->CastSpell(unitTarget,24736, true);
        }
        break;
        // Summon Black Qiraji Battle Tank
        case 26656:
        {
            if(!unitTarget)
                return;
                
            if(unitTarget->IsInCombat())
            {
                SendCastResult(SPELL_FAILED_TARGET_IN_COMBAT);
                return;
            }

            // Prevent stacking of mounts
            unitTarget->RemoveAurasByType(SPELL_AURA_MOUNTED);

            // Two separate mounts depending on area id (allows use both in and out of specific instance)
            if (unitTarget->GetAreaId() == 3428)
                unitTarget->CastSpell(unitTarget, 25863, TRIGGERED_NONE);
            else
                unitTarget->CastSpell(unitTarget, 26655, TRIGGERED_NONE);
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
                if(unitTarget->HasAura(spellid+i))
                    return;
            unitTarget->CastSpell(unitTarget, spellid+urand(0, 4), true);
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
            switch ( urand(1,6) )
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
            unitTarget->RemoveMovementImpairingAuras(true);
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
            p_caster->KilledMonsterCredit(18388, cTarget->GetGUID());
            cTarget->SetDeathState(CORPSE);
            cTarget->RemoveCorpse();
            break;
        }

        // Goblin Weather Machine
        case 46203:
        {
            if(!unitTarget)
                return;

            uint32 spellId = 0;
            switch((uint32)rand32()%4)
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
                        if (GetVirtualMapForMapAndZone(m_caster->GetMapId(),m_caster->GetAreaId()) == 530)
                            unitTarget->CastSpell(unitTarget, 51617, true);
                        else
                            unitTarget->CastSpell(unitTarget, 48024, true);
                        break;
                    case 300: 
                        if (GetVirtualMapForMapAndZone(m_caster->GetMapId(),m_caster->GetAreaId()) == 530)
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
            unitTarget->CastSpell(m_caster, m_spellInfo->Effects[0].BasePoints, true);
            return;
        }
        case 45717:
        {
            m_caster->CastSpell(unitTarget, 45726, true);
            return;
        }
        // Burn 
        case 45151:
        {
            // Already Burning
            if (unitTarget->HasAura(46394))
                return;

            if (unitTarget->IsPet())
                return;

            if (m_originalCaster)
                unitTarget->CastSpell(unitTarget, 46394, m_originalCasterGUID);
            return;
        }
        // Quest 11026 Kill Credit
        case 40828:
        {
            if (Unit *summoner = ((TempSummon*)m_caster)->GetSummoner()) {
                if (summoner->ToPlayer())
                    summoner->ToPlayer()->KilledMonsterCredit(23327, m_caster->GetGUID());
            }
            return;
        }
        case 39649:
        {
            for (uint8 i = 0; i < 8; i++)
                m_caster->CastSpell(unitTarget, 41159, TRIGGERED_FULL_MASK);
                
            return;
        }
        case 41072:
        {
            m_caster->CastSpell(unitTarget, 41065, TRIGGERED_FULL_MASK);
            return;
        }
        case 32580:
        {
            if(_unitCaster)
                if (Creature* bunny = m_caster->FindNearestCreature(21352, 20.0f, true)) {
                    switch (m_caster->GetAreaId()) {
                    case 3776:
                    {
                        if (Creature* spirit = bunny->SummonCreature(21452, bunny->GetPositionX(), bunny->GetPositionY(), bunny->GetPositionZ(), bunny->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                            spirit->AI()->AttackStart(_unitCaster);
                        break;
                    }
                    case 3773:
                    {
                        if (Creature* spirit = bunny->SummonCreature(21446, bunny->GetPositionX(), bunny->GetPositionY(), bunny->GetPositionZ(), bunny->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
                            spirit->AI()->AttackStart(_unitCaster);
                        break;
                    }
                    }
                }
            
            return;
        }
        case 43723:
        {
            m_caster->CastSpell(m_caster, 43753, TRIGGERED_NONE);
            return;
        }
        case 26541:
        case 26532:
        case 26528:
        case 26469:
        {
            if (!m_caster->ToPlayer() || !_unitCaster)
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
                
            ObjectGuid old_critter_guid = _unitCaster->GetCritterGUID();
            Pet* old_critter = _unitCaster->GetMap()->GetPet(old_critter_guid);
            // for same pet just despawn
            if (old_critter && old_critter->GetEntry() == petEntry) {
                old_critter->ToTempSummon()->UnSummon();
                return;
            }

            // check reagent
            if (!_unitCaster->ToPlayer()->HasItemCount(17202, 1, false))
                return;

            // despawn old pet before summon new
            if(old_critter)
                old_critter->ToTempSummon()->UnSummon();
            
            _unitCaster->ToPlayer()->CastSpell(_unitCaster, triggeredSpellId);

            return;
        }
        case 26393:
        {
            if (unitTarget)
                unitTarget->CastSpell(unitTarget, 26394, TRIGGERED_FULL_MASK);
                
            return;
        }
        case 349:
        {
            if (m_caster->ToPlayer()->GetBattleground())
            {
                m_caster->ToPlayer()->GetSession()->SendAreaTriggerMessage("Vous avez ete exclu du champ de bataille pour inactivite.");
                m_caster->ToPlayer()->LeaveBattleground();
            }
            return;
        }
        case 41467: //Illidari council : Gathios Judgement
        {
            if (unitTarget && _unitCaster)
            {
                if(_unitCaster->HasAura(41469)) //SPELL_SEAL_OF_COMMAND
                {
                    _unitCaster->RemoveAurasDueToSpell(41469);
                    _unitCaster->CastSpell(unitTarget, 41470); //SPELL_JUDGEMENT_OF_COMMAND
                }
                else if (_unitCaster->HasAura(41459)) //SPELL_SEAL_OF_BLOOD
                {
                    _unitCaster->RemoveAurasDueToSpell(41459);
                    _unitCaster->CastSpell(unitTarget, 41461); //SPELL_JUDGEMENT_OF_BLOOD
                }
            }
            return;
        }
    }

    if(!unitTarget || !unitTarget->IsAlive()) // can we remove this check?
    {
        TC_LOG_ERROR("FIXME","Spell %u in EffectScriptEffect does not have unitTarget", m_spellInfo->Id);
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
            if (unitTarget->HasAura(46394))
                return;

            if (unitTarget->IsPet())
                return;

            unitTarget->CastSpell(unitTarget, 46394, true); break;
        }
        case 41931:
        {
            int bag=19;
            int slot=0;
            Item* item = nullptr;

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
                m_caster->CastSpell(m_caster,42518, true);
                return;
            }
        }
        // Force Cast - Portal Effect: Sunwell Isle
        case 44876: 
            unitTarget->CastSpell(unitTarget, 44870, true); 
            break;
        // spell of Brutallus - Stomp
        case 45185:
        {
            if(unitTarget->HasAura(46394)) // spell of Brutallus - Burn
                unitTarget->RemoveAurasDueToSpell(46394);
            break;
        }
        // Negative Energy
        case 46289: 
            m_caster->CastSpell(unitTarget, 46285, true); 
            break;
        //5,000 Gold
        case 46642:
        {
            if(unitTarget && unitTarget->GetTypeId() == TYPEID_PLAYER)
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
                if (!_unitCaster)
                    break;
                uint32 spellId2 = 0;

                // all seals have aura dummy
                Unit::AuraEffectList const& m_dummyAuras = _unitCaster->GetAuraEffectsByType(SPELL_AURA_DUMMY);
                for(auto m_dummyAura : m_dummyAuras)
                {
                    SpellInfo const *spellInfo = m_dummyAura->GetSpellInfo();

                    // search seal (all seals have judgement's aura dummy spell id in 2 effect
                    if ( !spellInfo || m_dummyAura->GetSpellInfo()->GetSpellSpecific() != SPELL_SPECIFIC_SEAL || m_dummyAura->GetEffIndex() != 2 )
                        continue;

                    // must be calculated base at raw base points in spell proto, GetModifier()->m_value for S.Righteousness modified by SPELLMOD_DAMAGE
                    spellId2 = m_dummyAura->GetSpellInfo()->Effects[2].BasePoints+1;

                    if(spellId2 <= 1)
                        continue;

                    // found, remove seal
                    _unitCaster->RemoveAurasDueToSpell(m_dummyAura->GetId());

                    // Sanctified Judgement
                    Unit::AuraEffectList const& m_auras = _unitCaster->GetAuraEffectsByType(SPELL_AURA_DUMMY);
                    for(auto m_aura : m_auras)
                    {
                        if (m_aura->GetSpellInfo()->SpellIconID == 205 && m_aura->GetSpellInfo()->Attributes == 0x01D0LL)
                        {
                            int32 chance = m_aura->GetAmount();
                            if ( roll_chance_i(chance) )
                            {
                                int32 mana = spellInfo->ManaCost;
                                if (!mana)
                                    mana = spellInfo->ManaCostPercentage * _unitCaster->GetCreateMana() / 100;
                                if ( Player* modOwner = m_caster->GetSpellModOwner() )
                                    modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_COST, mana);
                                mana = int32(mana* 0.8f);
                                CastSpellExtraArgs args;
                                args.TriggerFlags = TRIGGERED_FULL_MASK;
                                args.AddSpellBP0(int32(mana));
                                args.SetTriggeringAura(m_aura);
                                m_caster->CastSpell(m_caster, 31930, args);
                            }
                            break;
                        }
                    }

                    break;
                }

                m_caster->CastSpell(unitTarget,spellId2, true);
                return;
            }
        }
    }

    // normal DB scripted effect
    m_caster->GetMap()->ScriptsStart(sSpellScripts, m_spellInfo->Id, m_caster, unitTarget);
}

void Spell::EffectSanctuary(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget)
        return;

    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
        unitTarget->ToPlayer()->SendAttackSwingCancelAttack();     // melee and ranged forced attack cancel

    if (unitTarget->GetTypeId() == TYPEID_PLAYER && !unitTarget->GetMap()->IsDungeon())
    {
        // stop all pve combat for players outside dungeons, suppress pvp combat
        unitTarget->CombatStop(false, false);
    }
    else
    {
        // in dungeons (or for nonplayers), reset this unit on all enemies' threat lists
        for (auto const& pair : unitTarget->GetThreatManager().GetThreatenedByMeList())
            pair.second->ScaleThreat(0.0f);
    }

    // windrunner: Vanish allows to remove all threat and cast regular stealth so other spells can be used
    if(_unitCaster && m_spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE && (m_spellInfo->SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE_VANISH))
        _unitCaster->RemoveAurasByType(SPELL_AURA_MOD_ROOT);

    // makes spells cast before this time fizzle
    unitTarget->m_lastSanctuaryTime = GameTime::GetGameTimeMS();
}

void Spell::EffectAddComboPoints(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    Player const* player = m_caster->ToPlayer();
    if (!player)
        return;

    if (!player->m_playerMovingMe)
        return;

    if (damage <= 0)
        return;
        
    //HACK
    if (m_spellInfo->Id == 15250) 
    {
        player->m_playerMovingMe->AddComboPoints(unitTarget, damage, true);
        return;
    }

    player->m_playerMovingMe->AddComboPoints(unitTarget, damage, false);
}

void Spell::EffectDuel(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!m_caster || !unitTarget || m_caster->GetTypeId() != TYPEID_PLAYER || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *caster = m_caster->ToPlayer();
    Player *target = unitTarget->ToPlayer();

    // caster or target already have requested duel
    if( caster->duel || target->duel || !target->GetSocial() || target->GetSocial()->HasIgnore(caster->GetGUID().GetCounter()) )
        return;

    // Players can only fight a duel with each other outside (=not inside dungeons and not in capital cities)
    // Don't have to check the target's map since you cannot challenge someone across maps
    if(caster->GetMap()->Instanceable())
    {
        SendCastResult(SPELL_FAILED_NO_DUELING);            // Dueling isn't allowed here
        return;
    }

    AreaTableEntry const* casterAreaEntry = sAreaTableStore.LookupEntry(caster->GetAreaId());
    if(casterAreaEntry && (casterAreaEntry->flags & AREA_FLAG_CAPITAL) )
    {
        SendCastResult(SPELL_FAILED_NO_DUELING);            // Dueling isn't allowed here
        return;
    }

    AreaTableEntry const* targetAreaEntry = sAreaTableStore.LookupEntry(target->GetAreaId());
    if(targetAreaEntry && (targetAreaEntry->flags & AREA_FLAG_CAPITAL) )
    {
        SendCastResult(SPELL_FAILED_NO_DUELING);            // Dueling isn't allowed here
        return;
    }

    //ENSURE TARGET CAN SEE CASTER (else he won't have any duel demands on client (hackyyyy))
    if(!target->HaveAtClient(caster))
    {
         caster->SendUpdateToPlayer(target); 
         target->m_clientGUIDs.insert(caster->GetGUID());
         target->SendInitialVisiblePackets((Unit*)caster);
    }

    //CREATE DUEL FLAG OBJECT
    auto  pGameObj = new GameObject;

    uint32 gameobject_id = m_spellInfo->Effects[i].MiscValue;

    Map *map = caster->GetMap();
    if(!pGameObj->Create(map->GenerateLowGuid<HighGuid::GameObject>(), gameobject_id, map, caster->GetPhaseMask(),
        Position(caster->GetPositionX()+(unitTarget->GetPositionX()- caster->GetPositionX())/2 ,
            caster->GetPositionY()+(unitTarget->GetPositionY()- caster->GetPositionY())/2 ,
            caster->GetPositionZ(),
            caster->GetOrientation()), G3D::Quat(), 0, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    pGameObj->SetUInt32Value(GAMEOBJECT_FACTION, caster->GetFaction() );
    pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, caster->GetLevel()+1 );
    int32 duration = m_spellInfo->GetDuration();
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);

    ExecuteLogEffectSummonObject(i, pGameObj);

    caster->AddGameObject(pGameObj);
    map->AddToMap(pGameObj, true);
    //END

    // Send request
    WorldPacket data(SMSG_DUEL_REQUESTED, 16);
    data << pGameObj->GetGUID();
    data << caster->GetGUID();
    caster->SendDirectMessage(&data);
    target->SendDirectMessage(&data);

    // create duel-info
#ifdef LICH_KING
    bool isMounted = (GetSpellInfo()->Id == 62875);
    caster->duel = std::make_unique<DuelInfo>(target, caster, isMounted);
    target->duel = std::make_unique<DuelInfo>(caster, caster, isMounted);
#else
    caster->duel = std::make_unique<DuelInfo>(target, caster, false);
    target->duel = std::make_unique<DuelInfo>(caster, caster, false);
#endif
   
    caster->SetGuidValue(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());
    target->SetGuidValue(PLAYER_DUEL_ARBITER,pGameObj->GetGUID());
    sScriptMgr->OnPlayerDuelRequest(target, caster);
}

void Spell::EffectStuck(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if(!sWorld->getConfig(CONFIG_CAST_UNSTUCK))
        return;

    Player* pTarget = unitTarget->ToPlayer();

    if(pTarget->IsInFlight())
        return;

    // Stuck spell trigger Hearthstone cooldown
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(8690);
    if(!spellInfo)
        return;
    Spell spell(pTarget, spellInfo, TRIGGERED_FULL_MASK);
    spell.SendSpellCooldown();
    
    Unit::Kill(pTarget, pTarget, false);
    // Teleport to nearest graveyard
    pTarget->RepopAtGraveyard();
}

void Spell::EffectSummonPlayer(uint32 /*i*/)
{
    // workaround - this effect should not use target map
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!_unitCaster)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    // Evil Twin (ignore player summon, but hide this for summoner)
    if(unitTarget->GetDummyAura(23445))
        return;

    float x,y,z;
    //m_caster->GetClosePoint(x,y,z,unitTarget->GetCombatReach());
    m_caster->GetPosition(x, y, z);

    (unitTarget->ToPlayer())->SetSummonPoint(m_caster->GetMapId(),x,y,z);

    WorldPacket data(SMSG_SUMMON_REQUEST, 8+4+4);
    data << uint64(_unitCaster->GetGUID());                    // summoner guid
    data << uint32(_unitCaster->GetZoneId());                  // summoner zone
    data << uint32(MAX_PLAYER_SUMMON_DELAY*1000);           // auto decline after msecs
    (unitTarget->ToPlayer())->SendDirectMessage(&data);
}

static ScriptInfo generateActivateCommand()
{
    ScriptInfo si;
    si.command = SCRIPT_COMMAND_ACTIVATE_OBJECT;
    return si;
}

void Spell::EffectActivateObject(uint32 effect_idx)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!gameObjTarget)
        return;

    if(Player* player = m_caster->ToPlayer())
        gameObjTarget->Use(player);

    ScriptInfo activateCommand;
    activateCommand.command = SCRIPT_COMMAND_ACTIVATE_OBJECT;

    gameObjTarget->GetMap()->ScriptCommandStart(activateCommand, 0, m_caster, gameObjTarget);
}

void Spell::EffectEnchantHeldItem(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

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

    if (m_spellInfo->Effects[i].MiscValue)
    {
        uint32 enchant_id = m_spellInfo->Effects[i].MiscValue;
        int32 duration = m_spellInfo->GetDuration();          //Try duration index first ..
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
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = m_caster->ToPlayer();
    if(!itemTarget || !itemTarget->GetTemplate()->DisenchantID)
        return;

    p_caster->UpdateCraftSkill(m_spellInfo->Id);

    (m_caster->ToPlayer())->SendLoot(itemTarget->GetGUID(),LOOT_DISENCHANTING);

    // item will be removed at disenchanting end
}

void Spell::EffectInebriate(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *player = unitTarget->ToPlayer();
    uint16 currentDrunk = player->GetDrunkValue();

    uint8 drunkMod = damage;
    if (currentDrunk + drunkMod > 100)
    {
        currentDrunk = 100;
#ifdef LICH_KING
        if (rand_chance() < 25.0f)
            player->CastSpell(player, 67468, TRIGGERED_NONE);    // Drunken Vomit
#endif
    }
    else
        currentDrunk += drunkMod;

    player->SetDrunkValue(currentDrunk, m_CastItem ? m_CastItem->GetEntry() : 0);
}

void Spell::EffectFeedPet(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* _player = m_caster->ToPlayer();

    Item* foodItem = itemTarget;
    if (!foodItem)
        return;

    Pet *pet = _player->GetPet();
    if(!pet)
        return;

    if(!pet->IsAlive())
        return;

    int32 benefit = pet->GetCurrentFoodBenefitLevel(foodItem->GetTemplate()->ItemLevel);
    if(benefit <= 0)
        return;

    ExecuteLogEffectDestroyItem(effIndex, foodItem->GetEntry());

    uint32 count = 1;
    _player->DestroyItemCount(foodItem, count, true);
    // TODO: fix crash when a spell has two effects, both pointed at the same item target

    CastSpellExtraArgs args;
    args.TriggerFlags = TRIGGERED_FULL_MASK;
    args.AddSpellBP0(int32(benefit));
    m_caster->CastSpell(m_caster, m_spellInfo->Effects[effIndex].TriggerSpell, args);
}

void Spell::EffectDismissPet(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    Player* playerCaster = m_caster->ToPlayer();
    if (!playerCaster)
        return;

    Pet* pet = playerCaster->GetPet();

    // do not let dismiss dead pet
    if(!pet || !pet->IsAlive())
        return;

    ExecuteLogEffectUnsummonObject(effIndex, pet);
    playerCaster->RemovePet(pet,PET_SAVE_NOT_IN_SLOT, REMOVE_PET_REASON_SCRIPT);
}

void Spell::EffectSummonObject(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!_unitCaster)
        return;

    uint32 go_id = m_spellInfo->Effects[i].MiscValue;

    uint8 slot = 0;
    switch(m_spellInfo->Effects[i].Effect)
    {
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT1: slot = 0; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT2: slot = 1; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT3: slot = 2; break;
        case SPELL_EFFECT_SUMMON_OBJECT_SLOT4: slot = 3; break;
        default: return;
    }

    ObjectGuid guid = _unitCaster->m_ObjectSlot[slot];
    if(guid != 0)
    {
        GameObject* obj = ObjectAccessor::GetGameObject(*_unitCaster, guid);

        if(obj) 
            obj->Delete();

        _unitCaster->m_ObjectSlot[slot].Clear();
    }

    auto  pGameObj = new GameObject;

    float rot2 = sin(_unitCaster->GetOrientation()/2);
    float rot3 = cos(_unitCaster->GetOrientation()/2);

    float x,y,z;
    // If dest location if present
    if (m_targets.HasDst())
        destTarget->GetPosition(x, y, z);
    // Summon in random point all other units if location present
    else
        _unitCaster->GetClosePoint(x,y,z,DEFAULT_PLAYER_BOUNDING_RADIUS);

    Map *map = m_caster->GetMap();
    if(!pGameObj->Create(map->GenerateLowGuid<HighGuid::GameObject>(), go_id, map, _unitCaster->GetPhaseMask(), Position(x, y, z, _unitCaster->GetOrientation()), G3D::Quat(0, 0, rot2, rot3), 0, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    //pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL,m_caster->GetLevel());
    int32 duration = m_spellInfo->GetDuration();
    pGameObj->SetRespawnTime(duration > 0 ? duration/1000 : 0);
    pGameObj->SetSpellId(m_spellInfo->Id);
    _unitCaster->AddGameObject(pGameObj);

    ExecuteLogEffectSummonObject(i, pGameObj);

    map->AddToMap(pGameObj, true);
    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
    data << pGameObj->GetGUID();
    _unitCaster->SendMessageToSet(&data,true);

    _unitCaster->m_ObjectSlot[slot] = pGameObj->GetGUID();
}

void Spell::EffectResurrect(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    if(unitTarget->IsAlive() || !unitTarget->IsInWorld())
        return;
    
    //sun: According to WoWWiki, silent fail on cross faction res
    if (m_caster->GetTypeId() == TYPEID_PLAYER && !sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP))
        if (m_caster->ToPlayer()->GetTeam() != unitTarget->ToPlayer()->GetTeam())
            return;

    switch (m_spellInfo->Id)
    {
        // Defibrillate (Goblin Jumper Cables) have 33% chance on success
        case 8342:
            if (roll_chance_i(67))
            {
                m_caster->CastSpell(m_caster, 8338, m_CastItem);
                return;
            }
            break;
        // Defibrillate (Goblin Jumper Cables XL) have 50% chance on success
        case 22999:
            if (roll_chance_i(50))
            {
                m_caster->CastSpell(m_caster, 23055, m_CastItem);
                return;
            }
            break;
        default:
            break;
    }

    Player* pTarget = (unitTarget->ToPlayer());

    if(pTarget->isRessurectRequested())       // already have one active request
        return;

    uint32 health = pTarget->CountPctFromMaxHealth(damage);
    uint32 mana   = CalculatePct(pTarget->GetMaxPower(POWER_MANA), damage);

    ExecuteLogEffectResurrect(effIndex, pTarget);

    pTarget->setResurrectRequestData(m_caster->GetGUID(), m_caster->GetMapId(), m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), health, mana);
    SendResurrectRequest(pTarget);
}

void Spell::EffectAddExtraAttacks(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || !unitTarget->IsAlive())
        return;

    Unit *victim = unitTarget->GetVictim();

    // attack prevented
    // fixme, some attacks may not target current victim, this is right now not handled
    if (!victim || !unitTarget->IsWithinMeleeRange(victim) || !unitTarget->HasInArc( 2*M_PI/3, victim ))
        return;

    // Only for proc/log informations
    unitTarget->m_extraAttacks = damage;

    ExecuteLogEffectExtraAttacks(effIndex, unitTarget, damage);

    unitTarget->AttackerStateUpdate(victim, BASE_ATTACK, true);
}

void Spell::EffectParry(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (unitTarget->GetTypeId() == TYPEID_PLAYER)
    {
        (unitTarget->ToPlayer())->SetCanParry(true);
    }
}

void Spell::EffectBlock(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    (unitTarget->ToPlayer())->SetCanBlock(true);
}

void Spell::EffectLeap(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(unitTarget->IsInFlight())
        return;

    if(!m_targets.HasDst())
        return;

    unitTarget->NearTeleportTo( POSITION_GET_X_Y_Z(destTarget), unitTarget->GetOrientation(), unitTarget == m_caster);
}

void Spell::EffectReputation(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = unitTarget->ToPlayer();

    int32  repChange = damage;//+1;           // field store reputation change -1

    uint32 factionId = m_spellInfo->Effects[i].MiscValue;

    FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionId);
    if(!factionEntry)
        return;

    repChange = player->CalculateReputationGain(REPUTATION_SOURCE_SPELL, 0, repChange, factionId);

    player->GetReputationMgr().ModifyReputation(factionEntry, repChange);
}

void Spell::EffectQuestComplete(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player *_player = m_caster->ToPlayer();

    uint32 quest_id = m_spellInfo->Effects[i].MiscValue;
    _player->AreaExploredOrEventHappens(quest_id);
}

void Spell::EffectSelfResurrect(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player || !player->IsInWorld() || player->IsAlive())
        return;

    uint32 health = 0;
    uint32 mana = 0;

    // flat case
    if(damage < 0)
    {
        health = uint32(-damage);
        mana = m_spellInfo->Effects[i].MiscValue;
    }
    // percent case
    else
    {
        health = uint32(damage/100.0f*player->GetMaxHealth());
        if(player->GetMaxPower(POWER_MANA) > 0)
            mana = uint32(damage/100.0f*player->GetMaxPower(POWER_MANA));
    }

    player->ResurrectPlayer(0.0f);

    player->SetHealth(health);
    player->SetPower(POWER_MANA, mana);
    player->SetPower(POWER_RAGE, 0);
    player->SetPower(POWER_ENERGY, player->GetMaxPower(POWER_ENERGY));

    player->SpawnCorpseBones();

    player->SaveToDB();
}

void Spell::EffectSkinning(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (unitTarget->GetTypeId() != TYPEID_UNIT)
        return;

    Player* player = m_caster->ToPlayer();
    if (!player)
        return;

    Creature* creature = unitTarget->ToCreature();
    int32 targetLevel = creature->GetLevel();

    uint32 skill = creature->GetCreatureTemplate()->GetRequiredLootSkill();

    creature->SetDisableGravity(false);
    creature->SetFlying(false);
    creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
    creature->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
    player->SendLoot(creature->GetGUID(), LOOT_SKINNING);

    int32 reqValue = targetLevel < 10 ? 0 : targetLevel < 20 ? (targetLevel-10)*10 : targetLevel*5;

    int32 skillValue = player->GetPureSkillValue(skill);

    // Double chances for elites
    player->UpdateGatherSkill(skill, skillValue, reqValue, creature->isElite() ? 2 : 1 );
}

void Spell::EffectCharge(uint32 i)
{
    if(!m_caster)
        return;
        
    if (!_unitCaster)
        return;

    if (_unitCaster->ToPlayer())
    {    
        if (Battleground * bg = (_unitCaster->ToPlayer())->GetBattleground()) {
            if (bg->GetStatus() == STATUS_WAIT_JOIN)
                return;
        }
    }

    Unit *target = m_targets.GetUnitTarget();
    if (!target)
        return;

    if (effectHandleMode == SPELL_EFFECT_HANDLE_LAUNCH_TARGET )
    {
        // charge changes fall time
        if (m_caster->GetTypeId() == TYPEID_PLAYER)
            m_caster->ToPlayer()->SetFallInformation(0, m_caster->GetPositionZ());

        float speed = G3D::fuzzyGt(m_spellInfo->Speed, 0.0f) ? m_spellInfo->Speed : SPEED_CHARGE;
        // Spell is not using explicit target - no generated path
        if (m_preGeneratedPath->GetPathType() == PATHFIND_BLANK)
        {
            Position pos = unitTarget->GetFirstWalkableCollisionPosition(unitTarget->GetCombatReach(), unitTarget->GetRelativeAngle(m_caster));
            _unitCaster->GetMotionMaster()->MoveCharge(pos.m_positionX, pos.m_positionY, pos.m_positionZ, speed);
        }
        else
            _unitCaster->GetMotionMaster()->MoveCharge(*m_preGeneratedPath, speed);
    }

    if (effectHandleMode == SPELL_EFFECT_HANDLE_HIT_TARGET )
    {
        //TODO: Why is this handled here?
        uint32 triggeredSpellId = 0;
        switch (i) {
        case 0:
            if (m_spellInfo->Effects[1].Effect == SPELL_EFFECT_TRIGGER_SPELL)
                triggeredSpellId = m_spellInfo->Effects[1].TriggerSpell;
            if (m_spellInfo->Effects[2].Effect == SPELL_EFFECT_TRIGGER_SPELL)
                triggeredSpellId = m_spellInfo->Effects[2].TriggerSpell;
            break;
        case 1:
            if (m_spellInfo->Effects[0].Effect == SPELL_EFFECT_TRIGGER_SPELL)
                triggeredSpellId = m_spellInfo->Effects[0].TriggerSpell;
            if (m_spellInfo->Effects[2].Effect == SPELL_EFFECT_TRIGGER_SPELL)
                triggeredSpellId = m_spellInfo->Effects[2].TriggerSpell;
            break;
        case 2:
            if (m_spellInfo->Effects[0].Effect == SPELL_EFFECT_TRIGGER_SPELL)
                triggeredSpellId = m_spellInfo->Effects[0].TriggerSpell;
            if (m_spellInfo->Effects[1].Effect == SPELL_EFFECT_TRIGGER_SPELL)
                triggeredSpellId = m_spellInfo->Effects[1].TriggerSpell;
            break;
        }
        // not all charge effects used in negative spells
        if (!m_spellInfo->IsPositive() && _unitCaster->GetTypeId() == TYPEID_PLAYER)
            _unitCaster->Attack(target, true);

        if(triggeredSpellId)
            _unitCaster->CastSpell(target, triggeredSpellId, TRIGGERED_FULL_MASK);
    }
}

void Spell::EffectKnockBack(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget)
        return;

#ifdef LICH_KING
    if (Creature* creatureTarget = unitTarget->ToCreature())
        if (creatureTarget->IsWorldBoss() || creatureTarget->IsDungeonBoss())
            return;
#else
    // Effect only works on players
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
#endif

    // Spells with SPELL_EFFECT_KNOCK_BACK (like Thunderstorm) can't knockback target if target has ROOT/STUN
    if (unitTarget->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED))
        return;

    // Instantly interrupt non melee spells being cast
    if (unitTarget->IsNonMeleeSpellCast(true))
        unitTarget->InterruptNonMeleeSpells(true);

    float ratio = 0.1f;
    float speedxy = float(m_spellInfo->Effects[i].MiscValue) * ratio;
    float speedz = float(damage) * ratio;
    if (speedxy < 0.1f && speedz < 0.1f)
        return;

    float x, y;
    if (m_spellInfo->Effects[i].Effect == SPELL_EFFECT_KNOCK_BACK_DEST)
    {
        if (m_targets.HasDst())
        {
            destTarget->GetPosition(x, y);
        }
        else
            return;
    }
    else //if (m_spellInfo->Effects[i].Effect == SPELL_EFFECT_KNOCK_BACK)
    {
        m_caster->GetPosition(x, y);
    }

    unitTarget->KnockbackFrom(x, y, speedxy, speedz);
}

void Spell::EffectSendTaxi(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    unitTarget->ToPlayer()->ActivateTaxiPathTo(m_spellInfo->Effects[i].MiscValue, m_spellInfo->Id);
}

void Spell::EffectPullTowards(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || !m_caster)
        return;

#ifndef LICH_KING
    // Effect only works on players
    if(unitTarget->GetTypeId()!=TYPEID_PLAYER)
        return;
#endif

    // Hack, effect scripted on black hole's script
    if (m_spellInfo->Id == 46230)
        return;

    //sunstrider: fully reworked logic and working for SPELL_EFFECT_PULL_TOWARDS. Not tested with SPELL_EFFECT_PULL_TOWARDS_DEST but since the only spells using it has a heavy logic in boss_muru, make sure to test the boss when removing this hack

    Position pos;
    float dist = 0.0f;
    if (m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_PULL_TOWARDS_DEST)
    {
        if (m_targets.HasDst())
        {
            pos.Relocate(*destTarget);
            dist = unitTarget->GetDistance2d(pos.GetPositionX(), pos.GetPositionY());
        } 
        else
            return;
    }
    else //if (m_spellInfo->Effects[i].Effect == SPELL_EFFECT_PULL_TOWARDS)
    {
        pos.Relocate(m_caster);
        dist = unitTarget->GetDistance2d(m_caster); //this includes combat reaches of both unit
    }

    if (damage && dist > damage) //=~ do not use damage if they would make player go over pulling unit
        dist = float(damage);
    if (dist == 0.0f)
        return;

    float speedXY = float(m_spellInfo->Effects[effIndex].MiscValue) * 0.1f;
    float time = dist / speedXY;
    float speedZ = ((pos.GetPositionZ() - unitTarget->GetPositionZ()) / time + 0.5f * time * Movement::gravity);

    (unitTarget->ToPlayer())->KnockbackFrom(pos.GetPositionX(), pos.GetPositionY(), -speedXY, speedZ);
}

void Spell::EffectDispelMechanic(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget)
        return;

    uint32 mechanic = m_spellInfo->Effects[i].MiscValue;
    
    // Luffa (http://www.wowhead.com/spell=23595): "Removes one Bleed effect.  Will not work on bleeds cast by enemies over level 60."
    if (mechanic == MECHANIC_BLEED && m_spellInfo->Id == 23595 && m_caster && m_caster->ToPlayer() && m_caster->ToPlayer()->GetLevel() > 60)
        return;

    DispelList dispel_list;
    Unit::AuraMap const& auras = unitTarget->GetOwnedAuras();
    for (Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        Aura* aura = itr->second;
        if (!aura->GetApplicationOfTarget(unitTarget->GetGUID()))
            continue;
        if (roll_chance_i(aura->CalcDispelChance(unitTarget, !unitTarget->IsFriendlyTo(m_caster))))
            if ((aura->GetSpellInfo()->GetAllEffectsMechanicMask() & (1 << mechanic)))
                dispel_list.emplace_back(aura->GetId(), aura->GetCasterGUID());
    }

    for (auto itr = dispel_list.begin(); itr != dispel_list.end(); ++itr)
        unitTarget->RemoveAura(itr->first, itr->second, 0, AURA_REMOVE_BY_ENEMY_SPELL);
}

void Spell::EffectResurrectPet(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (damage < 0)
        return;

    Player *_player = m_caster->ToPlayer();
    if(!_player)
        return;

    // Maybe player dismissed dead pet or pet despawned?
    bool hadPet = true;

    if (!_player->GetPet())
    {
        // Position passed to SummonPet is irrelevant with current implementation,
        // pet will be relocated without using these coords in Pet::LoadPetFromDB
        _player->SummonPet(0, 0.0f, 0.0f, 0.0f, 0.0f, SUMMON_PET, 0);
        hadPet = false;
    }

    Pet* pet = _player->GetPet(); // Attempt to get current pet
    if(!pet || pet->IsAlive())
        return;

    // If player did have a pet before reviving, teleport it
    if (hadPet)
    {
        // Reposition the pet's corpse before reviving so as not to grab aggro
        // We can use a different, more accurate version of GetClosePoint() since we have a pet
        float x, y, z; // Will be used later to reposition the pet if we have one
        _player->GetClosePoint(x, y, z, PET_FOLLOW_DIST, pet->GetFollowAngle());
        pet->NearTeleportTo(x, y, z, _player->GetOrientation());
        pet->Relocate(x, y, z, _player->GetOrientation()); // This is needed so SaveStayPosition() will get the proper coords.
    }

    pet->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
    pet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
    pet->SetDeathState(ALIVE);
    pet->ClearUnitState(UNIT_STATE_ALL_STATE);
    pet->SetHealth(pet->CountPctFromMaxHealth(damage));

    // Reset things for when the AI to takes over
    CharmInfo *ci = pet->GetCharmInfo();
    if (ci)
    {
        // In case the pet was at stay, we don't want it running back
        ci->SaveStayPosition();
        ci->SetIsAtStay(ci->HasCommandState(COMMAND_STAY));

        ci->SetIsFollowing(false);
        ci->SetIsCommandAttack(false);
        ci->SetIsCommandFollow(false);
        ci->SetIsReturning(false);
    }

    pet->AIM_Initialize();

    // _player->PetSpellInitialize(); -- action bar not removed at death and not required send at revive
    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
}

void Spell::EffectDestroyAllTotems(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!_unitCaster)
        return;

    int32 mana = 0;
    for (uint8 slot = SUMMON_SLOT_TOTEM; slot < MAX_TOTEM_SLOT; ++slot)
    {
        if(!slot)
            continue;

        Creature* totem = _unitCaster->GetMap()->GetCreature(_unitCaster->m_SummonSlot[slot]);
        if(totem && totem->IsTotem())
        {
            uint32 spell_id = totem->GetUInt32Value(UNIT_CREATED_BY_SPELL);
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_id);
            if (spellInfo)
            {
                mana += spellInfo->ManaCost;
                mana += int32(CalculatePct(_unitCaster->GetCreateMana(), spellInfo->ManaCostPercentage));
            }
            ((Totem*)totem)->UnSummon();
        }
    }

    ApplyPct(mana, damage);
    if (mana)
    {
        CastSpellExtraArgs args;
        args.TriggerFlags = TRIGGERED_FULL_MASK;
        args.AddSpellBP0(int32(mana));
        m_caster->CastSpell(m_caster, 39104, args);
    }

    _unitCaster->SendEnergizeSpellLog(_unitCaster, m_spellInfo->Id, mana, POWER_MANA);
}

void Spell::EffectDurabilityDamage(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    int32 slot = m_spellInfo->Effects[i].MiscValue;

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if(slot < 0)
    {
        (unitTarget->ToPlayer())->DurabilityPointsLossAll(damage,(slot < -1));
        ExecuteLogEffectDurabilityDamage(i, unitTarget, -1, -1);
        return;
    }

    // invalid slot value
    if(slot >= INVENTORY_SLOT_BAG_END)
        return;

    if (Item* item = (unitTarget->ToPlayer())->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
    {
        (unitTarget->ToPlayer())->DurabilityPointsLoss(item, damage);
        ExecuteLogEffectDurabilityDamage(i, unitTarget, item->GetEntry(), slot);
    }
}

void Spell::EffectDurabilityDamagePCT(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    int32 slot = m_spellInfo->Effects[i].MiscValue;

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
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!_unitCaster || !unitTarget)
        return;

    unitTarget->GetThreatManager().ModifyThreatByPercent(_unitCaster, damage);
}

void Spell::EffectTransmitted(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    if (!_unitCaster)
        return;

    uint32 name_id = m_spellInfo->Effects[effIndex].MiscValue;
    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(name_id);
    if (!goinfo)
    {
        TC_LOG_ERROR("network.opcode","Gameobject (Entry: %u) not exist and not created at spell (ID: %u) cast",name_id, m_spellInfo->Id);
        return;
    }

    float fx,fy,fz;

    if(m_targets.HasDst())
    {
        destTarget->GetPosition(fx, fy, fz);
    }
    //FIXME: this can be better check for most objects but still hack
    else if(m_spellInfo->Effects[effIndex].RadiusEntry->ID && m_spellInfo->Speed==0)
    {
        float dis = m_spellInfo->Effects[effIndex].CalcRadius(_unitCaster->GetSpellModOwner(), this);
        _unitCaster->GetClosePoint(fx, fy, fz, DEFAULT_PLAYER_BOUNDING_RADIUS, dis);
    }
    else
    {
        float min_dis = m_spellInfo->GetMinRange();
        float max_dis = m_spellInfo->GetMaxRange(false, m_caster->GetSpellModOwner(), this);
        float dis = rand_norm() * (max_dis - min_dis) + min_dis;

        _unitCaster->GetClosePoint(fx, fy, fz, DEFAULT_PLAYER_BOUNDING_RADIUS, dis);
    }

    Map *cMap = m_caster->GetMap();

    if(goinfo->type==GAMEOBJECT_TYPE_FISHINGNODE)
    {
        LiquidData liqData;
        // uint32 area_id = m_caster->GetAreaId();
        ZLiquidStatus status = cMap->GetLiquidStatus(fx, fy, fz + 1.0f, MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN, &liqData);
        if(status == LIQUID_MAP_NO_WATER)
            if ( !cMap->IsInWater(fx, fy, fz + 1.f/* -0.5f */, &liqData))             // Hack to prevent fishing bobber from failing to land on fishing hole
            { // but this is not proper, we really need to ignore not materialized objects
                SendCastResult(SPELL_FAILED_NOT_HERE);
                SendChannelUpdate(0);
                return;
            }

        // replace by water level in this case
        fz = liqData.level;
        
    }
    // if gameobject is summoning object, it should be spawned right on caster's position
    else if(goinfo->type==GAMEOBJECT_TYPE_SUMMONING_RITUAL)
    {
        _unitCaster->GetPosition(fx,fy,fz);
    }

    auto  pGameObj = new GameObject;

    Position pos = { fx, fy, fz, _unitCaster->GetOrientation() };
    G3D::Quat rot = G3D::Matrix3::fromEulerAnglesZYX(_unitCaster->GetOrientation(), 0.f, 0.f);
    if(!pGameObj->Create(cMap->GenerateLowGuid<HighGuid::GameObject>(), name_id, cMap, _unitCaster->GetPhaseMask(),
        pos, rot, 255, GO_STATE_READY))
    {
        delete pGameObj;
        return;
    }

    int32 duration = m_spellInfo->GetDuration();

    switch(goinfo->type)
    {
        case GAMEOBJECT_TYPE_FISHINGNODE:
        {
            _unitCaster->SetChannelObjectGuid(pGameObj->GetGUID());
                                                            // Orientation3
            pGameObj->SetFloatValue(GAMEOBJECT_PARENTROTATION + 2, 0.88431775569915771 );
                                                            // Orientation4
            pGameObj->SetFloatValue(GAMEOBJECT_PARENTROTATION + 3, -0.4668855369091033 );
            _unitCaster->AddGameObject(pGameObj);              // will removed at spell cancel

            // end time of range when possible catch fish (FISHING_BOBBER_READY_TIME..GetDuration(m_spellInfo))
            // start time == fish-FISHING_BOBBER_READY_TIME (0..GetDuration(m_spellInfo)-FISHING_BOBBER_READY_TIME)
            int32 lastSec = 0;
            switch(urand(0, 3))
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
                pGameObj->AddUniqueUse(_unitCaster->ToPlayer());
                _unitCaster->AddGameObject(pGameObj);          // will removed at spell cancel
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
    if(TempSummon* summonCaster = _unitCaster->ToTempSummon())
        if(Unit* owner = summonCaster->GetSummoner())
            pGameObj->SetOwnerGUID(owner->GetGUID());

    //pGameObj->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->GetLevel() );
    pGameObj->SetSpellId(m_spellInfo->Id);

    ExecuteLogEffectSummonObject(effIndex, pGameObj);

    cMap->AddToMap(pGameObj, true);

    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
    data << uint64(pGameObj->GetGUID());
    _unitCaster->SendMessageToSet(&data,true);

    if(uint32 linkedEntry = pGameObj->GetLinkedGameObjectEntry())
    {
        auto  linkedGO = new GameObject;
        if(linkedGO->Create(cMap->GenerateLowGuid<HighGuid::GameObject>(), linkedEntry, cMap, pGameObj->GetPhaseMask(),
            pos, rot, 255, GO_STATE_READY))
        {
            linkedGO->SetRespawnTime(duration > 0 ? duration/1000 : 0);
            //linkedGO->SetUInt32Value(GAMEOBJECT_LEVEL, m_caster->GetLevel() );
            linkedGO->SetSpellId(m_spellInfo->Id);
            linkedGO->SetOwnerGUID(_unitCaster->GetGUID() );

            ExecuteLogEffectSummonObject(effIndex, linkedGO);

            linkedGO->GetMap()->AddToMap(linkedGO, true);
        }
        else
        {
            delete linkedGO;
            linkedGO = nullptr;
            return;
        }
    }
}

void Spell::EffectProspecting(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* p_caster = m_caster->ToPlayer();
    if(!itemTarget || !(itemTarget->GetTemplate()->BagFamily & BAG_FAMILY_MASK_MINING_SUPP))
        return;

    if(itemTarget->GetCount() < 5)
        return;

    if( sWorld->getConfig(CONFIG_SKILL_PROSPECTING))
    {
        uint32 SkillValue = p_caster->GetPureSkillValue(SKILL_JEWELCRAFTING);
        uint32 reqSkillValue = itemTarget->GetTemplate()->RequiredSkillRank;
        p_caster->UpdateGatherSkill(SKILL_JEWELCRAFTING, SkillValue, reqSkillValue);
    }

    (m_caster->ToPlayer())->SendLoot(itemTarget->GetGUID(), LOOT_PROSPECTING);
}

void Spell::EffectSkill(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT)
        return;

    // TODO
}

/* There is currently no need for this effect. We handle it in Battleground.cpp
   If we would handle the resurrection here, the spiritguide would instantly disappear as the
   player revives, and so we wouldn't see the spirit heal visual effect on the npc.
   This is why we use a half sec delay between the visual effect and the resurrection itself */
void Spell::EffectSpiritHeal(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    /*
    if(!unitTarget || unitTarget->IsAlive())
        return;
    if(unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;
    if(!unitTarget->IsInWorld())
        return;

    //m_spellInfo->Effects[i].BasePoints; == 99 (percent?)
    //(unitTarget->ToPlayer())->setResurrect(m_caster->GetGUID(), unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(), unitTarget->GetMaxHealth(), unitTarget->GetMaxPower(POWER_MANA));
    (unitTarget->ToPlayer())->ResurrectPlayer(1.0f);
    (unitTarget->ToPlayer())->SpawnCorpseBones();
    */
}

// remove insignia spell effect
void Spell::EffectSkinPlayerCorpse(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if ( (m_caster->GetTypeId() != TYPEID_PLAYER) || (unitTarget->GetTypeId() != TYPEID_PLAYER) || (unitTarget->IsAlive()) )
        return;

    (unitTarget->ToPlayer())->RemovedInsignia( m_caster->ToPlayer() );
}

void Spell::EffectStealBeneficialBuff(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    TC_LOG_DEBUG("spells", "Effect: StealBeneficialBuff");

    if (!unitTarget || unitTarget == m_caster)                 // can't steal from self
        return;

    DispelChargesList stealList;

    // Create dispel mask by dispel type
    uint32 dispelMask = SpellInfo::GetDispelMask(DispelType(m_spellInfo->Effects[effIndex].MiscValue));
    Unit::AuraMap const& auras = unitTarget->GetOwnedAuras();
    for (Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        Aura* aura = itr->second;
        AuraApplication const* aurApp = aura->GetApplicationOfTarget(unitTarget->GetGUID());
        if (!aurApp)
            continue;

        if ((aura->GetSpellInfo()->GetDispelMask()) & dispelMask)
        {
            // Need check for passive? this
            if (!aurApp->IsPositive() || aura->IsPassive() || aura->GetSpellInfo()->HasAttribute(SPELL_ATTR4_NOT_STEALABLE))
                continue;

            // 2.4.3 Patch Notes: "Dispel effects will no longer attempt to remove effects that have 100% dispel resistance."
            int32 chance = aura->CalcDispelChance(unitTarget, !unitTarget->IsFriendlyTo(m_caster));
            if (!chance)
                continue;

            // The charges / stack amounts don't count towards the total number of auras that can be dispelled.
            // Ie: A dispel on a target with 5 stacks of Winters Chill and a Polymorph has 1 / (1 + 1) -> 50% chance to dispell
            // Polymorph instead of 1 / (5 + 1) -> 16%.
#ifdef LICH_KING
            bool dispelCharges = aura->GetSpellInfo()->HasAttribute(SPELL_ATTR7_DISPEL_CHARGES);
            uint8 charges = dispelCharges ? aura->GetCharges() : aura->GetStackAmount();
#else
            uint8 charges = aura->GetStackAmount();
#endif
            if (charges > 0)
                stealList.emplace_back(aura, chance, charges);
        }
    }

    if (stealList.empty())
        return;

    size_t remaining = stealList.size();

    // Ok if exist some buffs for dispel try dispel it
    uint32 failCount = 0;
    DispelList successList;
    successList.reserve(damage);

    WorldPacket dataFail(SMSG_DISPEL_FAILED, 8 + 8 + 4 + 4 + damage * 4);
    // dispel N = damage buffs (or while exist buffs for dispel)
    for (int32 count = 0; count < damage && remaining > 0;)
    {
        // Random select buff for dispel
        DispelChargesList::iterator itr = stealList.begin();
        std::advance(itr, urand(0, remaining - 1));

        if (itr->RollDispel())
        {
            successList.emplace_back(itr->GetAura()->GetId(), itr->GetAura()->GetCasterGUID());
            if (!itr->DecrementCharge())
            {
                --remaining;
                std::swap(*itr, stealList[remaining]);
            }
        }
        else
        {
            if (!failCount)
            {
                // Failed to dispell
                dataFail << uint64(m_caster->GetGUID());            // Caster GUID
                dataFail << uint64(unitTarget->GetGUID());          // Victim GUID
                dataFail << uint32(m_spellInfo->Id);                // dispel spell id
            }
            ++failCount;
            dataFail << uint32(itr->GetAura()->GetId());            // Spell Id
        }
        ++count;
    }

    if (failCount)
        m_caster->SendMessageToSet(&dataFail, true);

    if (successList.empty())
        return;

    WorldPacket dataSuccess(SMSG_SPELLSTEALLOG, 8 + 8 + 4 + 1 + 4 + damage * 5);
    dataSuccess << unitTarget->GetPackGUID();   // Victim GUID
    dataSuccess << m_caster->GetPackGUID();     // Caster GUID
    dataSuccess << uint32(m_spellInfo->Id);     // dispel spell id
    dataSuccess << uint8(0);                    // not used
    dataSuccess << uint32(successList.size());  // count
    for (auto itr = successList.begin(); itr != successList.end(); ++itr)
    {
        dataSuccess << uint32(itr->first);          // Spell Id
        dataSuccess << uint8(0);                    // 0 - steals !=0 transfers
        unitTarget->RemoveAurasDueToSpellBySteal(itr->first, itr->second, m_caster);
    }
    m_caster->SendMessageToSet(&dataSuccess, true);
}

void Spell::EffectKillCredit(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    (unitTarget->ToPlayer())->KilledMonsterCredit(m_spellInfo->Effects[i].MiscValue, ObjectGuid::Empty);
}

void Spell::EffectQuestFail(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if(!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    (unitTarget->ToPlayer())->FailQuest(m_spellInfo->Effects[i].MiscValue);
}

void Spell::EffectBind(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* player = (Player*)unitTarget;

    uint32 area_id;
    WorldLocation loc;
    loc.m_mapId = player->GetMapId();
    loc.m_positionX = player->GetPositionX();
    loc.m_positionY = player->GetPositionY();
    loc.m_positionZ = player->GetPositionZ();
    loc.m_orientation = player->GetOrientation();

    area_id = player->GetAreaId();

    player->SetHomebind(loc, area_id);

    // binding
    WorldPacket data(SMSG_BINDPOINTUPDATE, (4+4+4+4+4));
    data << float(loc.m_positionX);
    data << float(loc.m_positionY);
    data << float(loc.m_positionZ);
    data << uint32(loc.m_mapId);
    data << uint32(area_id);
    player->SendDirectMessage(&data);

    TC_LOG_DEBUG("spells", "EffectBind: New homebind X: %f, Y: %f, Z: %f, MapId: %u, AreaId: %u",
        loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), loc.GetMapId(), area_id);

    // zone update
    data.Initialize(SMSG_PLAYERBOUND, 8+4);
    data << uint64(m_caster->GetGUID());
    data << uint32(area_id);
    player->SendDirectMessage(&data);
}

void Spell::EffectRedirectThreat(uint32 /*i*/)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!_unitCaster)
        return;

#ifndef LICH_KING
    //only misdirect spell on BC, redirect 100%
    damage = 100;
#endif

    if (unitTarget)
        _unitCaster->GetThreatManager().RegisterRedirectThreat(m_spellInfo->Id, unitTarget->GetGUID(), uint32(damage));
}

void Spell::EffectPlaySound(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    Player* player = unitTarget->ToPlayer();
    if (!player)
        return;

#ifdef LICH_KING
    switch (m_spellInfo->Id)
    {
    case 58730: // Restricted Flight Area
    case 58600: // Restricted Flight Area
        player->GetSession()->SendNotification(LANG_ZONE_NOFLYZONE);
        break;
    default:
        break;
    }
#endif

    uint32 soundId = m_spellInfo->Effects[effIndex].MiscValue;

    if (!sSoundEntriesStore.LookupEntry(soundId))
    {
        TC_LOG_ERROR("spells", "EffectPlaySound: Sound (Id: %u) does not exist in spell %u.", soundId, m_spellInfo->Id);
        return;
    }

    player->PlayDirectSound(soundId, player);
}

void Spell::EffectPlayMusic(uint32 effIndex)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget || unitTarget->GetTypeId() != TYPEID_PLAYER)
        return;

    uint32 soundid = m_spellInfo->Effects[effIndex].MiscValue;

    if (!sSoundEntriesStore.LookupEntry(soundid))
    {
        TC_LOG_ERROR("spells", "EffectPlayMusic: Sound (Id: %u) does not exist in spell %u.", soundid, m_spellInfo->Id);
        return;
    }

    unitTarget->ToPlayer()->GetSession()->SendPlayMusic(soundid);
}

void Spell::EffectForceCastWithValue(uint32 i)
{
    if (effectHandleMode != SPELL_EFFECT_HANDLE_HIT_TARGET)
        return;

    if (!unitTarget)
        return;

    uint32 triggered_spell_id = m_spellInfo->Effects[i].TriggerSpell;

    // normal case
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(triggered_spell_id);

    if (!spellInfo)
    {
        TC_LOG_ERROR("FIXME","EffectForceCastWithValue of spell %u: triggering unknown spell id %i", m_spellInfo->Id, triggered_spell_id);
        return;
    }
    
    int32 bp = damage;
    CastSpellExtraArgs args;
    args.TriggerFlags = TRIGGERED_FULL_MASK;
    args.AddSpellMod(SPELLVALUE_BASE_POINT0, int32(bp));
    args.AddSpellMod(SPELLVALUE_BASE_POINT1, int32(bp));
    args.AddSpellMod(SPELLVALUE_BASE_POINT2, int32(bp));
    unitTarget->CastSpell(m_caster, spellInfo->Id, args);
}
