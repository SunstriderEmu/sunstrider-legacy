#include <omp.h>

#include "Log.h"

#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Unit.h"
#include "QuestDef.h"
#include "Player.h"
#include "Creature.h"
#include "Spell.h"
#include "Group.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "Formulas.h"
#include "Pet.h"
#include "WaypointMovementGenerator.h"

#include "Totem.h"
#include "BattleGround.h"
#include "OutdoorPvP.h"
#include "InstanceSaveMgr.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ChatTextBuilder.h"
#include "PathGenerator.h"
#include "CreatureGroups.h"
#include "PetAI.h"
#include "NullCreatureAI.h"
#include "ScriptCalls.h"
#include "ScriptMgr.h"

#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "Transport.h"
#include "InstanceScript.h"
#include "UpdateFieldFlags.h"
#include "LogsDatabaseAccessor.h"

#include <math.h>

float baseMoveSpeed[MAX_MOVE_TYPE] =
{
    2.5f,                                                   // MOVE_WALK
    7.0f,                                                   // MOVE_RUN
    4.5f,                                                   // MOVE_RUN_BACK
    4.722222f,                                              // MOVE_SWIM
    2.5f,                                                   // MOVE_SWIM_BACK
    3.141594f,                                              // MOVE_TURN_RATE
    7.0f,                                                   // MOVE_FLIGHT
    4.5f,                                                   // MOVE_FLIGHT_BACK
};

void InitTriggerAuraData();

// auraTypes contains attacker auras capable of proc'ing cast auras
static Unit::AuraTypeSet GenerateAttakerProcCastAuraTypes()
{
    static Unit::AuraTypeSet auraTypes;
    auraTypes.insert(SPELL_AURA_DUMMY);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_SPELL);
    auraTypes.insert(SPELL_AURA_MOD_HASTE);
    auraTypes.insert(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    return auraTypes;
}

// auraTypes contains victim auras capable of proc'ing cast auras
static Unit::AuraTypeSet GenerateVictimProcCastAuraTypes()
{
    static Unit::AuraTypeSet auraTypes;
    auraTypes.insert(SPELL_AURA_DUMMY);
    auraTypes.insert(SPELL_AURA_PRAYER_OF_MENDING);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_SPELL);
    return auraTypes;
}

// auraTypes contains auras capable of proc effect/damage (but not cast) for attacker
static Unit::AuraTypeSet GenerateAttakerProcEffectAuraTypes()
{
    static Unit::AuraTypeSet auraTypes;
    auraTypes.insert(SPELL_AURA_MOD_DAMAGE_DONE);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_DAMAGE);
    auraTypes.insert(SPELL_AURA_MOD_CASTING_SPEED);
    auraTypes.insert(SPELL_AURA_MOD_RATING);
    return auraTypes;
}

// auraTypes contains auras capable of proc effect/damage (but not cast) for victim
static Unit::AuraTypeSet GenerateVictimProcEffectAuraTypes()
{
    static Unit::AuraTypeSet auraTypes;
    auraTypes.insert(SPELL_AURA_MOD_RESISTANCE);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_DAMAGE);
    auraTypes.insert(SPELL_AURA_MOD_PARRY_PERCENT);
    auraTypes.insert(SPELL_AURA_MOD_BLOCK_PERCENT);
    auraTypes.insert(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    return auraTypes;
}

static Unit::AuraTypeSet attackerProcCastAuraTypes = GenerateAttakerProcCastAuraTypes();
static Unit::AuraTypeSet attackerProcEffectAuraTypes = GenerateAttakerProcEffectAuraTypes();

static Unit::AuraTypeSet victimProcCastAuraTypes = GenerateVictimProcCastAuraTypes();
static Unit::AuraTypeSet victimProcEffectAuraTypes   = GenerateVictimProcEffectAuraTypes();

// auraTypes contains auras capable of proc'ing for attacker and victim
static Unit::AuraTypeSet GenerateProcAuraTypes()
{
    InitTriggerAuraData();

    Unit::AuraTypeSet auraTypes;
    auraTypes.insert(attackerProcCastAuraTypes.begin(),attackerProcCastAuraTypes.end());
    auraTypes.insert(attackerProcEffectAuraTypes.begin(),attackerProcEffectAuraTypes.end());
    auraTypes.insert(victimProcCastAuraTypes.begin(),victimProcCastAuraTypes.end());
    auraTypes.insert(victimProcEffectAuraTypes.begin(),victimProcEffectAuraTypes.end());
    return auraTypes;
}

static Unit::AuraTypeSet procAuraTypes = GenerateProcAuraTypes();

bool IsPassiveStackableSpell( uint32 spellId )
{
    if(!IsPassiveSpell(spellId))
        return false;

    SpellInfo const* spellProto = sSpellMgr->GetSpellInfo(spellId);
    if(!spellProto)
        return false;

    for(int j = 0; j < 3; ++j)
    {
        //from Hellground : set::find is faster than std::find ( O(logN) < O(N))
        if (procAuraTypes.find(Unit::AuraTypeSet::value_type(spellProto->Effects[j].ApplyAuraName)) != procAuraTypes.end())
            return false;
    }

    return true;
}

Unit::Unit()
: WorldObject(), m_movedByPlayer(nullptr), i_motionMaster(new MotionMaster(this)), m_ThreatManager(this), m_HostileRefManager(this),
m_IsInNotifyList(false), m_Notified(false), IsAIEnabled(false), NeedChangeAI(false), movespline(new Movement::MoveSpline()),
i_AI(NULL), i_disabledAI(NULL), m_removedAurasCount(0), m_procDeep(0), m_unitTypeMask(UNIT_MASK_NONE),
_lastDamagedTime(0), m_movesplineTimer(0)
{
    m_objectType |= TYPEMASK_UNIT;
    m_objectTypeId = TYPEID_UNIT;
                                                            // 2.3.2 - 0x70
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_LIVING | UPDATEFLAG_STATIONARY_POSITION);

    m_attackTimer[BASE_ATTACK]   = 0;
    m_attackTimer[OFF_ATTACK]    = 0;
    m_attackTimer[RANGED_ATTACK] = 0;
    m_modAttackSpeedPct[BASE_ATTACK] = 1.0f;
    m_modAttackSpeedPct[OFF_ATTACK] = 1.0f;
    m_modAttackSpeedPct[RANGED_ATTACK] = 1.0f;

    m_extraAttacks = 0;
    m_canDualWield = false;
    m_justCCed = 0;

    m_rootTimes = 0;

    m_state = 0;
    m_form = FORM_NONE;
    m_deathState = ALIVE;

    for (uint32 i = 0; i < CURRENT_MAX_SPELL; i++)
        m_currentSpells[i] = NULL;

    m_addDmgOnce = 0;

    for(int i = 0; i < MAX_TOTEM; ++i)
        m_TotemSlot[i]  = 0;

    m_ObjectSlot[0] = m_ObjectSlot[1] = m_ObjectSlot[2] = m_ObjectSlot[3] = 0;
    //m_Aura = NULL;
    //m_AurasCheck = 2000;
    //m_removeAuraTimer = 4;
    //tmpAura = NULL;

    m_AurasUpdateIterator = m_Auras.end();
    m_Visibility = VISIBILITY_ON;

    m_interruptMask = 0;
    m_detectInvisibilityMask = 0;
    m_invisibilityMask = 0;
    m_transform = 0;
    m_ShapeShiftFormSpellId = 0;
    m_canModifyStats = false;

    for (int i = 0; i < MAX_SPELL_IMMUNITY; ++i)
        m_spellImmune[i].clear();
    for (int i = 0; i < UNIT_MOD_END; ++i)
    {
        m_auraModifiersGroup[i][BASE_VALUE] = 0.0f;
        m_auraModifiersGroup[i][BASE_PCT] = 1.0f;
        m_auraModifiersGroup[i][TOTAL_VALUE] = 0.0f;
        m_auraModifiersGroup[i][TOTAL_PCT] = 1.0f;
    }
                                                            // implement 50% base damage from offhand
    m_auraModifiersGroup[UNIT_MOD_DAMAGE_OFFHAND][TOTAL_PCT] = 0.5f;

    for (int i = 0; i < 3; i++)
    {
        m_weaponDamage[i][MINDAMAGE] = BASE_MINDAMAGE;
        m_weaponDamage[i][MAXDAMAGE] = BASE_MAXDAMAGE;
    }
    for (int i = 0; i < MAX_STATS; ++i)
        m_createStats[i] = 0.0f;

    m_attacking = NULL;
    m_modMeleeHitChance = 0.0f;
    m_modRangedHitChance = 0.0f;
    m_modSpellHitChance = 0.0f;
    m_baseSpellCritChance = 5;

    m_CombatTimer = 0;
    m_lastManaUse = 0;

    //m_victimThreat = 0.0f;
    for (int i = 0; i < MAX_SPELL_SCHOOL; ++i)
        m_threatModifier[i] = 1.0f;
    m_isSorted = true;
    for (int i = 0; i < MAX_MOVE_TYPE; ++i)
        m_speed_rate[i] = 1.0f;

    m_charmInfo = NULL;
    m_redirectThreatPercent = 0;
    m_misdirectionTargetGUID = 0;
    m_misdirectionLastTargetGUID = 0;

    // remove aurastates allowing special moves
    for(int i=0; i < MAX_REACTIVE; ++i)
        m_reactiveTimer[i] = 0;
        
    IsRotating = 0;
    m_attackVictimOnEnd = false;
    
    _targetLocked = false;

    _lastLiquid = NULL;
}

Unit::~Unit()
{
    // set current spells as deletable
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; i++)
    {
        if (m_currentSpells[i])
        {
            m_currentSpells[i]->SetReferencedFromCurrent(false);
            m_currentSpells[i] = NULL;
        }
    }

    RemoveAllGameObjects();
    RemoveAllDynObjects();
    _DeleteAuras();

    delete i_motionMaster;
    delete movespline;

    // remove view point for spectator
    if (!m_sharedVision.empty())
    {
        for (auto itr : m_sharedVision)
        {
            if(Player* p = ObjectAccessor::GetPlayer(*this, itr))
            {
                if (p->isSpectator() && p->getSpectateFrom())
                {
                    p->getSpectateFrom()->RemovePlayerFromVision(p);
                    if (m_sharedVision.empty())
                        break;
                    --itr;
                } else {
                    RemovePlayerFromVision(p);
                    if (m_sharedVision.empty())
                        break;
                    --itr;
                }
            }
        }
        m_sharedVision.clear();
    }

    if(m_charmInfo) delete m_charmInfo;

    assert(!m_attacking);
    assert(m_attackers.empty());
    assert(m_sharedVision.empty());

#ifdef WITH_UNIT_CRASHFIX
    for (unsigned int i = 0; i < TOTAL_AURAS; i++) {
        if (m_modAuras[i]._M_impl._M_node._M_prev == NULL) {
            TC_LOG_ERROR("AURA:Corrupted m_modAuras _M_prev (%p) at index %d (higuid %d loguid %d)", m_modAuras, i, GetGUIDHigh(), GetGUIDLow());
            m_modAuras[i]._M_impl._M_node._M_prev = m_modAuras[i]._M_impl._M_node._M_next;
        }

        if (m_modAuras[i]._M_impl._M_node._M_next == NULL) {
            TC_LOG_ERROR("AURA:Corrupted m_modAuras _M_next (%p) at index %d (higuid %d loguid %d)", m_modAuras, i, GetGUIDHigh(), GetGUIDLow());
            m_modAuras[i]._M_impl._M_node._M_next = m_modAuras[i]._M_impl._M_node._M_prev;
        }
    }
#endif
}

void Unit::Update( uint32 p_time )
{
    // WARNING! Order of execution here is important, do not change.
    // Spells must be processed with event system BEFORE they go to _UpdateSpells.
    // Or else we may have some SPELL_STATE_FINISHED spells stalled in pointers, that is bad.
    m_Events.Update( p_time );

    if (!IsInWorld())
        return;

    _UpdateSpells( p_time );
    if (m_justCCed)
        m_justCCed--;

    if (IsInCombat())
    {
        // update combat timer only for players and pets
        if (GetTypeId() == TYPEID_PLAYER || (this->ToCreature())->IsPet() || (this->ToCreature())->IsCharmed())
        {
            // Check UNIT_STATE_MELEE_ATTACKING or UNIT_STATE_CHASE (without UNIT_STATE_FOLLOW in this case) so pets can reach far away
            // targets without stopping half way there and running off.
            // These flags are reset after target dies or another command is given.
            if( m_HostileRefManager.isEmpty() )
            {
                // m_CombatTimer set at aura start and it will be freeze until aura removing
                if ( m_CombatTimer <= p_time )
                    ClearInCombat();
                else
                    m_CombatTimer -= p_time;
            }
        }
    }

    //not implemented before 3.0.2
    //if(!HasUnitState(UNIT_STATE_CASTING))
    {
        if(uint32 base_att = GetAttackTimer(BASE_ATTACK))
            SetAttackTimer(BASE_ATTACK, (p_time >= base_att ? 0 : base_att - p_time) );
        if(uint32 ranged_att = GetAttackTimer(RANGED_ATTACK))
            SetAttackTimer(RANGED_ATTACK, (p_time >= ranged_att ? 0 : ranged_att - p_time) );
        if(uint32 off_att = GetAttackTimer(OFF_ATTACK))
            SetAttackTimer(OFF_ATTACK, (p_time >= off_att ? 0 : off_att - p_time) );
    }

    // update abilities available only for fraction of time
    UpdateReactives( p_time );

    if (IsAlive())
    {
        ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, HealthBelowPct(20));
        ModifyAuraState(AURA_STATE_HEALTHLESS_35_PERCENT, HealthBelowPct(35));
    }

    UpdateSplineMovement(p_time);
    i_motionMaster->UpdateMotion(p_time);
}

bool Unit::HaveOffhandWeapon() const
{
    if (Player const* player = ToPlayer())
        return player->GetWeaponForAttack(OFF_ATTACK, true) != nullptr;

    return CanDualWield();
}

void Unit::MonsterMoveWithSpeed(float x, float y, float z, float speed, bool generatePath, bool forceDestination)
{
    Movement::MoveSplineInit init(this);
    init.MoveTo(x, y, z, generatePath, forceDestination);
    init.SetVelocity(speed);
    init.Launch();
}


void Unit::ResetAttackTimer(WeaponAttackType type)
{
    m_attackTimer[type] = uint32(GetAttackTime(type) * m_modAttackSpeedPct[type]);
}

bool Unit::IsWithinCombatRange(Unit *obj, float dist2compare) const
{
    if (!obj || !IsInMap(obj)) return false;

    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float distsq = dx*dx + dy*dy + dz*dz;

    float sizefactor = GetCombatReach() + obj->GetCombatReach();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool Unit::IsWithinMeleeRange(Unit *obj, float dist) const
{
    if (!obj || !IsInMap(obj)) return false;

    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float distsq = dx*dx + dy*dy + dz*dz;

    float sizefactor = GetMeleeReach() + obj->GetMeleeReach();
    float maxdist = dist + sizefactor;

    return distsq < maxdist * maxdist;
}

void Unit::GetRandomContactPoint( const Unit* obj, float &x, float &y, float &z, float distance2dMin, float distance2dMax ) const
{
    float combat_reach = GetCombatReach();
    if(combat_reach < 0.1) // sometimes bugged for players
    {
        //TC_LOG_ERROR("Unit %u (Type: %u) has invalid combat_reach %f",GetGUIDLow(),GetTypeId(),combat_reach);
       // if(GetTypeId() ==  TYPEID_UNIT)
          //  TC_LOG_ERROR("FIXME","Creature entry %u has invalid combat_reach", (this->ToCreature())->GetEntry());
        combat_reach = DEFAULT_COMBAT_REACH;
    }
    uint32 attacker_number = GetAttackers().size();
    if(attacker_number > 0) --attacker_number;
    GetNearPoint(obj,x,y,z,obj->GetCombatReach(), distance2dMin+(distance2dMax-distance2dMin)*GetMap()->rand_norm()
                 , GetAngle(obj) + (attacker_number ? (M_PI/2 - M_PI * GetMap()->rand_norm()) * (float)attacker_number / combat_reach / 3 : 0));
}

void Unit::StartAutoRotate(uint8 type, uint32 fulltime, double Angle, bool attackVictimOnEnd)
{
    m_attackVictimOnEnd = attackVictimOnEnd;

    if (Angle > 0)
    {
        RotateAngle = Angle;
    }
    else
    {
        if(GetVictim())
            RotateAngle = GetAngle(GetVictim());
        else
            RotateAngle = GetOrientation();
    }

    RotateTimer = fulltime;    
    RotateTimerFull = fulltime;    
    IsRotating = type;
    LastTargetGUID = GetUInt64Value(UNIT_FIELD_TARGET);
    SetTarget(0);
}

void Unit::AutoRotate(uint32 time)
{
    if(!IsRotating)return;
    if(IsRotating == CREATURE_ROTATE_LEFT)
    {
        RotateAngle += (double)time/RotateTimerFull*(double)M_PI*2;
        if (RotateAngle >= M_PI*2)RotateAngle = 0;
    }
    else
    {
        RotateAngle -= (double)time/RotateTimerFull*(double)M_PI*2;
        if (RotateAngle < 0)RotateAngle = M_PI*2;
    }    
    SetOrientation(RotateAngle);
    StopMoving();
    if(RotateTimer <= time)
    {
        IsRotating = CREATURE_ROTATE_NONE;
        RotateAngle = 0;
        RotateTimer = RotateTimerFull;
        if (m_attackVictimOnEnd)
            SetTarget(LastTargetGUID);
    }else RotateTimer -= time;
}

void Unit::RemoveMovementImpairingAuras()
{
    for(AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        if(iter->second->GetSpellInfo()->HasAttribute(SPELL_ATTR_CU_MOVEMENT_IMPAIR))
            RemoveAura(iter);
        else
            ++iter;
    }
}

void Unit::RemoveAurasByType(AuraType auraType)
{
    if (auraType >= TOTAL_AURAS) return;
    AuraList::iterator iter, next;
    for (iter = m_modAuras[auraType].begin(); iter != m_modAuras[auraType].end(); iter = next)
    {
        next = iter;
        ++next;

        if (*iter)
        {
            RemoveAurasDueToSpell((*iter)->GetId());
            if (!m_modAuras[auraType].empty())
                next = m_modAuras[auraType].begin();
            else
                return;
        }
    }
}

void Unit::RemoveAuraTypeByCaster(AuraType auraType, uint64 casterGUID)
{
    if (auraType >= TOTAL_AURAS) return;

    for(AuraList::iterator iter = m_modAuras[auraType].begin(); iter != m_modAuras[auraType].end(); )
    {
        Aura *aur = *iter;
        ++iter;

        if (aur)
        {
            uint32 removedAuras = m_removedAurasCount;
            RemoveAurasByCasterSpell(aur->GetId(), casterGUID);
            if (m_removedAurasCount > removedAuras + 1)
                iter = m_modAuras[auraType].begin();
        }
    }
}

void Unit::RemoveAurasWithInterruptFlags(uint32 flag, uint32 except, bool withChanneled)
{
    if(!(m_interruptMask & flag))
        return;

    // interrupt auras
    AuraList::iterator iter;
    for (iter = m_interruptableAuras.begin(); iter != m_interruptableAuras.end(); )
    {
        Aura *aur = *iter;
        ++iter;

        //TC_LOG_DEBUG("FIXME","auraflag:%u flag:%u = %u", aur->GetSpellInfo()->AuraInterruptFlags,flag, aur->GetSpellInfo()->AuraInterruptFlags & flag);

        if(aur && (aur->GetSpellInfo()->AuraInterruptFlags & flag))
        {
            if(aur->IsInUse())
                TC_LOG_ERROR("FIXME","Aura %u is trying to remove itself! Flag %u. May cause crash!", aur->GetId(), flag);

            else if(!except || aur->GetId() != except)
            {
                uint32 removedAuras = m_removedAurasCount;

                RemoveAurasDueToSpell(aur->GetId());
                if (m_removedAurasCount > removedAuras + 1)
                    iter = m_interruptableAuras.begin();

            }
        }
    }

    // interrupt channeled spell
    if (withChanneled) {
        if(Spell* spell = m_currentSpells[CURRENT_CHANNELED_SPELL])
            if(spell->getState() == SPELL_STATE_CASTING
                && (spell->m_spellInfo->ChannelInterruptFlags & flag)
                && spell->m_spellInfo->Id != except)
                InterruptNonMeleeSpells(false);
    }

    UpdateInterruptMask();
}

void Unit::UpdateInterruptMask()
{
    m_interruptMask = 0;
    for(AuraList::iterator i = m_interruptableAuras.begin(); i != m_interruptableAuras.end(); ++i)
    {
        if(*i)
            m_interruptMask |= (*i)->GetSpellInfo()->AuraInterruptFlags;
    }
    if(Spell* spell = m_currentSpells[CURRENT_CHANNELED_SPELL])
        if(spell->getState() == SPELL_STATE_CASTING)
            m_interruptMask |= spell->m_spellInfo->ChannelInterruptFlags;
}

uint32 Unit::GetAuraCount(uint32 spellId) const
{
    uint32 count = 0;
    for (AuraMap::const_iterator itr = m_Auras.lower_bound(spellEffectPair(spellId, 0)); itr != m_Auras.upper_bound(spellEffectPair(spellId, 0)); ++itr)
    {
        if (!itr->second->GetStackAmount())
            count++;
        else
            count += (uint32)itr->second->GetStackAmount();
    }

    return count;
}

bool Unit::HasAuraType(AuraType auraType) const
{
    return (!m_modAuras[auraType].empty());
}

bool Unit::HasAuraTypeWithFamilyFlags(AuraType auraType, uint32 familyName,uint64 familyFlags) const
{
    AuraList const &auras = GetAurasByType(auraType);
    for(AuraList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
        if(SpellInfo const *iterSpellProto = (*itr)->GetSpellInfo())
            if(iterSpellProto->SpellFamilyName == familyName && iterSpellProto->SpellFamilyFlags & familyFlags)
                return true;

    return false;
}

bool Unit::HasAuraTypeWithCaster(AuraType auraType, uint64 casterGUID) const
{
    AuraList const &auras = GetAurasByType(auraType);
    for(auto itr : auras)
        if(itr->GetCasterGUID() == casterGUID)
            return true;

    return false;
}

bool Unit::HasAuraWithCaster(uint32 spellId, uint32 effIndex, uint64 casterGUID) const
{
    for(auto itr : m_Auras)
    {
        if(    itr.second->GetId() == spellId
            && itr.second->GetEffIndex() == effIndex
            && itr.second->GetCasterGUID() == casterGUID)
            return true;
    }
    return false;
}

bool Unit::HasAuraWithCasterNot(uint32 spellId, uint32 effIndex, uint64 casterGUID) const
{
    for(auto itr : m_Auras)
    {
         if(   itr.second->GetId() == spellId
            && itr.second->GetEffIndex() == effIndex
            && itr.second->GetCasterGUID() != casterGUID)
            return true;
    }
    return false;
}

/* Called by DealDamage for auras that have a chance to be dispelled on damage taken. */
void Unit::RemoveSpellbyDamageTaken(uint32 damage, uint32 spellId)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if(!spellInfo || spellInfo->HasAttribute(SPELL_ATTR_CU_CANT_BREAK_CC))
        return;

    // The chance to dispel an aura depends on the damage taken with respect to the casters level.
    uint32 max_dmg = GetLevel() > 8 ? 30 * GetLevel() - 100 : 50;
    float chance = (float(damage) / max_dmg * 100.0f)*0.8;

    AuraList::iterator i, next;
    for(i = m_ccAuras.begin(); i != m_ccAuras.end(); i = next)
    {
        next = i;
        ++next;

        if(*i && (!spellId || (*i)->GetId() != spellId) && roll_chance_f(chance))
        {
            RemoveAurasDueToSpell((*i)->GetId());
            if (!m_ccAuras.empty())
                next = m_ccAuras.begin();
            else
                return;
        }
    }
}

uint32 Unit::DealDamage(Unit *pVictim, uint32 damage, CleanDamage const* cleanDamage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask, SpellInfo const *spellProto, bool durabilityLoss)
{
    if(pVictim->GetTypeId()== TYPEID_UNIT && (pVictim->ToCreature())->IsAIEnabled)
    {
        pVictim->ToCreature()->AI()->DamageTaken(this, damage);
    }

    if(this->GetTypeId()== TYPEID_UNIT && (this->ToCreature())->IsAIEnabled)
        if (IsAIEnabled)
            ToCreature()->AI()->DamageDealt(pVictim, damage, damagetype);

    if (!pVictim->IsAlive() || pVictim->IsInFlight() || (pVictim->GetTypeId() == TYPEID_UNIT && (pVictim->ToCreature())->IsInEvadeMode()))
        return 0;

    // Kidney Shot
    if (pVictim->HasAuraEffect(408) || pVictim->HasAuraEffect(8643)) {
        Aura *aur = NULL;
        if (pVictim->HasAuraEffect(408))
            aur = pVictim->GetAura(408, 0);
        else if (pVictim->HasAuraEffect(8643))
            aur = pVictim->GetAura(8643, 0);
        if (aur) {
            Unit *ksCaster = aur->GetCaster();
            if (ksCaster && ksCaster->GetTypeId() == TYPEID_PLAYER) {
                if (ksCaster->HasSpell(14176))
                    damage *= 1.09f;
                else if (ksCaster->HasSpell(14175))
                    damage *= 1.06f;
                else if (ksCaster->HasSpell(14174))
                    damage *= 1.03f;
            }
        }
    }
    
    // Spell 37224: This hack should be removed one day
    if (HasAuraEffect(37224) && spellProto && spellProto->SpellFamilyFlags == 0x1000000000LL && spellProto->SpellIconID == 2562)
        damage += 30;
    
    //You don't lose health from damage taken from another player while in a sanctuary
    //You still see it in the combat log though
    if(pVictim != this && GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() == TYPEID_PLAYER)
    {
        const AreaTableEntry *area = sAreaTableStore.LookupEntry(pVictim->GetAreaId());
        if(area && (area->flags & AREA_FLAG_SANCTUARY || (World::IsZoneSanctuary(area->ID))))       //sanctuary
            return 0;
    }

    // Handler for god command

    if(pVictim->GetTypeId() == TYPEID_PLAYER)
        if (pVictim->ToPlayer()->GetCommandStatus(CHEAT_GOD))
            return 0;

    //Script Event damage taken
    if( pVictim->GetTypeId()== TYPEID_UNIT && (pVictim->ToCreature())->IsAIEnabled )
    {
        // Set tagging
        if(!pVictim->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TAPPED) && !(pVictim->ToCreature())->IsPet())
        {
            //Set Loot
            switch(GetTypeId())
            {
                case TYPEID_PLAYER:
                {
                    (pVictim->ToCreature())->SetLootRecipient(this);
                    //Set tagged
                    (pVictim->ToCreature())->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TAPPED);
                    break;
                }
                case TYPEID_UNIT:
                {
                    if((this->ToCreature())->IsPet())
                    {
                        (pVictim->ToCreature())->SetLootRecipient(this->GetOwner());
                        (pVictim->ToCreature())->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TAPPED);
                    }
                    break;
                }
            }
        }
    }

    if (damagetype != NODAMAGE)
    {
       // interrupting auras with AURA_INTERRUPT_FLAG_DAMAGE before checking !damage (absorbed damage breaks that type of auras)
        if (spellProto)
        {
            if (!(spellProto->HasAttribute(SPELL_ATTR4_DAMAGE_DOESNT_BREAK_AURAS)))
                pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_DAMAGE, spellProto->Id);
        }
        else
            pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_DAMAGE, 0);
            
        pVictim->RemoveSpellbyDamageTaken(damage, spellProto ? spellProto->Id : 0);
    }

    if(!damage)
    {
        // Rage from physical damage received .
        if(cleanDamage && cleanDamage->damage && (damageSchoolMask & SPELL_SCHOOL_MASK_NORMAL) && pVictim->GetTypeId() == TYPEID_PLAYER && (pVictim->GetPowerType() == POWER_RAGE))
            (pVictim->ToPlayer())->RewardRage(cleanDamage->damage, 0, false);

        return 0;
    }

    //TC_LOG_DEBUG("FIXME","DealDamageStart");

    uint32 health = pVictim->GetHealth();
    //TC_LOG_DEBUG("FIXME","deal dmg:%d to health:%d ",damage,health);

    // duel ends when player has 1 or less hp
    bool duel_hasEnded = false;
    if(pVictim->GetTypeId() == TYPEID_PLAYER && (pVictim->ToPlayer())->duel && damage >= (health-1))
    {
        // prevent kill only if killed in duel and killed by opponent or opponent controlled creature
        if((pVictim->ToPlayer())->duel->opponent==this || (pVictim->ToPlayer())->duel->opponent->GetGUID() == GetOwnerGUID())
            damage = health-1;

        duel_hasEnded = true;
    }

    // Rage from Damage made (only from direct weapon damage)
    if( cleanDamage && damagetype==DIRECT_DAMAGE && this != pVictim && GetTypeId() == TYPEID_PLAYER && (GetPowerType() == POWER_RAGE))
    {
        uint32 weaponSpeedHitFactor;

        switch(cleanDamage->attackType)
        {
            case BASE_ATTACK:
            {
                if(cleanDamage->hitOutCome == MELEE_HIT_CRIT)
                    weaponSpeedHitFactor = uint32(GetAttackTime(cleanDamage->attackType)/1000.0f * 7);
                else
                    weaponSpeedHitFactor = uint32(GetAttackTime(cleanDamage->attackType)/1000.0f * 3.5f);

                (this->ToPlayer())->RewardRage(damage, weaponSpeedHitFactor, true);

                break;
            }
            case OFF_ATTACK:
            {
                if(cleanDamage->hitOutCome == MELEE_HIT_CRIT)
                    weaponSpeedHitFactor = uint32(GetAttackTime(cleanDamage->attackType)/1000.0f * 3.5f);
                else
                    weaponSpeedHitFactor = uint32(GetAttackTime(cleanDamage->attackType)/1000.0f * 1.75f);

                (this->ToPlayer())->RewardRage(damage, weaponSpeedHitFactor, true);

                break;
            }
            case RANGED_ATTACK:
            default:
                break;
        }
    }

    if(pVictim->GetTypeId() == TYPEID_PLAYER && GetTypeId() == TYPEID_PLAYER)
    {
        if(Battleground *bg = pVictim->ToPlayer()->GetBattleground())
        {
            Player* attacker = (this->ToPlayer());
            if(attacker != (pVictim->ToPlayer()))
                bg->UpdatePlayerScore(attacker, SCORE_DAMAGE_DONE, damage);
            
	        bg->UpdatePlayerScore(pVictim->ToPlayer(), SCORE_DAMAGE_TAKEN, damage);
        }
    }

    if (pVictim->GetTypeId() == TYPEID_UNIT && !(pVictim->ToCreature())->IsPet())
    {
        if(!(pVictim->ToCreature())->hasLootRecipient())
            (pVictim->ToCreature())->SetLootRecipient(this);

        if(GetCharmerOrOwnerPlayerOrPlayerItself())
            (pVictim->ToCreature())->LowerPlayerDamageReq(health < damage ?  health : damage);
    }
    
    if (health <= damage)
    {
        //TC_LOG_DEBUG("FIXME","DealDamage: victim just died");
        Kill(pVictim, durabilityLoss);
        
        //Hook for OnPVPKill Event
        if (pVictim->GetTypeId() == TYPEID_PLAYER && GetTypeId() == TYPEID_PLAYER)
        {
            Player *killer = ToPlayer();
            Player *killed = pVictim->ToPlayer();
            //    sScriptMgr->OnPVPKill(killer, killed);
        }
    }
    else                                                    // if (health <= damage)
    {
        //TC_LOG_DEBUG("FIXME","DealDamageAlive");

        pVictim->ModifyHealth(- (int32)damage);

        if(damagetype != DOT)
        {
            if(!GetVictim())
            /*{
                // if have target and damage pVictim just call AI reaction
                if(pVictim != GetVictim() && pVictim->GetTypeId()==TYPEID_UNIT && (pVictim->ToCreature())->IsAIEnabled)
                    (pVictim->ToCreature())->AI()->AttackedBy(this);
            }
            else*/
            {
                // if not have main target then attack state with target (including AI call)
                if(pVictim != GetVictim() && pVictim->GetTypeId()==TYPEID_UNIT && (pVictim->ToCreature())->IsAIEnabled)
                    (pVictim->ToCreature())->AI()->AttackedBy(this);

                //start melee attacks only after melee hit
                if(!ToCreature() || ToCreature()->GetReactState() != REACT_PASSIVE)
                    Attack(pVictim,(damagetype == DIRECT_DAMAGE));
            }
        }

        if(damagetype == DIRECT_DAMAGE || damagetype == SPELL_DIRECT_DAMAGE)
        {
            //TODO: This is from procflag, I do not know which spell needs this
            //Maim?
            //if (!spellProto || !(spellProto->AuraInterruptFlags&AURA_INTERRUPT_FLAG_DIRECT_DAMAGE))
                pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_DIRECT_DAMAGE, spellProto ? spellProto->Id : 0);
        }

        if (pVictim->GetTypeId() != TYPEID_PLAYER)
        {
            if(spellProto && IsDamageToThreatSpell(spellProto)) {
                //TC_LOG_INFO("DealDamage (IsDamageToThreatSpell), AddThreat : %f * 2 = %f",damage,damage*2);
                pVictim->AddThreat(this, damage*2, damageSchoolMask, spellProto);
            } else {
                float threat = damage * sSpellMgr->GetSpellThreatModPercent(spellProto);
                //TC_LOG_INFO("DealDamage, AddThreat : %f",threat);
                pVictim->AddThreat(this, threat, damageSchoolMask, spellProto);
            }
        }
        else                                                // victim is a player
        {
            // Rage from damage received
            if(this != pVictim && pVictim->GetPowerType() == POWER_RAGE)
            {
                uint32 rage_damage = damage + (cleanDamage ? cleanDamage->damage : 0);
                (pVictim->ToPlayer())->RewardRage(rage_damage, 0, false);
            }

            // random durability for items (HIT TAKEN)
            if (roll_chance_f(sWorld->GetRate(RATE_DURABILITY_LOSS_DAMAGE)))
            {
              EquipmentSlots slot = EquipmentSlots(GetMap()->urand(0,EQUIPMENT_SLOT_END-1));
                (pVictim->ToPlayer())->DurabilityPointLossForEquipSlot(slot);
            }
        }

        if(GetTypeId()==TYPEID_PLAYER)
        {
            // random durability for items (HIT DONE)
            if (roll_chance_f(sWorld->GetRate(RATE_DURABILITY_LOSS_DAMAGE)))
            {
              EquipmentSlots slot = EquipmentSlots(GetMap()->urand(0,EQUIPMENT_SLOT_END-1));
                (this->ToPlayer())->DurabilityPointLossForEquipSlot(slot);
            }
        }

        if (damagetype != NODAMAGE && damage)// && pVictim->GetTypeId() == TYPEID_PLAYER)
        {
            /*const SpellInfo *se = i->second->GetSpellInfo();
            next = i; ++next;
            if (spellProto && spellProto->Id == se->Id) // Not drop auras added by self
                continue;
            if( se->AuraInterruptFlags & AURA_INTERRUPT_FLAG_DAMAGE )
            {
                bool remove = true;
                if (se->ProcFlags & (1<<3))
                {
                    if (!roll_chance_i(se->ProcChance))
                        remove = false;
                }
                if (remove)
                {
                    pVictim->RemoveAurasDueToSpell(i->second->GetId());
                    // FIXME: this may cause the auras with proc chance to be rerolled several times
                    next = vAuras.begin();
                }
            }
        }*/

            if(pVictim != this && pVictim->GetTypeId() == TYPEID_PLAYER) // does not support creature push_back
            {
                if(damagetype != DOT)
                {
                    if(Spell* spell = pVictim->m_currentSpells[CURRENT_GENERIC_SPELL])
                    {
                        if(spell->getState() == SPELL_STATE_PREPARING)
                        {
                            uint32 interruptFlags = spell->m_spellInfo->InterruptFlags;
                            if(interruptFlags & SPELL_INTERRUPT_FLAG_DAMAGE)
                                pVictim->InterruptNonMeleeSpells(false);
                            else if(interruptFlags & SPELL_INTERRUPT_FLAG_PUSH_BACK)
                                spell->Delayed();
                        }
                    }

                    if(Spell* spell = pVictim->m_currentSpells[CURRENT_CHANNELED_SPELL])
                    {
                        if(spell->getState() == SPELL_STATE_CASTING)
                        {
                            uint32 channelInterruptFlags = spell->m_spellInfo->ChannelInterruptFlags;
                            if (((channelInterruptFlags & CHANNEL_FLAG_DELAY) != 0) && (damagetype != DOT))
                                spell->DelayedChannel();
                        }
                    }
                }
            }
        }

        // last damage from duel opponent
        if(duel_hasEnded)
        {
            assert(pVictim->GetTypeId()==TYPEID_PLAYER);
            Player *he = pVictim->ToPlayer();

            assert(he->duel);

            he->SetHealth(he->GetMaxHealth()/10.0f);

            he->duel->opponent->CombatStopWithPets(true);
            he->CombatStopWithPets(true);

            he->CastSpell(he, 7267, true);                  // beg
            he->DuelComplete(DUEL_WON);
        }
    }

    TC_LOG_DEBUG("FIXME","DealDamageEnd returned %d damage", damage);

    return damage;
}

void Unit::CastStop(uint32 except_spellid)
{
    for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
        if (m_currentSpells[i] && m_currentSpells[i]->m_spellInfo->Id!=except_spellid)
            InterruptSpell(i,false, false);
}

uint32 Unit::CastSpell(Unit* Victim, uint32 spellId, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId );

    if(!spellInfo)
    {
        TC_LOG_ERROR("FIXME","CastSpell: unknown spell id %i by caster: %s %u)", spellId,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_UNKNOWN;
    }

    return CastSpell(Victim,spellInfo,triggered,castItem,triggeredByAura, originalCaster);
}

uint32 Unit::CastSpell(Unit* Victim,SpellInfo const *spellInfo, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster, bool skipHit)
{
    if(!spellInfo)
    {
        TC_LOG_ERROR("spell","CastSpell: unknown spell by caster: %s %u)", (GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_UNKNOWN;
    }

    SpellCastTargets targets;
    uint32 targetMask = spellInfo->Targets;
    //if(targetMask & (TARGET_FLAG_UNIT|TARGET_FLAG_UNIT_MINIPET))
    for(int i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if(sSpellMgr->SpellTargetType[spellInfo->Effects[i].TargetA.GetTarget()] == TARGET_TYPE_UNIT_TARGET)
        {
            if(!Victim)
            {
                TC_LOG_ERROR("spell","CastSpell: spell id %i by caster: %s %u) does not have unit target", spellInfo->Id,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
                return SPELL_FAILED_BAD_TARGETS;
            }
            else
                break;
        }
    }
    targets.SetUnitTarget(Victim);

    if(targetMask & (TARGET_FLAG_SOURCE_LOCATION|TARGET_FLAG_DEST_LOCATION))
    {
        if(!Victim)
        {
            TC_LOG_ERROR("spell","CastSpell: spell id %i by caster: %s %u) does not have destination", spellInfo->Id,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
            return SPELL_FAILED_BAD_TARGETS;
        }
        targets.SetDestination(Victim);
    }

    /*if (castItem)
        TC_LOG_DEBUG("spell","WORLD: cast Item spellId - %i", spellInfo->Id);*/

    if(!originalCaster && triggeredByAura)
        originalCaster = triggeredByAura->GetCasterGUID();

    Spell *spell = new Spell(this, spellInfo, triggered, originalCaster, NULL, false );

    spell->m_CastItem = castItem;
    spell->m_skipHitCheck = skipHit;
    return spell->prepare(&targets, triggeredByAura);
}

uint32 Unit::CastCustomSpell(Unit* target, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    CustomSpellValues values;
    if(bp0) values.AddSpellMod(SPELLVALUE_BASE_POINT0, *bp0);
    if(bp1) values.AddSpellMod(SPELLVALUE_BASE_POINT1, *bp1);
    if(bp2) values.AddSpellMod(SPELLVALUE_BASE_POINT2, *bp2);
    return CastCustomSpell(spellId, values, target, triggered, castItem, triggeredByAura, originalCaster);
}

uint32 Unit::CastCustomSpell(uint32 spellId, SpellValueMod mod, uint32 value, Unit* target, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    CustomSpellValues values;
    values.AddSpellMod(mod, value);
    return CastCustomSpell(spellId, values, target, triggered, castItem, triggeredByAura, originalCaster);
}

uint32 Unit::CastCustomSpell(uint32 spellId, CustomSpellValues const &value, Unit* Victim, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId );
    if(!spellInfo)
    {
        TC_LOG_ERROR("spell","CastSpell: unknown spell id %i by caster: %s %u)", spellId,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_UNKNOWN;
    }

    SpellCastTargets targets;
    uint32 targetMask = spellInfo->Targets;

    //check unit target
    for(int i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if(sSpellMgr->SpellTargetType[spellInfo->Effects[i].TargetA.GetTarget()] == TARGET_TYPE_UNIT_TARGET)
        {
            if(!Victim)
            {
                TC_LOG_ERROR("spell","CastSpell: spell id %i by caster: %s %u) does not have unit target", spellInfo->Id,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
                return SPELL_FAILED_BAD_TARGETS;
            }
            else
                break;
        }
    }
    targets.SetUnitTarget(Victim);

    //check destination
    if(targetMask & (TARGET_FLAG_SOURCE_LOCATION|TARGET_FLAG_DEST_LOCATION))
    {
        if(!Victim)
        {
            TC_LOG_ERROR("spell","CastSpell: spell id %i by caster: %s %u) does not have destination", spellInfo->Id,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
            return SPELL_FAILED_BAD_TARGETS;
        }
        targets.SetDestination(Victim);
    }

    if(!originalCaster && triggeredByAura)
        originalCaster = triggeredByAura->GetCasterGUID();

    Spell *spell = new Spell(this, spellInfo, triggered, originalCaster );

    if(castItem)
    {
        //TC_LOG_DEBUG("spell","WORLD: cast Item spellId - %i", spellInfo->Id);
        spell->m_CastItem = castItem;
    }

    for(CustomSpellValues::const_iterator itr = value.begin(); itr != value.end(); ++itr)
        spell->SetSpellValue(itr->first, itr->second);

    return spell->prepare(&targets, triggeredByAura);
}

// used for scripting
uint32 Unit::CastSpell(float x, float y, float z, uint32 spellId, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId );

    if(!spellInfo)
    {
        TC_LOG_ERROR("spell","CastSpell(x,y,z): unknown spell id %i by caster: %s %u)", spellId,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_UNKNOWN;
    }

/*    if (castItem)
        TC_LOG_DEBUG("FIXME","WORLD: cast Item spellId - %i", spellInfo->Id); */

    if(!originalCaster && triggeredByAura)
        originalCaster = triggeredByAura->GetCasterGUID();

    Spell *spell = new Spell(this, spellInfo, triggered, originalCaster );

    SpellCastTargets targets;
    targets.SetDestination(x, y, z);
    spell->m_CastItem = castItem;
    return spell->prepare(&targets, triggeredByAura);
}

// used for scripting
uint32 Unit::CastSpell(GameObject *go, uint32 spellId, bool triggered, Item *castItem, Aura* triggeredByAura, uint64 originalCaster)
{
    if(!go)
        return SPELL_FAILED_UNKNOWN;

    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId );

    if(!spellInfo)
    {
        TC_LOG_ERROR("spell","CastSpell(x,y,z): unknown spell id %i by caster: %s %u)", spellId,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_UNKNOWN;
    }

    if(!(spellInfo->Targets & ( TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_UNIT_ENEMY)))
    {
        TC_LOG_ERROR("spell","CastSpell: spell id %i by caster: %s %u) is not gameobject spell", spellId,(GetTypeId()==TYPEID_PLAYER ? "player (GUID:" : "creature (Entry:"),(GetTypeId()==TYPEID_PLAYER ? GetGUIDLow() : GetEntry()));
        return SPELL_FAILED_UNKNOWN;
    }

  /*  if (castItem)
        TC_LOG_DEBUG("FIXME","WORLD: cast Item spellId - %i", spellInfo->Id); */

    if(!originalCaster && triggeredByAura)
        originalCaster = triggeredByAura->GetCasterGUID();

    Spell *spell = new Spell(this, spellInfo, triggered, originalCaster );

    SpellCastTargets targets;
    targets.SetGOTarget(go);
    spell->m_CastItem = castItem;
    return spell->prepare(&targets, triggeredByAura);
}

// Obsolete func need remove, here only for comotability vs another patches
uint32 Unit::SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage, bool isTriggeredSpell, bool useSpellDamage)
{
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellID);
    SpellNonMeleeDamage damageInfo(this, pVictim, spellInfo->Id, spellInfo->SchoolMask);
    damage = SpellDamageBonusDone(pVictim, spellInfo, damage, SPELL_DIRECT_DAMAGE);
    damage = pVictim->SpellDamageBonusTaken(this, spellInfo, damage, SPELL_DIRECT_DAMAGE);

    CalculateSpellDamageTaken(&damageInfo, damage, spellInfo);
    SendSpellNonMeleeDamageLog(&damageInfo);
    DealSpellDamage(&damageInfo, true);
    return damageInfo.damage;
}

void Unit::CalculateSpellDamageTaken(SpellNonMeleeDamage *damageInfo, int32 damage, SpellInfo const *spellInfo, WeaponAttackType attackType, bool crit)
{
    if (damage < 0)
        return;

    Unit *pVictim = damageInfo->target;
    if(!pVictim || !pVictim->IsAlive())
        return;

    SpellSchoolMask damageSchoolMask = SpellSchoolMask(damageInfo->schoolMask);
    uint32 crTypeMask = pVictim->GetCreatureTypeMask();
    // Check spell crit chance
    //bool crit = IsSpellCrit(pVictim, spellInfo, damageSchoolMask, attackType);
    bool blocked = false;
    // Per-school calc
    switch (spellInfo->DmgClass)
    {
        // Melee and Ranged Spells
        case SPELL_DAMAGE_CLASS_RANGED:
        case SPELL_DAMAGE_CLASS_MELEE:
        {
            // Physical Damage
            if ( damageSchoolMask & SPELL_SCHOOL_MASK_NORMAL )
            {
                // Get blocked status
                blocked = IsSpellBlocked(pVictim, spellInfo, attackType);
            }

            if (crit)
            {
                damageInfo->HitInfo|= SPELL_HIT_TYPE_CRIT;

                // Calculate crit bonus
                uint32 crit_bonus = damage;
                // Apply crit_damage bonus for melee spells
                if(Player* modOwner = GetSpellModOwner())
                    modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_CRIT_DAMAGE_BONUS, crit_bonus);
                damage += crit_bonus;

                // Apply SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE or SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE
                int32 critPctDamageMod=0;
                if(attackType == RANGED_ATTACK)
                    critPctDamageMod += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE);
                else
                {
                    critPctDamageMod += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE);
                    critPctDamageMod += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_DAMAGE_BONUS_MELEE);
                }
                // Increase crit damage from SPELL_AURA_MOD_CRIT_PERCENT_VERSUS
                critPctDamageMod += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_CRIT_PERCENT_VERSUS, crTypeMask);

                if (critPctDamageMod!=0)
                    damage = int32((damage) * float((100.0f + critPctDamageMod)/100.0f));

                // Resilience - reduce crit damage
                if (pVictim->GetTypeId()==TYPEID_PLAYER)
                    damage -= (pVictim->ToPlayer())->GetMeleeCritDamageReduction(damage);
            }
            // Spell weapon based damage CAN BE crit & blocked at same time
            if (blocked)
            {
                damageInfo->blocked = uint32(pVictim->GetShieldBlockValue());
                if (damage < damageInfo->blocked)
                    damageInfo->blocked = damage;
                damage-=damageInfo->blocked;
            }
        }
        break;
        // Magical Attacks
        case SPELL_DAMAGE_CLASS_NONE:
        case SPELL_DAMAGE_CLASS_MAGIC:
        {
            // If crit add critical bonus
            if (crit)
            {
                damageInfo->HitInfo|= SPELL_HIT_TYPE_CRIT;
                damage = SpellCriticalBonus(spellInfo, damage, pVictim);
                // Resilience - reduce crit damage
                if (pVictim->GetTypeId()==TYPEID_PLAYER && !(spellInfo->HasAttribute(SPELL_ATTR4_IGNORE_RESISTANCES)))
                    damage -= (pVictim->ToPlayer())->GetSpellCritDamageReduction(damage);
            }
        }
        break;
    }


    if( damageSchoolMask & SPELL_SCHOOL_MASK_NORMAL  && !spellInfo->HasAttribute(SPELL_ATTR_CU_IGNORE_ARMOR))
        damage = CalcArmorReducedDamage(pVictim, damage);

    // Calculate absorb resist
    if(damage > 0)
    {
        CalcAbsorbResist(pVictim, damageSchoolMask, SPELL_DIRECT_DAMAGE, damage, &damageInfo->absorb, &damageInfo->resist, (spellInfo ? spellInfo->Id : 0));
        damage-= damageInfo->absorb + damageInfo->resist;
    }
    else
        damage = 0;
        
    if (spellInfo && spellInfo->Id == 46576) {
        if (Aura* aur = pVictim->GetAura(46458, 0))
            damage = 300 * aur->GetStackAmount();
    }    
    
    damageInfo->damage = damage;
}

void Unit::DealSpellDamage(SpellNonMeleeDamage *damageInfo, bool durabilityLoss)
{
    if (damageInfo==0)
        return;

    Unit *pVictim = damageInfo->target;

    if(!pVictim)
        return;

    if (!pVictim->IsAlive() || pVictim->IsInFlight() || (pVictim->GetTypeId() == TYPEID_UNIT && (pVictim->ToCreature())->IsInEvadeMode()))
        return;

    SpellInfo const *spellProto = sSpellMgr->GetSpellInfo(damageInfo->SpellID);
    if (spellProto == NULL)
    {
        TC_LOG_ERROR("spell","Unit::DealSpellDamage have wrong damageInfo->SpellID: %u", damageInfo->SpellID);
        return;
    }

    //You don't lose health from damage taken from another player while in a sanctuary
    //You still see it in the combat log though
    if(pVictim != this && GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() == TYPEID_PLAYER)
    {
        const AreaTableEntry *area = sAreaTableStore.LookupEntry(pVictim->GetAreaId());
        if(area && area->flags & AREA_FLAG_SANCTUARY)                     //sanctuary
            return;
    }

    // update at damage Judgement aura duration that applied by attacker at victim
    if(damageInfo->damage && spellProto->Id == 35395)
    {
        AuraMap& vAuras = pVictim->GetAuras();
        for(AuraMap::iterator itr = vAuras.begin(); itr != vAuras.end(); ++itr)
        {
            SpellInfo const *spellInfo = (*itr).second->GetSpellInfo();
            if(spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && spellInfo->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT))
            {
                (*itr).second->SetAuraDuration((*itr).second->GetAuraMaxDuration());
                (*itr).second->UpdateAuraDuration();
            }
        }
    }
    // Call default DealDamage
    CleanDamage cleanDamage(damageInfo->cleanDamage, BASE_ATTACK, MELEE_HIT_NORMAL);
    DealDamage(pVictim, damageInfo->damage, &cleanDamage, SPELL_DIRECT_DAMAGE, SpellSchoolMask(damageInfo->schoolMask), spellProto, durabilityLoss);
}

//TODO for melee need create structure as in
void Unit::CalculateMeleeDamage(Unit *pVictim, uint32 damage, CalcDamageInfo *damageInfo, WeaponAttackType attackType)
{
    damageInfo->attacker         = this;
    damageInfo->target           = pVictim;
    damageInfo->damageSchoolMask = GetMeleeDamageSchoolMask();
    damageInfo->attackType       = attackType;
    damageInfo->damage           = 0;
    damageInfo->cleanDamage      = 0;
    damageInfo->absorb           = 0;
    damageInfo->resist           = 0;
    damageInfo->blocked_amount   = 0;

    damageInfo->TargetState      = 0;
    damageInfo->HitInfo          = 0;
    damageInfo->procAttacker     = PROC_FLAG_NONE;
    damageInfo->procVictim       = PROC_FLAG_NONE;
    damageInfo->procEx           = PROC_EX_NONE;
    damageInfo->hitOutCome       = MELEE_HIT_EVADE;

    if(!pVictim)
        return;
    if(!this->IsAlive() || !pVictim->IsAlive())
        return;

    // Select HitInfo/procAttacker/procVictim flag based on attack type
    switch (attackType)
    {
        case BASE_ATTACK:
            damageInfo->procAttacker = PROC_FLAG_SUCCESSFUL_MELEE_HIT;
            damageInfo->procVictim   = PROC_FLAG_TAKEN_MELEE_HIT;
            damageInfo->HitInfo      = HITINFO_NORMALSWING2;
            break;
        case OFF_ATTACK:
            damageInfo->procAttacker = PROC_FLAG_SUCCESSFUL_MELEE_HIT | PROC_FLAG_SUCCESSFUL_OFFHAND_HIT;
            damageInfo->procVictim   = PROC_FLAG_TAKEN_MELEE_HIT;//|PROC_FLAG_TAKEN_OFFHAND_HIT // not used
            damageInfo->HitInfo = HITINFO_LEFTSWING;
            break;
        case RANGED_ATTACK:
            damageInfo->procAttacker = PROC_FLAG_SUCCESSFUL_RANGED_HIT;
            damageInfo->procVictim   = PROC_FLAG_TAKEN_RANGED_HIT;
            damageInfo->HitInfo = 0x08;// test
            break;
        default:
            break;
    }

    // Physical Immune check
    if(damageInfo->target->IsImmunedToDamage(SpellSchoolMask(damageInfo->damageSchoolMask),true))
    {
       damageInfo->HitInfo       |= HITINFO_NORMALSWING;
       damageInfo->TargetState    = VICTIMSTATE_IS_IMMUNE;

       damageInfo->procEx |=PROC_EX_IMMUNE;
       damageInfo->damage         = 0;
       damageInfo->cleanDamage    = 0;
       return;
    }
    damage += CalculateDamage(damageInfo->attackType, false, NULL, damageInfo->target);
    // Add melee damage bonus
    MeleeDamageBonus(damageInfo->target, &damage, damageInfo->attackType);
    // Calculate armor reduction
    damageInfo->damage = (damageInfo->damageSchoolMask & SPELL_SCHOOL_MASK_NORMAL) ? CalcArmorReducedDamage(damageInfo->target, damage) : damage;
    damageInfo->cleanDamage += damage - damageInfo->damage;

    damageInfo->hitOutCome = RollMeleeOutcomeAgainst(damageInfo->target, damageInfo->attackType, (SpellSchoolMask)damageInfo->damageSchoolMask);

    // Disable parry or dodge for ranged attack
    if(damageInfo->attackType == RANGED_ATTACK)
    {
        if (damageInfo->hitOutCome == MELEE_HIT_PARRY) damageInfo->hitOutCome = MELEE_HIT_NORMAL;
        if (damageInfo->hitOutCome == MELEE_HIT_DODGE) damageInfo->hitOutCome = MELEE_HIT_MISS;
    }

    switch(damageInfo->hitOutCome)
    {
        case MELEE_HIT_EVADE:
        {
            damageInfo->HitInfo    |= HITINFO_MISS|HITINFO_SWINGNOHITSOUND;
            damageInfo->TargetState = VICTIMSTATE_EVADES;

            damageInfo->procEx|=PROC_EX_EVADE;
            damageInfo->damage = 0;
            damageInfo->cleanDamage = 0;
            return;
        }
        case MELEE_HIT_MISS:
        {
            damageInfo->HitInfo    |= HITINFO_MISS;
            damageInfo->TargetState = VICTIMSTATE_NORMAL;

            damageInfo->procEx|=PROC_EX_MISS;
            damageInfo->damage = 0;
            damageInfo->cleanDamage = 0;
            break;
        }
        case MELEE_HIT_NORMAL:
            damageInfo->TargetState = VICTIMSTATE_NORMAL;
            damageInfo->procEx|=PROC_EX_NORMAL_HIT;
            break;
        case MELEE_HIT_CRIT:
        {
            damageInfo->HitInfo     |= HITINFO_CRITICALHIT;
            damageInfo->TargetState  = VICTIMSTATE_NORMAL;

            damageInfo->procEx|=PROC_EX_CRITICAL_HIT;
            // Crit bonus calc
            damageInfo->damage += damageInfo->damage;
            int32 mod=0;
            // Apply SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE or SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE
            if(damageInfo->attackType == RANGED_ATTACK)
                mod += damageInfo->target->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE);
            else
            {
                mod += damageInfo->target->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE);
                mod += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_DAMAGE_BONUS_MELEE);
            }

            uint32 crTypeMask = damageInfo->target->GetCreatureTypeMask();

            // Increase crit damage from SPELL_AURA_MOD_CRIT_PERCENT_VERSUS
            mod += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_CRIT_PERCENT_VERSUS, crTypeMask);
            if (mod!=0)
                damageInfo->damage = int32((damageInfo->damage) * float((100.0f + mod)/100.0f));

            // Resilience - reduce crit damage
            if (pVictim->GetTypeId()==TYPEID_PLAYER)
            {
                uint32 resilienceReduction = (pVictim->ToPlayer())->GetMeleeCritDamageReduction(damageInfo->damage);
                damageInfo->damage      -= resilienceReduction;
                damageInfo->cleanDamage += resilienceReduction;
            }
            break;
        }
        case MELEE_HIT_PARRY:
            damageInfo->TargetState  = VICTIMSTATE_PARRY;
            damageInfo->procEx|=PROC_EX_PARRY;
            damageInfo->cleanDamage += damageInfo->damage;
            damageInfo->damage = 0;
            break;

        case MELEE_HIT_DODGE:
            damageInfo->TargetState  = VICTIMSTATE_DODGE;
            damageInfo->procEx|=PROC_EX_DODGE;
            damageInfo->cleanDamage += damageInfo->damage;
            damageInfo->damage = 0;
            break;
        case MELEE_HIT_BLOCK:
        {
            damageInfo->TargetState = VICTIMSTATE_NORMAL;
            damageInfo->procEx|=PROC_EX_BLOCK;
            damageInfo->blocked_amount = damageInfo->target->GetShieldBlockValue();
            if (damageInfo->blocked_amount >= damageInfo->damage)
            {
                damageInfo->TargetState = VICTIMSTATE_BLOCKS;
                damageInfo->blocked_amount = damageInfo->damage;
            }
            damageInfo->damage      -= damageInfo->blocked_amount;
            damageInfo->cleanDamage += damageInfo->blocked_amount;
            break;
        }
        case MELEE_HIT_GLANCING:
        {
            damageInfo->HitInfo     |= HITINFO_GLANCING;
            damageInfo->TargetState  = VICTIMSTATE_NORMAL;
            damageInfo->procEx|=PROC_EX_NORMAL_HIT;
            int32 leveldif = int32(pVictim->GetLevel()) - int32(GetLevel());
            if (leveldif > 3) leveldif = 3;
            float reducePercent = 1 - leveldif * 0.1f;
            damageInfo->cleanDamage += damageInfo->damage-uint32(reducePercent *  damageInfo->damage);
            damageInfo->damage   = uint32(reducePercent *  damageInfo->damage);
            break;
        }
        case MELEE_HIT_CRUSHING:
        {
            damageInfo->HitInfo     |= HITINFO_CRUSHING;
            damageInfo->TargetState  = VICTIMSTATE_NORMAL;
            damageInfo->procEx|=PROC_EX_NORMAL_HIT;
            // 150% normal damage
            damageInfo->damage += (damageInfo->damage / 2);
            break;
        }
        default:

            break;
    }

    // Calculate absorb resist
    if(int32(damageInfo->damage) > 0)
    {
        damageInfo->procVictim |= PROC_FLAG_TAKEN_ANY_DAMAGE;
        // Calculate absorb & resists
        CalcAbsorbResist(damageInfo->target, SpellSchoolMask(damageInfo->damageSchoolMask), DIRECT_DAMAGE, damageInfo->damage, &damageInfo->absorb, &damageInfo->resist, 0);
        damageInfo->damage-=damageInfo->absorb + damageInfo->resist;
        if (damageInfo->absorb)
        {
            damageInfo->HitInfo|=HITINFO_ABSORB;
            damageInfo->procEx|=PROC_EX_ABSORB;
            damageInfo->procVictim |= PROC_FLAG_HAD_DAMAGE_BUT_ABSORBED;
        }
        if (damageInfo->resist)
            damageInfo->HitInfo|=HITINFO_RESIST;

    }
    else // Umpossible get negative result but....
        damageInfo->damage = 0;
}

void Unit::DealMeleeDamage(CalcDamageInfo *damageInfo, bool durabilityLoss)
{
    if (!damageInfo) return;
    Unit *pVictim = damageInfo->target;

    if(!pVictim)
        return;

    if (!pVictim->IsAlive() || pVictim->IsInFlight() || (pVictim->GetTypeId() == TYPEID_UNIT && (pVictim->ToCreature())->IsInEvadeMode()))
        return;

    //You don't lose health from damage taken from another player while in a sanctuary
    //You still see it in the combat log though
    if(pVictim != this && GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() == TYPEID_PLAYER)
    {
        const AreaTableEntry *area = sAreaTableStore.LookupEntry(pVictim->GetAreaId());
        if(area && area->flags & 0x800)                     //sanctuary
            return;
    }

    // Call default DealDamage
    CleanDamage cleanDamage(damageInfo->cleanDamage,damageInfo->attackType,damageInfo->hitOutCome);
    DealDamage(pVictim, damageInfo->damage, &cleanDamage, DIRECT_DAMAGE, SpellSchoolMask(damageInfo->damageSchoolMask), NULL, durabilityLoss);

    // If this is a creature and it attacks from behind it has a probability to daze it's victim
    if( (damageInfo->hitOutCome==MELEE_HIT_CRIT || damageInfo->hitOutCome==MELEE_HIT_CRUSHING || damageInfo->hitOutCome==MELEE_HIT_NORMAL || damageInfo->hitOutCome==MELEE_HIT_GLANCING) &&
        GetTypeId() != TYPEID_PLAYER && !(this->ToCreature())->GetCharmerOrOwnerGUID() && !pVictim->HasInArc(M_PI, this)
        && (pVictim->GetTypeId() == TYPEID_PLAYER || !(pVictim->ToCreature())->IsWorldBoss()))
    {
        // -probability is between 0% and 40%
        // 20% base chance
        float Probability = 20;

        //there is a newbie protection, at level 10 just 7% base chance; assuming linear function
        if( pVictim->GetLevel() < 30 )
            Probability = 0.65f*pVictim->GetLevel()+0.5;

        uint32 VictimDefense=pVictim->GetDefenseSkillValue();
        uint32 AttackerMeleeSkill=GetUnitMeleeSkill();

        Probability *= AttackerMeleeSkill/(float)VictimDefense;

        if(Probability > 40)
            Probability = 40;

        if(roll_chance_f(Probability))
            CastSpell(pVictim, 1604, true);
    }

    // update at damage Judgement aura duration that applied by attacker at victim
    if(damageInfo->damage)
    {
        AuraMap& vAuras = pVictim->GetAuras();
        for(AuraMap::iterator itr = vAuras.begin(); itr != vAuras.end(); ++itr)
        {
            SpellInfo const *spellInfo = (*itr).second->GetSpellInfo();
            if( spellInfo->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT) && spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && ((*itr).second->GetCasterGUID() == GetGUID()) && spellInfo->Id != 41461) //Gathios judgement of blood (can't seem to find a general rule to avoid this hack)
            {
                (*itr).second->SetAuraDuration((*itr).second->GetAuraMaxDuration());
                (*itr).second->UpdateAuraDuration();
            }
        }
    }

    if(GetTypeId() == TYPEID_PLAYER)
        (this->ToPlayer())->CastItemCombatSpell(pVictim, damageInfo->attackType, damageInfo->procVictim, damageInfo->procEx);

    // Do effect if any damage done to target
    if (damageInfo->procVictim & PROC_FLAG_TAKEN_ANY_DAMAGE)
    {
        // victim's damage shield
        std::set<Aura*> alreadyDone;
        uint32 removedAuras = pVictim->m_removedAurasCount;
        AuraList const& vDamageShields = pVictim->GetAurasByType(SPELL_AURA_DAMAGE_SHIELD);
        for(AuraList::const_iterator i = vDamageShields.begin(), next = vDamageShields.begin(); i != vDamageShields.end(); i = next)
        {
           next++;
           if (alreadyDone.find(*i) == alreadyDone.end())
           {
               alreadyDone.insert(*i);
               uint32 damage=(*i)->GetModifier()->m_amount;
               SpellInfo const *spellProto = sSpellMgr->GetSpellInfo((*i)->GetId());
               if(!spellProto)
                   continue;
               //Calculate absorb resist ??? no data in opcode for this possibly unable to absorb or resist?
               //uint32 absorb;
               //uint32 resist;
               //CalcAbsorbResist(pVictim, SpellSchools(spellProto->School), SPELL_DIRECT_DAMAGE, damage, &absorb, &resist);
               //damage-=absorb + resist;

               WorldPacket data(SMSG_SPELLDAMAGESHIELD,(8+8+4+4));
               data << uint64(pVictim->GetGUID());
               data << uint64(GetGUID());
               data << uint32(spellProto->SchoolMask);
               data << uint32(damage);
               pVictim->SendMessageToSet(&data, true );

               pVictim->DealDamage(this, damage, 0, SPELL_DIRECT_DAMAGE, spellProto->GetSchoolMask(), spellProto, true);

               if (pVictim->m_removedAurasCount > removedAuras)
               {
                   removedAuras = pVictim->m_removedAurasCount;
                   next = vDamageShields.begin();
               }
           }
        }
    }
}

void Unit::SetEmoteState(uint32 state)
{
    SetUInt32Value(UNIT_NPC_EMOTESTATE, state);
}

void Unit::HandleEmoteCommand(uint32 emote_id)
{
    WorldPacket data( SMSG_EMOTE, 12 );
    data << emote_id << GetGUID();
    WPAssert(data.size() == 12);

    SendMessageToSet(&data, true);
}

uint32 Unit::CalcArmorReducedDamage(Unit* pVictim, const uint32 damage)
{
    if(sWorld->getConfig(CONFIG_DEBUG_DISABLE_ARMOR))
        return damage;

    uint32 newdamage = 0;
    float armor = pVictim->GetArmor();
    // Ignore enemy armor by SPELL_AURA_MOD_TARGET_RESISTANCE aura
    armor += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_RESISTANCE, SPELL_SCHOOL_MASK_NORMAL);

    if (armor<0.0f) armor=0.0f;

    float tmpvalue = 0.0f;
    if(GetLevel() <= 59)                                    //Level 1-59
        tmpvalue = armor / (armor + 400.0f + 85.0f * GetLevel());
    else if(GetLevel() < 70)                                //Level 60-69
        tmpvalue = armor / (armor - 22167.5f + 467.5f * GetLevel());
    else                                                    //Level 70+
        tmpvalue = armor / (armor + 10557.5f);

    if(tmpvalue < 0.0f)
        tmpvalue = 0.0f;
    if(tmpvalue > 0.75f)
        tmpvalue = 0.75f;
    newdamage = uint32(damage - (damage * tmpvalue));

    return (newdamage > 1) ? newdamage : 1;
}

void Unit::CalcAbsorbResist(Unit *pVictim,SpellSchoolMask schoolMask, DamageEffectType damagetype, const uint32 damage, uint32 *absorb, uint32 *resist, uint32 spellId)
{
    if(!pVictim || !pVictim->IsAlive() || !damage)
        return;

    SpellInfo const* spellProto = sSpellMgr->GetSpellInfo(spellId);

    // Magic damage, check for resists
    if(  (schoolMask & SPELL_SCHOOL_MASK_SPELL)                                          // Is magic and not holy
         && (  !spellProto 
               || !sSpellMgr->IsBinaryMagicResistanceSpell(spellProto) 
               || !(spellProto->HasAttribute(SPELL_ATTR4_IGNORE_RESISTANCES)) 
               || !(spellProto->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT)) ) // Non binary spell (this was already handled in DoSpellHitOnUnit) (see Spell::IsBinaryMagicResistanceSpell for more)
      )              
    {
        // Get base victim resistance for school
        int32 resistance = (float)pVictim->GetResistance(GetFirstSchoolInMask(schoolMask));
        // Ignore resistance by self SPELL_AURA_MOD_TARGET_RESISTANCE aura (aka spell penetration)
        resistance += (float)GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_RESISTANCE, schoolMask);
        // Resistance can't be negative
        
        if(resistance < 0) 
            resistance = 0;

        float fResistance = (float)resistance * (float)(0.15f / GetLevel()); //% from 0.0 to 1.0
     
        //incompressive magic resist. Can't seem to find the proper rule for this... meanwhile let's have use an approximation
        int32 levelDiff = pVictim->GetLevel() - GetLevel();
        if(levelDiff > 0)
            fResistance += (int32) ((levelDiff<3?levelDiff:3) * (0.006f)); //Cap it a 3 level diff, probably not blizz but this doesn't change anything at HL and is A LOT less boring for people pexing

        // Resistance can't be more than 75%
        if (fResistance > 0.75f)
            fResistance = 0.75f;

        uint32 ran = GetMap()->urand(0, 100);
        uint32 faq[4] = {24,6,4,6};
        uint8 m = 0;
        float Binom = 0.0f;
        for (uint8 i = 0; i < 4; i++)
        {
            Binom += 2400 *( powf(fResistance, i) * powf( (1-fResistance), (4-i)))/faq[i];
            if (ran > Binom )
                ++m;
            else
                break;
        }
        if (damagetype == DOT && m == 4)
            *resist += uint32(damage - 1);
        else
            *resist += uint32(damage * m / 4);
        if(*resist > damage)
            *resist = damage;
    }
    else
        *resist = 0;

    int32 RemainingDamage = damage - *resist;

    AuraList const& vOverrideScripts = pVictim->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for(AuraList::const_iterator i = vOverrideScripts.begin(), next; i != vOverrideScripts.end(); i = next)
    {
        next = i; ++next;

        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            break;

        // Shadow of Death - set cheat death on cooldown
        if ((*i)->GetSpellInfo()->Id == 40251 && pVictim->GetHealth() <= RemainingDamage)
        {
            (pVictim->ToPlayer())->AddSpellCooldown(31231,0,time(NULL)+60);
            break;
        }
    }

    // Need to remove expired auras after
    bool expiredExists = false;

    // absorb without mana cost
    int32 reflectDamage = 0;
    Aura* reflectAura = NULL;
    AuraList const& vSchoolAbsorb = pVictim->GetAurasByType(SPELL_AURA_SCHOOL_ABSORB);
    for(AuraList::const_iterator i = vSchoolAbsorb.begin(); i != vSchoolAbsorb.end() && RemainingDamage > 0; ++i)
    {
        int32 absorbAmount = (*i)->GetModifierValue();

        // should not happen....
        if (absorbAmount <=0)
        {
            expiredExists = true;
            continue;
        }

        if (((*i)->GetModifier()->m_miscvalue & schoolMask)==0)
            continue;

        // Cheat Death
        if((*i)->GetSpellInfo()->SpellFamilyName==SPELLFAMILY_ROGUE && (*i)->GetSpellInfo()->SpellIconID == 2109)
        {
            if ((pVictim->ToPlayer())->HasSpellCooldown(31231))
                continue;
            if (pVictim->GetHealth() <= RemainingDamage)
            {
                int32 chance = absorbAmount;
                if (roll_chance_i(chance))
                {
                    pVictim->CastSpell(pVictim,31231,true);
                    (pVictim->ToPlayer())->AddSpellCooldown(31231,0,time(NULL)+60);

                    // with health > 10% lost health until health==10%, in other case no losses
                    uint32 health10 = pVictim->GetMaxHealth()/10;
                    RemainingDamage = pVictim->GetHealth() > health10 ? pVictim->GetHealth() - health10 : 0;
                }
            }
            continue;
        }
        
        // Shadow of Death
        if ((*i)->GetSpellInfo()->Id == 40251)
        {
            if (pVictim->GetHealth() <= RemainingDamage)
            {
                RemainingDamage = 0;
                // Will be cleared next update
                (*i)->SetAuraDuration(0);
            }
            continue;
        }

        int32 currentAbsorb;

        //Reflective Shield
        if ((pVictim != this))
        {
            if(Unit* caster = (*i)->GetCaster())
            {
                if ((*i)->GetSpellInfo()->SpellFamilyName == SPELLFAMILY_PRIEST && (*i)->GetSpellInfo()->SpellFamilyFlags == 0x1)
                {
                    AuraList const& vOverRideCS = caster->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for(AuraList::const_iterator k = vOverRideCS.begin(); k != vOverRideCS.end(); ++k)
                    {
                        switch((*k)->GetModifier()->m_miscvalue)
                        {
                            case 5065:                          // Rank 1
                            case 5064:                          // Rank 2
                            case 5063:                          // Rank 3
                            case 5062:                          // Rank 4
                            case 5061:                          // Rank 5
                            {
                                if(RemainingDamage >= absorbAmount)
                                     reflectDamage = absorbAmount * (*k)->GetModifier()->m_amount/100;
                                else
                                    reflectDamage = (*k)->GetModifier()->m_amount * RemainingDamage/100;
                                reflectAura = *i;

                            } break;
                            default: break;
                        }

                        if(reflectDamage)
                            break;
                    }
                }
                // Reflective Shield, NPC
                else if ((*i)->GetSpellInfo()->Id == 41475)
                {
                    if(RemainingDamage >= absorbAmount)
                        reflectDamage = absorbAmount * 0.5f;
                    else
                        reflectDamage = RemainingDamage * 0.5f;
                    reflectAura = *i;
                }
            }
        }

        if (RemainingDamage >= absorbAmount)
        {
            currentAbsorb = absorbAmount;
            expiredExists = true;
        }
        else
            currentAbsorb = RemainingDamage;

        absorbAmount -= currentAbsorb;
        RemainingDamage -= currentAbsorb;
        
        (*i)->SetModifierValue(absorbAmount);
    }
    // do not cast spells while looping auras; auras can get invalid otherwise
    if (reflectDamage)
        pVictim->CastCustomSpell(this, 33619, &reflectDamage, NULL, NULL, true, NULL, reflectAura);

    // Remove all expired absorb auras
    if (expiredExists)
    {
        for (AuraList::const_iterator i = vSchoolAbsorb.begin(); i != vSchoolAbsorb.end(); )
        {
            Aura *aur = (*i);
            ++i;
            if (aur->GetModifier()->m_amount <= 0)
            {
                uint32 removedAuras = pVictim->m_removedAurasCount;
                pVictim->RemoveAurasDueToSpell( aur->GetId() );
                if (removedAuras + 1 < pVictim->m_removedAurasCount)
                    i = vSchoolAbsorb.begin();
            }
        }
    }

    // absorb by mana cost
    AuraList const& vManaShield = pVictim->GetAurasByType(SPELL_AURA_MANA_SHIELD);
    for(AuraList::const_iterator i = vManaShield.begin(), next; i != vManaShield.end() && RemainingDamage > 0; i = next)
    {
        next = i; ++next;
        int32 absorbAmount = (*i)->GetModifierValue();

        // check damage school mask
        if(((*i)->GetModifier()->m_miscvalue & schoolMask)==0)
            continue;

        int32 currentAbsorb;
        if (RemainingDamage >= absorbAmount)
            currentAbsorb = absorbAmount;
        else
            currentAbsorb = RemainingDamage;

        float manaMultiplier = (*i)->GetSpellInfo()->Effects[(*i)->GetEffIndex()].ValueMultiplier;
        if(Player *modOwner = GetSpellModOwner())
            modOwner->ApplySpellMod((*i)->GetId(), SPELLMOD_MULTIPLE_VALUE, manaMultiplier);

        if(manaMultiplier)
        {
            int32 maxAbsorb = int32(pVictim->GetPower(POWER_MANA) / manaMultiplier);
            if (currentAbsorb > maxAbsorb)
                currentAbsorb = maxAbsorb;
        }

        absorbAmount -= currentAbsorb;
        if(absorbAmount <= 0)
        {
            pVictim->RemoveAurasDueToSpell((*i)->GetId());
            next = vManaShield.begin();
        }

        int32 manaReduction = int32(currentAbsorb * manaMultiplier);
        pVictim->ApplyPowerMod(POWER_MANA, manaReduction, false);

        RemainingDamage -= currentAbsorb;

        (*i)->SetModifierValue(absorbAmount);
    }

    // only split damage if not damaging yourself
    if(pVictim != this)
    {
        //split damage flag
        AuraList const& vSplitDamageFlat = pVictim->GetAurasByType(SPELL_AURA_SPLIT_DAMAGE_FLAT);
        for(AuraList::const_iterator i = vSplitDamageFlat.begin(), next; i != vSplitDamageFlat.end() && RemainingDamage >= 0; i = next)
        {
            next = i; ++next;

            // check damage school mask
            if(((*i)->GetModifier()->m_miscvalue & schoolMask)==0)
                continue;

            // Damage can be splitted only if aura has an alive caster
            Unit *caster = (*i)->GetCaster();
            if(!caster || caster == pVictim || !caster->IsInWorld() || !caster->IsAlive())
                continue;

            int32 currentAbsorb;
            if (RemainingDamage >= (*i)->GetModifier()->m_amount)
                currentAbsorb = (*i)->GetModifier()->m_amount;
            else
                currentAbsorb = RemainingDamage;

            RemainingDamage -= currentAbsorb;

            // check if caster is immune to damage
            if (caster->IsImmunedToDamage(schoolMask))
            {
                pVictim->SendSpellMiss(caster, (*i)->GetSpellInfo()->Id, SPELL_MISS_IMMUNE);
                continue;
            }

            SendSpellNonMeleeDamageLog(caster, (*i)->GetSpellInfo()->Id, currentAbsorb, schoolMask, 0, 0, false, 0, false);

            CleanDamage cleanDamage = CleanDamage(currentAbsorb, BASE_ATTACK, MELEE_HIT_NORMAL);
            DealDamage(caster, currentAbsorb, &cleanDamage, DOT, schoolMask, (*i)->GetSpellInfo(), false);
        }

        //split damage percent
        AuraList const& vSplitDamagePct = pVictim->GetAurasByType(SPELL_AURA_SPLIT_DAMAGE_PCT);
        for(AuraList::const_iterator i = vSplitDamagePct.begin(), next; i != vSplitDamagePct.end() && RemainingDamage >= 0; i = next)
        {
            next = i; ++next;

            // check damage school mask
            if(((*i)->GetModifier()->m_miscvalue & schoolMask)==0)
                continue;

            // Damage can be splitted only if aura has an alive caster
            Unit *caster = (*i)->GetCaster();
            if(!caster || caster == pVictim || !caster->IsInWorld() || !caster->IsAlive())
                continue;

            int32 splitted = int32(RemainingDamage * (*i)->GetModifier()->m_amount / 100.0f);

            RemainingDamage -= splitted;

            // check if caster is immune to damage
            if (caster->IsImmunedToDamage(schoolMask))
            {
                pVictim->SendSpellMiss(caster, (*i)->GetSpellInfo()->Id, SPELL_MISS_IMMUNE);
                continue;
            }

            SendSpellNonMeleeDamageLog(caster, (*i)->GetSpellInfo()->Id, splitted, schoolMask, 0, 0, false, 0, false);

            CleanDamage cleanDamage = CleanDamage(splitted, BASE_ATTACK, MELEE_HIT_NORMAL);
            DealDamage(caster, splitted, &cleanDamage, DOT, schoolMask, (*i)->GetSpellInfo(), false);
        }
    }

    *absorb = damage - RemainingDamage - *resist;
}

void Unit::AttackerStateUpdate(Unit *pVictim, WeaponAttackType attType, bool extra )
{
    if (ToPlayer() && ToPlayer()->isSpectator())
        return;

    if((!extra && HasUnitState(UNIT_STATE_CANNOT_AUTOATTACK)) || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED) )
        return;

    if (!pVictim->IsAlive())
        return;

    if(attType == BASE_ATTACK && sWorld->getConfig(CONFIG_DEBUG_DISABLE_MAINHAND))
        return;

    if ((attType == BASE_ATTACK || attType == OFF_ATTACK) && !IsWithinLOSInMap(pVictim))
        return;
        
    CombatStart(pVictim);
    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_MELEE_ATTACK);
    
    if (pVictim->GetTypeId() == TYPEID_UNIT && (pVictim->ToCreature())->IsAIEnabled)
        (pVictim->ToCreature())->AI()->AttackedBy(this);

    if (attType != BASE_ATTACK && attType != OFF_ATTACK)
        return;                                             // ignore ranged case

    // melee attack spell casted at main hand attack only
    if (!extra && attType == BASE_ATTACK && m_currentSpells[CURRENT_MELEE_SPELL])
    {
        m_currentSpells[CURRENT_MELEE_SPELL]->cast();
        return;
    }

    CalcDamageInfo damageInfo;
    CalculateMeleeDamage(pVictim, 0, &damageInfo, attType);
    // Send log damage message to client
    SendAttackStateUpdate(&damageInfo);
    DealMeleeDamage(&damageInfo,true);
    ProcDamageAndSpell(damageInfo.target, damageInfo.procAttacker, damageInfo.procVictim, damageInfo.procEx, damageInfo.damage, damageInfo.attackType);

    if (GetTypeId() == TYPEID_PLAYER)
        TC_LOG_DEBUG("entities.unit","AttackerStateUpdate: (Player) %u attacked %u (TypeId: %u) for %u dmg, absorbed %u, blocked %u, resisted %u.",
            GetGUIDLow(), pVictim->GetGUIDLow(), pVictim->GetTypeId(), damageInfo.damage, damageInfo.absorb, damageInfo.blocked_amount, damageInfo.resist);
    else
        TC_LOG_DEBUG("entities.unit","AttackerStateUpdate: (NPC)    %u attacked %u (TypeId: %u) for %u dmg, absorbed %u, blocked %u, resisted %u.",
            GetGUIDLow(), pVictim->GetGUIDLow(), pVictim->GetTypeId(), damageInfo.damage, damageInfo.absorb, damageInfo.blocked_amount, damageInfo.resist);

    // HACK: Warrior enrage not losing procCharges when dealing melee damage
    if (GetTypeId() == TYPEID_PLAYER) {
        uint32 enrageId = 0;
        if (HasAuraEffect(12880))
            enrageId = 12880;
        else if (HasAuraEffect(14201))
            enrageId = 14201;
        else if (HasAuraEffect(14202))
            enrageId = 14202;
        else if (HasAuraEffect(14203))
            enrageId = 14203;
        else if (HasAuraEffect(14204))
            enrageId = 14204;
            
        if (enrageId) {
            if (Aura* enrageAura = GetAuraByCasterSpell(enrageId, GetGUID())) {
                enrageAura->SetAuraProcCharges(enrageAura->GetAuraProcCharges()-1);
            }
        }
    }
}

MeleeHitOutcome Unit::RollMeleeOutcomeAgainst(const Unit *pVictim, WeaponAttackType attType, SpellSchoolMask schoolMask) const
{
    // This is only wrapper

    // Miss chance based on melee
    //float miss_chance = MeleeMissChanceCalc(pVictim, attType);
    float miss_chance = MeleeSpellMissChance(pVictim, attType, int32(GetWeaponSkillValue(attType,pVictim)) - int32(pVictim->GetDefenseSkillValue(this)), 0);

    // Critical hit chance
    float crit_chance = GetUnitCriticalChance(attType, pVictim);

    // stunned target cannot dodge and this is checked in GetUnitDodgeChance() (returned 0 in this case)
    float dodge_chance = pVictim->GetUnitDodgeChance();
    float block_chance = pVictim->GetUnitBlockChance();
    float parry_chance = pVictim->GetUnitParryChance(); 

    // Useful if want to specify crit & miss chances for melee, else it could be removed
    TC_LOG_DEBUG ("entities.unit","MELEE OUTCOME: miss %f crit %f dodge %f parry %f block %f", miss_chance,crit_chance,dodge_chance,parry_chance,block_chance);

    return RollMeleeOutcomeAgainst(pVictim, attType, int32(crit_chance*100), int32(miss_chance*100), int32(dodge_chance*100),int32(parry_chance*100),int32(block_chance*100), false);
}

MeleeHitOutcome Unit::RollMeleeOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType, int32 crit_chance, int32 miss_chance, int32 dodge_chance, int32 parry_chance, int32 block_chance, bool SpellCasted ) const
{
    if(pVictim->GetTypeId()==TYPEID_UNIT && (pVictim->ToCreature())->IsInEvadeMode())
        return MELEE_HIT_EVADE;

    int32 attackerMaxSkillValueForLevel = GetMaxSkillValueForLevel(pVictim);
    int32 victimMaxSkillValueForLevel = pVictim->GetMaxSkillValueForLevel(this);

    int32 attackerWeaponSkill = GetWeaponSkillValue(attType,pVictim);
    int32 victimDefenseSkill = pVictim->GetDefenseSkillValue(this);

    // bonus from skills is 0.04%
    int32    skillBonus  = 4 * ( attackerWeaponSkill - victimMaxSkillValueForLevel );
    int32    skillBonus2 = 4 * ( attackerMaxSkillValueForLevel - victimDefenseSkill );
    int32    sum = 0, tmp = 0;
    int32    roll = GetMap()->urand (0, 10000);

    TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: skill bonus of %d for attacker", skillBonus);
    TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: rolled %d, miss %d, dodge %d, parry %d, block %d, crit %d",
        roll, miss_chance, dodge_chance, parry_chance, block_chance, crit_chance);

    tmp = miss_chance;

    if (tmp > 0 && roll < (sum += tmp ))
    {
        TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: MISS");
        return MELEE_HIT_MISS;
    }

    // always crit against a sitting target (except 0 crit chance)
    if( pVictim->GetTypeId() == TYPEID_PLAYER && crit_chance > 0 && !pVictim->IsStandState() )
    {
        TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: CRIT (sitting victim)");
        return MELEE_HIT_CRIT;
    }

    // Dodge chance

    // only players can't dodge if attacker is behind
    if (pVictim->GetTypeId() == TYPEID_PLAYER && !pVictim->HasInArc(M_PI,this))
    {
        TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: attack came from behind and victim was a player.");
    }
    else
    {
        if(dodge_chance > 0) // check if unit _can_ dodge
        {
            int32 real_dodge_chance = dodge_chance;
            real_dodge_chance -= skillBonus;

            // Reduce dodge chance by attacker expertise rating
            if (GetTypeId() == TYPEID_PLAYER)
                real_dodge_chance -= int32((this->ToPlayer())->GetExpertiseDodgeOrParryReduction(attType)*100);
            // Modify dodge chance by attacker SPELL_AURA_MOD_COMBAT_RESULT_CHANCE
            real_dodge_chance+= GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_COMBAT_RESULT_CHANCE, VICTIMSTATE_DODGE)*100;
            real_dodge_chance+= GetTotalAuraModifier(SPELL_AURA_MOD_ENEMY_DODGE)*100;

            if (   (real_dodge_chance > 0)                                        
                && roll < (sum += real_dodge_chance))
            {
                TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: DODGE <%d, %d)", sum-real_dodge_chance, sum);
                return MELEE_HIT_DODGE;
            }
        }
    }

    // parry & block chances

    // check if attack comes from behind, nobody can parry or block if attacker is behind
    if (!pVictim->HasInArc(M_PI,this))
    {
        TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: attack came from behind.");
    }
    else
    {
        if(parry_chance > 0) // check if unit _can_ parry
        {
            int32 real_parry_chance = parry_chance;
            real_parry_chance -= skillBonus;

            // Reduce parry chance by attacker expertise rating
            if (GetTypeId() == TYPEID_PLAYER)
                real_parry_chance -= int32((this->ToPlayer())->GetExpertiseDodgeOrParryReduction(attType)*100);

            if (   (real_parry_chance > 0)     
                && (roll < (sum += real_parry_chance)))
            {
                TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: PARRY <%d, %d)", sum-real_parry_chance, sum);
                ((Unit*)pVictim)->HandleParryRush();
                return MELEE_HIT_PARRY;
            }
        }

        if(block_chance > 0)
        {
            int32 real_block_chance = block_chance;
            if(block_chance > 0) // check if unit _can_ block
                real_block_chance -= skillBonus;

            if (   (real_block_chance > 0)      
                && (roll < (sum += real_block_chance)))
            {
                // Critical chance
                int16 blocked_crit_chance = crit_chance + skillBonus2;
                if ( GetTypeId() == TYPEID_PLAYER && SpellCasted && blocked_crit_chance > 0 )
                {
                    if ( roll_chance_i(blocked_crit_chance/100))
                    {
                        TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: BLOCKED CRIT");
                        return MELEE_HIT_BLOCK_CRIT;
                    }
                }
                TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: BLOCK <%d, %d)", sum-blocked_crit_chance, sum);
                return MELEE_HIT_BLOCK;
            }
        }
    }

    // Critical chance
    int32 real_crit_chance = crit_chance + skillBonus2;

    if (real_crit_chance > 0 && roll < (sum += real_crit_chance))
    {
        TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: CRIT <%d, %d)", sum-real_crit_chance, sum);
        if(GetTypeId() == TYPEID_UNIT && ((this->ToCreature())->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_CRIT))
            TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: CRIT DISABLED)");
        else
            return MELEE_HIT_CRIT;
    }

    // Max 40% chance to score a glancing blow against mobs that are higher level (can do only players and pets and not with ranged weapon)
    if( attType != RANGED_ATTACK && !SpellCasted &&
        (GetTypeId() == TYPEID_PLAYER || (this->ToCreature())->IsPet()) &&
        pVictim->GetTypeId() != TYPEID_PLAYER && !(pVictim->ToCreature())->IsPet() &&
        GetLevel() < pVictim->GetLevelForTarget(this) )
    {
        // cap possible value (with bonuses > max skill)
        int32 skill = attackerWeaponSkill;
        int32 maxskill = attackerMaxSkillValueForLevel;
        skill = (skill > maxskill) ? maxskill : skill;

        tmp = (10 + (victimDefenseSkill - skill)) * 100;
        tmp = tmp > 4000 ? 4000 : tmp;
        if (roll < (sum += tmp))
        {
            TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: GLANCING <%d, %d)", sum-4000, sum);
            return MELEE_HIT_GLANCING;
        }
    }

    if(GetTypeId()!=TYPEID_PLAYER && !((this->ToCreature())->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_CRUSH) && !(this->ToCreature())->IsPet() && !SpellCasted /*Only autoattack can be crushing blow*/ )
    {
        // mobs can score crushing blows if they're 3 or more levels above victim
        // or when their weapon skill is 15 or more above victim's defense skill
        tmp = victimDefenseSkill;
        int32 tmpmax = victimMaxSkillValueForLevel;
        // having defense above your maximum (from items, talents etc.) has no effect
        tmp = tmp > tmpmax ? tmpmax : tmp;
        // tmp = mob's level * 5 - player's current defense skill
        tmp = attackerMaxSkillValueForLevel - tmp;
        if(tmp >= 15)
        {
            // add 2% chance per lacking skill point, min. is 15%
            tmp = tmp * 200 - 1500;
            if (roll < (sum += tmp))
            {
                TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: CRUSHING <%d, %d)", sum-tmp, sum);
                return MELEE_HIT_CRUSHING;
            }
        }
    }

    TC_LOG_DEBUG ("entities.unit","RollMeleeOutcomeAgainst: NORMAL");
    return MELEE_HIT_NORMAL;
}

uint32 Unit::CalculateDamage(WeaponAttackType attType, bool normalized, SpellInfo const* spellProto, Unit* target)
{
    float min_damage, max_damage;

    if (normalized && GetTypeId()==TYPEID_PLAYER) {
        (this->ToPlayer())->CalculateMinMaxDamage(attType, normalized, true, min_damage, max_damage, target);
    }
    else
    {
        switch (attType)
        {
            case RANGED_ATTACK:
                min_damage = GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE);
                max_damage = GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE);
                break;
            case BASE_ATTACK:
                min_damage = GetFloatValue(UNIT_FIELD_MINDAMAGE);
                max_damage = GetFloatValue(UNIT_FIELD_MAXDAMAGE);
                break;
            case OFF_ATTACK:
                min_damage = GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE);
                max_damage = GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE);
                break;
                // Just for good manner
            default:
                min_damage = 0.0f;
                max_damage = 0.0f;
                break;
        }
    }

    if (min_damage > max_damage)
    {
        std::swap(min_damage,max_damage);
    }

    if(max_damage == 0.0f)
        max_damage = 5.0f;

    return GetMap()->urand((uint32)min_damage, (uint32)max_damage);
}

float Unit::CalculateLevelPenalty(SpellInfo const* spellProto) const
{
    if(spellProto->SpellLevel <= 0)
        return 1.0f;

    float LvlPenalty = 0.0f;

    if(spellProto->SpellLevel < 20)
        LvlPenalty = 20.0f - spellProto->SpellLevel * 3.75f;
    float LvlFactor = (float(spellProto->SpellLevel) + 6.0f) / float(GetLevel());
    if(LvlFactor > 1.0f)
        LvlFactor = 1.0f;

    return (100.0f - LvlPenalty) * LvlFactor / 100.0f;
}

//LK OK
void Unit::SendMeleeAttackStart(Unit* pVictim)
{
    WorldPacket data( SMSG_ATTACKSTART, 8 + 8 );
    data << uint64(GetGUID());
    data << uint64(pVictim->GetGUID());
    SendMessageToSet(&data, true);
    TC_LOG_DEBUG("entities.unit", "WORLD: Sent SMSG_ATTACKSTART" );
}

//LK OK
void Unit::SendMeleeAttackStop(Unit* victim)
{
    WorldPacket data( SMSG_ATTACKSTOP, (8+8+4));
    data << GetPackGUID();
    if (victim)
        data << victim->GetPackGUID();
    else
        data << uint8(0);

    data << uint32(0);                                      //! Can also take the value 0x01, which seems related to updating rotation
    SendMessageToSet(&data, true);
    if (victim)
        TC_LOG_TRACE("entities.unit", "%s %u stopped attacking %s %u", (GetTypeId() == TYPEID_PLAYER ? "Player" : "Creature"), GetGUIDLow(), (victim->GetTypeId() == TYPEID_PLAYER ? "player" : "creature"), victim->GetGUIDLow());
    else
        TC_LOG_TRACE("entities.unit", "%s %u stopped attacking", (GetTypeId() == TYPEID_PLAYER ? "Player" : "Creature"), GetGUIDLow());
}

bool Unit::IsSpellBlocked(Unit *pVictim, SpellInfo const *spellProto, WeaponAttackType attackType)
{
    if (pVictim->HasInArc(M_PI,this))
    {
       float blockChance = pVictim->GetUnitBlockChance();

       float fAttackerSkill = GetWeaponSkillValue(attackType, pVictim)*0.04;
       float fDefenserSkill = pVictim->GetDefenseSkillValue(this)*0.04;

       blockChance += (fDefenserSkill - fAttackerSkill);

       if (blockChance < 0.0)
           blockChance = 0.0;

       if (roll_chance_f(blockChance))
           return true;
    }
    return false;
}

// Melee based spells can be miss, parry or dodge on this step
// Crit or block - determined on damage calculation phase! (and can be both in some time)
float Unit::MeleeSpellMissChance(const Unit *pVictim, WeaponAttackType attType, int32 skillDiff, uint32 spellId) const
{
    // Calculate hit chance (more correct for chance mod)
    int32 HitChance;

    // PvP - PvE melee chances
    /*int32 lchance = pVictim->GetTypeId() == TYPEID_PLAYER ? 5 : 7;
    int32 leveldif = pVictim->GetLevelForTarget(this) - GetLevelForTarget(pVictim);
    if(leveldif < 3)
        HitChance = 95 - leveldif;
    else
        HitChance = 93 - (leveldif - 2) * lchance;*/
    if (spellId || attType == RANGED_ATTACK || !HaveOffhandWeapon() || (GetTypeId() == TYPEID_UNIT && ToCreature()->IsWorldBoss()))
        HitChance = 95.0f;
    else
        HitChance = 76.0f;

    // Hit chance depends from victim auras
    if(attType == RANGED_ATTACK)
        HitChance += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE);
    else
        HitChance += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE);

    // Spellmod from SPELLMOD_RESIST_MISS_CHANCE
    if(spellId)
    {
        if(Player *modOwner = GetSpellModOwner())
            modOwner->ApplySpellMod(spellId, SPELLMOD_RESIST_MISS_CHANCE, HitChance);
    }

    // Miss = 100 - hit
    float miss_chance= 100.0f - HitChance;

    // Bonuses from attacker aura and ratings
    if (attType == RANGED_ATTACK)
        miss_chance -= m_modRangedHitChance;
    else
        miss_chance -= m_modMeleeHitChance;

    // bonus from skills is 0.04%
    //miss_chance -= skillDiff * 0.04f;
    int32 diff = -skillDiff;
    if(pVictim->GetTypeId()==TYPEID_PLAYER)
        miss_chance += diff > 0 ? diff * 0.04 : diff * 0.02;
    else
        miss_chance += diff > 10 ? 2 + (diff - 10) * 0.4 : diff * 0.1;

    // Limit miss chance from 0 to 60%
    if (miss_chance < 0.0f)
        return 0.0f;
    if (miss_chance > 60.0f)
        return 60.0f;
    return miss_chance;
}


int32 Unit::GetMechanicResistChance(const SpellInfo *spell)
{
    if(!spell)
        return 0;

    int32 resist_mech = 0;
    for(int eff = 0; eff < 3; ++eff)
    {
        if(spell->Effects[eff].Effect == 0)
           break;
        int32 effect_mech = GetEffectMechanic(spell, eff);
        /*if (spell->Effects[eff].ApplyAuraName == SPELL_AURA_MOD_TAUNT && (GetEntry() == 24882 || GetEntry() == 23576))
            return int32(1);*/
        if (effect_mech)
        {
            int32 temp = GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_MECHANIC_RESISTANCE, effect_mech);
            if (resist_mech < temp)
                resist_mech = temp;
        }
    }

    return resist_mech;
}

// Melee based spells hit result calculations
SpellMissInfo Unit::MeleeSpellHitResult(Unit *pVictim, SpellInfo const *spell)
{
    // Spells with SPELL_ATTR3_IGNORE_HIT_RESULT will additionally fully ignore
    // resist and deflect chances
    if (spell->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT))
        return SPELL_MISS_NONE;

    WeaponAttackType attType = BASE_ATTACK;

    if (spell->DmgClass == SPELL_DAMAGE_CLASS_RANGED)
        attType = RANGED_ATTACK;

    // bonus from skills is 0.04% per skill Diff
    int32 attackerWeaponSkill = int32(GetWeaponSkillValue(attType, pVictim));
    int32 skillDiff = attackerWeaponSkill - int32(pVictim->GetMaxSkillValueForLevel(this));
    int32 fullSkillDiff = attackerWeaponSkill - int32(pVictim->GetDefenseSkillValue(this));

    bool isCasting = pVictim->IsNonMeleeSpellCast(false);
    bool lostControl = pVictim->HasUnitState(UNIT_STATE_LOST_CONTROL);

    bool canParry = !isCasting && !lostControl && !(spell->Attributes & SPELL_ATTR0_IMPOSSIBLE_DODGE_PARRY_BLOCK);
    bool canDodge = !isCasting && !lostControl && !(spell->Attributes & SPELL_ATTR0_IMPOSSIBLE_DODGE_PARRY_BLOCK);
    bool canBlock = spell->HasAttribute(SPELL_ATTR3_BLOCKABLE_SPELL) && !isCasting && !lostControl && !(spell->Attributes & SPELL_ATTR0_IMPOSSIBLE_DODGE_PARRY_BLOCK);
    bool canMiss = true;

    if (Player* player = ToPlayer())
    {
        Item *tmpitem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        if (!tmpitem || !tmpitem->GetTemplate()->Block)
            canBlock = false;
    }

    // Creature has un-blockable attack info
    if (GetTypeId() == TYPEID_UNIT)
    {
        if ( ((Creature*)this)->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_BLOCK
            || ToCreature()->IsTotem())
            canBlock = false;
    }

    uint32 roll = GetMap()->urand (0, 10000);
    uint32 tmp;
    if (canMiss)
    {
        uint32 missChance = uint32(MeleeSpellMissChance(pVictim, attType, fullSkillDiff, spell->Id)*100.0f);

        // Roll miss
        tmp = missChance;
        if (roll < tmp)
            return SPELL_MISS_MISS;
    }

    // Chance resist mechanic
    int32 resist_chance = pVictim->GetMechanicResistChance(spell)*100;
    
    // Reduce spell hit chance for dispel mechanic spells from victim SPELL_AURA_MOD_DISPEL_RESIST
    if (IsDispelSpell(spell))
        resist_chance += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_DISPEL_RESIST)*100;

    tmp += resist_chance;
    if (roll < tmp)
        return SPELL_MISS_RESIST;

    // Handle ranged attacks
    if (attType == RANGED_ATTACK) {
        // Wand attacks can't miss
        if (spell->GetCategory() == 351)
               return SPELL_MISS_NONE;

        // Other ranged attacks cannot be parried or dodged
        // Can be blocked under suitable circumstances
        canParry = false;
        canDodge = false;
    }

    // Check for attack from behind
    if (!pVictim->HasInArc(M_PI, this) || spell->HasAttribute(SPELL_ATTR2_BEHIND_TARGET))
    {
        // Can`t dodge from behind in PvP (but its possible in PvE)
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            canDodge = false;
        // Can`t parry or block
        canParry = false;
        canBlock = false;
    }

    // Rogue talent`s cant be dodged
    AuraList const& mCanNotBeDodge = GetAurasByType(SPELL_AURA_IGNORE_COMBAT_RESULT);
    for (AuraList::const_iterator i = mCanNotBeDodge.begin(); i != mCanNotBeDodge.end(); ++i)
    {
        if ((*i)->GetModifier()->m_miscvalue == VICTIMSTATE_DODGE)       // can't be dodged rogue finishing move
        {
            if (spell->SpellFamilyName == SPELLFAMILY_ROGUE && (spell->SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE_FINISHING_MOVE))
            {
                canDodge = false;
                break;
            }
        }
    }

    if (canDodge)
    {
        // Roll dodge
        int32 dodgeChance = int32(pVictim->GetUnitDodgeChance()*100.0f) - skillDiff * 4;
        // Reduce enemy dodge chance by SPELL_AURA_MOD_COMBAT_RESULT_CHANCE
        dodgeChance += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_COMBAT_RESULT_CHANCE, VICTIMSTATE_DODGE) * 100;
        dodgeChance += GetTotalAuraModifier(SPELL_AURA_MOD_ENEMY_DODGE) * 100;

        // Reduce dodge chance by attacker expertise rating
        if (GetTypeId() == TYPEID_PLAYER)
            dodgeChance -= int32((this->ToPlayer())->GetExpertiseDodgeOrParryReduction(attType) * 100.0f);
        if (dodgeChance < 0)
            dodgeChance = 0;

        tmp += dodgeChance;
        if (roll < tmp)
            return SPELL_MISS_DODGE;
    }

    if (canParry)
    {
        // Roll parry
        int32 parryChance = int32(pVictim->GetUnitParryChance()*100.0f) - skillDiff * 4;
        // Reduce parry chance by attacker expertise rating
        if (GetTypeId() == TYPEID_PLAYER)
            parryChance -= int32((this->ToPlayer())->GetExpertiseDodgeOrParryReduction(attType) * 100.0f);
        // Can`t parry from behind
        if (parryChance < 0)
            parryChance = 0;

        tmp += parryChance;
        if (roll < tmp)
            return SPELL_MISS_PARRY;
    }

    if (canBlock)
    {
        if (pVictim->HasInArc(M_PI, this))
        {
            float blockChance = pVictim->GetUnitBlockChance();
            blockChance -= (0.04*fullSkillDiff);

            if (blockChance < 0.0)
                blockChance = 0.0;

            tmp += blockChance * 100;
            if (roll < tmp)
                return SPELL_MISS_BLOCK;
        }
    }

    return SPELL_MISS_NONE;
}

/*  From 0.0f to 1.0f. Used for binaries spell resistance.
http://www.wowwiki.com/Formulas:Magical_resistance#Magical_Resistances
*/
float Unit::GetAverageSpellResistance(Unit* caster, SpellSchoolMask damageSchoolMask)
{
    if(!caster)
        return 0;

    int32 resistance = GetResistance(GetFirstSchoolInMask(damageSchoolMask));
    resistance += caster->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_RESISTANCE, damageSchoolMask); // spell penetration

    if(resistance < 0)
        resistance = 0;

    float resistChance = (0.75f * resistance / (caster->GetLevel() * 5));
    if(resistChance > 0.75f)
        resistChance = 0.75f;

    return resistChance;
}

SpellMissInfo Unit::MagicSpellHitResult(Unit *pVictim, SpellInfo const *spell, Item* castItem)
{
    // Can`t miss on dead target (on skinning for example)
    if (!pVictim->IsAlive() || spell->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT))
        return SPELL_MISS_NONE;
        
    // Always 1% resist chance. Send this as SPELL_MISS_MISS (this is not BC blizzlike, this was changed in WotLK).
    uint32 rand = GetMap()->urand(0,10000);
    if (rand > 9900)
        return SPELL_MISS_MISS;

    SpellSchoolMask schoolMask = spell->GetSchoolMask();

    // PvP - PvE spell misschances per leveldif > 2
    int32 lchance = pVictim->GetTypeId() == TYPEID_PLAYER ? 7 : 11;
    int32 myLevel = int32(GetLevelForTarget(pVictim));
    // some spells using items should take another caster level into account ("Unreliable against targets higher than...")
    if(castItem) 
    {
        if(spell->MaxLevel != 0 && myLevel > spell->MaxLevel)
            myLevel = spell->MaxLevel;
        else if(castItem->GetTemplate()->RequiredLevel && castItem->GetTemplate()->RequiredLevel < 40) //not sure about this but this is based on wowhead.com/item=1404 and seems probable to me
            myLevel = (myLevel > 60) ? 60: myLevel;
    }
    int32 targetLevel = int32(pVictim->GetLevelForTarget(this));
    int32 leveldiff = targetLevel - myLevel;

    // Base hit chance from attacker and victim levels
    int32 modHitChance;
    if(leveldiff < 3)
        modHitChance = 96 - leveldiff;
    else
        modHitChance = 94 - (leveldiff - 2) * lchance;

    // Spellmod from SPELLMOD_RESIST_MISS_CHANCE
    if(Player *modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spell->Id, SPELLMOD_RESIST_MISS_CHANCE, modHitChance);

    // Increase from attacker SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT auras
    modHitChance += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT, schoolMask);

    // Chance hit from victim SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE auras
    modHitChance += pVictim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE, schoolMask);

    // Reduce spell hit chance for Area of effect spells from victim SPELL_AURA_MOD_AOE_AVOIDANCE aura
    if (IsAreaOfEffectSpell(spell))
        modHitChance -= pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_AOE_AVOIDANCE);

    // Reduce spell hit chance for dispel mechanic spells from victim SPELL_AURA_MOD_DISPEL_RESIST
    if (IsDispelSpell(spell))
        modHitChance -= pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_DISPEL_RESIST);

    // Chance resist mechanic (select max value from every mechanic spell effect)
    int32 resist_chance = pVictim->GetMechanicResistChance(spell);
    modHitChance -= resist_chance;

    // Chance resist debuff
    modHitChance -= pVictim->GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_DEBUFF_RESISTANCE, int32(spell->Dispel));

    int32 HitChance = modHitChance * 100;
    // Increase hit chance from attacker SPELL_AURA_MOD_SPELL_HIT_CHANCE and attacker ratings
    HitChance += int32(m_modSpellHitChance*100.0f);

    // Decrease hit chance from victim rating bonus
    if (pVictim->GetTypeId()==TYPEID_PLAYER)
        HitChance -= int32((pVictim->ToPlayer())->GetRatingBonusValue(CR_HIT_TAKEN_SPELL)*100.0f);

    // Hack - Always have 99% on taunts for Nalorakk & Brutallus.
    if ((spell->Effects[0].ApplyAuraName == SPELL_AURA_MOD_TAUNT || spell->Effects[1].ApplyAuraName == SPELL_AURA_MOD_TAUNT || spell->Effects[2].ApplyAuraName == SPELL_AURA_MOD_TAUNT)
        && (pVictim->GetEntry() == 24882 || pVictim->GetEntry() == 23576)) 
    {
        HitChance = 9900;
    }

    // Always have a minimal 1% chance
    if (HitChance <  100) HitChance =  100;

    // Final Result //
    bool resist = rand > HitChance;
    if (resist)
        return SPELL_MISS_RESIST;

    return SPELL_MISS_NONE;
}

// Calculate spell hit result can be:
// Every spell can: Evade/Immune/Reflect/Sucesful hit
// For melee based spells:
//   Miss
//   Dodge
//   Parry
// For spells
//   Resist
SpellMissInfo Unit::SpellHitResult(Unit *pVictim, SpellInfo const *spell, bool CanReflect, Item* castItem)
{
    if (ToCreature() && ToCreature()->IsTotem())
        if (Unit *owner = GetOwner())
            return owner->SpellHitResult(pVictim, spell, CanReflect, castItem);

    // Return evade for units in evade mode
    if (pVictim->GetTypeId()==TYPEID_UNIT && (pVictim->ToCreature())->IsInEvadeMode())
        return SPELL_MISS_EVADE;

    // Check for immune (use charges)
    if (pVictim->IsImmunedToSpell(spell,true))
        return SPELL_MISS_IMMUNE;

    // Check for immune (use charges)
    if (!spell->HasAttribute(SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY)  && pVictim->IsImmunedToDamage(spell->GetSchoolMask(),true))
        return SPELL_MISS_IMMUNE;

    // All positive spells can`t miss
    // TODO: client not show miss log for this spells - so need find info for this in dbc and use it!
    if (spell->IsPositive(!IsFriendlyTo(pVictim)))
        return SPELL_MISS_NONE;

    if(this == pVictim)
        return SPELL_MISS_NONE;
        
    //TC_LOG_INFO("SpellHitResult1 %u", spell->Id);

    // Try victim reflect spell
    if (CanReflect)
    {
        //TC_LOG_INFO("SpellHitResult2 %u", spell->Id);
        int32 reflectchance = pVictim->GetTotalAuraModifier(SPELL_AURA_REFLECT_SPELLS);
        //TC_LOG_INFO("SpellHitResult3 %u - reflect chance %d", spell->Id, reflectchance);
        Unit::AuraList const& mReflectSpellsSchool = pVictim->GetAurasByType(SPELL_AURA_REFLECT_SPELLS_SCHOOL);
        for(Unit::AuraList::const_iterator i = mReflectSpellsSchool.begin(); i != mReflectSpellsSchool.end(); ++i) {
            //TC_LOG_INFO("For1 %u %u", spell->Id, (*i)->GetId());
            if((*i)->GetModifier()->m_miscvalue & spell->GetSchoolMask()) {
                //TC_LOG_INFO("For2 %u %u %u %u %d", spell->Id, (*i)->GetId(), (*i)->GetModifier()->m_miscvalue, spell->GetSchoolMask(), (*i)->GetModifierValue());
                reflectchance = (*i)->GetModifierValue();
            }
        }
        //TC_LOG_INFO("SpellHitResult4 %u - reflect chance %d", spell->Id, reflectchance);
        if (reflectchance > 0 && roll_chance_i(reflectchance))
        {
            // Start triggers for remove charges if need (trigger only for victim, and mark as active spell)
            ProcDamageAndSpell(pVictim, PROC_FLAG_NONE, PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT, PROC_EX_REFLECT, 1, BASE_ATTACK, spell);
            // FIXME: Add a flag on unit itself, not to setRemoveReflect if unit is already flagged for it (prevent infinite delay on reflect lolz)
            if (Spell* sp = m_currentSpells[CURRENT_CHANNELED_SPELL])
                sp->setRemoveReflect();
            else if (Spell* sp = m_currentSpells[CURRENT_GENERIC_SPELL])
                sp->setRemoveReflect();
            return SPELL_MISS_REFLECT;
        }
    }

    //Check magic resistance for binaries spells (see IsBinaryMagicResistanceSpell(...) for more details). This check is not rolled inside attack table.
    if(    sSpellMgr->IsBinaryMagicResistanceSpell(spell)
        && !(spell->HasAttribute(SPELL_ATTR4_IGNORE_RESISTANCES)) 
        && !(spell->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT))  )
    {
        float random = (float)rand()/(float)RAND_MAX;
        float resistChance = pVictim->GetAverageSpellResistance(this,(SpellSchoolMask)spell->SchoolMask);
        if(resistChance > random)
            return SPELL_MISS_RESIST;
        //no else, the binary spell can still be resisted in the next check
    }

    switch (spell->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_RANGED:
        case SPELL_DAMAGE_CLASS_MELEE:
            return MeleeSpellHitResult(pVictim, spell);
        case SPELL_DAMAGE_CLASS_NONE:
            if (spell->SchoolMask & SPELL_SCHOOL_MASK_SPELL)
                return MagicSpellHitResult(pVictim, spell, castItem);
            else
                return SPELL_MISS_NONE;
        case SPELL_DAMAGE_CLASS_MAGIC:
            return MagicSpellHitResult(pVictim, spell, castItem);
    }

    return SPELL_MISS_NONE;
}

uint32 Unit::GetDefenseSkillValue(Unit const* target) const
{
    if(GetTypeId() == TYPEID_PLAYER)
    {
        // in PvP use full skill instead current skill value
        uint32 value = (target && target->GetTypeId() == TYPEID_PLAYER)
            ? (this->ToPlayer())->GetMaxSkillValue(SKILL_DEFENSE)
            : (this->ToPlayer())->GetSkillValue(SKILL_DEFENSE);
        value += uint32((this->ToPlayer())->GetRatingBonusValue(CR_DEFENSE_SKILL));
        return value;
    }
    else
        return GetUnitMeleeSkill(target);
}

float Unit::GetUnitDodgeChance() const
{
    if (HasUnitState(UNIT_STATE_LOST_CONTROL))
        return 0.0f;
    if( GetTypeId() == TYPEID_PLAYER )
        return GetFloatValue(PLAYER_DODGE_PERCENTAGE);
    else
    {
        if(((Creature const*)this)->IsTotem())
            return 0.0f;
        else
        {
            float dodge = 5.0f;
            dodge += GetTotalAuraModifier(SPELL_AURA_MOD_DODGE_PERCENT);
            return dodge > 0.0f ? dodge : 0.0f;
        }
    }
}

float Unit::GetUnitParryChance() const
{
    if (IsNonMeleeSpellCast(false) || HasUnitState(UNIT_STATE_LOST_CONTROL))
        return 0.0f;

    float chance = 0.0f;

    if(GetTypeId() == TYPEID_PLAYER)
    {
        Player const* player = (Player const*)this;
        if(player->CanParry() )
        {
            Item *tmpitem = player->GetWeaponForAttack(BASE_ATTACK,true);
            if(!tmpitem)
                tmpitem = player->GetWeaponForAttack(OFF_ATTACK,true);

            if(tmpitem)
                chance = GetFloatValue(PLAYER_PARRY_PERCENTAGE);
        }
    }
    else if(GetTypeId() == TYPEID_UNIT)
    {
        if(ToCreature()->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_PARRY
           || ToCreature()->IsTotem())
            chance = 0.0f;
        else if(ToCreature()->IsWorldBoss()) // Add some parry chance for bosses. Nobody seems to knows the exact rule but it's somewhere around 14%.
            chance = 13.0f;
        else if(GetCreatureType() != CREATURE_TYPE_BEAST)
        {
            chance = 5.0f;
            chance += GetTotalAuraModifier(SPELL_AURA_MOD_PARRY_PERCENT);
        }

    }

    return chance > 0.0f ? chance : 0.0f;
}

float Unit::GetUnitBlockChance() const
{
    if ( IsNonMeleeSpellCast(false) || IsCCed())
        return 0.0f;

    if(GetTypeId() == TYPEID_PLAYER)
    {
        Player const* player = (Player const*)this;
        if(player->CanBlock() )
            return GetFloatValue(PLAYER_BLOCK_PERCENTAGE);

        // is player but has no block ability or no not broken shield equipped
        return 0.0f;
    }
    else
    {
        if(   (ToCreature()->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_BLOCK)
           || (ToCreature()->IsTotem()) )
            return 0.0f;
        else
        {
            float block = 5.0f;
            block += GetTotalAuraModifier(SPELL_AURA_MOD_BLOCK_PERCENT);
            return block > 0.0f ? block : 0.0f;
        }
    }
}

float Unit::GetUnitCriticalChance(WeaponAttackType attackType, const Unit *pVictim) const
{
    float crit;

    if(GetTypeId() == TYPEID_PLAYER)
    {
        switch(attackType)
        {
            case BASE_ATTACK:
                crit = GetFloatValue( PLAYER_CRIT_PERCENTAGE );
                break;
            case OFF_ATTACK:
                crit = GetFloatValue( PLAYER_OFFHAND_CRIT_PERCENTAGE );
                break;
            case RANGED_ATTACK:
                crit = GetFloatValue( PLAYER_RANGED_CRIT_PERCENTAGE );
                break;
                // Just for good manner
            default:
                crit = 0.0f;
                break;
        }
    }
    else
    {
        crit = 5.0f;
        crit += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PERCENT);
    }

    // flat aura mods
    if(attackType == RANGED_ATTACK)
        crit += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE);
    else
        crit += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE);

    crit += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE);

    // reduce crit chance from Rating for players
    if (pVictim->GetTypeId()==TYPEID_PLAYER)
    {
        if (attackType==RANGED_ATTACK)
            crit -= (pVictim->ToPlayer())->GetRatingBonusValue(CR_CRIT_TAKEN_RANGED);
        else
            crit -= (pVictim->ToPlayer())->GetRatingBonusValue(CR_CRIT_TAKEN_MELEE);
    }

    if (crit < 0.0f)
        crit = 0.0f;
    return crit;
}

uint32 Unit::GetWeaponSkillValue (WeaponAttackType attType, Unit const* target) const
{
    uint32 value = 0;
    if(GetTypeId() == TYPEID_PLAYER)
    {
        Item* item = (this->ToPlayer())->GetWeaponForAttack(attType,true);

        // feral or unarmed skill only for base attack
        if(attType != BASE_ATTACK && !item )
        {
            if(attType == RANGED_ATTACK && GetClass() == CLASS_PALADIN) //hammer
                return GetMaxSkillValueForLevel();
            return 0;
        }

        if((this->ToPlayer())->IsInFeralForm())
            return GetMaxSkillValueForLevel();              // always maximized SKILL_FERAL_COMBAT in fact

        // weapon skill or (unarmed for base attack)
        uint32 skill;
        if (item && item->GetSkill() != SKILL_FIST_WEAPONS)
            skill = item->GetSkill();
        else
            skill = SKILL_UNARMED;

        // in PvP use full skill instead current skill value
        value = (target && target->isCharmedOwnedByPlayerOrPlayer())
            ? (this->ToPlayer())->GetMaxSkillValue(skill)
            : (this->ToPlayer())->GetSkillValue(skill);
        // Modify value from ratings
        value += uint32((this->ToPlayer())->GetRatingBonusValue(CR_WEAPON_SKILL));
        switch (attType)
        {
            case BASE_ATTACK:   value+=uint32((this->ToPlayer())->GetRatingBonusValue(CR_WEAPON_SKILL_MAINHAND));break;
            case OFF_ATTACK:    value+=uint32((this->ToPlayer())->GetRatingBonusValue(CR_WEAPON_SKILL_OFFHAND));break;
            case RANGED_ATTACK: value+=uint32((this->ToPlayer())->GetRatingBonusValue(CR_WEAPON_SKILL_RANGED));break;
            default: break;
        }
    }
    else
        value = GetUnitMeleeSkill(target);
   return value;
}

void Unit::BuildValuesUpdate(uint8 updateType, ByteBuffer* data, Player* target) const
{
    if (!target)
        return;

    ByteBuffer fieldBuffer;

    UpdateMask updateMask;
    updateMask.SetCount(m_valuesCount);

    uint32* flags = UnitUpdateFieldFlags;
    uint32 visibleFlag = UF_FLAG_PUBLIC;

    if (target == this)
        visibleFlag |= UF_FLAG_PRIVATE;

    Player* plr = GetCharmerOrOwnerPlayerOrPlayerItself();
    if (GetOwnerGUID() == target->GetGUID())
        visibleFlag |= UF_FLAG_OWNER;

    if (HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO))
        if (HasAuraTypeWithCaster(SPELL_AURA_EMPATHY, target->GetGUID()))
            visibleFlag |= UF_FLAG_SPECIAL_INFO;

    if (plr && plr->IsInSameRaidWith(target))
        visibleFlag |= UF_FLAG_PARTY_MEMBER;

    Creature const* creature = ToCreature();
    for (uint16 index = 0; index < m_valuesCount; ++index)
    {
        if (   _fieldNotifyFlags & flags[index]  //if the given flag was set to notify, if the given index is not set to UF_FLAG_NONE
            || ((flags[index] & visibleFlag) & UF_FLAG_SPECIAL_INFO) //given index has UF_FLAG_SPECIAL_INFO and target has SPELL_AURA_EMPATHY on the target
            || ((updateType == UPDATETYPE_VALUES ? _changesMask.GetBit(index) : m_uint32Values[index]) && (flags[index] & visibleFlag)) // flag has changed && flag is visible to player
            || (index == UNIT_FIELD_AURASTATE && HasFlag(UNIT_FIELD_AURASTATE, PER_CASTER_AURA_STATE_MASK)) //if index UNIT_FIELD_AURASTATE && unit has some state (we always send update while the object has those)
           )
        {
            updateMask.SetBit(index);

            if (index == UNIT_NPC_FLAGS)
            {
                uint32 appendValue = m_uint32Values[UNIT_NPC_FLAGS];

                //remove server custom flags 
                appendValue &= ~(UNIT_NPC_FLAG_GUARD | UNIT_NPC_FLAG_OUTDOORPVP);
             /* LK?  if (creature)
                    if (!target->CanSeeSpellClickOn(creature))
                        appendValue &= ~UNIT_NPC_FLAG_SPELLCLICK; */

                fieldBuffer << uint32(appendValue);
            }
            else if (index == UNIT_FIELD_AURASTATE)
            {
                // Check per caster aura states to not enable using a spell in client if specified aura is not by target
                fieldBuffer << BuildAuraStateUpdateForTarget(target);
            }
            // FIXME: Some values at server stored in float format but must be sent to client in uint32 format
            else if (index >= UNIT_FIELD_BASEATTACKTIME && index <= UNIT_FIELD_RANGEDATTACKTIME)
            {
                // convert from float to uint32 and send
                fieldBuffer << uint32(m_floatValues[index] < 0 ? 0 : m_floatValues[index]);
            }
            // there are some float values which may be negative or can't get negative due to other checks
            else if ((index >= UNIT_FIELD_NEGSTAT0   && index <= UNIT_FIELD_NEGSTAT4) ||
                (index >= UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE  && index <= (UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + 6)) ||
                (index >= UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE  && index <= (UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + 6)) ||
                (index >= UNIT_FIELD_POSSTAT0   && index <= UNIT_FIELD_POSSTAT4))
            {
                fieldBuffer << uint32(m_floatValues[index]);
            }
            // Gamemasters should be always able to select units - remove not selectable flag
            else if (index == UNIT_FIELD_FLAGS)
            {
                uint32 appendValue = m_uint32Values[UNIT_FIELD_FLAGS];
                if (target->IsGameMaster())
                    appendValue &= ~UNIT_FLAG_NOT_SELECTABLE;

                fieldBuffer << uint32(appendValue);
            }
            // use modelid_a if not gm, _h if gm for CREATURE_FLAG_EXTRA_TRIGGER creatures
            else if (index == UNIT_FIELD_DISPLAYID)
            {
                uint32 displayId = m_uint32Values[UNIT_FIELD_DISPLAYID];
                if (creature)
                {
                    CreatureTemplate const* cinfo = creature->GetCreatureTemplate();

                    // this also applies for transform auras
                    if (SpellInfo const* transform = sSpellMgr->GetSpellInfo(GetTransForm()))
                        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                            if (transform->Effects[i].ApplyAuraName == SPELL_AURA_TRANSFORM)
                                if (CreatureTemplate const* transformInfo = sObjectMgr->GetCreatureTemplate(transform->Effects[i].MiscValue))
                                {
                                    cinfo = transformInfo;
                                    break;
                                }

                    if (cinfo->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER)
                    {
                        if (target->IsGameMaster())
                        {
                            if (cinfo->Modelid1)
                                displayId = cinfo->Modelid1;    // Modelid1 is a visible model for gms
                            else
                                displayId = 17519;              // world visible trigger's model
                        }
                        else
                        {
                            if (cinfo->Modelid2)
                                displayId = cinfo->Modelid2;    // Modelid2 is an invisible model for players
                            else
                                displayId = 11686;              // world invisible trigger's model
                        }
                    }
                }

                fieldBuffer << uint32(displayId);
            }
            // hide lootable animation for unallowed players
            else if (index == UNIT_DYNAMIC_FLAGS)
            {
                uint32 dynamicFlags = m_uint32Values[UNIT_DYNAMIC_FLAGS] & ~(UNIT_DYNFLAG_TAPPED | UNIT_DYNFLAG_TAPPED_BY_PLAYER);

                if (creature)
                {
                    if (creature->hasLootRecipient())
                    {
                        dynamicFlags |= UNIT_DYNFLAG_TAPPED;
                        if (creature->isTappedBy(target))
                            dynamicFlags |= UNIT_DYNFLAG_TAPPED_BY_PLAYER;
                    }

                    if (!target->IsAllowedToLoot(creature))
                        dynamicFlags &= ~UNIT_DYNFLAG_LOOTABLE;
                }

                // unit UNIT_DYNFLAG_TRACK_UNIT should only be sent to caster of SPELL_AURA_MOD_STALKED auras
                if (dynamicFlags & UNIT_DYNFLAG_TRACK_UNIT)
                    if (!HasAuraTypeWithCaster(SPELL_AURA_MOD_STALKED, target->GetGUID()))
                        dynamicFlags &= ~UNIT_DYNFLAG_TRACK_UNIT;

                fieldBuffer << dynamicFlags;
            }
            // FG: pretend that OTHER players in own group are friendly ("blue")
            else if (index == UNIT_FIELD_BYTES_2 || index == UNIT_FIELD_FACTIONTEMPLATE)
            {
                if (/* IsControlledByPlayer() && */ target != this && IS_PLAYER_GUID(target->GetCharmerOrOwnerGUID()) && sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP) && IsInRaidWith(target))
                {
                    FactionTemplateEntry const* ft1 = GetFactionTemplateEntry();
                    FactionTemplateEntry const* ft2 = target->GetFactionTemplateEntry();
                    if (ft1 && ft2 && !ft1->IsFriendlyTo(*ft2))
                    {
                        if (index == UNIT_FIELD_BYTES_2)
                            // Allow targetting opposite faction in party when enabled in config
                            fieldBuffer << (m_uint32Values[UNIT_FIELD_BYTES_2] & ((UNIT_BYTE2_FLAG_SANCTUARY /*| UNIT_BYTE2_FLAG_AURAS | UNIT_BYTE2_FLAG_UNK5*/) << 8)); // this flag is at uint8 offset 1 !!
                        else
                            // pretend that all other HOSTILE players have own faction, to allow follow, heal, rezz (trade wont work)
                            fieldBuffer << uint32(target->GetFaction());
                    }
                    else
                        fieldBuffer << m_uint32Values[index];
                }
                else
                    fieldBuffer << m_uint32Values[index];
            }
            else
            {
                // send in current format (float as float, uint32 as uint32)
                fieldBuffer << m_uint32Values[index];
            }
        }
    }

    *data << uint8(updateMask.GetBlockCount());
    updateMask.AppendToPacket(data);
    data->append(fieldBuffer);
}

void Unit::_DeleteAuras()
{
    while(!m_removedAuras.empty())
    {
        delete m_removedAuras.front();
        m_removedAuras.pop_front();
    }
}

void Unit::_UpdateSpells( uint32 time )
{
    if(m_currentSpells[CURRENT_AUTOREPEAT_SPELL])
        _UpdateAutoRepeatSpell();

    // remove finished spells from current pointers
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; i++)
    {
        if (m_currentSpells[i] && m_currentSpells[i]->getState() == SPELL_STATE_FINISHED)
        {
            m_currentSpells[i]->SetReferencedFromCurrent(false);
            m_currentSpells[i] = NULL;                      // remove pointer
        }
    }

    // update auras
    // m_AurasUpdateIterator can be updated in inderect called code at aura remove to skip next planned to update but removed auras
    for (m_AurasUpdateIterator = m_Auras.begin(); m_AurasUpdateIterator != m_Auras.end(); )
    {
        Aura* i_aura = m_AurasUpdateIterator->second;
        ++m_AurasUpdateIterator;                            // need shift to next for allow update if need into aura update
        if (i_aura)
            i_aura->Update(time);
    }

    // remove expired auras
    for (AuraMap::iterator i = m_Auras.begin(); i != m_Auras.end(); )
    {
        if ( i->second->IsExpired() )
            RemoveAura(i);
        else
            ++i;
    }

    _DeleteAuras();

    if(!m_gameObj.empty())
    {
        std::list<GameObject*>::iterator ite1, dnext1;
        for (ite1 = m_gameObj.begin(); ite1 != m_gameObj.end(); ite1 = dnext1)
        {
            dnext1 = ite1;
            //(*i)->Update( difftime );
            if( !(*ite1)->isSpawned() )
            {
                (*ite1)->SetOwnerGUID(0);
                (*ite1)->SetRespawnTime(0);
                (*ite1)->Delete();
                dnext1 = m_gameObj.erase(ite1);
            }
            else
                ++dnext1;
        }
    }
}

void Unit::_UpdateAutoRepeatSpell()
{
    //check "realtime" interrupts
    if ( (GetTypeId() == TYPEID_PLAYER && (this->ToPlayer())->isMoving()) || IsNonMeleeSpellCast(false,false,true) )
    {
        // cancel wand shoot
        if(m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->GetCategory() == 351)
            InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
        m_AutoRepeatFirstCast = true;
        return;
    }

    //apply delay
    if ( m_AutoRepeatFirstCast && GetAttackTimer(RANGED_ATTACK) < 500 )
        SetAttackTimer(RANGED_ATTACK,500);
    m_AutoRepeatFirstCast = false;

    //castroutine
    if (IsAttackReady(RANGED_ATTACK))
    {
        // Check if able to cast
        if(m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->CheckCast(true) != SPELL_CAST_OK)
        {
            InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
            return;
        }

        // we want to shoot
        Spell* spell = new Spell(this, m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo, true, 0);
        spell->prepare(&(m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_targets));

        // all went good, reset attack
        ResetAttackTimer(RANGED_ATTACK);
    }
}

void Unit::SetCurrentCastedSpell( Spell * pSpell )
{
    assert(pSpell);                                         // NULL may be never passed here, use InterruptSpell or InterruptNonMeleeSpells

    uint32 CSpellType = pSpell->GetCurrentContainer();

    if (pSpell == m_currentSpells[CSpellType]) return;      // avoid breaking self

    // break same type spell if it is not delayed
    InterruptSpell(CSpellType,false);

    // special breakage effects:
    switch (CSpellType)
    {
        case CURRENT_GENERIC_SPELL:
        {
            // generic spells always break channeled not delayed spells
            InterruptSpell(CURRENT_CHANNELED_SPELL,false);

            // break wand autorepeat
            if ( m_currentSpells[CURRENT_AUTOREPEAT_SPELL] &&
                 m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->GetCategory() == 351) // wand
                    InterruptSpell(CURRENT_AUTOREPEAT_SPELL);

            //delay autoshoot by 0,5s
            if( pSpell->GetCastTime() > 0) // instant spells don't break autoshoot anymore, see 2.4.3 patchnotes)
                m_AutoRepeatFirstCast = true;

            AddUnitState(UNIT_STATE_CASTING);
        } break;

        case CURRENT_CHANNELED_SPELL:
        {
            // channel spells always break generic non-delayed and any channeled spells
            InterruptSpell(CURRENT_GENERIC_SPELL,false);
            InterruptSpell(CURRENT_CHANNELED_SPELL);

            // break wand autorepeat
            if ( m_currentSpells[CURRENT_AUTOREPEAT_SPELL] &&
                m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->GetCategory() == 351 )
                InterruptSpell(CURRENT_AUTOREPEAT_SPELL);

            //delay autoshoot by 0,5s
            m_AutoRepeatFirstCast = true;

            AddUnitState(UNIT_STATE_CASTING);
        } break;

        case CURRENT_AUTOREPEAT_SPELL:
        {
            // wand break other spells
            if (pSpell->m_spellInfo->GetCategory() == 351)
            {
                // generic autorepeats break generic non-delayed and channeled non-delayed spells
                InterruptSpell(CURRENT_GENERIC_SPELL,false);
                InterruptSpell(CURRENT_CHANNELED_SPELL,false);
            }
            // special action: set first cast flag
            m_AutoRepeatFirstCast = true;
        } break;

        default:
        {
            // other spell types don't break anything now
        } break;
    }

    // current spell (if it is still here) may be safely deleted now
    if (m_currentSpells[CSpellType])
        m_currentSpells[CSpellType]->SetReferencedFromCurrent(false);

    // set new current spell
    m_currentSpells[CSpellType] = pSpell;
    pSpell->SetReferencedFromCurrent(true);
}

void Unit::InterruptSpell(uint32 spellType, bool withDelayed, bool withInstant)
{
    assert(spellType < CURRENT_MAX_SPELL);

    Spell *spell = m_currentSpells[spellType];
    if(spell
        && (withDelayed || spell->getState() != SPELL_STATE_DELAYED)
        && (withInstant || spell->GetCastTime() > 0))
    {
        // for example, do not let self-stun aura interrupt itself
        if(!spell->IsInterruptable())
            return;

        m_currentSpells[spellType] = NULL;

        // send autorepeat cancel message for autorepeat spells
        if (spellType == CURRENT_AUTOREPEAT_SPELL)
        {
            if(GetTypeId()==TYPEID_PLAYER)
                (this->ToPlayer())->SendAutoRepeatCancel();
        }

        if (spell->getState() != SPELL_STATE_FINISHED)
            spell->cancel();
        spell->SetReferencedFromCurrent(false);
    }
}

bool Unit::HasDelayedSpell()
{
    if ( m_currentSpells[CURRENT_GENERIC_SPELL] &&
        (m_currentSpells[CURRENT_GENERIC_SPELL]->getState() == SPELL_STATE_DELAYED) )
        return true;

    return false;
}

bool Unit::IsNonMeleeSpellCast(bool withDelayed, bool skipChanneled, bool skipAutorepeat) const
{
    // We don't do loop here to explicitly show that melee spell is excluded.
    // Maybe later some special spells will be excluded too.

    // generic spells are casted when they are not finished and not delayed
    if ( m_currentSpells[CURRENT_GENERIC_SPELL] &&
        (m_currentSpells[CURRENT_GENERIC_SPELL]->getState() != SPELL_STATE_FINISHED) &&
        (withDelayed || m_currentSpells[CURRENT_GENERIC_SPELL]->getState() != SPELL_STATE_DELAYED) )
            return(true);

    // channeled spells may be delayed, but they are still considered casted
    else if ( !skipChanneled && m_currentSpells[CURRENT_CHANNELED_SPELL] &&
        (m_currentSpells[CURRENT_CHANNELED_SPELL]->getState() != SPELL_STATE_FINISHED) )
        return(true);

    // autorepeat spells may be finished or delayed, but they are still considered casted
    else if ( !skipAutorepeat && m_currentSpells[CURRENT_AUTOREPEAT_SPELL] )
        return(true);

    return(false);
}

void Unit::InterruptNonMeleeSpells(bool withDelayed, uint32 spell_id, bool withInstant)
{
    // generic spells are interrupted if they are not finished or delayed
    if (m_currentSpells[CURRENT_GENERIC_SPELL] && (!spell_id || m_currentSpells[CURRENT_GENERIC_SPELL]->m_spellInfo->Id==spell_id))
        InterruptSpell(CURRENT_GENERIC_SPELL,withDelayed,withInstant);

    // autorepeat spells are interrupted if they are not finished or delayed
    if (m_currentSpells[CURRENT_AUTOREPEAT_SPELL] && (!spell_id || m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->Id==spell_id))
        InterruptSpell(CURRENT_AUTOREPEAT_SPELL,withDelayed,withInstant);

    // channeled spells are interrupted if they are not finished, even if they are delayed
    if (m_currentSpells[CURRENT_CHANNELED_SPELL] && (!spell_id || m_currentSpells[CURRENT_CHANNELED_SPELL]->m_spellInfo->Id==spell_id))
        InterruptSpell(CURRENT_CHANNELED_SPELL,true,true);
}

Spell* Unit::FindCurrentSpellBySpellId(uint32 spell_id) const
{
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; i++)
        if(m_currentSpells[i] && m_currentSpells[i]->m_spellInfo->Id==spell_id)
            return m_currentSpells[i];
    return NULL;
}

bool Unit::isInAccessiblePlaceFor(Creature const* c) const
{
#ifdef LICH_KING
    if (c->GetMapId() == 618) // Ring of Valor
    {
        // skip transport check, check for being below floor level
        if (this->GetPositionZ() < 28.0f)
            return false;
        if (BattlegroundMap* bgMap = c->GetMap()->ToBattlegroundMap())
            if (Battleground* bg = bgMap->GetBG())
                if (bg->GetStartTime() < 80133) // 60000ms preparation time + 20133ms elevator rise time
                    return false;
    }
    else if (c->GetMapId() == 631) // Icecrown Citadel
    {
        // if static transport doesn't match - return false
        if (c->GetTransport() != this->GetTransport() && (c->GetTransport() && c->GetTransport()->IsStaticTransport() || this->GetTransport() && this->GetTransport()->IsStaticTransport()))
            return false;

        // special handling for ICC (map 631), for non-flying pets in Gunship Battle, for trash npcs this is done via CanAIAttack
        if (IS_PLAYER_GUID(c->GetOwnerGUID()) && !c->CanFly())
        {
            if (c->GetTransport() && !this->GetTransport() || !c->GetTransport() && this->GetTransport())
                return false;
            if (this->GetTransport())
            {
                if (c->GetPositionY() < 2033.0f)
                {
                    if (this->GetPositionY() > 2033.0f)
                        return false;
                }
                else if (c->GetPositionY() < 2438.0f)
                {
                    if (this->GetPositionY() < 2033.0f || this->GetPositionY() > 2438.0f)
                        return false;
                }
                else if (this->GetPositionY() < 2438.0f)
                    return false;
            }
        }
    }
    else
#endif
    {
        // Prevent any bugs by passengers exiting transports or normal creatures flying away (but still allow pets to do it)
        if (!c->GetOwnerGUID() && c->GetTransport() != this->GetTransport())
            return false;
    }

    if (IsInWater())
        return c->CanSwim();
    else
        return c->CanWalk() || c->CanFly();
}

bool Unit::IsInWater() const
{
    return GetBaseMap()->IsInWater(GetPositionX(), GetPositionY(), GetPositionZ());
}

bool Unit::IsUnderWater() const
{
    return GetBaseMap()->IsUnderWater(GetPositionX(), GetPositionY(), GetPositionZ());
}

void Unit::UpdateUnderwaterState(Map* m, float x, float y, float z)
{
    if (IsFlying() || !IsPet()
#ifdef LICH_LING
        && !IsVehicle()
#endif
     )
        return;

    LiquidData liquid_status;
    ZLiquidStatus res = m->getLiquidStatus(x, y, z, BASE_LIQUID_TYPE_MASK_ALL, &liquid_status);
    if (!res)
    {
        if (_lastLiquid && _lastLiquid->SpellId)
            RemoveAurasDueToSpell(_lastLiquid->SpellId);

        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_UNDERWATER);
        _lastLiquid = NULL;
        return;
    }

    if (uint32 type = liquid_status.baseLiquidType)
    {
        LiquidTypeEntry const* liquid = sLiquidTypeStore.LookupEntry(type);
        if (_lastLiquid && _lastLiquid->SpellId && _lastLiquid->Id != type)
            RemoveAurasDueToSpell(_lastLiquid->SpellId);

        if (liquid && liquid->SpellId)
        {
            if (res & (LIQUID_MAP_UNDER_WATER | LIQUID_MAP_IN_WATER))
            {
                if (!HasAuraEffect(liquid->SpellId))
                    CastSpell(this, liquid->SpellId, true);
            }
            else
                RemoveAurasDueToSpell(liquid->SpellId);
        }

        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_ABOVEWATER);
        _lastLiquid = liquid;
    }
    else if (_lastLiquid && _lastLiquid->SpellId)
    {
        RemoveAurasDueToSpell(_lastLiquid->SpellId);
        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_UNDERWATER);
        _lastLiquid = NULL;
    }
}

void Unit::DeMorph()
{
    SetDisplayId(GetNativeDisplayId());
}

int32 Unit::GetTotalAuraModifier(AuraType auratype) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return modifier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
        if ((*i))
            modifier += (*i)->GetModifierValue();

    return modifier;
}

float Unit::GetTotalAuraMultiplier(AuraType auratype) const
{
    float multiplier = 1.0f;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return multiplier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
        if ((*i))
            multiplier *= (100.0f + (*i)->GetModifierValue())/100.0f;

    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifier(AuraType auratype) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return modifier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if ((*i))
        {
            int32 amount = (*i)->GetModifierValue();
            if (amount > modifier)
                modifier = amount;
        }
    }

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifier(AuraType auratype) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return modifier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if ((*i))
        {
            int32 amount = (*i)->GetModifierValue();
            if (amount < modifier)
                modifier = amount;
        }
    }

    return modifier;
}

int32 Unit::GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return modifier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if ((*i))
        {
            Modifier const* mod = (*i)->GetModifier();
            if (mod->m_miscvalue & misc_mask)
                modifier += (*i)->GetModifierValue();
        }
    }
    return modifier;
}

float Unit::GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    float multiplier = 1.0f;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return multiplier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if ((*i))
        {
            Modifier const* mod = (*i)->GetModifier();
            if (mod->m_miscvalue & misc_mask)
                multiplier *= (100.0f + (*i)->GetModifierValue())/100.0f;
        }
    }
    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return modifier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if ((*i))
        {
            Modifier const* mod = (*i)->GetModifier();
            int32 amount = (*i)->GetModifierValue();
            if (mod->m_miscvalue & misc_mask && amount > modifier)
                modifier = amount;
        }
    }

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return modifier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if ((*i))
        {
            Modifier const* mod = (*i)->GetModifier();
            int32 amount = (*i)->GetModifierValue();
            if (mod->m_miscvalue & misc_mask && amount < modifier)
                modifier = amount;
        }
    }

    return modifier;
}

int32 Unit::GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return modifier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if ((*i))
        {
            Modifier const* mod = (*i)->GetModifier();
            if (mod->m_miscvalue == misc_value)
                modifier += (*i)->GetModifierValue();
        }
    }
    return modifier;
}

float Unit::GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const
{
    float multiplier = 1.0f;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return multiplier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if ((*i))
        {
            Modifier const* mod = (*i)->GetModifier();
            if (mod->m_miscvalue == misc_value)
                multiplier *= (100.0f + (*i)->GetModifierValue())/100.0f;
        }
    }
    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return modifier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if ((*i))
        {
            Modifier const* mod = (*i)->GetModifier();
            int32 amount = (*i)->GetModifierValue();
            if (mod->m_miscvalue == misc_value && amount > modifier)
                modifier = amount;
        }
    }

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const
{
    int32 modifier = 0;

    AuraList const& mTotalAuraList = GetAurasByType(auratype);
    if(mTotalAuraList.empty())
        return modifier;

    for(AuraList::const_iterator i = mTotalAuraList.begin();i != mTotalAuraList.end(); ++i)
    {
        if ((*i))
        {
            Modifier const* mod = (*i)->GetModifier();
            int32 amount = (*i)->GetModifierValue();
            if (mod->m_miscvalue == misc_value && amount < modifier)
                modifier = amount;
        }
    }

    return modifier;
}

/** Aura pointer will be deleted on aura remove in Unit::_DeleteAuras() */
bool Unit::AddAura(Aura *Aur)
{
    // ghost spell check, allow apply any auras at player loading in ghost mode (will be cleanup after load)
    if( (!IsAlive() && !(Aur->GetSpellInfo()->Attributes & SPELL_ATTR0_CASTABLE_WHILE_DEAD)) && Aur->GetId() != 20584 && Aur->GetId() != 8326 && Aur->GetId() != 2584 &&
        (GetTypeId()!=TYPEID_PLAYER || !(this->ToPlayer())->GetSession()->PlayerLoading()) )
    {
        delete Aur;
        return false;
    }

    if (IsImmunedToSpell(Aur->GetSpellInfo())) {
        delete Aur;
        return false;
    }

    if(Aur->GetTarget() != this)
    {
        TC_LOG_ERROR("spell","Aura (spell %u eff %u) add to aura list of %s (lowguid: %u) but Aura target is %s (lowguid: %u)",
            Aur->GetId(),Aur->GetEffIndex(),(GetTypeId()==TYPEID_PLAYER?"player":"creature"),GetGUIDLow(),
            (Aur->GetTarget()->GetTypeId()==TYPEID_PLAYER?"player":"creature"),Aur->GetTarget()->GetGUIDLow());
        delete Aur;
        return false;
    }

    if ((Aur->DoesAuraApplyAuraName(SPELL_AURA_MOD_CONFUSE) || Aur->DoesAuraApplyAuraName(SPELL_AURA_MOD_CHARM) ||
        Aur->DoesAuraApplyAuraName(SPELL_AURA_MOD_STUN)) && (Aur->GetSpellInfo()->Attributes & SPELL_ATTR0_HEARTBEAT_RESIST_CHECK))
        m_justCCed = 2;

    SpellInfo const* aurSpellInfo = Aur->GetSpellInfo();

    spellEffectPair spair = spellEffectPair(Aur->GetId(), Aur->GetEffIndex());

    bool stackModified=false;
    bool doubleMongoose=false;
    //if (Aur->GetId() == 28093) TC_LOG_INFO("Mongoose proc from item "I64FMTD, Aur->GetCastItemGUID());
    // passive and persistent auras can stack with themselves any number of times
    if (!Aur->IsPassive() && !Aur->IsPersistent())
    {
        for(AuraMap::iterator i2 = m_Auras.lower_bound(spair); i2 != m_Auras.upper_bound(spair);)
        {
            if(i2->second->GetCasterGUID()==Aur->GetCasterGUID())
            {
                if (!stackModified)
                {
                    // auras from same caster but different items (mongoose) can stack
                    if(Aur->GetCastItemGUID() != i2->second->GetCastItemGUID() && Aur->GetId() == 28093) {
                        i2++;
                        doubleMongoose = true;
                        //TC_LOG_INFO("Mongoose double proc from item "I64FMTD" !", Aur->GetCastItemGUID());
                        continue;
                    }
                    else if(aurSpellInfo->StackAmount) // replace aura if next will > spell StackAmount
                    {
                        // prevent adding stack more than once
                        stackModified=true;
                        Aur->SetStackAmount(i2->second->GetStackAmount());
                        Aur->SetPeriodicTimer(i2->second->GetPeriodicTimer());
                        if(Aur->GetStackAmount() < aurSpellInfo->StackAmount)
                            Aur->SetStackAmount(Aur->GetStackAmount()+1);
                    }
                    //keep old modifier if higher than new aura modifier
                    //(removed for now, this causes problem with some stacking auras)
                    /*
                    if(i2->second->GetModifierValuePerStack() > Aur->GetModifierValuePerStack())
                        Aur->SetModifierValuePerStack(i2->second->GetModifierValuePerStack());
                    if(i2->second->GetBasePoints() > Aur->GetBasePoints())
                        Aur->SetBasePoints(i2->second->GetBasePoints());
                        */
                    RemoveAura(i2,AURA_REMOVE_BY_STACK);
                    i2=m_Auras.lower_bound(spair);
                    continue;
                }
            }
            else if (Aur->GetSpellInfo()->HasAttribute(SPELL_ATTR_CU_SAME_STACK_DIFF_CASTERS)) {
                stackModified=true;
                Aur->SetStackAmount(i2->second->GetStackAmount());
                if(Aur->GetStackAmount() < aurSpellInfo->StackAmount)
                    Aur->SetStackAmount(Aur->GetStackAmount()+1);
            }
            else if (Aur->GetSpellInfo()->HasAttribute(SPELL_ATTR_CU_ONE_STACK_PER_CASTER_SPECIAL)) {
                ++i2;
                continue;
            }
            switch(aurSpellInfo->Effects[Aur->GetEffIndex()].ApplyAuraName)
            {
                // DOT or HOT from different casters will stack
                case SPELL_AURA_MOD_DECREASE_SPEED:
                    // Mind Flay
                    if(aurSpellInfo->SpellFamilyFlags & 0x0000000000800000LL && aurSpellInfo->SpellFamilyName == SPELLFAMILY_PRIEST)
                    {
                        ++i2;
                        continue;
                    }
                    break;
                case SPELL_AURA_MOD_DAMAGE_PERCENT_DONE:
                    // Ferocious Inspiration
                    if (aurSpellInfo->Id == 34456) {
                        ++i2;
                        continue;
                    }
                    break;
                case SPELL_AURA_DUMMY:
                    /* X don't merge to TC2 - BoL was removed and Mangle changed aura from dummy to 255 */
                    
                    // Blessing of Light exception - only one per target
                    if (aurSpellInfo->HasVisual(9180) && aurSpellInfo->SpellFamilyName == SPELLFAMILY_PALADIN)
                        break;
                    // Druid Mangle bear & cat
                    if (aurSpellInfo->SpellFamilyName == SPELLFAMILY_DRUID && aurSpellInfo->SpellIconID == 2312)
                        break;
                case SPELL_AURA_PERIODIC_DAMAGE:
                    if (aurSpellInfo->Id == 45032 || aurSpellInfo->Id == 45034) // Curse of Boundless Agony can only have one stack per target
                        break;
                    if (aurSpellInfo->Id == 44335)      // Vexallus
                        break;
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
                    if (aurSpellInfo->Id == 31944) // Doomfire DoT - only one per target
                        break;
                case SPELL_AURA_PERIODIC_ENERGIZE:
                case SPELL_AURA_PERIODIC_MANA_LEECH:
                case SPELL_AURA_PERIODIC_LEECH:
                case SPELL_AURA_POWER_BURN_MANA:
                case SPELL_AURA_OBS_MOD_POWER:
                case SPELL_AURA_OBS_MOD_HEALTH:
                    ++i2;
                    continue;
            }
            /*
            //keep old modifier if higher than new aura modifier
            //(removed for now, this causes problem with some stacking auras)
            if(i2->second->GetModifierValuePerStack() > Aur->GetModifierValuePerStack())
                Aur->SetModifierValuePerStack(i2->second->GetModifierValuePerStack());
            if(i2->second->GetBasePoints() > Aur->GetBasePoints())
                Aur->SetBasePoints(i2->second->GetBasePoints());
            */
            RemoveAura(i2,AURA_REMOVE_BY_STACK);
            i2=m_Auras.lower_bound(spair);
            continue;
        }
    }

    // passive auras stack with all (except passive spell proc auras)
    if ((!Aur->IsPassive() || !IsPassiveStackableSpell(Aur->GetId())) &&
        !(Aur->GetId() == 20584 || Aur->GetId() == 8326 || Aur->GetId() == 28093))
    {
        if (!RemoveNoStackAurasDueToAura(Aur))
        {
            delete Aur;
            return false;                                   // couldn't remove conflicting aura with higher rank
        }
    }

    // update single target auras list (before aura add to aura list, to prevent unexpected remove recently added aura)
    if (Aur->IsSingleTarget() && Aur->GetTarget())
    {
        m_GiantLock.lock();
        // caster pointer can be deleted in time aura remove, find it by guid at each iteration
        for(;;)
        {
            Unit* caster = Aur->GetCaster();
            if(!caster)                                     // caster deleted and not required adding scAura
                break;

            bool restart = false;
            AuraList& scAuras = caster->GetSingleCastAuras();
            for(AuraList::iterator itr = scAuras.begin(); itr != scAuras.end(); ++itr)
            {
                if( (*itr)->GetTarget() != Aur->GetTarget() &&
                    IsSingleTargetSpells((*itr)->GetSpellInfo(),aurSpellInfo) )
                {
                    if ((*itr)->IsInUse())
                    {
                        TC_LOG_ERROR("spell","Aura (Spell %u Effect %u) is in process but attempt removed at aura (Spell %u Effect %u) adding, need add stack rule for IsSingleTarget", (*itr)->GetId(), (*itr)->GetEffIndex(),Aur->GetId(), Aur->GetEffIndex());
                        continue;
                    }
                    (*itr)->GetTarget()->RemoveAura((*itr)->GetId(), (*itr)->GetEffIndex());
                    restart = true;
                    break;
                }
            }

            if(!restart)
            {
                // done
                scAuras.push_back(Aur);
                break;
            }
        }
        m_GiantLock.unlock();
    }

    // add aura, register in lists and arrays
    Aur->_AddAura(!(doubleMongoose && Aur->GetEffIndex() == 0));    // We should change slot only while processing the first effect of double mongoose
    m_Auras.insert(AuraMap::value_type(spellEffectPair(Aur->GetId(), Aur->GetEffIndex()), Aur));
    if (Aur->GetModifier()->m_auraname < TOTAL_AURAS)
    {
        m_modAuras[Aur->GetModifier()->m_auraname].push_back(Aur);
        if(Aur->GetSpellInfo()->AuraInterruptFlags)
        {
            m_interruptableAuras.push_back(Aur);
            AddInterruptMask(Aur->GetSpellInfo()->AuraInterruptFlags);
        }
        if((Aur->GetSpellInfo()->Attributes & SPELL_ATTR0_HEARTBEAT_RESIST_CHECK)
            && (Aur->GetModifier()->m_auraname != SPELL_AURA_MOD_POSSESS)) //only dummy aura is breakable
        {
            m_ccAuras.push_back(Aur);
        }
        if(AuraStateType aState = Aur->GetSpellInfo()->GetAuraState())
        {
            m_auraStateAuras.insert(AuraStateAurasMap::value_type(aState, Aur));
            ModifyAuraState(aState, true);
        }
    }

    Aur->ApplyModifier(true,true);

    uint32 id = Aur->GetId();
    if(Aur->GetSpellInfo()->HasAttribute(SPELL_ATTR_CU_LINK_AURA))
    {
        if(const std::vector<int32> *spell_triggered = sSpellMgr->GetSpellLinked(id + SPELL_LINK_AURA))
        {
            for(std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                if(*itr < 0)
                    ApplySpellImmune(id, IMMUNITY_ID, -(*itr), true);
                else if(Unit* caster = Aur->GetCaster())
                    caster->AddAura(*itr, this);
        }
    }

    return true;
}

void Unit::RemoveRankAurasDueToSpell(uint32 spellId)
{
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if(!spellInfo)
        return;

    AuraMap::iterator i,next;
    for (i = m_Auras.begin(); i != m_Auras.end(); i = next)
    {
        next = i;
        ++next;
        uint32 i_spellId = (*i).second->GetId();
        if((*i).second && i_spellId && i_spellId != spellId)
        {
            if(sSpellMgr->IsRankSpellDueToSpell(spellInfo,i_spellId))
            {
                RemoveAurasDueToSpell(i_spellId);

                if( m_Auras.empty() )
                    break;
                else
                    next =  m_Auras.begin();
            }
        }
    }
}

bool Unit::RemoveNoStackAurasDueToAura(Aura *Aur)
{
    if (!Aur)
        return false;

    SpellInfo const* spellProto = Aur->GetSpellInfo();
    if (!spellProto)
        return false;

    uint32 spellId = Aur->GetId();
    uint32 effIndex = Aur->GetEffIndex();

    AuraMap::iterator i,next;
    for (i = m_Auras.begin(); i != m_Auras.end(); i = next)
    {
        next = i;
        ++next;
        if (!(*i).second) continue;

        SpellInfo const* i_spellProto = (*i).second->GetSpellInfo();

        if (!i_spellProto)
            continue;

        uint32 i_spellId = i_spellProto->Id;

        if (spellId==i_spellId)
            continue;

        if(i_spellProto->IsPassive())
        {
            if(IsPassiveStackableSpell(i_spellId))
                continue;

            // passive non-stackable spells not stackable only with another rank of same spell
            if (!sSpellMgr->IsRankSpellDueToSpell(spellProto, i_spellId))
                continue;
        }

        uint32 i_effIndex = (*i).second->GetEffIndex();

        bool is_triggered_by_spell = false;
        // prevent triggered aura of removing aura that triggered it
        for(int j = 0; j < 3; ++j)
            if (i_spellProto->Effects[j].TriggerSpell == spellProto->Id)
                is_triggered_by_spell = true;
        if (is_triggered_by_spell) continue;

        for(int j = 0; j < 3; ++j)
        {
            // prevent remove dummy triggered spells at next effect aura add
            switch(spellProto->Effects[j].Effect)                   // main spell auras added added after triggered spell
            {
                case SPELL_EFFECT_DUMMY:
                    switch(spellId)
                    {
                        case 5420: if(i_spellId==34123) is_triggered_by_spell = true; break;
                    }
                    break;
            }

            if(is_triggered_by_spell)
                break;

            // prevent remove form main spell by triggered passive spells
            switch(i_spellProto->Effects[j].ApplyAuraName)    // main aura added before triggered spell
            {
                case SPELL_AURA_MOD_SHAPESHIFT:
                    switch(i_spellId)
                    {
                        case 24858: if(spellId==24905)                  is_triggered_by_spell = true; break;
                        case 33891: if(spellId==5420 || spellId==34123) is_triggered_by_spell = true; break;
                        case 34551: if(spellId==22688)                  is_triggered_by_spell = true; break;
                    }
                    break;
            }
        }

        if(!is_triggered_by_spell)
        {
            bool sameCaster = Aur->GetCasterGUID() == (*i).second->GetCasterGUID();
            if( sSpellMgr->IsNoStackSpellDueToSpell(spellId, i_spellId, sameCaster) )
            {
                //some spells should be not removed by lower rank of them (totem, paladin aura)
                if (!sameCaster
                    &&(spellProto->Effects[effIndex].Effect==SPELL_EFFECT_APPLY_AREA_AURA_PARTY)
                    &&(spellProto->DurationEntry && spellProto->DurationEntry->ID == 21) //lolwhat ?
                    &&(sSpellMgr->IsRankSpellDueToSpell(spellProto, i_spellId))
                    &&(CompareAuraRanks(spellId, effIndex, i_spellId, i_effIndex) < 0))
                    return false;

                // Its a parent aura (create this aura in ApplyModifier)
                if ((*i).second->IsInUse())
                {
                    TC_LOG_ERROR("spell","Aura (Spell %u Effect %u) is in process but attempt removed at aura (Spell %u Effect %u) adding, need add stack rule for Unit::RemoveNoStackAurasDueToAura", i->second->GetId(), i->second->GetEffIndex(),Aur->GetId(), Aur->GetEffIndex());
                    continue;
                }

                uint64 caster = (*i).second->GetCasterGUID();
                // Remove all auras by aura caster
                for (uint8 a=0;a<3;++a)
                {
                    spellEffectPair spair = spellEffectPair(i_spellId, a);
                    for(AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair);)
                    {
                        if(iter->second->GetCasterGUID()==caster)
                        {
                            RemoveAura(iter, AURA_REMOVE_BY_STACK);
                            iter = m_Auras.lower_bound(spair);
                        }
                        else
                            ++iter;
                    }
                }

                if( m_Auras.empty() )
                    break;
                else
                    next =  m_Auras.begin();
            }
        }
    }
    return true;
}

void Unit::RemoveAura(uint32 spellId, uint32 effindex, Aura* except)
{
    spellEffectPair spair = spellEffectPair(spellId, effindex);
    for(AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair);)
    {
        if(iter->second!=except)
        {
            RemoveAura(iter);
            iter = m_Auras.lower_bound(spair);
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasByCasterSpell(uint32 spellId, uint32 effindex, uint64 casterGUID)
{
    spellEffectPair spair = spellEffectPair(spellId, effindex);
    for (AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair);)
    {
        if (iter->second->GetCasterGUID() == casterGUID)
        {
            RemoveAura(iter);
            iter = m_Auras.upper_bound(spair);          // overwrite by more appropriate
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasByCasterSpell(uint32 spellId, uint64 casterGUID)
{
    for(int k = 0; k < 3; ++k)
    {
        spellEffectPair spair = spellEffectPair(spellId, k);
        for (AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair);)
        {
            if (iter->second->GetCasterGUID() == casterGUID)
            {
                RemoveAura(iter);
                iter = m_Auras.upper_bound(spair);          // overwrite by more appropriate
            }
            else
                ++iter;
        }
    }
}

void Unit::SetAurasDurationByCasterSpell(uint32 spellId, uint64 casterGUID, int32 duration)
{
    for(uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        spellEffectPair spair = spellEffectPair(spellId, i);
        for(AuraMap::const_iterator itr = m_Auras.lower_bound(spair); itr != m_Auras.upper_bound(spair); ++itr)
        {
            if(itr->second->GetCasterGUID()==casterGUID)
            {
                itr->second->SetAuraDuration(duration);
                break;
            }
        }
    }
}

Aura* Unit::GetAuraByCasterSpell(uint32 spellId, uint64 casterGUID)
{
    // Returns first found aura from spell-use only in cases where effindex of spell doesn't matter!
    for(uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        spellEffectPair spair = spellEffectPair(spellId, i);
        for(AuraMap::const_iterator itr = m_Auras.lower_bound(spair); itr != m_Auras.upper_bound(spair); ++itr)
        {
            if(itr->second->GetCasterGUID()==casterGUID)
                return itr->second;
        }
    }
    return NULL;
}

Aura* Unit::GetAuraByCasterSpell(uint32 spellId, uint32 effIndex, uint64 casterGUID)
{
    // Returns first found aura from spell-use only in cases where effindex of spell doesn't matter!
    spellEffectPair spair = spellEffectPair(spellId, effIndex);
    for(AuraMap::const_iterator itr = m_Auras.lower_bound(spair); itr != m_Auras.upper_bound(spair); ++itr)
    {
        if(itr->second->GetCasterGUID() == casterGUID)
            return itr->second;
    }
    return NULL;
}

void Unit::RemoveAurasDueToSpellByDispel(uint32 spellId, uint64 casterGUID, Unit *dispeler)
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end(); )
    {
        Aura *aur = iter->second;
        if (aur->GetId() == spellId && aur->GetCasterGUID() == casterGUID)
        {
            // Custom dispel case
            // Unstable Affliction
            if (aur->GetSpellInfo()->SpellFamilyName == SPELLFAMILY_WARLOCK && (aur->GetSpellInfo()->SpellFamilyFlags & 0x010000000000LL))
            {
                int32 damage = aur->GetModifier()->m_amount*9;
                uint64 caster_guid = aur->GetCasterGUID();

                // Remove aura
                RemoveAura(iter, AURA_REMOVE_BY_DISPEL);

                // backfire damage and silence
                dispeler->CastCustomSpell(dispeler, 31117, &damage, NULL, NULL, true, NULL, NULL,caster_guid);

                iter = m_Auras.begin();                     // iterator can be invalidate at cast if self-dispel
            }
            else
                RemoveAura(iter, AURA_REMOVE_BY_DISPEL);
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasDueToSpellBySteal(uint32 spellId, uint64 casterGUID, Unit *stealer)
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end(); )
    {
        Aura *aur = iter->second;
        if (aur->GetId() == spellId && aur->GetCasterGUID() == casterGUID)
        {
            //int32 basePoints = aur->GetBasePoints();
            // construct the new aura for the attacker
            Aura * new_aur = CreateAura(aur->GetSpellInfo(), aur->GetEffIndex(), NULL/*&basePoints*/, stealer);
            if(!new_aur)
                continue;

            // set its duration and maximum duration
            // max duration 2 minutes (in msecs)
            int32 dur = aur->GetAuraDuration();
            const int32 max_dur = 2*MINUTE*1000;
            new_aur->SetAuraMaxDuration( max_dur > dur ? dur : max_dur );
            new_aur->SetAuraDuration( max_dur > dur ? dur : max_dur );

            // Unregister _before_ adding to stealer
            aur->UnregisterSingleCastAura();
            // strange but intended behaviour: Stolen single target auras won't be treated as single targeted
            new_aur->SetIsSingleTarget(false);
            // add the new aura to stealer
            stealer->AddAura(new_aur);
            // Remove aura as dispel
            if (iter->second->GetStackAmount() > 1) {
                // reapply modifier with reduced stack amount
                iter->second->ApplyModifier(false, true);
                iter->second->SetStackAmount(iter->second->GetStackAmount() - 1);
                iter->second->ApplyModifier(true, true);
                iter->second->UpdateSlotCounterAndDuration();
                ++iter;
            }
            else
                RemoveAura(iter, AURA_REMOVE_BY_DISPEL);
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasDueToSpellByCancel(uint32 spellId)
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end(); )
    {
        if (iter->second->GetId() == spellId)
            RemoveAura(iter, AURA_REMOVE_BY_CANCEL);
        else
            ++iter;
    }
}

void Unit::RemoveAurasWithAttribute(uint32 flags)
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        SpellInfo const *spell = iter->second->GetSpellInfo();
        if (spell->Attributes & flags)
            RemoveAura(iter);
        else
            ++iter;
    }
}

void Unit::RemoveAurasWithCustomAttribute(SpellCustomAttributes flags)
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();) {
        SpellInfo const *spell = iter->second->GetSpellInfo();
        if (spell->HasAttribute(flags))
            RemoveAura(iter);
        else
            ++iter;
    }
}

void Unit::RemoveAurasWithDispelType( DispelType type )
{
    // Create dispel mask by dispel type
    uint32 dispelMask = GetDispellMask(type);
    // Dispel all existing auras vs current dispel type
    for(AuraMap::iterator itr = m_Auras.begin(); itr != m_Auras.end(); )
    {
        SpellInfo const* spell = itr->second->GetSpellInfo();
        if( (1<<spell->Dispel) & dispelMask )
        {
            // Dispel aura
            RemoveAurasDueToSpell(spell->Id);
            itr = m_Auras.begin();
        }
        else
            ++itr;
    }
}

bool Unit::RemoveAurasWithSpellFamily(uint32 spellFamilyName, uint8 count, bool withPassive)
{
    uint8 myCount = count;
    bool ret = false;
    for(AuraMap::iterator itr = m_Auras.begin(); itr != m_Auras.end() && myCount > 0; )
    {
        SpellInfo const* spell = itr->second->GetSpellInfo();
        if (spell->SpellFamilyName == spellFamilyName && spell->IsPositive())
        {
            if (spell->IsPassive() && !withPassive)
                ++itr;
            else {
                RemoveAurasDueToSpell(spell->Id);
                itr = m_Auras.begin();
                myCount--;
                ret = true;
            }
        }
        else
            ++itr;
    }
    
    return ret;
}

void Unit::RemoveSingleAuraFromStackByDispel(uint32 spellId)
{
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end(); )
    {
        Aura *aur = iter->second;
        if (aur->GetId() == spellId)
        {
            if(iter->second->GetStackAmount() > 1)
            {
                // reapply modifier with reduced stack amount
                iter->second->ApplyModifier(false,true);
                iter->second->SetStackAmount(iter->second->GetStackAmount()-1);
                iter->second->ApplyModifier(true,true);

                iter->second->UpdateSlotCounterAndDuration();
                return; // not remove aura if stack amount > 1
            }
            else
                RemoveAura(iter,AURA_REMOVE_BY_DISPEL);
        }
        else
            ++iter;
    }
}

void Unit::RemoveSingleAuraFromStack(uint32 spellId, uint32 effindex)
{
    AuraMap::iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if(iter != m_Auras.end())
    {
        if(iter->second->GetStackAmount() > 1)
        {
            // reapply modifier with reduced stack amount
            iter->second->ApplyModifier(false,true);
            iter->second->SetStackAmount(iter->second->GetStackAmount()-1);
            iter->second->ApplyModifier(true,true);

            iter->second->UpdateSlotCounterAndDuration();
            return; // not remove aura if stack amount > 1
        }
        RemoveAura(iter);
    }
}

void Unit::RemoveAurasDueToSpell(uint32 spellId, Aura* except)
{
    for (int i = 0; i < MAX_SPELL_EFFECTS; ++i)
        RemoveAura(spellId,i,except);
}

void Unit::RemoveAurasDueToItemSpell(Item* castItem,uint32 spellId)
{
    for (int k=0; k < 3; ++k)
    {
        spellEffectPair spair = spellEffectPair(spellId, k);
        for (AuraMap::iterator iter = m_Auras.lower_bound(spair); iter != m_Auras.upper_bound(spair);)
        {
            if (iter->second->GetCastItemGUID() == castItem->GetGUID())
            {
                RemoveAura(iter);
                iter = m_Auras.upper_bound(spair);          // overwrite by more appropriate
            }
            else
                ++iter;
        }
    }
}

void Unit::RemoveNotOwnSingleTargetAuras()
{
    m_GiantLock.lock();
    // single target auras from other casters
    for (AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end(); )
    {
        if (iter->second->GetCasterGUID()!=GetGUID() && iter->second->GetSpellInfo()->IsSingleTarget())
            RemoveAura(iter);
        else
            ++iter;
    }

    // single target auras at other targets
    AuraList& scAuras = GetSingleCastAuras();
    for (AuraList::iterator iter = scAuras.begin(); iter != scAuras.end(); )
    {
        Aura* aur = *iter;
        ++iter;
        if (aur->GetTarget()!=this)
        {
            uint32 removedAuras = m_removedAurasCount;
            aur->GetTarget()->RemoveAura( aur->GetId(),aur->GetEffIndex() );
            if (m_removedAurasCount > removedAuras + 1)
                iter = scAuras.begin();
        }
    }
    m_GiantLock.unlock();
}

void Unit::RemoveAura(AuraMap::iterator &i, AuraRemoveMode mode)
{
    Aura* Aur = i->second;

    // if unit currently update aura list then make safe update iterator shift to next
    if (m_AurasUpdateIterator == i)
        ++m_AurasUpdateIterator;

    // some ShapeshiftBoosts at remove trigger removing other auras including parent Shapeshift aura
    // remove aura from list before to prevent deleting it before
    m_Auras.erase(i);
    ++m_removedAurasCount;

    SpellInfo const* AurSpellInfo = Aur->GetSpellInfo();
    Unit* caster = NULL;
    Aur->UnregisterSingleCastAura();

    // remove from list before mods removing (prevent cyclic calls, mods added before including to aura list - use reverse order)
    if (Aur->GetModifier()->m_auraname < TOTAL_AURAS)
    {
        m_modAuras[Aur->GetModifier()->m_auraname].remove(Aur);

        if(Aur->GetSpellInfo()->AuraInterruptFlags)
        {
            m_interruptableAuras.remove(Aur);
            UpdateInterruptMask();
        }

        if((Aur->GetSpellInfo()->Attributes & SPELL_ATTR0_HEARTBEAT_RESIST_CHECK)
            && (Aur->GetModifier()->m_auraname != SPELL_AURA_MOD_POSSESS)) //only dummy aura is breakable
        {
            m_ccAuras.remove(Aur);
        }
    }

    bool auraStateFound = false;
    AuraStateType auraState = Aur->GetSpellInfo()->GetAuraState();
    if (auraState)
    {
        bool canBreak = false;
        // Get mask of all aurastates from remaining auras
        for (AuraStateAurasMap::iterator itr = m_auraStateAuras.lower_bound(auraState); itr != m_auraStateAuras.upper_bound(auraState) && !(auraStateFound && canBreak);)
        {
            if (itr->second == Aur)
            {
                m_auraStateAuras.erase(itr);
                itr = m_auraStateAuras.lower_bound(auraState);
                canBreak = true;
                continue;
            }
            auraStateFound = true;
            ++itr;
        }
    }
    
    // Remove aurastates only if were not found
    if (!auraStateFound)
        ModifyAuraState(auraState, false);
              
    // Set remove mode
    Aur->SetRemoveMode(mode);

    // Statue unsummoned at aura remove
    Totem* statue = NULL;
    if(Aur->GetAuraDuration() && !Aur->IsPersistent() && AurSpellInfo->IsChanneled())
    {
        if(!caster)                                         // can be already located for IsSingleTargetSpell case
            caster = Aur->GetCaster();

        if(caster && caster->IsAlive())
        {
            // stop caster chanelling state
            if(caster->m_currentSpells[CURRENT_CHANNELED_SPELL]
                //prevent recurential call
                && caster->m_currentSpells[CURRENT_CHANNELED_SPELL]->getState() != SPELL_STATE_FINISHED)
            {
                if (caster==this || !IsAreaOfEffectSpell(AurSpellInfo))
                {
                    // remove auras only for non-aoe spells or when chanelled aura is removed
                    // because aoe spells don't require aura on target to continue
                    if (AurSpellInfo->Effects[Aur->GetEffIndex()].ApplyAuraName != SPELL_AURA_PERIODIC_DUMMY
                        && AurSpellInfo->Effects[Aur->GetEffIndex()].ApplyAuraName != SPELL_AURA_DUMMY)
                        //don't stop channeling of scripted spells (this is actually a hack)
                    {
                        caster->m_currentSpells[CURRENT_CHANNELED_SPELL]->cancel();
                        caster->m_currentSpells[CURRENT_CHANNELED_SPELL]=NULL;
            
                    }
                }

                if(caster->GetTypeId()==TYPEID_UNIT && (caster->ToCreature())->IsTotem() && ((Totem*)caster)->GetTotemType()==TOTEM_STATUE)
                    statue = ((Totem*)caster);
            }

            // Unsummon summon as possessed creatures on spell cancel
            if(caster->GetTypeId() == TYPEID_PLAYER)
            {
                for(int i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    if(AurSpellInfo->Effects[i].Effect == SPELL_EFFECT_SUMMON &&
                        (AurSpellInfo->Effects[i].MiscValueB == SUMMON_TYPE_POSESSED ||
                         AurSpellInfo->Effects[i].MiscValueB == SUMMON_TYPE_POSESSED2 ||
                         AurSpellInfo->Effects[i].MiscValueB == SUMMON_TYPE_POSESSED3))
                    {
                        (caster->ToPlayer())->StopCastingCharm();
                        break;
                    }
                }
            }
        }
    }

    assert(!Aur->IsInUse());
    Aur->ApplyModifier(false,true);

    Aur->SetStackAmount(0);

    // set aura to be removed during unit::_updatespells
    m_removedAuras.push_back(Aur);

    Aur->_RemoveAura();

    bool stack = false;
    spellEffectPair spair = spellEffectPair(Aur->GetId(), Aur->GetEffIndex());
    for(AuraMap::const_iterator itr = GetAuras().lower_bound(spair); itr != GetAuras().upper_bound(spair); ++itr)
    {
        if (itr->second->GetCasterGUID()==GetGUID())
        {
            stack = true;
        }
    }
    if (!stack)
    {
        // Remove all triggered by aura spells vs unlimited duration
        Aur->CleanupTriggeredSpells();

        // Remove Linked Auras
        uint32 id = Aur->GetId();
        if(Aur->GetSpellInfo()->HasAttribute(SPELL_ATTR_CU_LINK_REMOVE))
        {
            if(const std::vector<int32> *spell_triggered = sSpellMgr->GetSpellLinked(-(int32)id))
            {
                for(std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                    if(*itr < 0)
                        RemoveAurasDueToSpell(-(*itr));
                    else if(Unit* caster = Aur->GetCaster())
                        CastSpell(this, *itr, true, 0, 0, caster->GetGUID());
            }
        }
        if(Aur->GetSpellInfo()->HasAttribute(SPELL_ATTR_CU_LINK_AURA))
        {
            if(const std::vector<int32> *spell_triggered = sSpellMgr->GetSpellLinked(id + SPELL_LINK_AURA))
            {
                for(std::vector<int32>::const_iterator itr = spell_triggered->begin(); itr != spell_triggered->end(); ++itr)
                    if(*itr < 0)
                        ApplySpellImmune(id, IMMUNITY_ID, -(*itr), false);
                    else
                        RemoveAurasDueToSpell(*itr);
            }
        }
    }

    if(statue)
        statue->UnSummon();

    i = m_Auras.begin();
}

void Unit::RemoveAllAuras()
{
    while (!m_Auras.empty())
    {
        AuraMap::iterator iter = m_Auras.begin();
        RemoveAura(iter);
    }

    m_Auras.clear();
}

void Unit::RemoveAllAurasExcept(uint32 spellId)
{
    AuraMap::iterator iter = m_Auras.begin();
    while (iter != m_Auras.end())
    {
        if(!(iter->second->GetId() == spellId))
            RemoveAura(iter);
        else 
            iter++;
    }
}

void Unit::RemoveArenaAuras(bool onleave)
{
    // in join, remove positive buffs, on end, remove negative
    // used to remove positive visible auras in arenas
    for(AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        if  (  !(iter->second->GetSpellInfo()->HasAttribute(SPELL_ATTR4_UNK21)) // don't remove stances, shadowform, pally/hunter auras
            && !iter->second->IsPassive()                               // don't remove passive auras
            && (!(iter->second->GetSpellInfo()->Attributes & SPELL_ATTR2_PRESERVE_ENCHANT_IN_ARENA))
            && (!onleave || !iter->second->IsPositive())                // remove all buffs on enter, negative buffs on leave
            && (iter->second->IsPositive() || !(iter->second->GetSpellInfo()->HasAttribute(SPELL_ATTR3_DEATH_PERSISTENT))) //dont remove death persistent debuff such as deserter
            )
            RemoveAura(iter);
        else
            ++iter;
    }

    if (Player* plr = ToPlayer()) {
        if (Pet* pet = GetPet())
            pet->RemoveArenaAuras(onleave);
        else
            plr->RemoveAllCurrentPetAuras(); //still remove auras if the players hasnt called his pet yet
    }
}

void Unit::RemoveAllAurasOnDeath()
{
    // used just after dying to remove all visible auras
    // and disable the mods for the passive ones
    for(AuraMap::iterator iter = m_Auras.begin(); iter != m_Auras.end();)
    {
        if (!iter->second->IsPassive() && !iter->second->IsDeathPersistent())
            RemoveAura(iter, AURA_REMOVE_BY_DEATH);
        else
            ++iter;
    }
}

void Unit::DelayAura(uint32 spellId, uint32 effindex, int32 delaytime)
{
    AuraMap::const_iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if (iter != m_Auras.end())
    {
        if (iter->second->GetAuraDuration() < delaytime)
            iter->second->SetAuraDuration(0);
        else
            iter->second->SetAuraDuration(iter->second->GetAuraDuration() - delaytime);
        iter->second->UpdateAuraDuration();
    }
}

void Unit::_RemoveAllAuraMods()
{
    for (AuraMap::const_iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        (*i).second->ApplyModifier(false);
    }
}

void Unit::_ApplyAllAuraMods()
{
    for (AuraMap::const_iterator i = m_Auras.begin(); i != m_Auras.end(); ++i)
    {
        (*i).second->ApplyModifier(true);
    }
}

Aura* Unit::GetAura(uint32 spellId, uint32 effindex)
{
    AuraMap::const_iterator iter = m_Auras.find(spellEffectPair(spellId, effindex));
    if (iter != m_Auras.end())
        return iter->second;
    return NULL;
}

void Unit::AddDynObject(DynamicObject* dynObj)
{
    m_dynObjGUIDs.push_back(dynObj->GetGUID());
}

void Unit::RemoveDynObject(uint32 spellid)
{
    if(m_dynObjGUIDs.empty())
        return;
    for (DynObjectGUIDs::iterator i = m_dynObjGUIDs.begin(); i != m_dynObjGUIDs.end();)
    {
        DynamicObject* dynObj = ObjectAccessor::GetDynamicObject(*this, *i);
        if(!dynObj) // may happen if a dynobj is removed when grid unload
        {
            i = m_dynObjGUIDs.erase(i);
        }
        else if(spellid == 0 || dynObj->GetSpellId() == spellid)
        {
            dynObj->Delete();
            i = m_dynObjGUIDs.erase(i);
        }
        else
            ++i;
    }
}

void Unit::RemoveAllDynObjects()
{
    while(!m_dynObjGUIDs.empty())
    {
        DynamicObject* dynObj = ObjectAccessor::GetDynamicObject(*this,*m_dynObjGUIDs.begin());
        if(dynObj)
            dynObj->Delete();
        m_dynObjGUIDs.erase(m_dynObjGUIDs.begin());
    }
}

DynamicObject * Unit::GetDynObject(uint32 spellId, uint32 effIndex)
{
    for (DynObjectGUIDs::iterator i = m_dynObjGUIDs.begin(); i != m_dynObjGUIDs.end();)
    {
        DynamicObject* dynObj = ObjectAccessor::GetDynamicObject(*this, *i);
        if(!dynObj)
        {
            i = m_dynObjGUIDs.erase(i);
            continue;
        }

        if (dynObj->GetSpellId() == spellId && dynObj->GetEffIndex() == effIndex)
            return dynObj;
        ++i;
    }
    return NULL;
}

DynamicObject * Unit::GetDynObject(uint32 spellId)
{
    for (DynObjectGUIDs::iterator i = m_dynObjGUIDs.begin(); i != m_dynObjGUIDs.end();)
    {
        DynamicObject* dynObj = ObjectAccessor::GetDynamicObject(*this, *i);
        if(!dynObj)
        {
            i = m_dynObjGUIDs.erase(i);
            continue;
        }

        if (dynObj->GetSpellId() == spellId)
            return dynObj;
        ++i;
    }
    return NULL;
}

void Unit::AddGameObject(GameObject* gameObj)
{
    assert(gameObj && gameObj->GetOwnerGUID()==0);
    m_gameObj.push_back(gameObj);
    gameObj->SetOwnerGUID(GetGUID());
}

void Unit::RemoveGameObject(GameObject* gameObj, bool del)
{
    assert(gameObj && gameObj->GetOwnerGUID()==GetGUID());

    // GO created by some spell
    if ( GetTypeId()==TYPEID_PLAYER && gameObj->GetSpellId() )
    {
        SpellInfo const* createBySpell = sSpellMgr->GetSpellInfo(gameObj->GetSpellId());
        // Need activate spell use for owner
        if (createBySpell && createBySpell->Attributes & SPELL_ATTR0_DISABLED_WHILE_ACTIVE)
            (this->ToPlayer())->SendCooldownEvent(createBySpell);
    }
    gameObj->SetOwnerGUID(0);
    m_gameObj.remove(gameObj);
    if(del)
    {
        gameObj->SetRespawnTime(0);
        gameObj->Delete();
    }
}

void Unit::RemoveGameObject(uint32 spellid, bool del)
{
    if(m_gameObj.empty())
        return;
    std::list<GameObject*>::iterator i, next;
    for (i = m_gameObj.begin(); i != m_gameObj.end(); i = next)
    {
        next = i;
        if(spellid == 0 || (*i)->GetSpellId() == spellid)
        {
            (*i)->SetOwnerGUID(0);
            if(del)
            {
                (*i)->SetRespawnTime(0);
                (*i)->Delete();
            }

            next = m_gameObj.erase(i);
        }
        else
            ++next;
    }
}

void Unit::RemoveAllGameObjects()
{
    // remove references to unit
    for(std::list<GameObject*>::iterator i = m_gameObj.begin(); i != m_gameObj.end();)
    {
        (*i)->SetOwnerGUID(0);
        (*i)->SetRespawnTime(0);
        (*i)->Delete();
        i = m_gameObj.erase(i);
    }
}

void Unit::SendSpellNonMeleeDamageLog(SpellNonMeleeDamage *log)
{
    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, (16+4+4+1+4+4+1+1+4+4+1)); // we guess size
    data << log->target->GetPackGUID();
    data << log->attacker->GetPackGUID();
    data << uint32(log->SpellID);
    data << uint32(log->damage);                             //damage amount
    data << uint8 (log->schoolMask);                         //damage school
    data << uint32(log->absorb);                             //AbsorbedDamage
    data << uint32(log->resist);                             //resist
    data << uint8 (log->physicalLog);                        // damsge type? flag
    data << uint8 (log->unused);                             //unused
    data << uint32(log->blocked);                            //blocked
    data << uint32(log->HitInfo);
    data << uint8 (0);                                       // flag to use extend data
    SendMessageToSet( &data, true );
}

void Unit::SendSpellNonMeleeDamageLog(Unit *target,uint32 SpellID,uint32 Damage, SpellSchoolMask damageSchoolMask,uint32 AbsorbedDamage, uint32 Resist,bool PhysicalDamage, uint32 Blocked, bool CriticalHit)
{
    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, (16+4+4+1+4+4+1+1+4+4+1)); // we guess size
    data << target->GetPackGUID();
    data << GetPackGUID();
    data << uint32(SpellID);
    data << uint32(Damage-AbsorbedDamage-Resist-Blocked);
    data << uint8(damageSchoolMask);                        // spell school
    data << uint32(AbsorbedDamage);                         // AbsorbedDamage
    data << uint32(Resist);                                 // resist
    data << uint8(PhysicalDamage);                          // if 1, then client show spell name (example: %s's ranged shot hit %s for %u school or %s suffers %u school damage from %s's spell_name
    data << uint8(0);                                       // unk IsFromAura
    data << uint32(Blocked);                                // blocked
    data << uint32(CriticalHit ? 0x27 : 0x25);              // hitType, flags: 0x2 - SPELL_HIT_TYPE_CRIT, 0x10 - replace caster?
    data << uint8(0);                                       // isDebug?
    SendMessageToSet( &data, true );
}

void Unit::ProcDamageAndSpell(Unit *pVictim, uint32 procAttacker, uint32 procVictim, uint32 procExtra, uint32 amount, WeaponAttackType attType, SpellInfo const *procSpell, bool canTrigger)
{
     // Not much to do if no flags are set.
    if (procAttacker && canTrigger)
        ProcDamageAndSpellFor(false,pVictim,procAttacker, procExtra,attType, procSpell, amount);
    // Now go on with a victim's events'n'auras
    // Not much to do if no flags are set or there is no victim
    if(pVictim && pVictim->IsAlive() && procVictim)
        pVictim->ProcDamageAndSpellFor(true,this,procVictim, procExtra, attType, procSpell, amount);
}

void Unit::SendSpellMiss(Unit *target, uint32 spellID, SpellMissInfo missInfo)
{
    WorldPacket data(SMSG_SPELLLOGMISS, (4+8+1+4+8+1));
    data << uint32(spellID);
    data << uint64(GetGUID());
    data << uint8(0);                                       // can be 0 or 1
    data << uint32(1);                                      // target count
    // for(i = 0; i < target count; ++i)
    data << uint64(target->GetGUID());                      // target GUID
    data << uint8(missInfo);
    // end loop
    SendMessageToSet(&data, true);
}

void Unit::SendAttackStateUpdate(CalcDamageInfo *damageInfo)
{
    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, (16+84));    // we guess size
    data << (uint32)damageInfo->HitInfo;
    data << GetPackGUID();
    data << damageInfo->target->GetPackGUID();
    data << (uint32)(damageInfo->damage);     // Full damage

    data << (uint8)1;                         // Sub damage count
    //===  Sub damage description
    data << (uint32)(damageInfo->damageSchoolMask); // School of sub damage
    data << (float)damageInfo->damage;        // sub damage
    data << (uint32)damageInfo->damage;       // Sub Damage
    data << (uint32)damageInfo->absorb;       // Absorb
    data << (uint32)damageInfo->resist;       // Resist
    //=================================================
    data << (uint32)damageInfo->TargetState;
    data << (uint32)0;
    data << (uint32)0;
    data << (uint32)damageInfo->blocked_amount;
    SendMessageToSet( &data, true );/**/
}

void Unit::SendAttackStateUpdate(uint32 HitInfo, Unit *target, uint8 SwingType, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, VictimState TargetState, uint32 BlockedAmount)
{
    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, (16+45));    // we guess size
    data << (uint32)HitInfo;
    data << GetPackGUID();
    data << target->GetPackGUID();
    data << (uint32)(Damage-AbsorbDamage-Resist-BlockedAmount);

    data << (uint8)SwingType;                               // count?

    // for(i = 0; i < SwingType; ++i)
    data << (uint32)damageSchoolMask;
    data << (float)(Damage-AbsorbDamage-Resist-BlockedAmount);
    // still need to double check damage
    data << (uint32)(Damage-AbsorbDamage-Resist-BlockedAmount);
    data << (uint32)AbsorbDamage;
    data << (uint32)Resist;
    // end loop

    data << (uint32)TargetState;

    if( AbsorbDamage == 0 )                                 //also 0x3E8 = 0x3E8, check when that happens
        data << (uint32)0;
    else
        data << (uint32)-1;

    data << (uint32)0;
    data << (uint32)BlockedAmount;

    SendMessageToSet( &data, true );
}

bool Unit::HandleHasteAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellInfo const * procSpell, uint32 /*procFlag*/, uint32 /*procEx*/, uint32 cooldown)
{
    SpellInfo const *hasteSpell = triggeredByAura->GetSpellInfo();

    Item* castItem = triggeredByAura->GetCastItemGUID() && GetTypeId()==TYPEID_PLAYER
        ? (this->ToPlayer())->GetItemByGuid(triggeredByAura->GetCastItemGUID()) : NULL;

    uint32 triggered_spell_id = 0;
    Unit* target = pVictim;
    int32 basepoints0 = 0;

    switch(hasteSpell->SpellFamilyName)
    {
        case SPELLFAMILY_ROGUE:
        {
            switch(hasteSpell->Id)
            {
                // Blade Flurry
                case 13877:
                case 33735:
                {
                    target = SelectNearbyTarget();
                    if(!target)
                        return false;
                    basepoints0 = damage;
                    triggered_spell_id = 22482;
                    break;
                }
            }
            break;
        }
    }

    // processed charge only counting case
    if(!triggered_spell_id)
        return true;

    SpellInfo const* triggerEntry = sSpellMgr->GetSpellInfo(triggered_spell_id);

    if(!triggerEntry)
    {
        TC_LOG_ERROR("FIXME","Unit::HandleHasteAuraProc: Spell %u have not existed triggered spell %u",hasteSpell->Id,triggered_spell_id);
        return false;
    }

    // default case
    if(!target || (target!=this && !target->IsAlive()))
        return false;

    if( cooldown && GetTypeId()==TYPEID_PLAYER && (this->ToPlayer())->HasSpellCooldown(triggered_spell_id))
        return false;

    if(basepoints0)
        CastCustomSpell(target,triggered_spell_id,&basepoints0,NULL,NULL,true,castItem,triggeredByAura);
    else
        CastSpell(target,triggered_spell_id,true,castItem,triggeredByAura);

    if( cooldown && GetTypeId()==TYPEID_PLAYER )
        (this->ToPlayer())->AddSpellCooldown(triggered_spell_id,0,time(NULL) + cooldown);

    return true;
}

bool Unit::HandleDummyAuraProc(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellInfo const * procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown)
{
    SpellInfo const *dummySpell = triggeredByAura->GetSpellInfo();
    uint32 effIndex = triggeredByAura->GetEffIndex ();

    Item* castItem = triggeredByAura->GetCastItemGUID() && GetTypeId()==TYPEID_PLAYER
        ? (this->ToPlayer())->GetItemByGuid(triggeredByAura->GetCastItemGUID()) : NULL;

    uint32 triggered_spell_id = 0;
    Unit* target = pVictim;
    int32 basepoints0 = 0;

    switch(dummySpell->SpellFamilyName)
    {
        case SPELLFAMILY_GENERIC:
        {
            switch (dummySpell->Id)
            {
                // Eye of Eye
                case 9799:
                case 25988:
                {
                    // prevent damage back from weapon special attacks
                    if (!procSpell || procSpell->DmgClass != SPELL_DAMAGE_CLASS_MAGIC )
                        return false;

                    // return damage % to attacker but < 50% own total health
                    basepoints0 = triggeredByAura->GetModifier()->m_amount*int32(damage)/100;
                    if(basepoints0 > GetMaxHealth()/2)
                        basepoints0 = GetMaxHealth()/2;

                    triggered_spell_id = 25997;
                    break;
                }
                // Sweeping Strikes
                case 12328:
                case 18765:
                case 35429:
                {
                    // prevent chain of triggered spell from same triggered spell
                    if(procSpell && (procSpell->Id==12723 || procSpell->Id==1680 || procSpell->Id==25231))
                        return false;
                        
                    if (procSpell && procSpell->SpellFamilyFlags & 0x420400000LL)    // Execute && Whirlwind && Cleave
                        return false;

                    target = SelectNearbyTarget();
                    if(!target)
                        return false;

                    triggered_spell_id = 12723;
                    basepoints0 = damage;
                    break;
                }
                // Sword Specialization: shouldn't proc from same spell or from windfury
                case 12281:
                case 12812:
                case 12813:
                case 12814:
                case 12815:
                case 13960:
                case 13961:
                case 13962:
                case 13963:
                case 13964:
                {
                    // Sword Spec
                    if (procSpell && procSpell->SpellIconID == 1462)
                        return false;
                        
                    // Windfury
                    if (procSpell && procSpell->SpellIconID == 1397)
                        return false;
                }
                // Unstable Power
                case 24658:
                {
                    if (!procSpell || procSpell->Id == 24659)
                        return false;
                    // Need remove one 24659 aura
                    RemoveSingleAuraFromStack(24659, 0);
                    RemoveSingleAuraFromStack(24659, 1);
                    return true;
                }
                // Restless Strength
                case 24661:
                {
                    // Need remove one 24662 aura
                    RemoveSingleAuraFromStack(24662, 0);
                    return true;
                }
                // Adaptive Warding (Frostfire Regalia set)
                case 28764:
                {
                    if(!procSpell)
                        return false;

                    // find Mage Armor
                    bool found = false;
                    AuraList const& mRegenInterupt = GetAurasByType(SPELL_AURA_MOD_MANA_REGEN_INTERRUPT);
                    for(AuraList::const_iterator iter = mRegenInterupt.begin(); iter != mRegenInterupt.end(); ++iter)
                    {
                        if(SpellInfo const* iterSpellProto = (*iter)->GetSpellInfo())
                        {
                            if(iterSpellProto->SpellFamilyName==SPELLFAMILY_MAGE && (iterSpellProto->SpellFamilyFlags & 0x10000000))
                            {
                                found=true;
                                break;
                            }
                        }
                    }
                    if(!found)
                        return false;

                    switch(GetFirstSchoolInMask(procSpell->GetSchoolMask()))
                    {
                        case SPELL_SCHOOL_NORMAL:
                        case SPELL_SCHOOL_HOLY:
                            return false;                   // ignored
                        case SPELL_SCHOOL_FIRE:   triggered_spell_id = 28765; break;
                        case SPELL_SCHOOL_NATURE: triggered_spell_id = 28768; break;
                        case SPELL_SCHOOL_FROST:  triggered_spell_id = 28766; break;
                        case SPELL_SCHOOL_SHADOW: triggered_spell_id = 28769; break;
                        case SPELL_SCHOOL_ARCANE: triggered_spell_id = 28770; break;
                        default:
                            return false;
                    }

                    target = this;
                    break;
                }
                // Obsidian Armor (Justice Bearer`s Pauldrons shoulder)
                case 27539:
                {
                    if(!procSpell)
                        return false;

                    switch(GetFirstSchoolInMask(procSpell->GetSchoolMask()))
                    {
                        case SPELL_SCHOOL_NORMAL:
                            return false;                   // ignore
                        case SPELL_SCHOOL_HOLY:   triggered_spell_id = 27536; break;
                        case SPELL_SCHOOL_FIRE:   triggered_spell_id = 27533; break;
                        case SPELL_SCHOOL_NATURE: triggered_spell_id = 27538; break;
                        case SPELL_SCHOOL_FROST:  triggered_spell_id = 27534; break;
                        case SPELL_SCHOOL_SHADOW: triggered_spell_id = 27535; break;
                        case SPELL_SCHOOL_ARCANE: triggered_spell_id = 27540; break;
                        default:
                            return false;
                    }

                    target = this;
                    break;
                }
                // Mana Leech (Passive) (Priest Pet Aura)
                case 28305:
                {
                    // Cast on owner
                    target = GetOwner();
                    if(!target)
                        return false;

                    basepoints0 = int32(damage * 2.5f);     // manaregen
                    triggered_spell_id = 34650;
                    break;
                }
                // Mark of Malice
                case 33493:
                {
                    // Cast finish spell at last charge
                    if (triggeredByAura->m_procCharges > 1)
                        return false;

                    target = this;
                    triggered_spell_id = 33494;
                    break;
                }
                // Twisted Reflection (boss spell)
                case 21063:
                    triggered_spell_id = 21064;
                    break;
                // Vampiric Aura (boss spell)
                case 38196:
                {
                    basepoints0 = 3 * damage;               // 300%
                    if (basepoints0 < 0)
                        return false;

                    triggered_spell_id = 31285;
                    target = this;
                    break;
                }
                // Aura of Madness (Darkmoon Card: Madness trinket)
                //=====================================================
                // 39511 Sociopath: +35 strength (Paladin, Rogue, Druid, Warrior)
                // 40997 Delusional: +70 attack power (Rogue, Hunter, Paladin, Warrior, Druid)
                // 40998 Kleptomania: +35 agility (Warrior, Rogue, Paladin, Hunter, Druid)
                // 40999 Megalomania: +41 damage/healing (Druid, Shaman, Priest, Warlock, Mage, Paladin)
                // 41002 Paranoia: +35 spell/melee/ranged crit strike rating (All classes)
                // 41005 Manic: +35 haste (spell, melee and ranged) (All classes)
                // 41009 Narcissism: +35 intellect (Druid, Shaman, Priest, Warlock, Mage, Paladin, Hunter)
                // 41011 Martyr Complex: +35 stamina (All classes)
                // 41406 Dementia: Every 5 seconds either gives you +5% damage/healing. (Druid, Shaman, Priest, Warlock, Mage, Paladin)
                // 41409 Dementia: Every 5 seconds either gives you -5% damage/healing. (Druid, Shaman, Priest, Warlock, Mage, Paladin)
                case 39446:
                {
                    if(GetTypeId() != TYPEID_PLAYER || !this->IsAlive())
                        return false;

                    // Select class defined buff
                    switch (GetClass())
                    {
                        case CLASS_PALADIN:                 // 39511,40997,40998,40999,41002,41005,41009,41011,41409
                        case CLASS_DRUID:                   // 39511,40997,40998,40999,41002,41005,41009,41011,41409
                        {
                            uint32 RandomSpell[]={39511,40997,40998,40999,41002,41005,41009,41011,41409};
                            triggered_spell_id = RandomSpell[ GetMap()->irand(0, sizeof(RandomSpell)/sizeof(uint32) - 1) ];
                            break;
                        }
                        case CLASS_ROGUE:                   // 39511,40997,40998,41002,41005,41011
                        case CLASS_WARRIOR:                 // 39511,40997,40998,41002,41005,41011
                        {
                            uint32 RandomSpell[]={39511,40997,40998,41002,41005,41011};
                            triggered_spell_id = RandomSpell[ GetMap()->irand(0, sizeof(RandomSpell)/sizeof(uint32) - 1) ];
                            break;
                        }
                        case CLASS_PRIEST:                  // 40999,41002,41005,41009,41011,41406,41409
                        case CLASS_SHAMAN:                  // 40999,41002,41005,41009,41011,41406,41409
                        case CLASS_MAGE:                    // 40999,41002,41005,41009,41011,41406,41409
                        case CLASS_WARLOCK:                 // 40999,41002,41005,41009,41011,41406,41409
                        {
                            uint32 RandomSpell[]={40999,41002,41005,41009,41011,41406,41409};
                            triggered_spell_id = RandomSpell[ GetMap()->irand(0, sizeof(RandomSpell)/sizeof(uint32) - 1) ];
                            break;
                        }
                        case CLASS_HUNTER:                  // 40997,40999,41002,41005,41009,41011,41406,41409
                        {
                            uint32 RandomSpell[]={40997,40999,41002,41005,41009,41011,41406,41409};
                            triggered_spell_id = RandomSpell[ GetMap()->irand(0, sizeof(RandomSpell)/sizeof(uint32) - 1) ];
                            break;
                        }
                        default:
                            return false;
                    }

                    target = this;
                    if (roll_chance_i(10))
                        (this->ToPlayer())->Say("This is Madness!", LANG_UNIVERSAL);
                    break;
                }
                /*
                // TODO: need find item for aura and triggered spells
                // Sunwell Exalted Caster Neck (??? neck)
                // cast ??? Light's Wrath if Exalted by Aldor
                // cast ??? Arcane Bolt if Exalted by Scryers*/
                case 46569:
                    return false;                           // disable for while
                /*
                {
                    if(GetTypeId() != TYPEID_PLAYER)
                        return false;

                    // Get Aldor reputation rank
                    if ((this->ToPlayer())->GetReputationRank(932) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = ???
                        break;
                    }
                    // Get Scryers reputation rank
                    if ((this->ToPlayer())->GetReputationRank(934) == REP_EXALTED)
                    {
                        triggered_spell_id = ???
                        break;
                    }
                    return false;
                } */
                // Sunwell Exalted Caster Neck (Shattered Sun Pendant of Acumen neck)
                // cast 45479 Light's Wrath if Exalted by Aldor
                // cast 45429 Arcane Bolt if Exalted by Scryers
                case 45481:
                {
                    if(GetTypeId() != TYPEID_PLAYER)
                        return false;

                    // Get Aldor reputation rank
                    if ((this->ToPlayer())->GetReputationRank(932) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = 45479;
                        break;
                    }
                    // Get Scryers reputation rank
                    if ((this->ToPlayer())->GetReputationRank(934) == REP_EXALTED)
                    {
                        if(this->IsFriendlyTo(target))
                            return false;

                        triggered_spell_id = 45429;
                        break;
                    }
                    return false;
                }
                // Sunwell Exalted Melee Neck (Shattered Sun Pendant of Might neck)
                // cast 45480 Light's Strength if Exalted by Aldor
                // cast 45428 Arcane Strike if Exalted by Scryers
                case 45482:
                {
                    if(GetTypeId() != TYPEID_PLAYER)
                        return false;

                    // Get Aldor reputation rank
                    if ((this->ToPlayer())->GetReputationRank(932) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = 45480;
                        break;
                    }
                    // Get Scryers reputation rank
                    if ((this->ToPlayer())->GetReputationRank(934) == REP_EXALTED)
                    {
                        triggered_spell_id = 45428;
                        break;
                    }
                    return false;
                }
                // Sunwell Exalted Tank Neck (Shattered Sun Pendant of Resolve neck)
                // cast 45431 Arcane Insight if Exalted by Aldor
                // cast 45432 Light's Ward if Exalted by Scryers
                case 45483:
                {
                    if(GetTypeId() != TYPEID_PLAYER)
                        return false;

                    // Get Aldor reputation rank
                    if ((this->ToPlayer())->GetReputationRank(932) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = 45432;
                        break;
                    }
                    // Get Scryers reputation rank
                    if ((this->ToPlayer())->GetReputationRank(934) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = 45431;
                        break;
                    }
                    return false;
                }
                // Sunwell Exalted Healer Neck (Shattered Sun Pendant of Restoration neck)
                // cast 45478 Light's Salvation if Exalted by Aldor
                // cast 45430 Arcane Surge if Exalted by Scryers
                case 45484:
                {
                    if(GetTypeId() != TYPEID_PLAYER)
                        return false;

                    // Get Aldor reputation rank
                    if ((this->ToPlayer())->GetReputationRank(932) == REP_EXALTED)
                    {
                        target = this;
                        triggered_spell_id = 45478;
                        break;
                    }
                    // Get Scryers reputation rank
                    if ((this->ToPlayer())->GetReputationRank(934) == REP_EXALTED)
                    {
                        triggered_spell_id = 45430;
                        break;
                    }
                    return false;
                }
            }
            break;
        }
        case SPELLFAMILY_MAGE:
        {
            // Magic Absorption
            if (dummySpell->SpellIconID == 459)             // only this spell have SpellIconID == 459 and dummy aura
            {
                if (GetPowerType() != POWER_MANA)
                    return false;

                // mana reward
                basepoints0 = (triggeredByAura->GetModifier()->m_amount * GetMaxPower(POWER_MANA) / 100);
                target = this;
                triggered_spell_id = 29442;
                break;
            }
            // Master of Elements
            if (dummySpell->SpellIconID == 1920)
            {
                if(!procSpell)
                    return false;

                // mana cost save
                basepoints0 = procSpell->ManaCost * triggeredByAura->GetModifier()->m_amount/100;
                if( basepoints0 <=0 )
                    return false;

                target = this;
                triggered_spell_id = 29077;
                break;
            }
            // Incanter's Regalia set (add trigger chance to Mana Shield)
            if (dummySpell->SpellFamilyFlags & 0x0000000000008000LL)
            {
                if (GetTypeId() != TYPEID_PLAYER)
                    return false;
  
                if (!HasAuraEffect(37424))
                    return false;

                target = this;
                triggered_spell_id = 37436;
                break;
            }
            switch(dummySpell->Id)
            {
                // Ignite
                case 11119:
                case 11120:
                case 12846:
                case 12847:
                case 12848:
                {
                    if(procSpell && procSpell->Id == 34913) // No Ignite proc's from Molten Armor
                        return false;
                    
                    switch (dummySpell->Id)
                    {
                        case 11119: basepoints0 = int32(0.04f*damage); break;
                        case 11120: basepoints0 = int32(0.08f*damage); break;
                        case 12846: basepoints0 = int32(0.12f*damage); break;
                        case 12847: basepoints0 = int32(0.16f*damage); break;
                        case 12848: basepoints0 = int32(0.20f*damage); break;
                        default:
                            TC_LOG_ERROR("spell","Unit::HandleDummyAuraProc: non handled spell id: %u (IG)",dummySpell->Id);
                            return false;
                    }

                    AuraList const &DoT = pVictim->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for (AuraList::const_iterator itr = DoT.begin(); itr != DoT.end(); ++itr)
                        if ((*itr)->GetId() == 12654 && (*itr)->GetCaster() == this)
                            if ((*itr)->GetBasePoints() > 0)
                                basepoints0 += int((*itr)->GetBasePoints()/((*itr)->GetTickNumber() + 1));

                    triggered_spell_id = 12654;
                    break;
                }
                // Combustion
                case 11129:
                {
                    //last charge and crit
                    if (triggeredByAura->m_procCharges <= 1 && (procEx & PROC_EX_CRITICAL_HIT) )
                    {
                        RemoveAurasDueToSpell(28682);       //-> remove Combustion auras
                        return true;                        // charge counting (will removed)
                    }

                    CastSpell(this, 28682, true, castItem, triggeredByAura);
                    return (procEx & PROC_EX_CRITICAL_HIT);// charge update only at crit hits, no hidden cooldowns
                }
            }
            break;
        }
        case SPELLFAMILY_WARRIOR:
        {
            // Retaliation
            if(dummySpell->SpellFamilyFlags==0x0000000800000000LL)
            {
                // check attack comes not from behind
                if (!HasInArc(M_PI, pVictim) || HasUnitState(UNIT_STATE_STUNNED))
                    return false;

                triggered_spell_id = 22858;
                break;
            }
            else if (dummySpell->SpellIconID == 1697)  // Second Wind
            {
                // only for spells and hit/crit (trigger start always) and not start from self casted spells (5530 Mace Stun Effect for example)
                if (procSpell == 0 || !(procEx & (PROC_EX_NORMAL_HIT|PROC_EX_CRITICAL_HIT)) || this == pVictim)
                    return false;
                // Need stun or root mechanic
                if (procSpell->Mechanic != MECHANIC_ROOT && procSpell->Mechanic != MECHANIC_STUN)
                {
                    int32 i;
                    for (i=0; i<3; i++)
                        if (procSpell->Effects[i].Mechanic == MECHANIC_ROOT || procSpell->Effects[i].Mechanic == MECHANIC_STUN)
                            break;
                    if (i == 3)
                        return false;
                }

                switch (dummySpell->Id)
                {
                    case 29838: triggered_spell_id=29842; break;
                    case 29834: triggered_spell_id=29841; break;
                    default:
                        TC_LOG_ERROR("spell","Unit::HandleDummyAuraProc: non handled spell id: %u (SW)",dummySpell->Id);
                    return false;
                }

                target = this;
                break;
            }
            break;
        }
        case SPELLFAMILY_WARLOCK:
        {
            // Seed of Corruption
            if (dummySpell->SpellFamilyFlags & 0x0000001000000000LL)
            {
                if(procSpell && procSpell->Id == 27285)
                    return false;
                Modifier const* mod = triggeredByAura->GetModifier();
                // if damage is more than need or target die from damage deal finish spell
                if( mod->m_amount <= damage || GetHealth() <= damage )
                {
                    // remember guid before aura delete
                    uint64 casterGuid = triggeredByAura->GetCasterGUID();
                    // Remove our seed aura before casting
                    RemoveAurasByCasterSpell(triggeredByAura->GetId(),casterGuid);
                    // Cast finish spell
                    if(Unit* caster = ObjectAccessor::GetUnit(*this, casterGuid))
                        caster->CastSpell(this, 27285, true, castItem);
                    return true;                            // no hidden cooldown
                }

                // Damage counting
                triggeredByAura->SetModifierValue(triggeredByAura->GetModifierValue() - damage);
                return true;
            }
            // Seed of Corruption (Mobs cast) - no die req
            if (dummySpell->SpellFamilyFlags == 0x00LL && dummySpell->SpellIconID == 1932)
            {
                // No Chain Procs
                if(procSpell && procSpell->Id == 32865 )
                    return false;

                Modifier const* mod = triggeredByAura->GetModifier();
                // if damage is more than need deal finish spell
                if( mod->m_amount <= damage )
                {
                    // remember guid before aura delete
                    uint64 casterGuid = triggeredByAura->GetCasterGUID();

                    // Remove aura (before cast for prevent infinite loop handlers)
                    RemoveAurasDueToSpell(triggeredByAura->GetId());

                    // Cast finish spell (triggeredByAura already not exist!)
                    if(Unit* caster = ObjectAccessor::GetUnit(*this, casterGuid))
                        caster->CastSpell(this, 32865, true, castItem);
                    return true;                            // no hidden cooldown
                }
                // Damage counting
                triggeredByAura->SetModifierValue(triggeredByAura->GetModifierValue() - damage);
                return true;
            }
            switch(dummySpell->Id)
            {
                // Nightfall
                case 18094:
                case 18095:
                {
                    target = this;
                    triggered_spell_id = 17941;
                    break;
                }
                //Soul Leech
                case 30293:
                case 30295:
                case 30296:
                {
                    // health
                    basepoints0 = int32(damage*triggeredByAura->GetModifier()->m_amount/100);
                    target = this;
                    triggered_spell_id = 30294;
                    break;
                }
                // Shadowflame (Voidheart Raiment set bonus)
                case 37377:
                {
                    triggered_spell_id = 37379;
                    break;
                }
                // Pet Healing (Corruptor Raiment or Rift Stalker Armor)
                case 37381:
                {
                    target = GetPet();
                    if(!target)
                        return false;

                    // heal amount
                    basepoints0 = damage * triggeredByAura->GetModifier()->m_amount/100;
                    triggered_spell_id = 37382;
                    break;
                }
                // Shadowflame Hellfire (Voidheart Raiment set bonus)
                case 39437:
                {
                    triggered_spell_id = 37378;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            // Vampiric Touch
            if( dummySpell->SpellFamilyFlags & 0x0000040000000000LL )
            {
                if(!pVictim || !pVictim->IsAlive())
                    return false;

                // pVictim is caster of aura
                if(triggeredByAura->GetCasterGUID() != pVictim->GetGUID())
                    return false;

                // energize amount
                basepoints0 = triggeredByAura->GetModifier()->m_amount*damage/100;
                pVictim->CastCustomSpell(pVictim,34919,&basepoints0,NULL,NULL,true,castItem,triggeredByAura);
                return true;                                // no hidden cooldown
            }
            switch(dummySpell->Id)
            {
                // Vampiric Embrace
                case 15286:
                {
                    if(!pVictim || !pVictim->IsAlive())
                        return false;

                    // pVictim is caster of aura
                    if(triggeredByAura->GetCasterGUID() != pVictim->GetGUID())
                        return false;

                    // heal amount
                    basepoints0 = triggeredByAura->GetModifier()->m_amount*damage/100;
                    pVictim->CastCustomSpell(pVictim,15290,&basepoints0,NULL,NULL,true,castItem,triggeredByAura);
                    return true;                                // no hidden cooldown
                }
                // Priest Tier 6 Trinket (Ashtongue Talisman of Acumen)
                case 40438:
                {
                    // Shadow Word: Pain
                    if( procSpell->SpellFamilyFlags & 0x0000000000008000LL )
                        triggered_spell_id = 40441;
                    // Renew
                    else if( procSpell->SpellFamilyFlags & 0x0000000000000040LL )
                        triggered_spell_id = 40440;
                    else
                        return false;

                    target = this;
                    break;
                }
                // Oracle Healing Bonus ("Garments of the Oracle" set)
                case 26169:
                {
                    // heal amount
                    basepoints0 = int32(damage * 10/100);
                    target = this;
                    triggered_spell_id = 26170;
                    break;
                }
                // Frozen Shadoweave (Shadow's Embrace set) warning! its not only priest set
                case 39372:
                {
                    if(!procSpell || (procSpell->GetSchoolMask() & (SPELL_SCHOOL_MASK_FROST | SPELL_SCHOOL_MASK_SHADOW))==0 )
                        return false;

                    // heal amount
                    basepoints0 = int32(damage * 2 / 100);
                    target = this;
                    triggered_spell_id = 39373;
                    break;
                }
                // Vestments of Faith (Priest Tier 3) - 4 pieces bonus
                case 28809:
                {
                    triggered_spell_id = 28810;
                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_DRUID:
        {
            switch(dummySpell->Id)
            {
                // Healing Touch (Dreamwalker Raiment set)
                case 28719:
                {
                    // mana back
                    basepoints0 = int32(procSpell->ManaCost * 30 / 100);
                    target = this;
                    triggered_spell_id = 28742;
                    break;
                }
                // Healing Touch Refund (Idol of Longevity trinket)
                case 28847:
                {
                    target = this;
                    triggered_spell_id = 28848;
                    break;
                }
                // Mana Restore (Malorne Raiment set / Malorne Regalia set)
                case 37288:
                case 37295:
                {
                    target = this;
                    triggered_spell_id = 37238;
                    break;
                }
                // Druid Tier 6 Trinket
                case 40442:
                {
                    float  chance;

                    // Starfire
                    if( procSpell->SpellFamilyFlags & 0x0000000000000004LL )
                    {
                        triggered_spell_id = 40445;
                        chance = 25.f;
                    }
                    // Rejuvenation
                    else if( procSpell->SpellFamilyFlags & 0x0000000000000010LL )
                    {
                        triggered_spell_id = 40446;
                        chance = 25.f;
                    }
                    // Mangle (cat/bear)
                    else if( procSpell->SpellFamilyFlags & 0x0000044000000000LL )
                    {
                        triggered_spell_id = 40452;
                        chance = 40.f;
                    }
                    else
                        return false;

                    if (!roll_chance_f(chance))
                        return false;

                    target = this;
                    break;
                }
                // Maim Interrupt
                /*case 44835:
                {
                    // Deadly Interrupt Effect
                    //triggered_spell_id = 32747;
                    //break;
                }*/
            }
            break;
        }
        case SPELLFAMILY_ROGUE:
        {
            switch(dummySpell->Id)
            {
                // Deadly Throw Interrupt
                case 32748:
                {
                    // Prevent cast Deadly Throw Interrupt on self from last effect (apply dummy) of Deadly Throw
                    if(this == pVictim)
                        return false;

                    triggered_spell_id = 32747;
                    break;
                }
            }
            // Quick Recovery
            if( dummySpell->SpellIconID == 2116 )
            {
                if(!procSpell)
                    return false;

                // only rogue's finishing moves (maybe need additional checks)
                if( procSpell->SpellFamilyName!=SPELLFAMILY_ROGUE ||
                    (procSpell->SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE_FINISHING_MOVE) == 0)
                    return false;

                // energy cost save
                basepoints0 = procSpell->ManaCost * triggeredByAura->GetModifier()->m_amount/100;
                if(basepoints0 <= 0)
                    return false;

                target = this;
                triggered_spell_id = 31663;
                break;
            }
            break;
        }
        case SPELLFAMILY_HUNTER:
        {
            // Thrill of the Hunt
            if ( dummySpell->SpellIconID == 2236 )
            {
                if(!procSpell)
                    return false;

                // mana cost save
                basepoints0 = procSpell->ManaCost * 40/100;
                if(basepoints0 <= 0)
                    return false;

                target = this;
                triggered_spell_id = 34720;
                break;
            }
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            // Seal of Righteousness - melee proc dummy
            if (dummySpell->SpellFamilyFlags&0x000000008000000LL && triggeredByAura->GetEffIndex()==0)
            {
                if(GetTypeId() != TYPEID_PLAYER)
                    return false;

                uint32 spellId;
                switch (triggeredByAura->GetId())
                {
                    case 21084: spellId = 25742; break;     // Rank 1
                    case 20287: spellId = 25740; break;     // Rank 2
                    case 20288: spellId = 25739; break;     // Rank 3
                    case 20289: spellId = 25738; break;     // Rank 4
                    case 20290: spellId = 25737; break;     // Rank 5
                    case 20291: spellId = 25736; break;     // Rank 6
                    case 20292: spellId = 25735; break;     // Rank 7
                    case 20293: spellId = 25713; break;     // Rank 8
                    case 27155: spellId = 27156; break;     // Rank 9
                    default:
                        TC_LOG_ERROR("spell","Unit::HandleDummyAuraProc: non handled possibly SoR (Id = %u)", triggeredByAura->GetId());
                        return false;
                }
                Item *item = (this->ToPlayer())->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                float speed = (item ? item->GetTemplate()->Delay : BASE_ATTACK_TIME)/1000.0f;

                float damageBasePoints;
                if(item && item->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                    // two hand weapon
                    damageBasePoints=1.20f*triggeredByAura->GetModifier()->m_amount * 1.2f * 1.03f * speed/100.0f + 1;
                else
                    // one hand weapon/no weapon
                    damageBasePoints=0.85f*ceil(triggeredByAura->GetModifier()->m_amount * 1.2f * 1.03f * speed/100.0f) - 1;

                int32 damagePoint = int32(damageBasePoints + 0.03f * (GetWeaponDamageRange(BASE_ATTACK,MINDAMAGE)+GetWeaponDamageRange(BASE_ATTACK,MAXDAMAGE))/2.0f) + 1;

                // apply damage bonuses manually
                if (damagePoint >= 0)
                {
                    damagePoint = SpellDamageBonusDone(pVictim, dummySpell, damagePoint, SPELL_DIRECT_DAMAGE);
                    damagePoint = pVictim->SpellDamageBonusTaken(this, dummySpell, damagePoint, SPELL_DIRECT_DAMAGE);
                }

                CastCustomSpell(pVictim,spellId,&damagePoint,NULL,NULL,true,NULL, triggeredByAura);
                return true;                                // no hidden cooldown
            }
            // Seal of Blood do damage trigger
            if(dummySpell->SpellFamilyFlags & 0x0000040000000000LL)
            {
                switch(triggeredByAura->GetEffIndex())
                {
                    case 0:
                        triggered_spell_id = 31893;
                        break;
                    case 1:
                    {
                        // damage
                        damage += CalculateDamage(BASE_ATTACK, false) * 35 / 100; // add spell damage from prev effect (35%)
                        basepoints0 =  triggeredByAura->GetModifier()->m_amount * damage / 100;

                        target = this;

                        triggered_spell_id = 32221;
                        break;
                    }
                }
            }

            switch(dummySpell->Id)
            {
                // Holy Power (Redemption Armor set)
                case 28789:
                {
                    if(!pVictim)
                        return false;

                    // Set class defined buff
                    switch (pVictim->GetClass())
                    {
                        case CLASS_PALADIN:
                        case CLASS_PRIEST:
                        case CLASS_SHAMAN:
                        case CLASS_DRUID:
                            triggered_spell_id = 28795;     // Increases the friendly target's mana regeneration by $s1 per 5 sec. for $d.
                            break;
                        case CLASS_MAGE:
                        case CLASS_WARLOCK:
                            triggered_spell_id = 28793;     // Increases the friendly target's spell damage and healing by up to $s1 for $d.
                            break;
                        case CLASS_HUNTER:
                        case CLASS_ROGUE:
                            triggered_spell_id = 28791;     // Increases the friendly target's attack power by $s1 for $d.
                            break;
                        case CLASS_WARRIOR:
                            triggered_spell_id = 28790;     // Increases the friendly target's armor
                            break;
                        default:
                            return false;
                    }
                    break;
                }
                //Seal of Vengeance
                case 31801:
                {
                    if(effIndex != 0)                       // effect 1,2 used by seal unleashing code
                        return false;

                    triggered_spell_id = 31803;
                    // On target with 5 stacks of Holy Vengeance direct damage is done
                    AuraList const& auras = pVictim->GetAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
                    for(AuraList::const_iterator itr = auras.begin(); itr!=auras.end(); ++itr)
                    {
                        if((*itr)->GetId() == 31803 && (*itr)->GetCasterGUID() == this->GetGUID())
                        {
                            // 10% of tick done as direct damage
                            if ((*itr)->GetStackAmount() == 5)
                            {
                                int32 directDamage = SpellDamageBonusDone(pVictim,(*itr)->GetSpellInfo(),(*itr)->GetModifierValuePerStack(),DOT)/2;
                                damage = pVictim->SpellDamageBonusTaken(this, (*itr)->GetSpellInfo(), directDamage, DOT)/2;
                                CastCustomSpell(pVictim, 42463, &directDamage,NULL,NULL,true,0,triggeredByAura);
                            }
                            break;
                        }
                    }
                    break;
                }
                // Spiritual Att.
                case 31785:
                case 33776:
                {
                    // if healed by another unit (pVictim)
                    if(this == pVictim)
                        return false;

                    // heal amount
                    basepoints0 = triggeredByAura->GetModifier()->m_amount*std::min(damage,GetMaxHealth() - GetHealth())/100;
                    target = this;

                    if(basepoints0)
                        triggered_spell_id = 31786;
                    break;
                }
                // Paladin Tier 6 Trinket (Ashtongue Talisman of Zeal)
                case 40470:
                {
                    if( !procSpell )
                        return false;

                    float  chance;

                    // Flash of light/Holy light
                    if( procSpell->SpellFamilyFlags & 0x00000000C0000000LL)
                    {
                        triggered_spell_id = 40471;
                        chance = 15.f;
                    }
                    // Judgement
                    else if( procSpell->SpellFamilyFlags & 0x0000000000800000LL )
                    {
                        triggered_spell_id = 40472;
                        chance = 50.f;
                    }
                    else
                        return false;

                    if (!roll_chance_f(chance))
                        return false;

                    break;
                }
            }
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            switch(dummySpell->Id)
            {
                // Totemic Power (The Earthshatterer set)
                case 28823:
                {
                    if( !pVictim )
                        return false;

                    // Set class defined buff
                    switch (pVictim->GetClass())
                    {
                        case CLASS_PALADIN:
                        case CLASS_PRIEST:
                        case CLASS_SHAMAN:
                        case CLASS_DRUID:
                            triggered_spell_id = 28824;     // Increases the friendly target's mana regeneration by $s1 per 5 sec. for $d.
                            break;
                        case CLASS_MAGE:
                        case CLASS_WARLOCK:
                            triggered_spell_id = 28825;     // Increases the friendly target's spell damage and healing by up to $s1 for $d.
                            break;
                        case CLASS_HUNTER:
                        case CLASS_ROGUE:
                            triggered_spell_id = 28826;     // Increases the friendly target's attack power by $s1 for $d.
                            break;
                        case CLASS_WARRIOR:
                            triggered_spell_id = 28827;     // Increases the friendly target's armor
                            break;
                        default:
                            return false;
                    }
                    break;
                }
                // Lesser Healing Wave (Totem of Flowing Water Relic)
                case 28849:
                {
                    target = this;
                    triggered_spell_id = 28850;
                    break;
                }
                // Windfury Weapon (Passive) 1-5 Ranks
                case 33757:
                {
                    if(GetTypeId()!=TYPEID_PLAYER)
                        return false;

                    if(!castItem || !castItem->IsEquipped())
                        return false;

                    if(triggeredByAura && castItem->GetGUID() != triggeredByAura->GetCastItemGUID())
                        return false;

                    // custom cooldown processing case
                    if( cooldown && (this->ToPlayer())->HasSpellCooldown(dummySpell->Id))
                        return false;

                    uint32 spellId;
                    switch (castItem->GetEnchantmentId(EnchantmentSlot(TEMP_ENCHANTMENT_SLOT)))
                    {
                        case 283: spellId = 33757; break;   //1 Rank
                        case 284: spellId = 33756; break;   //2 Rank
                        case 525: spellId = 33755; break;   //3 Rank
                        case 1669:spellId = 33754; break;   //4 Rank
                        case 2636:spellId = 33727; break;   //5 Rank
                        default:
                        {
                            TC_LOG_ERROR("spell","Unit::HandleDummyAuraProc: non handled item enchantment (rank?) %u for spell id: %u (Windfury)",
                                castItem->GetEnchantmentId(EnchantmentSlot(TEMP_ENCHANTMENT_SLOT)),dummySpell->Id);
                            return false;
                        }
                    }

                    SpellInfo const* windfurySpellEntry = sSpellMgr->GetSpellInfo(spellId);
                    if(!windfurySpellEntry)
                    {
                        TC_LOG_ERROR("spell","Unit::HandleDummyAuraProc: non existed spell id: %u (Windfury)",spellId);
                        return false;
                    }

                    int32 extra_attack_power = CalculateSpellDamage(windfurySpellEntry,0,windfurySpellEntry->Effects[0].BasePoints,pVictim);

                    // Off-Hand case
                    if ( castItem->GetSlot() == EQUIPMENT_SLOT_OFFHAND )
                    {
                        // Value gained from additional AP
                        basepoints0 = int32(extra_attack_power/14.0f * GetAttackTime(OFF_ATTACK)/1000/2);
                        triggered_spell_id = 33750;
                    }
                    // Main-Hand case
                    else
                    {
                        // Value gained from additional AP
                        basepoints0 = int32(extra_attack_power/14.0f * GetAttackTime(BASE_ATTACK)/1000);
                        triggered_spell_id = 25504;
                    }

                    // apply cooldown before cast to prevent processing itself
                    if( cooldown )
                        (this->ToPlayer())->AddSpellCooldown(dummySpell->Id,0,time(NULL) + cooldown);

                    // Attack Twice
                    for ( uint32 i = 0; i<2; ++i )
                        CastCustomSpell(pVictim,triggered_spell_id,&basepoints0,NULL,NULL,true,castItem,triggeredByAura);

                    return true;
                }
                // Shaman Tier 6 Trinket
                case 40463:
                {
                    if( !procSpell )
                        return false;

                    float  chance;
                    if (procSpell->SpellFamilyFlags & 0x0000000000000001LL)
                    {
                        triggered_spell_id = 40465;         // Lightning Bolt
                        chance = 15.f;
                    }
                    else if (procSpell->SpellFamilyFlags & 0x0000000000000080LL)
                    {
                        triggered_spell_id = 40465;         // Lesser Healing Wave
                        chance = 10.f;
                    }
                    else if (procSpell->SpellFamilyFlags & 0x0000001000000000LL)
                    {
                        triggered_spell_id = 40466;         // Stormstrike
                        chance = 50.f;
                    }
                    else
                        return false;

                    if (!roll_chance_f(chance))
                        return false;

                    target = this;
                    break;
                }
            }

            // Earth Shield
            if(dummySpell->SpellFamilyFlags==0x40000000000LL)
            {
                // heal
                target = this;
                basepoints0 = triggeredByAura->GetModifier()->m_amount;
                if(Unit* caster = triggeredByAura->GetCaster())
                    basepoints0 = caster->SpellHealingBonusDone(caster, triggeredByAura->GetSpellInfo(), basepoints0, SPELL_DIRECT_DAMAGE);
                triggered_spell_id = 379;
                break;
            }
            // Lightning Overload
            if (dummySpell->SpellIconID == 2018)            // only this spell have SpellFamily Shaman SpellIconID == 2018 and dummy aura
            {
                if(!procSpell || GetTypeId() != TYPEID_PLAYER || !pVictim )
                    return false;

                // custom cooldown processing case
                if( cooldown && GetTypeId()==TYPEID_PLAYER && (this->ToPlayer())->HasSpellCooldown(dummySpell->Id))
                    return false;

                uint32 spellId = 0;
                // Every Lightning Bolt and Chain Lightning spell have duplicate vs half damage and zero cost
                switch (procSpell->Id)
                {
                    // Lightning Bolt
                    case   403: spellId = 45284; break;     // Rank  1
                    case   529: spellId = 45286; break;     // Rank  2
                    case   548: spellId = 45287; break;     // Rank  3
                    case   915: spellId = 45288; break;     // Rank  4
                    case   943: spellId = 45289; break;     // Rank  5
                    case  6041: spellId = 45290; break;     // Rank  6
                    case 10391: spellId = 45291; break;     // Rank  7
                    case 10392: spellId = 45292; break;     // Rank  8
                    case 15207: spellId = 45293; break;     // Rank  9
                    case 15208: spellId = 45294; break;     // Rank 10
                    case 25448: spellId = 45295; break;     // Rank 11
                    case 25449: spellId = 45296; break;     // Rank 12
                    // Chain Lightning
                    case   421: spellId = 45297; break;     // Rank  1
                    case   930: spellId = 45298; break;     // Rank  2
                    case  2860: spellId = 45299; break;     // Rank  3
                    case 10605: spellId = 45300; break;     // Rank  4
                    case 25439: spellId = 45301; break;     // Rank  5
                    case 25442: spellId = 45302; break;     // Rank  6
                    default:
                        TC_LOG_ERROR("spell","Unit::HandleDummyAuraProc: non handled spell id: %u (LO)", procSpell->Id);
                        return false;
                }
                // No thread generated mod
                SpellModifier *mod = new SpellModifier;
                mod->op = SPELLMOD_THREAT;
                mod->value = -100;
                mod->type = SPELLMOD_PCT;
                mod->spellId = dummySpell->Id;
                mod->effectId = 0;
                mod->lastAffected = NULL;
                mod->mask = 0x0000000000000003LL;
                mod->charges = 0;
                (this->ToPlayer())->AddSpellMod(mod, true);

                // Remove cooldown (Chain Lightning - have Category Recovery time)
                if (procSpell->SpellFamilyFlags & 0x0000000000000002LL)
                    (this->ToPlayer())->RemoveSpellCooldown(spellId);

                // Hmmm.. in most case spells already set half basepoints but...
                // Lightning Bolt (2-10 rank) have full basepoint and half bonus from level
                // As on wiki:
                // BUG: Rank 2 to 10 (and maybe 11) of Lightning Bolt will proc another Bolt with FULL damage (not halved). This bug is known and will probably be fixed soon.
                // So - no add changes :)
                CastSpell(pVictim, spellId, true, castItem, triggeredByAura);

                (this->ToPlayer())->AddSpellMod(mod, false);

                if( cooldown && GetTypeId()==TYPEID_PLAYER )
                    (this->ToPlayer())->AddSpellCooldown(dummySpell->Id,0,time(NULL) + cooldown);

                return true;
            }
            break;
        }
        case SPELLFAMILY_POTION:
        {
            if (dummySpell->Id == 17619)
            {
                if (procSpell->SpellFamilyName == SPELLFAMILY_POTION)
                {
                    for (uint8 i=0;i<3;i++)
                    {
                        if (procSpell->Effects[i].Effect==SPELL_EFFECT_HEAL)
                        {
                            triggered_spell_id = 21399;
                        }
                        else if (procSpell->Effects[i].Effect==SPELL_EFFECT_ENERGIZE)
                        {
                            triggered_spell_id = 21400;
                        }
                        else continue;
                        basepoints0 = CalculateSpellDamage(procSpell,i,procSpell->Effects[i].BasePoints,this) * 0.4f;
                        CastCustomSpell(this,triggered_spell_id,&basepoints0,NULL,NULL,true,castItem,triggeredByAura);
                    }
                    return true;
                }
            }
        }
        default:
            break;
    }

    // processed charge only counting case
    if(!triggered_spell_id)
        return true;

    SpellInfo const* triggerEntry = sSpellMgr->GetSpellInfo(triggered_spell_id);

    if(!triggerEntry)
    {
        TC_LOG_ERROR("spell.aura","Unit::HandleDummyAuraProc: Spell %u have not existed triggered spell %u",dummySpell->Id,triggered_spell_id);
        return false;
    }

    // default case
    if(!target || (target != this && !target->IsAlive()))
        return false;

    if( cooldown && GetTypeId()==TYPEID_PLAYER && (this->ToPlayer())->HasSpellCooldown(triggered_spell_id))
        return false;

    if(basepoints0)
        CastCustomSpell(target,triggered_spell_id,&basepoints0,NULL,NULL,true,castItem,triggeredByAura);
    else
        CastSpell(target,triggered_spell_id,true,castItem,triggeredByAura);

    if( cooldown && GetTypeId()==TYPEID_PLAYER )
        (this->ToPlayer())->AddSpellCooldown(triggered_spell_id,0,time(NULL) + cooldown);

    return true;
}

bool Unit::HandleProcTriggerSpell(Unit *pVictim, uint32 damage, Aura* triggeredByAura, SpellInfo const *procSpell, uint32 ProcFlags, uint32 procEx, uint32 cooldown)
{
    // Get triggered aura spell info
    SpellInfo const* auraSpellInfo = triggeredByAura->GetSpellInfo();
    
    //TC_LOG_INFO("ProcSpell %u (%s) triggered spell %u (%s)", procSpell->Id, procSpell->SpellName[sWorld->GetDefaultDbcLocale()], auraSpellInfo->Id, auraSpellInfo->SpellName[sWorld->GetDefaultDbcLocale()]);

    // Basepoints of trigger aura
    int32 triggerAmount = triggeredByAura->GetModifier()->m_amount;

    // Set trigger spell id, target, custom basepoints
    uint32 trigger_spell_id = auraSpellInfo->Effects[triggeredByAura->GetEffIndex()].TriggerSpell;
    Unit*  target = NULL;
    int32  basepoints0 = 0;

    Item* castItem = triggeredByAura->GetCastItemGUID() && GetTypeId()==TYPEID_PLAYER
        ? (this->ToPlayer())->GetItemByGuid(triggeredByAura->GetCastItemGUID()) : NULL;

    // Try handle unknown trigger spells
    if (sSpellMgr->GetSpellInfo(trigger_spell_id)==NULL)
    switch (auraSpellInfo->SpellFamilyName)
    {
     //=====================================================================
     // Generic class
     // ====================================================================
     // .....
     //=====================================================================
     case SPELLFAMILY_GENERIC:
     if (auraSpellInfo->Id==43820)   // Charm of the Witch Doctor (Amani Charm of the Witch Doctor trinket)
     {
          // Pct value stored in dummy
          basepoints0 = pVictim->GetCreateHealth() * auraSpellInfo->Effects[1].BasePoints / 100;
          target = pVictim;
          break;
     }
     else if (auraSpellInfo->Id == 45054) {     // Item 34470: don't proc on positive spells like Health Funnel
        if (procSpell && procSpell->Id == 755)
            return true;
        // Unsure
        if (procSpell && procSpell->IsPositive())
            return true;
        break;
     }
     else if (auraSpellInfo->Id == 27522 || auraSpellInfo->Id == 46939)   // Black bow of the Betrayer
     {
         // On successful melee or ranged attack gain $29471s1 mana and if possible drain $27526s1 mana from the target.
         if (this->IsAlive())
             CastSpell(this, 29471, true, castItem, triggeredByAura);
         if (pVictim && pVictim->IsAlive()) {
             //CastSpell(pVictim, 27526, true, castItem, triggeredByAura);
             if (pVictim->GetPowerType() == POWER_MANA && pVictim->GetPower(POWER_MANA) > 8)
                CastSpell(this, 27526, true, castItem, triggeredByAura);
        }
        //RemoveAurasDueToSpell(46939);
         return true;
     }
     break;
     //=====================================================================
     // Mage
     //=====================================================================
     // Blazing Speed (Rank 1,2) trigger = 18350
     //=====================================================================
     case SPELLFAMILY_MAGE:
         //nothing
     break;
     //=====================================================================
     // Warrior
     //=====================================================================
     // Rampage (Rank 1-3) trigger = 18350
     //=====================================================================
     case SPELLFAMILY_WARRIOR:
         //nothing
     break;
     //=====================================================================
     // Warlock
     //=====================================================================
     // Pyroclasm             trigger = 18350
     // Drain Soul (Rank 1-5) trigger = 0
     //=====================================================================
     case SPELLFAMILY_WARLOCK:
     {
         // Pyroclasm
         if (auraSpellInfo->SpellIconID == 1137)
         {
             if(!pVictim || !pVictim->IsAlive() || pVictim == this || procSpell == NULL)
                 return false;
             // Calculate spell tick count for spells
             uint32 tick = 1; // Default tick = 1

             // Hellfire have 15 tick
             if (procSpell->SpellFamilyFlags&0x0000000000000040LL)
                 tick = 1;  // was 15
             // Rain of Fire have 4 tick
             else if (procSpell->SpellFamilyFlags&0x0000000000000020LL)
                 tick = 4;  // was 4
             else
                 return false;

             // Calculate chance = baseChance / tick
             float chance = 0;
             switch (auraSpellInfo->Id)
             {
                 case 18096: chance = 13.0f / tick; break;
                 case 18073: chance = 26.0f / tick; break;
             }
             // Roll chance
             if (!roll_chance_f(chance))
                 return false;

             //triggered_spell_id = 18093;
            CastSpell(pVictim, 18093, true);
         }
         // Drain Soul
         else if (auraSpellInfo->SpellFamilyFlags & 0x0000000000004000LL)
         {
             Unit::AuraList const& mAddFlatModifier = GetAurasByType(SPELL_AURA_ADD_FLAT_MODIFIER);
             for(Unit::AuraList::const_iterator i = mAddFlatModifier.begin(); i != mAddFlatModifier.end(); ++i)
             {
                 if ((*i)->GetModifier()->m_miscvalue == SPELLMOD_CHANCE_OF_SUCCESS && (*i)->GetSpellInfo()->SpellIconID == 113)
                 {
                     int32 value2 = CalculateSpellDamage((*i)->GetSpellInfo(),2,(*i)->GetSpellInfo()->Effects[2].BasePoints,this);
                     basepoints0 = value2 * GetMaxPower(POWER_MANA) / 100;
                 }
             }
             if ( basepoints0 == 0 )
                 return false;
             trigger_spell_id = 18371;
         }
         break;
     }
     //=====================================================================
     // Priest
     //=====================================================================
     // Greater Heal Refund         trigger = 18350
     // Blessed Recovery (Rank 1-3) trigger = 18350
     // Shadowguard (1-7)           trigger = 28376
     //=====================================================================
     case SPELLFAMILY_PRIEST:
     {
         // Blessed Recovery
         if (auraSpellInfo->SpellIconID == 1875)
         {
             basepoints0 = damage * triggerAmount / 100 / 3;
             target = this;
             if(basepoints0 == 0)
                 return false;
         }
         break;
     }
     //=====================================================================
     // Druid
     // ====================================================================
     // Druid Forms Trinket  trigger = 18350
     //=====================================================================
     case SPELLFAMILY_DRUID:
     {
         // Druid Forms Trinket
         if (auraSpellInfo->Id==37336)
         {
             switch(m_form)
             {
                 case 0:              trigger_spell_id = 37344;break;
                 case FORM_CAT:       trigger_spell_id = 37341;break;
                 case FORM_BEAR:
                 case FORM_DIREBEAR:  trigger_spell_id = 37340;break;
                 case FORM_TREE:      trigger_spell_id = 37342;break;
                 case FORM_MOONKIN:   trigger_spell_id = 37343;break;
                 default:
                     return false;
             }
         }
         break;
     }
     //=====================================================================
     // Hunter
     // ====================================================================
     // ......
     //=====================================================================
     case SPELLFAMILY_HUNTER:
     break;
     //=====================================================================
     // Paladin
     // ====================================================================
     // Blessed Life                   trigger = 31934
     // Healing Discount               trigger = 18350
     // Illumination (Rank 1-5)        trigger = 18350
     // Lightning Capacitor            trigger = 18350
     //=====================================================================
     case SPELLFAMILY_PALADIN:
     {
         // Blessed Life
         if (auraSpellInfo->SpellIconID == 2137)
         {
             switch (auraSpellInfo->Id)
             {
                 case 31828: // Rank 1
                 case 31829: // Rank 2
                 case 31830: // Rank 3
                    //TC_LOG_DEBUG("spell","Blessed life trigger!");
                 break;
                 default:
                     TC_LOG_ERROR("spell","Unit::HandleProcTriggerSpell: Spell %u miss possibly Blessed Life", auraSpellInfo->Id);
                 return false;
             }
         }
         // Healing Discount
         if (auraSpellInfo->Id==37705)
         {
             // triggers Healing Trance
             switch (GetClass())
             {
                 case CLASS_PALADIN: trigger_spell_id = 37723; break;
                 case CLASS_DRUID: trigger_spell_id = 37721; break;
                 case CLASS_PRIEST: trigger_spell_id = 37706; break;
                 case CLASS_SHAMAN: trigger_spell_id= 37722; break;
                 default: return false;
             }
             target = this;
         }
         // Illumination
         else if (auraSpellInfo->SpellIconID==241)
         {
             if(!procSpell)
                 return false;
             // procspell is triggered spell but we need mana cost of original casted spell
             uint32 originalSpellId = procSpell->Id;
             // Holy Shock
             if(procSpell->SpellFamilyFlags & 0x1000000000000LL) // Holy Shock heal
             {
                 switch(procSpell->Id)
                 {
                     case 25914: originalSpellId = 20473; break;
                     case 25913: originalSpellId = 20929; break;
                     case 25903: originalSpellId = 20930; break;
                     case 27175: originalSpellId = 27174; break;
                     case 33074: originalSpellId = 33072; break;
                     default:
                         TC_LOG_ERROR("spell","Unit::HandleProcTriggerSpell: Spell %u not handled in HShock",procSpell->Id);
                     return false;
                 }
             }
             SpellInfo const *originalSpell = sSpellMgr->GetSpellInfo(originalSpellId);
             if(!originalSpell)
             {
                 TC_LOG_ERROR("spell","Unit::HandleProcTriggerSpell: Spell %u unknown but selected as original in Illu",originalSpellId);
                 return false;
             }
             // percent stored in effect 1 (class scripts) base points
             basepoints0 = originalSpell->ManaCost*(auraSpellInfo->Effects[1].BasePoints+1)/100;
             trigger_spell_id = 20272;
             target = this;
         }
     }
     //=====================================================================
     // Shaman
     //====================================================================
     // Nature's Guardian (Rank 1-5) trigger = 18350
     //=====================================================================
     case SPELLFAMILY_SHAMAN:
     {
         if (auraSpellInfo->SpellIconID == 2013) //Nature's Guardian
         {
             // Check health condition - should drop to less 30% (damage deal after this!)
             if (!(10*(int32(GetHealth() - damage)) < 3 * GetMaxHealth()))
                 return false;

             if(pVictim && pVictim->IsAlive())
                 pVictim->getThreatManager().modifyThreatPercent(this,-10);

             basepoints0 = triggerAmount * GetMaxHealth() / 100;
             trigger_spell_id = 31616;
             target = this;
         }
         break;
     }
     // default
     default:
         break;
    }

    // All ok. Check current trigger spell
    SpellInfo const* triggerEntry = sSpellMgr->GetSpellInfo(trigger_spell_id);
    if ( triggerEntry == NULL )
    {
        // Not cast unknown spell
        // TC_LOG_ERROR("FIXME","Unit::HandleProcTriggerSpell: Spell %u have 0 in EffectTriggered[%d], not handled custom case?",auraSpellInfo->Id,triggeredByAura->GetEffIndex());
        return false;
    }

    // check if triggering spell can stack with current target's auras (if not - don't proc)
    // don't check if 
    // aura is passive (talent's aura)
    // trigger_spell_id's aura is already active (allow to refresh triggered auras)
    // trigger_spell_id's triggeredByAura is already active (for example shaman's shields)
    AuraMap::iterator i,next;
    uint32 aura_id = 0;
    for (i = m_Auras.begin(); i != m_Auras.end(); i = next)
    {
        next = i;
        ++next;
        if (!(*i).second) continue;
            aura_id = (*i).second->GetSpellInfo()->Id;
            if ( (*i).second->GetSpellInfo()->IsPassive() || aura_id == trigger_spell_id || aura_id == triggeredByAura->GetSpellInfo()->Id ) continue;
        if (sSpellMgr->IsNoStackSpellDueToSpell(trigger_spell_id, (*i).second->GetSpellInfo()->Id, ((*i).second->GetCasterGUID() == GetGUID())))
            return false;
    }

    // Costum requirements (not listed in procEx) Warning! damage dealing after this
    // Custom triggered spells
    switch (auraSpellInfo->Id)
    {
        // Lightning Capacitor
        case 37657:
        {
            if (!pVictim || !pVictim->IsAlive())
                return false;
            // stacking
            CastSpell(this, 37658, true, NULL, triggeredByAura);
            // counting
            Aura * dummy = GetDummyAura(37658);
            if (!dummy)
                return false;
            // release at 3 aura in stack (cont contain in basepoint of trigger aura)
            if (dummy->GetStackAmount() <= 2)
                return false;

            RemoveAurasDueToSpell(37658);
            target = pVictim;
            break;
        }

        case 24905:   // Moonkin Form (Passive)
        {
            basepoints0 = GetTotalAttackPowerValue(BASE_ATTACK, pVictim) * 30 / 100;
            target = this;
            break;
        }
        // Lightning Shield (The Ten Storms set)
        case 23551:
        {
            target = pVictim;
            break;
        }
        // Mana Surge (The Earthfury set)
        case 23572:
        {
            if(!procSpell)
                return false;
            basepoints0 = procSpell->ManaCost * 35 / 100;
            target = this;
            break;
        }
        //Leader of the pack
        case 24932:
        {
            if (triggerAmount == 0)
                return false;
            basepoints0 = triggerAmount * GetMaxHealth() / 100;
            break;
        }
        // Blackout
        case 15326:
        {
            if (procSpell->Id == 2096 || procSpell->Id == 10909)
                return false;
            if (procSpell->IsPositive())
                return false;
            break;
        }
        // Persistent Shield (Scarab Brooch trinket)
        // This spell originally trigger 13567 - Dummy Trigger (vs dummy efect)
        case 26467:
        {
            basepoints0 = damage * 15 / 100;
            target = pVictim;
            trigger_spell_id = 26470;
            break;
        }
        // Cheat Death
        case 28845:
        {
            // When your health drops below 20% ....
            if (GetHealth() - damage > GetMaxHealth() / 5 || GetHealth() < GetMaxHealth() / 5)
                return false;
            break;
        }
        // Deadly Swiftness (Rank 1)
        case 31255:
        {
            // whenever you deal damage to a target who is below 20% health.
            if (pVictim->GetHealth() > pVictim->GetMaxHealth() / 5)
                return false;

            target = this;
            trigger_spell_id = 22588;
        }
        // Greater Heal Refund (Avatar Raiment set)
        case 37594:
        {
            // Not give if target alredy have full health
            if (pVictim->GetHealth() == pVictim->GetMaxHealth())
                return false;
            // If your Greater Heal brings the target to full health, you gain $37595s1 mana.
            if (pVictim->GetHealth() + damage < pVictim->GetMaxHealth())
                return false;
            break;
        }
        // Unyielding Knights
        case 38164:
        {
            if (pVictim->GetEntry() != 19457)
                return false;
            break;
        }
        // Bonus Healing (Crystal Spire of Karabor mace)
        case 40971:
        {
            // If your target is below $s1% health
            if (pVictim->GetHealth() > pVictim->GetMaxHealth() * triggerAmount / 100)
                return false;
            break;
        }
        // Evasive Maneuvers (Commendation of Kael`thas trinket)
        case 45057:
        {
            // reduce you below $s1% health
            if (GetHealth() - damage > GetMaxHealth() * triggerAmount / 100)
                return false;
            break;
        }
        // Warriors Sword spec
        /*case 12281:
        case 12812:
        case 12813:
        case 12814:
        case 12815:*/
        case 16459:
            return false;
    }

    // Costum basepoints/target for exist spell
    // dummy basepoints or other customs
    switch(trigger_spell_id)
    {
        // Cast positive spell on enemy target
        case 7099:  // Curse of Mending
        case 39647: // Curse of Mending
        case 29494: // Temptation
        case 20233: // Improved Lay on Hands (cast on target)
        {
            target = pVictim;
            break;
        }
        // Combo points add triggers (need add combopoint only for main tatget, and after possible combopoints reset)
        case 15250: // Rogue Setup
        {
            /*if(!pVictim || pVictim != GetVictim())   // applied only for main target
                return false;*/
            break;                                   // continue normal case
        }
        // Finish movies that add combo
        case 14189: // Seal Fate (Netherblade set)
        {
            // Need add combopoint AFTER finish movie (or they dropped in finish phase)
            break;
        }
        case 14157: // Ruthlessness
        {
            return false; //prevent adding the combo point BEFORE finish movie. Ruthlessness is handled in Player::ClearComboPoints()  
            // Need add combopoint AFTER finish movie (or they dropped in finish phase)
            break;
        }
        // Shamanistic Rage triggered spell
        case 30824:
        {
            basepoints0 = int32(GetTotalAttackPowerValue(BASE_ATTACK, pVictim) * triggerAmount / 100);
            trigger_spell_id = 30824;
            break;
        }
        // Enlightenment (trigger only from mana cost spells)
        case 35095:
        {
            if(!procSpell || (procSpell->PowerType != POWER_MANA) || (procSpell->ManaCost==0 && procSpell->ManaCostPercentage==0 && procSpell->ManaCostPerlevel==0 ))
                return false;
            break;
        }
    }

    switch(auraSpellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_PALADIN:
            // Judgement of Light and Judgement of Wisdom
            if (auraSpellInfo->SpellFamilyFlags & 0x0000000000080000LL)
            {
                pVictim->CastSpell(pVictim, trigger_spell_id, true, castItem, triggeredByAura);
                return true;                        // no hidden cooldown
            }
            break;
        default:
            break;
    }

    if( cooldown && GetTypeId()==TYPEID_PLAYER && (this->ToPlayer())->HasSpellCooldown(trigger_spell_id))
        return false;

    // try detect target manually if not set
    if ( target == NULL && pVictim)
    {
        // Do not allow proc negative spells on self
        if (GetGUID()==pVictim->GetGUID() && !(triggerEntry->IsPositive() || (ProcFlags & PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL)) && !(procEx & PROC_EX_REFLECT))
            return false;
        target = !(ProcFlags & PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL) && triggerEntry->IsPositive() ? this : pVictim;
    }

    // default case
    if(!target || (target != this && (!target->IsAlive() || !target->IsAttackableByAOE())))
        return false;

    // apply spell cooldown before casting to prevent triggering spells with SPELL_EFFECT_ADD_EXTRA_ATTACKS if spell has hidden cooldown
    if( cooldown && GetTypeId()==TYPEID_PLAYER )
        (this->ToPlayer())->AddSpellCooldown(trigger_spell_id,0,time(NULL) + cooldown);

    if(basepoints0)
        CastCustomSpell(target,trigger_spell_id,&basepoints0,NULL,NULL,true,castItem,triggeredByAura);
    else
        CastSpell(target,trigger_spell_id,true,castItem,triggeredByAura);

    return true;
}

bool Unit::HandleOverrideClassScriptAuraProc(Unit *pVictim, Aura *triggeredByAura, SpellInfo const *procSpell, uint32 cooldown)
{
    int32 scriptId = triggeredByAura->GetModifier()->m_miscvalue;

    if(!pVictim || !pVictim->IsAlive())
        return false;

    Item* castItem = triggeredByAura->GetCastItemGUID() && GetTypeId()==TYPEID_PLAYER
        ? (this->ToPlayer())->GetItemByGuid(triggeredByAura->GetCastItemGUID()) : NULL;

    uint32 triggered_spell_id = 0;

    switch(scriptId)
    {
        case 836:                                           // Improved Blizzard (Rank 1)
        {
            if (!procSpell || !procSpell->HasVisual(9487))
                return false;

            triggered_spell_id = 12484;
            break;
        }
        case 988:                                           // Improved Blizzard (Rank 2)
        {
            if (!procSpell || !procSpell->HasVisual(9487))
                return false;

            triggered_spell_id = 12485;
            break;
        }
        case 989:                                           // Improved Blizzard (Rank 3)
        {
            if (!procSpell || !procSpell->HasVisual(9487))
                return false;

            triggered_spell_id = 12486;
            break;
        }
        case 4086:                                          // Improved Mend Pet (Rank 1)
        case 4087:                                          // Improved Mend Pet (Rank 2)
        {
            int32 chance = triggeredByAura->GetSpellInfo()->Effects[triggeredByAura->GetEffIndex()].Effect;
            if(!roll_chance_i(chance))
                return false;

            triggered_spell_id = 24406;
            break;
        }
        case 4533:                                          // Dreamwalker Raiment 2 pieces bonus
        {
            // Chance 50%
            if (!roll_chance_i(50))
                return false;

            switch (pVictim->GetPowerType())
            {
                case POWER_MANA:   triggered_spell_id = 28722; break;
                case POWER_RAGE:   triggered_spell_id = 28723; break;
                case POWER_ENERGY: triggered_spell_id = 28724; break;
                default:
                    return false;
            }
            break;
        }
        case 4537:                                          // Dreamwalker Raiment 6 pieces bonus
            triggered_spell_id = 28750;                     // Blessing of the Claw
            break;
        case 5497:                                          // Improved Mana Gems (Serpent-Coil Braid)
            triggered_spell_id = 37445;                     // Mana Surge
            break;
    }

    // not processed
    if(!triggered_spell_id)
        return false;

    // standard non-dummy case
    SpellInfo const* triggerEntry = sSpellMgr->GetSpellInfo(triggered_spell_id);

    if(!triggerEntry)
    {
        TC_LOG_ERROR("FIXME","Unit::HandleOverrideClassScriptAuraProc: Spell %u triggering for class script id %u",triggered_spell_id,scriptId);
        return false;
    }

    if( cooldown && GetTypeId()==TYPEID_PLAYER && (this->ToPlayer())->HasSpellCooldown(triggered_spell_id))
        return false;

    CastSpell(pVictim, triggered_spell_id, true, castItem, triggeredByAura);

    if( cooldown && GetTypeId()==TYPEID_PLAYER )
        (this->ToPlayer())->AddSpellCooldown(triggered_spell_id,0,time(NULL) + cooldown);

    return true;
}

void Unit::SetPowerType(Powers new_powertype)
{
    SetByteValue(UNIT_FIELD_BYTES_0, 3, new_powertype);

    if(GetTypeId() == TYPEID_PLAYER)
    {
        if((this->ToPlayer())->GetGroup())
            (this->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_POWER_TYPE);
    }
    else if((this->ToCreature())->IsPet())
    {
        Pet *pet = ((Pet*)this);
        if(pet->isControlled())
        {
            Unit *owner = GetOwner();
            if(owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
                (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_POWER_TYPE);
        }
    }

    float powerMultiplier = 1.0f;
    if (!IsPet())
        if (Creature* creature = ToCreature())
            powerMultiplier = creature->GetCreatureTemplate()->ModMana;

    switch (new_powertype)
    {
        default:
        case POWER_MANA:
            break;
        case POWER_RAGE:
            SetMaxPower(POWER_RAGE, uint32(std::ceil(GetCreatePowers(POWER_RAGE) * powerMultiplier)));
            SetPower(POWER_RAGE, 0);
            break;
        case POWER_FOCUS:
            SetMaxPower(POWER_FOCUS, uint32(std::ceil(GetCreatePowers(POWER_FOCUS) * powerMultiplier)));
            SetPower(POWER_FOCUS, uint32(std::ceil(GetCreatePowers(POWER_FOCUS) * powerMultiplier)));
            break;
        case POWER_ENERGY:
            SetMaxPower(POWER_ENERGY, uint32(std::ceil(GetCreatePowers(POWER_ENERGY) * powerMultiplier)));
            break;
        case POWER_HAPPINESS:
            SetMaxPower(POWER_HAPPINESS, uint32(std::ceil(GetCreatePowers(POWER_HAPPINESS) * powerMultiplier)));
            SetPower(POWER_HAPPINESS, uint32(std::ceil(GetCreatePowers(POWER_HAPPINESS) * powerMultiplier)));
            break;
    }
}

FactionTemplateEntry const* Unit::GetFactionTemplateEntry() const
{
    FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(GetFaction());
    if(!entry)
    {
        static uint64 guid = 0;                             // prevent repeating spam same faction problem

        if(GetGUID() != guid)
        {
            if(GetTypeId() == TYPEID_PLAYER)
                TC_LOG_ERROR("FIXME","Player %s have invalid faction (faction template id) #%u", (this->ToPlayer())->GetName().c_str(), GetFaction());
            else
                TC_LOG_ERROR("FIXME","Creature (template id: %u) have invalid faction (faction template id) #%u", (this->ToCreature())->GetCreatureTemplate()->Entry, GetFaction());
            guid = GetGUID();
        }
    }
    return entry;
}

bool Unit::IsHostileTo(Unit const* unit) const
{
    
    
    // always non-hostile to self
    if (unit == this)
        return false;

    // always non-hostile to GM in GM mode
    if (unit->GetTypeId() == TYPEID_PLAYER && (((Player const*) unit)->IsGameMaster() || ((Player const*) unit)->isSpectator()))
        return false;

    // always hostile to current victim
    if (GetVictim() == unit || unit->GetVictim() == this)
        return true;

    // Karazhan chess exception
    if (GetFaction() == 1689 && unit->GetFaction() == 1690)
        return true;

    if (GetFaction() == 1690 && unit->GetFaction() == 1689)
        return true;

    // test pet/charm masters instead pers/charmeds
    Unit const* myOwner = GetCharmerOrOwner();
    Unit const* targetOwner = unit->GetCharmerOrOwner();

    // always hostile to owner's enemy
    if (myOwner && (myOwner->GetVictim() == unit || unit->GetVictim() == myOwner))
        return true;

    // always hostile to enemy owner
    if (targetOwner && (GetVictim() == targetOwner || targetOwner->GetVictim() == this))
        return true;

    // always hostile to owner of owner's enemy
    if (myOwner && targetOwner && (myOwner->GetVictim() == targetOwner || targetOwner->GetVictim() == myOwner))
        return true;

    Unit const* meOrMyOwner = myOwner ? myOwner : this;
    Unit const* target = targetOwner ? targetOwner : unit;

    // always non-hostile to target with common owner, or to owner/pet
    if (meOrMyOwner == target)
        return false;    

    // special cases (Duel, etc)
    if (meOrMyOwner->GetTypeId() == TYPEID_PLAYER && target->GetTypeId() == TYPEID_PLAYER) {
        Player const* pTester = (Player const*) meOrMyOwner;
        Player const* pTarget = (Player const*) target;

        // Duel
        if (pTester->duel && pTester->duel->opponent == pTarget && pTester->duel->startTime != 0)
            return true;

        // Duel area case
        if( (meOrMyOwner->ToPlayer() && meOrMyOwner->ToPlayer()->IsInDuelArea())
            || (pTarget->ToPlayer() && pTarget->ToPlayer()->IsInDuelArea())
          )
            return false;

        // Group
        if (pTester->GetGroup() && pTester->GetGroup() == pTarget->GetGroup())
            return false;

        // Sanctuary
        if (pTarget->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY) && pTester->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY))
            return false;

        // PvP FFA state
        if (pTester->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP) && pTarget->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP))
            return true;

        //= PvP states
        // Green/Blue (can't attack)
        if (pTester->GetTeam() == pTarget->GetTeam())
            return false;

        // Red (can attack) if true, Blue/Yellow (can't attack) in another case
        return pTester->IsPvP() && pTarget->IsPvP();
    }

    // faction base cases
    FactionTemplateEntry const* tester_faction = meOrMyOwner->GetFactionTemplateEntry();
    FactionTemplateEntry const* target_faction = target->GetFactionTemplateEntry();
    if (!tester_faction || !target_faction)
        return false;

    if (target->IsAttackingPlayer() && meOrMyOwner->IsContestedGuard())
        return true;

    // PvC forced reaction and reputation case
    if (meOrMyOwner->GetTypeId() == TYPEID_PLAYER) {
        // forced reaction
        ForcedReactions::const_iterator forceItr = (meOrMyOwner->ToPlayer())->m_forcedReactions.find(target_faction->faction);
        if (forceItr != (meOrMyOwner->ToPlayer())->m_forcedReactions.end())
            return forceItr->second <= REP_HOSTILE;

        // if faction have reputation then hostile state for tester at 100% dependent from at_war state
        if (FactionEntry const* raw_target_faction = sFactionStore.LookupEntry(target_faction->faction))
            if (raw_target_faction->reputationListID >= 0)
                if (FactionState const* factionState = (meOrMyOwner->ToPlayer())->GetFactionState(raw_target_faction))
                    return (factionState->Flags & FACTION_FLAG_AT_WAR);
    }        // CvP forced reaction and reputation case
    else if (target->GetTypeId() == TYPEID_PLAYER) {
        // forced reaction
        ForcedReactions::const_iterator forceItr = ((Player const*) target)->m_forcedReactions.find(tester_faction->faction);
        if (forceItr != ((Player const*) target)->m_forcedReactions.end())
            return forceItr->second <= REP_HOSTILE;

        // apply reputation state
        FactionEntry const* raw_tester_faction = sFactionStore.LookupEntry(tester_faction->faction);
        if (raw_tester_faction && raw_tester_faction->reputationListID >= 0)
            return ((Player const*) target)->GetReputationRank(raw_tester_faction) <= REP_HOSTILE;
    }

    // common faction based case (CvC,PvC,CvP)
    return tester_faction->IsHostileTo(*target_faction);
}

bool Unit::IsFriendlyTo(Unit const* unit) const
{
    // always friendly to self
    if(unit==this)
        return true;

    // always friendly to GM in GM mode
    if(unit->GetTypeId()==TYPEID_PLAYER && (((Player const*)unit)->IsGameMaster() || ((Player const*)unit)->isSpectator()))
        return true;

    // always non-friendly to enemy
    if(unit->GetTypeId()==TYPEID_UNIT && (GetVictim()==unit || unit->GetVictim()==this))
        return false;
        
    // Karazhan chess exception
    if (GetFaction() == 1689 && unit->GetFaction() == 1690)
        return false;
    
    if (GetFaction() == 1690 && unit->GetFaction() == 1689)
        return false; 

    // test pet/charm masters instead pers/charmeds
    Unit const* testerOwner = GetCharmerOrOwner();
    Unit const* targetOwner = unit->GetCharmerOrOwner();

    // always non-friendly to owner's enemy
    if(testerOwner && (testerOwner->GetVictim()==unit || unit->GetVictim()==testerOwner))
        return false;

    // always non-friendly to enemy owner
    if(targetOwner && (GetVictim()==targetOwner || targetOwner->GetVictim()==this))
        return false;

    // always non-friendly to owner of owner's enemy
    if(testerOwner && targetOwner && (testerOwner->GetVictim()==targetOwner || targetOwner->GetVictim()==testerOwner))
        return false;

    Unit const* tester = testerOwner ? testerOwner : this;
    Unit const* target = targetOwner ? targetOwner : unit;

    // always friendly to target with common owner, or to owner/pet
    if(tester==target)
        return true;

    // special cases (Duel)
    if(tester->GetTypeId()==TYPEID_PLAYER && target->GetTypeId()==TYPEID_PLAYER)
    {
        Player const* pTester = (Player const*)tester;
        Player const* pTarget = (Player const*)target;

        // Duel
        if(pTester->duel && pTester->duel->opponent == target && pTester->duel->startTime != 0)
            return false;

        // Group
        if(pTester->GetGroup() && pTester->GetGroup()==pTarget->GetGroup())
            return true;

        // Sanctuary
        if(pTarget->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY) && pTester->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY))
            return true;

        // PvP FFA state
        if(pTester->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP) && pTarget->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP))
            return false;

        //= PvP states
        // Green/Blue (non-attackable)
        if(pTester->GetTeam()==pTarget->GetTeam())
            return true;

        // Blue (friendly/non-attackable) if not PVP, or Yellow/Red in another case (attackable)
        return !pTarget->IsPvP();
    }

    // faction base cases
    FactionTemplateEntry const*tester_faction = tester->GetFactionTemplateEntry();
    FactionTemplateEntry const*target_faction = target->GetFactionTemplateEntry();
    if(!tester_faction || !target_faction)
        return false;

    if(target->IsAttackingPlayer() && tester->IsContestedGuard())
        return false;

    // PvC forced reaction and reputation case
    if(tester->GetTypeId()==TYPEID_PLAYER)
    {
        // forced reaction
        ForcedReactions::const_iterator forceItr = ((Player const*)tester)->m_forcedReactions.find(target_faction->faction);
        if(forceItr!=((Player const*)tester)->m_forcedReactions.end())
            return forceItr->second >= REP_FRIENDLY;

        // if faction have reputation then friendly state for tester at 100% dependent from at_war state
        if(FactionEntry const* raw_target_faction = sFactionStore.LookupEntry(target_faction->faction))
            if(raw_target_faction->reputationListID >=0)
                if(FactionState const* FactionState = (tester->ToPlayer())->GetFactionState(raw_target_faction))
                    return !(FactionState->Flags & FACTION_FLAG_AT_WAR);
    }
    // CvP forced reaction and reputation case
    else if(target->GetTypeId()==TYPEID_PLAYER)
    {
        // forced reaction
        ForcedReactions::const_iterator forceItr = ((Player const*)target)->m_forcedReactions.find(tester_faction->faction);
        if(forceItr!=((Player const*)target)->m_forcedReactions.end())
            return forceItr->second >= REP_FRIENDLY;

        // apply reputation state
        if(FactionEntry const* raw_tester_faction = sFactionStore.LookupEntry(tester_faction->faction))
            if(raw_tester_faction->reputationListID >=0 )
                return ((Player const*)target)->GetReputationRank(raw_tester_faction) >= REP_FRIENDLY;
    }

    // common faction based case (CvC,PvC,CvP)
    return tester_faction->IsFriendlyTo(*target_faction);
}

bool Unit::IsHostileToPlayers() const
{
    FactionTemplateEntry const* my_faction = GetFactionTemplateEntry();
    if(!my_faction)
        return false;

    FactionEntry const* raw_faction = sFactionStore.LookupEntry(my_faction->faction);
    if(raw_faction && raw_faction->reputationListID >=0 )
        return false;

    return my_faction->IsHostileToPlayers();
}

bool Unit::IsNeutralToAll() const
{
    FactionTemplateEntry const* my_faction = GetFactionTemplateEntry();
    if(!my_faction)
        return true;

    FactionEntry const* raw_faction = sFactionStore.LookupEntry(my_faction->faction);
    if(raw_faction && raw_faction->reputationListID >=0 )
        return false;

    return my_faction->IsNeutralToAll();
}

/* return true if we started attacking a new target */
bool Unit::Attack(Unit *victim, bool meleeAttack)
{
    if (!victim || victim == this)
        return false;

    // dead units can neither attack nor be attacked
    if (!IsAlive() || !victim->IsAlive())
        return false;

    // player cannot attack in mount state
    if (GetTypeId() == TYPEID_PLAYER) {
        if (IsMounted())
            return false;
    } else {
        Creature* c = victim->ToCreature();
        if (c && c->IsInEvadeMode())
            return false;
    }

    // nobody can attack GM in GM-mode
    if (victim->GetTypeId() == TYPEID_PLAYER) {
        if ((victim->ToPlayer())->IsGameMaster() || (victim->ToPlayer())->isSpectator())
            return false;
    } else {
        if ((victim->ToCreature())->IsInEvadeMode())
            return false;
    }

    // remove SPELL_AURA_MOD_UNATTACKABLE at attack (in case non-interruptible spells stun aura applied also that not let attack)
    if (HasAuraType(SPELL_AURA_MOD_UNATTACKABLE))
        RemoveAurasByType(SPELL_AURA_MOD_UNATTACKABLE);

    if (GetTypeId() == TYPEID_UNIT && GetStandState() == UNIT_STAND_STATE_DEAD)
        SetStandState(UNIT_STAND_STATE_STAND);

    //already attacking
    if (m_attacking) {
        if (m_attacking == victim) {
            // switch to melee attack from ranged/magic
            if (meleeAttack)
            {
                if(!HasUnitState(UNIT_STATE_MELEE_ATTACKING)) 
                {
                    AddUnitState(UNIT_STATE_MELEE_ATTACKING);
                    SendMeleeAttackStart(victim);
                    return true;
                }
            } else if (HasUnitState(UNIT_STATE_MELEE_ATTACKING)) 
            {
                ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
                SendMeleeAttackStop(victim); //melee attack stop
                return true;
            }

            return false;
        }

        AttackStop();
    }

    //Set our target
    SetTarget(victim->GetGUID());        

    m_attacking = victim;
    m_attacking->_addAttacker(this);

    //if(m_attacking->GetTypeId()==TYPEID_UNIT && (m->ToCreature()_attacking)->IsAIEnabled)
    //    (m->ToCreature()_attacking)->AI()->AttackedBy(this);

    if (GetTypeId() == TYPEID_UNIT && !(ToCreature()->IsPet())) {
        SendAIReaction(AI_REACTION_HOSTILE);
        ToCreature()->CallAssistance();

        // should not let player enter combat by right clicking target
        SetInCombatWith(victim);
        if (victim->GetTypeId() == TYPEID_PLAYER)
            victim->SetInCombatWith(this);
        else
            (victim->ToCreature())->AI()->AttackedBy(this);
        AddThreat(victim, 0.0f);
    }

    // delay offhand weapon attack to next attack time
    if (HaveOffhandWeapon())
        ResetAttackTimer(OFF_ATTACK);

    if (meleeAttack) 
    {
        AddUnitState(UNIT_STATE_MELEE_ATTACKING);
        SendMeleeAttackStart(victim);
    }

    return true;
}

bool Unit::IsCombatStationary()
{
    return IsInRoots();
}

bool Unit::AttackStop()
{
    if (!m_attacking)
        return false;

    Unit* victim = m_attacking;
    getThreatManager().clearCurrentVictim();

    m_attacking->_removeAttacker(this);
    m_attacking = NULL;

    //Clear our target
    SetTarget(0);

    ClearUnitState(UNIT_STATE_MELEE_ATTACKING);

    InterruptSpell(CURRENT_MELEE_SPELL);

    if( GetTypeId()==TYPEID_UNIT )
    {
        // reset call assistance
        (this->ToCreature())->SetNoCallAssistance(false);
    }

    SendMeleeAttackStop(victim);

    return true;
}

void Unit::CombatStop(bool cast)
{
    if(cast && IsNonMeleeSpellCast(false))
        InterruptNonMeleeSpells(false);

    AttackStop();
    RemoveAllAttackers();
    if( GetTypeId()==TYPEID_PLAYER )
        (this->ToPlayer())->SendAttackSwingCancelAttack();     // melee and ranged forced attack cancel
    ClearInCombat();
}

void Unit::CombatStopWithPets(bool cast)
{
    CombatStop(cast);
    if(Pet* pet = GetPet())
        pet->CombatStop(cast);
    if(Unit* charm = GetCharm())
        charm->CombatStop(cast);
    if(GetTypeId()==TYPEID_PLAYER)
    {
        GuardianPetList const& guardians = (this->ToPlayer())->GetGuardians();
        for(GuardianPetList::const_iterator itr = guardians.begin(); itr != guardians.end(); ++itr)
            if(Unit* guardian = ObjectAccessor::GetUnit(*this,*itr))
                guardian->CombatStop(cast);
    }
}

bool Unit::IsAttackingPlayer() const
{
    if(HasUnitState(UNIT_STATE_ATTACK_PLAYER))
        return true;

    Pet* pet = GetPet();
    if(pet && pet->IsAttackingPlayer())
        return true;

    Unit* charmed = GetCharm();
    if(charmed && charmed->IsAttackingPlayer())
        return true;

    for (int8 i = 0; i < MAX_TOTEM; i++)
    {
        if(m_TotemSlot[i])
        {
            Creature *totem = ObjectAccessor::GetCreature(*this, m_TotemSlot[i]);
            if(totem && totem->IsAttackingPlayer())
                return true;
        }
    }

    return false;
}

void Unit::RemoveAllAttackers()
{
    while (!m_attackers.empty())
    {
        AttackerSet::iterator iter = m_attackers.begin();
        if(!(*iter)->AttackStop())
        {
            TC_LOG_ERROR("FIXME","WORLD: Unit has an attacker that isn't attacking it!");
            m_attackers.erase(iter);
        }
    }
}

void Unit::ModifyAuraState(AuraStateType flag, bool apply)
{
    if (!flag)
        return;

    if (apply)
    {
        if (!HasFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1)))
        {
            SetFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1));
            if(GetTypeId() == TYPEID_PLAYER)
            {
                const PlayerSpellMap& sp_list = (this->ToPlayer())->GetSpellMap();
                for (PlayerSpellMap::const_iterator itr = sp_list.begin(); itr != sp_list.end(); ++itr)
                {
                    if(itr->second->state == PLAYERSPELL_REMOVED) continue;
                    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(itr->first);
                    if (!spellInfo || !spellInfo->IsPassive()) continue;
                    if (spellInfo->CasterAuraState == flag)
                        CastSpell(this, itr->first, true, NULL);
                }
            }
        }
    }
    else
    {
        if (HasFlag(UNIT_FIELD_AURASTATE,1<<(flag-1)))
        {
            RemoveFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1));
            Unit::AuraMap& tAuras = GetAuras();
            for (Unit::AuraMap::iterator itr = tAuras.begin(); itr != tAuras.end();)
            {
                SpellInfo const* spellProto = (*itr).second->GetSpellInfo();
                if (spellProto->CasterAuraState == flag)
                {
                    // exceptions (applied at state but not removed at state change)
                    // Rampage
                    if(spellProto->SpellIconID==2006 && spellProto->SpellFamilyName==SPELLFAMILY_WARRIOR && spellProto->SpellFamilyFlags==0x100000)
                    {
                        ++itr;
                        continue;
                    }

                    RemoveAura(itr);
                }
                else
                    ++itr;
            }
        }
    }
}

uint32 Unit::BuildAuraStateUpdateForTarget(Unit* target) const
{
    uint32 auraStates = GetUInt32Value(UNIT_FIELD_AURASTATE) &~(PER_CASTER_AURA_STATE_MASK);
    for (AuraStateAurasMap::const_iterator itr = m_auraStateAuras.begin(); itr != m_auraStateAuras.end(); ++itr)
        if ((1 << (itr->first - 1)) & PER_CASTER_AURA_STATE_MASK)
            if (itr->second->GetCasterGUID() == target->GetGUID())
                auraStates |= (1 << (itr->first - 1));

    return auraStates;
}

bool Unit::HasAuraState(AuraStateType flag, SpellInfo const* spellProto, Unit const* Caster) const
{
    if (Caster)
    {
#ifdef LICH_KING
        if (spellProto)
        {
            auto stateAuras = Caster->GetAurasByType(SPELL_AURA_ABILITY_IGNORE_AURASTATE);
            AuraEffectList const& stateAuras = Caster->GetAuraEffectsByType(SPELL_AURA_ABILITY_IGNORE_AURASTATE);
            for (AuraEffectList::const_iterator j = stateAuras.begin(); j != stateAuras.end(); ++j)
                if ((*j)->IsAffectedOnSpell(spellProto))
                    return true;
        } 
#endif
        // Check per caster aura state
        // If aura with aurastate by caster not found return false
        if ((1 << (flag - 1)) & PER_CASTER_AURA_STATE_MASK)
        {
            AuraStateAurasMapBounds range = m_auraStateAuras.equal_range(flag);
            for (AuraStateAurasMap::const_iterator itr = range.first; itr != range.second; ++itr)
                if (itr->second->GetCasterGUID() == Caster->GetGUID())
                    return true;
            return false;
        }
    }

    return HasFlag(UNIT_FIELD_AURASTATE, 1 << (flag - 1));
}

Unit *Unit::GetOwner() const
{
    uint64 ownerid = GetOwnerGUID();
    if(!ownerid)
        return NULL;
    return ObjectAccessor::GetUnit(*this, ownerid);
}

Unit *Unit::GetCharmer() const
{
    if(uint64 charmerid = GetCharmerGUID())
        return ObjectAccessor::GetUnit(*this, charmerid);
    return NULL;
}

Player* Unit::GetCharmerOrOwnerPlayerOrPlayerItself() const
{
    uint64 guid = GetCharmerOrOwnerGUID();
    if(IS_PLAYER_GUID(guid))
        return ObjectAccessor::GetPlayer(*this, guid);

    Player *p = const_cast<Player*>(ToPlayer());

    return GetTypeId()==TYPEID_PLAYER ? p : NULL;
}

Pet* Unit::GetPet() const
{
    if(uint64 pet_guid = GetPetGUID())
    {
        if(!IS_PET_GUID(pet_guid))
            return nullptr;

        Pet* pet = ObjectAccessor::GetPet(*this, pet_guid);

        if (!pet)
            return nullptr;

        if (IsInWorld() && pet)
            return pet;

        //there may be a guardian in slot
        //TC_LOG_ERROR("entities.pet","Unit::GetPet: Pet %u not exist.",GUID_LOPART(pet_guid));
    }

    return nullptr;
}

Unit* Unit::GetCharm() const
{
    if(uint64 charm_guid = GetCharmGUID())
    {
        if(Unit* pet = ObjectAccessor::GetUnit(*this, charm_guid))
            return pet;

        TC_LOG_ERROR("entities.pet","Unit::GetCharm: Charmed creature %u not exist.",GUID_LOPART(charm_guid));
        const_cast<Unit*>(this)->SetCharm(0);
    }

    return NULL;
}

void Unit::SetPet(Pet* pet)
{
    SetUInt64Value(UNIT_FIELD_SUMMON, pet ? pet->GetGUID() : 0);

    // FIXME: hack, speed must be set only at follow
    if(pet)
        for(int i = 0; i < MAX_MOVE_TYPE; ++i)
            if(m_speed_rate[i] > 1.0f)
                pet->SetSpeed(UnitMoveType(i), m_speed_rate[i], true);
}

void Unit::SetCharm(Unit* pet)
{
    if(GetTypeId() == TYPEID_PLAYER)
        SetUInt64Value(UNIT_FIELD_CHARM, pet ? pet->GetGUID() : 0);
}

void Unit::AddPlayerToVision(Player* plr)
{
    if(m_sharedVision.empty())
    {
        //set active so that creatures around in grid are active as well
        SetKeepActive(true);
        SetWorldObject(true);
    }
    m_sharedVision.push_back(plr->GetGUID());
    plr->SetFarsightTarget(this);
}

void Unit::RemovePlayerFromVision(Player* plr)
{
    m_sharedVision.remove(plr->GetGUID());
    if(m_sharedVision.empty())
    {
        SetKeepActive(false);
        SetWorldObject(false);
    }
    plr->ClearFarsight();
}

void Unit::RemoveBindSightAuras()
{
    RemoveAurasByType(SPELL_AURA_BIND_SIGHT);
}

void Unit::RemoveCharmAuras()
{
    RemoveAurasByType(SPELL_AURA_MOD_CHARM);
    RemoveAurasByType(SPELL_AURA_MOD_POSSESS_PET);
    RemoveAurasByType(SPELL_AURA_MOD_POSSESS);
}

void Unit::UnsummonAllTotems()
{
    for (int8 i = 0; i < MAX_TOTEM; ++i)
    {
        if(!m_TotemSlot[i])
            continue;

        Creature *OldTotem = ObjectAccessor::GetCreature(*this, m_TotemSlot[i]);
        if (OldTotem && OldTotem->IsTotem())
            ((Totem*)OldTotem)->UnSummon();
    }
}

void Unit::SendHealSpellLog(Unit *pVictim, uint32 SpellID, uint32 Damage, bool critical)
{
    // we guess size
    WorldPacket data(SMSG_SPELLHEALLOG, (8+8+4+4+1));
    data << pVictim->GetPackGUID();
    data << GetPackGUID();
    data << uint32(SpellID);
    data << uint32(Damage);
    data << uint8(critical ? 1 : 0);
    data << uint8(0);                                       // unused in client?
    SendMessageToSet(&data, true);
}

void Unit::SendEnergizeSpellLog(Unit *pVictim, uint32 SpellID, uint32 Damage, Powers powertype)
{
    WorldPacket data(SMSG_SPELLENERGIZELOG, (8+8+4+4+4+1));
    data << pVictim->GetPackGUID();
    data << GetPackGUID();
    data << uint32(SpellID);
    data << uint32(powertype);
    data << uint32(Damage);
    SendMessageToSet(&data, true);
}

uint32 Unit::GetCastingTimeForBonus(SpellInfo const* spellProto, DamageEffectType damagetype, uint32 CastingTime) const
{
    // Not apply this to creature casted spells with casttime == 0
    if (CastingTime == 0 && GetTypeId() == TYPEID_UNIT && !IsPet())
        return 3500;

    if (CastingTime > 7000) CastingTime = 7000;
    if (CastingTime < 1500) CastingTime = 1500;

    if (damagetype == DOT && !spellProto->IsChanneled())
        CastingTime = 3500;

    int32 overTime = 0;
    uint8 effects = 0;
    bool DirectDamage = false;
    bool AreaEffect = false;

    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        switch (spellProto->Effects[i].Effect)
        {
        case SPELL_EFFECT_SCHOOL_DAMAGE:
        case SPELL_EFFECT_POWER_DRAIN:
        case SPELL_EFFECT_HEALTH_LEECH:
        case SPELL_EFFECT_ENVIRONMENTAL_DAMAGE:
        case SPELL_EFFECT_POWER_BURN:
        case SPELL_EFFECT_HEAL:
            DirectDamage = true;
            break;
        case SPELL_EFFECT_APPLY_AURA:
            switch (spellProto->Effects[i].ApplyAuraName)
            {
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PERIODIC_HEAL:
            case SPELL_AURA_PERIODIC_LEECH:
                if (spellProto->GetDuration())
                    overTime = spellProto->GetDuration();
                break;
            default:
                /* From sunwell core. Why ?
                // -5% per additional effect
                ++effects;
                */
                break;
            }
        default:
            break;
        }

        if (spellProto->Effects[i].IsTargetingArea())
            AreaEffect = true;
    }

    // Combined Spells with Both Over Time and Direct Damage
    if (overTime > 0 && CastingTime > 0 && DirectDamage)
    {
        // mainly for DoTs which are 3500 here otherwise
        uint32 OriginalCastTime = spellProto->CalcCastTime();
        if (OriginalCastTime > 7000) OriginalCastTime = 7000;
        if (OriginalCastTime < 1500) OriginalCastTime = 1500;
        // Portion to Over Time
        float PtOT = (overTime / 15000.0f) / ((overTime / 15000.0f) + (OriginalCastTime / 3500.0f));

        if (damagetype == DOT)
            CastingTime = uint32(CastingTime * PtOT);
        else if (PtOT < 1.0f)
            CastingTime = uint32(CastingTime * (1 - PtOT));
        else
            CastingTime = 0;
    }

    // Area Effect Spells receive only half of bonus
    if (AreaEffect)
        CastingTime /= 2;

    // 50% for damage and healing spells for leech spells from damage bonus and 0% from healing
    for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
    {
        if (spellProto->Effects[j].Effect == SPELL_EFFECT_HEALTH_LEECH ||
            (spellProto->Effects[j].Effect == SPELL_EFFECT_APPLY_AURA && spellProto->Effects[j].ApplyAuraName == SPELL_AURA_PERIODIC_LEECH))
        {
            CastingTime /= 2;
            break;
        }
    }

    /* From sunwellcore, why ? Is this LK ?
    // -5% of total per any additional effect
    for (uint8 i = 0; i < effects; ++i)
        CastingTime *= 0.95f;
        */

    return CastingTime;
}

float Unit::CalculateDefaultCoefficient(SpellInfo const *spellInfo, DamageEffectType damagetype) const
{
    float forceDotCoeff = 0.0f;
    int32 CastingTime = 0;
    //HACK TIME. This should move into spell_bonus_data table one day (when it exists)
    switch (spellInfo->SpellFamilyName)
    {
    case SPELLFAMILY_WARRIOR:
    case SPELLFAMILY_ROGUE:
    case SPELLFAMILY_HUNTER:
        return 0.0f;
        /*
    case SPELLFAMILY_GENERIC:
        // Siphon Essence - 0%
        if (spellInfo->AttributesEx == 0x10000000 && spellInfo->SpellIconID == 2027)
            return 0.0f;
        // Goblin Rocket Launcher - 0%
        else if (spellInfo->SpellIconID == 184 && spellInfo->Attributes == 4259840)
            return 0.0f; 
        // Darkmoon Card: Vengeance - 0.1%
        else if (spellInfo->HasVisual(9850) && spellInfo->SpellIconID == 2230)
            CastingTime = 3500;
        break;
    case SPELLFAMILY_MAGE:
        // Pyroblast - 115% of Fire Damage, DoT - 20% of Fire Damage
        if ((spellInfo->SpellFamilyFlags & 0x400000LL) && spellInfo->SpellIconID == 184)
        {
            forceDotCoeff = damagetype == DOT ? 0.2f : 1.0f;
            CastingTime = damagetype == DOT ? 3500 : 4025;
        }
        // Fireball - 100% of Fire Damage, DoT - 0% of Fire Damage
        else if ((spellInfo->SpellFamilyFlags & 0x1LL) && spellInfo->SpellIconID == 185)
        {
            CastingTime = 3500;
            forceDotCoeff = damagetype == DOT ? 0.0f : 1.0f;
        }
        // Molten armor
        else if (spellInfo->SpellFamilyFlags & 0x0000000800000000LL)
        {
            CastingTime = 0;
        }
        // Arcane Missiles triggered spell
        else if ((spellInfo->SpellFamilyFlags & 0x200000LL) && spellInfo->SpellIconID == 225)
        {
            CastingTime = 1000;
        }
        // Blizzard triggered spell
        else if ((spellInfo->SpellFamilyFlags & 0x80080LL) && spellInfo->SpellIconID == 285)
        {
            CastingTime = 500;
        }
        break;*/
    case SPELLFAMILY_WARLOCK:
        /*
        // Life Tap
        if ((spellInfo->SpellFamilyFlags & 0x40000LL) && spellInfo->SpellIconID == 208)
        {
            CastingTime = 2800;                         // 80% from +shadow damage
        }
        */
        // Dark Pact
        if ((spellInfo->SpellFamilyFlags & 0x80000000LL) && spellInfo->SpellIconID == 154 && GetPetGUID())
        {
            CastingTime = 3360;                         // 96% from +shadow damage
        }
        /*
        // Soul Fire - 115% of Fire Damage
        else if ((spellInfo->SpellFamilyFlags & 0x8000000000LL) && spellInfo->SpellIconID == 184)
        {
            CastingTime = 4025;
        }
        // Curse of Agony - 120% of Shadow Damage
        else if ((spellInfo->SpellFamilyFlags & 0x0000000400LL) && spellInfo->SpellIconID == 544)
        {
            forceDotCoeff = 1.2f;
        }
        // Drain Mana - 0% of Shadow Damage
        else if ((spellInfo->SpellFamilyFlags & 0x10LL) && spellInfo->SpellIconID == 548)
        {
            CastingTime = 0;
        }
        // Drain Soul 214.3%
        else if ((spellInfo->SpellFamilyFlags & 0x4000LL) && spellInfo->SpellIconID == 113)
        {
            CastingTime = 7500;
        }
        // Hellfire
        else if ((spellInfo->SpellFamilyFlags & 0x40LL) && spellInfo->SpellIconID == 937)
        {
            CastingTime = damagetype == DOT ? 5000 : 500; // self damage seems to be so
        }
        // Unstable Affliction - 180%
        else if (spellInfo->Id == 31117 && spellInfo->SpellIconID == 232)
        {
            CastingTime = 6300;
        }
        // Corruption 93%
        else if ((spellInfo->SpellFamilyFlags & 0x2LL) && spellInfo->SpellIconID == 313)
        {
            forceDotCoeff = 0.93f;
        }
        */
        break;
    case SPELLFAMILY_PALADIN:
        /*
        // Seal and Judgement of Light
        if (spellInfo->SpellFamilyFlags & 0x100040000LL)
            CastingTime = 0;

        // Consecration - 95% of Holy Damage
        if ((spellInfo->SpellFamilyFlags & 0x20LL) && spellInfo->SpellIconID == 51)
        {
            forceDotCoeff = 0.95f;
            CastingTime = 3500;
        }
        */
        // Seal of Righteousness - 10.2%/9.8% ( based on weapon type ) of Holy Damage, multiplied by weapon speed
        if ((spellInfo->SpellFamilyFlags & 0x8000000LL) && spellInfo->SpellIconID == 25)
        {
            Item *item = (this->ToPlayer())->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            float wspeed = GetAttackTime(BASE_ATTACK) / 1000.0f;

            if (item && item->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                CastingTime = uint32(wspeed * 3500 * 0.102f);
            else
                CastingTime = uint32(wspeed * 3500 * 0.098f);
        }
        /*
        // Judgement of Righteousness - 73%
        else if ((spellInfo->SpellFamilyFlags & 1024) && spellInfo->SpellIconID == 25)
        {
            CastingTime = 2555;
        }
        // Seal of Vengeance - 17% per Fully Stacked Tick - 5 Applications
        else if ((spellInfo->SpellFamilyFlags & 0x80000000000LL) && spellInfo->SpellIconID == 2292)
        {
            forceDotCoeff = 0.85f;
            CastingTime = 1850;
        }
        // Holy shield - 5% of Holy Damage
        else if ((spellInfo->SpellFamilyFlags & 0x4000000000LL) && spellInfo->SpellIconID == 453)
        {
            CastingTime = 175;
        }
        // Blessing of Sanctuary - 0%
        else if ((spellInfo->SpellFamilyFlags & 0x10000000LL) && spellInfo->SpellIconID == 29)
        {
            CastingTime = 0;
        }
        break;
    case  SPELLFAMILY_SHAMAN:
        // Healing stream from totem (add 6% per tick from hill bonus owner)
        if (spellInfo->SpellFamilyFlags & 0x000000002000LL)
            CastingTime = 210;
        // Earth Shield 30% per charge
        else if (spellInfo->SpellFamilyFlags & 0x40000000000LL)
            CastingTime = 1050;
        // totem attack
        else if (spellInfo->SpellFamilyFlags & 0x000040000000LL)
        {
            if (spellInfo->SpellIconID == 33)          // Fire Nova totem attack must be 21.4%(untested)
                CastingTime = 749;                      // ignore CastingTime and use as modifier
            else if (spellInfo->SpellIconID == 680)    // Searing Totem attack 8%
                CastingTime = 280;                      // ignore CastingTime and use as modifier
            else if (spellInfo->SpellIconID == 37)     // Magma totem attack must be 6.67%(untested)
                CastingTime = 234;                      // ignore CastingTimePenalty and use as modifier
        }
        // Lightning Shield (and proc shield from T2 8 pieces bonus ) 33% per charge
        else if ((spellInfo->SpellFamilyFlags & 0x00000000400LL) || spellInfo->Id == 23552)
            CastingTime = 1155;                         // ignore CastingTimePenalty and use as modifier
        break;
    case SPELLFAMILY_PRIEST:
        // Holy Nova - 14%
        if ((spellInfo->SpellFamilyFlags & 0x8000000LL) && spellInfo->SpellIconID == 1874)
            CastingTime = 500;
        // Mana Burn - 0% of Shadow Damage
        else if ((spellInfo->SpellFamilyFlags & 0x10LL) && spellInfo->SpellIconID == 212)
        {
            CastingTime = 0;
        }
        // Mind Flay - 59% of Shadow Damage
        else if ((spellInfo->SpellFamilyFlags & 0x800000LL) && spellInfo->SpellIconID == 548)
        {
            CastingTime = 2065;
        }
        // Holy Fire - 86.71%, DoT - 16.5%
        else if ((spellInfo->SpellFamilyFlags & 0x100000LL) && spellInfo->SpellIconID == 156)
        {
            forceDotCoeff = damagetype == DOT ? 0.165f : 1.0f;
            CastingTime = damagetype == DOT ? 3500 : 3035;
        }
        // Shadowguard - 28% per charge
        else if ((spellInfo->SpellFamilyFlags & 0x2000000LL) && spellInfo->SpellIconID == 19)
        {
            CastingTime = 980;
        }
        // Touch of Weakeness - 10%
        else if ((spellInfo->SpellFamilyFlags & 0x80000LL) && spellInfo->SpellIconID == 1591)
        {
            CastingTime = 350;
        }
        // Reflective Shield (back damage) - 0% (other spells fit to check not have damage effects/auras)
        else if (spellInfo->SpellFamilyFlags == 0 && spellInfo->SpellIconID == 566)
        {
            CastingTime = 0;
        }
        // Holy Nova - 14%
        else if ((spellInfo->SpellFamilyFlags & 0x400000LL) && spellInfo->SpellIconID == 1874)
        {
            CastingTime = 500;
        }
        break;
    case SPELLFAMILY_DRUID:
        // Hurricane triggered spell
        if ((spellInfo->SpellFamilyFlags & 0x400000LL) && spellInfo->SpellIconID == 220)
        {
            CastingTime = 500;
        }
        // Lifebloom
        else if (spellInfo->SpellFamilyFlags & 0x1000000000LL)
        {
            CastingTime = damagetype == DOT ? 3500 : 1200;
            forceDotCoeff = damagetype == DOT ? 0.519f : 1.0f;
        }
        // Tranquility triggered spell
        else if (spellInfo->SpellFamilyFlags & 0x80LL)
            CastingTime = 667;
        // Regrowth
        else if (spellInfo->SpellFamilyFlags & 0x40LL)
        {
            forceDotCoeff = damagetype == DOT ? 0.705f : 1.0f;
            CastingTime = damagetype == DOT ? 3500 : 1010;
        }
        // Improved Leader of the Pack
        else if (spellInfo->AttributesEx2 == 536870912 && spellInfo->SpellIconID == 312
            && spellInfo->AttributesEx3 == 33554432)
        {
            CastingTime = 0;
        }
        */
        break;
    default:
        break;
    }

    // Damage over Time spells bonus calculation
    float DotFactor = 1.0f;
    if (damagetype == DOT)
    {
        int32 DotDuration = spellInfo->GetDuration();
        if (forceDotCoeff == 0.0f)
        {
            if (!spellInfo->IsChanneled() && DotDuration > 0)
                DotFactor = DotDuration / 15000.0f;
        }
        else {
            DotFactor = forceDotCoeff;
        }

        if (uint32 DotTicks = spellInfo->GetMaxTicks())
            DotFactor /= DotTicks;
    }

    //if not hacked
    if (CastingTime == 0)
    {
        CastingTime = spellInfo->IsChanneled() ? spellInfo->GetDuration() : spellInfo->CalcCastTime();
        // Distribute Damage over multiple effects, reduce by AoE
        CastingTime = GetCastingTimeForBonus(spellInfo, damagetype, CastingTime);
    }

    // As wowwiki says: C = (Cast Time / 3.5)
    return (CastingTime / 3500.0f) * DotFactor;
}

uint32 Unit::SpellDamageBonusTaken(Unit* caster, SpellInfo const *spellProto, uint32 pdamage, DamageEffectType damagetype, uint32 stack /* = 1 */)
{
    if (!spellProto || damagetype == DIRECT_DAMAGE)
        return pdamage;

    int32 TakenTotal = 0;
    float TakenTotalMod = 1.0f;

    // ..taken
    AuraList const& mModDamagePercentTaken = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    for (AuraList::const_iterator i = mModDamagePercentTaken.begin(); i != mModDamagePercentTaken.end(); ++i)
        if ((*i)->GetModifier()->m_miscvalue & spellProto->GetSchoolMask())
            AddPct(TakenTotalMod, (*i)->GetModifierValue());


    bool hasmangle = false;
    // .. taken pct: dummy auras
    AuraList const& mDummyAuras = GetAurasByType(SPELL_AURA_DUMMY);
    for (AuraList::const_iterator i = mDummyAuras.begin(); i != mDummyAuras.end(); ++i)
    {
        switch ((*i)->GetSpellInfo()->SpellIconID)
        {
            //Cheat Death
        case 2109:
            if (((*i)->GetModifier()->m_miscvalue & spellProto->GetSchoolMask()))
            {
                if (GetTypeId() != TYPEID_PLAYER)
                    continue;
                float mod = -1.0f * (ToPlayer())->GetRatingBonusValue(CR_CRIT_TAKEN_SPELL) * 2 * 4;
                AddPct(TakenTotalMod, std::max(mod, float((*i)->GetModifier()->m_amount)));
                TakenTotalMod *= (mod + 100.0f) / 100.0f;
            }
            break;
            //This is changed in WLK, using aura 255
            //Mangle bear and cat. This hack should be removed 
        case 2312:
        case 44955:
            // don't apply mod twice
            if (hasmangle)
                break;
            hasmangle = true;
            for (int j = 0; j<3; j++)
            {
                if (GetEffectMechanic(spellProto, j) == MECHANIC_BLEED)
                {
                    TakenTotalMod *= (100.0f + (*i)->GetModifier()->m_amount) / 100.0f;
                    break;
                }
            }
            break;

        }
    }

#ifdef LICH_KING
    // From caster spells
    if (caster)
    {
        AuraEffectList const& mOwnerTaken = GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_FROM_CASTER);
        for (AuraEffectList::const_iterator i = mOwnerTaken.begin(); i != mOwnerTaken.end(); ++i)
            if ((*i)->GetCasterGUID() == caster->GetGUID() && (*i)->IsAffectedOnSpell(spellProto))
                if (spellProto->ValidateAttribute6SpellDamageMods(caster, *i, damagetype == DOT))
                    AddPct(TakenTotalMod, (*i)->GetAmount());
    }

    if (uint32 mechanicMask = spellProto->GetAllEffectsMechanicMask())
    {
        int32 modifierMax = 0;
        int32 modifierMin = 0;
        AuraEffectList const& mTotalAuraList = GetAuraEffectsByType(SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT);
        for (AuraEffectList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
        {
            if (!spellProto->ValidateAttribute6SpellDamageMods(caster, *i, damagetype == DOT))
                continue;

            // Only death knight spell with this aura, ZOMG!
            if ((*i)->GetSpellInfo()->SpellFamilyName == SPELLFAMILY_DEATHKNIGHT)
                if (!caster || caster->GetGUID() != (*i)->GetCasterGUID())
                    continue;

            if (mechanicMask & uint32(1 << (*i)->GetMiscValue()))
            {
                if ((*i)->GetAmount() > 0)
                {
                    if ((*i)->GetAmount() > modifierMax)
                        modifierMax = (*i)->GetAmount();
                }
                else if ((*i)->GetAmount() < modifierMin)
                    modifierMin = (*i)->GetAmount();
            }
        }

        AddPct(TakenTotalMod, modifierMax);
        AddPct(TakenTotalMod, modifierMin);
    }
#endif

    int32 TakenAdvertisedBenefit = SpellBaseDamageBonusTaken(spellProto->GetSchoolMask(), damagetype == DOT);
    float coeff = 0;
    SpellBonusEntry const* bonus = sSpellMgr->GetSpellBonusData(spellProto->Id);
    if (bonus)
        coeff = (damagetype == DOT) ? bonus->dot_damage : bonus->direct_damage;

    // Default calculation
    if (TakenAdvertisedBenefit)
    {
        if (coeff <= 0.0f)
        {
            if (caster)
                coeff = caster->CalculateDefaultCoefficient(spellProto, damagetype) * int32(stack);
            else
                coeff = CalculateDefaultCoefficient(spellProto, damagetype) * int32(stack);
        }
        float factorMod = CalculateLevelPenalty(spellProto) * stack;
        TakenTotal += int32(TakenAdvertisedBenefit * coeff * factorMod);
    }

#ifdef LICH_KING

    // xinef: sanctified wrath talent
    if (caster && TakenTotalMod < 1.0f && caster->HasAuraType(SPELL_AURA_MOD_IGNORE_TARGET_RESIST))
    {
        float ignoreModifier = 1.0f - TakenTotalMod;
        bool addModifier = false;
        AuraEffectList const& ResIgnoreAuras = caster->GetAuraEffectsByType(SPELL_AURA_MOD_IGNORE_TARGET_RESIST);
        for (AuraEffectList::const_iterator j = ResIgnoreAuras.begin(); j != ResIgnoreAuras.end(); ++j)
            if ((*j)->GetMiscValue() & spellProto->SchoolMask)
            {
                ApplyPct(ignoreModifier, (*j)->GetAmount());
                addModifier = true;
            }

        if (addModifier)
            TakenTotalMod += ignoreModifier;
    }
#endif

    float tmpDamage = (float(pdamage) + TakenTotal) * TakenTotalMod;

    return uint32(std::max(tmpDamage, 0.0f));
}

uint32 Unit::SpellDamageBonusDone(Unit* victim, SpellInfo const *spellProto, uint32 pdamage, DamageEffectType damagetype, float TotalMod, uint32 stack)
{
    //HACK TIME
    switch (spellProto->SpellFamilyName)
    {
    case SPELLFAMILY_MAGE:
        // Ignite - do not modify, it is (8*Rank)% damage of procing Spell
        if (spellProto->Id == 12654)
            return pdamage;
        break;
    case SPELLFAMILY_PALADIN:
        if (spellProto->SpellFamilyName == SPELLFAMILY_PALADIN && spellProto->SpellIconID == 25 && spellProto->HasAttribute(SPELL_ATTR4_UNK23))
            return pdamage;
    }

    if(!spellProto || !victim || damagetype == DIRECT_DAMAGE )
        return pdamage;
        
    if (spellProto->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS))
        return pdamage;

    int32 BonusDamage = 0;
    if( GetTypeId()==TYPEID_UNIT )
    {
        // Pets just add their bonus damage to their spell damage
        // note that their spell damage is just gain of their own auras
        if ((this->ToCreature())->IsPet() && spellProto->DmgClass == SPELL_DAMAGE_CLASS_MAGIC)
        {
            BonusDamage = ((Pet*)this)->GetBonusDamage();
        }
        // For totems get damage bonus from owner (statue isn't totem in fact)
        else if ((this->ToCreature())->IsTotem() && ((Totem*)this)->GetTotemType()!=TOTEM_STATUE)
        {
            if(Unit* owner = GetOwner())
                return owner->SpellDamageBonusDone(victim, spellProto, pdamage, damagetype, TotalMod, stack);
        }
#ifdef LICH_KING
        // Dancing Rune Weapon...
        else if (GetEntry() == 27893)
        {
            if (Unit* owner = GetOwner())
                return owner->SpellDamageBonusDone(victim, spellProto, pdamage, damagetype, TotalMod, stack) / 2;
        }
#endif
    }

    // Done total percent damage auras
    float ApCoeffMod = 1.0f;
    int32 DoneTotal = 0;
    float DoneTotalMod = TotalMod ? TotalMod : SpellPctDamageModsDone(victim, spellProto, damagetype);

    uint32 creatureTypeMask = victim->GetCreatureTypeMask();
    // Add flat bonus from spell damage versus
    DoneTotal += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS, creatureTypeMask);

    // done scripted mod (take it from owner)
    Unit* owner = GetOwner() ? GetOwner() : this;
    AuraList const& mOverrideClassScript = owner->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraList::const_iterator i = mOverrideClassScript.begin(); i != mOverrideClassScript.end(); ++i)
    {
        if(!sSpellMgr->IsAffectedBySpell(spellProto, (*i)->GetId(), (*i)->GetEffIndex(), 0 ))
            continue;
        /*sif (!(*i)->IsAffectedOnSpell(spellProto))
            continue;*/

        switch ((*i)->GetMiscValue())
        {
        case 4418: // Increased Shock Damage
        case 4554: // Increased Lightning Damage
        case 4555: // Improved Moonfire
        case 5142: // Increased Lightning Damage
        case 5147: // Improved Consecration / Libram of Resurgence
        case 5148: // Idol of the Shooting Star
        case 6008: // Increased Lightning Damage
        case 8627: // Totem of Hex
        {
            DoneTotal += (*i)->GetAmount();
            break;
        }
        }
    }

#ifdef LICH_KING
    // Custom scripted damage
    if (spellProto->SpellFamilyName == SPELLFAMILY_DEATHKNIGHT)
    {
        // Sigil of the Vengeful Heart
        if (spellProto->SpellFamilyFlags[0] & 0x2000)
            if (AuraEffect* aurEff = GetAuraEffect(64962, EFFECT_1))
                AddPct(DoneTotal, aurEff->GetAmount());

        // Impurity
        if (AuraEffect *aurEff = GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 1986, 0))
            AddPct(ApCoeffMod, aurEff->GetAmount());

        // Blood Boil - bonus for diseased targets
        if (spellProto->SpellFamilyFlags[0] & 0x00040000)
            if (victim->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_DEATHKNIGHT, 0, 0, 0x00000002, GetGUID()))
            {
                DoneTotal += 95;
                ApCoeffMod = 1.5835f;
            }
    }
#endif

    // Taken/Done fixed damage bonus auras
    int32 DoneAdvertisedBenefit = SpellBaseDamageBonusDone(spellProto->GetSchoolMask()) + BonusDamage;

    float coeff = 0.0f;
    SpellBonusEntry const* bonus = sSpellMgr->GetSpellBonusData(spellProto->Id);
    if (bonus)
    {
        if (damagetype == DOT)
        {
            coeff = bonus->dot_damage;
            if (bonus->ap_dot_bonus > 0)
            {
                WeaponAttackType attType = (spellProto->IsRangedWeaponSpell() && spellProto->DmgClass != SPELL_DAMAGE_CLASS_MELEE) ? RANGED_ATTACK : BASE_ATTACK;
                float APbonus = float(victim->GetTotalAuraModifier(attType == BASE_ATTACK ? SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS : SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS));
                APbonus += GetTotalAttackPowerValue(attType, victim);
                DoneTotal += int32(bonus->ap_dot_bonus * stack * ApCoeffMod * APbonus);
            }
        }
        else
        {
            coeff = bonus->direct_damage;
            if (bonus->ap_bonus > 0)
            {
                WeaponAttackType attType = (spellProto->IsRangedWeaponSpell() && spellProto->DmgClass != SPELL_DAMAGE_CLASS_MELEE) ? RANGED_ATTACK : BASE_ATTACK;
                float APbonus = float(victim->GetTotalAuraModifier(attType == BASE_ATTACK ? SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS : SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS));
                APbonus += GetTotalAttackPowerValue(attType, victim);
                DoneTotal += int32(bonus->ap_bonus * stack * ApCoeffMod * APbonus);
            }
        }
    }
    else {
        coeff = CalculateDefaultCoefficient(spellProto, damagetype) * int32(stack);
    }

    // Default calculation
    if (coeff && DoneAdvertisedBenefit)
    {
        float factorMod = CalculateLevelPenalty(spellProto) * stack;

        if (Player* modOwner = GetSpellModOwner())
        {
            coeff *= 100.0f;
            modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_BONUS_MULTIPLIER, coeff);
            coeff /= 100.0f;
        }

        DoneTotal += int32(DoneAdvertisedBenefit * coeff * factorMod);
    }

    float tmpDamage = (float(pdamage) + DoneTotal) * DoneTotalMod;

    // apply spellmod to Done damage
    if(Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, damagetype == DOT ? SPELLMOD_DOT : SPELLMOD_DAMAGE, tmpDamage);

    return uint32(std::max(tmpDamage, 0.0f));
}

float Unit::SpellPctDamageModsDone(Unit* victim, SpellInfo const *spellProto, DamageEffectType damagetype)
{
    if (!spellProto || !victim || damagetype == DIRECT_DAMAGE)
        return 1.0f;

    // Some spells don't benefit from done mods
    if (spellProto->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS))
        return 1.0f;

    // For totems get damage bonus from owner
    if (GetTypeId() == TYPEID_UNIT)
    {
        if (ToCreature()->IsTotem())
        {
            if (Unit* owner = GetOwner())
                return owner->SpellPctDamageModsDone(victim, spellProto, damagetype);
        }
#ifdef LICH_KING
        // Dancing Rune Weapon...
        else if (GetEntry() == 27893)
        {
            if (Unit* owner = GetOwner())
                return owner->SpellPctDamageModsDone(victim, spellProto, damagetype);
        }
#endif
    }

    // Done total percent damage auras
    float DoneTotalMod = 1.0f;

    // ..done
    AuraList const& mModDamagePercentDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);
    for (AuraList::const_iterator i = mModDamagePercentDone.begin(); i != mModDamagePercentDone.end(); ++i)
    {
        //Some auras affect only weapons, like wand spec (6057) or 2H spec (12714)
        if ((*i)->GetSpellInfo()->Attributes & SPELL_ATTR0_AFFECT_WEAPON && (*i)->GetSpellInfo()->EquippedItemClass != -1)
            continue;

        if ((*i)->GetModifier()->m_miscvalue & spellProto->GetSchoolMask())
        {
            if ((*i)->GetSpellInfo()->EquippedItemClass == -1)
                AddPct(DoneTotalMod, (*i)->GetModifierValue());
            else if (!(*i)->GetSpellInfo()->HasAttribute(SPELL_ATTR5_SPECIAL_ITEM_CLASS_CHECK) && ((*i)->GetSpellInfo()->EquippedItemSubClassMask == 0))
                AddPct(DoneTotalMod, (*i)->GetModifierValue());
            else if (ToPlayer() && ToPlayer()->HasItemFitToSpellRequirements((*i)->GetSpellInfo()))
                AddPct(DoneTotalMod, (*i)->GetModifierValue());
        }
    }

#ifdef LICH_KING
    // bonus against aurastate
    AuraEffectList const& mDamageDoneVersusAurastate = GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE);
    for (AuraEffectList::const_iterator i = mDamageDoneVersusAurastate.begin(); i != mDamageDoneVersusAurastate.end(); ++i)
        if (victim->HasAuraState(AuraStateType((*i)->GetMiscValue())) && spellProto->ValidateAttribute6SpellDamageMods(this, *i, damagetype == DOT))
            AddPct(DoneTotalMod, (*i)->GetAmount());

#endif

    uint32 creatureTypeMask = victim->GetCreatureTypeMask();
    AuraList const& mDamageDoneVersus = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS);
    for (AuraList::const_iterator i = mDamageDoneVersus.begin(); i != mDamageDoneVersus.end(); ++i)
        if (creatureTypeMask & uint32((*i)->GetModifier()->m_miscvalue))
            DoneTotalMod *= ((*i)->GetModifierValue() + 100.0f) / 100.0f;


    // .. taken pct: scripted (increases damage of * against targets *)
    AuraList const& mOverrideClassScript = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraList::const_iterator i = mOverrideClassScript.begin(); i != mOverrideClassScript.end(); ++i)
    {
        switch ((*i)->GetModifier()->m_miscvalue)
        {
            //Molten Fury (id 31679)
        case 4920:
        case 4919:
            if (HasAuraState(AURA_STATE_HEALTHLESS_20_PERCENT))
                AddPct(DoneTotalMod, (*i)->GetAmount());
            break;
        }
    }

    // Custom scripted damage
    switch (spellProto->SpellFamilyName)
    {
    case SPELLFAMILY_MAGE:
        // Ice Lance
        if (spellProto->SpellIconID == 186)
        {
            if(victim->IsFrozen())
                DoneTotalMod *= 3.0f;
        }
    break;
    }

    return DoneTotalMod;
}

int32 Unit::SpellBaseDamageBonusDone(SpellSchoolMask schoolMask, Unit* pVictim)
{
    int32 DoneAdvertisedBenefit = 0;

    // ..done
    AuraList const& mDamageDone = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE);
    for(AuraList::const_iterator i = mDamageDone.begin();i != mDamageDone.end(); ++i)
        if(((*i)->GetModifier()->m_miscvalue & schoolMask) != 0 &&
        (*i)->GetSpellInfo()->EquippedItemClass == -1 &&
                                                            // -1 == any item class (not wand then)
        (*i)->GetSpellInfo()->EquippedItemInventoryTypeMask == 0 )
                                                            // 0 == any inventory type (not wand then)
            DoneAdvertisedBenefit += (*i)->GetModifierValue();

    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Damage bonus from stats
        AuraList const& mDamageDoneOfStatPercent = GetAurasByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT);
        for(AuraList::const_iterator i = mDamageDoneOfStatPercent.begin();i != mDamageDoneOfStatPercent.end(); ++i)
        {
            if((*i)->GetModifier()->m_miscvalue & schoolMask)
            {
                SpellInfo const* iSpellProto = (*i)->GetSpellInfo();
                uint8 eff = (*i)->GetEffIndex();

                // stat used dependent from next effect aura SPELL_AURA_MOD_SPELL_HEALING presence and misc value (stat index)
                Stats usedStat = STAT_INTELLECT;
                if(eff < 2 && iSpellProto->Effects[eff+1].ApplyAuraName==SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT)
                    usedStat = Stats(iSpellProto->Effects[eff+1].MiscValue);

                DoneAdvertisedBenefit += int32(GetStat(usedStat) * (*i)->GetModifierValue() / 100.0f);
            }
        }
        // ... and attack power
        AuraList const& mDamageDonebyAP = GetAurasByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER);
        for(AuraList::const_iterator i =mDamageDonebyAP.begin();i != mDamageDonebyAP.end(); ++i)
            if ((*i)->GetModifier()->m_miscvalue & schoolMask)
                DoneAdvertisedBenefit += int32(GetTotalAttackPowerValue(BASE_ATTACK, pVictim) * (*i)->GetModifierValue() / 100.0f);

    }
    return DoneAdvertisedBenefit;
}

int32 Unit::SpellBaseDamageBonusTaken(SpellSchoolMask schoolMask, bool isDoT)
{

    int32 TakenAdvertisedBenefit = 0;

    AuraList const& mDamageTaken = GetAurasByType(SPELL_AURA_MOD_DAMAGE_TAKEN);
    for(AuraList::const_iterator i = mDamageTaken.begin();i != mDamageTaken.end(); ++i)
        if (((*i)->GetModifier()->m_miscvalue & schoolMask) != 0)
        {
            /* SunWell core has this additional check. May be useful one day, can't investigate it right now.
            // Xinef: if we have DoT damage type and aura has charges, check if it affects DoTs
            // Xinef: required for hemorrhage & rupture / garrote
            if (isDoT && (*i)->IsUsingCharges() && !((*i)->GetSpellInfo()->ProcFlags & PROC_FLAG_TAKEN_PERIODIC))
                continue;
            */

            TakenAdvertisedBenefit += (*i)->GetModifierValue();
        }

    return TakenAdvertisedBenefit;
}

bool Unit::IsSpellCrit(Unit *pVictim, SpellInfo const *spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType)
{        
    // Mobs can't crit except for totems
    if (IS_CREATURE_GUID(GetGUID()))
    {
        uint32 owner_guid = GetOwnerGUID();
        if(IS_PLAYER_GUID(owner_guid))
        {
            Player* owner = ObjectAccessor::GetPlayer(*this, owner_guid);
            Creature* c = ToCreature();
            if(owner && c && c->IsTotem())
                return owner->IsSpellCrit(pVictim,spellProto,schoolMask,attackType);
        }
        return false;
    }

    // not critting spell
    if((spellProto->HasAttribute(SPELL_ATTR2_CANT_CRIT)))
        return false;
    
    for(int i=0;i<3;++i)
    {
        switch (spellProto->Effects[i].Effect)
        {
            // NPCs cannot crit with school damage spells
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            {
                if (!GetCharmerOrOwnerPlayerOrPlayerItself())
                    return false;
                break;
            }
            // Leech spells are not considered as direct spell damage ( they cannot crit )
            case SPELL_EFFECT_HEALTH_LEECH:
                return false;
        }
    }

    float crit_chance = 0.0f;
    switch(spellProto->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_NONE:
            return false;
        case SPELL_DAMAGE_CLASS_MAGIC:
        {
            if (schoolMask & SPELL_SCHOOL_MASK_NORMAL)
                crit_chance = 0.0f;
            // For other schools
            else if (GetTypeId() == TYPEID_PLAYER)
                crit_chance = GetFloatValue( PLAYER_SPELL_CRIT_PERCENTAGE1 + GetFirstSchoolInMask(schoolMask));
            else
            {
                crit_chance = m_baseSpellCritChance;
                crit_chance += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL, schoolMask);
            }
            // taken
            if (pVictim && !spellProto->IsPositive(!IsFriendlyTo(pVictim)))
            {
                // Modify critical chance by victim SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE
                crit_chance += pVictim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE, schoolMask);
                // Modify critical chance by victim SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE
                crit_chance += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE);
                // Modify by player victim resilience
                if (pVictim->GetTypeId() == TYPEID_PLAYER)
                    crit_chance -= (pVictim->ToPlayer())->GetRatingBonusValue(CR_CRIT_TAKEN_SPELL);
                // scripted (increase crit chance ... against ... target by x%
                if(pVictim->IsFrozen()) // Shatter
                {
                    AuraList const& mOverrideClassScript = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for(AuraList::const_iterator i = mOverrideClassScript.begin(); i != mOverrideClassScript.end(); ++i)
                    {
                        switch((*i)->GetModifier()->m_miscvalue)
                        {
                            case 849: crit_chance+= 10.0f; break; //Shatter Rank 1
                            case 910: crit_chance+= 20.0f; break; //Shatter Rank 2
                            case 911: crit_chance+= 30.0f; break; //Shatter Rank 3
                            case 912: crit_chance+= 40.0f; break; //Shatter Rank 4
                            case 913: crit_chance+= 50.0f; break; //Shatter Rank 5
                        }
                    }
                }
                // arcane potency
                if (HasAuraEffect(12536,0) || HasAuraEffect(12043,0)) { // clearcasting or presence of mind
                    if (HasSpell(31571)) crit_chance+= 10.0f;
                    if (HasSpell(31572)) crit_chance+= 20.0f;
                    if (HasSpell(31573)) crit_chance+= 30.0f;
                }
            }
            break;
        }
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
        {
            if (pVictim)
            {
                crit_chance = GetUnitCriticalChance(attackType, pVictim);
                crit_chance+= (int32(GetMaxSkillValueForLevel(pVictim)) - int32(pVictim->GetDefenseSkillValue(this))) * 0.04f;
                crit_chance+= GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL, schoolMask);
                // always crit against a sitting target (except 0 crit chance)
                if(crit_chance > 0 && !pVictim->IsStandState())
                {
                   return true;
                }
            }
            break;
        }
        default:
            return false;
    }
    // percent done
    // only players use intelligence for critical chance computations
    if(Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_CRITICAL_CHANCE, crit_chance);

    crit_chance = crit_chance > 0.0f ? crit_chance : 0.0f;
    bool success = roll_chance_f(crit_chance);

    return success;
}

uint32 Unit::SpellCriticalBonus(SpellInfo const *spellProto, uint32 damage, Unit *pVictim)
{
    // Calculate critical bonus
    int32 crit_bonus;
    switch(spellProto->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:                      // for melee based spells is 100%
        case SPELL_DAMAGE_CLASS_RANGED:
            // TODO: write here full calculation for melee/ranged spells
            crit_bonus = damage;
            break;
        default:
            crit_bonus = damage / 2;                        // for spells is 50%
            break;
    }
    
    int mod_critDamageBonus = GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_DAMAGE_BONUS_MELEE);
    crit_bonus += int32(crit_bonus * mod_critDamageBonus) / 100.0f;

    // adds additional damage to crit_bonus (from talents)
    if(Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_CRIT_DAMAGE_BONUS, crit_bonus);

    if(pVictim)
    {
        uint32 creatureTypeMask = pVictim->GetCreatureTypeMask();
        crit_bonus = int32(crit_bonus * GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_CRIT_PERCENT_VERSUS, creatureTypeMask));
    }

    if(crit_bonus > 0)
        damage += crit_bonus;

    return damage;
}

float Unit::SpellPctHealingModsDone(Unit* victim, SpellInfo const *spellProto, DamageEffectType damagetype)
{
    // For totems get healing bonus from owner (statue isn't totem in fact)
    if (GetTypeId() == TYPEID_UNIT && ToCreature()->IsTotem())
        if (Unit* owner = GetOwner())
            return owner->SpellPctHealingModsDone(victim, spellProto, damagetype);

    // Some spells don't benefit from done mods
    if (spellProto->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS))
        return 1.0f;

#ifdef LICH_KING
    // xinef: Some spells don't benefit from done mods
    if (spellProto->HasAttribute(SPELL_ATTR6_LIMIT_PCT_HEALING_MODS))
        return 1.0f;
#endif

    // No bonus healing for potion spells
    if (spellProto->SpellFamilyName == SPELLFAMILY_POTION)
        return 1.0f;

    float DoneTotalMod = 1.0f;

    // Healing done percent
    AuraList const& mHealingDonePct = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE_PERCENT);
    for (AuraList::const_iterator i = mHealingDonePct.begin(); i != mHealingDonePct.end(); ++i)
        AddPct(DoneTotalMod, (*i)->GetAmount());

    // done scripted mod (take it from owner)
    Unit* owner = GetOwner() ? GetOwner() : this;
    AuraList const& mOverrideClassScript = owner->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraList::const_iterator i = mOverrideClassScript.begin(); i != mOverrideClassScript.end(); ++i)
    {
        if (!sSpellMgr->IsAffectedBySpell(spellProto, (*i)->GetId(), (*i)->GetEffIndex(), 0))
        continue;
        /*
        if (!(*i)->IsAffectedOnSpell(spellProto))
            continue;
            */
        switch ((*i)->GetMiscValue())
        {
        case   21: // Test of Faith
        case 6935:
        case 6918:
            if (victim->HealthBelowPct(50))
                AddPct(DoneTotalMod, (*i)->GetAmount());
            break;
#ifdef LICH_KING
        case 7798: // Glyph of Regrowth
        {
            if (victim->GetAuraEffect(SPELL_AURA_PERIODIC_HEAL, SPELLFAMILY_DRUID, 0x40, 0, 0))
                AddPct(DoneTotalMod, (*i)->GetAmount());
            break;
        }

        case 7871: // Glyph of Lesser Healing Wave
        {
            // xinef: affected by any earth shield
            if (victim->GetAuraEffect(SPELL_AURA_DUMMY, SPELLFAMILY_SHAMAN, 0, 0x00000400, 0))
                AddPct(DoneTotalMod, (*i)->GetAmount());
            break;
        }
#endif
        default:
            break;
        }
    }

    switch (spellProto->SpellFamilyName)
    {
    case SPELLFAMILY_GENERIC:
#ifdef LICH_KING

        // Talents and glyphs for healing stream totem
        if (spellProto->Id == 52042)
        {
            // Glyph of Healing Stream Totem
            if (AuraEffect *dummy = owner->GetAuraEffect(55456, EFFECT_0))
                AddPct(DoneTotalMod, dummy->GetAmount());

            // Healing Stream totem - Restorative Totems
            if (AuraEffect *aurEff = GetDummyAuraEffect(SPELLFAMILY_SHAMAN, 338, 1))
                AddPct(DoneTotalMod, aurEff->GetAmount());
        }
#endif
        break;
    case SPELLFAMILY_PRIEST:
#ifdef LICH_KING
        // T9 HEALING 4P, empowered renew instant heal
        if (spellProto->Id == 63544)
            if (AuraEffect *aurEff = GetAuraEffect(67202, EFFECT_0))
                AddPct(DoneTotalMod, aurEff->GetAmount());
#endif
        break;
    }

    return DoneTotalMod;
}

uint32 Unit::SpellHealingBonusTaken(Unit* caster, SpellInfo const *spellProto, uint32 healamount, DamageEffectType damagetype, uint32 stack /* = 1 */)
{
    float TakenTotalMod = 1.0f;

    // Healing taken percent
    float minval = float(GetMaxNegativeAuraModifier(SPELL_AURA_MOD_HEALING_PCT));
    if (minval)
        AddPct(TakenTotalMod, minval);

    float maxval = float(GetMaxPositiveAuraModifier(SPELL_AURA_MOD_HEALING_PCT));
    if (maxval)
        AddPct(TakenTotalMod, maxval);

    // These Spells are doing fixed amount of healing (TODO found less hack-like check)
    // Can we removed some of these because of the "No bonus healing for SPELL_DAMAGE_CLASS_NONE class spells by default" below check ?
    if (spellProto->Id == 15290 || spellProto->Id == 39373 ||
        spellProto->Id == 33778 || spellProto->Id == 379 ||
        spellProto->Id == 38395 || spellProto->Id == 40972 ||
        spellProto->Id == 22845 || spellProto->Id == 33504 ||
        spellProto->Id == 34299 || spellProto->Id == 27813 ||
        spellProto->Id == 27817 || spellProto->Id == 27818 ||
        spellProto->Id == 30294 || spellProto->Id == 18790 ||
        spellProto->Id == 5707 ||
        spellProto->Id == 31616 || spellProto->Id == 37382 ||
        spellProto->Id == 38325)
    {
        float heal = float(int32(healamount)) * TakenTotalMod;
        return uint32(std::max(heal, 0.0f));
        return uint32(heal);
    }

#ifdef LICH_KING
    // Tenacity increase healing % taken
    if (AuraEffect const* Tenacity = GetAuraEffect(58549, 0))
        AddPct(TakenTotalMod, Tenacity->GetAmount());
#endif

    // Healing Done
    int32 TakenTotal = 0;

    // Taken fixed damage bonus auras
    int32 TakenAdvertisedBenefit = SpellBaseHealingBonusTaken(spellProto->GetSchoolMask());

    //MIGHTY HACKS BLOCK
    {
        // Blessing of Light dummy effects healing taken from Holy Light and Flash of Light
        if (spellProto->SpellFamilyName == SPELLFAMILY_PALADIN && (spellProto->SpellFamilyFlags & 0x00000000C0000000LL))
        {
            AuraList const& mDummyAuras = GetAurasByType(SPELL_AURA_DUMMY);
            for (AuraList::const_iterator i = mDummyAuras.begin(); i != mDummyAuras.end(); ++i)
            {
                if ((*i)->GetSpellInfo()->HasVisual(9180))
                {
                    // Flash of Light
                    if ((spellProto->SpellFamilyFlags & 0x0000000040000000LL) && (*i)->GetEffIndex() == 1)
                        TakenAdvertisedBenefit += (*i)->GetModifier()->m_amount;
                    // Holy Light
                    else if ((spellProto->SpellFamilyFlags & 0x0000000080000000LL) && (*i)->GetEffIndex() == 0)
                        TakenAdvertisedBenefit += (*i)->GetModifier()->m_amount;
                }
                // Libram of the Lightbringer
                else if ((*i)->GetSpellInfo()->Id == 34231)
                {
                    // Holy Light
                    if ((spellProto->SpellFamilyFlags & 0x0000000080000000LL))
                        TakenAdvertisedBenefit += (*i)->GetModifier()->m_amount;
                }
                // Blessed Book of Nagrand || Libram of Light || Libram of Divinity
                else if ((*i)->GetSpellInfo()->Id == 32403 || (*i)->GetSpellInfo()->Id == 28851 || (*i)->GetSpellInfo()->Id == 28853)
                {
                    // Flash of Light
                    if ((spellProto->SpellFamilyFlags & 0x0000000040000000LL))
                        TakenAdvertisedBenefit += (*i)->GetModifier()->m_amount;
                }
            }
        }

        // Healing Wave cast (these are dummy auras)
        if (spellProto->SpellFamilyName == SPELLFAMILY_SHAMAN && spellProto->SpellFamilyFlags & 0x0000000000000040LL)
        {
            // Search for Healing Way on Victim (stack up to 3 time)
            Unit::AuraList const& auraDummy = GetAurasByType(SPELL_AURA_DUMMY);
            for (Unit::AuraList::const_iterator itr = auraDummy.begin(); itr != auraDummy.end(); ++itr)
            {
                if ((*itr)->GetId() == 29203)
                {
                    uint32 percentIncrease = (*itr)->GetModifier()->m_amount * (*itr)->GetStackAmount();
                    AddPct(TakenTotalMod, percentIncrease);
                    break;
                }
            }
        }
    }

#ifdef LICH_KING
    // Nourish cast, glyph of nourish
    if (spellProto->SpellFamilyName == SPELLFAMILY_DRUID && spellProto->SpellFamilyFlags[1] & 0x2000000 && caster)
    {
        bool any = false;
        bool hasglyph = caster->GetAuraEffectDummy(62971);
        AuraList const& auras = GetAurasByType(SPELL_AURA_PERIODIC_HEAL);
        for (AuraList::const_iterator i = auras.begin(); i != auras.end(); ++i)
        {
            if (((*i)->GetCasterGUID() == caster->GetGUID()))
            {
                SpellInfo const *spell = (*i)->GetSpellInfo();
                // Rejuvenation, Regrowth, Lifebloom, or Wild Growth
                if (!any && spell->SpellFamilyFlags.HasFlag(0x50, 0x4000010, 0))
                {
                    TakenTotalMod *= 1.2f;
                    any = true;
                }

                if (hasglyph)
                    TakenTotalMod += 0.06f;
            }
        }
    }

    if (damagetype == DOT)
    {
        // Healing over time taken percent
        float minval_hot = float(GetMaxNegativeAuraModifier(SPELL_AURA_MOD_HOT_PCT));
        if (minval_hot)
            AddPct(TakenTotalMod, minval_hot);

        float maxval_hot = float(GetMaxPositiveAuraModifier(SPELL_AURA_MOD_HOT_PCT));
        if (maxval_hot)
            AddPct(TakenTotalMod, maxval_hot);
    }
#endif

    SpellBonusEntry const* bonus = sSpellMgr->GetSpellBonusData(spellProto->Id);
    float coeff = 0;
    float factorMod = 1.0f;
    if (bonus)
        coeff = (damagetype == DOT) ? bonus->dot_damage : bonus->direct_damage;

    // No bonus healing for SPELL_DAMAGE_CLASS_NONE class spells by default
    if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE)
    {
        healamount = uint32(std::max((float(healamount) * TakenTotalMod), 0.0f));
        return healamount;
    }

    // Default calculation
    if (TakenAdvertisedBenefit)
    {
        float TakenCoeff = 0.0f;
        if (coeff <= 0)
            coeff = CalculateDefaultCoefficient(spellProto, damagetype) * int32(stack);  // As wowwiki says: C = (Cast Time / 3.5) * 1.88 (for healing spells)

        factorMod *= CalculateLevelPenalty(spellProto) * int32(stack);
        if (Player* modOwner = GetSpellModOwner())
        {
            coeff *= 100.0f;
            modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_BONUS_MULTIPLIER, coeff);
            coeff /= 100.0f;
        }

        TakenTotal += int32(TakenAdvertisedBenefit * (coeff > 0 ? coeff : TakenCoeff) * factorMod);
    }

#ifdef LICH_KING
    if (caster)
    {
        AuraList const& mHealingGet = GetAurasByType(SPELL_AURA_MOD_HEALING_RECEIVED);
        for (AuraList::const_iterator i = mHealingGet.begin(); i != mHealingGet.end(); ++i)
            if (caster->GetGUID() == (*i)->GetCasterGUID() && (*i)->IsAffectedOnSpell(spellProto))
                AddPct(TakenTotalMod, (*i)->GetAmount());
    }
#endif

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        switch (spellProto->Effects[i].ApplyAuraName)
        {
            // Bonus healing does not apply to these spells
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
            TakenTotal = 0;
            break;
        }
        if (spellProto->Effects[i].Effect == SPELL_EFFECT_HEALTH_LEECH)
            TakenTotal = 0;
    }

#ifdef LICH_KING
    // No positive taken bonus, custom attr
    if ((spellProto->HasAttribute(SPELL_ATTR6_LIMIT_PCT_HEALING_MODS) || spellProto->HasAttribute(SPELL_ATTR0_CU_NO_POSITIVE_TAKEN_BONUS)) && TakenTotalMod > 1.0f)
    {
        TakenTotal = 0;
        TakenTotalMod = 1.0f;
    }
#endif

    float heal = float(int32(healamount) + TakenTotal) * TakenTotalMod;

    return uint32(std::max(heal, 0.0f));
}

uint32 Unit::SpellHealingBonusDone(Unit* victim, SpellInfo const *spellProto, uint32 healamount, DamageEffectType damagetype, float TotalMod /* = 0.0f */, uint32 stack /* = 1 */)
{
    if (spellProto && spellProto->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS))
        return healamount;

    // For totems get healing bonus from owner (statue isn't totem in fact)
    if (GetTypeId() == TYPEID_UNIT && ToCreature()->IsTotem())
        if (Unit* owner = GetOwner())
            return owner->SpellHealingBonusDone(victim, spellProto, healamount, damagetype, TotalMod, stack);

    // No bonus healing for potion spells
    if (spellProto->SpellFamilyName == SPELLFAMILY_POTION)
        return healamount;

    float ApCoeffMod = 1.0f;
    float DoneTotalMod = TotalMod ? TotalMod : SpellPctHealingModsDone(victim, spellProto, damagetype);
    int32 DoneTotal = 0;

    // done scripted mod (take it from owner)
    Unit* owner = GetOwner() ? GetOwner() : this;
    AuraList const& mOverrideClassScript = owner->GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraList::const_iterator i = mOverrideClassScript.begin(); i != mOverrideClassScript.end(); ++i)
    {
        if (!sSpellMgr->IsAffectedBySpell(spellProto, (*i)->GetId(), (*i)->GetEffIndex(), 0))
            continue;
        /*if (!(*i)->IsAffectedOnSpell(spellProto))
            continue;*/
        switch ((*i)->GetMiscValue())
        {
        case 4415: // Increased Rejuvenation Healing
        case 4953:
        case 3736: // Hateful Totem of the Third Wind / Increased Lesser Healing Wave / LK Arena (4/5/6) Totem of the Third Wind / Savage Totem of the Third Wind
            DoneTotal += (*i)->GetAmount();
            break;
        }
    }

    // Done fixed damage bonus auras
    int32 DoneAdvertisedBenefit = SpellBaseHealingBonusDone(spellProto->GetSchoolMask());
    float coeff = 0.0f;

#ifdef LICH_KING
    switch (spellProto->SpellFamilyName)
    {
    case SPELLFAMILY_DEATHKNIGHT:
        // Impurity
        if (AuraEffect *aurEff = GetDummyAuraEffect(SPELLFAMILY_DEATHKNIGHT, 1986, 0))
            AddPct(ApCoeffMod, aurEff->GetAmount());

        break;
    }

    // No bonus healing for SPELL_DAMAGE_CLASS_NONE class spells by default
    if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE)
        return healamount;

#endif

    // Check for table values
    SpellBonusEntry const* bonus = sSpellMgr->GetSpellBonusData(spellProto->Id);
    if (bonus)
    {
        if (damagetype == DOT)
        {
            coeff = bonus->dot_damage;
            if (bonus->ap_dot_bonus > 0)
                DoneTotal += int32(bonus->ap_dot_bonus * ApCoeffMod * stack * GetTotalAttackPowerValue(
                        (spellProto->IsRangedWeaponSpell() && spellProto->DmgClass != SPELL_DAMAGE_CLASS_MELEE) ? RANGED_ATTACK : BASE_ATTACK, victim)
                    );
        }
        else
        {
            coeff = bonus->direct_damage;
            if (bonus->ap_bonus > 0)
                DoneTotal += int32(bonus->ap_bonus * ApCoeffMod * stack * GetTotalAttackPowerValue(
                    (spellProto->IsRangedWeaponSpell() && spellProto->DmgClass != SPELL_DAMAGE_CLASS_MELEE) ? RANGED_ATTACK : BASE_ATTACK, victim));
        }
    }
    else
    {
        // No bonus healing for SPELL_DAMAGE_CLASS_NONE class spells by default
        if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE)
            return healamount;

        //default coef
        coeff = CalculateDefaultCoefficient(spellProto, damagetype) * int32(stack);  // As wowwiki says: C = (Cast Time / 3.5)
    }

    // Default calculation
    if (coeff && DoneAdvertisedBenefit)
    {
        float factorMod = CalculateLevelPenalty(spellProto) * stack;
        if (Player* modOwner = GetSpellModOwner())
        {
            coeff *= 100.0f;
            modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_BONUS_MULTIPLIER, coeff);
            coeff /= 100.0f;
        }
        DoneTotal += int32(DoneAdvertisedBenefit * coeff * factorMod);
    }

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        switch (spellProto->Effects[i].ApplyAuraName)
        {
            // Bonus healing does not apply to these spells
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
            DoneTotal = 0;
            break;
        }
        if (spellProto->Effects[i].Effect == SPELL_EFFECT_HEALTH_LEECH)
            DoneTotal = 0;
    }

    // use float as more appropriate for negative values and percent applying
    float heal = float(int32(healamount) + DoneTotal) * DoneTotalMod;
    // apply spellmod to Done amount

    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, damagetype == DOT ? SPELLMOD_DOT : SPELLMOD_DAMAGE, heal);

    return uint32(std::max(heal, 0.0f));
}

int32 Unit::SpellBaseHealingBonusDone(SpellSchoolMask schoolMask)
{
    int32 AdvertisedBenefit = 0;

    AuraList const& mHealingDone = GetAurasByType(SPELL_AURA_MOD_HEALING_DONE);
    for(AuraList::const_iterator i = mHealingDone.begin();i != mHealingDone.end(); ++i)
        if(((*i)->GetModifier()->m_miscvalue & schoolMask) != 0)
            AdvertisedBenefit += (*i)->GetModifierValue();

    // Healing bonus of spirit, intellect and strength
    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Healing bonus from stats
        AuraList const& mHealingDoneOfStatPercent = GetAurasByType(SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT);
        for(AuraList::const_iterator i = mHealingDoneOfStatPercent.begin();i != mHealingDoneOfStatPercent.end(); ++i)
        {
            // stat used dependent from misc value (stat index)
            Stats usedStat = Stats((*i)->GetSpellInfo()->Effects[(*i)->GetEffIndex()].MiscValue);
            AdvertisedBenefit += int32(GetStat(usedStat) * (*i)->GetModifierValue() / 100.0f);
        }

        // ... and attack power
        AuraList const& mHealingDonebyAP = GetAurasByType(SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER);
        for(AuraList::const_iterator i = mHealingDonebyAP.begin();i != mHealingDonebyAP.end(); ++i)
            if ((*i)->GetModifier()->m_miscvalue & schoolMask)
                AdvertisedBenefit += int32(GetTotalAttackPowerValue(BASE_ATTACK) * (*i)->GetModifierValue() / 100.0f);
    }
    return AdvertisedBenefit;
}

int32 Unit::SpellBaseHealingBonusTaken(SpellSchoolMask schoolMask)
{
    int32 AdvertisedBenefit = 0;
    AuraList const& mDamageTaken = GetAurasByType(SPELL_AURA_MOD_HEALING);
    for(AuraList::const_iterator i = mDamageTaken.begin();i != mDamageTaken.end(); ++i)
    {
        if(((*i)->GetModifier()->m_miscvalue & schoolMask) != 0)
            AdvertisedBenefit += (*i)->GetModifierValue();

        //HACK
        if((*i)->GetId() == 34123) //tree of life "Increases healing received by 25% of the Tree of Life's total spirit." -> This means, add 25% drood spirit as healing bonus to healing spell taken
        {
            if((*i)->GetCaster() && (*i)->GetCaster()->GetTypeId() == TYPEID_PLAYER)
                AdvertisedBenefit += int32(0.25f * ((*i)->GetCaster()->ToPlayer())->GetStat(STAT_SPIRIT));
        }
    }
    return AdvertisedBenefit;
}

bool Unit::IsImmunedToDamage(SpellSchoolMask shoolMask, bool useCharges)
{
    //If m_immuneToSchool type contain this school type, IMMUNE damage.
    SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
    for (SpellImmuneList::const_iterator itr = schoolList.begin(); itr != schoolList.end(); ++itr)
        if(itr->type & shoolMask)
            return true;

    //If m_immuneToDamage type contain magic, IMMUNE damage.
    SpellImmuneList const& damageList = m_spellImmune[IMMUNITY_DAMAGE];
    for (SpellImmuneList::const_iterator itr = damageList.begin(); itr != damageList.end(); ++itr)
        if(itr->type & shoolMask)
            return true;

    return false;
}

bool Unit::IsImmunedToSpell(SpellInfo const* spellInfo, bool useCharges)
{
    if (!spellInfo)
        return false;

    // Hack for blue dragon
    switch (spellInfo->Id)
    {
        case 45848:
        case 45838:
            return false;
    }

    //Single spells immunity
    SpellImmuneList const& idList = m_spellImmune[IMMUNITY_ID];
    for(SpellImmuneList::const_iterator itr = idList.begin(); itr != idList.end(); ++itr)
    {
        if(itr->type == spellInfo->Id)
        {
            return true;
        }
    }

    if(spellInfo->Attributes & SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY)
        return false;

    SpellImmuneList const& dispelList = m_spellImmune[IMMUNITY_DISPEL];
    for(SpellImmuneList::const_iterator itr = dispelList.begin(); itr != dispelList.end(); ++itr)
        if(itr->type == spellInfo->Dispel)
            return true;

    if( !(spellInfo->HasAttribute(SPELL_ATTR1_UNAFFECTED_BY_SCHOOL_IMMUNE)) &&         // unaffected by school immunity
        !(spellInfo->HasAttribute(SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY))               // can remove immune (by dispell or immune it)
        && (spellInfo->Id != 42292))
    {
        SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
        for(SpellImmuneList::const_iterator itr = schoolList.begin(); itr != schoolList.end(); ++itr)
        {
            SpellInfo const* spellImmuneInfo = sSpellMgr->GetSpellInfo(itr->spellId);
            if(!spellImmuneInfo)
                continue;
            if( !(spellImmuneInfo->IsPositive() && spellInfo->IsPositive()) && //at least one of the given spell and the immune spell is negative
                (itr->type & spellInfo->GetSchoolMask()) )
                return true;
        }
    }

    SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
    for(SpellImmuneList::const_iterator itr = mechanicList.begin(); itr != mechanicList.end(); ++itr)
    {
        if(itr->type == spellInfo->Mechanic)
        {
            return true;
        }
    }

    if(ToCreature() && ToCreature()->IsTotem())
        if(spellInfo->IsChanneled())
            return true;

    return false;
}

bool Unit::IsImmunedToSpellEffect(uint32 effect, uint32 mechanic) const
{
    //If m_immuneToEffect type contain this effect type, IMMUNE effect.
    SpellImmuneList const& effectList = m_spellImmune[IMMUNITY_EFFECT];
    for (SpellImmuneList::const_iterator itr = effectList.begin(); itr != effectList.end(); ++itr)
        if(itr->type == effect)
            return true;

    SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
    for (SpellImmuneList::const_iterator itr = mechanicList.begin(); itr != mechanicList.end(); ++itr)
        if(itr->type == mechanic)
            return true;

    return false;
}

bool Unit::IsDamageToThreatSpell(SpellInfo const * spellInfo) const
{
    if(!spellInfo)
        return false;

    uint32 family = spellInfo->SpellFamilyName;
    uint64 flags = spellInfo->SpellFamilyFlags;

    if((family == 5 && flags == 256) ||                     //Searing Pain
        (family == SPELLFAMILY_SHAMAN && flags == SPELLFAMILYFLAG_SHAMAN_FROST_SHOCK))
        return true;

    return false;
}

/*
uint32 Unit::MeleeDamageBonusTaken(Unit* attacker, uint32 pdamage, WeaponAttackType attType, SpellInfo const *spellProto = NULL)
{

}
*/

void Unit::MeleeDamageBonus(Unit *pVictim, uint32 *pdamage,WeaponAttackType attType, SpellInfo const *spellProto)
{
    if(!pVictim)
        return;

    if(*pdamage == 0)
        return;

    uint32 creatureTypeMask = pVictim->GetCreatureTypeMask();

    // Taken/Done fixed damage bonus auras
    int32 DoneFlatBenefit = 0;
    int32 TakenFlatBenefit = 0;

    // ..done (for creature type by mask) in taken
    AuraList const& mDamageDoneCreature = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE);
    for(AuraList::const_iterator i = mDamageDoneCreature.begin();i != mDamageDoneCreature.end(); ++i)
        if(creatureTypeMask & uint32((*i)->GetModifier()->m_miscvalue))
            DoneFlatBenefit += (*i)->GetModifierValue();
    // ..done
    // SPELL_AURA_MOD_DAMAGE_DONE included in weapon damage

    // ..done (base at attack power for marked target and base at attack power for creature type)
    float APBonus = GetAPBonusVersus(attType, pVictim);
    if (APBonus != 0.0f)                                         // Can be negative
    {
        bool normalized = false;
        if(spellProto)
        {
            for (uint8 i = 0; i<3;i++)
            {
                if (spellProto->Effects[i].Effect == SPELL_EFFECT_NORMALIZED_WEAPON_DMG)
                {
                    normalized = true;
                    break;
                }
            }
        }

        DoneFlatBenefit += int32((APBonus/14.0f) * GetAPMultiplier(attType,normalized));
    }

    // ..taken
    AuraList const& mDamageTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_TAKEN);
    for(AuraList::const_iterator i = mDamageTaken.begin();i != mDamageTaken.end(); ++i)
        if((*i)->GetModifier()->m_miscvalue & GetMeleeDamageSchoolMask())
            TakenFlatBenefit += (*i)->GetModifierValue();

    if(attType!=RANGED_ATTACK)
        TakenFlatBenefit += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN);
    else
        TakenFlatBenefit += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN);

    // Done/Taken total percent damage auras
    float DoneTotalMod = 1;
    float TakenTotalMod = 1;

    // ..done
    // SPELL_AURA_MOD_DAMAGE_PERCENT_DONE included in weapon damage
    // SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT  included in weapon damage

    AuraList const& mDamageDoneVersus = GetAurasByType(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS);
    for(AuraList::const_iterator i = mDamageDoneVersus.begin();i != mDamageDoneVersus.end(); ++i)
        if(creatureTypeMask & uint32((*i)->GetModifier()->m_miscvalue))
            DoneTotalMod *= ((*i)->GetModifierValue()+100.0f)/100.0f;
    // ..taken
    AuraList const& mModDamagePercentTaken = pVictim->GetAurasByType(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN);
    for(AuraList::const_iterator i = mModDamagePercentTaken.begin(); i != mModDamagePercentTaken.end(); ++i) {
        if((*i)->GetModifier()->m_miscvalue & GetMeleeDamageSchoolMask()) 
            TakenTotalMod *= ((*i)->GetModifierValue()+100.0f)/100.0f;
    }
        
    // .. taken pct: dummy auras
    AuraList const& mDummyAuras = pVictim->GetAurasByType(SPELL_AURA_DUMMY);
    for(AuraList::const_iterator i = mDummyAuras.begin(); i != mDummyAuras.end(); ++i)
    {
        switch((*i)->GetSpellInfo()->SpellIconID)
        {
            //Cheat Death
            case 2109:
                if((*i)->GetModifier()->m_miscvalue & SPELL_SCHOOL_MASK_NORMAL)
                {
                    if(pVictim->GetTypeId() != TYPEID_PLAYER)
                        continue;
                    float mod = (pVictim->ToPlayer())->GetRatingBonusValue(CR_CRIT_TAKEN_MELEE)*(-8.0f);
                    if (mod < (*i)->GetModifier()->m_amount)
                        mod = (*i)->GetModifier()->m_amount;
                    TakenTotalMod *= (mod+100.0f)/100.0f;
                }
                break;
            //Mangle
            case 2312:
                if(spellProto==NULL)
                    break;
                // Should increase Shred (initial Damage of Lacerate and Rake handled in Spell::EffectSchoolDMG)
                if(spellProto->SpellFamilyName==SPELLFAMILY_DRUID && (spellProto->SpellFamilyFlags==0x00008000LL))
                    TakenTotalMod *= (100.0f+(*i)->GetModifier()->m_amount)/100.0f;
                break;
        }
    }

    // .. taken pct: class scripts
    AuraList const& mclassScritAuras = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for(AuraList::const_iterator i = mclassScritAuras.begin(); i != mclassScritAuras.end(); ++i)
    {
        switch((*i)->GetMiscValue())
        {
            case 6427: case 6428:                           // Dirty Deeds
                if(pVictim->HasAuraState(AURA_STATE_HEALTHLESS_35_PERCENT))
                {
                    Aura* eff0 = GetAura((*i)->GetId(),0);
                    if(!eff0 || (*i)->GetEffIndex()!=1)
                    {
                        TC_LOG_ERROR("FIXME","Spell structure of DD (%u) changed.",(*i)->GetId());
                        continue;
                    }

                    // effect 0 have expected value but in negative state
                    TakenTotalMod *= (-eff0->GetModifier()->m_amount+100.0f)/100.0f;
                }
                break;
        }
    }

    if(attType != RANGED_ATTACK)
    {
        AuraList const& mModMeleeDamageTakenPercent = pVictim->GetAurasByType(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT);
        for(AuraList::const_iterator i = mModMeleeDamageTakenPercent.begin(); i != mModMeleeDamageTakenPercent.end(); ++i)
            TakenTotalMod *= ((*i)->GetModifierValue()+100.0f)/100.0f;
    }
    else
    {
        AuraList const& mModRangedDamageTakenPercent = pVictim->GetAurasByType(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT);
        for(AuraList::const_iterator i = mModRangedDamageTakenPercent.begin(); i != mModRangedDamageTakenPercent.end(); ++i)
            TakenTotalMod *= ((*i)->GetModifierValue()+100.0f)/100.0f;
    }

    float tmpDamage = float(int32(*pdamage) + DoneFlatBenefit) * DoneTotalMod;

    // apply spellmod to Done damage
    if(spellProto)
    {
        if(Player* modOwner = GetSpellModOwner())
            modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_DAMAGE, tmpDamage);
    }

    tmpDamage = (tmpDamage + TakenFlatBenefit)*TakenTotalMod;

    // bonus result can be negative
    *pdamage =  tmpDamage > 0 ? uint32(tmpDamage) : 0;
}

void Unit::ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply)
{
    if (apply)
    {
        for (SpellImmuneList::iterator itr = m_spellImmune[op].begin(), next; itr != m_spellImmune[op].end(); itr = next)
        {
            next = itr; ++next;
            if(itr->type == type)
            {
                m_spellImmune[op].erase(itr);
                next = m_spellImmune[op].begin();
            }
        }
        SpellImmune Immune;
        Immune.spellId = spellId;
        Immune.type = type;
        m_spellImmune[op].push_back(Immune);
    }
    else
    {
        for (SpellImmuneList::iterator itr = m_spellImmune[op].begin(); itr != m_spellImmune[op].end(); ++itr)
        {
            if(itr->spellId == spellId)
            {
                m_spellImmune[op].erase(itr);
                break;
            }
        }
    }

}

void Unit::ApplySpellDispelImmunity(const SpellInfo * spellProto, DispelType type, bool apply)
{
    ApplySpellImmune(spellProto->Id,IMMUNITY_DISPEL, type, apply);

    if (apply && spellProto->HasAttribute(SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY))
        RemoveAurasWithDispelType(type);
}

float Unit::GetWeaponProcChance() const
{
    // normalized proc chance for weapon attack speed
    // (odd formula...)
    if(IsAttackReady(BASE_ATTACK))
        return (GetAttackTime(BASE_ATTACK) * 1.8f / 1000.0f);
    else if (HaveOffhandWeapon() && IsAttackReady(OFF_ATTACK))
        return (GetAttackTime(OFF_ATTACK) * 1.6f / 1000.0f);
    return 0;
}

float Unit::GetPPMProcChance(uint32 WeaponSpeed, float PPM) const
{
    // proc per minute chance calculation
    if (PPM <= 0) return 0.0f;
    uint32 result = uint32((WeaponSpeed * PPM) / 600.0f);   // result is chance in percents (probability = Speed_in_sec * (PPM / 60))
    return result;
}

void Unit::Mount(uint32 mount, bool flying)
{
    if(!mount)
        return;

    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_MOUNT);

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, mount);

    SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT );

    // unsummon pet
    if(GetTypeId() == TYPEID_PLAYER)
    {
        Pet* pet = GetPet();
        if(pet)
        {
            /*Battleground *bg = (this->ToPlayer())->GetBattleground();
            // don't unsummon pet in arena but SetFlag UNIT_FLAG_STUNNED to disable pet's interface*/
            if (!flying)
                pet->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
            else
            {
                if(pet->isControlled())
                {
                    (this->ToPlayer())->SetTemporaryUnsummonedPetNumber(pet->GetCharmInfo()->GetPetNumber());
                    (this->ToPlayer())->SetOldPetSpell(pet->GetUInt32Value(UNIT_CREATED_BY_SPELL));
                }
                (this->ToPlayer())->RemovePet(NULL, PET_SAVE_NOT_IN_SLOT);
                return;
            }
        }
        (this->ToPlayer())->SetTemporaryUnsummonedPetNumber(0);
    }
}

void Unit::Dismount()
{
    if(!IsMounted())
        return;

    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_MOUNTED);

    SetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);
    RemoveFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT );

    // only resummon old pet if the player is already added to a map
    // this prevents adding a pet to a not created map which would otherwise cause a crash
    // (it could probably happen when logging in after a previous crash)
    if(GetTypeId() == TYPEID_PLAYER && IsInWorld() && IsAlive())
    {
        if(Pet *pPet = GetPet())
        {
            if(pPet->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED) && !pPet->HasUnitState(UNIT_STATE_STUNNED))
                pPet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        } else {
            ToPlayer()->ResummonPetTemporaryUnSummonedIfAny();
        }
    }
}

void Unit::SetInCombatWith(Unit* enemy)
{
    Unit* eOwner = enemy->GetCharmerOrOwnerOrSelf();
    if(eOwner->IsPvP())
    {
        SetInCombatState(true, enemy);
        return;
    }

    //check for duel
    if(eOwner->GetTypeId() == TYPEID_PLAYER && (eOwner->ToPlayer())->duel)
    {
        Unit const* myOwner = GetCharmerOrOwnerOrSelf();
        if(((Player const*)eOwner)->duel->opponent == myOwner)
        {
            SetInCombatState(true,enemy);
            return;
        }
    }
    SetInCombatState(false, enemy);
}

bool Unit::IsInCombatWith(Unit const* enemy) const
{
    return HasInThreatList(enemy->GetGUID());
}

void Unit::CombatStart(Unit* target, bool updatePvP)
{
    if(HasUnitState(UNIT_STATE_EVADE) || target->HasUnitState(UNIT_STATE_EVADE))
        return;

    if(!target->IsStandState()/* && !target->HasUnitState(UNIT_STATE_STUNNED)*/)
        target->SetStandState(PLAYER_STATE_NONE);

    if(!target->IsInCombat() && target->GetTypeId() != TYPEID_PLAYER
        && !(target->ToCreature())->HasReactState(REACT_PASSIVE) && (target->ToCreature())->IsAIEnabled)
    {
        (target->ToCreature())->AI()->AttackStart(this);
        
        if (InstanceScript* instance = ((InstanceScript*)target->GetInstanceScript()))
            instance->MonsterPulled(target->ToCreature(), this);
    }
    
    if (IsAIEnabled)
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);

    SetInCombatWith(target);
    target->SetInCombatWith(this);
  
    Unit *who = target->GetCharmerOrOwnerOrSelf();
    if(who->GetTypeId() == TYPEID_PLAYER)
        SetContestedPvP(who->ToPlayer());

    Player* me = GetCharmerOrOwnerPlayerOrPlayerItself();
    if(updatePvP 
        && me && who->IsPvP()
        && (who->GetTypeId() != TYPEID_PLAYER
        || !me->duel || me->duel->opponent != who))
    {
        me->UpdatePvP(true);
        me->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
    }
    
    if (GetTypeId() != TYPEID_PLAYER && ToCreature()->IsPet() && GetOwner() &&
            (ToPet()->getPetType() == HUNTER_PET || GetOwner()->GetClass() == CLASS_WARLOCK)) {
        GetOwner()->SetInCombatWith(target);
        target->SetInCombatWith(GetOwner());
    }

}

void Unit::SetInCombatState(bool PvP, Unit* enemy)
{
    // only alive units can be in combat
    if(!IsAlive())
        return;

    if(PvP)
        m_CombatTimer = 5250;

    if (IsInCombat() || HasUnitState(UNIT_STATE_EVADE))
        return;

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
    
    if(m_currentSpells[CURRENT_GENERIC_SPELL] && m_currentSpells[CURRENT_GENERIC_SPELL]->getState() != SPELL_STATE_FINISHED)
    {
        if(!m_currentSpells[CURRENT_GENERIC_SPELL]->m_spellInfo->CanBeUsedInCombat())
            InterruptSpell(CURRENT_GENERIC_SPELL);
    }

    if(IsNonMeleeSpellCast(false))
        for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
            if(m_currentSpells[i] && !m_currentSpells[i]->m_spellInfo->CanBeUsedInCombat())
                InterruptSpell(i,false);

    if (Creature* creature = ToCreature())
    {
        // Set home position at place of engaging combat for escorted creatures
        if ((IsAIEnabled && creature->AI()->IsEscorted()) ||
            GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
            creature->SetHomePosition(GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());

        if (enemy)
        {
            if (IsAIEnabled)
            {
                creature->AI()->EnterCombat(enemy);
                RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC); // unit has engaged in combat, remove immunity so players can fight back
            }
            if (creature->GetFormation())
                creature->GetFormation()->MemberAttackStart(creature, enemy);
        }

        if (IsPet())
        {
            UpdateSpeed(MOVE_RUN, true);
            UpdateSpeed(MOVE_SWIM, true);
            UpdateSpeed(MOVE_FLIGHT, true);
        }

       if (!(creature->GetCreatureTemplate()->type_flags & CREATURE_TYPEFLAGS_MOUNTED_COMBAT))
            Dismount();
    }

    if(GetPetGUID()) 
    {
        if (Pet* pet = GetPet())
        {
           pet->SetInCombatState(PvP, enemy);
           pet->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);
        }
    }
}

void Unit::ClearInCombat()
{
    m_CombatTimer = 0;
    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

    // Player's state will be cleared in Player::UpdateContestedPvP
    if (GetTypeId()!=TYPEID_PLAYER) {
        Creature* creature = this->ToCreature();
        if (creature->GetCreatureTemplate() && creature->GetCreatureTemplate()->unit_flags & UNIT_FLAG_IMMUNE_TO_PC)
            SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            
        ClearUnitState(UNIT_STATE_ATTACK_PLAYER);
    }

    if(GetTypeId() != TYPEID_PLAYER && (this->ToCreature())->IsPet())
    {
        if(Unit *owner = GetOwner())
        {
            for(int i = 0; i < MAX_MOVE_TYPE; ++i)
                if(owner->GetSpeedRate(UnitMoveType(i)) > m_speed_rate[UnitMoveType(i)])
                    SetSpeed(UnitMoveType(i), owner->GetSpeedRate(UnitMoveType(i)), true);
        }
    }
    else if(!IsCharmed())
        return;

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);
}

// force will use IsFriendlyTo instead of IsHostileTo, so that neutral creatures can also attack players
// force also ignore feign death
CanAttackResult Unit::CanAttack(Unit const* target, bool force /*= true*/) const
{
    ASSERT(target);

    if (force) {
        if (IsFriendlyTo(target))
            return CAN_ATTACK_RESULT_FRIENDLY;
    } else if (!IsHostileTo(target))
        return CAN_ATTACK_RESULT_FRIENDLY;

    if(target->HasFlag(UNIT_FIELD_FLAGS,
        UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC))
        return CAN_ATTACK_RESULT_TARGET_FLAGS;

    if(   (target->GetTypeId() == TYPEID_PLAYER && ((target->ToPlayer())->IsGameMaster() || (target->ToPlayer())->isSpectator()))
       || (target->GetTypeId() == TYPEID_UNIT && target->GetEntry() == 10 && GetTypeId() != TYPEID_PLAYER && !IsPet()) //training dummies
      ) 
       return CAN_ATTACK_RESULT_OTHERS; 

    // feign death case
    if (!force && target->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH)) {
        if ((GetTypeId() != TYPEID_PLAYER && !GetOwner()) || (GetOwner() && GetOwner()->GetTypeId() != TYPEID_PLAYER))
            return CAN_ATTACK_RESULT_FEIGN_DEATH;
        // if this == player or owner == player check other conditions
    } else if (!target->IsAlive()) // real dead case ~UNIT_FLAG2_FEIGN_DEATH && UNIT_STATE_DIED
        return CAN_ATTACK_RESULT_DEAD;
    else if (target->GetTransForm() == FORM_SPIRITOFREDEMPTION)
        return CAN_ATTACK_RESULT_OTHERS;
    
    //Sathrovarr the Corruptor HACK
    if (target->GetEntry() == 24892 && IsPet())
        return CAN_ATTACK_RESULT_OTHERS;

    if ((m_invisibilityMask || target->m_invisibilityMask) && !CanDetectInvisibilityOf(target))
        return CAN_ATTACK_RESULT_CANNOT_DETECT_INVI;

    if (target->GetVisibility() == VISIBILITY_GROUP_STEALTH)
    {
        StealthDetectedStatus stealthDetectStatus = CanDetectStealthOf(target, GetDistance(target));
        if(stealthDetectStatus == DETECTED_STATUS_NOT_DETECTED)
            return CAN_ATTACK_RESULT_CANNOT_DETECT_STEALTH;
        else if(stealthDetectStatus == DETECTED_STATUS_WARNING)
            return CAN_ATTACK_RESULT_CANNOT_DETECT_STEALTH_WARN_RANGE;
    }

    return CAN_ATTACK_RESULT_OK;
}

bool Unit::IsAttackableByAOE() const
{
    if(!IsAlive())
        return false;

    if(HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC))
        return false;

    if(Player const* p = ToPlayer())
    {
        if(   p->IsGameMaster()  
           || p->isSpectator()
           || p->GetVisibility() == VISIBILITY_OFF
          )
        return false;
    }

    if(Creature const* c = ToCreature())
    {
        if(   c->IsTotem()
           || c->IsInEvadeMode())
            return false;
    }

    if(IsInFlight())
        return false;
    
    return true;
}

int32 Unit::ModifyHealth(int32 dVal)
{
    if(GetDeathState() != ALIVE && GetDeathState() != JUST_RESPAWNED)
    {
        TC_LOG_ERROR("FIXME","Unit::ModifyHealth was called but unit is not alive, aborting.");
        return 0;
    }
    
    if(dVal == 0)
        return 0;

    int32 gain = 0;
    
    // Part of Evade mechanics. Only track health lost, not gained.
    if (dVal < 0 && GetTypeId() != TYPEID_PLAYER && !IsPet())
        SetLastDamagedTime(time(NULL));

    int32 curHealth = (int32)GetHealth();

    int32 val = dVal + curHealth;

    if(val <= 0)
    {
        SetHealth(0);
        return -curHealth;
    }

    int32 maxHealth = (int32)GetMaxHealth();

    if(val < maxHealth)
    {
        SetHealth(val);
        gain = val - curHealth;
    }
    else if(curHealth != maxHealth)
    {
        SetHealth(maxHealth);
        gain = maxHealth - curHealth;
    }

    return gain;
}

void Unit::ModSpellCastTime(SpellInfo const* spellInfo, int32 & castTime, Spell * spell)
{
    if (!spellInfo || castTime < 0)
        return;

    //called from caster
    if(Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_CASTING_TIME, castTime, spell);

    //magic spells
    if (!(spellInfo->HasAttribute(SPELL_ATTR0_ABILITY) || spellInfo->HasAttribute(SPELL_ATTR0_TRADESPELL) || spellInfo->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS)) &&
        ((GetTypeId() == TYPEID_PLAYER && spellInfo->SpellFamilyName) || GetTypeId() == TYPEID_UNIT))
    {
        if(!spellInfo->IsChanneled() || spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_DURATION))
            castTime = int32(float(castTime) * GetFloatValue(UNIT_MOD_CAST_SPEED));
    }
    //ranged attacks
    else if (spellInfo->HasAttribute(SPELL_ATTR0_RANGED) && !spellInfo->HasAttribute(SPELL_ATTR2_AUTOREPEAT_FLAG))
        castTime = int32(float(castTime) * m_modAttackSpeedPct[RANGED_ATTACK]);
}

int32 Unit::ModifyPower(Powers power, int32 dVal)
{
    int32 gain = 0;

    if(dVal==0)
        return 0;

    int32 curPower = (int32)GetPower(power);

    int32 val = dVal + curPower;
    if(val <= 0)
    {
        SetPower(power,0);
        return -curPower;
    }

    int32 maxPower = (int32)GetMaxPower(power);

    if(val < maxPower)
    {
        SetPower(power,val);
        gain = val - curPower;
    }
    else if(curPower != maxPower)
    {
        SetPower(power,maxPower);
        gain = maxPower - curPower;
    }

    return gain;
}

// returns negative amount on power reduction
int32 Unit::ModifyPowerPct(Powers power, float pct, bool apply)
{
    float amount = (float)GetMaxPower(power);
    ApplyPercentModFloatVar(amount, pct, apply);

    return ModifyPower(power, (int32)amount - (int32)GetMaxPower(power));
}

bool Unit::IsVisibleForOrDetect(Unit const* u, bool detect, bool inVisibleList, bool is3dDistance) const
{
    if(!u || !IsInMap(u))
        return false;

    return u->CanSeeOrDetect(this, detect, inVisibleList, is3dDistance);
}

bool Unit::CanSeeOrDetect(Unit const* u, bool detect, bool inVisibleList, bool is3dDistance) const
{
    return true;
}

bool Unit::CanDetectInvisibilityOf(Unit const* u) const
{
    if (m_invisibilityMask & u->m_invisibilityMask) // same group
        return true;
    
    if (GetTypeId() != TYPEID_PLAYER && u->m_invisibilityMask == 0) // An entity with no invisibility is always detectable, right?
        return true;
    
    AuraList const& auras = u->GetAurasByType(SPELL_AURA_MOD_STALKED); // Hunter mark
    for (AuraList::const_iterator iter = auras.begin(); iter != auras.end(); ++iter)
        if ((*iter)->GetCasterGUID() == GetGUID())
            return true;

    // Common invisibility mask
    Unit::AuraList const& iAuras = u->GetAurasByType(SPELL_AURA_MOD_INVISIBILITY);
    Unit::AuraList const& dAuras = GetAurasByType(SPELL_AURA_MOD_INVISIBILITY_DETECTION);
    if (uint32 mask = (m_detectInvisibilityMask & u->m_invisibilityMask)) {
        for (uint32 i = 0; i < 10; ++i) {
            if (((1 << i) & mask) == 0)
                continue;

            // find invisibility level
            uint32 invLevel = 0;
            for (Unit::AuraList::const_iterator itr = iAuras.begin(); itr != iAuras.end(); ++itr)
                if (((*itr)->GetModifier()->m_miscvalue) == i && invLevel < (*itr)->GetModifier()->m_amount)
                    invLevel = (*itr)->GetModifier()->m_amount;

            // find invisibility detect level
            uint32 detectLevel = 0;
            if (i == 6 && GetTypeId() == TYPEID_PLAYER) // special drunk detection case
                detectLevel = (this->ToPlayer())->GetDrunkValue();
            else {
                for (Unit::AuraList::const_iterator itr = dAuras.begin(); itr != dAuras.end(); ++itr)
                    if (((*itr)->GetModifier()->m_miscvalue) == i && detectLevel < (*itr)->GetModifier()->m_amount)
                        detectLevel = (*itr)->GetModifier()->m_amount;
            }

            if (invLevel <= detectLevel)
                return true;
        }
    }

    return false;
}

StealthDetectedStatus Unit::CanDetectStealthOf(Unit const* target, float targetDistance) const
{
    if(GetTypeId() == TYPEID_PLAYER)
        if (ToPlayer()->isSpectator() && !sWorld->getConfig(CONFIG_ARENA_SPECTATOR_STEALTH))
            return DETECTED_STATUS_NOT_DETECTED;
    
    if (!IsAlive())
        return DETECTED_STATUS_NOT_DETECTED;

    if (HasAuraType(SPELL_AURA_DETECT_STEALTH))
        return DETECTED_STATUS_DETECTED;

    AuraList const& auras = target->GetAurasByType(SPELL_AURA_MOD_STALKED); // Hunter mark
    for (AuraList::const_iterator iter = auras.begin(); iter != auras.end(); ++iter)
        if ((*iter)->GetCasterGUID() == GetGUID())
            return DETECTED_STATUS_DETECTED;
    
    if(target->HasAuraEffect(18461,0)) //vanish dummy spell, 2.5s duration after vanish
        return DETECTED_STATUS_NOT_DETECTED;
    
    if (targetDistance == 0.0f) //collision
        return DETECTED_STATUS_DETECTED;

    if (!HasInArc(M_PI/2.0f*3.0f, target)) // can't see 90� behind
        return DETECTED_STATUS_NOT_DETECTED;
    
    //http://wolfendonkane.pagesperso-orange.fr/furtivite.html
    
    float visibleDistance = 17.5f;
    visibleDistance += float(GetLevelForTarget(target)) - target->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH)/5.0f; //max level stealth spell have 350, so if same level and no talent/items boost, this will equal 0
    visibleDistance -= target->GetTotalAuraModifier(SPELL_AURA_MOD_STEALTH_LEVEL); //mainly from talents, improved stealth for rogue and druid add 15 yards in total (15 points). Items with Increases your effective stealth level by 1 have 5.
    visibleDistance += (float)(GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_DETECT, 0) /2.0f); //spells like Track Hidden have 30 here, so you can see 15 yards further. Spells with miscvalue != 0 aren't meant to detect units but traps only
    
    //min and max caps
    if(visibleDistance > MAX_PLAYER_STEALTH_DETECT_RANGE)
        visibleDistance = MAX_PLAYER_STEALTH_DETECT_RANGE;
    else if (visibleDistance < 2.5f)
        visibleDistance = 2.5f; //this can still be reduced with the following check

    //reduce visibility distance depending on angle
    if(!HasInArc(M_PI,target)) //not in front (180�)
        visibleDistance = visibleDistance / 2;
    else if(!HasInArc(M_PI/2,target)) //not in 90� cone in front
        visibleDistance = visibleDistance / 1.5;
    
    float diff = targetDistance - visibleDistance;
    if(diff < 0.0f)
        return DETECTED_STATUS_DETECTED;
    else if (diff <= STEALTH_DETECT_WARNING_RANGE)
        return DETECTED_STATUS_WARNING;
    else // diff > STEALTH_DETECT_WARNING_RANGE
        return DETECTED_STATUS_NOT_DETECTED;
}

void Unit::DestroyForNearbyPlayers()
{
    if(!IsInWorld())
        return;

    std::list<Unit*> targets;
    Trinity::AnyUnitInObjectRangeCheck check(this, GetMap()->GetVisibilityRange());
    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(targets, check);
    VisitNearbyWorldObject(GetMap()->GetVisibilityRange(), searcher);
    for(std::list<Unit*>::iterator iter = targets.begin(); iter != targets.end(); ++iter)
        if(*iter != this && (*iter)->GetTypeId() == TYPEID_PLAYER
            && ((*iter)->ToPlayer())->HaveAtClient(this))
        {
            DestroyForPlayer((*iter)->ToPlayer());
            ((*iter)->ToPlayer())->m_clientGUIDs.erase(GetGUID());
        }
}

void Unit::SetVisibility(UnitVisibility x)
{
    m_Visibility = x;

    if(IsInWorld())
        SetToNotify();

    if(x == VISIBILITY_GROUP_STEALTH)
        DestroyForNearbyPlayers();
}

void Unit::UpdateSpeed(UnitMoveType mtype, bool forced, bool withPet /*= true*/)
{
    int32 main_speed_mod  = 0;
    float stack_bonus     = 1.0f;
    float non_stack_bonus = 1.0f;

    switch(mtype)
    {
        case MOVE_WALK:
            return;
        case MOVE_RUN:
        {
            if (IsMounted()) // Use on mount auras
            {
                main_speed_mod  = GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED);
                stack_bonus     = GetTotalAuraMultiplier(SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS);
                non_stack_bonus = (100.0f + GetMaxPositiveAuraModifier(SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK))/100.0f;
            }
            else
            {
                main_speed_mod  = GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_SPEED);
                stack_bonus     = GetTotalAuraMultiplier(SPELL_AURA_MOD_SPEED_ALWAYS);
                non_stack_bonus = (100.0f + GetMaxPositiveAuraModifier(SPELL_AURA_MOD_SPEED_NOT_STACK))/100.0f;
            }
            break;
        }
        case MOVE_RUN_BACK:
            return;
        case MOVE_SWIM:
        {
            main_speed_mod  = GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_SWIM_SPEED);
            break;
        }
        case MOVE_SWIM_BACK:
            return;
        case MOVE_FLIGHT:
        {
            if (IsMounted()) // Use on mount auras
                main_speed_mod  = GetMaxPositiveAuraModifier(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED);
            else             // Use not mount (shapeshift for example) auras (should stack)
                main_speed_mod  = GetTotalAuraModifier(SPELL_AURA_MOD_INCREASE_FLIGHT_SPEED);
            stack_bonus     = GetTotalAuraMultiplier(SPELL_AURA_MOD_MOUNTED_FLIGHT_SPEED_ALWAYS);
            non_stack_bonus = (100.0 + GetMaxPositiveAuraModifier(SPELL_AURA_MOD_FLIGHT_SPEED_NOT_STACK))/100.0f;
            break;
        }
        case MOVE_FLIGHT_BACK:
            return;
        default:
            TC_LOG_ERROR("FIXME","Unit::UpdateSpeed: Unsupported move type (%d)", mtype);
            return;
    }

    float bonus = non_stack_bonus > stack_bonus ? non_stack_bonus : stack_bonus;
    // now we ready for speed calculation
    float speed  = main_speed_mod ? bonus*(100.0f + main_speed_mod)/100.0f : bonus;

    switch(mtype)
    {
        case MOVE_RUN:
        case MOVE_SWIM:
        case MOVE_FLIGHT:
        {
            // Normalize speed by 191 aura SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED if need
            // TODO: possible affect only on MOVE_RUN
            if(int32 normalization = GetMaxPositiveAuraModifier(SPELL_AURA_USE_NORMAL_MOVEMENT_SPEED))
            {
                //avoid reducing speed for creatures immune to snare or daze
                if (Creature* creature = ToCreature())
                {
                    uint32 immuneMask = creature->GetCreatureTemplate()->MechanicImmuneMask;
                    if (immuneMask & (1 << MECHANIC_SNARE) || immuneMask & (1 << MECHANIC_DAZE))
                        break;
                }

                // Use speed from aura
                float max_speed = normalization / baseMoveSpeed[mtype];
                if (speed > max_speed)
                    speed = max_speed;
            }
            break;
        }
        default:
            break;
    }

    // Apply strongest slow aura mod to speed
    int32 slow = GetMaxNegativeAuraModifier(SPELL_AURA_MOD_DECREASE_SPEED);
    if (slow)
        AddPct(speed, slow);

#ifdef LICH_KING
    if (float minSpeedMod = (float)GetMaxPositiveAuraModifier(SPELL_AURA_MOD_MINIMUM_SPEED))
        float min_speed = minSpeedMod / 100.0f;
    {
        if (speed < min_speed)
            float min_speed = minSpeedMod / 100.0f;
        speed = min_speed;
        if (speed < min_speed)
    }
    speed = min_speed;
#endif

    SetSpeed(mtype, speed, forced, withPet);
}

/* return true speed */
float Unit::GetSpeed( UnitMoveType mtype ) const
{
    return m_speed_rate[mtype]*baseMoveSpeed[mtype];
}

/* Set speed rate of unit */
void Unit::SetSpeed(UnitMoveType mtype, float rate, bool forced, bool withPet /*= true*/)
{
    if (rate < 0)
        rate = 0.0f;

    // Update speed only on change
    if (m_speed_rate[mtype] == rate)
        return;

    m_speed_rate[mtype] = rate;

    PropagateSpeedChange();

    WorldPacket data;
    if(!forced)
    {
        switch(mtype)
        {
            case MOVE_WALK:
                data.Initialize(MSG_MOVE_SET_WALK_SPEED, 8+4+1+4+4+4+4+4+4+4);
                break;
            case MOVE_RUN:
                data.Initialize(MSG_MOVE_SET_RUN_SPEED, 8+4+1+4+4+4+4+4+4+4);
                break;
            case MOVE_RUN_BACK:
                data.Initialize(MSG_MOVE_SET_RUN_BACK_SPEED, 8+4+1+4+4+4+4+4+4+4);
                break;
            case MOVE_SWIM:
                data.Initialize(MSG_MOVE_SET_SWIM_SPEED, 8+4+1+4+4+4+4+4+4+4);
                break;
            case MOVE_SWIM_BACK:
                data.Initialize(MSG_MOVE_SET_SWIM_BACK_SPEED, 8+4+1+4+4+4+4+4+4+4);
                break;
            case MOVE_TURN_RATE:
                data.Initialize(MSG_MOVE_SET_TURN_RATE, 8+4+1+4+4+4+4+4+4+4);
                break;
            case MOVE_FLIGHT:
                data.Initialize(MSG_MOVE_SET_FLIGHT_SPEED, 8+4+1+4+4+4+4+4+4+4);
                break;
            case MOVE_FLIGHT_BACK:
                data.Initialize(MSG_MOVE_SET_FLIGHT_BACK_SPEED, 8+4+1+4+4+4+4+4+4+4);
                break;
#ifdef LICH_KING
            case MOVE_PITCH_RATE:
                data.Initialize(MSG_MOVE_SET_PITCH_RATE, 8+4+2+4+4+4+4+4+4+4);
                break;
#endif
            default:
                TC_LOG_ERROR("entities.unit","Unit::SetSpeed: Unsupported move type (%d), data not sent to client.",mtype);
                return;
        }

        data << GetPackGUID();
        BuildMovementPacket(&data);
        data << float(GetSpeed(mtype));
        SendMessageToSet(&data, true);
    }
    else
    {
        if (GetTypeId() == TYPEID_PLAYER)
        {
            // register forced speed changes for WorldSession::HandleForceSpeedChangeAck
            // and do it only for real sent packets and use run for run/mounted as client expected
            ++ToPlayer()->m_forced_speed_changes[mtype];

            if (!IsInCombat())
                if (Pet* pet = ToPlayer()->GetPet())
                    pet->SetSpeed(mtype, m_speed_rate[mtype], forced);
        }

        switch(mtype)
        {
            case MOVE_WALK:
                data.Initialize(SMSG_FORCE_WALK_SPEED_CHANGE, 16);
                break;
            case MOVE_RUN:
                data.Initialize(SMSG_FORCE_RUN_SPEED_CHANGE, 17);
                break;
            case MOVE_RUN_BACK:
                data.Initialize(SMSG_FORCE_RUN_BACK_SPEED_CHANGE, 16);
                break;
            case MOVE_SWIM:
                data.Initialize(SMSG_FORCE_SWIM_SPEED_CHANGE, 16);
                break;
            case MOVE_SWIM_BACK:
                data.Initialize(SMSG_FORCE_SWIM_BACK_SPEED_CHANGE, 16);
                break;
            case MOVE_TURN_RATE:
                data.Initialize(SMSG_FORCE_TURN_RATE_CHANGE, 16);
                break;
            case MOVE_FLIGHT:
                data.Initialize(SMSG_FORCE_FLIGHT_SPEED_CHANGE, 16);
                break;
            case MOVE_FLIGHT_BACK:
                data.Initialize(SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE, 16);
                break;
#ifdef LICH_KING
            case MOVE_PITCH_RATE:
                data.Initialize(SMSG_FORCE_PITCH_RATE_CHANGE, 16);
                break;
#endif
            default:
                TC_LOG_ERROR("entities.unit","Unit::SetSpeed: Unsupported move type (%d), data not sent to client.",mtype);
                return;
        }
        data << GetPackGUID();
        data << (uint32)0;                                  // moveEvent, NUM_PMOVE_EVTS = 0x39
        if (mtype == MOVE_RUN)
            data << uint8(0);                               // new 2.1.0
        data << float(GetSpeed(mtype));
        SendMessageToSet( &data, true );
    }
    if (withPet) {
        if(GetPetGUID() && !IsInCombat() && m_speed_rate[mtype] >= 1.0f) {
            if (Pet* pet = GetPet())
                pet->SetSpeed(mtype, m_speed_rate[mtype], forced);
        }
        if (GetTypeId() == TYPEID_PLAYER) {
            if (Pet* minipet = ToPlayer()->GetMiniPet())
                minipet->SetSpeed(mtype, m_speed_rate[mtype], forced);
        }
    }
}

void Unit::SetSpeedRate(UnitMoveType mtype, float rate) 
{ 
    m_speed_rate[mtype] = rate; 
    PropagateSpeedChange();
}

void Unit::SetDeathState(DeathState s)
{
    if (s != ALIVE && s!= JUST_RESPAWNED)
    {
        CombatStop();
        DeleteThreatList();
        GetHostileRefManager().deleteReferences();
        ClearComboPointHolders();                           // any combo points pointed to unit lost at it death

        if(IsNonMeleeSpellCast(false))
            InterruptNonMeleeSpells(false);
    }

    if (s == JUST_DIED)
    {
        RemoveAllAurasOnDeath();
        UnsummonAllTotems();

        ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
        ModifyAuraState(AURA_STATE_HEALTHLESS_35_PERCENT, false);
        // remove aurastates allowing special moves
        ClearAllReactives();
        ClearDiminishings();
        if (IsInWorld())
        {
            // Only clear MotionMaster for entities that exists in world
            // Avoids crashes in the following conditions :
            //  * Using 'call pet' on dead pets
            //  * Using 'call stabled pet'
            //  * Logging in with dead pets
            GetMotionMaster()->Clear(false);
            GetMotionMaster()->MoveIdle();
        }
        StopMoving();
        DisableSpline();
        //without this when removing IncreaseMaxHealth aura player may stuck with 1 hp
        //dont know why since in IncreaseMaxHealth currenthealth is checked
        SetHealth(0);
        SetPower(GetPowerType(), 0);
    }
    else if(s == JUST_RESPAWNED)
    {
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE); // clear skinnable for creature and player (at battleground)
    }

    m_deathState = s;
}

void Unit::SetOwnerGUID(uint64 owner) 
{ 
    if (GetOwnerGUID() == owner)
        return;

    SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner); 
    if (!owner)
        return;

    // Update owner dependent fields
    Player* player = ObjectAccessor::GetPlayer(*this, owner);
    if (!player || !player->HaveAtClient(this)) // if player cannot see this unit yet, he will receive needed data with create object
        return;

    SetFieldNotifyFlag(UF_FLAG_OWNER);

    UpdateData udata;
    WorldPacket packet;
    BuildValuesUpdateBlockForPlayer(&udata, player);
    udata.BuildPacket(&packet);
    player->SendDirectMessage(&packet);

    RemoveFieldNotifyFlag(UF_FLAG_OWNER);
}

/*########################################
########                          ########
########       AGGRO SYSTEM       ########
########                          ########
########################################*/
bool Unit::CanHaveThreatList() const
{
    // only creatures can have threat list
    if( GetTypeId() != TYPEID_UNIT )
        return false;

    // only alive units can have threat list
    if (!IsAlive()/* || IsDying()*/)
        return false;

    // totems can not have threat list
    if( (this->ToCreature())->IsTotem() )
        return false;

    // pets can not have a threat list, unless they are controlled by a creature
    if( (this->ToCreature())->IsPet() && IS_PLAYER_GUID(((Pet*)this)->GetOwnerGUID()) )
        return false;

    return true;
}

float Unit::GetThreat(Unit* pUnit) const
{
    if (!pUnit)
        return 0.0f;

    if (CanHaveThreatList())
        return m_ThreatManager.getThreat(pUnit);

    return 0.0f;
}

//======================================================================

void Unit::ApplyTotalThreatModifier(float& threat, SpellSchoolMask schoolMask)
{
    if(!HasAuraType(SPELL_AURA_MOD_THREAT))
        return;

    SpellSchools school = GetFirstSchoolInMask(schoolMask);
    threat = threat * m_threatModifier[school];
}

//======================================================================

void Unit::AddThreat(Unit* pVictim, float threat, SpellSchoolMask schoolMask, SpellInfo const *threatSpell)
{
    // Only mobs can manage threat lists
    if(CanHaveThreatList())
        m_ThreatManager.addThreat(pVictim, threat, schoolMask, threatSpell);
}

//======================================================================

void Unit::DeleteThreatList()
{
    m_ThreatManager.clearReferences();
}

//======================================================================

void Unit::TauntApply(Unit* taunter)
{
    assert(GetTypeId()== TYPEID_UNIT);

    if(!taunter || (taunter->GetTypeId() == TYPEID_PLAYER && ((taunter->ToPlayer())->IsGameMaster() || (taunter->ToPlayer())->isSpectator())))
        return;

    if(!CanHaveThreatList())
        return;

    Unit *target = GetVictim();
    if(target && target == taunter)
        return;

    // Only attack taunter if this is a valid target
    if (!IsCombatStationary() || CanReachWithMeleeAttack(taunter)) {
        SetInFront(taunter);

        if ((this->ToCreature())->IsAIEnabled) {
            (this->ToCreature())->AI()->AttackStart(taunter);
        }
    }

    //m_ThreatManager.tauntApply(taunter);
}

//======================================================================

void Unit::TauntFadeOut(Unit *taunter)
{
    assert(GetTypeId()== TYPEID_UNIT);

    if(!taunter || (taunter->GetTypeId() == TYPEID_PLAYER && ((taunter->ToPlayer())->IsGameMaster() || (taunter->ToPlayer())->isSpectator())))
        return;

    if(!CanHaveThreatList())
        return;

    Unit *target = GetVictim();
    if(!target || target != taunter)
        return;

    if(m_ThreatManager.isThreatListEmpty())
    {
        if((this->ToCreature())->IsAIEnabled) {
            (this->ToCreature())->AI()->EnterEvadeMode();
        }
        return;
    }

    //m_ThreatManager.tauntFadeOut(taunter);
    target = m_ThreatManager.getHostilTarget();

    if (target && target != taunter)
    {
        SetInFront(target);
        if ((this->ToCreature())->IsAIEnabled) {
            (this->ToCreature())->AI()->AttackStart(target);
        }
    }
}

bool Unit::HasInThreatList(uint64 hostileGUID) const
{
    if (!CanHaveThreatList())
        return false;
        
    auto& threatList = m_ThreatManager.getThreatList();
    for (std::list<HostileReference*>::const_iterator itr = threatList.begin(); itr != threatList.end(); ++itr) {
        Unit* current = (*itr)->getTarget();
        if (current && current->GetGUID() == hostileGUID)
            return true;
    }
    
    return false;
}

//======================================================================

Unit* Creature::SelectVictim(bool evade)
{
    //function provides main threat functionality
    //next-victim-selection algorithm and evade mode are called
    //threat list sorting etc.

    //This should not be called by unit who does not have a threatlist
    //or who does not have threat (totem/pet/critter)
    //otherwise enterevademode every update

    Unit* target = NULL;
    
    // First checking if we have some taunt on us
    AuraList const& tauntAuras = GetAurasByType(SPELL_AURA_MOD_TAUNT);
    if (!tauntAuras.empty())
    {
        Unit* caster = tauntAuras.back()->GetCaster();

        // The last taunt aura caster is alive an we are happy to attack him
        if (caster && caster->IsAlive())
            return GetVictim();
        else if (tauntAuras.size() > 1)
        {
            // We do not have last taunt aura caster but we have more taunt auras,
            // so find first available target

            // Auras are pushed_back, last caster will be on the end
            AuraList::const_iterator aura = --tauntAuras.end();
            do
            {
                --aura;
                caster = (*aura)->GetCaster();
                if (caster && CanSeeOrDetect(caster, true) && /* IsValidAttackTarget */ CanAttack(caster) && caster->isInAccessiblePlaceFor(ToCreature()))
                {
                    target = caster;
                    break;
                }
            } while (aura != tauntAuras.begin());
        }
        else
            target = GetVictim();
    }

    if(!m_ThreatManager.isThreatListEmpty())
    {
        //TC_LOG_INFO("%s SelectVictim2", GetName());
        if(!HasAuraType(SPELL_AURA_MOD_TAUNT)) {
            //TC_LOG_INFO("FIXME","%s if");
            target = m_ThreatManager.getHostilTarget();
        }
        else {
            //TC_LOG_INFO("%s else");
            target = GetVictim();
        }
    }

    if(target)
    {
        if(CanAttack(target) == CAN_ATTACK_RESULT_OK)
        {
            //TC_LOG_INFO("%s SelectVictim3", GetName());
            SetInFront(target); 
            return target;
        }
    }
    
    // Case where mob is being kited.
    // Mob may not be in range to attack or may have dropped target. In any case,
    // don't evade if damage received within the last 10 seconds
    // Does not apply to world bosses to prevent kiting to cities
   /* if (!IsWorldBoss() && !GetInstanceId()) { 
        if (time(NULL) - GetLastDamagedTime() <= MAX_AGGRO_RESET_TIME)
            return target;
    } */

    // last case when creature don't must go to evade mode:
    // it in combat but attacker not make any damage and not enter to aggro radius to have record in threat list
    // for example at owner command to pet attack some far away creature
    // Note: creature not have targeted movement generator but have attacker in this case
    /*if( GetMotionMaster()->GetCurrentMovementGeneratorType() != CHASE_MOTION_TYPE )
    {
        for(AttackerSet::const_iterator itr = m_attackers.begin(); itr != m_attackers.end(); ++itr)
        {
            if( (*itr)->IsInMap(this) && CanAttack(*itr) == CAN_ATTACK_RESULT_OK && (*itr)->isInAccessiblePlaceFor(this->ToCreature()) )
                return NULL;
        }
    }*/

    // search nearby enemy before enter evade mode
    //TC_LOG_INFO("%s SelectVictim5", GetName());
    if(HasReactState(REACT_AGGRESSIVE))
    {
        //TC_LOG_INFO("%s SelectVictim6", GetName());
        target = SelectNearestTarget();
        if(target && !IsOutOfThreatArea(target))
            return target;
    }
    
    /* if(m_attackers.size())
        return NULL; */

    if(m_invisibilityMask)
    {
        Unit::AuraList const& iAuras = GetAurasByType(SPELL_AURA_MOD_INVISIBILITY);
        for(Unit::AuraList::const_iterator itr = iAuras.begin(); itr != iAuras.end(); ++itr)
            if((*itr)->IsPermanent() && evade)
            {
                AI()->EnterEvadeMode();
                break;
            }
        return NULL;
    }

    // enter in evade mode in other case
    if (evade) {
        AI()->EnterEvadeMode();
    }
    //TC_LOG_INFO("%s: Returning null", GetName());
    return NULL;
}

//======================================================================
//======================================================================
//======================================================================

int32 Unit::CalculateSpellDamage(SpellInfo const* spellProto, uint8 effect_index, int32 effBasePoints, Unit const* /*target*/)
{
    Player* unitPlayer = (GetTypeId() == TYPEID_PLAYER) ? this->ToPlayer() : NULL;
    
    //TC_LOG_INFO("CalculateSpellDamage for spell %u, effect index %u", spellProto->Id, effect_index);

    uint8 comboPoints = unitPlayer ? unitPlayer->GetComboPoints() : 0;

    //Cap caster level with MaxLevel (upper) and BaseLevel (lower), then get the difference between that level and the spell level
    int32 level = int32(GetLevel());
    if (level > (int32)spellProto->MaxLevel && spellProto->MaxLevel > 0)
        level = (int32)spellProto->MaxLevel;
    else if (level < (int32)spellProto->BaseLevel)
        level = (int32)spellProto->BaseLevel;
    level -= (int32)spellProto->SpellLevel;
    
    /** Dice rolls are from 1 to DieSides */
    float basePointsPerLevel = spellProto->Effects[effect_index].RealPointsPerLevel;
    int32 basePoints = int32(effBasePoints + level * basePointsPerLevel);
    float comboDamage = spellProto->Effects[effect_index].PointsPerComboPoint;
    int32 diceCount = spellProto->Effects[effect_index].BaseDice; //actually diceCount is always 0 or 1
    int32 maxRoll = diceCount * spellProto->Effects[effect_index].DieSides;

    int32 value = basePoints + (maxRoll ? irand(1, maxRoll) : 0);
    //random damage
    if(comboDamage != 0 && unitPlayer /*&& target && (target->GetGUID() == unitPlayer->GetComboTarget())*/)
        value += (int32)(comboDamage * comboPoints);

    if(Player* modOwner = GetSpellModOwner())
    {
        modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_ALL_EFFECTS, value);
        switch(effect_index)
        {
            case 0:
                modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_EFFECT1, value);
                break;
            case 1:
                modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_EFFECT2, value);
                break;
            case 2:
                modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_EFFECT3, value);
                break;
        }
    }

    if(!basePointsPerLevel && (spellProto->Attributes & SPELL_ATTR0_LEVEL_DAMAGE_CALCULATION && spellProto->SpellLevel) &&
            spellProto->Effects[effect_index].Effect != SPELL_EFFECT_WEAPON_PERCENT_DAMAGE &&
            spellProto->Effects[effect_index].Effect != SPELL_EFFECT_KNOCK_BACK)
            //there are many more: slow speed, -healing pct
        //value = int32(value*0.25f*exp(GetLevel()*(70-spellProto->SpellLevel)/1000.0f));
        value = int32(value * (int32)GetLevel() / (int32)(spellProto->SpellLevel ? spellProto->SpellLevel : 1));

    //TC_LOG_INFO("Returning %u", value);

    return value;
}

int32 Unit::CalculateSpellDuration(SpellInfo const* spellProto, uint8 effect_index, Unit const* target)
{
    Player* unitPlayer = (GetTypeId() == TYPEID_PLAYER) ? this->ToPlayer() : NULL;

    uint8 comboPoints = unitPlayer ? unitPlayer->GetComboPoints() : 0;

    int32 minduration = spellProto->GetDuration();
    int32 maxduration = spellProto->GetMaxDuration();

    int32 duration;

    if( minduration != -1 && minduration != maxduration )
        duration = minduration + int32((maxduration - minduration) * comboPoints / 5);
    else
        duration = minduration;

    if (duration > 0)
    {
        int32 mechanic = GetEffectMechanic(spellProto, effect_index);
        // Find total mod value (negative bonus)
        int32 durationMod_always = target->GetTotalAuraModifierByMiscValue(SPELL_AURA_MECHANIC_DURATION_MOD, mechanic);
        // Find max mod (negative bonus)
        int32 durationMod_not_stack = target->GetMaxNegativeAuraModifierByMiscValue(SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK, mechanic);

        int32 durationMod = 0;
        // Select strongest negative mod
        if (durationMod_always > durationMod_not_stack)
            durationMod = durationMod_not_stack;
        else
            durationMod = durationMod_always;

        if (durationMod != 0)
            duration = int32(int64(duration) * (100+durationMod) /100);

        if (duration < 0) duration = 0;
    }

    return duration;
}

DiminishingLevels Unit::GetDiminishing(DiminishingGroup group)
{
    for(Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if(i->DRGroup != group)
            continue;

        if(!i->hitCount)
            return DIMINISHING_LEVEL_1;

        if(!i->hitTime)
            return DIMINISHING_LEVEL_1;

        // If last spell was casted more than 15 seconds ago - reset the count.
        if(i->stack==0 && GetMSTimeDiff(i->hitTime,GetMSTime()) > 15000)
        {
            i->hitCount = DIMINISHING_LEVEL_1;
            return DIMINISHING_LEVEL_1;
        }
        // or else increase the count.
        else
        {
            return DiminishingLevels(i->hitCount);
        }
    }
    return DIMINISHING_LEVEL_1;
}

void Unit::IncrDiminishing(DiminishingGroup group)
{
    // Checking for existing in the table
    bool IsExist = false;
    for(Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if(i->DRGroup != group)
            continue;

        IsExist = true;
        if(i->hitCount < DIMINISHING_LEVEL_IMMUNE)
            i->hitCount += 1;

        break;
    }

    if(!IsExist)
        m_Diminishing.push_back(DiminishingReturn(group,GetMSTime(),DIMINISHING_LEVEL_2));
}

void Unit::ApplyDiminishingToDuration(DiminishingGroup group, int32 &duration,Unit* caster,DiminishingLevels Level)
{
    if(duration == -1 || group == DIMINISHING_NONE)/*(caster->IsFriendlyTo(this) && caster != this)*/
        return;

    //Hack to avoid incorrect diminishing on mind control
    if(group == DIMINISHING_CHARM && caster == this)
        return;

    // test pet/charm masters instead pets/charmedsz
    Unit const* targetOwner = GetCharmerOrOwner();
    Unit const* casterOwner = caster->GetCharmerOrOwner();

    // Duration of crowd control abilities on pvp target is limited by 10 sec. (2.2.0)
    if(duration > 10000 && IsDiminishingReturnsGroupDurationLimited(group))
    {
        Unit const* target = targetOwner ? targetOwner : this;
        Unit const* source = casterOwner ? casterOwner : caster;

        if(target->GetTypeId() == TYPEID_PLAYER && source->GetTypeId() == TYPEID_PLAYER)
            duration = 10000;
    }

    float mod = 1.0f;

    // Some diminishings applies to mobs too (for example, Stun)
    if((GetDiminishingReturnsGroupType(group) == DRTYPE_PLAYER && (targetOwner ? targetOwner->GetTypeId():GetTypeId())  == TYPEID_PLAYER) || GetDiminishingReturnsGroupType(group) == DRTYPE_ALL)
    {
        DiminishingLevels diminish = Level;
        switch(diminish)
        {
            case DIMINISHING_LEVEL_1: break;
            case DIMINISHING_LEVEL_2: mod = 0.5f; break;
            case DIMINISHING_LEVEL_3: mod = 0.25f; break;
            case DIMINISHING_LEVEL_IMMUNE: mod = 0.0f;break;
            default: break;
        }
    }

    duration = int32(duration * mod);
}

void Unit::ApplyDiminishingAura( DiminishingGroup group, bool apply )
{
    // Checking for existing in the table
    for(Diminishing::iterator i = m_Diminishing.begin(); i != m_Diminishing.end(); ++i)
    {
        if(i->DRGroup != group)
            continue;

        i->hitTime = GetMSTime();

        if(apply)
            i->stack += 1;
        else if(i->stack)
            i->stack -= 1;

        break;
    }
}

bool Unit::IsVisibleForInState( Player const* u, bool inVisibleList ) const
{
    return IsVisibleForOrDetect(u, false, inVisibleList, false);
}

uint32 Unit::GetCreatureType() const
{
    if(GetTypeId() == TYPEID_PLAYER)
    {
        SpellShapeshiftEntry const* ssEntry = sSpellShapeshiftStore.LookupEntry((this->ToPlayer())->m_form);
        if(ssEntry && ssEntry->creatureType > 0)
            return ssEntry->creatureType;
        else
            return CREATURE_TYPE_HUMANOID;
    }
    else
        return (this->ToCreature())->GetCreatureTemplate()->type;
}

bool Unit::IsFFAPvP() const 
{ 
    return HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP); 
}

/*#######################################
########                         ########
########       STAT SYSTEM       ########
########                         ########
#######################################*/

bool Unit::HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply)
{
    if(unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_END)
    {
        TC_LOG_ERROR("FIXME","ERROR in HandleStatModifier(): non existed UnitMods or wrong UnitModifierType!");
        return false;
    }

    switch(modifierType)
    {
        case BASE_VALUE:
        case TOTAL_VALUE:
            m_auraModifiersGroup[unitMod][modifierType] += apply ? amount : -amount;
            break;
        case BASE_PCT:
        case TOTAL_PCT:
            ApplyPercentModFloatVar(m_auraModifiersGroup[unitMod][modifierType], amount, apply);
            break;
        default:
            break;
    }

    if(!CanModifyStats())
        return false;

    switch(unitMod)
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
        case UNIT_MOD_HAPPINESS:           UpdateMaxPower(GetPowerTypeByAuraGroup(unitMod));         break;

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

    return true;
}

bool Unit::CanUseAttackType(uint8 attacktype) const
{
    switch (attacktype)
    {
        case BASE_ATTACK:
            return !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISARMED);
#ifdef LICH_KING
        case OFF_ATTACK:
            return !HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISARM_OFFHAND);
        case RANGED_ATTACK:
            return !HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISARM_RANGED);
#endif
        default:
            return true;
    }
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

float Unit::GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const
{
    if( unitMod >= UNIT_MOD_END || modifierType >= MODIFIER_TYPE_END)
    {
        TC_LOG_ERROR("FIXME","ERROR: trial to access non existed modifier value from UnitMods!");
        return 0.0f;
    }

    if(modifierType == TOTAL_PCT && m_auraModifiersGroup[unitMod][modifierType] <= 0.0f)
        return 0.0f;

    return m_auraModifiersGroup[unitMod][modifierType];
}

float Unit::GetTotalStatValue(Stats stat) const
{
    UnitMods unitMod = UnitMods(UNIT_MOD_STAT_START + stat);

    if(m_auraModifiersGroup[unitMod][TOTAL_PCT] <= 0.0f)
        return 0.0f;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value  = m_auraModifiersGroup[unitMod][BASE_VALUE] + GetCreateStat(stat);
    value *= m_auraModifiersGroup[unitMod][BASE_PCT];
    value += m_auraModifiersGroup[unitMod][TOTAL_VALUE];
    value *= m_auraModifiersGroup[unitMod][TOTAL_PCT];

    return value;
}

float Unit::GetTotalAuraModValue(UnitMods unitMod) const
{
    if(unitMod >= UNIT_MOD_END)
    {
        TC_LOG_ERROR("entities.unit","ERROR: trial to access non existed UnitMods in GetTotalAuraModValue()!");
        return 0.0f;
    }

    if(m_auraModifiersGroup[unitMod][TOTAL_PCT] <= 0.0f)
        return 0.0f;

    float value = m_auraModifiersGroup[unitMod][BASE_VALUE];
    value *= m_auraModifiersGroup[unitMod][BASE_PCT];
    value += m_auraModifiersGroup[unitMod][TOTAL_VALUE];
    
    //add dynamic flat mods
    if (unitMod == UNIT_MOD_ATTACK_POWER_RANGED && (GetClassMask() & CLASSMASK_WAND_USERS) == 0) {
        AuraList const& mRAPbyIntellect = GetAurasByType(SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT);
        for (AuraList::const_iterator i = mRAPbyIntellect.begin();i != mRAPbyIntellect.end(); ++i)
            value += int32(GetStat(Stats((*i)->GetModifier()->m_miscvalue)) * (*i)->GetModifierValue() / 100.0f);
    }
    
    value *= m_auraModifiersGroup[unitMod][TOTAL_PCT];

    return value;
}

SpellSchools Unit::GetSpellSchoolByAuraGroup(UnitMods unitMod) const
{
    SpellSchools school = SPELL_SCHOOL_NORMAL;

    switch(unitMod)
    {
        case UNIT_MOD_RESISTANCE_HOLY:     school = SPELL_SCHOOL_HOLY;          break;
        case UNIT_MOD_RESISTANCE_FIRE:     school = SPELL_SCHOOL_FIRE;          break;
        case UNIT_MOD_RESISTANCE_NATURE:   school = SPELL_SCHOOL_NATURE;        break;
        case UNIT_MOD_RESISTANCE_FROST:    school = SPELL_SCHOOL_FROST;         break;
        case UNIT_MOD_RESISTANCE_SHADOW:   school = SPELL_SCHOOL_SHADOW;        break;
        case UNIT_MOD_RESISTANCE_ARCANE:   school = SPELL_SCHOOL_ARCANE;        break;

        default:
            break;
    }

    return school;
}

Stats Unit::GetStatByAuraGroup(UnitMods unitMod) const
{
    Stats stat = STAT_STRENGTH;

    switch(unitMod)
    {
        case UNIT_MOD_STAT_STRENGTH:    stat = STAT_STRENGTH;      break;
        case UNIT_MOD_STAT_AGILITY:     stat = STAT_AGILITY;       break;
        case UNIT_MOD_STAT_STAMINA:     stat = STAT_STAMINA;       break;
        case UNIT_MOD_STAT_INTELLECT:   stat = STAT_INTELLECT;     break;
        case UNIT_MOD_STAT_SPIRIT:      stat = STAT_SPIRIT;        break;

        default:
            break;
    }

    return stat;
}

Powers Unit::GetPowerTypeByAuraGroup(UnitMods unitMod) const
{
    Powers power = POWER_MANA;

    switch(unitMod)
    {
        case UNIT_MOD_MANA:       power = POWER_MANA;       break;
        case UNIT_MOD_RAGE:       power = POWER_RAGE;       break;
        case UNIT_MOD_FOCUS:      power = POWER_FOCUS;      break;
        case UNIT_MOD_ENERGY:     power = POWER_ENERGY;     break;
        case UNIT_MOD_HAPPINESS:  power = POWER_HAPPINESS;  break;

        default:
            break;
    }

    return power;
}

float Unit::GetAPBonusVersus(WeaponAttackType attType, Unit* victim) const
{
    if(!victim)
        return 0.0f;

    float bonus = 0.0f;

    //bonus from my own mods
    AuraType versusBonusType = (attType == RANGED_ATTACK) ? SPELL_AURA_MOD_RANGED_ATTACK_POWER_VERSUS : SPELL_AURA_MOD_MELEE_ATTACK_POWER_VERSUS;
    uint32 creatureTypeMask = victim->GetCreatureTypeMask();
    AuraList const& mCreatureAttackPower = GetAurasByType(versusBonusType);
    for(auto itr : mCreatureAttackPower)
        if(creatureTypeMask & uint32(itr->GetModifier()->m_miscvalue))
            bonus += itr->GetModifierValue();

    //bonus from target mods
    AuraType attackerBonusType = (attType == RANGED_ATTACK) ? SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS : SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS;
    bonus += victim->GetTotalAuraModifier(attackerBonusType);

    return bonus;
}

float Unit::GetTotalAttackPowerValue(WeaponAttackType attType, Unit* victim) const
{
    int32 ap;
    if (attType == RANGED_ATTACK)
    {
        ap = GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER) + GetInt32Value(UNIT_FIELD_RANGED_ATTACK_POWER_MODS);
        if (ap < 0)
            return 0.0f;
        ap = ap * (1.0f + GetFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER));
    }
    else
    {
        ap = GetInt32Value(UNIT_FIELD_ATTACK_POWER) + GetInt32Value(UNIT_FIELD_ATTACK_POWER_MODS);
        if (ap < 0)
            return 0.0f;
        ap = ap * (1.0f + GetFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER));
    }

    if (victim) 
        ap += GetAPBonusVersus(attType,victim);

    if(ap < 0.0f)
        ap = 0.0f;

    return ap;
}

float Unit::GetWeaponDamageRange(WeaponAttackType attType ,WeaponDamageRange type) const
{
    if (attType == OFF_ATTACK && !HaveOffhandWeapon())
        return 0.0f;

    return m_weaponDamage[attType][type];
}

void Unit::SetLevel(uint32 lvl)
{
    SetUInt32Value(UNIT_FIELD_LEVEL, lvl);

    // group update
    if ((GetTypeId() == TYPEID_PLAYER) && (this->ToPlayer())->GetGroup())
        (this->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_LEVEL);

    if (GetTypeId() == TYPEID_PLAYER)
        sWorld->UpdateGlobalPlayerData(ToPlayer()->GetGUIDLow(), PLAYER_UPDATE_DATA_LEVEL, "", lvl);
}

void Unit::SetHealth(uint32 val)
{
    if(GetDeathState() == JUST_DIED)
        val = 0;
    else
    {
        uint32 maxHealth = GetMaxHealth();
        if(maxHealth < val)
            val = maxHealth;
    }

    SetUInt32Value(UNIT_FIELD_HEALTH, val);

    // group update
    if (Player* player = ToPlayer())
    {
        if (player->HaveSpectators())
        {
            SpectatorAddonMsg msg;
            msg.SetPlayer(player->GetName());
            msg.SetCurrentHP(val);
            player->SendSpectatorAddonMsgToBG(msg);
        }

        if(player->GetGroup())
            player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_CUR_HP);
    }
    else if((this->ToCreature())->IsPet())
    {
        Pet *pet = ((Pet*)this);
        if(pet->isControlled())
        {
            Unit *owner = GetOwner();
            if(owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
                (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_HP);
        }
    }
}

void Unit::SetMaxHealth(uint32 val)
{
    uint32 health = GetHealth();
    SetUInt32Value(UNIT_FIELD_MAXHEALTH, val);

    // group update
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (ToPlayer()->HaveSpectators())
        {
            SpectatorAddonMsg msg;
            msg.SetPlayer(ToPlayer()->GetName());
            msg.SetMaxHP(val);
            ToPlayer()->SendSpectatorAddonMsgToBG(msg);
        }

        if((this->ToPlayer())->GetGroup())
            (this->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_MAX_HP);
    }
    else if((this->ToCreature())->IsPet())
    {
        Pet *pet = ((Pet*)this);
        if(pet->isControlled())
        {
            Unit *owner = GetOwner();
            if(owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
                (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MAX_HP);
        }
    }

    if(val < health)
        SetHealth(val);
}

void Unit::SetPower(Powers power, uint32 val)
{
    if(GetPower(power) == val)
        return;

    uint32 maxPower = GetMaxPower(power);
    if(maxPower < val)
        val = maxPower;

    SetStatInt32Value(UNIT_FIELD_POWER1 + power, val);

    // group update
    if (Player* player = ToPlayer())
    {
        if (player->HaveSpectators())
        {
            SpectatorAddonMsg msg;
            msg.SetPlayer(player->GetName());
            msg.SetCurrentPower(val);
            msg.SetPowerType(power);
            player->SendSpectatorAddonMsgToBG(msg);
        }

        if(player->GetGroup())
            player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_CUR_POWER);
    }
    else if((this->ToCreature())->IsPet())
    {
        Pet *pet = ((Pet*)this);
        if(pet->isControlled())
        {
            Unit *owner = GetOwner();
            if(owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
                (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_POWER);
        }

        // Update the pet's character sheet with happiness damage bonus
        if(pet->getPetType() == HUNTER_PET && power == POWER_HAPPINESS)
        {
            pet->UpdateDamagePhysical(BASE_ATTACK);
        }
    }
}

void Unit::SetMaxPower(Powers power, uint32 val)
{
    uint32 cur_power = GetPower(power);
    SetStatInt32Value(UNIT_FIELD_MAXPOWER1 + power, val);

    // group update
    if(GetTypeId() == TYPEID_PLAYER)
    {
        if (ToPlayer()->HaveSpectators())
        {
            SpectatorAddonMsg msg;
            msg.SetPlayer(ToPlayer()->GetName());
            msg.SetMaxPower(val);
            msg.SetPowerType(power);
            ToPlayer()->SendSpectatorAddonMsgToBG(msg);
        }

        if((this->ToPlayer())->GetGroup())
            (this->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_MAX_POWER);
    }
    else if((this->ToCreature())->IsPet())
    {
        Pet *pet = ((Pet*)this);
        if(pet->isControlled())
        {
            Unit *owner = GetOwner();
            if(owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
                (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MAX_POWER);
        }
    }

    if(val < cur_power)
        SetPower(power, val);
}

void Unit::ApplyPowerMod(Powers power, uint32 val, bool apply)
{
    ApplyModUInt32Value(UNIT_FIELD_POWER1+power, val, apply);

    // group update
    if(GetTypeId() == TYPEID_PLAYER)
    {
        if((this->ToPlayer())->GetGroup())
            (this->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_CUR_POWER);
    }
    else if((this->ToCreature())->IsPet())
    {
        Pet *pet = ((Pet*)this);
        if(pet->isControlled())
        {
            Unit *owner = GetOwner();
            if(owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
                (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_CUR_POWER);
        }
    }
}

void Unit::ApplyMaxPowerMod(Powers power, uint32 val, bool apply)
{
    ApplyModUInt32Value(UNIT_FIELD_MAXPOWER1+power, val, apply);

    // group update
    if(GetTypeId() == TYPEID_PLAYER)
    {
        if((this->ToPlayer())->GetGroup())
            (this->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_MAX_POWER);
    }
    else if((this->ToCreature())->IsPet())
    {
        Pet *pet = ((Pet*)this);
        if(pet->isControlled())
        {
            Unit *owner = GetOwner();
            if(owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
                (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MAX_POWER);
        }
    }
}

void Unit::ApplyAuraProcTriggerDamage( Aura* aura, bool apply )
{
    AuraList& tAuraProcTriggerDamage = m_modAuras[SPELL_AURA_PROC_TRIGGER_DAMAGE];
    if(apply)
        tAuraProcTriggerDamage.push_back(aura);
    else
        tAuraProcTriggerDamage.remove(aura);
}

uint32 Unit::GetCreatePowers( Powers power ) const
{
    // POWER_FOCUS and POWER_HAPPINESS only have hunter pet
    switch(power)
    {
        case POWER_MANA:      return GetCreateMana();
        case POWER_RAGE:      return 1000;
        case POWER_FOCUS:     return (GetTypeId()==TYPEID_PLAYER || !((Creature const*)this)->IsPet() || ((Pet const*)this)->getPetType()!=HUNTER_PET ? 0 : 100);
        case POWER_ENERGY:    return 100;
        case POWER_HAPPINESS: return (GetTypeId()==TYPEID_PLAYER || !((Creature const*)this)->IsPet() || ((Pet const*)this)->getPetType()!=HUNTER_PET ? 0 : 1050000);
        default:              break;
    }

    return 0;
}

void Unit::AddToWorld()
{
    if(!IsInWorld())
    {
        WorldObject::AddToWorld();
        m_Notified = false;
        m_IsInNotifyList = false;
        SetToNotify();
    }
}

void Unit::RemoveFromWorld()
{
    // cleanup
    if(IsInWorld())
    {
        RemoveCharmAuras();
        RemoveBindSightAuras();
        RemoveNotOwnSingleTargetAuras();
        WorldObject::RemoveFromWorld();
    }
}

void Unit::CleanupsBeforeDelete(bool finalCleanup)
{
    if (GetTransport())
    {
        GetTransport()->RemovePassenger(this);
        SetTransport(NULL);
        m_movementInfo.transport.Reset();
        m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
    }

    if(GetTypeId()==TYPEID_UNIT && (this->ToCreature())->IsAIEnabled) {
        (this->ToCreature())->AI()->OnRemove();
    }
    // This needs to be before RemoveFromWorld to make GetCaster() return a valid pointer on aura removal
    InterruptNonMeleeSpells(true);

    assert(m_uint32Values);

    //A unit may be in removelist and not in world, but it is still in grid
    //and may have some references during delete
    RemoveAllAuras();
    m_Events.KillAllEvents(false);                      // non-delatable (currently casted spells) will not deleted now but it will deleted at call in Map::RemoveAllObjectsInRemoveList
    CombatStop();
    ClearComboPointHolders();
    DeleteThreatList();
    GetHostileRefManager().setOnlineOfflineState(false);
    RemoveAllGameObjects();
    RemoveAllDynObjects();
    GetMotionMaster()->Clear(false);                    // remove different non-standard movement generators.

    WorldObject::CleanupsBeforeDelete(finalCleanup);
}

void Unit::UpdateCharmAI()
{
    if(GetTypeId() == TYPEID_PLAYER)
        return;

    if(i_disabledAI) // disabled AI must be primary AI
    {
        if(!IsCharmed())
        {
            if(i_AI) delete i_AI;
            i_AI = i_disabledAI;
            i_disabledAI = NULL;
        }
    }
    else
    {
        if(IsCharmed())
        {
            i_disabledAI = i_AI;
            if(IsPossessed())
                i_AI = new PossessedAI(this->ToCreature());
            else
                i_AI = new PetAI(this->ToCreature());
        }
    }
}

CharmInfo* Unit::InitCharmInfo()
{
    if(!m_charmInfo)
        m_charmInfo = new CharmInfo(this);

    return m_charmInfo;
}

void Unit::DeleteCharmInfo()
{
    if(!m_charmInfo)
        return;

    delete m_charmInfo;
    m_charmInfo = NULL;
}

CharmInfo::CharmInfo(Unit* unit)
: m_unit(unit), m_CommandState(COMMAND_FOLLOW), m_petnumber(0), m_barInit(false)
{
    for(int i =0; i<4; ++i)
    {
        m_charmspells[i].spellId = 0;
        m_charmspells[i].active = ACT_DISABLED;
    }
    if(m_unit->GetTypeId() == TYPEID_UNIT)
    {
        m_oldReactState = (m_unit->ToCreature())->GetReactState();
        (m_unit->ToCreature())->SetReactState(REACT_PASSIVE);
    }
}

CharmInfo::~CharmInfo()
{
    if(m_unit->GetTypeId() == TYPEID_UNIT)
    {
        (m_unit->ToCreature())->SetReactState(m_oldReactState);
    }
}

void CharmInfo::InitPetActionBar()
{
    if (m_barInit)
        return;

    // the first 3 SpellOrActions are attack, follow and stay
    for(uint32 i = 0; i < 3; i++)
    {
        PetActionBar[i].Type = ACT_COMMAND;
        PetActionBar[i].SpellOrAction = COMMAND_ATTACK - i;

        PetActionBar[i + 7].Type = ACT_REACTION;
        PetActionBar[i + 7].SpellOrAction = COMMAND_ATTACK - i;
    }
    for(uint32 i=0; i < 4; i++)
    {
        PetActionBar[i + 3].Type = ACT_DISABLED;
        PetActionBar[i + 3].SpellOrAction = 0;
    }
    m_barInit = true;
}

void CharmInfo::InitEmptyActionBar(bool withAttack)
{
    if (m_barInit)
        return;

    for(uint32 x = 0; x < 10; ++x)
    {
        PetActionBar[x].Type = ACT_CAST;
        PetActionBar[x].SpellOrAction = 0;
    }
    if (m_unit->ToCreature()) {
        if ((m_unit->ToCreature())->GetEntry() == 23109) {
            PetActionBar[0].Type = ACT_CAST;
            PetActionBar[0].SpellOrAction = 40325;
            m_barInit = true;
            return;
        }
    }
    if (withAttack)
    {
        PetActionBar[0].Type = ACT_COMMAND;
        PetActionBar[0].SpellOrAction = COMMAND_ATTACK;
    }
    m_barInit = true;
}

void CharmInfo::InitPossessCreateSpells()
{
    if (m_unit->GetEntry() == 25653)
        InitEmptyActionBar(false);
    else
        InitEmptyActionBar();

    if(m_unit->GetTypeId() == TYPEID_UNIT)
    {
        for(uint32 i = 0; i < CREATURE_MAX_SPELLS; ++i)
        {
            uint32 spellid = (m_unit->ToCreature())->m_spells[i];
            if(IsPassiveSpell(spellid))
                m_unit->CastSpell(m_unit, spellid, true);
            else
                AddSpellToAB(0, spellid, i, ACT_CAST);
        }
    }
}

void CharmInfo::InitCharmCreateSpells()
{
    if(m_unit->GetTypeId() == TYPEID_PLAYER)                //charmed players don't have spells
    {
        InitEmptyActionBar();
        return;
    }

    InitEmptyActionBar(false);

    for(uint32 x = 0; x < CREATURE_MAX_SPELLS; ++x)
    {
        uint32 spellId = (m_unit->ToCreature())->m_spells[x];
        m_charmspells[x].spellId = spellId;

        if(!spellId)
            continue;

        SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if(!spellInfo)
            continue;

        if (spellInfo->IsPassive())
        {
            m_unit->CastSpell(m_unit, spellId, true);
            m_charmspells[x].active = ACT_PASSIVE;
        }
        else
        {
            ActiveStates newstate;
            bool onlyselfcast = true;

            for(uint32 i = 0;i<3 && onlyselfcast;++i)       //non existent spell will not make any problems as onlyselfcast would be false -> break right away
            {
                if(spellInfo->Effects[i].TargetA.GetTarget()!= TARGET_UNIT_CASTER && spellInfo->Effects[i].TargetA.GetTarget()!= 0)
                    onlyselfcast = false;
            }
             
            if(onlyselfcast || !spellInfo->IsPositive())   //only self cast and spells versus enemies are autocastable
                newstate = ACT_DISABLED;
            else
                newstate = ACT_CAST;

            AddSpellToAB(0, spellId, x, newstate);
        }
    }
}

bool CharmInfo::AddSpellToAB(uint32 oldid, uint32 newid, uint8 index, ActiveStates newstate)
{
    if((PetActionBar[index].Type == ACT_DISABLED || PetActionBar[index].Type == ACT_ENABLED || PetActionBar[index].Type == ACT_CAST) && PetActionBar[index].SpellOrAction == oldid)
    {
        PetActionBar[index].SpellOrAction = newid;
        if(!oldid)
        {
            if(newstate == ACT_DECIDE)
                PetActionBar[index].Type = ACT_DISABLED;
            else
                PetActionBar[index].Type = newstate;
        }
        return true;
    }

    return false;
}

void CharmInfo::ToggleCreatureAutocast(uint32 spellid, bool apply)
{
    if(IsPassiveSpell(spellid))
        return;

    for(uint32 x = 0; x < CREATURE_MAX_SPELLS; ++x)
    {
        if(spellid == m_charmspells[x].spellId)
        {
            m_charmspells[x].active = apply ? ACT_ENABLED : ACT_DISABLED;
        }
    }
}

void CharmInfo::SetPetNumber(uint32 petnumber, bool statwindow)
{
    m_petnumber = petnumber;
    if(statwindow)
        m_unit->SetUInt32Value(UNIT_FIELD_PETNUMBER, m_petnumber);
    else
        m_unit->SetUInt32Value(UNIT_FIELD_PETNUMBER, 0);
}

//TODO : replace by AURA_STATE_FROZEN
bool Unit::IsFrozen() const
{
    AuraList const& mRoot = GetAurasByType(SPELL_AURA_MOD_ROOT);
    for(AuraList::const_iterator i = mRoot.begin(); i != mRoot.end(); ++i)
        if((*i)->GetSpellInfo()->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST)
            return true;
    return false;
}

struct ProcTriggeredData
{
    ProcTriggeredData(SpellProcEventEntry const * _spellProcEvent, Aura* _triggeredByAura)
        : spellProcEvent(_spellProcEvent), triggeredByAura(_triggeredByAura),
        triggeredByAura_SpellPair(Unit::spellEffectPair(triggeredByAura->GetId(),triggeredByAura->GetEffIndex()))
        {}
    SpellProcEventEntry const *spellProcEvent;
    Aura* triggeredByAura;
    Unit::spellEffectPair triggeredByAura_SpellPair;
};

typedef std::list< ProcTriggeredData > ProcTriggeredList;
typedef std::list< uint32> RemoveSpellList;

// List of auras that CAN be trigger but may not exist in spell_proc_event
// in most case need for drop charges
// in some types of aura need do additional check
// for example SPELL_AURA_MECHANIC_IMMUNITY - need check for mechanic
static bool isTriggerAura[TOTAL_AURAS];
static bool isNonTriggerAura[TOTAL_AURAS];
void InitTriggerAuraData()
{
    for (int i=0;i<TOTAL_AURAS;i++)
    {
      isTriggerAura[i]=false;
      isNonTriggerAura[i] = false;
    }
    isTriggerAura[SPELL_AURA_DUMMY] = true;
    isTriggerAura[SPELL_AURA_MOD_CONFUSE] = true;
    isTriggerAura[SPELL_AURA_MOD_THREAT] = true;
    isTriggerAura[SPELL_AURA_MOD_STUN] = true; // Aura not have charges but need remove him on trigger
    isTriggerAura[SPELL_AURA_MOD_DAMAGE_DONE] = true;
    isTriggerAura[SPELL_AURA_MOD_DAMAGE_TAKEN] = true;
    isTriggerAura[SPELL_AURA_MOD_RESISTANCE] = true;
    isTriggerAura[SPELL_AURA_MOD_ROOT] = true;
    isTriggerAura[SPELL_AURA_REFLECT_SPELLS] = true;
    isTriggerAura[SPELL_AURA_DAMAGE_IMMUNITY] = true;
    isTriggerAura[SPELL_AURA_PROC_TRIGGER_SPELL] = true;
    isTriggerAura[SPELL_AURA_PROC_TRIGGER_DAMAGE] = true;
    isTriggerAura[SPELL_AURA_MOD_CASTING_SPEED] = true;
    isTriggerAura[SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT] = true;
    isTriggerAura[SPELL_AURA_MOD_POWER_COST_SCHOOL] = true;
    isTriggerAura[SPELL_AURA_REFLECT_SPELLS_SCHOOL] = true;
    isTriggerAura[SPELL_AURA_MECHANIC_IMMUNITY] = true;
    isTriggerAura[SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN] = true;
    isTriggerAura[SPELL_AURA_SPELL_MAGNET] = true;
    isTriggerAura[SPELL_AURA_MOD_ATTACK_POWER] = true;
    isTriggerAura[SPELL_AURA_ADD_CASTER_HIT_TRIGGER] = true;
    isTriggerAura[SPELL_AURA_OVERRIDE_CLASS_SCRIPTS] = true;
    isTriggerAura[SPELL_AURA_MOD_MECHANIC_RESISTANCE] = true;
    isTriggerAura[SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS] = true;
    isTriggerAura[SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS] = true;
    isTriggerAura[SPELL_AURA_MOD_HASTE] = true;
    isTriggerAura[SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE]=true;
    isTriggerAura[SPELL_AURA_PRAYER_OF_MENDING] = true;

    isNonTriggerAura[SPELL_AURA_MOD_POWER_REGEN]=true;
    isNonTriggerAura[SPELL_AURA_RESIST_PUSHBACK]=true;
}

uint32 createProcExtendMask(SpellNonMeleeDamage *damageInfo, SpellMissInfo missCondition)
{
    uint32 procEx = PROC_EX_NONE;
    // Check victim state
    if (missCondition!=SPELL_MISS_NONE)
    switch (missCondition)
    {
        case SPELL_MISS_MISS:    procEx|=PROC_EX_MISS;   break;
        case SPELL_MISS_RESIST:  procEx|=PROC_EX_RESIST; break;
        case SPELL_MISS_DODGE:   procEx|=PROC_EX_DODGE;  break;
        case SPELL_MISS_PARRY:   procEx|=PROC_EX_PARRY;  break;
        case SPELL_MISS_BLOCK:   procEx|=PROC_EX_BLOCK;  break;
        case SPELL_MISS_EVADE:   procEx|=PROC_EX_EVADE;  break;
        case SPELL_MISS_IMMUNE:  procEx|=PROC_EX_IMMUNE; break;
        case SPELL_MISS_IMMUNE2: procEx|=PROC_EX_IMMUNE; break;
        case SPELL_MISS_DEFLECT: procEx|=PROC_EX_DEFLECT;break;
        case SPELL_MISS_ABSORB:  procEx|=PROC_EX_ABSORB; break;
        case SPELL_MISS_REFLECT: procEx|=PROC_EX_REFLECT;break;
        default:
            break;
    }
    else
    {
        // On block
        if (damageInfo->blocked)
            procEx|=PROC_EX_BLOCK;
        // On absorb
        if (damageInfo->absorb)
            procEx|=PROC_EX_ABSORB;
        // On crit
        if (damageInfo->HitInfo & SPELL_HIT_TYPE_CRIT)
            procEx|=PROC_EX_CRITICAL_HIT;
        else
            procEx|=PROC_EX_NORMAL_HIT;
    }
    return procEx;
}

void Unit::ProcDamageAndSpellFor( bool isVictim, Unit * pTarget, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, SpellInfo const * procSpell, uint32 damage )
{
    ++m_procDeep;
    if (m_procDeep > 5)
    {
        TC_LOG_ERROR("FIXME","Prevent possible stack owerflow in Unit::ProcDamageAndSpellFor");
        if (procSpell)
            TC_LOG_ERROR("FIXME","  Spell %u", procSpell->Id);
        --m_procDeep;
        return;
    }
    
    // For melee/ranged based attack need update skills and set some Aura states
    if (procFlag & MELEE_BASED_TRIGGER_MASK)
    {
        // Update skills here for players
        if (GetTypeId() == TYPEID_PLAYER)
        {
            // On melee based hit/miss/resist need update skill (for victim and attacker)
            if (procExtra&(PROC_EX_NORMAL_HIT|PROC_EX_CRITICAL_HIT|PROC_EX_MISS|PROC_EX_RESIST))
            {
                if (pTarget->GetTypeId() != TYPEID_PLAYER && pTarget->GetCreatureType() != CREATURE_TYPE_CRITTER)
                    (this->ToPlayer())->UpdateCombatSkills(pTarget, attType, MELEE_HIT_MISS, isVictim);
            }
            // Update defence if player is victim and parry/dodge/block
            if (isVictim && procExtra&(PROC_EX_DODGE|PROC_EX_PARRY|PROC_EX_BLOCK))
                (this->ToPlayer())->UpdateDefense();
        }
        // If exist crit/parry/dodge/block need update aura state (for victim and attacker)
        if (procExtra & (PROC_EX_CRITICAL_HIT|PROC_EX_PARRY|PROC_EX_DODGE|PROC_EX_BLOCK))
        {
            // for victim
            if (isVictim)
            {
                // if victim and dodge attack
                if (procExtra&PROC_EX_DODGE)
                {
                    //Update AURA_STATE on dodge
                    if (GetClass() != CLASS_ROGUE) // skip Rogue Riposte
                    {
                        ModifyAuraState(AURA_STATE_DEFENSE, true);
                        StartReactiveTimer( REACTIVE_DEFENSE );
                    }
                }
                // if victim and parry attack
                if (procExtra & PROC_EX_PARRY)
                {
                    // For Hunters only Counterattack (skip Mongoose bite)
                    if (GetClass() == CLASS_HUNTER)
                    {
                        ModifyAuraState(AURA_STATE_HUNTER_PARRY, true);
                        StartReactiveTimer( REACTIVE_HUNTER_PARRY );
                    }
                    else
                    {
                        ModifyAuraState(AURA_STATE_DEFENSE, true);
                        StartReactiveTimer( REACTIVE_DEFENSE );
                    }
                }
                // if and victim block attack
                if (procExtra & PROC_EX_BLOCK)
                {
                    ModifyAuraState(AURA_STATE_DEFENSE,true);
                    StartReactiveTimer( REACTIVE_DEFENSE );
                }
            }
            else //For attacker
            {
                // Overpower on victim dodge
                if (procExtra&PROC_EX_DODGE && GetTypeId() == TYPEID_PLAYER && GetClass() == CLASS_WARRIOR)
                {
                    (this->ToPlayer())->AddComboPoints(pTarget, 1);
                    StartReactiveTimer( REACTIVE_OVERPOWER );
                }
                // Enable AURA_STATE_CRIT on crit
                if (procExtra & PROC_EX_CRITICAL_HIT)
                {
                    ModifyAuraState(AURA_STATE_CRIT, true);
                    StartReactiveTimer( REACTIVE_CRIT );
                    if(GetClass()==CLASS_HUNTER)
                    {
                        ModifyAuraState(AURA_STATE_HUNTER_CRIT_STRIKE, true);
                        StartReactiveTimer( REACTIVE_HUNTER_CRIT );
                    }
                }
            }
        }
    }

    RemoveSpellList removedSpells;
    ProcTriggeredList procTriggered;
    // Fill procTriggered list
    
    for(AuraMap::const_iterator itr = GetAuras().begin(); itr!= GetAuras().end(); ++itr)
    {
        SpellProcEventEntry const* spellProcEvent = NULL;
        //TC_LOG_INFO("IsTriggeredAtSpellProcEvent: %u %u %x %x %u %s %s", itr->second->GetId(), procSpell ? procSpell->Id : 0, procFlag, procExtra, attType, isVictim ? "victim" : "attacker", (damage > 0) ? "damage > 0" : "damage < 0");
        if(!IsTriggeredAtSpellProcEvent(itr->second, procSpell, procFlag, procExtra, attType, isVictim, (damage > 0), spellProcEvent)) {
            //TC_LOG_INFO("FIXME","No.");
            continue;
        }

        procTriggered.push_back( ProcTriggeredData(spellProcEvent, itr->second) );
    }
    // Handle effects proceed this time
    for(ProcTriggeredList::iterator i = procTriggered.begin(); i != procTriggered.end(); ++i)
    {
        // Some auras can be deleted in function called in this loop (except first, ofc)
        // Until storing auars in std::multimap to hard check deleting by another way
        if(i != procTriggered.begin())
        {
            bool found = false;
            AuraMap::const_iterator lower = GetAuras().lower_bound(i->triggeredByAura_SpellPair);
            AuraMap::const_iterator upper = GetAuras().upper_bound(i->triggeredByAura_SpellPair);
            for(AuraMap::const_iterator itr = lower; itr!= upper; ++itr)
            {
                if(itr->second==i->triggeredByAura)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
            {
//                TC_LOG_DEBUG("FIXME","Spell aura %u (id:%u effect:%u) has been deleted before call spell proc event handler", i->triggeredByAura->GetModifier()->m_auraname, i->triggeredByAura_SpellPair.first, i->triggeredByAura_SpellPair.second);
//                TC_LOG_DEBUG("FIXME","It can be deleted one from early proccesed auras:");
//                for(ProcTriggeredList::iterator i2 = procTriggered.begin(); i != i2; ++i2)
//                    TC_LOG_DEBUG("FIXME","     Spell aura %u (id:%u effect:%u)", i->triggeredByAura->GetModifier()->m_auraname,i2->triggeredByAura_SpellPair.first,i2->triggeredByAura_SpellPair.second);
//                    TC_LOG_DEBUG("FIXME","     <end of list>");
                continue;
            }
        }

        SpellProcEventEntry const *spellProcEvent = i->spellProcEvent;
        Aura *triggeredByAura = i->triggeredByAura;
        Modifier const* auraModifier = triggeredByAura->GetModifier();
        SpellInfo const *spellInfo = triggeredByAura->GetSpellInfo();
        bool useCharges = triggeredByAura->m_procCharges > 0;
        // For players set spell cooldown if need
        uint32 cooldown = 0;
        if (GetTypeId() == TYPEID_PLAYER && spellProcEvent && spellProcEvent->cooldown)
            cooldown = spellProcEvent->cooldown;

        switch(auraModifier->m_auraname)
        {
            case SPELL_AURA_PROC_TRIGGER_SPELL:
            {
                //TC_LOG_INFO("ProcDamageAndSpell: casting spell %u (triggered by %s aura of spell %u)", spellInfo->Id,(isVictim?"a victim's":"an attacker's"), triggeredByAura->GetId());
                // Don`t drop charge or add cooldown for not started trigger
                if (!HandleProcTriggerSpell(pTarget, damage, triggeredByAura, procSpell, procFlag, procExtra, cooldown))
                    continue;

                break;
            }
            case SPELL_AURA_PROC_TRIGGER_DAMAGE:
            {
                SpellNonMeleeDamage damageInfo(this, pTarget, spellInfo->Id, spellInfo->SchoolMask);
                uint32 damage = SpellDamageBonusDone(pTarget, spellInfo, auraModifier->m_amount, SPELL_DIRECT_DAMAGE);
                damage = pTarget->SpellDamageBonusTaken(this, spellInfo, damage, SPELL_DIRECT_DAMAGE);
                CalculateSpellDamageTaken(&damageInfo, damage, spellInfo);
                SendSpellNonMeleeDamageLog(&damageInfo);
                DealSpellDamage(&damageInfo, true);
                break;
            }
            case SPELL_AURA_MANA_SHIELD:
            case SPELL_AURA_DUMMY:
            {
                if (!HandleDummyAuraProc(pTarget, damage, triggeredByAura, procSpell, procFlag, procExtra, cooldown))
                    continue;
                break;
            }
            case SPELL_AURA_MOD_HASTE:
            {
                if (!HandleHasteAuraProc(pTarget, damage, triggeredByAura, procSpell, procFlag, procExtra, cooldown))
                    continue;
                
                if (triggeredByAura->GetSpellInfo()->HasVisual(2759) && triggeredByAura->GetSpellInfo()->SpellIconID == 108) { // Shaman and Warrior Flurry
                    if (procExtra & PROC_EX_CRITICAL_HIT)
                        useCharges = false;
                }
                
                break;
            }
            case SPELL_AURA_OVERRIDE_CLASS_SCRIPTS:
            {
                if (!HandleOverrideClassScriptAuraProc(pTarget, triggeredByAura, procSpell, cooldown))
                    continue;
                break;
            }
            case SPELL_AURA_PRAYER_OF_MENDING:
            {
                HandleMendingAuraProc(triggeredByAura);
                break;
            }
            case SPELL_AURA_MOD_STUN:
                // Remove by default, but if charge exist drop it
                if (triggeredByAura->m_procCharges == 0)
                   removedSpells.push_back(triggeredByAura->GetId());
                break;
            //case SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS:
            case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:
                // Hunter's Mark (1-4 Rangs) increase AP with every hit
                if (spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER && (spellInfo->SpellFamilyFlags&0x0000000000000400LL))
                {
                    uint32 basevalue = triggeredByAura->GetBasePoints();
                    triggeredByAura->SetModifierValue(triggeredByAura->GetModifierValue() + (basevalue+1)/10);
                    if (triggeredByAura->GetModifierValue() > (basevalue+1)*4)
                        triggeredByAura->SetModifierValue((basevalue+1)*4);
                }
                break;
            case SPELL_AURA_MOD_CASTING_SPEED:
                // Skip melee hits or instant cast spells
                if (procSpell == NULL || procSpell->CalcCastTime() == 0)
                    continue;
                break;
            case SPELL_AURA_REFLECT_SPELLS_SCHOOL:
                // Skip Melee hits and spells ws wrong school
                if (procSpell == NULL || (auraModifier->m_miscvalue & procSpell->SchoolMask) == 0)
                    continue;
                break;
            case SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT:
            case SPELL_AURA_MOD_POWER_COST_SCHOOL:
                // Skip melee hits and spells ws wrong school or zero cost
                if (procSpell == NULL ||
                    (procSpell->ManaCost == 0 && procSpell->ManaCostPercentage == 0) || // Cost check
                    (auraModifier->m_miscvalue & procSpell->SchoolMask) == 0)         // School check
                    continue;
                break;
            case SPELL_AURA_MECHANIC_IMMUNITY:
                // Compare mechanic
                if (procSpell==NULL || procSpell->Mechanic != auraModifier->m_miscvalue)
                    continue;
                break;
            case SPELL_AURA_MOD_MECHANIC_RESISTANCE:
                // Compare mechanic
                if (procSpell==NULL || procSpell->Mechanic != auraModifier->m_miscvalue)
                    continue;
                break;
            default:
                // nothing do, just charges counter
                break;
        }
        // Remove charge (aura can be removed by triggers)
        if(useCharges && triggeredByAura->GetId() != 23920)
        {
            // need found aura on drop (can be dropped by triggers)
            AuraMap::const_iterator lower = GetAuras().lower_bound(i->triggeredByAura_SpellPair);
            AuraMap::const_iterator upper = GetAuras().upper_bound(i->triggeredByAura_SpellPair);
            for(AuraMap::const_iterator itr = lower; itr!= upper; ++itr)
            {
                if(itr->second == i->triggeredByAura)
                {
                     triggeredByAura->m_procCharges -=1;
                     triggeredByAura->UpdateAuraCharges();
                     if (triggeredByAura->m_procCharges <= 0)
                          removedSpells.push_back(triggeredByAura->GetId());
                    break;
                }
            }
        }
    }
    if (removedSpells.size())
    {
        // Sort spells and remove dublicates
        removedSpells.sort();
        removedSpells.unique();
        // Remove auras from removedAuras
        for(RemoveSpellList::const_iterator i = removedSpells.begin(); i != removedSpells.end();i++)
            RemoveAurasDueToSpell(*i);
    }
    --m_procDeep;
}

SpellSchoolMask Unit::GetMeleeDamageSchoolMask() const
{
    return SPELL_SCHOOL_MASK_NORMAL;
}

bool Unit::isCharmedOwnedByPlayerOrPlayer() const { return IS_PLAYER_GUID(GetCharmerOrOwnerOrOwnGUID()); }

Player* Unit::GetSpellModOwner() const
{
    if(GetTypeId()==TYPEID_PLAYER) {
        Player *p = const_cast<Player*>(ToPlayer());
        return p;
    }

    if((this->ToCreature())->IsPet() || (this->ToCreature())->IsTotem())
    {
        Unit* owner = GetOwner();
        if(owner && owner->GetTypeId()==TYPEID_PLAYER)
            return owner->ToPlayer();
    }
    return NULL;
}

///----------Pet responses methods-----------------
void Unit::SendPetCastFail(uint32 spellid, uint8 msg)
{
    Unit *owner = GetCharmerOrOwner();
    if(!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_PET_CAST_FAILED, (4+1));
    data << uint32(spellid);
    data << uint8(msg);
    (owner->ToPlayer())->SendDirectMessage(&data);
}

void Unit::SendPetActionFeedback (uint8 msg)
{
    Unit* owner = GetOwner();
    if(!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_PET_ACTION_FEEDBACK, 1);
    data << uint8(msg);
    (owner->ToPlayer())->SendDirectMessage(&data);
}

void Unit::SendPetTalk (uint32 pettalk)
{
    Unit* owner = GetOwner();
    if(!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_PET_ACTION_SOUND, 8+4);
    data << uint64(GetGUID());
    data << uint32(pettalk);
    (owner->ToPlayer())->SendDirectMessage(&data);
}

void Unit::SendPetSpellCooldown (uint32 spellid, time_t cooltime)
{
    Unit* owner = GetOwner();
    if(!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_SPELL_COOLDOWN, 8+1+4+4);
    data << uint64(GetGUID());
    data << uint8(0x0);                                     // flags (0x1, 0x2)
    data << uint32(spellid);
    data << uint32(cooltime);

    (owner->ToPlayer())->SendDirectMessage(&data);
}

void Unit::SendPetAIReaction()
{
    Unit* owner = GetOwner();
    if(!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    SendAIReaction(AI_REACTION_HOSTILE, owner->ToPlayer());
}

void Unit::SendAIReaction(AIReaction reaction, Player* target)
{
    WorldPacket data(SMSG_AI_REACTION, 12);
    data << uint64(GetGUID());
    data << uint32(reaction); 
    if(target)
        target->SendDirectMessage(&data);
    else
        SendMessageToSet(&data, true);
}

///----------End of Pet responses methods----------

void Unit::StopMoving()
{
    ClearUnitState(UNIT_STATE_MOVING);

    // not need send any packets if not in world or not moving
    if (!IsInWorld() || movespline->Finalized())
        return;

    // Update position now since Stop does not start a new movement that can be updated later
    UpdateSplinePosition();
    Movement::MoveSplineInit init(this);
    init.Stop();
}

void Unit::StopMovingOnCurrentPos() // pussywizard
{
    ClearUnitState(UNIT_STATE_MOVING);

    // not need send any packets if not in world
    if (!IsInWorld())
        return;

    DisableSpline(); // pussywizard: required so Launch() won't recalculate position from previous spline
    Movement::MoveSplineInit init(this);
    init.MoveTo(GetPositionX(), GetPositionY(), GetPositionZ());
    init.SetFacing(GetOrientation());
    init.Launch();
}

void Unit::SendMovementFlagUpdate()
{
    WorldPacket data;
    BuildHeartBeatMsg(&data);
    SendMessageToSet(&data, false);
}

void Unit::SendMovementFlagUpdate(float dist)
{
    WorldPacket data;
    BuildHeartBeatMsg(&data);
    SendMessageToSetInRange(&data, dist, false);
}

bool Unit::IsSitState() const
{
    uint8 s = GetStandState();
    return s == PLAYER_STATE_SIT_CHAIR || s == PLAYER_STATE_SIT_LOW_CHAIR ||
        s == PLAYER_STATE_SIT_MEDIUM_CHAIR || s == PLAYER_STATE_SIT_HIGH_CHAIR ||
        s == PLAYER_STATE_SIT;
}

bool Unit::IsStandState() const
{
    uint8 s = GetStandState();
    return !IsSitState() && s != PLAYER_STATE_SLEEP && s != PLAYER_STATE_KNEEL;
}

void Unit::SetStandState(uint8 state)
{
    SetByteValue(UNIT_FIELD_BYTES_1, 0, state);

    if (IsStandState())
       RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_SEATED);

    if(GetTypeId()==TYPEID_PLAYER)
    {
        WorldPacket data(SMSG_STANDSTATE_UPDATE, 1);
        data << (uint8)state;
        (this->ToPlayer())->SendDirectMessage(&data);
    }
}

bool Unit::IsPolymorphed() const
{
    uint32 transformId = GetTransForm();
    if (!transformId)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(transformId);
    if (!spellInfo)
        return false;

    return spellInfo->GetSpellSpecific() == SPELL_MAGE_POLYMORPH;
}

void Unit::SetDisplayId(uint32 modelId)
{
    SetUInt32Value(UNIT_FIELD_DISPLAYID, modelId);

    if(GetTypeId() == TYPEID_UNIT && (this->ToCreature())->IsPet())
    {
        Pet *pet = ((Pet*)this);
        if(!pet->isControlled())
            return;
        Unit *owner = GetOwner();
        if(owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
            (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MODEL_ID);
    }
}

void Unit::ClearComboPointHolders()
{
    while(!m_ComboPointHolders.empty())
    {
        uint32 lowguid = *m_ComboPointHolders.begin();

        Player* plr = sObjectMgr->GetPlayer(MAKE_NEW_GUID(lowguid, 0, HIGHGUID_PLAYER));
        if(plr && plr->GetComboTarget()==GetGUID())         // recheck for safe
            plr->ClearComboPoints();                        // remove also guid from m_ComboPointHolders;
        else
            m_ComboPointHolders.erase(lowguid);             // or remove manually
    }
}

void Unit::ClearAllReactives()
{

    for(int i=0; i < MAX_REACTIVE; ++i)
        m_reactiveTimer[i] = 0;

    if (HasAuraState( AURA_STATE_DEFENSE))
        ModifyAuraState(AURA_STATE_DEFENSE, false);
    if (GetClass() == CLASS_HUNTER && HasAuraState( AURA_STATE_HUNTER_PARRY))
        ModifyAuraState(AURA_STATE_HUNTER_PARRY, false);
    if (HasAuraState( AURA_STATE_CRIT))
        ModifyAuraState(AURA_STATE_CRIT, false);
    if (GetClass() == CLASS_HUNTER && HasAuraState( AURA_STATE_HUNTER_CRIT_STRIKE)  )
        ModifyAuraState(AURA_STATE_HUNTER_CRIT_STRIKE, false);

    if(GetClass() == CLASS_WARRIOR && GetTypeId() == TYPEID_PLAYER)
        (this->ToPlayer())->ClearComboPoints();
}

void Unit::UpdateReactives( uint32 p_time )
{
    for(int i = 0; i < MAX_REACTIVE; ++i)
    {
        ReactiveType reactive = ReactiveType(i);

        if(!m_reactiveTimer[reactive])
            continue;

        if ( m_reactiveTimer[reactive] <= p_time)
        {
            m_reactiveTimer[reactive] = 0;

            switch ( reactive )
            {
                case REACTIVE_DEFENSE:
                    if (HasAuraState(AURA_STATE_DEFENSE))
                        ModifyAuraState(AURA_STATE_DEFENSE, false);
                    break;
                case REACTIVE_HUNTER_PARRY:
                    if ( GetClass() == CLASS_HUNTER && HasAuraState(AURA_STATE_HUNTER_PARRY))
                        ModifyAuraState(AURA_STATE_HUNTER_PARRY, false);
                    break;
                case REACTIVE_CRIT:
                    if (HasAuraState(AURA_STATE_CRIT))
                        ModifyAuraState(AURA_STATE_CRIT, false);
                    break;
                case REACTIVE_HUNTER_CRIT:
                    if ( GetClass() == CLASS_HUNTER && HasAuraState(AURA_STATE_HUNTER_CRIT_STRIKE) )
                        ModifyAuraState(AURA_STATE_HUNTER_CRIT_STRIKE, false);
                    break;
                case REACTIVE_OVERPOWER:
                    if(GetClass() == CLASS_WARRIOR && GetTypeId() == TYPEID_PLAYER)
                        (this->ToPlayer())->ClearComboPoints();
                    break;
                default:
                    break;
            }
        }
        else
        {
            m_reactiveTimer[reactive] -= p_time;
        }
    }
}

Unit* Unit::SelectNearbyTarget(float dist) const
{
    std::list<Unit *> targets;
    Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(this, this, dist);
    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, u_check);
    VisitNearbyObject(dist, searcher);

    // remove current target
    if(GetVictim())
        targets.remove(GetVictim());

    // remove not LoS targets
    for(std::list<Unit *>::iterator tIter = targets.begin(); tIter != targets.end();)
    {
        if(!IsWithinLOSInMap(*tIter))
        {
            std::list<Unit *>::iterator tIter2 = tIter;
            ++tIter;
            targets.erase(tIter2);
        }
        else
            ++tIter;
    }

    // no appropriate targets
    if(targets.empty())
        return NULL;

    // select random
    uint32 rIdx = GetMap()->urand(0,targets.size()-1);
    std::list<Unit *>::const_iterator tcIter = targets.begin();
    for(uint32 i = 0; i < rIdx; ++i)
        ++tcIter;

    return *tcIter;
}

void Unit::ApplyAttackTimePercentMod( WeaponAttackType att,float val, bool apply )
{
    //TC_LOG_DEBUG("FIXME","ApplyAttackTimePercentMod(%u,%f,%s)",att,val,apply?"true":"false");
    float remainingTimePct = (float)m_attackTimer[att] / (GetAttackTime(att) * m_modAttackSpeedPct[att]);
    //TC_LOG_DEBUG("FIXME","remainingTimePct = %f",remainingTimePct);    
    //TC_LOG_DEBUG("FIXME","m_modAttackSpeedPct[att] before = %f",m_modAttackSpeedPct[att]);
    if(val > 0)
    {
        ApplyPercentModFloatVar(m_modAttackSpeedPct[att], val, !apply);
        ApplyPercentModFloatValue(UNIT_FIELD_BASEATTACKTIME+att,val,!apply);
    }
    else
    {
        ApplyPercentModFloatVar(m_modAttackSpeedPct[att], -val, apply);
        ApplyPercentModFloatValue(UNIT_FIELD_BASEATTACKTIME+att,-val,apply);
    }
    //TC_LOG_DEBUG("FIXME","m_modAttackSpeedPct[att] after = %f",m_modAttackSpeedPct[att]);
    m_attackTimer[att] = uint32(GetAttackTime(att) * m_modAttackSpeedPct[att] * remainingTimePct);
}

void Unit::ApplyCastTimePercentMod(float val, bool apply )
{
    if(val > 0)
        ApplyPercentModFloatValue(UNIT_MOD_CAST_SPEED,val,!apply);
    else
        ApplyPercentModFloatValue(UNIT_MOD_CAST_SPEED,-val,apply);
}

void Unit::UpdateAuraForGroup(uint8 slot)
{
    if(GetTypeId() == TYPEID_PLAYER)
    {
        Player* player = this->ToPlayer();
        if(player->GetGroup())
        {
            player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_AURAS);
            player->SetAuraUpdateMask(slot);
        }
    }
    else if(GetTypeId() == TYPEID_UNIT && (this->ToCreature())->IsPet())
    {
        Pet *pet = ((Pet*)this);
        if(pet->isControlled())
        {
            Unit *owner = GetOwner();
            if(owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
            {
                (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_AURAS);
                pet->SetAuraUpdateMask(slot);
            }
        }
    }
}

float Unit::GetAPMultiplier(WeaponAttackType attType, bool normalized)
{
    if (!normalized || GetTypeId() != TYPEID_PLAYER)
        return float(GetAttackTime(attType))/1000.0f;

    Item *Weapon = (this->ToPlayer())->GetWeaponForAttack(attType);
    if (!Weapon)
        return 2.4;                                         // fist attack

    switch (Weapon->GetTemplate()->InventoryType)
    {
        case INVTYPE_2HWEAPON:
            return 3.3;
        case INVTYPE_RANGED:
        case INVTYPE_RANGEDRIGHT:
        case INVTYPE_THROWN:
            return 2.8;
        case INVTYPE_WEAPON:
        case INVTYPE_WEAPONMAINHAND:
        case INVTYPE_WEAPONOFFHAND:
        default:
            return Weapon->GetTemplate()->SubClass==ITEM_SUBCLASS_WEAPON_DAGGER ? 1.7 : 2.4;
    }
}

Aura* Unit::GetDummyAura( uint32 spell_id ) const
{
    Unit::AuraList const& mDummy = GetAurasByType(SPELL_AURA_DUMMY);
    for(Unit::AuraList::const_iterator itr = mDummy.begin(); itr != mDummy.end(); ++itr)
        if ((*itr)->GetId() == spell_id)
            return *itr;

    return NULL;
}

bool Unit::IsUnderLastManaUseEffect() const
{
    return  GetMSTimeDiff(m_lastManaUse,GetMSTime()) < 5000;
}

void Unit::SetContestedPvP(Player *attackedPlayer)
{
    Player* player = GetCharmerOrOwnerPlayerOrPlayerItself();

    if(!player || (attackedPlayer && (attackedPlayer == player || (player->duel && (player->duel->opponent == attackedPlayer)))))
        return;

    player->SetContestedPvPTimer(30000);
    if(!player->HasUnitState(UNIT_STATE_ATTACK_PLAYER))
    {
        player->AddUnitState(UNIT_STATE_ATTACK_PLAYER);
        player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP);
        // call MoveInLineOfSight for nearby contested guards
        player->SetVisibility(player->GetVisibility());
    }
    if(!HasUnitState(UNIT_STATE_ATTACK_PLAYER))
    {
        AddUnitState(UNIT_STATE_ATTACK_PLAYER);
        // call MoveInLineOfSight for nearby contested guards
        SetToNotify();
    }
}

void Unit::AddPetAura(PetAura const* petSpell)
{
    m_petAuras.insert(petSpell);
    if(Pet* pet = GetPet())
        pet->CastPetAura(petSpell);
}

void Unit::RemovePetAura(PetAura const* petSpell)
{
    m_petAuras.erase(petSpell);
    if(Pet* pet = GetPet())
        pet->RemoveAurasDueToSpell(petSpell->GetAura(pet->GetEntry()));
}

Pet* Unit::CreateTamedPetFrom(Creature* creatureTarget,uint32 spell_id)
{
    Pet* pet = new Pet(HUNTER_PET);

    if(!pet->CreateBaseAtCreature(creatureTarget))
    {
        delete pet;
        return NULL;
    }

    pet->SetOwnerGUID(GetGUID());
    pet->SetCreatorGUID(GetGUID());
    pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, GetFaction());
    pet->SetUInt32Value(UNIT_CREATED_BY_SPELL, spell_id);

    if(!pet->InitStatsForLevel(creatureTarget->GetLevel()))
    {
        TC_LOG_ERROR("FIXME","ERROR: Pet::InitStatsForLevel() failed for creature (Entry: %u)!",creatureTarget->GetEntry());
        delete pet;
        return NULL;
    }

    pet->GetCharmInfo()->SetPetNumber(sObjectMgr->GeneratePetNumber(), true);
    // this enables pet details window (Shift+P)
    pet->AIM_Initialize();
    pet->InitPetCreateSpells();
    pet->SetHealth(pet->GetMaxHealth());

    return pet;
}

bool Unit::IsTriggeredAtSpellProcEvent(Aura* aura, SpellInfo const* procSpell, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, bool isVictim, bool active, SpellProcEventEntry const*& spellProcEvent )
{
    SpellInfo const* spellProto = aura->GetSpellInfo();

    // Get proc Event Entry
    spellProcEvent = sSpellMgr->GetSpellProcEvent(spellProto->Id);

    // Aura info stored here
    Modifier const* mod = aura->GetModifier();

    //TC_LOG_INFO("FIXME","IsTriggeredAtSpellProcEvent1");
    // Skip this auras
    if (isNonTriggerAura[mod->m_auraname])
        return false;
    //TC_LOG_INFO("IsTriggeredAtSpellProcEvent2");
    // If not trigger by default and spellProcEvent==NULL - skip
    if (!isTriggerAura[mod->m_auraname] && spellProcEvent==NULL)
        return false;
    //TC_LOG_INFO("IsTriggeredAtSpellProcEvent3");
    // Get EventProcFlag
    uint32 EventProcFlag;
    if (spellProcEvent && spellProcEvent->ProcFlags) // if exist get custom spellProcEvent->ProcFlags
        EventProcFlag = spellProcEvent->ProcFlags;
    else
        EventProcFlag = spellProto->ProcFlags;       // else get from spell proto
    // Continue if no trigger exist
    if (!EventProcFlag)
        return false;
    //TC_LOG_INFO("IsTriggeredAtSpellProcEvent4");
    // Inner fire exception
    if (procFlag & PROC_FLAG_HAD_DAMAGE_BUT_ABSORBED && procExtra & PROC_EX_ABSORB) {
        if (spellProto->HasVisual(211) && spellProto->SpellFamilyName == 6)
            return false;
    }
    //TC_LOG_INFO("IsTriggeredAtSpellProcEvent5");
    // Check spellProcEvent data requirements
    if(!SpellMgr::IsSpellProcEventCanTriggeredBy(spellProcEvent, EventProcFlag, procSpell, procFlag, procExtra, active))
        return false;

    // Aura added by spell can`t trigger from self (prevent drop cahres/do triggers)
    // But except periodic triggers (can triggered from self)
    if(procSpell && procSpell->Id == spellProto->Id && !(spellProto->ProcFlags & PROC_FLAG_TAKEN_PERIODIC))
        return false;
    //TC_LOG_INFO("FIXME","IsTriggeredAtSpellProcEvent6");
    // Check if current equipment allows aura to proc
    if(!isVictim && GetTypeId() == TYPEID_PLAYER)
    {
        if(spellProto->EquippedItemClass == ITEM_CLASS_WEAPON)
        {
            Item *item = NULL;
            if(attType == BASE_ATTACK)
                item = (this->ToPlayer())->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            else if (attType == OFF_ATTACK)
                item = (this->ToPlayer())->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            else
                item = (this->ToPlayer())->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);

            if (!(this->ToPlayer())->IsUseEquipedWeapon(attType==BASE_ATTACK))
                return false;

            if(!item || item->IsBroken() || item->GetTemplate()->Class != ITEM_CLASS_WEAPON || !((1<<item->GetTemplate()->SubClass) & spellProto->EquippedItemSubClassMask))
                return false;
        }
        else if(spellProto->EquippedItemClass == ITEM_CLASS_ARMOR)
        {
            // Check if player is wearing shield
            Item *item = (this->ToPlayer())->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if(!item || item->IsBroken() || item->GetTemplate()->Class != ITEM_CLASS_ARMOR || !((1<<item->GetTemplate()->SubClass) & spellProto->EquippedItemSubClassMask))
                return false;
        }
    }
    //TC_LOG_INFO("FIXME","IsTriggeredAtSpellProcEvent7");
    // Get chance from spell
    float chance = (float)spellProto->ProcChance;
    // If in spellProcEvent exist custom chance, chance = spellProcEvent->customChance;
    if(spellProcEvent && spellProcEvent->customChance)
        chance = spellProcEvent->customChance;
    // If PPM exist calculate chance from PPM
    if(!isVictim && spellProcEvent && spellProcEvent->ppmRate != 0)
    {
        uint32 WeaponSpeed = GetAttackTime(attType);
        chance = GetPPMProcChance(WeaponSpeed, spellProcEvent->ppmRate);
    }
    // Apply chance modifer aura
    if(Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id,SPELLMOD_CHANCE_OF_SUCCESS,chance);

    //TC_LOG_INFO("FIXME","IsTriggeredAtSpellProcEvent8");
    return roll_chance_f(chance);
}

bool Unit::HandleMendingAuraProc( Aura* triggeredByAura )
{
    // aura can be deleted at casts
    SpellInfo const* spellProto = triggeredByAura->GetSpellInfo();
    uint32 effIdx = triggeredByAura->GetEffIndex();
    int32 heal = triggeredByAura->GetModifier()->m_amount;
    //uint64 caster_guid = triggeredByAura->GetCasterGUID();
    uint64 caster_guid = GetGUID();

    // jumps
    int32 jumps = triggeredByAura->m_procCharges-1;

    // current aura expire
    triggeredByAura->m_procCharges = 1;             // will removed at next charges decrease

    // next target selection
    if(jumps > 0 && GetTypeId()==TYPEID_PLAYER && IS_PLAYER_GUID(caster_guid))
    {
        if(Player* caster = (triggeredByAura->GetCaster()->ToPlayer()))
        {
            float radius;
            if (spellProto->Effects[effIdx].HasRadius())
                radius = spellProto->Effects[effIdx].CalcRadius(caster->GetSpellModOwner());
            else
                radius = spellProto->GetMaxRange(false, caster->GetSpellModOwner());

            if(Player* target = (this->ToPlayer())->GetNextRandomRaidMember(radius))
            {
                // aura will applied from caster, but spell casted from current aura holder
                SpellModifier *mod = new SpellModifier;
                mod->op = SPELLMOD_CHARGES;
                mod->value = jumps-5;               // negative
                mod->type = SPELLMOD_FLAT;
                mod->spellId = spellProto->Id;
                mod->effectId = effIdx;
                mod->lastAffected = NULL;
                mod->mask = spellProto->SpellFamilyFlags;
                mod->charges = 0;

                caster->AddSpellMod(mod, true);
                CastCustomSpell(target,spellProto->Id,&heal,NULL,NULL,true,NULL,triggeredByAura,caster->GetGUID());
                caster->AddSpellMod(mod, false);

                heal = SpellHealingBonusDone(target, spellProto, heal, HEAL);
                heal = target->SpellHealingBonusTaken(caster, spellProto, heal, HEAL);
            }
        }
    }
    // else double heal here?

    // heal
    CastCustomSpell(this,33110,&heal,NULL,NULL,true,NULL,NULL,caster_guid);
    return true;
}

void Unit::RemoveAurasAtChanneledTarget(SpellInfo const* spellInfo, Unit * caster)
{
/*    uint64 target_guid = GetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT);
    if(target_guid == GetGUID())
        return;

    if(!IS_UNIT_GUID(target_guid))
        return;

    Unit* target = ObjectAccessor::GetUnit(*this, target_guid);*/
    if(!caster)
        return;

    for (AuraMap::iterator iter = GetAuras().begin(); iter != GetAuras().end(); )
    {
        if (iter->second->GetId() == spellInfo->Id && iter->second->GetCasterGUID() == caster->GetGUID())
            RemoveAura(iter);
        else
            ++iter;
    }
}

/*-----------------------TRINITY-----------------------------*/

void Unit::SetToNotify()
{
    if(m_IsInNotifyList)
        return;

    if(Map *map = GetMap())
        map->AddUnitToNotify(this);
}

void Unit::Kill(Unit *pVictim, bool durabilityLoss)
{
    pVictim->SetHealth(0);

    // find player: owner of controlled `this` or `this` itself maybe
    Player *player = GetCharmerOrOwnerPlayerOrPlayerItself();
    Creature* creature = pVictim->ToCreature();

    bool bRewardIsAllowed = true;
    if(pVictim->GetTypeId() == TYPEID_UNIT)
    {
        bRewardIsAllowed = (pVictim->ToCreature())->IsDamageEnoughForLootingAndReward();
        if(!bRewardIsAllowed)
            (pVictim->ToCreature())->SetLootRecipient(NULL);
    }
    
    if(bRewardIsAllowed && pVictim->GetTypeId() == TYPEID_UNIT && (pVictim->ToCreature())->GetLootRecipient())
        player = (pVictim->ToCreature())->GetLootRecipient();

    // Reward player, his pets, and group/raid members
    // call kill spell proc event (before real die and combat stop to triggering auras removed at death/combat stop)
    if(bRewardIsAllowed && player && player!=pVictim)
    {
        WorldPacket data(SMSG_PARTYKILLLOG, (8+8)); // send event PARTY_KILL
        data << uint64(player->GetGUID()); // player with killing blow
        data << uint64(pVictim->GetGUID()); // victim

        Player* looter = player;
        Group* group = player->GetGroup();

        bool hasLooterGuid = false;

        if (group)
        {
            group->BroadcastPacket(&data, group->GetMemberGroup(player->GetGUID()) != 0);

            if (creature)
            {
                group->UpdateLooterGuid(creature, true);
                if (group->GetLooterGuid())
                {
                    looter = ObjectAccessor::FindPlayer(group->GetLooterGuid());
                    if (looter)
                    {
                        hasLooterGuid = true;
                        creature->SetLootRecipient(looter);   // update creature loot recipient to the allowed looter.
                    }
                }
            }
        }
        else
        {
            player->SendDirectMessage(&data);

            if (creature)
            {
                WorldPacket data2(SMSG_LOOT_LIST, 8 + 1 + 1);
                data2 << uint64(creature->GetGUID());
                data2 << uint8(0); // master loot guid
                data2 << uint8(0); // no group looter
                player->SendMessageToSet(&data2, true);
            }
        }

        // Generate loot before updating looter
        if (creature)
        {
            Loot* loot = &creature->loot;

            loot->clear();
            if (uint32 lootid = creature->GetCreatureTemplate()->lootid)
                loot->FillLoot(lootid, LootTemplates_Creature, looter /*, false, false, creature->GetLootMode() */);

            loot->generateMoneyLoot(creature->GetCreatureTemplate()->mingold, creature->GetCreatureTemplate()->maxgold);

            if (group)
            {
                if (hasLooterGuid)
                    group->SendLooter(creature, looter);
                else
                    group->SendLooter(creature, NULL);

                // Update round robin looter only if the creature had loot
                if (!loot->empty())
                    group->UpdateLooterGuid(creature);
            }
        }

        if(player->RewardPlayerAndGroupAtKill(pVictim))
            player->ProcDamageAndSpell(pVictim, PROC_FLAG_KILL_AND_GET_XP, PROC_FLAG_KILLED, PROC_EX_NONE, 0);
        else
            player->ProcDamageAndSpell(pVictim, PROC_FLAG_NONE, PROC_FLAG_KILLED,PROC_EX_NONE, 0);
    }

    // if talent known but not triggered (check priest class for speedup check)
    bool SpiritOfRedemption = false;
    if(pVictim->GetTypeId()==TYPEID_PLAYER && pVictim->GetClass()==CLASS_PRIEST )
    {
        AuraList const& vDummyAuras = pVictim->GetAurasByType(SPELL_AURA_DUMMY);
        for(AuraList::const_iterator itr = vDummyAuras.begin(); itr != vDummyAuras.end(); ++itr)
        {
            if((*itr)->GetSpellInfo()->SpellIconID==1654)
            {
                // save value before aura remove
                uint32 ressSpellId = pVictim->GetUInt32Value(PLAYER_SELF_RES_SPELL);
                if(!ressSpellId)
                    ressSpellId = (pVictim->ToPlayer())->GetResurrectionSpellId();
                //Remove all expected to remove at death auras (most important negative case like DoT or periodic triggers)
                pVictim->RemoveAllAurasOnDeath();
                // restore for use at real death
                pVictim->SetUInt32Value(PLAYER_SELF_RES_SPELL,ressSpellId);

                // FORM_SPIRITOFREDEMPTION and related auras
                pVictim->CastSpell(pVictim,27827,true,NULL,*itr);
                //pVictim->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE); // should not be attackable
                SpiritOfRedemption = true;
                (pVictim->ToPlayer())->SetSpiritRedeptionKiller(GetGUID());
                break;
            }
        }
    }

    if(!SpiritOfRedemption)
    {
        pVictim->SetDeathState(JUST_DIED);
        //pVictim->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE); // reactive attackable flag
    }

    // 10% durability loss on death
    // clean InHateListOf
    if (pVictim->GetTypeId() == TYPEID_PLAYER)
    {
        // remember victim PvP death for corpse type and corpse reclaim delay
        // at original death (not at SpiritOfRedemtionTalent timeout)
        (pVictim->ToPlayer())->SetPvPDeath(player!=NULL);

        // only if not player and not controlled by player pet. And not at BG
        if (durabilityLoss && !player && !(pVictim->ToPlayer())->InBattleground())
        {
            (pVictim->ToPlayer())->DurabilityLossAll(0.10f,false);
            // durability lost message
            WorldPacket data(SMSG_DURABILITY_DAMAGE_DEATH, 0);
            (pVictim->ToPlayer())->SendDirectMessage(&data);
        }
        // Call KilledUnit for creatures
        if (GetTypeId() == TYPEID_UNIT && (this->ToCreature())->IsAIEnabled) {
            (this->ToCreature())->AI()->KilledUnit(pVictim);

            auto attackers = pVictim->GetAttackers();
            for(auto attacker : attackers)
            {
                if(Creature* c = attacker->ToCreature())
                    if(c->IsAIEnabled)
                        c->AI()->AttackedUnitDied(pVictim);
            }
        }
            
        if (GetTypeId() == TYPEID_PLAYER) {
            if (Pet* minipet = ToPlayer()->GetMiniPet()) {
                if (minipet->IsAIEnabled)
                    minipet->AI()->MasterKilledUnit(pVictim);
            }
            if (Pet* pet = ToPlayer()->GetPet()) {
                if (pet->IsAIEnabled)
                    pet->AI()->MasterKilledUnit(pVictim);
            }
            for (uint8 slot = 0; slot < MAX_TOTEM; slot++) {
                if (Creature* totem = ObjectAccessor::GetCreature(*this, m_TotemSlot[slot]))
                    totem->AI()->MasterKilledUnit(pVictim);
            }
            if (Creature* totem = ObjectAccessor::GetCreature(*this, m_TotemSlot254)) // Slot for some quest totems
                totem->AI()->MasterKilledUnit(pVictim);
        }

        // last damage from non duel opponent or opponent controlled creature
        if((pVictim->ToPlayer())->duel)
        {
            (pVictim->ToPlayer())->duel->opponent->CombatStopWithPets(true);
            (pVictim->ToPlayer())->CombatStopWithPets(true);
            (pVictim->ToPlayer())->DuelComplete(DUEL_INTERUPTED);
        }
        
        if (InstanceScript* instance = ((InstanceScript*)pVictim->GetInstanceScript()))
            instance->PlayerDied(pVictim->ToPlayer());
    }
    else                                                // creature died
    {
        Creature *cVictim = pVictim->ToCreature();
        
        if(!cVictim->IsPet())
        {
            //save threat list before deleting it
            if(cVictim->IsWorldBoss())
                cVictim->ConvertThreatListIntoPlayerListAtDeath();

            cVictim->DeleteThreatList();
            if(!cVictim->GetFormation() || !cVictim->GetFormation()->isLootLinked(cVictim)) //the flag is set when whole group is dead for those with linked loot 
                cVictim->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        }

        // Call KilledUnit for creatures, this needs to be called after the lootable flag is set
        if (GetTypeId() == TYPEID_UNIT && (this->ToCreature())->IsAIEnabled) {
            (this->ToCreature())->AI()->KilledUnit(pVictim);
        }
            
        if (GetTypeId() == TYPEID_PLAYER) {
            if (Pet* minipet = ToPlayer()->GetMiniPet()) {
                if (minipet->IsAIEnabled)
                    minipet->AI()->MasterKilledUnit(pVictim);
            }
            if (Pet* pet = ToPlayer()->GetPet()) {
                if (pet->IsAIEnabled)
                    pet->AI()->MasterKilledUnit(pVictim);
            }
            for (uint8 slot = 0; slot < MAX_TOTEM; slot++) {
                if (Creature* totem = ObjectAccessor::GetCreature(*this, m_TotemSlot[slot]))
                    totem->AI()->MasterKilledUnit(pVictim);
            }
            if (Creature* totem = ObjectAccessor::GetCreature(*this, m_TotemSlot254)) // Slot for some quest totems
                totem->AI()->MasterKilledUnit(pVictim);
        }

        // Call creature just died function
        if (cVictim->IsAIEnabled) {
            cVictim->AI()->JustDied(this);
        }

        cVictim->WarnDeathToFriendly();
        
        // Despawn creature pet if alive
        if (Pet* pet = cVictim->GetPet()) {
            if (pet->IsAlive())
                pet->DisappearAndDie();
        }
        
        // Log down if worldboss
        if (cVictim->IsWorldBoss() && (cVictim->GetMap()->IsRaid() || cVictim->GetMap()->IsWorldMap()))
        {
            LogBossDown(cVictim);
        }

        // Dungeon specific stuff, only applies to players killing creatures
        if(cVictim->GetInstanceId())
        {
            InstanceScript *pInstance = ((InstanceScript*)cVictim->GetInstanceScript());
            if (pInstance)
                pInstance->OnCreatureKill(cVictim);

            Map *m = cVictim->GetMap();
            Player *creditedPlayer = GetCharmerOrOwnerPlayerOrPlayerItself();
            // TODO: do instance binding anyway if the charmer/owner is offline

            if(m->IsDungeon() && creditedPlayer)
            {
                if(m->IsRaid() || m->IsHeroic())
                {
                    if(cVictim->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_INSTANCE_BIND)
                    {
                        ((InstanceMap *)m)->PermBindAllPlayers(creditedPlayer);
                    }
                }
                else
                {
                    // the reset time is set but not added to the scheduler
                    // until the players leave the instance
                    time_t resettime = cVictim->GetRespawnTimeEx() + 2 * HOUR;
                    if(InstanceSave *save = sInstanceSaveMgr->GetInstanceSave(cVictim->GetInstanceId()))
                        if(save->GetResetTime() < resettime) save->SetResetTime(resettime);
                }
            }
        }
    }

    // outdoor pvp things, do these after setting the death state, else the player activity notify won't work... doh...
    // handle player kill only if not suicide (spirit of redemption for example)
    if(player && this != pVictim)
        if(OutdoorPvP * pvp = player->GetOutdoorPvP())
            pvp->HandleKill(player, pVictim);

    if(pVictim->GetTypeId() == TYPEID_PLAYER)
        if(OutdoorPvP * pvp = (pVictim->ToPlayer())->GetOutdoorPvP())
            pvp->HandlePlayerActivityChanged(pVictim->ToPlayer());

    // battleground things (do this at the end, so the death state flag will be properly set to handle in the bg->handlekill)
    if(player && player->InBattleground() && !SpiritOfRedemption)
    {
        if(Battleground *bg = player->GetBattleground())
        {
            if(pVictim->GetTypeId() == TYPEID_PLAYER)
                bg->HandleKillPlayer(pVictim->ToPlayer(), player);
            else
                bg->HandleKillUnit(pVictim->ToCreature(), player);
        }
    }
}

void Unit::SetControlled(bool apply, UnitState state)
{
    if (apply)
    {
        if (HasUnitState(state))
            return;

        AddUnitState(state);
        switch (state)
        {
            case UNIT_STATE_STUNNED:
                SetStunned(true);
                CastStop();
                break;
            case UNIT_STATE_ROOT:
                if (!HasUnitState(UNIT_STATE_STUNNED))
                    SetRooted(true);
                break;
            case UNIT_STATE_CONFUSED:
                if (!HasUnitState(UNIT_STATE_STUNNED))
                {
                    ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
                    SendMeleeAttackStop();
                    // SendAutoRepeatCancel ?
                    SetConfused(true);
                    CastStop();
                }
                break;
            case UNIT_STATE_FLEEING:
                if (!HasUnitState(UNIT_STATE_STUNNED | UNIT_STATE_CONFUSED))
                {
                    ClearUnitState(UNIT_STATE_MELEE_ATTACKING);
                    SendMeleeAttackStop();
                    // SendAutoRepeatCancel ?
                    SetFeared(true);
                    CastStop();
                }
                break;
            default:
                break;
        }
    }
    else
    {
        switch (state)
        {
            case UNIT_STATE_STUNNED:
                if (HasAuraType(SPELL_AURA_MOD_STUN))
                    return;

                SetStunned(false);
                break;
            case UNIT_STATE_ROOT:
                if (HasAuraType(SPELL_AURA_MOD_ROOT))
                    return;

                SetRooted(false);
                break;
            case UNIT_STATE_CONFUSED:
                if (HasAuraType(SPELL_AURA_MOD_CONFUSE))
                    return;

                SetConfused(false);
                break;
            case UNIT_STATE_FLEEING:
                if (HasAuraType(SPELL_AURA_MOD_FEAR))
                    return;

                SetFeared(false);
                break;
            default:
                return;
        }

        ClearUnitState(state);

        if (HasUnitState(UNIT_STATE_STUNNED))
            SetStunned(true);
        else
        {
            if (HasUnitState(UNIT_STATE_ROOT))
                SetRooted(true);

            if (HasUnitState(UNIT_STATE_CONFUSED))
                SetConfused(true);
            else if (HasUnitState(UNIT_STATE_FLEEING))
                SetFeared(true);
        }
    }
}

void Unit::SetStunned(bool apply)
{
    if (apply)
    {
        // We need to stop fear on stun and root or we will get teleport to destination issue as MVMGEN for fear keeps going on		
        if (HasUnitState(UNIT_STATE_FLEEING))
            SetFeared(false);

        SetTarget(0);
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);

        // MOVEMENTFLAG_ROOT cannot be used in conjunction with MOVEMENTFLAG_MASK_MOVING (tested 3.3.5a)
        // this will freeze clients. That's why we remove MOVEMENTFLAG_MASK_MOVING before
        // setting MOVEMENTFLAG_ROOT
        RemoveUnitMovementFlag(MOVEMENTFLAG_MASK_MOVING);
        AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
        StopMoving();

        if (GetTypeId() == TYPEID_PLAYER)
            SetStandState(UNIT_STAND_STATE_STAND);

        WorldPacket data(SMSG_FORCE_MOVE_ROOT, 8);
        data << GetPackGUID();
        data << uint32(0);
        SendMessageToSet(&data, true);

        CastStop();
    }
    else
    {
        AttackStop(); //This will reupdate current victim. patch 2.4.3 : When a stun wears off, the creature that was stunned will prefer the last target with the highest threat, versus the current target. 

        if (IsAlive() && GetVictim())
            SetTarget(EnsureVictim()->GetGUID());

        // don't remove UNIT_FLAG_STUNNED for pet when owner is mounted (disabled pet's interface)
        Unit* owner = GetOwner();
        if (!owner || (owner->GetTypeId() == TYPEID_PLAYER && !owner->ToPlayer()->IsMounted()))
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);

        if (!HasUnitState(UNIT_STATE_ROOT))         // prevent moving if it also has root effect
        {
            WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 8+4);
            data << GetPackGUID();
            data << uint32(0);
            SendMessageToSet(&data, true);

            RemoveUnitMovementFlag(MOVEMENTFLAG_ROOT);
        }
    }
}

void Unit::SetRooted(bool apply)
{
    if (apply)
    {
        // We need to stop fear on stun and root or we will get teleport to destination issue as MVMGEN for fear keeps going on		
        if (HasUnitState(UNIT_STATE_FLEEING))
            SetFeared(false);

        if (m_rootTimes > 0) // blizzard internal check?
            m_rootTimes++;

        // MOVEMENTFLAG_ROOT cannot be used in conjunction with MOVEMENTFLAG_MASK_MOVING (tested 3.3.5a)
        // this will freeze clients. That's why we remove MOVEMENTFLAG_MASK_MOVING before
        // setting MOVEMENTFLAG_ROOT
        RemoveUnitMovementFlag(MOVEMENTFLAG_MASK_MOVING);
        AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
        StopMoving();

        if (GetTypeId() == TYPEID_PLAYER)
        {
            WorldPacket data(SMSG_FORCE_MOVE_ROOT, 10);
            data << GetPackGUID();
            data << m_rootTimes;
            SendMessageToSet(&data, true);
        }
        else
        {
            WorldPacket data(SMSG_SPLINE_MOVE_ROOT, 8);
            data << GetPackGUID();
            SendMessageToSet(&data, true);
        }
    }
    else
    {
        if (!HasUnitState(UNIT_STATE_STUNNED))      // prevent moving if it also has stun effect
        {
            if (GetTypeId() == TYPEID_PLAYER)
            {
                WorldPacket data(SMSG_FORCE_MOVE_UNROOT, 10);
                data << GetPackGUID();
                data << ++m_rootTimes;
                SendMessageToSet(&data, true);
            }
            else
            {
                WorldPacket data(SMSG_SPLINE_MOVE_UNROOT, 8);
                data << GetPackGUID();
                SendMessageToSet(&data, true);
            }

            RemoveUnitMovementFlag(MOVEMENTFLAG_ROOT);
        }
    }
}

void Unit::SetFeared(bool apply)
{
    if (apply)
    {
        SetTarget(0);

        Unit* caster = NULL;
        //Unit::AuraEffectList const& fearAuras = GetAuraEffectsByType(SPELL_AURA_MOD_FEAR);
        Unit::AuraList const& fearAuras = GetAurasByType(SPELL_AURA_MOD_FEAR);
        if (!fearAuras.empty())
            caster = ObjectAccessor::GetUnit(*this, fearAuras.front()->GetCasterGUID());
        if (!caster)
            caster = GetAttackerForHelper();

       GetMotionMaster()->MoveFleeing(caster, fearAuras.empty() ? sWorld->getConfig(CONFIG_CREATURE_FAMILY_FLEE_DELAY) : 0);             // caster == NULL processed in MoveFleeing
    }
    else
    {
        AttackStop();  //This will reupdate current victim. patch 2.4.3 : When a stun wears off, the creature that was stunned will prefer the last target with the highest threat, versus the current target. I'm not sure this should apply to confuse but this seems logical.

        if (IsAlive())
        {
            if (GetMotionMaster()->GetCurrentMovementGeneratorType() == FLEEING_MOTION_TYPE)
                GetMotionMaster()->MovementExpired();
            if (GetVictim())
                SetTarget(EnsureVictim()->GetGUID());
        }
    }

    if (Player* player = ToPlayer())
        player->SetClientControl(this, !apply);
}

void Unit::SetConfused(bool apply)
{
    if (apply)
    {
        SetTarget(0);
        GetMotionMaster()->MoveConfused();
    }
    else
    {
        AttackStop();  //This will reupdate current victim. patch 2.4.3 : When a stun wears off, the creature that was stunned will prefer the last target with the highest threat, versus the current target. I'm not sure this should apply to fear but this seems logical.

        if (IsAlive())
        {
            if (GetMotionMaster()->GetCurrentMovementGeneratorType() == CONFUSED_MOTION_TYPE)
                GetMotionMaster()->MovementExpired();
            if (GetVictim())
                SetTarget(EnsureVictim()->GetGUID());
        }
    }

    if (Player* player = ToPlayer())
        player->SetClientControl(this, !apply);
}
/* TODOSPELL old version
bool Unit::SetCharmedBy(Unit* charmer, CharmType type, AuraApplication const* aurApp)
{
    if (!charmer)
        return false;

    // dismount players when charmed
    if (GetTypeId() == TYPEID_PLAYER)
        RemoveAurasByType(SPELL_AURA_MOUNTED);

    if (charmer->GetTypeId() == TYPEID_PLAYER)
        charmer->RemoveAurasByType(SPELL_AURA_MOUNTED);

    ASSERT(type != CHARM_TYPE_POSSESS || charmer->GetTypeId() == TYPEID_PLAYER);
    ASSERT((type == CHARM_TYPE_VEHICLE) == IsVehicle());

    TC_LOG_DEBUG("entities.unit", "SetCharmedBy: charmer %u (GUID %u), charmed %u (GUID %u), type %u.", charmer->GetEntry(), charmer->GetGUIDLow(), GetEntry(), GetGUIDLow(), uint32(type));

    if (this == charmer)
    {
        TC_LOG_FATAL("entities.unit", "Unit::SetCharmedBy: Unit %u (GUID %u) is trying to charm itself!", GetEntry(), GetGUIDLow());
        return false;
    }

    //if (HasUnitState(UNIT_STATE_UNATTACKABLE))
    //    return false;

    if (GetTypeId() == TYPEID_PLAYER && ToPlayer()->GetTransport())
    {
        TC_LOG_FATAL("entities.unit", "Unit::SetCharmedBy: Player on transport is trying to charm %u (GUID %u)", GetEntry(), GetGUIDLow());
        return false;
    }

    // Already charmed
    if (GetCharmerGUID())
    {
        TC_LOG_FATAL("entities.unit", "Unit::SetCharmedBy: %u (GUID %u) has already been charmed but %u (GUID %u) is trying to charm it!", GetEntry(), GetGUIDLow(), charmer->GetEntry(), charmer->GetGUIDLow());
        return false;
    }

    CastStop();
    CombatStop(); /// @todo CombatStop(true) may cause crash (interrupt spells)
    DeleteThreatList();

    Player* playerCharmer = charmer->ToPlayer();

    // Charmer stop charming
    if (playerCharmer)
    {
        playerCharmer->StopCastingCharm();
        playerCharmer->StopCastingBindSight();
    }

    // Charmed stop charming
    if (GetTypeId() == TYPEID_PLAYER)
    {
        ToPlayer()->StopCastingCharm();
        ToPlayer()->StopCastingBindSight();
    }

    // StopCastingCharm may remove a possessed pet?
    if (!IsInWorld())
    {
        TC_LOG_FATAL("entities.unit", "Unit::SetCharmedBy: %u (GUID %u) is not in world but %u (GUID %u) is trying to charm it!", GetEntry(), GetGUIDLow(), charmer->GetEntry(), charmer->GetGUIDLow());
        return false;
    }

    // charm is set by aura, and aura effect remove handler was called during apply handler execution
    // prevent undefined behaviour
    if (aurApp && aurApp->GetRemoveMode())
        return false;

    _oldFactionId = getFaction();
    SetFaction(charmer->getFaction());

    // Set charmed
    charmer->SetCharm(this, true);

    if (GetTypeId() == TYPEID_UNIT)
    {
        ToCreature()->AI()->OnCharmed(true);
        GetMotionMaster()->MoveIdle();
    }
    else
    {
        Player* player = ToPlayer();
        if (player->IsAFK())
            player->ToggleAFK();
        player->SetClientControl(this, false);
    }

    // charm is set by aura, and aura effect remove handler was called during apply handler execution
    // prevent undefined behaviour
    if (aurApp && aurApp->GetRemoveMode())
        return false;

    // Pets already have a properly initialized CharmInfo, don't overwrite it.
    if (type != CHARM_TYPE_VEHICLE && !GetCharmInfo())
    {
        InitCharmInfo();
        if (type == CHARM_TYPE_POSSESS)
            GetCharmInfo()->InitPossessCreateSpells();
        else
            GetCharmInfo()->InitCharmCreateSpells();
    }

    if (playerCharmer)
    {
        switch (type)
        {
            case CHARM_TYPE_VEHICLE:
                SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
                playerCharmer->SetClientControl(this, true);
                playerCharmer->VehicleSpellInitialize();
                break;
            case CHARM_TYPE_POSSESS:
                AddUnitState(UNIT_STATE_POSSESSED);
                SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
                charmer->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                playerCharmer->SetClientControl(this, true);
                playerCharmer->PossessSpellInitialize();
                break;
            case CHARM_TYPE_CHARM:
                if (GetTypeId() == TYPEID_UNIT && charmer->getClass() == CLASS_WARLOCK)
                {
                    CreatureTemplate const* cinfo = ToCreature()->GetCreatureTemplate();
                    if (cinfo && cinfo->type == CREATURE_TYPE_DEMON)
                    {
                        // to prevent client crash
                        SetByteValue(UNIT_FIELD_BYTES_0, 1, (uint8)CLASS_MAGE);

                        // just to enable stat window
                        if (GetCharmInfo())
                            GetCharmInfo()->SetPetNumber(sObjectMgr->GeneratePetNumber(), true);

                        // if charmed two demons the same session, the 2nd gets the 1st one's name
                        SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(NULL))); // cast can't be helped
                    }
                }
                playerCharmer->CharmSpellInitialize();
                break;
            default:
            case CHARM_TYPE_CONVERT:
                break;
        }
    }
    return true;
}
*/
void Unit::SetCharmedBy(Unit* charmer, bool possess)
{
    if(!charmer)
        return;

    assert(!possess || charmer->GetTypeId() == TYPEID_PLAYER);

    if(this == charmer)
        return;

    if(IsInFlight())
        return;

    if(GetTypeId() == TYPEID_PLAYER && (this->ToPlayer())->GetTransport())
        return;

    if(ToCreature())
        ToCreature()->SetWalk(false);
    else
        RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);

    CastStop();
    CombatStop(); //TODO: CombatStop(true) may cause crash (interrupt spells)
    DeleteThreatList();
    SetEmoteState(0);
    HandleEmoteCommand(0);

    // Charmer stop charming
    if(charmer->GetTypeId() == TYPEID_PLAYER)
        (charmer->ToPlayer())->StopCastingCharm();

    // Charmed stop charming
    if(GetTypeId() == TYPEID_PLAYER)
        (this->ToPlayer())->StopCastingCharm();

    // StopCastingCharm may remove a possessed pet?
    if(!IsInWorld())
        return;

    // Set charmed
    charmer->SetCharm(this);
    SetCharmerGUID(charmer->GetGUID());
    SetFaction(charmer->GetFaction());
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

    if(GetTypeId() == TYPEID_UNIT)
    {
        (this->ToCreature())->AI()->OnCharmed(charmer, true);
        GetMotionMaster()->MoveIdle();
    }
    else
    {
        if((this->ToPlayer())->IsAFK())
            (this->ToPlayer())->ToggleAFK();
        (this->ToPlayer())->SetViewport(GetGUID(), false);
    }

    // Pets already have a properly initialized CharmInfo, don't overwrite it.
    if(GetTypeId() == TYPEID_PLAYER || (GetTypeId() == TYPEID_UNIT && !(this->ToCreature())->IsPet()))
    {
        CharmInfo *charmInfo = InitCharmInfo();
        if(possess)
            charmInfo->InitPossessCreateSpells();
        else
            charmInfo->InitCharmCreateSpells();
    }

    //Set possessed
    if(possess)
    {
        AddUnitState(UNIT_STATE_POSSESSED);
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
        AddPlayerToVision(charmer->ToPlayer());
        (charmer->ToPlayer())->SetViewport(GetGUID(), true);
        charmer->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
    }
    // Charm demon
    else if(GetTypeId() == TYPEID_UNIT && charmer->GetTypeId() == TYPEID_PLAYER && charmer->GetClass() == CLASS_WARLOCK)
    {
        CreatureTemplate const *cinfo = (this->ToCreature())->GetCreatureTemplate();
        if(cinfo && cinfo->type == CREATURE_TYPE_DEMON)
        {
            //to prevent client crash
            SetFlag(UNIT_FIELD_BYTES_0, 2048);

            //just to enable stat window
            if(GetCharmInfo())
                GetCharmInfo()->SetPetNumber(sObjectMgr->GeneratePetNumber(), true);

            //if charmed two demons the same session, the 2nd gets the 1st one's name
            SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(NULL));
        }
    }

    if(possess)
        (charmer->ToPlayer())->PossessSpellInitialize();
    else if(charmer->GetTypeId() == TYPEID_PLAYER)
        (charmer->ToPlayer())->CharmSpellInitialize();
}

/* TODOSPELL old version
void Unit::RemoveCharmedBy(Unit* charmer)
{
    if (!IsCharmed())
        return;

    if (!charmer)
        charmer = GetCharmer();
    if (charmer != GetCharmer()) // one aura overrides another?
    {
//        TC_LOG_FATAL("entities.unit", "Unit::RemoveCharmedBy: this: " UI64FMTD " true charmer: " UI64FMTD " false charmer: " UI64FMTD,
//            GetGUID(), GetCharmerGUID(), charmer->GetGUID());
//        ABORT();
        return;
    }

    CharmType type;
    if (HasUnitState(UNIT_STATE_POSSESSED))
        type = CHARM_TYPE_POSSESS;
    else if (charmer && charmer->IsOnVehicle(this))
        type = CHARM_TYPE_VEHICLE;
    else
        type = CHARM_TYPE_CHARM;

    CastStop();
    CombatStop(); /// @todo CombatStop(true) may cause crash (interrupt spells)
    GetHostileRefManager().deleteReferences();
    DeleteThreatList();

    if (_oldFactionId)
    {
        SetFaction(_oldFactionId);
        _oldFactionId = 0;
    }
    else
        RestoreFaction();

    GetMotionMaster()->InitDefault();

    if (Creature* creature = ToCreature())
    {
        // Creature will restore its old AI on next update
        if (creature->AI())
            creature->AI()->OnCharmed(false);

        // Vehicle should not attack its passenger after he exists the seat
        if (type != CHARM_TYPE_VEHICLE)
            LastCharmerGUID = charmer->GetGUID();
    }

    // If charmer still exists
    if (!charmer)
        return;

    ASSERT(type != CHARM_TYPE_POSSESS || charmer->GetTypeId() == TYPEID_PLAYER);
    ASSERT(type != CHARM_TYPE_VEHICLE || (GetTypeId() == TYPEID_UNIT && IsVehicle()));

    charmer->SetCharm(this, false);

    Player* playerCharmer = charmer->ToPlayer();

    if (playerCharmer)
    {
        switch (type)
        {
            case CHARM_TYPE_VEHICLE:
                playerCharmer->SetClientControl(this, false);
                playerCharmer->SetClientControl(charmer, true);
                RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
                break;
            case CHARM_TYPE_POSSESS:
                playerCharmer->SetClientControl(this, false);
                playerCharmer->SetClientControl(charmer, true);
                charmer->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
                ClearUnitState(UNIT_STATE_POSSESSED);
                break;
            case CHARM_TYPE_CHARM:
                if (GetTypeId() == TYPEID_UNIT && charmer->getClass() == CLASS_WARLOCK)
                {
                    CreatureTemplate const* cinfo = ToCreature()->GetCreatureTemplate();
                    if (cinfo && cinfo->type == CREATURE_TYPE_DEMON)
                    {
                        SetByteValue(UNIT_FIELD_BYTES_0, 1, uint8(cinfo->unit_class));
                        if (GetCharmInfo())
                            GetCharmInfo()->SetPetNumber(0, true);
                        else
                            TC_LOG_ERROR("entities.unit", "Aura::HandleModCharm: target=" UI64FMTD " with typeid=%d has a charm aura but no charm info!", GetGUID(), GetTypeId());
                    }
                }
                break;
            case CHARM_TYPE_CONVERT:
                break;
        }
    }

    if (Player* player = ToPlayer())
        player->SetClientControl(this, true);

    // a guardian should always have charminfo
    if (playerCharmer && this != charmer->GetFirstControlled())
        playerCharmer->SendRemoveControlBar();
    else if (GetTypeId() == TYPEID_PLAYER || (GetTypeId() == TYPEID_UNIT && !ToCreature()->IsGuardian()))
        DeleteCharmInfo();
}
*/
void Unit::RemoveCharmedBy(Unit *charmer)
{
    if(!IsCharmed())
        return;

    if(!charmer)
        charmer = GetCharmer();
    else if(charmer != GetCharmer()) // one aura overrides another?
        return;

    bool possess = HasUnitState(UNIT_STATE_POSSESSED);

    CastStop();
    CombatStop(); //TODO: CombatStop(true) may cause crash (interrupt spells)
    GetHostileRefManager().deleteReferences();
    DeleteThreatList();
    SetCharmerGUID(0);
    RestoreFaction();
    GetMotionMaster()->InitDefault();
    //reset emote state
    if(Creature* c = ToCreature())
        c->ResetCreatureEmote();
    
    HandleEmoteCommand(0);

    if(possess)
    {
        ClearUnitState(UNIT_STATE_POSSESSED);
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
    }

    if(GetTypeId() == TYPEID_UNIT)
    {
        if(!(this->ToCreature())->IsPet())
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

        (this->ToCreature())->AI()->OnCharmed(charmer, false);
        if(IsAlive() && (this->ToCreature())->IsAIEnabled)
        {
            if(charmer && !IsFriendlyTo(charmer))
            {
                (this->ToCreature())->AddThreat(charmer, 10000.0f);
                (this->ToCreature())->AI()->AttackStart(charmer);
            }
            else {
                (this->ToCreature())->AI()->EnterEvadeMode();
            }
        }
    }
    else
        (this->ToPlayer())->SetViewport(GetGUID(), true);

    // If charmer still exists
    if(!charmer)
        return;

    assert(!possess || charmer->GetTypeId() == TYPEID_PLAYER);

    charmer->SetCharm(0);
    if(possess)
    {
        RemovePlayerFromVision(charmer->ToPlayer());
        (charmer->ToPlayer())->SetViewport(charmer->GetGUID(), true);
        charmer->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
    }
    // restore UNIT_FIELD_BYTES_0
    else if(GetTypeId() == TYPEID_UNIT && charmer->GetTypeId() == TYPEID_PLAYER && charmer->GetClass() == CLASS_WARLOCK)
    {
        CreatureTemplate const *cinfo = (this->ToCreature())->GetCreatureTemplate();
        if(cinfo && cinfo->type == CREATURE_TYPE_DEMON)
        {
            CreatureAddon const *cainfo = (this->ToCreature())->GetCreatureAddon();
            if(cainfo && cainfo->bytes0 != 0)
                SetUInt32Value(UNIT_FIELD_BYTES_0, cainfo->bytes0);
            else
                RemoveFlag(UNIT_FIELD_BYTES_0, 2048);

            if(GetCharmInfo())
                GetCharmInfo()->SetPetNumber(0, true);
            else
                TC_LOG_ERROR("spell.aura","Aura::HandleModCharm: target=" UI64FMTD " with typeid=%d has a charm aura but no charm info!", GetGUID(), GetTypeId());
        }
    }

    if(GetTypeId() == TYPEID_PLAYER || (GetTypeId() == TYPEID_UNIT && !(this->ToCreature())->IsPet()))
    {
        DeleteCharmInfo();
    }

    if(possess || charmer->GetTypeId() == TYPEID_PLAYER)
    {
        // Remove pet spell action bar
        WorldPacket data(SMSG_PET_SPELLS, 8);
        data << uint64(0);
        (charmer->ToPlayer())->SendDirectMessage(&data);
    }
}

void Unit::RestoreFaction()
{
    if(GetTypeId() == TYPEID_PLAYER)
        (this->ToPlayer())->SetFactionForRace(GetRace());
    else
    {
        CreatureTemplate const *cinfo = (this->ToCreature())->GetCreatureTemplate();

        if((this->ToCreature())->IsPet())
        {
            if(Unit* owner = GetOwner())
                SetFaction(owner->GetFaction());
            else if(cinfo)
                SetFaction(cinfo->faction);
        }
        else if(cinfo)  // normal creature
            SetFaction(cinfo->faction);
    }
}

bool Unit::IsInPartyWith(Unit const *unit) const
{
    if(this == unit)
        return true;

    const Unit *u1 = GetCharmerOrOwnerOrSelf();
    const Unit *u2 = unit->GetCharmerOrOwnerOrSelf();
    if(u1 == u2)
        return true;

    if(u1->GetTypeId() == TYPEID_PLAYER && u2->GetTypeId() == TYPEID_PLAYER)
        return (u1->ToPlayer())->IsInSameGroupWith(u2->ToPlayer());
    else
        return false;
}

bool Unit::IsInRaidWith(Unit const *unit) const
{
    if(this == unit)
        return true;

    const Unit *u1 = GetCharmerOrOwnerOrSelf();
    const Unit *u2 = unit->GetCharmerOrOwnerOrSelf();
    if(u1 == u2)
        return true;

    if(u1->GetTypeId() == TYPEID_PLAYER && u2->GetTypeId() == TYPEID_PLAYER)
        return (u1->ToPlayer())->IsInSameRaidWith(u2->ToPlayer());
    else
        return false;
}

void Unit::GetRaidMember(std::list<Unit*> &nearMembers, float radius)
{
    Player *owner = GetCharmerOrOwnerPlayerOrPlayerItself();
    if(!owner)
        return;

    Group *pGroup = owner->GetGroup();
    if(!pGroup)
        return;

    for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* Target = itr->GetSource();

        // IsHostileTo check duel and controlled by enemy
        if( Target && Target != this && Target->IsAlive()
            && IsWithinDistInMap(Target, radius) && !IsHostileTo(Target) )
            nearMembers.push_back(Target);
    }
}

void Unit::GetPartyMember(std::list<Unit*> &TagUnitMap, float radius)
{
    Unit *owner = GetCharmerOrOwnerOrSelf();
    Group *pGroup = NULL;
    if (owner->GetTypeId() == TYPEID_PLAYER)
        pGroup = (owner->ToPlayer())->GetGroup();

    if(pGroup)
    {
        uint8 subgroup = (owner->ToPlayer())->GetSubGroup();

        for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* Target = itr->GetSource();

            // IsHostileTo check duel and controlled by enemy
            if( Target && Target->GetSubGroup()==subgroup && !IsHostileTo(Target) )
            {
                if(Target->IsAlive() && IsWithinDistInMap(Target, radius) )
                    TagUnitMap.push_back(Target);

                if(Pet* pet = Target->GetPet())
                    if(pet->IsAlive() &&  IsWithinDistInMap(pet, radius) )
                        TagUnitMap.push_back(pet);
            }
        }
    }
    else
    {
        if(owner->IsAlive() && (owner == this || IsWithinDistInMap(owner, radius)))
            TagUnitMap.push_back(owner);
        if(Pet* pet = owner->GetPet())
            if(pet->IsAlive() && (pet == this && IsWithinDistInMap(pet, radius)))
                TagUnitMap.push_back(pet);
    }
}

void Unit::AddAura(uint32 spellId, Unit* target)
{
    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if(!spellInfo)
        return;
        
    if(!target || (!target->IsAlive() && !(spellInfo->Attributes & SPELL_ATTR0_CASTABLE_WHILE_DEAD)))
        return;

    if (target->IsImmunedToSpell(spellInfo))
        return;
        
    for(uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if(spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA)
        {
            if(target->IsImmunedToSpellEffect(spellInfo->Effects[i].Effect, spellInfo->Effects[i].Mechanic))
                continue;

            /*if(spellInfo->Effects[i].TargetA.GetTarget()== TARGET_UNIT_CASTER)
            {
                Aura *Aur = CreateAura(spellInfo, i, NULL, this, this);
                AddAura(Aur);
            }
            else*/
            {
                Aura *Aur = CreateAura(spellInfo, i, NULL, target, this);
                target->AddAura(Aur);
            }
        }
    }
}

Unit* Unit::GetRedirectThreatTarget() 
{ 
    return m_misdirectionTargetGUID ? ObjectAccessor::GetUnit(*this, m_misdirectionTargetGUID) : NULL; 
}
Unit* Unit::GetLastRedirectTarget() 
{ 
    return m_misdirectionLastTargetGUID ? ObjectAccessor::GetUnit(*this, m_misdirectionLastTargetGUID) : NULL; 
}

Unit* Unit::GetSummoner() const
{ 
    return m_summoner ? ObjectAccessor::GetUnit(*this, m_summoner) : NULL; 
}

Creature* Unit::FindCreatureInGrid(uint32 entry, float range, bool isAlive)
{
    Creature* pCreature = NULL;

    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*this, entry, isAlive, range);
    Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(pCreature, creature_check);

    TypeContainerVisitor<Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck>, GridTypeMapContainer> creature_searcher(searcher);

    cell.Visit(pair, creature_searcher, *GetMap());
    
    return pCreature;
}

GameObject* Unit::FindGOInGrid(uint32 entry, float range)
{
    GameObject* pGo = NULL;

    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Trinity::NearestGameObjectEntryInObjectRangeCheck go_check(*this, entry, range);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> searcher(this, pGo, go_check);

    TypeContainerVisitor<Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck>, GridTypeMapContainer> go_searcher(searcher);

    cell.Visit(pair, go_searcher, *GetMap());
    
    return pGo;
}

void Unit::SetFullTauntImmunity(bool apply)
{
    ApplySpellImmune(0, IMMUNITY_ID, 31789, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 39377, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 54794, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 37017, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 37486, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 49613, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 694, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 25266, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 39270, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 27344, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 6795, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 39270, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 1161, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 5209, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 355, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 34105, apply);
    ApplySpellImmune(0, IMMUNITY_ID, 53477, apply);
}

// From MaNGOS
bool Unit::CanReachWithMeleeAttack(Unit* pVictim, float flat_mod /*= 0.0f*/) const
{
    if (!pVictim)
        return false;

    // The measured values show BASE_MELEE_OFFSET in (1.3224, 1.342)
    float reach = GetFloatValue(UNIT_FIELD_COMBATREACH) + pVictim->GetFloatValue(UNIT_FIELD_COMBATREACH) +
        1.33f + flat_mod;

    if (reach < ATTACK_DISTANCE)
        reach = ATTACK_DISTANCE;

    // This check is not related to bounding radius
    float dx = GetPositionX() - pVictim->GetPositionX();
    float dy = GetPositionY() - pVictim->GetPositionY();
    float dz = GetPositionZ() - pVictim->GetPositionZ();

    return dx*dx + dy*dy + dz*dz < reach*reach;
}

bool Unit::IsCCed() const
{
    return (IsAlive() && (IsFeared() || IsCharmed() || HasUnitState(UNIT_STATE_STUNNED) || HasUnitState(UNIT_STATE_CONFUSED)));
}

////////////////////////////////////////////////////////////
// Methods of class GlobalCooldownMgr

bool GlobalCooldownMgr::HasGlobalCooldown(SpellInfo const* spellInfo) const
{
    GlobalCooldownList::const_iterator itr = m_GlobalCooldowns.find(spellInfo->StartRecoveryCategory);
    return itr != m_GlobalCooldowns.end() && itr->second.duration && GetMSTimeDiff(itr->second.cast_time, GetMSTime()) < itr->second.duration;
}

void GlobalCooldownMgr::AddGlobalCooldown(SpellInfo const* spellInfo, uint32 gcd)
{
    m_GlobalCooldowns[spellInfo->StartRecoveryCategory] = GlobalCooldown(gcd, GetMSTime());
}

void GlobalCooldownMgr::CancelGlobalCooldown(SpellInfo const* spellInfo)
{
    m_GlobalCooldowns[spellInfo->StartRecoveryCategory].duration = 0;
}

bool Unit::HasAuraWithMechanic(Mechanics mechanic) const
{
    AuraMap const &auras = GetAuras();
    for(AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
        if(SpellInfo const *iterSpellProto = itr->second->GetSpellInfo())
            if(iterSpellProto->Mechanic == mechanic)
                return true;
    return false;
}

void Unit::RestoreDisplayId()
{
    Aura* handledAura = NULL;
    // try to receive model from transform auras
    Unit::AuraList const& transforms = GetAurasByType(SPELL_AURA_TRANSFORM);
    if (!transforms.empty())
    {
        // iterate over already applied transform auras - from newest to oldest
        for (Unit::AuraList::const_reverse_iterator i = transforms.rbegin(); i != transforms.rend(); ++i)
        {
            if (!handledAura)
                handledAura = (*i);
            // prefer negative auras
            if(!(*i)->GetSpellInfo()->IsPositive())
            {
                handledAura = (*i);
                break;
            }
        }
    }
    
    // transform aura was found
    if (handledAura)
    {
        //unapply (this is still active so can't be reapplied without unapplying first) then re apply
        handledAura->ApplyModifier(false);
        handledAura->ApplyModifier(true);
    }

    // no transform aura found, check for shapeshift
    else if (uint32 modelId = GetModelForForm(GetShapeshiftForm()))
        SetDisplayId(modelId);
    // nothing found - set modelid to default
    else
    {
        SetDisplayId(GetNativeDisplayId());
        SetTransForm(0);
    }
}

uint32 Unit::GetModelForForm(ShapeshiftForm form) const
{
    //set different model the first april
    time_t t = time(NULL);
	tm* timePtr = localtime(&t);
    bool firstApril = timePtr->tm_mon == 3 && timePtr->tm_mday == 1;

    uint32 modelid = 0;
    switch(form)
    {
        case FORM_CAT:
            if(Player::TeamForRace(GetRace()) == TEAM_ALLIANCE)
                modelid = firstApril ? 729 : 892;
            else
                modelid = firstApril ? 657 : 8571;
            break;
        case FORM_TRAVEL:
            if(firstApril)
                modelid = GetGender() == GENDER_FEMALE ? (rand()%2 ? 1547 : 18406) : 1917;
            else
                modelid = 632;
            break;
        case FORM_AQUA:
            modelid = firstApril ? 4591 : 2428;
            break;
        case FORM_GHOUL:
            if(Player::TeamForRace(GetRace()) == TEAM_ALLIANCE)
                modelid = 10045;
            break;
        case FORM_BEAR:
        case FORM_DIREBEAR:
            if(Player::TeamForRace(GetRace()) == TEAM_ALLIANCE)
                modelid = firstApril ? 865 : 2281;
            else
                modelid = firstApril ? 706 : 2289;
            break;
        case FORM_CREATUREBEAR:
            modelid = 902;
            break;
        case FORM_GHOSTWOLF:
            modelid = firstApril ? 1531 : 4613;
            break;
        case FORM_FLIGHT:
            if(Player::TeamForRace(GetRace()) == TEAM_ALLIANCE)
                modelid = firstApril ? 9345 : 20857;
            else
                modelid = firstApril ? 9345 : 20872;
            break;
        case FORM_MOONKIN:
            if(Player::TeamForRace(GetRace()) == TEAM_ALLIANCE)
                modelid = firstApril ? 17034 : 15374;
            else
                modelid = firstApril ? 17034 : 15375;
            break;
        case FORM_FLIGHT_EPIC:
            if(Player::TeamForRace(GetRace()) == TEAM_ALLIANCE)
                modelid = firstApril ? 6212 : 21243;
            else
                modelid = firstApril ? 19259 : 21244;
            break;
        case FORM_TREE:
            modelid = firstApril ? ( GetGender() == GENDER_FEMALE ? 17340 : 2432) : 864;
            break;
        case FORM_SPIRITOFREDEMPTION:
            modelid = 16031;
            break;
        case FORM_AMBIENT:
        case FORM_SHADOW:
        case FORM_STEALTH:
        case FORM_BATTLESTANCE:
        case FORM_BERSERKERSTANCE:
        case FORM_DEFENSIVESTANCE:
        case FORM_NONE:
            break;
        default:
            TC_LOG_ERROR("FIXME","Unit::GetModelForForm : Unknown Shapeshift Type: %u", form);
    }

    return modelid;
}

bool Unit::IsSpellDisabled(uint32 const spellId)
{
    if(GetTypeId() == TYPEID_PLAYER)
    {
        if(sObjectMgr->IsPlayerSpellDisabled(spellId))
            return true;
    }
    else if (GetTypeId() == TYPEID_UNIT && (ToCreature())->IsPet())
    {
        if(sObjectMgr->IsPetSpellDisabled(spellId))
            return true;
    }
    else
    {
        if(sObjectMgr->IsCreatureSpellDisabled(spellId))
            return true;
    }
    return false;
}

void Unit::HandleParryRush()
{
    if(ToCreature() && ToCreature()->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_PARRY_RUSH)
        return;

    uint32 timeLeft = GetAttackTimer(BASE_ATTACK);
    uint32 attackTime = GetAttackTime(BASE_ATTACK);

    int newAttackTime = timeLeft - (int)(0.4*attackTime);
    float newPercentTimeLeft = newAttackTime / (float)attackTime;
    if(newPercentTimeLeft < 0.2)
        SetAttackTimer(BASE_ATTACK, (uint32)(0.2*attackTime) ); //20% floor
    else
        SetAttackTimer(BASE_ATTACK, (int)newAttackTime );
}

bool Unit::SetWalk(bool enable)
{
    if (enable == IsWalking())
        return false;

    if (enable)
        AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
    else
        RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);

    return true;
}

bool Unit::SetDisableGravity(bool disable, bool /*packetOnly = false*/)
{
    if (disable)
        RemoveUnitMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING);

    if (disable == IsLevitating())
        return false;

    if (disable)
    {
        AddUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
    }
    else
    {
        RemoveUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
        if (!HasUnitMovementFlag(MOVEMENTFLAG_CAN_FLY))
        {
            m_movementInfo.SetFallTime(0);
            AddUnitMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING);
        }
    }

    return true;
}

bool Unit::SetSwim(bool enable)
{
    if (enable == HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING))
        return false;

    if (enable)
        AddUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
    else
        RemoveUnitMovementFlag(MOVEMENTFLAG_SWIMMING);

    return true;
}

bool Unit::SetFlying(bool enable, bool packetOnly /* = false */)
{
    if(enable)    
        RemoveUnitMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING);

    if (enable == CanFly())
        return false;

    if (enable)
    {
        AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY);
    }
    else
    {
        RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_MASK_MOVING_FLY);
        if (!IsLevitating())
        {
            m_movementInfo.SetFallTime(0);
            AddUnitMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING);
        }
    }

    return true;
}

bool Unit::SetWaterWalking(bool enable, bool /*packetOnly = false */)
{
    if (enable == HasUnitMovementFlag(MOVEMENTFLAG_WATERWALKING))
        return false;

    if (enable)
        AddUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);
    else
        RemoveUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);

    return true;
}

bool Unit::SetFeatherFall(bool enable, bool /*packetOnly = false */)
{
    if (enable == HasUnitMovementFlag(MOVEMENTFLAG_FALLING_SLOW))
        return false;

    if (enable)
        AddUnitMovementFlag(MOVEMENTFLAG_FALLING_SLOW);
    else
        RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING_SLOW);

    return true;
}

bool Unit::SetHover(bool enable, bool /*packetOnly = false*/)
{
    if (enable == HasUnitMovementFlag(MOVEMENTFLAG_HOVER))
        return false;

    float hoverHeight = UNIT_DEFAULT_HOVERHEIGHT;

    if (enable)
    {
        //! No need to check height on ascent
        AddUnitMovementFlag(MOVEMENTFLAG_HOVER);
        if (hoverHeight)
            UpdateHeight(GetPositionZ() + hoverHeight);
    }
    else
    {
        RemoveUnitMovementFlag(MOVEMENTFLAG_HOVER);
        if (hoverHeight)
        {
            float newZ = GetPositionZ() - hoverHeight;
            UpdateAllowedPositionZ(GetPositionX(), GetPositionY(), newZ);
            UpdateHeight(newZ);
        }
    }

    return true;
}

void Unit::SetInFront(WorldObject const* target)
{
    if (!HasUnitState(UNIT_STATE_CANNOT_TURN))
        SetOrientation(GetAngle(target));
}

void Unit::SetInFront(float x, float y)
{
    if(!HasUnitState(UNIT_STATE_CANNOT_TURN) && !IsUnitRotating()) 
        SetOrientation(GetAngle(x,y));
}

bool Unit::HandleSpellClick(Unit* clicker, int8 seatId)
{
    //TODO LK vehicules

    bool result = true;

    Creature* creature = ToCreature();
    if (creature && creature->IsAIEnabled)
        creature->AI()->OnSpellClick(clicker, result);

    return result;
}

void Unit::BuildMovementPacket(ByteBuffer *data) const
{
    *data << uint32(GetUnitMovementFlags());            // movement flags
#ifdef LICH_KING
    *data << uint16(GetExtraUnitMovementFlags());
#else
    *data << uint8(0);                                  // 2.3.0, always set to 0
#endif
    *data << uint32(GetMSTime());                       // time / counter
    *data << GetPositionX();
    *data << GetPositionY();
    *data << GetPositionZMinusOffset();
    *data << GetOrientation();

    // 0x00000200
    if (GetUnitMovementFlags() & MOVEMENTFLAG_ONTRANSPORT)
    {
        *data << (uint64)GetTransport()->GetGUID();
        *data << float (GetTransOffsetX());
        *data << float (GetTransOffsetY());
        *data << float (GetTransOffsetZ());
        *data << float (GetTransOffsetO());
        *data << uint32(GetTransTime());
    }

    // 0x02200000
    if ((GetUnitMovementFlags() & (MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_PLAYER_FLYING)))
        *data << (float)m_movementInfo.pitch;

    *data << (uint32)m_movementInfo.fallTime;

    // 0x00001000
    if (GetUnitMovementFlags() & MOVEMENTFLAG_JUMPING_OR_FALLING)
    {
        *data << (float)m_movementInfo.jump.zspeed;
        *data << (float)m_movementInfo.jump.sinAngle;
        *data << (float)m_movementInfo.jump.cosAngle;
        *data << (float)m_movementInfo.jump.xyspeed;
    }

    // 0x04000000
    if (GetUnitMovementFlags() & MOVEMENTFLAG_SPLINE_ELEVATION)
        *data << (float)m_movementInfo.splineElevation;
}

void Unit::KnockbackFrom(float x, float y, float speedXY, float speedZ)
{
    Player* player = ToPlayer();
    if (!player)
    {
        if (Unit* charmer = GetCharmer())
        {
            player = charmer->ToPlayer();
            if (player && player->m_mover != this)
                player = NULL;
        }
    }

    if (!player)
    {
        GetMotionMaster()->MoveKnockbackFrom(x, y, speedXY, speedZ);
    }
    else
    {
        float vcos, vsin;
        GetSinCos(x, y, vsin, vcos);

        WorldPacket data(SMSG_MOVE_KNOCK_BACK, (8+4+4+4+4+4));
        data << GetPackGUID();
        data << uint32(0);                                      // counter
        data << float(vcos);                                    // x direction
        data << float(vsin);                                    // y direction
        data << float(speedXY);                                 // Horizontal speed
        data << float(-speedZ);                                 // Z Movement speed (vertical)

        player->SendDirectMessage(&data);

        if (player->HasAuraType(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED) || player->HasAuraType(SPELL_AURA_FLY))
            player->SetFlying(true, true);
    }
}


void Unit::JumpTo(float speedXY, float speedZ, bool forward)
{
    float angle = forward ? 0 : M_PI;
    if (GetTypeId() == TYPEID_UNIT)
        GetMotionMaster()->MoveJumpTo(angle, speedXY, speedZ);
    else
    {
        float vcos = std::cos(angle+GetOrientation());
        float vsin = std::sin(angle+GetOrientation());

        WorldPacket data(SMSG_MOVE_KNOCK_BACK, (8+4+4+4+4+4));
        data << GetPackGUID();
        data << uint32(0);                                      // Sequence
        data << float(vcos);                                    // x direction
        data << float(vsin);                                    // y direction
        data << float(speedXY);                                 // Horizontal speed
        data << float(-speedZ);                                 // Z Movement speed (vertical)

        ToPlayer()->SendDirectMessage(&data);
    }
}

void Unit::JumpTo(WorldObject* obj, float speedZ)
{
    float x, y, z;
    obj->GetContactPoint(this, x, y, z);
    float speedXY = GetExactDist2d(x, y) * 10.0f / speedZ;
    GetMotionMaster()->MoveJump(x, y, z, speedXY, speedZ);
}

bool Unit::IsFalling() const
{
    return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING | MOVEMENTFLAG_FALLING_FAR) || movespline->isFalling();
}

void Unit::NearTeleportTo(float x, float y, float z, float orientation, bool casting /*= false*/)
{
    DisableSpline();
    if (GetTypeId() == TYPEID_PLAYER)
        ToPlayer()->TeleportTo(GetMapId(), x, y, z, orientation, TELE_TO_NOT_LEAVE_TRANSPORT | TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET | (casting ? TELE_TO_SPELL : 0));
    else
    {
        Position pos = {x, y, z, orientation};
        SendTeleportPacket(pos);
        UpdatePosition(x, y, z, orientation, true);
        ObjectAccessor::UpdateObjectVisibility(this);
    }
}

void Unit::SendTeleportPacket(Position& pos)
{
    Position oldPos = { GetPositionX(), GetPositionY(), GetPositionZMinusOffset(), GetOrientation() };
    if (GetTypeId() == TYPEID_UNIT)
        Relocate(&pos);

    WorldPacket data2(MSG_MOVE_TELEPORT, 38);
    data2 << GetPackGUID();
    BuildMovementPacket(&data2);
    if (GetTypeId() == TYPEID_UNIT)
        Relocate(&oldPos);
    if (GetTypeId() == TYPEID_PLAYER)
        Relocate(&pos);
    SendMessageToSet(&data2, false);
}

bool Unit::UpdatePosition(float x, float y, float z, float orientation, bool teleport)
{
    // prevent crash when a bad coord is sent by the client
    if (!Trinity::IsValidMapCoord(x, y, z, orientation))
    {
        TC_LOG_DEBUG("entities.unit", "Unit::UpdatePosition(%f, %f, %f) .. bad coordinates!", x, y, z);
        return false;
    }

    bool turn = (GetOrientation() != orientation);
    bool relocated = (teleport || GetPositionX() != x || GetPositionY() != y || GetPositionZ() != z);

    // TODO: Check if orientation transport offset changed instead of only global orientation
    if (turn)
        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TURNING);

    if (relocated)
    {
        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_MOVE);

        // move and update visible state if need
        if (GetTypeId() == TYPEID_PLAYER)
            GetMap()->PlayerRelocation(ToPlayer(), x, y, z, orientation);
        else
            GetMap()->CreatureRelocation(ToCreature(), x, y, z, orientation);
    }
    else if (turn)
        UpdateOrientation(orientation);

    // code block for underwater state update
    UpdateUnderwaterState(GetMap(), x, y, z);

    if(GetTypeId() == TYPEID_PLAYER)
        ToPlayer()->CheckAreaExploreAndOutdoor();

    return (relocated || turn);
}

bool Unit::UpdatePosition(const Position &pos, bool teleport)
{
    return UpdatePosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), teleport);
}

//! Only server-side orientation update, does not broadcast to client
void Unit::UpdateOrientation(float orientation)
{
    SetOrientation(orientation);
}

//! Only server-side height update, does not broadcast to client
void Unit::UpdateHeight(float newZ)
{
    Relocate(GetPositionX(), GetPositionY(), newZ);
}

class SplineHandler
{
public:
    SplineHandler(Unit* unit) : _unit(unit) { }

    bool operator()(Movement::MoveSpline::UpdateResult result)
    {
        auto motionType = _unit->GetMotionMaster()->GetCurrentMovementGeneratorType();
        if ((result & (Movement::MoveSpline::Result_NextSegment | Movement::MoveSpline::Result_JustArrived)) 
            && _unit->GetTypeId() == TYPEID_UNIT 
            && (motionType == WAYPOINT_MOTION_TYPE)
            && _unit->movespline->GetId() == _unit->GetMotionMaster()->GetCurrentSplineId())
        {
            Creature* creature = _unit->ToCreature();
            if (creature)
            {
                auto moveGenerator = static_cast<WaypointMovementGenerator<Creature>*>(creature->GetMotionMaster()->top());
                moveGenerator->SplineFinished(creature, creature->movespline->currentPathIdx());

                //warn formation of leader movement if needed. atm members don't use spline movement and move point to point.
                if (result & Movement::MoveSpline::Result_NextSegment)
                {
                    if (creature && creature->GetFormation() && creature->GetFormation()->getLeader() == creature)
                    {
                        Position dest;
                        if (moveGenerator->GetCurrentDestinationPoint(creature, dest))
                            creature->GetFormation()->LeaderMoveTo(dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), !creature->IsWalking());
                    }
                }
            }
        }

        return true;
    }

private:
    Unit* _unit;
};

void Unit::UpdateSplineMovement(uint32 t_diff)
{
    if (movespline->Finalized())
        return;

    // this code cant be placed inside WaypointMovementGenerator, because we cant delete active MoveGen while it is updated
    SplineHandler handler(this);
    movespline->updateState(t_diff, handler);
    if (!movespline->Initialized())
    {
        DisableSpline();
        return;
    }

    bool arrived = movespline->Finalized();

    if (arrived)
        DisableSpline();

    m_movesplineTimer.Update(t_diff);
    if (m_movesplineTimer.Passed() || arrived)
        UpdateSplinePosition();
}

void Unit::UpdateSplinePosition()
{
    static uint32 const positionUpdateDelay = 400;

    m_movesplineTimer.Reset(positionUpdateDelay);
    Movement::Location loc = movespline->ComputePosition();

    if (movespline->onTransport)
    {
        Position& pos = m_movementInfo.transport.pos;
        pos.m_positionX = loc.x;
        pos.m_positionY = loc.y;
        pos.m_positionZ = loc.z;
        pos.m_orientation = loc.orientation;

        if (TransportBase* transport = GetTransport())
            transport->CalculatePassengerPosition(loc.x, loc.y, loc.z, &loc.orientation);
    }

    // Xinef: this is bullcrap, if we had spline running update orientation along with position
    //if (HasUnitState(UNIT_STATE_CANNOT_TURN))
    //    loc.orientation = GetOrientation();

 //   if (GetTypeId() == TYPEID_PLAYER)
        UpdatePosition(loc.x, loc.y, loc.z, loc.orientation);
   // else
     //   ToCreature()->SetPosition(loc.x, loc.y, loc.z, loc.orientation);
}

void Unit::DisableSpline()
{
    m_movementInfo.RemoveMovementFlag(MovementFlags(MOVEMENTFLAG_SPLINE_ENABLED|MOVEMENTFLAG_FORWARD));
    movespline->_Interrupt();
}

bool Unit::IsPossessedByPlayer() const
{
    return HasUnitState(UNIT_STATE_POSSESSED) && IS_PLAYER_GUID(GetCharmerGUID());
}

bool Unit::IsPossessing(Unit* u) const
{
    return u->IsPossessed() && GetCharmGUID() == u->GetGUID();
}

bool Unit::IsPossessing() const
{
    if (Unit* u = GetCharm())
        return u->IsPossessed();
    else
        return false;
}

bool Unit::CanFreeMove() const
{
    return !HasUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING | UNIT_STATE_IN_FLIGHT |
        UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED) && GetOwnerGUID() == 0;
}

void Unit::SetFacingTo(float ori)
{
    Movement::MoveSplineInit init(this);
    init.MoveTo(GetPositionX(), GetPositionY(), GetPositionZMinusOffset(), false);
    if (HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && GetTransGUID())
        init.DisableTransportPathTransformations(); // It makes no sense to target global orientation
    init.SetFacing(ori);
    init.Launch();
}

void Unit::SetFacingToObject(WorldObject const* object)
{
    // never face when already moving
    if (!IsStopped())
        return;

    /// @todo figure out under what conditions creature will move towards object instead of facing it where it currently is.
    Movement::MoveSplineInit init(this);
    init.MoveTo(GetPositionX(), GetPositionY(), GetPositionZMinusOffset());
    init.SetFacing(GetAngle(object));   // when on transport, GetAngle will still return global coordinates (and angle) that needs transforming
    init.Launch();
}

float Unit::GetPositionZMinusOffset() const
{
    float offset = 0.0f;
    if (HasUnitMovementFlag(MOVEMENTFLAG_HOVER))
        offset = UNIT_DEFAULT_HOVERHEIGHT;

    return GetPositionZ() - offset;
}

bool Unit::IsInDisallowedMountForm() const
{
    if (SpellInfo const* transformSpellInfo = sSpellMgr->GetSpellInfo(GetTransForm()))
        if (transformSpellInfo->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_MOUNTED))
            return false;

    if (ShapeshiftForm form = GetShapeshiftForm())
    {
        SpellShapeshiftEntry const* shapeshift = sSpellShapeshiftStore.LookupEntry(form);
        if (!shapeshift)
            return true;

        if (!(shapeshift->flags1 & 0x1))
            return true;
    }

    if (GetDisplayId() == GetNativeDisplayId())
        return false;

    CreatureDisplayInfoEntry const* display = sCreatureDisplayInfoStore.LookupEntry(GetDisplayId());
    if (!display)
        return true;

    CreatureDisplayInfoExtraEntry const* displayExtra = sCreatureDisplayInfoExtraStore.LookupEntry(display->ExtraId);
    if (!displayExtra)
        return true;

    CreatureModelDataEntry const* model = sCreatureModelDataStore.LookupEntry(display->ModelId);
    ChrRacesEntry const* race = sChrRacesStore.LookupEntry(displayExtra->Race);

    if (model && !(model->Flags & 0x80))
        if (race && !(race->Flags & 0x4))
            return true;

    return false;
}

void Unit::SendSpellDamageResist(Unit* target, uint32 spellId, bool debug)
{
    WorldPacket data(SMSG_PROCRESIST, 8+8+4+1);
    data << uint64(GetGUID());
    data << uint64(target->GetGUID());
    data << uint32(spellId);
    data << uint8(debug ? 1 : 0); // bool - log format: 0-default, 1-debug
    SendMessageToSet(&data, true);
}

void Unit::SendPeriodicAuraLog(SpellPeriodicAuraLogInfo* pInfo)
{
    Aura const* aura = pInfo->auraEff;

    WorldPacket data(SMSG_PERIODICAURALOG, 30);
    data << GetPackGUID();
    data.appendPackGUID(aura->GetCasterGUID());
    data << uint32(aura->GetId());                          // spellId
    data << uint32(1);                                      // count
    data << uint32(aura->GetAuraType());                    // auraId
    switch (aura->GetAuraType())
    {
        case SPELL_AURA_PERIODIC_DAMAGE:
        case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
            data << uint32(pInfo->damage);                  // damage
            data << uint32(aura->GetSpellInfo()->SchoolMask);
            data << uint32(pInfo->absorb);                  // absorb
            data << uint32(pInfo->resist);                  // resist
            break;
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_OBS_MOD_HEALTH:
            data << uint32(pInfo->damage);                  // damage
            break;
        case SPELL_AURA_OBS_MOD_POWER:
        case SPELL_AURA_PERIODIC_ENERGIZE:
            data << uint32(aura->GetMiscValue());           // power type
            data << uint32(pInfo->damage);                  // damage
            break;
        case SPELL_AURA_PERIODIC_MANA_LEECH:
            data << uint32(aura->GetMiscValue());           // power type
            data << uint32(pInfo->damage);                  // amount
            data << float(pInfo->multiplier);               // gain multiplier
            break;
        default:
            TC_LOG_ERROR("entities.unit", "Unit::SendPeriodicAuraLog: unknown aura %u", uint32(aura->GetAuraType()));
            return;
    }

    SendMessageToSet(&data, true);
}

void Unit::SendSpellDamageImmune(Unit* target, uint32 spellId)
{
    WorldPacket data(SMSG_SPELLORDAMAGE_IMMUNE, 8+8+4+1);
    data << uint64(GetGUID());
    data << uint64(target->GetGUID());
    data << uint32(spellId);
    data << uint8(0); // bool - log format: 0-default, 1-debug
    SendMessageToSet(&data, true);
}

void Unit::LogBossDown(Creature* cVictim)
{
    if (Player* killingPlayer = GetCharmerOrOwnerPlayerOrPlayerItself()) {
        std::map<uint32, uint32> guildOccurs;
        uint8 groupSize = 0;
        uint32 downByGuildId = 0;
        uint32 leaderGuid = 0;
        float guildPercentage = 0;
        bool mustLog = true;

        if (Group* group = killingPlayer->GetGroup()) 
        {
            leaderGuid = group->GetLeaderGUID();
            groupSize = group->GetMembersCount();
            for (GroupReference* gr = group->GetFirstMember(); gr != NULL; gr = gr->next())
            {
                if (Player* groupGuy = gr->GetSource())
                    guildOccurs[groupGuy->GetGuildId()]++;
            }
        }

        if (groupSize) {
            for (auto itr = guildOccurs.begin(); itr != guildOccurs.end(); itr++) 
            {
                guildPercentage = ((float)itr->second / groupSize) * 100;
                if (guildPercentage >= 67.0f) {
                    downByGuildId = itr->first;
                    break;
                }
            }                    
        }
        else
            mustLog = false; // Don't log solo'ing
        
        std::string bossName = cVictim->GetNameForLocaleIdx(LOCALE_enUS);
        std::string bossNameFr = cVictim->GetNameForLocaleIdx(LOCALE_frFR);
        const char* guildname = "-"; //we'll have to replace this by wathever we want on the website
        if (downByGuildId)
            guildname = sObjectMgr->GetGuildNameById(downByGuildId).c_str();

        uint32 logEntry = cVictim->GetEntry();
                
        // Special cases
        switch (logEntry) 
        {
        case 15192: // Anachronos
            mustLog = false;
            break;
        case 23420: // Essence of Desire -> Reliquary of Souls
            bossName = "Reliquary of Souls";
            bossNameFr = "Reliquaire des Ames";
            break;
        case 23418: // Ros
        case 23419:
        case 22856:
        case 18835: // Maulgar adds
        case 18836:
        case 18834:
        case 18832:
            mustLog = false;
            break;
        case 22949: // Illidari Council 1st member, kept for logging
            bossName = "Illidari Council";
            bossNameFr = "Conseil Illidari";
            break;
        case 22950:
        case 22951:
        case 22952:
        case 15302: // Shadow of Taerar
        case 17256:
            mustLog = false;
            break;
        case 25165: // Eredar Twins, log only if both are defeated
        case 25166:
        {
            bossName = "Eredar Twins";
            bossNameFr = "Jumelles Eredar";
            InstanceScript *pInstance = (((InstanceMap*)(cVictim->GetMap()))->GetInstanceScript());
            if (pInstance && pInstance->GetData(4) != 3)
                mustLog = false;
            break;
        }
        case 17533: // Romulo
            mustLog = false;
            break;
        case 17534: // Julianne
            bossName = "Romulo and Julianne";
            bossNameFr = "Romulo et Julianne";
            break;
        default:
            break;
        }

        if (mustLog)
            LogsDatabaseAccessor::BossDown(cVictim, bossName, bossNameFr, downByGuildId, guildname, guildPercentage, leaderGuid);
    }
}

void Unit::Talk(std::string const& text, ChatMsg msgType, Language language, float textRange, WorldObject const* target)
{
    Trinity::CustomChatTextBuilder builder(this, msgType, text, language, target);
    Trinity::LocalizedPacketDo<Trinity::CustomChatTextBuilder> localizer(builder);
    Trinity::PlayerDistWorker<Trinity::LocalizedPacketDo<Trinity::CustomChatTextBuilder> > worker(this, textRange, localizer);
    VisitNearbyWorldObject(textRange, worker);
}

void Unit::Say(std::string const& text, Language language, WorldObject const* target /*= nullptr*/)
{
    Talk(text, CHAT_MSG_MONSTER_SAY, language, sWorld->getConfig(CONFIG_LISTEN_RANGE_SAY), target);
}

void Unit::Yell(std::string const& text, Language language, WorldObject const* target /*= nullptr*/)
{
    Talk(text, CHAT_MSG_MONSTER_YELL, language, sWorld->getConfig(CONFIG_LISTEN_RANGE_YELL), target);
}

void Unit::TextEmote(std::string const& text, WorldObject const* target /*= nullptr*/, bool isBossEmote /*= false*/)
{
    Talk(text, isBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, sWorld->getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), target);
}

//todo: localize this
void Unit::ServerEmote(std::string const& text, bool isBossEmote)
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, isBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, this, nullptr, text);
    sWorld->SendGlobalMessage(&data);
}

//todo: localize this
void Unit::YellToMap(std::string const& text, Language language)
{
    Map const* map = GetMap();
    if(!map)
        return;

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_MONSTER_YELL, language, this, nullptr, text);

    Map::PlayerList const& players = map->GetPlayers();
    if (!players.isEmpty()) {
        for(Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr) {
            if (Player* plr = itr->GetSource())
                plr->SendDirectMessage(&data);
        }
    }
}

void Unit::Whisper(std::string const& text, Language language, Player* target, bool isBossWhisper /*= false*/)
{
    if (!target)
        return;

    LocaleConstant locale = target->GetSession()->GetSessionDbLocaleIndex();
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, isBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_WHISPER, language, this, target, text, 0, "", locale);
    target->SendDirectMessage(&data);
}

void Unit::Talk(uint32 textId, ChatMsg msgType, float textRange, WorldObject const* target)
{
    if (!sObjectMgr->GetBroadcastText(textId))
    {
        TC_LOG_ERROR("entities.unit", "Unit::Talk: `broadcast_text` was not %u found", textId);
        return;
    }

    Trinity::BroadcastTextBuilder builder(this, msgType, textId, target);
    Trinity::LocalizedPacketDo<Trinity::BroadcastTextBuilder> localizer(builder);
    Trinity::PlayerDistWorker<Trinity::LocalizedPacketDo<Trinity::BroadcastTextBuilder> > worker(this, textRange, localizer);
    VisitNearbyWorldObject(textRange, worker);
}

void Unit::Say(uint32 textId, WorldObject const* target /*= nullptr*/)
{
    Talk(textId, CHAT_MSG_MONSTER_SAY, sWorld->getConfig(CONFIG_LISTEN_RANGE_SAY), target);
}

void Unit::Yell(uint32 textId, WorldObject const* target /*= nullptr*/)
{
    Talk(textId, CHAT_MSG_MONSTER_YELL, sWorld->getConfig(CONFIG_LISTEN_RANGE_YELL), target);
}

void Unit::TextEmote(uint32 textId, WorldObject const* target /*= nullptr*/, bool isBossEmote /*= false*/)
{
    Talk(textId, isBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, sWorld->getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), target);
}

void Unit::Whisper(uint32 textId, Player* target, bool isBossWhisper /*= false*/)
{
    if (!target)
        return;

    BroadcastText const* bct = sObjectMgr->GetBroadcastText(textId);
    if (!bct)
    {
        TC_LOG_ERROR("entities.unit", "Unit::Whisper: `broadcast_text` was not %u found", textId);
        return;
    }

    LocaleConstant locale = target->GetSession()->GetSessionDbLocaleIndex();
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, isBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, this, target, bct->GetText(locale, GetGender()), 0, "", locale);
    target->SendDirectMessage(&data);
}

void Unit::old_Talk(uint32 textId, ChatMsg msgType, float textRange, uint64 targetGUID, uint32 language)
{
    Unit* target = nullptr;
    if(targetGUID)
    {
        target = ObjectAccessor::GetUnit(*this, targetGUID);
        if(!target)
            TC_LOG_ERROR("entities.unit", "WorldObject::old_Talk: unit with guid " UI64FMTD " was not found. Defaulting to no target.", targetGUID);
    }

    Trinity::OldScriptTextBuilder builder(this, msgType, textId, Language(language), target);
    Trinity::LocalizedPacketDo<Trinity::OldScriptTextBuilder> localizer(builder);
    Trinity::PlayerDistWorker<Trinity::LocalizedPacketDo<Trinity::OldScriptTextBuilder> > worker(this, textRange, localizer);
    VisitNearbyWorldObject(textRange, worker);
}

void Unit::old_Say(int32 textId, uint32 language, uint64 TargetGuid)
{
    old_Talk(textId, CHAT_MSG_MONSTER_SAY, sWorld->getConfig(CONFIG_LISTEN_RANGE_SAY), TargetGuid, language);
}

void Unit::old_Yell(int32 textId, uint32 language, uint64 TargetGuid)
{
    old_Talk(textId, CHAT_MSG_MONSTER_YELL, sWorld->getConfig(CONFIG_LISTEN_RANGE_YELL), TargetGuid, language);
}

void Unit::old_TextEmote(int32 textId, uint64 TargetGuid, bool IsBossEmote)
{
    old_Talk(textId, IsBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, sWorld->getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), TargetGuid, LANG_UNIVERSAL);
}

void Unit::old_Whisper(int32 textId, uint64 receiverGUID, bool IsBossWhisper)
{
    if (!receiverGUID)
        return;

    Player* target = ObjectAccessor::FindPlayer(receiverGUID);
    if(!target)
    {
        TC_LOG_ERROR("entities.unit", "WorldObject::old_Whisper: player with guid " UI64FMTD " was not found", receiverGUID);
        return;
    }

    LocaleConstant locale = target->GetSession()->GetSessionDbLocaleIndex();

    char const* text = sObjectMgr->GetTrinityString(textId,locale);

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, IsBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, this, target, text, 0, "", locale);
    target->SendDirectMessage(&data);
}

/*
Logic:
Search a ground under default target, then a bit higher.
Repeat this closer and closer until we found a valid target location.
A valid target location is a location either in LoS or accessible by path

//to improve: water handling in loop (position in water are valid heights if you can swim)
//to improve: you're not suppose to be able to go on non walkable slopes
*/
Position Unit::GetLeapPosition(float dist)
{
    Position currentPos = GetPosition();
    float angle = GetOrientation();
    float destx, desty, destz; //work variables
    destx = currentPos.m_positionX + dist * std::cos(angle);
    desty = currentPos.m_positionY + dist * std::sin(angle);
    destz = currentPos.m_positionZ;
    Position defaultTarget(destx, desty, destz); //position in front
    Position targetPos(currentPos);

    // Prevent invalid coordinates here, position is unchanged
    if (!Trinity::IsValidMapCoord(destx, desty))
    {
        TC_LOG_ERROR("vmap", "WorldObject::GetLeapPosition Could not leap to invalid coordinates X: %f and Y: %f!", destx, desty);
        return currentPos;
    }

    //replace this by dichotomic search ?
    uint8 maxSteps = 10;
    float stepLength = dist / float(maxSteps);
    //search for the closest valid target height, step by step closer
    //replace this by dichotomic search ?
    for (uint8 i = 0; i < maxSteps; i++)
    {
        float maxSearchDist = (dist - i * stepLength) / 1.8f + GetObjectSize()*2; //allow smaller z diff at close range, greater z diff at long range (linear reduction)
        //TC_LOG_TRACE("vmap", "WorldObject::GetLeapPosition Searching for valid target, step %i. maxSearchDist = %f.", i, maxSearchDist);

        //start with higher check then lower
        for (int8 j = 1; j >= 0; j--)
        {
            //search at given z then at z + maxSearchDist
            float mapHeight = GetMap()->GetHeight(PhaseMask(1), destx, desty, destz + j * maxSearchDist / 2, true, maxSearchDist, true);
            if (mapHeight != INVALID_HEIGHT)
            {
                //if no collision
                if (!GetCollisionPosition(currentPos, destx, desty, mapHeight, targetPos, GetObjectSize()))
                {
                    //TC_LOG_TRACE("vmap", "WorldObject::GetLeapPositionFound valid target point, %f %f %f was in LoS", targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ());
                    //GetCollisionPosition already set targetPos to destx, desty, destz
                    goto exitloopfounddest;
                }

                //if is accessible by path (allow just a bit longer than a straight line)
                float distToTarget = GetExactDistance(destx, desty, mapHeight);
                PathGenerator path(this);
                path.SetPathLengthLimit(distToTarget - 6.0f); //this is a hack to help with the imprecision of this check into the path generator
                if (path.CalculatePath(destx, desty, mapHeight, false, false)
                    && ((path.GetPathType() & (PATHFIND_SHORT | PATHFIND_NOPATH | PATHFIND_INCOMPLETE)) == 0)
                    )
                {
                    //additional internal validity hack to compensate for mmap imprecision. Check if path last two points are in Los
                    Movement::PointsArray points = path.GetPath();
                    if (points.size() >= 2) //if shorter than that the LoS check from before should be valid anyway, noneed for else
                    {
                        G3D::Vector3 lastPoint = points[points.size()-1];
                        G3D::Vector3 beforeLastPoint = points[points.size()-2];
                        Position tempposition(beforeLastPoint.x, beforeLastPoint.y, beforeLastPoint.z + 1.5f);
                        if (!GetCollisionPosition(tempposition, lastPoint.x, lastPoint.y, lastPoint.z + 1.5f, targetPos))
                        {
                            if (mapHeight != INVALID_HEIGHT)
                            {
                                targetPos.Relocate(destx, desty, mapHeight + 1.0f);
                                //TC_LOG_TRACE("vmap", "WorldObject::GetLeapPosition Found valid target point, %f %f %f was accessible by path.", targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ());
                                goto exitloopfounddest;
                            }
                        }
                    }
                }
            }
        }

        //no valid dest found, try closer at next step
        //TC_LOG_TRACE("vmap", "WorldObject::GetLeapPosition No valid dest found at this iteration");
        destx -= stepLength * std::cos(angle);
        desty -= stepLength * std::sin(angle);
    }

    //No valid dest found
        if (CanSwim() && GetMap()->IsUnderWater(POSITION_GET_X_Y_Z(&defaultTarget)))
    {
        targetPos = defaultTarget;
    }
    else if (this->IsFalling())
    {
        //try to find a ground not far
        float mapHeight = GetMap()->GetHeight(PhaseMask(1), currentPos.GetPositionX(), currentPos.GetPositionY(), currentPos.GetPositionZ(), true, 15.0f, true);
        if (mapHeight != INVALID_HEIGHT)
            targetPos.m_positionZ = mapHeight + 1.0f;
    }
    targetPos = currentPos;
    //TC_LOG_TRACE("vmap", "WorldObject::GetLeapPosition Could not get to target, stay at current position %f %f %f", targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ());

exitloopfounddest:
    Trinity::NormalizeMapCoord(targetPos.m_positionX);
    Trinity::NormalizeMapCoord(targetPos.m_positionY);
    Trinity::NormalizeMapCoord(targetPos.m_positionZ);

    return targetPos;
}
