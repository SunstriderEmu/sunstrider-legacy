
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
#include "SpellAuraEffects.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "Formulas.h"
#include "Pet.h"
#include "WaypointMovementGenerator.h"
#include "TotemAI.h"
#include "Totem.h"
#include "GameTime.h"
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
#include "ScriptMgr.h"
#include "TemporarySummon.h"
#include "PlayerAI.h"
#include "CharacterCache.h"
#include "AntiCheatMgr.h"
#include "SpellHistory.h"
#include "ChaseMovementGenerator.h"
#include "AbstractFollower.h"

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
#ifdef LICH_KING
    3.14f                  // MOVE_PITCH_RATE
#endif
};

void InitTriggerAuraData();


DamageInfo::DamageInfo(Unit* attacker, Unit* victim, uint32 damage, SpellInfo const* spellInfo, SpellSchoolMask schoolMask, DamageEffectType damageType, WeaponAttackType attackType)
    : m_attacker(attacker), m_victim(victim), m_damage(damage), m_spellInfo(spellInfo), m_schoolMask(schoolMask), 
    m_damageType(damageType), m_attackType(attackType), m_absorb(0), m_resist(0), m_block(0), m_hitMask(0)
{
}

DamageInfo::DamageInfo(std::vector<DamageInfo> const dmg)
    : m_damage(0), m_schoolMask(SPELL_SCHOOL_MASK_NONE), m_absorb(0), m_resist(0), m_block(0), m_hitMask(0), 
    m_attacker(dmg[0].m_attacker), m_victim(dmg[0].m_victim), m_spellInfo(dmg[0].m_spellInfo),
    m_damageType(dmg[0].m_damageType), m_attackType(dmg[0].m_attackType)
{
    for (auto itr : dmg)
    {
        m_damage += itr.m_damage;
        m_schoolMask = SpellSchoolMask(m_schoolMask + itr.m_schoolMask);
        m_absorb += itr.m_absorb;
        m_resist += itr.m_resist;
        m_hitMask |= itr.m_hitMask;
        m_block += itr.m_block;
    }
}

DamageInfo::DamageInfo(CalcDamageInfo const& dmgInfo) : DamageInfo({ std::move(DamageInfo(dmgInfo, 0)), std::move(DamageInfo(dmgInfo, 1))
#ifndef LICH_KING
, std::move(DamageInfo(dmgInfo, 2)), std::move(DamageInfo(dmgInfo, 3)), std::move(DamageInfo(dmgInfo, 4))
#endif
 })
{
}

DamageInfo::DamageInfo(CalcDamageInfo const& dmgInfo, uint8 damageIndex)
    : m_attacker(dmgInfo.Attacker), m_victim(dmgInfo.Target), m_damage(dmgInfo.Damages[damageIndex].Damage), m_spellInfo(nullptr), m_schoolMask(SpellSchoolMask(dmgInfo.Damages[damageIndex].DamageSchoolMask)),
    m_damageType(DIRECT_DAMAGE), m_attackType(dmgInfo.AttackType), m_absorb(dmgInfo.Damages[damageIndex].Absorb), m_resist(dmgInfo.Damages[damageIndex].Resist), m_block(dmgInfo.Blocked), m_hitMask(0)
{
    switch (dmgInfo.TargetState)
    {
    case VICTIMSTATE_IS_IMMUNE:
        m_hitMask |= PROC_HIT_IMMUNE;
        break;
    case VICTIMSTATE_BLOCKS:
        m_hitMask |= PROC_HIT_FULL_BLOCK;
        break;
    }

#ifdef LICH_KING
    if (dmgInfo.HitInfo & (HITINFO_PARTIAL_ABSORB | HITINFO_FULL_ABSORB))
#else
    if (dmgInfo.HitInfo & (HITINFO_ABSORB))
#endif
        m_hitMask |= PROC_HIT_ABSORB;
#ifdef LICH_KING
    if (dmgInfo.HitInfo & HITINFO_FULL_RESIST)
#else
    if (dmgInfo.HitInfo & HITINFO_RESIST && m_damage == 0)
#endif
        m_hitMask |= PROC_HIT_FULL_RESIST;

    if (m_block)
        m_hitMask |= PROC_HIT_BLOCK;

    bool const damageNullified = 
#ifdef LICH_KING
        (dmgInfo.HitInfo & (HITINFO_FULL_ABSORB | HITINFO_FULL_RESIST)) != 0 ||
#else
        (m_hitMask & PROC_HIT_FULL_RESIST) != 0 ||
        (dmgInfo.HitInfo & HITINFO_ABSORB && m_damage == 0) ||
#endif
        (m_hitMask & (PROC_HIT_IMMUNE | PROC_HIT_FULL_BLOCK)) != 0;

    switch (dmgInfo.HitOutCome)
    {
    case MELEE_HIT_MISS:
        m_hitMask |= PROC_HIT_MISS;
        break;
    case MELEE_HIT_DODGE:
        m_hitMask |= PROC_HIT_DODGE;
        break;
    case MELEE_HIT_PARRY:
        m_hitMask |= PROC_HIT_PARRY;
        break;
    case MELEE_HIT_EVADE:
        m_hitMask |= PROC_HIT_EVADE;
        break;
    case MELEE_HIT_BLOCK:
    case MELEE_HIT_CRUSHING:
    case MELEE_HIT_GLANCING:
    case MELEE_HIT_NORMAL:
        if (!damageNullified)
            m_hitMask |= PROC_HIT_NORMAL;
        break;
    case MELEE_HIT_CRIT:
        if (!damageNullified)
            m_hitMask |= PROC_HIT_CRITICAL;
        break;
    }
}

DamageInfo::DamageInfo(SpellNonMeleeDamage const& spellNonMeleeDamage, DamageEffectType damageType, WeaponAttackType attackType, uint32 hitMask)
    : m_attacker(spellNonMeleeDamage.attacker), m_victim(spellNonMeleeDamage.target), m_damage(spellNonMeleeDamage.damage),
    m_spellInfo(sSpellMgr->GetSpellInfo(spellNonMeleeDamage.SpellID)), m_schoolMask(SpellSchoolMask(spellNonMeleeDamage.schoolMask)), m_damageType(damageType),
    m_attackType(attackType), m_absorb(spellNonMeleeDamage.absorb), m_resist(spellNonMeleeDamage.resist), m_block(spellNonMeleeDamage.blocked), m_hitMask(hitMask)
{
    if (spellNonMeleeDamage.blocked)
        m_hitMask |= PROC_HIT_BLOCK;
    if (spellNonMeleeDamage.absorb)
        m_hitMask |= PROC_HIT_ABSORB;
}

void DamageInfo::ModifyDamage(int32 amount)
{
    amount = std::max(amount, -static_cast<int32>(GetDamage()));
    m_damage += amount;
}

void DamageInfo::AbsorbDamage(uint32 amount)
{
    amount = std::min(amount, GetDamage());
    m_absorb += amount;
    m_damage -= amount;
    m_hitMask |= PROC_HIT_ABSORB;
}

void DamageInfo::ResistDamage(uint32 amount)
{
    amount = std::min(amount, GetDamage());
    m_resist += amount;
    m_damage -= amount;
    if (!m_damage)
    {
#ifdef LICH_KING
        m_hitMask |= PROC_HIT_FULL_RESIST;
#else
        m_hitMask |= PROC_HIT_FULL_RESIST;
#endif
        m_hitMask &= ~(PROC_HIT_NORMAL | PROC_HIT_CRITICAL);
    }
}

void DamageInfo::BlockDamage(uint32 amount)
{
    amount = std::min(amount, GetDamage());
    m_block += amount;
    m_damage -= amount;
    m_hitMask |= PROC_HIT_BLOCK;
    if (!m_damage)
    {
        m_hitMask |= PROC_HIT_FULL_BLOCK;
        m_hitMask &= ~(PROC_HIT_NORMAL | PROC_HIT_CRITICAL);
    }
}

uint32 DamageInfo::GetHitMask() const
{
    return m_hitMask;
}

// auraTypes contains attacker auras capable of proc'ing cast auras
static Unit::AuraTypeSet GenerateAttakerProcCastAuraTypes()
{
    static Unit::AuraTypeSet auraTypes;
    auraTypes.insert(SPELL_AURA_DUMMY);
    auraTypes.insert(SPELL_AURA_PROC_TRIGGER_SPELL);
    auraTypes.insert(SPELL_AURA_MOD_MELEE_HASTE);
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

    for(const auto & Effect : spellProto->Effects)
    {
        //from Hellground : set::find is faster than std::find ( O(logN) < O(N))
        if (procAuraTypes.find(Unit::AuraTypeSet::value_type(Effect.ApplyAuraName)) != procAuraTypes.end())
            return false;
    }

    return true;
}

DispelableAura::DispelableAura(Aura* aura, int32 dispelChance, uint8 dispelCharges) :
    _aura(aura), _chance(dispelChance), _charges(dispelCharges)
{
}

DispelableAura::~DispelableAura() = default;

bool DispelableAura::RollDispel() const
{
    return roll_chance_i(_chance);
}

Unit::Unit(bool isWorldObject)
: WorldObject(isWorldObject), m_playerMovingMe(nullptr), i_motionMaster(new MotionMaster(this)), m_combatManager(this), m_threatManager(this),
IsAIEnabled(false), NeedChangeAI(false), movespline(new Movement::MoveSpline()), m_Diminishing(), m_lastSanctuaryTime(0),
i_AI(nullptr), i_disabledAI(nullptr), m_removedAurasCount(0), m_unitTypeMask(UNIT_MASK_NONE),
_lastDamagedTime(0), m_movesplineTimer(0), m_ControlledByPlayer(false), m_procDeep(0),
_last_in_water_status(false),
_last_isunderwater_status(false),
m_duringRemoveFromWorld(false),
m_disabledRegen(false),
m_cleanupDone(false), 
m_regenTimer(0),
m_spellHistory(new SpellHistory(this))
{
    m_objectType |= TYPEMASK_UNIT;
    m_objectTypeId = TYPEID_UNIT;
                                                           
#ifdef LICH_KING
    m_updateFlag = (UPDATEFLAG_LIVING | UPDATEFLAG_STATIONARY_POSITION);
#else
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_LIVING | UPDATEFLAG_STATIONARY_POSITION); // 2.3.2 - 0x70
#endif

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

    for (auto & m_currentSpell : m_currentSpells)
        m_currentSpell = nullptr;

    m_addDmgOnce = 0;

    //m_Aura = NULL;
    //m_AurasCheck = 2000;
    //m_removeAuraTimer = 4;
    //tmpAura = NULL;

    m_auraUpdateIterator = m_ownedAuras.end();

    m_interruptMask = 0;
    m_transform = 0;
    m_ShapeShiftFormSpellId = 0;
    m_canModifyStats = false;

    for (uint8 i = 0; i < UNIT_MOD_END; ++i)
    {
        m_auraFlatModifiersGroup[i][BASE_VALUE] = 0.0f;
        m_auraFlatModifiersGroup[i][TOTAL_VALUE] = 0.0f;
        m_auraPctModifiersGroup[i][BASE_PCT] = 1.0f;
        m_auraPctModifiersGroup[i][TOTAL_PCT] = 1.0f;
    }

    for (auto & i : m_spellImmune)
        i.clear();
                                                            // implement 50% base damage from offhand
    m_auraPctModifiersGroup[UNIT_MOD_DAMAGE_OFFHAND][TOTAL_PCT] = 0.5f;


    for (uint8 i = 0; i < MAX_ATTACK; ++i)
    {
        m_weaponDamage[i][MINDAMAGE][0] = BASE_MINDAMAGE;
        m_weaponDamage[i][MAXDAMAGE][0] = BASE_MAXDAMAGE;

        for (uint8 j = 1; j < MAX_ITEM_PROTO_DAMAGES; ++j)
        {
            m_weaponDamage[i][MINDAMAGE][j] = 0.f;
            m_weaponDamage[i][MAXDAMAGE][j] = 0.f;
        }
    }

    for (float & m_createStat : m_createStats)
        m_createStat = 0.0f;

    m_attacking = nullptr;
    m_modMeleeHitChance = 0.0f;
    m_modRangedHitChance = 0.0f;
    m_modSpellHitChance = 0.0f;
    m_baseSpellCritChance = 5;

    m_lastManaUse = 0;

    m_isSorted = true;
    for (float & i : m_speed_rate)
        i = 1.0f;

    m_charmInfo = nullptr;

    // remove aurastates allowing special moves
    for(uint32 & i : m_reactiveTimer)
        i = 0;
        
    IsRotating = 0;
    m_attackVictimOnEnd = false;
    
    _targetLocked = false;
    m_CombatDistance = 0;//MELEE_RANGE;

    _lastLiquid = nullptr;

    m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
}

Unit::~Unit()
{
    // set current spells as deletable
    for (auto & m_currentSpell : m_currentSpells)
    {
        if (m_currentSpell)
        {
            m_currentSpell->SetReferencedFromCurrent(false);
            m_currentSpell = nullptr;
        }
    }

    RemoveAllGameObjects();
    RemoveAllDynObjects();
    _DeleteRemovedAuras();

    m_Events.KillAllEvents(true);

    delete i_motionMaster;
    delete movespline;
    delete m_charmInfo;
    delete m_spellHistory;

    assert(!m_duringRemoveFromWorld);
    assert(!m_attacking);
    assert(m_attackers.empty());
    assert(m_sharedVision.empty());
    ASSERT(m_Controlled.empty());
    ASSERT(m_appliedAuras.empty());
    ASSERT(m_ownedAuras.empty());
    ASSERT(m_removedAuras.empty());
    ASSERT(m_gameObj.empty());
    ASSERT(m_dynObj.empty());
}

void Unit::Update(uint32 p_time)
{
    // WARNING! Order of execution here is important, do not change.
    // Spells must be processed with event system BEFORE they go to _UpdateSpells.
    // Or else we may have some SPELL_STATE_FINISHED spells stalled in pointers, that is bad.
    m_Events.Update(p_time);

    if (!IsInWorld())
        return;

    _UpdateSpells(p_time);
    if (m_justCCed)
        m_justCCed--;

    // If this is set during update SetCantProc(false) call is missing somewhere in the code
    // Having this would prevent spells from being proced, so let's crash
    ASSERT(!m_procDeep);

    m_combatManager.Update(p_time);

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
    i_motionMaster->Update(p_time);
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
    GetMotionMaster()->LaunchMoveSpline(std::move(init), 0, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);
}


void Unit::ResetAttackTimer(WeaponAttackType type)
{
    m_attackTimer[type] = uint32(GetAttackTime(type) * m_modAttackSpeedPct[type]);
}

bool Unit::IsWithinCombatRange(Unit const* obj, float dist2compare) const
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

bool Unit::IsWithinMeleeRangeAt(Position const& pos, Unit const* obj) const
{
    if (!obj || !IsInMap(obj) || !InSamePhase(obj))
        return false;

    float dx = pos.GetPositionX() - obj->GetPositionX();
    float dy = pos.GetPositionY() - obj->GetPositionY();
    float dz = pos.GetPositionZ() - obj->GetPositionZ();
    float distsq = dx*dx + dy*dy + dz*dz;

    float maxdist = GetMeleeRange(obj);

    return distsq < maxdist * maxdist;
}

float Unit::GetMeleeRange(Unit const* target) const
{
    float range = GetCombatReach() + target->GetCombatReach() + 4.0f / 3.0f;
    return std::max(range, NOMINAL_MELEE_RANGE);
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
            RotateAngle = GetAbsoluteAngle(GetVictim());
        else
            RotateAngle = GetOrientation();
    }

    RotateTimer = fulltime;    
    RotateTimerFull = fulltime;    
    IsRotating = type;
    LastTargetGUID = GetGuidValue(UNIT_FIELD_TARGET);
    SetTarget(ObjectGuid::Empty);
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

AuraApplication* Unit::GetVisibleAura(uint8 slot) const
{
    VisibleAuraMap::const_iterator itr = m_visibleAuras.find(slot);
    if (itr != m_visibleAuras.end())
        return itr->second;
    return 0;
}

void Unit::SetVisibleAura(uint8 slot, AuraApplication * aur)
{
    m_visibleAuras[slot] = aur;
    UpdateAuraForGroup(slot);
}

void Unit::RemoveVisibleAura(uint8 slot)
{
    m_visibleAuras.erase(slot);
    UpdateAuraForGroup(slot);
}

void Unit::UpdateInterruptMask()
{
    m_interruptMask = 0;
#ifdef LICH_KING
    for(auto & m_interruptableAura : m_interruptableAuras)
    {
        if(m_interruptableAura)
            m_interruptMask |= m_interruptableAura->GetSpellInfo()->AuraInterruptFlags;
    }
#endif
    if(Spell* spell = GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        if(spell->getState() == SPELL_STATE_CASTING)
            m_interruptMask |= spell->m_spellInfo->ChannelInterruptFlags;
}

bool Unit::HasAuraTypeWithFamilyFlags(AuraType auraType, uint32 familyName,uint64 familyFlags) const
{
    AuraEffectList const &auras = GetAuraEffectsByType(auraType);
    for(auto aura : auras)
        if(SpellInfo const *iterSpellProto = aura->GetSpellInfo())
            if(iterSpellProto->SpellFamilyName == familyName && iterSpellProto->SpellFamilyFlags & familyFlags)
                return true;

    return false;
}

/* Called by DealDamage for auras that have a chance to be removed on damage taken. */
void Unit::RemoveSpellbyDamageTaken(uint32 damage, uint32 spellId)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if(!spellInfo || spellInfo->HasAttribute(SPELL_ATTR4_DAMAGE_DOESNT_BREAK_AURAS))
        return;

#ifdef TESTS
    if (_disableSpellBreakChance)
        return;
#endif

    // The chance to dispel an aura depends on the damage taken with respect to the casters level.
    uint32 max_dmg = GetLevel() > 8 ? 30 * GetLevel() - 100 : 50;
    float chance = (float(damage) / max_dmg * 100.0f)*0.8;

    AuraApplicationList::iterator i, next;
    for(i = m_ccAuras.begin(); i != m_ccAuras.end(); i = next)
    {
        next = i;
        ++next;

        if(*i && (!spellId || (*i)->GetBase()->GetId() != spellId) && roll_chance_f(chance))
        {
            RemoveAurasDueToSpell((*i)->GetBase()->GetId());
            if (!m_ccAuras.empty())
                next = m_ccAuras.begin();
            else
                return;
        }
    }
}

/*static*/ void Unit::DealDamageMods(Unit const* victim, uint32& damage, uint32* absorb)
{
    if (!victim || !victim->IsAlive() || victim->HasUnitState(UNIT_STATE_IN_FLIGHT) || (victim->GetTypeId() == TYPEID_UNIT && victim->ToCreature()->IsEvadingAttacks()))
    {
        if (absorb)
            *absorb += damage;
        damage = 0;
    }
}

uint32 Unit::DealDamage(Unit* attacker, Unit *pVictim, uint32 damage, CleanDamage const* cleanDamage, DamageEffectType damagetype, SpellSchoolMask damageSchoolMask, SpellInfo const* spellProto, bool durabilityLoss)
{
    if (attacker && pVictim->IsImmunedToDamage(spellProto))
    {
        attacker->SendSpellDamageImmune(pVictim, spellProto->Id);
        return 0;
    }

    if(pVictim->GetTypeId()== TYPEID_UNIT && (pVictim->ToCreature())->IsAIEnabled)
    {
        pVictim->ToCreature()->AI()->DamageTaken(attacker, damage);
    }

    if(attacker && attacker->IsAIEnabled)
        if (attacker->IsAIEnabled)
            attacker->GetAI()->DamageDealt(pVictim, damage, damagetype);

    if (!pVictim->IsAlive() || pVictim->IsInFlight() || (pVictim->GetTypeId() == TYPEID_UNIT && (pVictim->ToCreature())->IsInEvadeMode()))
        return 0;

    if (attacker && pVictim->GetTypeId() == TYPEID_PLAYER && attacker != pVictim)
    {
        if (attacker != pVictim)
        {
            // Signal to pets that their owner was attacked - except when DOT.
            if (damagetype != DOT)
            {
                for (Unit* controlled : pVictim->m_Controlled)
                    if (Creature* cControlled = controlled->ToCreature())
                        if (cControlled->IsAIEnabled)
                            cControlled->AI()->OwnerAttackedBy(attacker);
            }
        }

        //sunstrider: moved out of last check, some damage are caused by self
        if (pVictim->ToPlayer()->GetCommandStatus(CHEAT_GOD))
            return 0;
    }

    // Kidney Shot hack
    if (pVictim->HasAura(408) || pVictim->HasAura(8643)) {
        Aura *aur = nullptr;
        if (pVictim->HasAura(408))
            aur = pVictim->GetAura(408);
        else if (pVictim->HasAura(8643))
            aur = pVictim->GetAura(8643);

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
    
    // Spell 37224: hack
    if (attacker && attacker->HasAura(37224) && spellProto && spellProto->SpellFamilyFlags == 0x1000000000LL && spellProto->SpellIconID == 2562)
        damage += 30;
    
    //You don't lose health from damage taken from another player while in a sanctuary
    //You still see it in the combat log though
    if(attacker && pVictim != attacker && attacker->GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() == TYPEID_PLAYER)
    {
        if(pVictim->IsInSanctuary())
            return 0;
    }

    //Script Event damage taken
    if(attacker && pVictim->GetTypeId() == TYPEID_UNIT && pVictim->IsAIEnabled)
    {
        // Set tagging
        if(!pVictim->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TAPPED) && !(pVictim->ToCreature())->IsPet())
        {
            //Set Loot
            switch(attacker->GetTypeId())
            {
                case TYPEID_PLAYER:
                {
                    (pVictim->ToCreature())->SetLootRecipient(attacker);
                    //Set tagged
                    (pVictim->ToCreature())->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TAPPED);
                    break;
                }
                case TYPEID_UNIT:
                {
                    if((attacker->ToCreature())->IsPet())
                    {
                        (pVictim->ToCreature())->SetLootRecipient(attacker->GetOwner());
                        (pVictim->ToCreature())->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_TAPPED);
                    }
                    break;
                }
            }
        }
    }

    if (damagetype != NODAMAGE)
    {
       // interrupting auras with AURA_INTERRUPT_FLAG_TAKE_DAMAGE before checking !damage (absorbed damage breaks that type of auras)
        if (spellProto)
        {
            if (!(spellProto->HasAttribute(SPELL_ATTR4_DAMAGE_DOESNT_BREAK_AURAS)))
                pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TAKE_DAMAGE, spellProto->Id);
        }
        else
            pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TAKE_DAMAGE, 0);
            
        // interrupt spells with SPELL_INTERRUPT_FLAG_ABORT_ON_DMG on absorbed damage (no dots)
        if (!damage && damagetype != DOT && cleanDamage && cleanDamage->absorbed_damage)
        {
            if (pVictim != attacker && pVictim->GetTypeId() == TYPEID_PLAYER)
            {
                if (Spell* spell = pVictim->m_currentSpells[CURRENT_GENERIC_SPELL])
                    if (spell->getState() == SPELL_STATE_PREPARING)
                    {
                        uint32 interruptFlags = spell->m_spellInfo->InterruptFlags;
                        if ((interruptFlags & SPELL_INTERRUPT_FLAG_ABORT_ON_DMG) != 0)
                            pVictim->InterruptNonMeleeSpells(false);
                    }
            }
        }

        pVictim->RemoveSpellbyDamageTaken(damage, spellProto ? spellProto->Id : 0);
    }

    //TC_LOG_DEBUG("FIXME","DealDamageStart");

    uint32 health = pVictim->GetHealth();
    //TC_LOG_DEBUG("FIXME","deal dmg:%d to health:%d ",damage,health);

    // duel ends when player has 1 or less hp
    bool duel_hasEnded = false;
    if(attacker && pVictim->GetTypeId() == TYPEID_PLAYER && (pVictim->ToPlayer())->duel && damage >= (health-1))
    {
        // prevent kill only if killed in duel and killed by opponent or opponent controlled creature
        if((pVictim->ToPlayer())->duel->opponent == attacker || (pVictim->ToPlayer())->duel->opponent->GetGUID() == attacker->GetOwnerGUID())
            damage = health-1;

        duel_hasEnded = true;
    }

    // Rage from Damage made (only from direct weapon damage)
    if(attacker && cleanDamage && damagetype==DIRECT_DAMAGE && attacker != pVictim && attacker->GetTypeId() == TYPEID_PLAYER && (attacker->GetPowerType() == POWER_RAGE))
    {
        uint32 weaponSpeedHitFactor;

        switch(cleanDamage->attackType)
        {
            case BASE_ATTACK:
            case OFF_ATTACK:
            {
                weaponSpeedHitFactor = uint32(attacker->GetAttackTime(cleanDamage->attackType) / 1000.0f * (cleanDamage->attackType == BASE_ATTACK ? 3.5f : 1.75f));
                if (cleanDamage->hitOutCome == MELEE_HIT_CRIT)
                    weaponSpeedHitFactor *= 2;

                (attacker->ToPlayer())->RewardRage(damage, weaponSpeedHitFactor, true);
                break;
            }
            case RANGED_ATTACK:
            default:
                break;
        }
    }

    if (!damage)
    {
        // Rage from absorbed damage
        if (cleanDamage && cleanDamage->absorbed_damage && pVictim->GetPowerType() == POWER_RAGE && pVictim->GetTypeId() == TYPEID_PLAYER)
            (pVictim->ToPlayer())->RewardRage(cleanDamage->absorbed_damage, 0, false);

        return 0;
    }

    if(attacker && pVictim->GetTypeId() == TYPEID_PLAYER && attacker->GetTypeId() == TYPEID_PLAYER)
    {
        if(Battleground *bg = pVictim->ToPlayer()->GetBattleground())
        {
            Player* playerAttacker = (attacker->ToPlayer());
            if(playerAttacker != (pVictim->ToPlayer()))
                bg->UpdatePlayerScore(playerAttacker, SCORE_DAMAGE_DONE, damage);
            
            bg->UpdatePlayerScore(pVictim->ToPlayer(), SCORE_DAMAGE_TAKEN, damage);
        }
    }

    if (attacker && pVictim->GetTypeId() == TYPEID_UNIT && !(pVictim->ToCreature())->IsPet())
    {
        if(!(pVictim->ToCreature())->hasLootRecipient())
            (pVictim->ToCreature())->SetLootRecipient(attacker);

        if(attacker->GetCharmerOrOwnerPlayerOrPlayerItself())
            (pVictim->ToCreature())->LowerPlayerDamageReq(health < damage ?  health : damage);
    }
    
    if (health <= damage)
    {
        //TC_LOG_DEBUG("FIXME","DealDamage: victim just died");
        Kill(attacker, pVictim, durabilityLoss);
        
        //Hook for OnPVPKill Event
        /*
        if (pVictim->GetTypeId() == TYPEID_PLAYER && GetTypeId() == TYPEID_PLAYER)
        {
            Player *killer = ToPlayer();
            Player *killed = pVictim->ToPlayer();
            sScriptMgr->OnPVPKill(killer, killed);
        }
        */
    }
    else                                                    // if (health <= damage)
    {
        //TC_LOG_DEBUG("FIXME","DealDamageAlive");

        pVictim->ModifyHealth(- (int32)damage);

        if(damagetype == DIRECT_DAMAGE || damagetype == SPELL_DIRECT_DAMAGE)
        {
            //TODO: This is from procflag, I do not know which spell needs this
            //Maim?
            //if (!spellProto || !(spellProto->AuraInterruptFlags&AURA_INTERRUPT_FLAG_DIRECT_DAMAGE))
                pVictim->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_DIRECT_DAMAGE, spellProto ? spellProto->Id : 0);
        }

        if (attacker && pVictim->GetTypeId() != TYPEID_PLAYER)
        {
            //TC_LOG_INFO("DealDamage, AddThreat : %f",threat);
            pVictim->GetThreatManager().AddThreat(attacker, float(damage), spellProto);
        }
        else                                                // victim is a player
        {
            // Rage from damage received
            if(attacker != pVictim && pVictim->GetPowerType() == POWER_RAGE)
            {
                uint32 rage_damage = damage + (cleanDamage ? cleanDamage->absorbed_damage : 0);
                (pVictim->ToPlayer())->RewardRage(rage_damage, 0, false);
            }

            // random durability for items (HIT TAKEN)
            if (attacker && roll_chance_f(sWorld->GetRate(RATE_DURABILITY_LOSS_DAMAGE)))
            {
                EquipmentSlots slot = EquipmentSlots(urand(0,EQUIPMENT_SLOT_END-1));
                (pVictim->ToPlayer())->DurabilityPointLossForEquipSlot(slot);
            }
        }

        if(attacker && attacker->GetTypeId()==TYPEID_PLAYER)
        {
            // random durability for items (HIT DONE)
            if (roll_chance_f(sWorld->GetRate(RATE_DURABILITY_LOSS_DAMAGE)))
            {
              EquipmentSlots slot = EquipmentSlots(urand(0,EQUIPMENT_SLOT_END-1));
                (attacker->ToPlayer())->DurabilityPointLossForEquipSlot(slot);
            }
        }

        if (damagetype != NODAMAGE && damagetype != DOT && damage)
        {
            if (pVictim != attacker && pVictim->GetTypeId() == TYPEID_PLAYER // does not support creature push_back
                && (!spellProto || !(
#ifdef LICH_KING
                    spellProto->HasAttribute(SPELL_ATTR7_NO_PUSHBACK_ON_DAMAGE) ||
#endif
                    spellProto->HasAttribute(SPELL_ATTR3_TREAT_AS_PERIODIC))
                   )
               )
            {
                if (Spell* spell = pVictim->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                {
                    if (spell->getState() == SPELL_STATE_PREPARING)
                    {
                        uint32 interruptFlags = spell->m_spellInfo->InterruptFlags;
                        if (interruptFlags & SPELL_INTERRUPT_FLAG_ABORT_ON_DMG)
                            pVictim->InterruptNonMeleeSpells(false);
                        else if (interruptFlags & SPELL_INTERRUPT_FLAG_PUSH_BACK)
                            spell->Delayed();
                    }
                }

                if (Spell* spell = pVictim->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                {
                    if (spell->getState() == SPELL_STATE_CASTING)
                    {
                        uint32 channelInterruptFlags = spell->m_spellInfo->ChannelInterruptFlags;
                        if (((channelInterruptFlags & CHANNEL_FLAG_DELAY) != 0) && (damagetype != DOT))
                            spell->DelayedChannel();
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

    //TC_LOG_DEBUG("FIXME","DealDamageEnd returned %d damage", damage);

    return damage;
}

void Unit::CastStop(uint32 except_spellid)
{
    for (uint32 i = CURRENT_FIRST_NON_MELEE_SPELL; i < CURRENT_MAX_SPELL; i++)
        if (m_currentSpells[i] && m_currentSpells[i]->m_spellInfo->Id!=except_spellid)
            InterruptSpell(i,false, false);
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

    // Spells with SPELL_ATTR4_FIXED_DAMAGE ignore resilience because their damage is based off another spell's damage.
    if (!spellInfo->HasAttribute(SPELL_ATTR4_FIXED_DAMAGE))
    {
        if (Unit::IsDamageReducedByArmor(damageSchoolMask, spellInfo))
            damage = Unit::CalcArmorReducedDamage(this, pVictim, damage, spellInfo, attackType);

        bool blocked = false;
        // Per-school calc
        switch (spellInfo->DmgClass)
        {
            // Melee and Ranged Spells
        case SPELL_DAMAGE_CLASS_RANGED:
        case SPELL_DAMAGE_CLASS_MELEE:
        {
            // Physical Damage
            if (damageSchoolMask & SPELL_SCHOOL_MASK_NORMAL)
            {
                // Spells with this attribute were already calculated in MeleeSpellHitResult
                if (!spellInfo->HasAttribute(SPELL_ATTR3_BLOCKABLE_SPELL))
                {
                    // Get blocked status
                    blocked = IsSpellBlocked(pVictim, spellInfo, attackType);
                }
            }

            if (crit)
            {
                damageInfo->HitInfo |= SPELL_HIT_TYPE_CRIT;

                // Calculate crit bonus
                uint32 crit_bonus = damage;
                // Apply crit_damage bonus for melee spells
                if (Player* modOwner = GetSpellModOwner())
                    modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_CRIT_DAMAGE_BONUS, crit_bonus);
                damage += crit_bonus;

                // Apply SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE or SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE
                int32 critPctDamageMod = 0;
                if (attackType == RANGED_ATTACK)
                    critPctDamageMod += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE);
                else
                {
                    critPctDamageMod += pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE);
                    critPctDamageMod += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_DAMAGE_BONUS);
                }
                // Increase crit damage from SPELL_AURA_MOD_CRIT_PERCENT_VERSUS
                critPctDamageMod += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_CRIT_PERCENT_VERSUS, crTypeMask);

                if (critPctDamageMod != 0)
                    damage = int32((damage) * float((100.0f + critPctDamageMod) / 100.0f));

                // Resilience - reduce crit damage
                if (pVictim->GetTypeId() == TYPEID_PLAYER)
                    damage -= (pVictim->ToPlayer())->GetMeleeCritDamageReduction(damage);
            }
            // Spell weapon based damage CAN BE crit & blocked at same time
            if (blocked)
            {
                damageInfo->blocked = uint32(pVictim->GetShieldBlockValue());
#ifdef LICH_KING
                // double blocked amount if block is critical
                if (victim->isBlockCritical())
                    damageInfo->blocked += damageInfo->blocked;
#endif
                if (damage <= int32(damageInfo->blocked))
                {
                    damageInfo->blocked = uint32(damage);
                    damageInfo->fullBlock = true;
                }
                damage -= damageInfo->blocked;
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
                damageInfo->HitInfo |= SPELL_HIT_TYPE_CRIT;
                damage = Unit::SpellCriticalDamageBonus(this, spellInfo, damage, pVictim);
                // Resilience - reduce crit damage
                if (pVictim->GetTypeId() == TYPEID_PLAYER && !(spellInfo->HasAttribute(SPELL_ATTR4_IGNORE_RESISTANCES)))
                    damage -= (pVictim->ToPlayer())->GetSpellCritDamageReduction(damage);
            }
        }
        break;
        }
    }

    if (damage < 0)
        damage = 0;

    //hackzz Hailstone Chill
    if (spellInfo && spellInfo->Id == 46576)
    {
        if (Aura* aur = pVictim->GetAura(46458))
            damage = 300 * aur->GetStackAmount();
    }

    // Calculate absorb resist
    damageInfo->damage = damage;
    DamageInfo dmgInfo(*damageInfo, SPELL_DIRECT_DAMAGE, BASE_ATTACK, PROC_HIT_NONE);
    Unit::CalcAbsorbResist(dmgInfo);
    damageInfo->absorb = dmgInfo.GetAbsorb();
    damageInfo->resist = dmgInfo.GetResist();

#ifdef LICH_KING
    if (damageInfo->absorb)
        damageInfo->HitInfo |= (damageInfo->damage - damageInfo->absorb == 0 ? HITINFO_FULL_ABSORB : HITINFO_PARTIAL_ABSORB);

    if (damageInfo->resist)
        damageInfo->HitInfo |= (damageInfo->damage - damageInfo->resist == 0 ? HITINFO_FULL_RESIST : HITINFO_PARTIAL_RESIST);
#else
    if (damageInfo->absorb)
    {
        damageInfo->HitInfo |= HITINFO_ABSORB;
        damageInfo->fullAbsorb = (damageInfo->damage - damageInfo->absorb == 0);
    }

    if (damageInfo->resist)
    {
        damageInfo->HitInfo |= HITINFO_RESIST;
        damageInfo->fullResist = (damageInfo->damage - damageInfo->resist == 0);
    }
#endif

    damageInfo->damage = dmgInfo.GetDamage();
}

void Unit::DealSpellDamage(SpellNonMeleeDamage *damageInfo, bool durabilityLoss)
{
    if (damageInfo == nullptr)
        return;

    Unit *pVictim = damageInfo->target;

    if(!pVictim)
        return;

    if (!pVictim->IsAlive() || pVictim->IsInFlight() || (pVictim->GetTypeId() == TYPEID_UNIT && pVictim->ToCreature()->IsEvadingAttacks()))
        return;

    SpellInfo const *spellProto = sSpellMgr->GetSpellInfo(damageInfo->SpellID);
    if (spellProto == nullptr)
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
        auto& vAuras = pVictim->GetAppliedAuras();
        for(auto & vAura : vAuras)
        {
            if (vAura.second->GetBase()->GetCasterGUID() != GetGUID())
                continue;

            SpellInfo const *spellInfo = vAura.second->GetBase()->GetSpellInfo();
            if (spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && spellInfo->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT))
                vAura.second->GetBase()->RefreshDuration();
        }
    }
    // Call default DealDamage
    CleanDamage cleanDamage(damageInfo->cleanDamage, damageInfo->absorb, BASE_ATTACK, MELEE_HIT_NORMAL);
    Unit::DealDamage(this, pVictim, damageInfo->damage, &cleanDamage, SPELL_DIRECT_DAMAGE, SpellSchoolMask(damageInfo->schoolMask), spellProto, durabilityLoss);

#ifdef TESTS
    if (GetTypeId() == TYPEID_PLAYER)
        if (auto playerBotAI = ToPlayer()->GetPlayerbotAI())
            playerBotAI->SpellDamageDealt(pVictim, damageInfo->damage, spellProto->Id);
#endif
}

//TODO for melee need create structure as in
void Unit::CalculateMeleeDamage(Unit *pVictim, CalcDamageInfo *damageInfo, WeaponAttackType attackType)
{
    damageInfo->Attacker         = this;
    damageInfo->Target           = pVictim;
    damageInfo->AttackType       = attackType;
    damageInfo->CleanDamage      = 0;
    damageInfo->Blocked = 0;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
    {
        damageInfo->Damages[i].DamageSchoolMask = GetMeleeDamageSchoolMask(attackType, i);
        damageInfo->Damages[i].Damage = 0;
        damageInfo->Damages[i].Absorb = 0;
        damageInfo->Damages[i].Resist = 0;
    }

    damageInfo->TargetState      = 0;
    damageInfo->HitInfo          = 0;
    damageInfo->ProcAttacker     = PROC_FLAG_NONE;
    damageInfo->ProcVictim       = PROC_FLAG_NONE;
    damageInfo->HitOutCome       = MELEE_HIT_EVADE;

    if(!pVictim)
        return;

    if(!this->IsAlive() || !pVictim->IsAlive())
        return;

     // Select HitInfo/procAttacker/procVictim flag based on attack type
    switch (attackType)
    {
        case BASE_ATTACK:
            damageInfo->ProcAttacker = PROC_FLAG_DONE_MELEE_AUTO_ATTACK | PROC_FLAG_DONE_MAINHAND_ATTACK;
            damageInfo->ProcVictim   = PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK;
            break;
        case OFF_ATTACK:
            damageInfo->ProcAttacker = PROC_FLAG_DONE_MELEE_AUTO_ATTACK | PROC_FLAG_DONE_OFFHAND_ATTACK;
            damageInfo->ProcVictim   = PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK;
            damageInfo->HitInfo      = HITINFO_OFFHAND;
            break;
        default:
            return;
    }

    // Physical Immune check (must immune to all damages)
    uint8 immunedMask = 0;
    bool immuneToAll = true;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        if (damageInfo->Target->IsImmunedToDamage(SpellSchoolMask(damageInfo->Damages[i].DamageSchoolMask)))
        {
            immunedMask |= (1 << i);
            immuneToAll = false;
        }

    if (immunedMask && immuneToAll) //sun: adapted for BC
    {
       damageInfo->HitInfo       |= HITINFO_NORMALSWING;
       damageInfo->TargetState    = VICTIMSTATE_IS_IMMUNE;
       damageInfo->CleanDamage    = 0;
       return;
    }

    damageInfo->HitOutCome = RollMeleeOutcomeAgainst(damageInfo->Target, damageInfo->AttackType);

    //sun: calc damage only if contact was made
    switch (damageInfo->HitOutCome)
    {
    case MELEE_HIT_EVADE:
    case MELEE_HIT_MISS:
    case MELEE_HIT_DODGE:
    case MELEE_HIT_BLOCK:
    case MELEE_HIT_PARRY:
        break;
    default:
    {
        for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        {
            if (immunedMask & (1 << i))
                continue;

            //sun: speedup, creatures only have one damage type
            if (i > 0 && GetTypeId() == TYPEID_UNIT)
                break;

            SpellSchoolMask schoolMask = SpellSchoolMask(damageInfo->Damages[i].DamageSchoolMask);
            bool const addPctMods = (schoolMask & SPELL_SCHOOL_MASK_NORMAL);

            uint32 damage = 0;
            damage += CalculateDamage(damageInfo->AttackType, false, addPctMods, (1 << i));
            // Add melee damage bonus
            damage = MeleeDamageBonusDone(damageInfo->Target, damage, damageInfo->AttackType, nullptr, schoolMask);
            if (damage) //sun: don't bother with the rest if no dmg done
            {
                damage = damageInfo->Target->MeleeDamageBonusTaken(this, damage, damageInfo->AttackType, nullptr, schoolMask);

                // Calculate armor reduction
                if (Unit::IsDamageReducedByArmor(SpellSchoolMask(damageInfo->Damages[i].DamageSchoolMask)))
                {
                    damageInfo->Damages[i].Damage = (addPctMods & SPELL_SCHOOL_MASK_NORMAL) ? Unit::CalcArmorReducedDamage(damageInfo->Attacker, damageInfo->Target, damage, nullptr, attackType) : damage;
                    damageInfo->CleanDamage += damage - damageInfo->Damages[i].Damage;
                }
                else
                    damageInfo->Damages[i].Damage = damage;
            }
        }
    }
    }

    // Disable parry or dodge for ranged attack
    if(damageInfo->AttackType == RANGED_ATTACK)
    {
        if (damageInfo->HitOutCome == MELEE_HIT_PARRY) damageInfo->HitOutCome = MELEE_HIT_NORMAL;
        if (damageInfo->HitOutCome == MELEE_HIT_DODGE) damageInfo->HitOutCome = MELEE_HIT_MISS;
    }

    switch(damageInfo->HitOutCome)
    {
        case MELEE_HIT_EVADE:
        {
            damageInfo->HitInfo    |= HITINFO_MISS | HITINFO_SWINGNOHITSOUND;
            damageInfo->TargetState = VICTIMSTATE_EVADES;

            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
                damageInfo->Damages[i].Damage = 0;
            damageInfo->CleanDamage = 0;
            return;
        }
        case MELEE_HIT_MISS:
        {
            damageInfo->HitInfo    |= HITINFO_MISS;
            damageInfo->TargetState = VICTIMSTATE_NORMAL;

            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
                damageInfo->Damages[i].Damage = 0;
            damageInfo->CleanDamage = 0;
            break;
        }
        case MELEE_HIT_NORMAL:
            damageInfo->TargetState = VICTIMSTATE_NORMAL;
            break;
        case MELEE_HIT_CRIT:
        {
            damageInfo->HitInfo     |= HITINFO_CRITICALHIT;
            damageInfo->TargetState  = VICTIMSTATE_NORMAL;

            // Crit bonus calc
            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
            {
                damageInfo->Damages[i].Damage *= 2;

                float mod = 0.0f;
                // Apply SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE or SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE
                if (damageInfo->AttackType == RANGED_ATTACK)
                    mod += damageInfo->Target->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_DAMAGE);
                else
                    mod += damageInfo->Target->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_DAMAGE);

                // Increase crit damage from SPELL_AURA_MOD_CRIT_DAMAGE_BONUS
                mod += (GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_CRIT_DAMAGE_BONUS, damageInfo->Damages[i].DamageSchoolMask) - 1.0f) * 100;

                uint32 crTypeMask = damageInfo->Target->GetCreatureTypeMask();

                // Increase crit damage from SPELL_AURA_MOD_CRIT_PERCENT_VERSUS
                mod += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_CRIT_PERCENT_VERSUS, crTypeMask);
                if (mod != 0)
                    AddPct(damageInfo->Damages[i].Damage, mod);

                // Resilience - reduce crit damage
                if (pVictim->GetTypeId() == TYPEID_PLAYER)
                {
                    uint32 resilienceReduction = (pVictim->ToPlayer())->GetMeleeCritDamageReduction(damageInfo->Damages[i].Damage);
                    damageInfo->Damages[i].Damage -= resilienceReduction;
                    damageInfo->CleanDamage += resilienceReduction;
                }
            }

            break;
        }
        case MELEE_HIT_PARRY:
            damageInfo->TargetState  = VICTIMSTATE_PARRY;
            damageInfo->CleanDamage = 0;
            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
            {
                damageInfo->CleanDamage += damageInfo->Damages[i].Damage;
                damageInfo->Damages[i].Damage = 0;
            }
            break;

        case MELEE_HIT_DODGE:
            damageInfo->TargetState  = VICTIMSTATE_DODGE;
            damageInfo->CleanDamage = 0;

            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
            {
                damageInfo->CleanDamage += damageInfo->Damages[i].Damage;
                damageInfo->Damages[i].Damage = 0;
            }
            break;
        case MELEE_HIT_BLOCK:
        {
            damageInfo->TargetState = VICTIMSTATE_NORMAL;
            damageInfo->Blocked = damageInfo->Target->GetShieldBlockValue();
#ifdef LICH_KING
            damageInfo->HitInfo |= HITINFO_BLOCK;
            // double blocked amount if block is critical
            if (damageInfo->Target->isBlockCritical())
                damageInfo->Blocked *= 2;
#endif
            uint32 remainingBlock = damageInfo->Blocked;
            uint8  fullBlockMask = 0;
            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
            {
                if (remainingBlock && remainingBlock >= damageInfo->Damages[i].Damage)
                {
                    fullBlockMask |= (1 << i);

                    remainingBlock -= damageInfo->Damages[i].Damage;
                    damageInfo->CleanDamage += damageInfo->Damages[i].Damage;
                    damageInfo->Damages[i].Damage = 0;
                }
                else
                {
                    damageInfo->CleanDamage += remainingBlock;
                    damageInfo->Damages[i].Damage -= remainingBlock;
                    remainingBlock = 0;
                }
            }

            // full block
            if (fullBlockMask == ((1 << 0) | (1 << 1)))
            {
                damageInfo->TargetState = VICTIMSTATE_BLOCKS;
                damageInfo->Blocked -= remainingBlock;
            }
            break;
        }
        case MELEE_HIT_GLANCING:
        {
            damageInfo->HitInfo     |= HITINFO_GLANCING;
            damageInfo->TargetState  = VICTIMSTATE_NORMAL;
            int32 leveldif = int32(pVictim->GetLevel()) - int32(GetLevel());
            if (leveldif > 3) 
                leveldif = 3;

            // against boss-level targets - 24% chance of 25% average damage reduction (damage reduction range : 20-30%)
            // against level 82 elites - 18% chance of 15% average damage reduction (damage reduction range : 10-20%)
            int32 const reductionMax = leveldif * 10;
            int32 const reductionMin = reductionMax - 10;
            float reducePercent = 1.f - irand(reductionMin, reductionMax) / 100.0f;

            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
            {
                uint32 reducedDamage = uint32(reducePercent * damageInfo->Damages[i].Damage);
                damageInfo->CleanDamage += damageInfo->Damages[i].Damage - reducedDamage;
                damageInfo->Damages[i].Damage = reducedDamage;
            }
            break;
        }
        case MELEE_HIT_CRUSHING:
        {
            damageInfo->HitInfo     |= HITINFO_CRUSHING;
            damageInfo->TargetState  = VICTIMSTATE_NORMAL;

            // 150% normal damage
            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
                damageInfo->Damages[i].Damage += (damageInfo->Damages[i].Damage / 2);
            break;
        }
        default:

            break;
    }
    uint32 tmpHitInfo[MAX_ITEM_PROTO_DAMAGES] = {};

    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
    {
        // Calculate absorb resist
        if (int32(damageInfo->Damages[i].Damage) > 0)
        {
            damageInfo->ProcVictim |= PROC_FLAG_TAKEN_DAMAGE;
            // Calculate absorb & resists
            DamageInfo dmgInfo(*damageInfo, i);
            Unit::CalcAbsorbResist(dmgInfo);
            damageInfo->Damages[i].Absorb = dmgInfo.GetAbsorb();
            damageInfo->Damages[i].Resist = dmgInfo.GetResist();

#ifdef LICH_KING
            if (damageInfo->Damages[i].Absorb)
                tmpHitInfo[i] |= (damageInfo->Damages[i].Damage - damageInfo->Damages[i].Absorb == 0 ? HITINFO_FULL_ABSORB : HITINFO_PARTIAL_ABSORB);

            if (damageInfo->Damages[i].Resist)
                tmpHitInfo[i] |= (damageInfo->Damages[i].Damage - damageInfo->Damages[i].Resist == 0 ? HITINFO_FULL_RESIST : HITINFO_PARTIAL_RESIST);
#else
            if (damageInfo->Damages[i].Absorb)
                tmpHitInfo[i] |= HITINFO_ABSORB;

            if (damageInfo->Damages[i].Resist)
                tmpHitInfo[i] |= HITINFO_RESIST;
#endif
            damageInfo->CleanDamage += damageInfo->Damages[i].Damage - dmgInfo.GetDamage();
            damageInfo->Damages[i].Damage = dmgInfo.GetDamage();
        }
        else // Impossible get negative result but....
            damageInfo->Damages[i].Damage = 0;
    }

#ifdef LICH_KING
    // set proper HitInfo flags
    if ((tmpHitInfo[0] & HITINFO_FULL_ABSORB) != 0)
    {
        // only set full absorb whenever both damages were fully absorbed
        damageInfo->HitInfo |= ((tmpHitInfo[1] & HITINFO_FULL_ABSORB) != 0) ? HITINFO_FULL_ABSORB : HITINFO_PARTIAL_ABSORB;
    }
    else
        damageInfo->HitInfo |= (tmpHitInfo[0] & HITINFO_PARTIAL_ABSORB);
    

    if (tmpHitInfo[0] == HITINFO_FULL_RESIST)
    {
        // only set full resist whenever both damages were fully resisted
        damageInfo->HitInfo |= ((tmpHitInfo[1] & HITINFO_FULL_RESIST) != 0) ? HITINFO_FULL_RESIST : HITINFO_PARTIAL_RESIST;
    }
    else
        damageInfo->HitInfo |= (tmpHitInfo[0] & HITINFO_FULL_RESIST);
#else
    //set HITINFO_ABSORB if any damage is absorbed
    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        if ((tmpHitInfo[i] & HITINFO_ABSORB) != 0)
        {
            damageInfo->HitInfo |= HITINFO_ABSORB;
            break;
        }

    //set HITINFO_RESIST if any damage is resisted
    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        if ((tmpHitInfo[i] & HITINFO_RESIST) != 0)
        {
            damageInfo->HitInfo |= HITINFO_RESIST;
            break;
        }
#endif
}

void Unit::DealMeleeDamage(CalcDamageInfo* damageInfo, bool durabilityLoss)
{
    if (!damageInfo) 
        return;

    Unit *pVictim = damageInfo->Target;

    if(!pVictim)
        return;

    if (!pVictim->IsAlive() || pVictim->IsInFlight() || (pVictim->GetTypeId() == TYPEID_UNIT && pVictim->ToCreature()->IsEvadingAttacks()))
        return;

    //You don't lose health from damage taken from another player while in a sanctuary
    //You still see it in the combat log though
    if(pVictim != this && GetTypeId() == TYPEID_PLAYER && pVictim->GetTypeId() == TYPEID_PLAYER && pVictim->IsInSanctuary())
        return;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
    {
        // Call default DealDamage
        CleanDamage cleanDamage(damageInfo->CleanDamage, damageInfo->Damages[i].Absorb, damageInfo->AttackType, damageInfo->HitOutCome);
        Unit::DealDamage(this, pVictim, damageInfo->Damages[i].Damage, &cleanDamage, DIRECT_DAMAGE, SpellSchoolMask(damageInfo->Damages[i].DamageSchoolMask), nullptr, durabilityLoss);
    }

#ifdef TESTS
    if (Player* p = GetCharmerOrOwnerPlayerOrPlayerItself())
        if (p->GetPlayerbotAI())
            p->GetPlayerbotAI()->DoneWhiteDamage(pVictim, damageInfo);
#endif

    // If this is a creature and it attacks from behind it has a probability to daze it's victim
    if( (damageInfo->HitOutCome == MELEE_HIT_CRIT || damageInfo->HitOutCome == MELEE_HIT_CRUSHING || damageInfo->HitOutCome == MELEE_HIT_NORMAL || damageInfo->HitOutCome == MELEE_HIT_GLANCING) &&
        GetTypeId() != TYPEID_PLAYER && !(this->ToCreature())->GetCharmerOrOwnerGUID() && !pVictim->HasInArc(M_PI, this)
        && (pVictim->GetTypeId() == TYPEID_PLAYER || !(pVictim->ToCreature())->IsWorldBoss()))
    {
        // -probability is between 0% and 40%
        // 20% base chance
        float Probability = 20;

        //there is a newbie protection, at level 10 just 7% base chance; assuming linear function
        if(pVictim->GetLevel() < 30)
            Probability = 0.65f*pVictim->GetLevel() + 0.5f;

        uint32 VictimDefense = pVictim->GetDefenseSkillValue();
        uint32 AttackerMeleeSkill = GetMaxSkillValueForLevel(pVictim);

        Probability *= AttackerMeleeSkill / (float)VictimDefense;

        if(Probability > 40)
            Probability = 40;

        if(roll_chance_f(Probability))
            CastSpell(pVictim, 1604, true);
    }

    if (GetTypeId() == TYPEID_PLAYER)
    {
        DamageInfo dmgInfo(*damageInfo);
        ToPlayer()->CastItemCombatSpell(dmgInfo);
    }

    // Do effect if any damage done to target
    if (damageInfo->Damages[0].Damage + damageInfo->Damages[1].Damage)
    {
        // We're going to call functions which can modify content of the list during iteration over it's elements
        // Let's copy the list so we can prevent iterator invalidation
        AuraEffectList vDamageShieldsCopy(pVictim->GetAuraEffectsByType(SPELL_AURA_DAMAGE_SHIELD));
        for (AuraEffect const* aurEff : vDamageShieldsCopy)
        {
            SpellInfo const* spellInfo = aurEff->GetSpellInfo();

            // Damage shield can be resisted...
            SpellMissInfo missInfo = pVictim->SpellHitResult(this, spellInfo, false);
            if (missInfo != SPELL_MISS_NONE)
            {
                pVictim->SendSpellMiss(this, spellInfo->Id, missInfo);
                continue;
            }

            // ...or immuned
            if (IsImmunedToDamage(spellInfo))
            {
                pVictim->SendSpellDamageImmune(this, spellInfo->Id);
                continue;
            }

            uint32 damage = aurEff->GetAmount();
            if (Unit* caster = aurEff->GetCaster())
            {
                damage = caster->SpellDamageBonusDone(this, spellInfo, damage, SPELL_DIRECT_DAMAGE, aurEff->GetEffIndex(), {});
                damage = SpellDamageBonusTaken(caster, spellInfo, damage, SPELL_DIRECT_DAMAGE);
            }

            // No Unit::CalcAbsorbResist here - opcode doesn't send that data - this damage is probably not affected by that
            Unit::DealDamageMods(this, damage, nullptr);

            WorldPacket data(SMSG_SPELLDAMAGESHIELD,(8+8+4+4));
            data << uint64(pVictim->GetGUID());
            data << uint64(GetGUID());
            data << uint32(spellInfo->Id);
            data << uint32(damage);
#ifdef LICH_KING
            int32 const overkill = int32(damage) - int32(GetHealth());
            data << uint32(std::max(overkill, 0)); // Overkill
#endif
            data << uint32(spellInfo->SchoolMask);
            pVictim->SendMessageToSet(&data, true );

            Unit::DealDamage(pVictim, this, damage, nullptr, SPELL_DIRECT_DAMAGE, spellInfo->GetSchoolMask(), spellInfo, true);
        }

        // HACK: update at damage Judgement aura duration that applied by attacker at victim
        AuraMap& vAuras = pVictim->GetOwnedAuras();
        for (auto & vAura : vAuras)
        {
            SpellInfo const *spellInfo = vAura.second->GetSpellInfo();
            if (spellInfo->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT) && spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && (vAura.second->GetCasterGUID() == GetGUID()) && spellInfo->Id != 41461) //Gathios judgement of blood (can't seem to find a general rule to avoid this hack)
                vAura.second->SetDuration(vAura.second->GetMaxDuration());
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

/*static*/ bool Unit::IsDamageReducedByArmor(SpellSchoolMask schoolMask, SpellInfo const* spellInfo /*= nullptr*/, int8 effIndex /*= -1*/)
{
    // only physical spells damage gets reduced by armor
    if ((schoolMask & SPELL_SCHOOL_MASK_NORMAL) == 0)
        return false;
    if (spellInfo)
    {
        // there are spells with no specific attribute but they have "ignores armor" in tooltip
        if (spellInfo->HasAttribute(SPELL_ATTR0_CU_IGNORE_ARMOR))
            return false;

        // bleeding effects are not reduced by armor
        if (effIndex != -1)
        {
            if (spellInfo->Effects[effIndex].ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE ||
                spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
                if (spellInfo->GetEffectMechanicMask(effIndex) & (1 << MECHANIC_BLEED))
                    return false;
        }
    }
    return true;
}

uint32 Unit::CalcArmorReducedDamage(Unit const* attacker, Unit* pVictim, const uint32 damage, SpellInfo const* spellInfo, WeaponAttackType attackType)
{
    if(sWorld->getConfig(CONFIG_DEBUG_DISABLE_ARMOR))
        return damage;

    float armor = pVictim->GetArmor();
    // Ignore enemy armor by SPELL_AURA_MOD_TARGET_RESISTANCE aura
    armor += attacker->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_RESISTANCE, SPELL_SCHOOL_MASK_NORMAL);

#ifdef LICH_KING

    AuraEffectList const& resIgnoreAurasAb = GetAuraEffectsByType(SPELL_AURA_MOD_ABILITY_IGNORE_TARGET_RESIST);
    for (AuraEffectList::const_iterator j = resIgnoreAurasAb.begin(); j != resIgnoreAurasAb.end(); ++j)
    {
        if ((*j)->GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL && (*j)->IsAffectedOnSpell(spellInfo))
            armor = std::floor(AddPct(armor, -(*j)->GetAmount()));
    }

    AuraEffectList const& resIgnoreAuras = GetAuraEffectsByType(SPELL_AURA_MOD_IGNORE_TARGET_RESIST);
    for (AuraEffectList::const_iterator j = resIgnoreAuras.begin(); j != resIgnoreAuras.end(); ++j)
    {
        if ((*j)->GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL)
            armor = std::floor(AddPct(armor, -(*j)->GetAmount()));
    }

    // Apply Player CR_ARMOR_PENETRATION rating and buffs from stances\specializations etc.
    // TODO LK TC
#endif

    if ( armor < 0.0f ) 
        armor = 0.0f;

    float levelModifier = attacker->GetLevel();
    float damageReduction = 0.0f;
    if(levelModifier <= 59)                                    //Level 1-59
        damageReduction = armor / (armor + 400.0f + 85.0f * levelModifier);
    else if(levelModifier < 70)                                //Level 60-69
        damageReduction = armor / (armor - 22167.5f + 467.5f * levelModifier);
    else                                                    //Level 70+
        damageReduction = armor / (armor + 10557.5f); //same as previous calculation, may be a speedup

    //caps
    RoundToInterval(damageReduction, 0.f, 0.75f);

    return std::max<uint32>(damage * (1.0f - damageReduction), 1);
}

float Unit::CalculateAverageResistReduction(WorldObject const* caster, SpellSchoolMask schoolMask, Unit const* victim, SpellInfo const* spellInfo)
{
    // Get base victim resistance for school
    int32 victimResistance = (float)victim->GetResistance(GetFirstSchoolInMask(schoolMask));
    if (caster)
    {
        // pets inherit 100% of masters penetration
        // Ignore resistance by self SPELL_AURA_MOD_TARGET_RESISTANCE aura (aka spell penetration)
        if (Player const* player = caster->GetSpellModOwner())
        {
            victimResistance += float(player->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_RESISTANCE, schoolMask));
            //victimResistance -= float(player->GetSpellPenetrationItemMod());
        }
        else if (Unit const* unitCaster = caster->ToUnit())
            victimResistance += float(unitCaster->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_RESISTANCE, schoolMask));
    }
    // holy resistance exists in pve and comes from level difference, ignore template values
    if (schoolMask & SPELL_SCHOOL_MASK_HOLY)
        victimResistance = 0.0f;

#ifdef LICH_KING
    // Chaos Bolt exception, ignore all target resistances (unknown attribute?)
    if (spellInfo && spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK && spellInfo->SpellIconID == 3178)
        victimResistance = 0.0f;
#endif

    // Resistance can't be negative
    victimResistance = std::max(victimResistance, 0);

    // TC: level-based resistance does not apply to binary spells, and cannot be overcome by spell penetration
    // "Empirical evidence shows that mobs that are higher level than yourself have an innate chance of partially resisting spell damage that is impossible to counter."
    /* "See also Wowwiki's article on Magical Resistance: http://wowwiki.wikia.com/wiki/Formulas:Magical_resistance?oldid=1603715 Level-based resistance (not to be confused with level-based miss) can play a factor in total resists. For every level that a mob has over the player, there is 8 resist (believed; the exact number may be higher) added. For boss fights, this means there is 15-24 resistance added. This extra resistance means there will be partial resists on non-binary spells from the added resistance. However, this resistance has been shown to not apply to binary spells at all. This level based resistance cannot be reduced by any means, not even Spell Penetration."
    But Is it 8 or 5 per level ?
    "Frostbolt, however, is a binary spell, meaning it has special rules regarding magic resist. A binary spell can only ever hit or miss; it will not ever partially resist. Because of this, frostbolt is completely unaffected by level based magic resist. We're not exactly sure why this occurs, but frost mages almost always see a 99% hit rate on frostbolt (assuming they are hit capped like they should be)  Therefore the 6% damage reduction can be completely ignored. This does not mean that frostbolt does more damage than fireball; it is only another consideration that has been made when comparing specs and theorycrafting."
    */
    if (caster && caster->GetTypeId() != TYPEID_GAMEOBJECT && (!spellInfo || !spellInfo->IsBinarySpell()))
    {
        int32 levelDiff = victim->GetLevelForTarget(caster) - caster->GetLevelForTarget(victim);
        if (levelDiff > 0)
            victimResistance += std::max(levelDiff * 5.0f, 0.0f);
    }

#ifdef LICH_KING
    uint32 level = victim->getLevel();
    float resistanceConstant = 0.0f;

    if (level == BOSS_LEVEL)
        resistanceConstant = BOSS_RESISTANCE_CONSTANT;
    else
        resistanceConstant = level * 5.0f;

    return victimResistance / (victimResistance + resistanceConstant);
#else
    uint32 resistLevel = caster->ToUnit() ? caster->ToUnit()->GetLevel() : victim->GetLevel();
    float fResistance = ((float)victimResistance / (float)(5.0f * resistLevel)) * 0.75f; //% from 0.0 to 1.0

    // Resistance can't be more than 75%
    if (fResistance > 0.75f)
        fResistance = 0.75f;

    return fResistance;
#endif
}

uint32 Unit::CalcSpellResistedDamage(Unit const* attacker, Unit* victim, uint32 damage, SpellSchoolMask schoolMask, SpellInfo const* spellInfo)
{
    // Magic damage, check for resists
    if (!(schoolMask & SPELL_SCHOOL_MASK_MAGIC))
        return 0;

    // Npcs can have holy resistance
    if ((schoolMask & SPELL_SCHOOL_MASK_HOLY) && victim->GetTypeId() != TYPEID_UNIT)
        return 0;

    // Ignore spells that can't be resisted
    if (spellInfo)
    {
        if (spellInfo->HasAttribute(SPELL_ATTR4_IGNORE_RESISTANCES))
            return 0;

        // Binary spells can't have damage part resisted
       if (spellInfo->IsBinarySpell())
            return 0;
    }

    float const averageResist = Unit::CalculateAverageResistReduction(attacker, schoolMask, victim, spellInfo);
   
    // Distribute reduction between 4 ranges of damage : 75%, 50%, 25% and 0%
    // Probably wrong... Interesting discussion here : https://github.com/cmangos/issues/issues/1193
    // Also note that we can't use LK since it has changed a lot after BC
    std::array<float, 4> discreteResistProbability = {};
    uint32 faq[4] = { 24,6,4,6 };
    for (uint32 i = 0; i < discreteResistProbability.size(); ++i)
        discreteResistProbability[i] = 2400 * (powf(averageResist, i) * powf((1 - averageResist), (4 - i))) / faq[i];

    float roll = float(rand_norm()) * 100.0f;
    uint8 resistance = 0; //resistance range (from 1 to 4)
    float probabilitySum = 0.0f;
    for (; resistance < discreteResistProbability.size(); ++resistance)
        if (roll < (probabilitySum += discreteResistProbability[resistance]))
            break;

    resistance = std::min(resistance, uint8(3)); //why can this reach 4? slightly wrong discreteResistProbability values?
    float damageResisted = uint32(damage * resistance / 4.0f);

    DEBUG_ASSERT(damageResisted < damage);

#ifdef LICH_KING
    if (damageResisted > 0.0f) // if any damage was resisted
    {
        int32 ignoredResistance = 0;
        ignoredResistance += GetTotalAuraModifier(SPELL_AURA_MOD_ABILITY_IGNORE_TARGET_RESIST, [schoolMask, spellInfo](AuraEffect const* aurEff) -> bool
        {
            if ((aurEff->GetMiscValue() & schoolMask) && aurEff->IsAffectedOnSpell(spellInfo))
                return true;
            return false;
        });

        ignoredResistance += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_IGNORE_TARGET_RESIST, schoolMask);

        ignoredResistance = std::min<int32>(ignoredResistance, 100);
        ApplyPct(damageResisted, 100 - ignoredResistance);

        // Spells with melee and magic school mask, decide whether resistance or armor absorb is higher
        if (spellInfo && spellInfo->HasAttribute(SPELL_ATTR0_CU_SCHOOLMASK_NORMAL_WITH_MAGIC)) //there is no such spell on BC
        {
            uint32 damageAfterArmor = CalcArmorReducedDamage(victim, damage, spellInfo, BASE_ATTACK);
            float armorReduction = damage - damageAfterArmor;

            // pick the lower one, the weakest resistance counts
            damageResisted = std::min(damageResisted, armorReduction);
        }
    }
#endif

    return damageResisted;
}

/*static*/ void Unit::CalcAbsorbResist(DamageInfo& damageInfo)
{
    if (!damageInfo.GetVictim() || !damageInfo.GetVictim()->IsAlive() || !damageInfo.GetDamage())
        return;

    uint32 resistedDamage = Unit::CalcSpellResistedDamage(damageInfo.GetAttacker(), damageInfo.GetVictim(), damageInfo.GetDamage(), damageInfo.GetSchoolMask(), damageInfo.GetSpellInfo());
    damageInfo.ResistDamage(resistedDamage);

#ifdef LICH_KING
    // Ignore Absorption Auras
    float auraAbsorbMod = 0.f;
    if (Unit* attacker = damageInfo.GetAttacker())
    {
        auraAbsorbMod = attacker->GetMaxPositiveAuraModifierByMiscMask(SPELL_AURA_MOD_TARGET_ABSORB_SCHOOL, damageInfo.GetSchoolMask());
        auraAbsorbMod = std::max(auraAbsorbMod, static_cast<float>(attacker->GetMaxPositiveAuraModifier(SPELL_AURA_MOD_TARGET_ABILITY_ABSORB_SCHOOL, [&damageInfo](AuraEffect const* aurEff) -> bool
        {
            if (!(aurEff->GetMiscValue() & damageInfo.GetSchoolMask()))
                return false;

            if (!aurEff->IsAffectedOnSpell(damageInfo.GetSpellInfo()))
                return false;

            return true;
        })));
    }

    RoundToInterval(auraAbsorbMod, 0.0f, 100.0f);

    int32 absorbIgnoringDamage = CalculatePct(damageInfo.GetDamage(), auraAbsorbMod);
    damageInfo.ModifyDamage(-absorbIgnoringDamage);
#endif

    AuraEffectList const& vOverrideScripts = damageInfo.GetVictim()->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraEffectList::const_iterator i = vOverrideScripts.begin(), next; i != vOverrideScripts.end(); i = next)
    {
        next = i; ++next;

        if (damageInfo.GetVictim()->GetTypeId() != TYPEID_PLAYER)
            break;

        // Shadow of Death - set cheat death on cooldown
        if ((*i)->GetSpellInfo()->Id == 40251 && damageInfo.GetVictim()->GetHealth() <= damageInfo.GetDamage())
        {
            (damageInfo.GetVictim()->ToPlayer())->GetSpellHistory()->AddCooldown(31231, 0, std::chrono::seconds(60));
            break;
        }
    }

    // We're going to call functions which can modify content of the list during iteration over it's elements
    // Let's copy the list so we can prevent iterator invalidation
    AuraEffectList vSchoolAbsorbCopy(damageInfo.GetVictim()->GetAuraEffectsByType(SPELL_AURA_SCHOOL_ABSORB));
    //vSchoolAbsorbCopy.sort(Trinity::AbsorbAuraOrderPred());

    // absorb without mana cost
    int32 reflectDamage = 0;
    AuraEffect* reflectAura = nullptr;
    for (AuraEffectList::iterator itr = vSchoolAbsorbCopy.begin(); (itr != vSchoolAbsorbCopy.end()) && (damageInfo.GetDamage() > 0); ++itr)
    {
        AuraEffect* absorbAurEff = *itr;
        // Check if aura was removed during iteration - we don't need to work on such auras
        AuraApplication const* aurApp = absorbAurEff->GetBase()->GetApplicationOfTarget(damageInfo.GetVictim()->GetGUID());
        if (!aurApp)
            continue;

        if (!(absorbAurEff->GetMiscValue() & damageInfo.GetSchoolMask()))
            continue;

        // Cheat Death hack
        if (damageInfo.GetVictim()->GetTypeId() == TYPEID_PLAYER && absorbAurEff->GetSpellInfo()->SpellFamilyName == SPELLFAMILY_ROGUE && absorbAurEff->GetSpellInfo()->SpellIconID == 2109)
        {
            Player* playerVictim = damageInfo.GetVictim()->ToPlayer();
            if (playerVictim->GetSpellHistory()->HasCooldown(31231))
                continue;
            if (playerVictim->GetHealth() <= damageInfo.GetDamage())
            {
                int32 chance = damageInfo.GetAbsorb();
                if (roll_chance_i(chance))
                {
                    playerVictim->CastSpell(playerVictim, 31231, true);
                    playerVictim->GetSpellHistory()->AddCooldown(31231, 0, std::chrono::seconds(60));

                    // with health > 10% lost health until health==10%, in other case no losses
                    uint32 health10 = playerVictim->GetMaxHealth() / 10;
                    if (playerVictim->GetHealth() <= health10)
                        damageInfo.AbsorbDamage(damageInfo.GetDamage()); //absorb remaining damage
                    else
                        damageInfo.AbsorbDamage(damageInfo.GetDamage() - (damageInfo.GetDamage() - health10)); //absorb damage up to 10% hp
                }
            }
            continue;
        }

        // Shadow of Death hack
        if (absorbAurEff->GetSpellInfo()->Id == 40251)
        {
            if (damageInfo.GetVictim()->GetHealth() <= damageInfo.GetDamage())
            {
                damageInfo.AbsorbDamage(damageInfo.GetDamage()); //absorb remaining damage
                absorbAurEff->GetBase()->Remove();
            }
            break;
        }

        //Reflective Shield
        if ((damageInfo.GetVictim() != damageInfo.GetAttacker()))
        {
            if (Unit* caster = absorbAurEff->GetCaster())
            {
                if (absorbAurEff->GetSpellInfo()->SpellFamilyName == SPELLFAMILY_PRIEST && absorbAurEff->GetSpellInfo()->SpellFamilyFlags == 0x1)
                {
                    AuraEffectList const& vOverRideCS = caster->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
                    for (auto k : vOverRideCS)
                    {
                        switch (k->GetMiscValue())
                        {
                        case 5065:                          // Rank 1
                        case 5064:                          // Rank 2
                        case 5063:                          // Rank 3
                        case 5062:                          // Rank 4
                        case 5061:                          // Rank 5
                        {
                            if (damageInfo.GetDamage() >= absorbAurEff->GetAmount())
                                reflectDamage = absorbAurEff->GetAmount() * k->GetAmount() / 100;
                            else
                                reflectDamage = k->GetAmount() * damageInfo.GetDamage() / 100;
                            reflectAura = absorbAurEff;

                        } break;
                        default: break;
                        }

                        if (reflectDamage)
                            break;
                    }
                }
                // Reflective Shield, NPC
                else if (absorbAurEff->GetSpellInfo()->Id == 41475)
                {
                    if (damageInfo.GetDamage() >= absorbAurEff->GetAmount())
                        reflectDamage = absorbAurEff->GetAmount() * 0.5f;
                    else
                        reflectDamage = damageInfo.GetDamage() * 0.5f;
                    reflectAura = absorbAurEff;
                }
            }
        }

        // get amount which can be still absorbed by the aura
        int32 currentAbsorb = absorbAurEff->GetAmount();
        // aura with infinite absorb amount - let the scripts handle absorbtion amount, set here to 0 for safety
        if (currentAbsorb < 0)
            currentAbsorb = 0;

        uint32 tempAbsorb = uint32(currentAbsorb);

        bool defaultPrevented = false;

        absorbAurEff->GetBase()->CallScriptEffectAbsorbHandlers(absorbAurEff, aurApp, damageInfo, tempAbsorb, defaultPrevented);
        currentAbsorb = tempAbsorb;

        if (defaultPrevented)
            continue;

        // absorb must be smaller than the damage itself
        currentAbsorb = RoundToInterval(currentAbsorb, 0, int32(damageInfo.GetDamage()));

        damageInfo.AbsorbDamage(currentAbsorb);

        tempAbsorb = currentAbsorb;
        absorbAurEff->GetBase()->CallScriptEffectAfterAbsorbHandlers(absorbAurEff, aurApp, damageInfo, tempAbsorb);

        // Check if our aura is using amount to count damage
        if (absorbAurEff->GetAmount() >= 0)
        {
            // Reduce shield amount
            absorbAurEff->SetAmount(absorbAurEff->GetAmount() - currentAbsorb);
            // Aura cannot absorb anything more - remove it
            if (absorbAurEff->GetAmount() <= 0)
                absorbAurEff->GetBase()->Remove(AURA_REMOVE_BY_ENEMY_SPELL);
        }
    }

    // do not cast spells while looping auras; auras can get invalid otherwise
    if (reflectDamage)
    {
        CastSpellExtraArgs args;
        args.TriggerFlags = TRIGGERED_FULL_MASK;
        args.AddSpellBP0(int32(reflectDamage));
        args.SetTriggeringAura(reflectAura);
        damageInfo.GetVictim()->CastSpell(damageInfo.GetAttacker(), 33619, args);
    }

    // absorb by mana cost
    AuraEffectList vManaShieldCopy(damageInfo.GetVictim()->GetAuraEffectsByType(SPELL_AURA_MANA_SHIELD));
    for (AuraEffectList::const_iterator itr = vManaShieldCopy.begin(); (itr != vManaShieldCopy.end()) && (damageInfo.GetDamage() > 0); ++itr)
    {
        AuraEffect* absorbAurEff = *itr;
        // Check if aura was removed during iteration - we don't need to work on such auras
        AuraApplication const* aurApp = absorbAurEff->GetBase()->GetApplicationOfTarget(damageInfo.GetVictim()->GetGUID());
        if (!aurApp)
            continue; 

        // check damage school mask
        if (!(absorbAurEff->GetMiscValue() & damageInfo.GetSchoolMask()))
            continue;

        // get amount which can be still absorbed by the aura
        int32 currentAbsorb = absorbAurEff->GetAmount();
        // aura with infinite absorb amount - let the scripts handle absorbtion amount, set here to 0 for safety
        if (currentAbsorb < 0)
            currentAbsorb = 0;

        uint32 tempAbsorb = currentAbsorb;

        bool defaultPrevented = false;

         absorbAurEff->GetBase()->CallScriptEffectManaShieldHandlers(absorbAurEff, aurApp, damageInfo, tempAbsorb, defaultPrevented);
        currentAbsorb = tempAbsorb;

        if (defaultPrevented)
            continue;

        // absorb must be smaller than the damage itself
        currentAbsorb = RoundToInterval(currentAbsorb, 0, int32(damageInfo.GetDamage()));

        int32 manaReduction = currentAbsorb;

        // lower absorb amount by talents
        if (float manaMultiplier = absorbAurEff->GetSpellInfo()->Effects[absorbAurEff->GetEffIndex()].CalcValueMultiplier(absorbAurEff->GetCaster()))
            manaReduction = int32(float(manaReduction) * manaMultiplier);

        int32 manaTaken = -damageInfo.GetVictim()->ModifyPower(POWER_MANA, -manaReduction);

        // take case when mana has ended up into account
        currentAbsorb = currentAbsorb ? int32(float(currentAbsorb) * (float(manaTaken) / float(manaReduction))) : 0;

        damageInfo.AbsorbDamage(currentAbsorb);

        tempAbsorb = currentAbsorb;
        absorbAurEff->GetBase()->CallScriptEffectAfterManaShieldHandlers(absorbAurEff, aurApp, damageInfo, tempAbsorb);

        // Check if our aura is using amount to count damage
        if (absorbAurEff->GetAmount() >= 0)
        {
            absorbAurEff->SetAmount(absorbAurEff->GetAmount() - currentAbsorb);
            if ((absorbAurEff->GetAmount() <= 0))
                absorbAurEff->GetBase()->Remove(AURA_REMOVE_BY_ENEMY_SPELL);
        }
    }

#ifdef LICH_KING
    damageInfo.ModifyDamage(absorbIgnoringDamage);
#endif

    // split damage auras - only when not damaging self
    if (damageInfo.GetVictim() != damageInfo.GetAttacker())
    {
        // We're going to call functions which can modify content of the list during iteration over it's elements
        // Let's copy the list so we can prevent iterator invalidation
        AuraEffectList vSplitDamageFlatCopy(damageInfo.GetVictim()->GetAuraEffectsByType(SPELL_AURA_SPLIT_DAMAGE_FLAT));
        for (AuraEffectList::iterator itr = vSplitDamageFlatCopy.begin(); (itr != vSplitDamageFlatCopy.end()) && (damageInfo.GetDamage() > 0); ++itr)
        {
            // Check if aura was removed during iteration - we don't need to work on such auras
            /*if (!((*itr)->GetBase()->IsAppliedOnTarget(damageInfo.GetVictim()->GetGUID())))
                continue;*/
            // check damage school mask
            if (!((*itr)->GetMiscValue() & damageInfo.GetSchoolMask()))
                continue;

            // Damage can be splitted only if aura has an alive caster
            Unit* caster = (*itr)->GetCaster();
            if (!caster || (caster == damageInfo.GetVictim()) || !caster->IsInWorld() || !caster->IsAlive())
                continue;

            int32 splitDamage = (*itr)->GetAmount();

            // absorb must be smaller than the damage itself
            splitDamage = RoundToInterval(splitDamage, 0, int32(damageInfo.GetDamage()));

            damageInfo.AbsorbDamage(splitDamage);

            // check if caster is immune to damage
            if (caster->IsImmunedToDamage(damageInfo.GetSchoolMask()))
            {
                damageInfo.GetVictim()->SendSpellMiss(caster, (*itr)->GetSpellInfo()->Id, SPELL_MISS_IMMUNE);
                continue;
            }

            uint32 splitted = splitDamage;
            uint32 splitted_absorb = 0;
            Unit::DealDamageMods(caster, splitted, &splitted_absorb);

            if (Unit* attacker = damageInfo.GetAttacker())
                attacker->SendSpellNonMeleeDamageLog(caster, (*itr)->GetSpellInfo()->Id, splitted, damageInfo.GetSchoolMask(), splitted_absorb, 0, false, 0, false);

            CleanDamage cleanDamage = CleanDamage(splitted, 0, BASE_ATTACK, MELEE_HIT_NORMAL);
            Unit::DealDamage(damageInfo.GetAttacker(), caster, splitted, &cleanDamage, DIRECT_DAMAGE, damageInfo.GetSchoolMask(), (*itr)->GetSpellInfo(), false);

            // sun: also proc here
            Unit::ProcSkillsAndAuras(damageInfo.GetAttacker(), caster, PROC_FLAG_NONE, PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG, PROC_SPELL_TYPE_DAMAGE, PROC_SPELL_PHASE_HIT, PROC_HIT_NONE, nullptr, &damageInfo, nullptr);
        }

        // We're going to call functions which can modify content of the list during iteration over it's elements
        // Let's copy the list so we can prevent iterator invalidation
        AuraEffectList vSplitDamagePctCopy(damageInfo.GetVictim()->GetAuraEffectsByType(SPELL_AURA_SPLIT_DAMAGE_PCT));
        for (AuraEffectList::iterator itr = vSplitDamagePctCopy.begin(); itr != vSplitDamagePctCopy.end() && damageInfo.GetDamage() > 0; ++itr)
        {
            // Check if aura was removed during iteration - we don't need to work on such auras
            AuraApplication const* aurApp = (*itr)->GetBase()->GetApplicationOfTarget(damageInfo.GetVictim()->GetGUID());
            if (!aurApp)
                continue;

            // check damage school mask
            if (!((*itr)->GetMiscValue() & damageInfo.GetSchoolMask()))
                continue;

            // Damage can be splitted only if aura has an alive caster
            Unit* caster = (*itr)->GetCaster();
            if (!caster || (caster == damageInfo.GetVictim()) || !caster->IsInWorld() || !caster->IsAlive())
                continue;

            uint32 splitDamage = CalculatePct(damageInfo.GetDamage(), (*itr)->GetAmount());

            (*itr)->GetBase()->CallScriptEffectSplitHandlers((*itr), aurApp, damageInfo, splitDamage);

            // absorb must be smaller than the damage itself
            splitDamage = RoundToInterval(splitDamage, uint32(0), uint32(damageInfo.GetDamage()));

            damageInfo.AbsorbDamage(splitDamage);

            // check if caster is immune to damage
            if (caster->IsImmunedToDamage(damageInfo.GetSchoolMask()))
            {
                damageInfo.GetVictim()->SendSpellMiss(caster, (*itr)->GetSpellInfo()->Id, SPELL_MISS_IMMUNE);
                continue;
            }

            uint32 split_absorb = 0;
            Unit::DealDamageMods(caster, splitDamage, &split_absorb);

            if (Unit* attacker = damageInfo.GetAttacker())
                attacker->SendSpellNonMeleeDamageLog(caster, (*itr)->GetSpellInfo()->Id, splitDamage, damageInfo.GetSchoolMask(), split_absorb, 0, false, 0, false);

            CleanDamage cleanDamage = CleanDamage(splitDamage, 0, BASE_ATTACK, MELEE_HIT_NORMAL);
            Unit::DealDamage(damageInfo.GetAttacker(), caster, splitDamage, &cleanDamage, DIRECT_DAMAGE, damageInfo.GetSchoolMask(), (*itr)->GetSpellInfo(), false);

            // break 'Fear' and similar auras
            Unit::ProcSkillsAndAuras(damageInfo.GetAttacker(), caster, PROC_FLAG_NONE, PROC_FLAG_TAKEN_SPELL_MAGIC_DMG_CLASS_NEG, PROC_SPELL_TYPE_DAMAGE, PROC_SPELL_PHASE_HIT, PROC_HIT_NONE, nullptr, &damageInfo, nullptr);
        }
    }
}

void Unit::AttackerStateUpdate(Unit* victim, WeaponAttackType attType, bool extra )
{
    if (ToPlayer() && ToPlayer()->isSpectator())
        return;

    if((!extra && HasUnitState(UNIT_STATE_CANNOT_AUTOATTACK)) || HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED) )
        return;

    if (!victim->IsAlive())
        return;

    if(attType == BASE_ATTACK && sWorld->getConfig(CONFIG_DEBUG_DISABLE_MAINHAND))
        return;

    // disable this check for performance boost ?
    if ((attType == BASE_ATTACK || attType == OFF_ATTACK) && !IsWithinLOSInMap(victim, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
        return;
        
    AttackedTarget(victim, true);
    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_MELEE_ATTACK);

    if (attType != BASE_ATTACK && attType != OFF_ATTACK)
        return;                                             // ignore ranged case

    //sun: could we reduce this SetFacingToObject spam? One move packet at each attacker state update
    if (GetTypeId() == TYPEID_UNIT && !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED) 
#ifdef LICH_KING
        && !HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN)
#endif
        )
        SetFacingToObject(victim, false); // update client side facing to face the target (prevents visual glitches when casting untargeted spells)
    
    // melee attack spell cast at main hand attack only - no normal melee dmg dealt
    if (!extra && attType == BASE_ATTACK && GetCurrentSpell(CURRENT_MELEE_SPELL))
    {
        GetCurrentSpell(CURRENT_MELEE_SPELL)->cast();
        return;
    }

    // attack can be redirected to another target
    victim = GetMeleeHitRedirectTarget(victim);

    CalcDamageInfo damageInfo;
    CalculateMeleeDamage(victim, &damageInfo, attType);
    // Send log damage message to client

    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        Unit::DealDamageMods(victim, damageInfo.Damages[i].Damage, &damageInfo.Damages[i].Absorb);
    SendAttackStateUpdate(&damageInfo);

    if(damageInfo.HitInfo == MELEE_HIT_PARRY && victim->GetTypeId() == TYPEID_UNIT)
        victim->HandleParryRush();

    DealMeleeDamage(&damageInfo, true);

    DamageInfo dmgInfo(damageInfo);
    Unit::ProcSkillsAndAuras(damageInfo.Attacker, damageInfo.Target, damageInfo.ProcAttacker, damageInfo.ProcVictim, PROC_SPELL_TYPE_NONE, PROC_SPELL_PHASE_NONE, dmgInfo.GetHitMask(), nullptr, &dmgInfo, nullptr);

    if (GetTypeId() == TYPEID_PLAYER)
        TC_LOG_DEBUG("entities.unit", "AttackerStateUpdate: (Player) %u attacked %u (TypeId: %u) for %u dmg, absorbed %u, blocked %u, resisted %u.",
            GetGUID().GetCounter(), victim->GetGUID().GetCounter(), victim->GetTypeId(), dmgInfo.GetDamage(), dmgInfo.GetAbsorb(), dmgInfo.GetBlock(), dmgInfo.GetResist());
    else
        TC_LOG_DEBUG("entities.unit", "AttackerStateUpdate: (NPC)    %u attacked %u (TypeId: %u) for %u dmg, absorbed %u, blocked %u, resisted %u.",
            GetGUID().GetCounter(), victim->GetGUID().GetCounter(), victim->GetTypeId(), dmgInfo.GetDamage(), dmgInfo.GetAbsorb(), dmgInfo.GetBlock(), dmgInfo.GetResist());

    // HACK: Warrior enrage not losing procCharges when dealing melee damage
    if (GetTypeId() == TYPEID_PLAYER) 
    {
        uint32 enrageId = 0;
        if (HasAura(12880))
            enrageId = 12880;
        else if (HasAura(14201))
            enrageId = 14201;
        else if (HasAura(14202))
            enrageId = 14202;
        else if (HasAura(14203))
            enrageId = 14203;
        else if (HasAura(14204))
            enrageId = 14204;
            
        if (enrageId) 
            if (Aura* enrageAura = GetAura(enrageId, GetGUID())) 
            {
                enrageAura->ModCharges(-1);
            }
    }
}

MeleeHitOutcome Unit::RollMeleeOutcomeAgainst(const Unit* victim, WeaponAttackType attType) const
{
#ifdef TESTS
    if (_forceMeleeResult < MELEE_HIT_TOTAL)
        return _forceMeleeResult;
#endif
    if (victim->GetTypeId() == TYPEID_UNIT)
    {
        if (victim->ToCreature()->IsEvadingAttacks())
            return MELEE_HIT_EVADE;
        if (victim->IsTotem())
            return MELEE_HIT_NORMAL;
    }

    // Miss chance based on melee
    //float miss_chance = MeleeMissChanceCalc(pVictim, attType);
    int32 const attackerMaxSkillValueForLevel = GetMaxSkillValueForLevel(victim);
    int32 const victimMaxSkillValueForLevel = victim->GetMaxSkillValueForLevel(this);

    int32 const attackerWeaponSkill = GetWeaponSkillValue(attType, victim);
    int32 const victimDefenseSkill = victim->GetDefenseSkillValue(this);

    float miss_chance = MeleeSpellMissChance(victim, attType, attackerWeaponSkill - victimMaxSkillValueForLevel, 0);

    // Critical hit chance
    float crit_chance = GetUnitCriticalChanceAgainst(attType, victim);

    // stunned target cannot dodge and this is checked in GetUnitDodgeChance() (returned 0 in this case)
    float dodge_chance = GetUnitDodgeChance(attType, victim);
    float block_chance = GetUnitBlockChance(attType, victim);
    float parry_chance = GetUnitParryChance(attType, victim);

    // melee attack table implementation
    // outcome priority:
    //   1. >    2. >    3. >       4. >    5. >   6. >       7. >  8.
    // MISS > DODGE > PARRY > GLANCING > BLOCK > CRIT > CRUSHING > HIT
    // http://wowwiki.wikia.com/wiki/Attack_table?oldid=1432543

    float sum = 0, tmp = 0;
    float roll = float(rand_norm()) * 100.0f;

    // check if attack comes from behind, nobody can parry or block if attacker is behind
    bool canParryOrBlock = victim->HasInArc(float(M_PI), this) 
#ifdef LICH_KING
        || victim->HasAuraType(SPELL_AURA_IGNORE_HIT_DIRECTION)
#endif
    ;

    // only creatures can dodge if attacker is behind
    bool canDodge = victim->GetTypeId() != TYPEID_PLAYER || canParryOrBlock;

    // if victim is casting or cc'd it can't avoid attacks
    if (victim->IsNonMeleeSpellCast(false) || victim->HasUnitState(UNIT_STATE_CONTROLLED))
    {
        canDodge = false;
        canParryOrBlock = false;
    }

    // 1. MISS
    tmp = miss_chance;
    if (tmp > 0 && roll < (sum += tmp))
        return MELEE_HIT_MISS;

    // always crit against a sitting target (except 0 crit chance)
    if (victim->GetTypeId() == TYPEID_PLAYER && crit_chance > 0 && !victim->IsStandState())
        return MELEE_HIT_CRIT;

    // 2. DODGE
    if (canDodge)
    {
        tmp = dodge_chance;
        if (tmp > 0                                         // check if unit _can_ dodge
            && roll < (sum += tmp))
            return MELEE_HIT_DODGE;
    }

    // 3. PARRY
    if (canParryOrBlock)
    {
        tmp = parry_chance;
        if (tmp > 0                                         // check if unit _can_ parry
            && roll < (sum += tmp))
        {
            return MELEE_HIT_PARRY;
        }
    }

    // 4. GLANCING
    // Max 40% chance to score a glancing blow against mobs that are higher level (can do only players and pets and not with ranged weapon)
    if ((GetTypeId() == TYPEID_PLAYER || IsPet()) &&
        victim->GetTypeId() != TYPEID_PLAYER && !victim->IsPet() &&
        GetLevel() < victim->GetLevelForTarget(this)
        )
    {
        // cap possible value (with bonuses > max skill)
        int32 skill = attackerWeaponSkill;
        int32 maxskill = attackerMaxSkillValueForLevel;
        skill = (skill > maxskill) ? maxskill : skill;

        // against boss-level targets - 24% chance of 25% average damage reduction (damage reduction range : 20-30%)
        // against level 82 elites - 18% chance of 15% average damage reduction (damage reduction range : 10-20%)
        tmp = 6 + (victimDefenseSkill - skill) * 1.2;
        tmp = std::min(tmp, 40.0f); 
        if (tmp > 0 && roll < (sum += tmp))
            return MELEE_HIT_GLANCING;
    }

    // 5. BLOCK
    if (canParryOrBlock)
    {
        tmp = block_chance;
        if (tmp > 0                                          // check if unit _can_ block
            && roll < (sum += tmp))
            return MELEE_HIT_BLOCK;
    }

    // 6.CRIT
    tmp = crit_chance;
    if (tmp > 0 && roll < (sum += tmp))
        return MELEE_HIT_CRIT;

    // 7. CRUSHING
    // mobs can score crushing blows if they're 4 or more levels above victim
    if (GetLevelForTarget(victim) >= victim->GetLevelForTarget(this) + 4 &&
        // can be from by creature (if can) or from controlled player that considered as creature
        !IsControlledByPlayer() &&
        !(GetTypeId() == TYPEID_UNIT && ToCreature()->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_CRUSH)
        )
    {
        // when their weapon skill is 15 or more above victim's defense skill
        tmp = victimDefenseSkill;
        // having defense above your maximum (from items, talents etc.) has no effect
        tmp = std::min(tmp, float(victimMaxSkillValueForLevel));
        // tmp = mob's level * 5 - player's current defense skill
        tmp = attackerMaxSkillValueForLevel - tmp;
        // minimum of 20 points diff (4 levels difference)
        tmp = std::max(tmp, 20.0f);

        // add 2% chance per lacking skill point
        tmp = tmp * 2.0f - 15.0f;
        if (tmp > 0 && roll < (sum += tmp))
            return MELEE_HIT_CRUSHING;
    }

    // 8. HIT
    return MELEE_HIT_NORMAL;
}

uint32 Unit::CalculateDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, uint8 itemDamagesMask /*= 0*/) const
{
    float minDamage = 0.0f;
    float maxDamage = 0.0f;

    if (normalized || !addTotalPct || itemDamagesMask)
    {
        // get all by default (sun: adapted for BC)
        if (!itemDamagesMask)
            for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
                itemDamagesMask |= (1 << i);

        for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
        {
            if (itemDamagesMask & (1 << i))
            {
                float minTmp, maxTmp;
                CalculateMinMaxDamage(attType, normalized, addTotalPct, minTmp, maxTmp, i);
                minDamage += minTmp;
                maxDamage += maxTmp;
            }
        }
    }
    else
    {
        switch (attType)
        {
            case RANGED_ATTACK:
                minDamage = GetFloatValue(UNIT_FIELD_MINRANGEDDAMAGE);
                maxDamage = GetFloatValue(UNIT_FIELD_MAXRANGEDDAMAGE);
                break;
            case BASE_ATTACK:
                minDamage = GetFloatValue(UNIT_FIELD_MINDAMAGE);
                maxDamage = GetFloatValue(UNIT_FIELD_MAXDAMAGE);
                break;
            case OFF_ATTACK:
                minDamage = GetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE);
                maxDamage = GetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE);
                break;
        }
    }

    minDamage = std::max(0.f, minDamage);
    maxDamage = std::max(0.f, maxDamage);

    if (minDamage > maxDamage)
        std::swap(minDamage, maxDamage);

    return urand((uint32)minDamage, (uint32)maxDamage);
}

float Unit::CalculateSpellpowerCoefficientLevelPenalty(SpellInfo const* spellInfo) const
{
    if (spellInfo->SpellLevel <= 0 || spellInfo->MaxLevel == 0)
        return 1.0f;

#ifdef LICH_KING
    return std::max(0.0f, std::min(1.0f, (22.0f + spellInfo->MaxLevel - GetLevel()) / 20.0f));
#else
    // http://et.worldofwarcraft.wikia.com/wiki/Downranking
    // http://wowwiki.wikia.com/wiki/Spell_power?oldid=1576621
    float lvlPenalty = 0.0f; //range: 0 - 71.25
    if (spellInfo->SpellLevel < 20)
        lvlPenalty = (20.0f - spellInfo->SpellLevel) * 3.75f;
 		
    float lvlFactor = (float(spellInfo->MaxLevel) + 6.0f) / float(GetLevel());
 		
    return std::min(1.0f, (100.0f - lvlPenalty) * lvlFactor / 100.0f);
#endif
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
        TC_LOG_TRACE("entities.unit", "%s %u stopped attacking %s %u", (GetTypeId() == TYPEID_PLAYER ? "Player" : "Creature"), GetGUID().GetCounter(), (victim->GetTypeId() == TYPEID_PLAYER ? "player" : "creature"), victim->GetGUID().GetCounter());
    else
        TC_LOG_TRACE("entities.unit", "%s %u stopped attacking", (GetTypeId() == TYPEID_PLAYER ? "Player" : "Creature"), GetGUID().GetCounter());
}

bool Unit::IsSpellBlocked(Unit* victim, SpellInfo const *spellProto, WeaponAttackType attackType)
{
    // These spells can't be blocked
    if (spellProto && (spellProto->HasAttribute(SPELL_ATTR0_IMPOSSIBLE_DODGE_PARRY_BLOCK) || spellProto->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT)))
        return false;

    // Can't block when casting/controlled
    if (victim->IsNonMeleeSpellCast(false) || victim->HasUnitState(UNIT_STATE_CONTROLLED))
        return false;

    if (victim->HasInArc(M_PI,this)
#ifdef LICH_KING
        || victim->HasAuraType(SPELL_AURA_IGNORE_HIT_DIRECTION)
#endif
        )
    {
       float blockChance = victim->GetUnitBlockChance(attackType, victim);
       if (blockChance && roll_chance_f(blockChance))
           return true;
    }
    return false;
}

int32 Unit::GetMechanicResistChance(const SpellInfo *spell)
{
    if(!spell)
        return 0;

    int32 resist_mech = 0;
    for(int32 eff = 0; eff < MAX_SPELL_EFFECTS; ++eff)
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

// Melee based spells can be miss, parry or dodge on this step
// Crit or block - determined on damage calculation phase! (and can be both in some time)
// http://wowwiki.wikia.com/wiki/Weapon_skill
float Unit::MeleeSpellMissChance(const Unit* victim, WeaponAttackType attType, int32 skillDiff, uint32 spellId) const
{
    // Calculate hit chance (more correct for chance mod)
    float missChance = victim->GetUnitMissChance();

    // melee attacks while dual wielding have +19% chance to miss
    if (!spellId && HaveOffhandWeapon() && attType != RANGED_ATTACK)
        missChance += 19.0f;

    // bonus from skills is 0.04%
    int32 diff = -skillDiff;
    if (victim->GetTypeId() == TYPEID_PLAYER)
        missChance += diff > 0 ? diff * 0.04 : diff * 0.02;
    else
    {
        missChance += diff > 10 ? 2 + (diff - 10) * 0.4 : diff * 0.1; // http://wowwiki.wikia.com/wiki/Weapon_skill
                                                                      //further reduce miss chance for low levels
        float levelFactor = victim->GetLevelForTarget(this);
        if (levelFactor < 10.f)
            missChance *= (levelFactor / 10.f);
    }

    // Spellmod from SPELLMOD_RESIST_MISS_CHANCE - Example spell 28842
    if (spellId)
    {
        float resistMissChance = 100.0f;
        if (Player *modOwner = GetSpellModOwner())
            modOwner->ApplySpellMod(spellId, SPELLMOD_RESIST_MISS_CHANCE, resistMissChance);
        missChance -= resistMissChance - 100.0f;
    }

    // Bonuses from attacker aura and ratings
    if (attType == RANGED_ATTACK)
        missChance -= m_modRangedHitChance;
    else
        missChance -= m_modMeleeHitChance;

    // miss chance from auras after calculating skill based miss
    missChance -= GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);

    // miss chance from SPELL_AURA_MOD_ATTACKER_xxx_HIT_CHANCE can exceed 60% miss cap (eg aura 50240)
    if (attType == RANGED_ATTACK)
        missChance -= victim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_HIT_CHANCE);
    else
        missChance -= victim->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE);

    return std::max(missChance, 0.f);
}

// Melee based spells hit result calculations
SpellMissInfo Unit::MeleeSpellHitResult(Unit* victim, SpellInfo const* spellInfo) const
{
    WeaponAttackType attType = BASE_ATTACK;

    if (spellInfo->DmgClass == SPELL_DAMAGE_CLASS_RANGED)
        attType = RANGED_ATTACK;

    // bonus from skills is 0.04% per skill Diff
    int32 attackerWeaponSkill;
    // skill value for these spells is 5 * level (check which ones with "SELECT * FROM spell_template WHERE DmgClass = 3 AND NOT (attributes & 0x00000002) AND NOT (equippedItemSubClassMask & 327692)")
    if (spellInfo->DmgClass == SPELL_DAMAGE_CLASS_RANGED && !spellInfo->IsRangedWeaponSpell())
        attackerWeaponSkill = GetMaxSkillValueForLevel();
    else
        attackerWeaponSkill = int32(GetWeaponSkillValue(attType, victim));

    int32 skillDiff = attackerWeaponSkill - int32(victim->GetMaxSkillValueForLevel(this));

    uint32 roll = urand(0, 9999);

    uint32 missChance = uint32(MeleeSpellMissChance(victim, attType, skillDiff, spellInfo->Id)*100.0f);
    // Roll miss
    uint32 tmp = missChance;
    if (roll < tmp)
        return SPELL_MISS_MISS;

    // Same spells cannot be parried/dodged
    if (spellInfo->HasAttribute(SPELL_ATTR0_IMPOSSIBLE_DODGE_PARRY_BLOCK))
        return SPELL_MISS_NONE;

    bool canDodge = true;
    bool canParry = true;
    bool canBlock = spellInfo->HasAttribute(SPELL_ATTR3_BLOCKABLE_SPELL);

    // if victim is casting or cc'd it can't avoid attacks
    if (victim->IsNonMeleeSpellCast(false) || victim->HasUnitState(UNIT_STATE_CONTROLLED))
    {
        canDodge = false;
        canParry = false;
        canBlock = false;
    }

    // Ranged attacks can only miss, resist and deflect
    if (attType == RANGED_ATTACK)
    {
        // Wand attacks can't miss
        if (spellInfo->GetCategory() == 351)
            return SPELL_MISS_NONE;

        canParry = false;
        canDodge = false;

#ifdef LICH_KING
        // only if in front
        if (!victim->HasUnitState(UNIT_STATE_CONTROLLED) && (victim->HasInArc(float(M_PI), this) 
            || victim->HasAuraType(SPELL_AURA_IGNORE_HIT_DIRECTION))
            )
        {
            int32 deflect_chance = victim->GetTotalAuraModifier(SPELL_AURA_DEFLECT_SPELLS) * 100;
            tmp += deflect_chance;
            if (roll < tmp)
                return SPELL_MISS_DEFLECT;
        }
#endif
        return SPELL_MISS_NONE;
    }

    // Check for attack from behind
    if (!victim->HasInArc(float(M_PI), this) || spellInfo->HasAttribute(SPELL_ATTR2_BEHIND_TARGET)) //sunstrider: actually.. it seems on retail SPELL_ATTR2_BEHIND_TARGET could be parry/block/dodge on edge cases, but let's not allow that since it looks like a bug to players
    {
#ifdef LICH_KING
        if (!victim->HasAuraType(SPELL_AURA_IGNORE_HIT_DIRECTION))
        {
#endif
            // Can't dodge from behind in PvP (but its possible in PvE)
            if (victim->GetTypeId() == TYPEID_PLAYER)
                canDodge = false;
            // Can't parry or block
            canParry = false;
            canBlock = false;
#ifdef LICH_KING
        }
        else // Only deterrence as of 3.3.5
        {
            if (spellInfo->HasAttribute(SPELL_ATTR0_CU_REQ_CASTER_BEHIND_TARGET))
                canParry = false;
        }
#endif

    }
    // Ignore combat result aura
    AuraEffectList const& _ignore = GetAuraEffectsByType(SPELL_AURA_IGNORE_COMBAT_RESULT);
    for (AuraEffect const* aurEff : _ignore)
    {
        // -- sunstrider: added hack, not sure these spells have the proper spell affect in DB
        if (aurEff->GetMiscValue() == VICTIMSTATE_DODGE)       // can't be dodged rogue finishing move
        {
            if (spellInfo->SpellFamilyName == SPELLFAMILY_ROGUE && (spellInfo->SpellFamilyFlags & SPELLFAMILYFLAG_ROGUE_FINISHING_MOVE))
            {
                canDodge = false;
                break;
            }
        }
        // --

        if (!aurEff->IsAffectedOnSpell(spellInfo))
            continue;

        switch (aurEff->GetMiscValue())
        {
        case MELEE_HIT_DODGE:
            canDodge = false;
            break;
        case MELEE_HIT_BLOCK: //spell 44452 has this ("Devastate"), but this misses a spell_affect to work, but I'm not sure if this is used on retail.
            canBlock = false;
            break;
        case MELEE_HIT_PARRY:
            canParry = false;
            break;
        default:
            TC_LOG_DEBUG("entities.unit", "Spell %u SPELL_AURA_IGNORE_COMBAT_RESULT has unhandled state %d", aurEff->GetId(), aurEff->GetMiscValue());
            break;
        }
    }

    // Chance resist mechanic
    int32 resist_chance = victim->GetMechanicResistChance(spellInfo) *100;
    // Reduce spell hit chance for dispel mechanic spells from victim SPELL_AURA_MOD_DISPEL_RESIST
    if (IsDispelSpell(spellInfo))
        resist_chance += victim->GetTotalAuraModifier(SPELL_AURA_MOD_DISPEL_RESIST)*100;

    tmp += resist_chance;
    if (roll < tmp)
        return SPELL_MISS_RESIST;

    if (canDodge)
    {
        // Roll dodge
        int32 dodgeChance = int32(GetUnitDodgeChance(attType, victim) * 100.0f);
        if (dodgeChance < 0)
            dodgeChance = 0;

        if (roll < (tmp += dodgeChance))
            return SPELL_MISS_DODGE;
    }

    if (canParry)
    {
        // Roll parry
        int32 parryChance = int32(GetUnitParryChance(attType, victim) * 100.0f);
        if (parryChance < 0)
            parryChance = 0;

        tmp += parryChance;
        if (roll < tmp)
            return SPELL_MISS_PARRY;
    }

    if (canBlock)
    {
        int32 blockChance = int32(GetUnitBlockChance(attType, victim) * 100.0f);
        if (blockChance < 0)
            blockChance = 0;
        tmp += blockChance;

        if (roll < tmp)
            return SPELL_MISS_BLOCK;
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
        return GetMaxSkillValueForLevel(target);
}

float Unit::GetUnitDodgeChance(WeaponAttackType attType, Unit const* victim) const
{
    int32 const attackerWeaponSkill = GetWeaponSkillValue(attType, victim);
    int32 const victimMaxSkillValueForLevel = victim->GetMaxSkillValueForLevel(this);
    int32 const skillDiff = victimMaxSkillValueForLevel - attackerWeaponSkill;

    float chance = 0.0f;
    float skillBonus = 0.0f;
    if (victim->GetTypeId() == TYPEID_PLAYER)
    {
        chance = victim->GetFloatValue(PLAYER_DODGE_PERCENTAGE);
        skillBonus = 0.04f * skillDiff;
    } 
    else
    {
        if (!victim->IsTotem())
        {
            chance = 5.0f;
            chance += victim->GetTotalAuraModifier(SPELL_AURA_MOD_DODGE_PERCENT);

            if (skillDiff <= 10)
                skillBonus = skillDiff * 0.1f;
            else
                skillBonus = 1.0f + (skillDiff - 10) * 0.1f;
        }
    }  
    
    chance += skillBonus;

    // Reduce enemy dodge chance by SPELL_AURA_MOD_COMBAT_RESULT_CHANCE
    chance += GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_COMBAT_RESULT_CHANCE, VICTIMSTATE_DODGE);

    // reduce dodge by SPELL_AURA_MOD_ENEMY_DODGE
    chance += GetTotalAuraModifier(SPELL_AURA_MOD_ENEMY_DODGE);

    // Reduce dodge chance by attacker expertise rating
    if (GetTypeId() == TYPEID_PLAYER)
        chance -= ToPlayer()->GetExpertiseDodgeOrParryReduction(attType);
    else
        chance -= GetTotalAuraModifier(SPELL_AURA_MOD_EXPERTISE) / 4.0f;
    return std::max(chance, 0.0f);
}

float Unit::GetUnitParryChance(WeaponAttackType attType, Unit const* victim) const
{
    int32 const attackerWeaponSkill = GetWeaponSkillValue(attType, victim);
    int32 const victimMaxSkillValueForLevel = victim->GetMaxSkillValueForLevel(this);
    int32 const skillDiff = victimMaxSkillValueForLevel - attackerWeaponSkill;

    float chance = 0.0f;
    float skillBonus = 0.0f;

    if (Player const* playerVictim = victim->ToPlayer())
    {
        if (playerVictim->CanParry())
        {
            Item* tmpitem = playerVictim->GetWeaponForAttack(BASE_ATTACK, true);
            if (!tmpitem)
                tmpitem = playerVictim->GetWeaponForAttack(OFF_ATTACK, true);

            if (tmpitem)
                chance = playerVictim->GetFloatValue(PLAYER_PARRY_PERCENTAGE);

            skillBonus = 0.04f * skillDiff;
        }
    }
    else
    {
        if(victim->ToCreature()->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_PARRY || victim->IsTotem())
            chance = 0.0f;
        else if (victim->ToCreature()->IsWorldBoss()) // sun: Add some parry chance for bosses. Nobody seems to knows the exact rule but it's somewhere around 14%.
        {
            chance = 13.0f;
            chance += victim->GetTotalAuraModifier(SPELL_AURA_MOD_PARRY_PERCENT);
        }
        else if(GetCreatureType() != CREATURE_TYPE_BEAST)
        {
            chance = 5.0f;
            chance += victim->GetTotalAuraModifier(SPELL_AURA_MOD_PARRY_PERCENT);

            if (skillDiff <= 10)
                skillBonus = skillDiff * 0.1f;
            else
                skillBonus = 1.0f + (skillDiff - 10) * 1.6f;
        }

    }

    chance += skillBonus;

    // Reduce parry chance by attacker expertise rating
    if (GetTypeId() == TYPEID_PLAYER)
        chance -= ToPlayer()->GetExpertiseDodgeOrParryReduction(attType);
    else
        chance -= GetTotalAuraModifier(SPELL_AURA_MOD_EXPERTISE) / 4.0f;
    return std::max(chance, 0.0f);
}

float Unit::GetUnitMissChance() const
{
    float miss_chance = 5.0f;

    if (Player const* player = ToPlayer())
        miss_chance += player->GetMissPercentageFromDefense();

    return miss_chance;
}

float Unit::GetUnitBlockChance(WeaponAttackType attType, Unit const* victim) const
{
    int32 const attackerWeaponSkill = GetWeaponSkillValue(attType, victim);
    int32 const victimMaxSkillValueForLevel = victim->GetMaxSkillValueForLevel(this);
    int32 const skillDiff = victimMaxSkillValueForLevel - attackerWeaponSkill;

    float chance = 0.0f;
    float skillBonus = 0.0f;
    if (Player const* playerVictim = victim->ToPlayer())
    {
        if (playerVictim->CanBlock())
        {
            Item* tmpitem = playerVictim->GetUseableItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if (tmpitem && !tmpitem->IsBroken() && tmpitem->GetTemplate()->Block)
            {
                chance = playerVictim->GetFloatValue(PLAYER_BLOCK_PERCENTAGE);
                skillBonus = 0.04f * skillDiff;
            }
        }
    }
    else
    {
        if (!victim->IsTotem() && !(victim->ToCreature()->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_BLOCK))
        {
            chance = 5.0f;
            chance += victim->GetTotalAuraModifier(SPELL_AURA_MOD_BLOCK_PERCENT);

            if (skillDiff <= 10)
                skillBonus = skillDiff * 0.1f;
            else
                skillBonus = 1.0f + (skillDiff - 10) * 0.1f;
        }
    }

    chance += skillBonus;
    return std::max(chance, 0.0f);
}

float Unit::GetUnitCriticalChanceDone(WeaponAttackType attackType) const
{
    float chance = 0.f;
    if (GetTypeId() == TYPEID_PLAYER)
    {
        switch (attackType)
        {
        case BASE_ATTACK:
            chance = GetFloatValue(PLAYER_CRIT_PERCENTAGE);
            break;
        case OFF_ATTACK:
            chance = GetFloatValue(PLAYER_OFFHAND_CRIT_PERCENTAGE);
            break;
        case RANGED_ATTACK:
            chance = GetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE);
            break;
            // Just for good manner
        default:
            chance = 0.0f;
            break;
        }
    }
    else
    {
        if (!(ToCreature()->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_NO_CRIT))
        {
            chance = 5.0f;
            chance += GetTotalAuraModifier(SPELL_AURA_MOD_WEAPON_CRIT_PERCENT);
        }
    }
    return chance;
}

float Unit::GetUnitCriticalChanceTaken(Unit const* attacker, WeaponAttackType attackType, float critDone) const
{
    int32 const attackerWeaponSkill = attacker->GetWeaponSkillValue(attackType, this);
    int32 const victimDefenseSkill = GetDefenseSkillValue(this);
    int32 const skillDiff = victimDefenseSkill - attackerWeaponSkill;

    float chance = critDone;
    float skillBonus = 0.0f;

    // flat aura mods
    if (attackType == RANGED_ATTACK)
        chance += GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_RANGED_CRIT_CHANCE);
    else
        chance += GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_MELEE_CRIT_CHANCE);

#ifdef LICH_KING
    chance += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_CHANCE_FOR_CASTER, [this](AuraEffect const* aurEff) -> bool
    {
        if (aurEff->GetCasterGUID() == GetGUID())
            return true;
        return false;
    });
#endif

    // reduce crit chance from Rating for players
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (attackType == RANGED_ATTACK)
            chance -= (ToPlayer())->GetRatingBonusValue(CR_CRIT_TAKEN_RANGED);
        else
            chance -= (ToPlayer())->GetRatingBonusValue(CR_CRIT_TAKEN_MELEE);
    }

    chance += GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE);

    // Apply crit chance from defense skill
    if (GetTypeId() == TYPEID_PLAYER)
        skillBonus = -skillDiff * 0.04f;
    else
    {
        skillBonus = -skillDiff * 0.12f;
        if (skillDiff >= 15)
            skillBonus -= 3.0f;
    }

    chance += skillBonus;

    // always crit against a sitting target (except 0 crit chance)
    if (chance > 0 && !IsStandState())
        return 1000.0f;

    return std::max(chance, 0.0f);
}

float Unit::GetUnitCriticalChanceAgainst(WeaponAttackType attackType, Unit const* victim) const
{
    float chance = GetUnitCriticalChanceDone(attackType);
    return victim->GetUnitCriticalChanceTaken(this, attackType, chance);
}

uint32 Unit::GetWeaponSkillValue(WeaponAttackType attType, Unit const* target) const
{
    uint32 value = 0;
    if (Player const* player = ToPlayer())
    {
        Item* item = player->GetWeaponForAttack(attType,true);

        // feral or unarmed skill only for base attack
        if(attType != BASE_ATTACK && !item )
        {
            if(attType == RANGED_ATTACK && GetClass() == CLASS_PALADIN) //hammer
                return GetMaxSkillValueForLevel();
            return 0;
        }

        if(player->IsInFeralForm())
            return GetMaxSkillValueForLevel();              // always maximized SKILL_FERAL_COMBAT in fact

        // weapon skill or (unarmed for base attack)
        uint32 skill;
        if (item && item->GetSkill() != SKILL_FIST_WEAPONS)
            skill = item->GetSkill();
        else
            skill = SKILL_UNARMED;

        // in PvP use full skill instead current skill value
        value = (target && target->IsCharmedOwnedByPlayerOrPlayer())
            ? player->GetMaxSkillValue(skill)
            : player->GetSkillValue(skill);
        // Modify value from ratings
        value += uint32(player->GetRatingBonusValue(CR_WEAPON_SKILL));
        switch (attType)
        {
            case BASE_ATTACK:   
                value+=uint32(player->GetRatingBonusValue(CR_WEAPON_SKILL_MAINHAND));
                break;
            case OFF_ATTACK:    
                value+=uint32(player->GetRatingBonusValue(CR_WEAPON_SKILL_OFFHAND));
                break;
            case RANGED_ATTACK: 
                value+=uint32(player->GetRatingBonusValue(CR_WEAPON_SKILL_RANGED));
                break;
            default: 
                break;
        }
    }
    else
        value = GetMaxSkillValueForLevel(target);

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

            switch (index)
            {
            case UNIT_FIELD_HEALTH:
            {
                //for creatures, send 0 health. This prevents health from showing in the bottom right tooltip when mouse hovering over the creature
                if (GetTypeId() == TYPEID_UNIT && m_uint32Values[UNIT_DYNAMIC_FLAGS] & UNIT_DYNFLAG_DEAD)
                    fieldBuffer << uint32(0);
                else
                    fieldBuffer << m_uint32Values[index];

            } break;
            case UNIT_NPC_FLAGS:
            {
                uint32 appendValue = m_uint32Values[UNIT_NPC_FLAGS];

#ifdef LICH_KING
                if (creature)
                    if (!target->CanSeeSpellClickOn(creature))
                        appendValue &= ~UNIT_NPC_FLAG_SPELLCLICK;
#endif

                fieldBuffer << uint32(appendValue);
            } break;
            case UNIT_FIELD_AURASTATE:
            {
                // Check per caster aura states to not enable using a spell in client if specified aura is not by target
                fieldBuffer << BuildAuraStateUpdateForTarget(target);
            } break;
            // FIXME: Some values at server stored in float format but must be sent to client in uint32 format
            case UNIT_FIELD_BASEATTACKTIME:
            case UNIT_FIELD_BASEATTACKTIME+1:
            case UNIT_FIELD_RANGEDATTACKTIME:
            {
                // convert from float to uint32 and send
                fieldBuffer << uint32(m_floatValues[index] < 0 ? 0 : m_floatValues[index]);
            } break;
            // there are some float values which may be negative or can't get negative due to other checks
            case UNIT_FIELD_NEGSTAT0:
            case UNIT_FIELD_NEGSTAT1:
            case UNIT_FIELD_NEGSTAT2:
            case UNIT_FIELD_NEGSTAT3:
            case UNIT_FIELD_NEGSTAT4:
            case UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE:
            case UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + 1:
            case UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + 2:
            case UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + 3:
            case UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + 4:
            case UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + 5:
            case UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + 6:
            case UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE:
            case UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + 1:
            case UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + 2:
            case UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + 3:
            case UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + 4:
            case UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + 5:
            case UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + 6:
            case UNIT_FIELD_POSSTAT0:
            case UNIT_FIELD_POSSTAT1:
            case UNIT_FIELD_POSSTAT2:
            case UNIT_FIELD_POSSTAT3:
            case UNIT_FIELD_POSSTAT4:
            {
                fieldBuffer << uint32(m_floatValues[index]);
            } break;
            // Gamemasters should be always able to select units - remove not selectable flag
            case UNIT_FIELD_FLAGS:;
            {
                uint32 appendValue = m_uint32Values[UNIT_FIELD_FLAGS];
                if (target->IsGameMaster())
                    appendValue &= ~UNIT_FLAG_NOT_SELECTABLE;

                fieldBuffer << uint32(appendValue);
            } break;
            // use modelid_a if not gm, _h if gm for CREATURE_FLAG_EXTRA_TRIGGER creatures
            case UNIT_FIELD_DISPLAYID:
            {
                uint32 displayId = m_uint32Values[UNIT_FIELD_DISPLAYID];
                if (creature)
                {
                    CreatureTemplate const* cinfo = creature->GetCreatureTemplate();

                    // this also applies for transform auras
                    if (SpellInfo const* transform = sSpellMgr->GetSpellInfo(GetTransForm()))
                        for (const auto & Effect : transform->Effects)
                            if (Effect.ApplyAuraName == SPELL_AURA_TRANSFORM)
                                if (CreatureTemplate const* transformInfo = sObjectMgr->GetCreatureTemplate(Effect.MiscValue))
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
            } break;
            // hide lootable animation for unallowed players
            case UNIT_DYNAMIC_FLAGS:
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
            } break;
            // FG: pretend that OTHER players in own group are friendly ("blue")
            case UNIT_FIELD_BYTES_2:
            case UNIT_FIELD_FACTIONTEMPLATE:
            {
                if (/* IsControlledByPlayer() && */ target != this && target->GetCharmerOrOwnerGUID().IsPlayer() && sWorld->getBoolConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP) && IsInRaidWith(target))
                {
                    FactionTemplateEntry const* ft1 = GetFactionTemplateEntry();
                    FactionTemplateEntry const* ft2 = target->GetFactionTemplateEntry();
                    if (ft1 && ft2 && !ft1->IsFriendlyTo(*ft2))
                    {
                        if (index == UNIT_FIELD_BYTES_2)
                            // Allow targetting opposite faction in party when enabled in config
                            fieldBuffer << (m_uint32Values[UNIT_FIELD_BYTES_2] & ((UNIT_BYTE2_FLAG_UNK3) << 8)); // this flag is at uint8 offset 1 !!
                        else
                            // pretend that all other HOSTILE players have own faction, to allow follow, heal, rezz (trade wont work)
                            fieldBuffer << uint32(target->GetFaction());
                    }
                    else
                        fieldBuffer << m_uint32Values[index];
                }
                else
                    fieldBuffer << m_uint32Values[index];
            } break;
            default:
            {
                // send in current format (float as float, uint32 as uint32)
                fieldBuffer << m_uint32Values[index];
            } break;
            }
            
        }
    }

    *data << uint8(updateMask.GetBlockCount());
    updateMask.AppendToPacket(data);
    data->append(fieldBuffer);
}

int32 Unit::GetHighestExclusiveSameEffectSpellGroupValue(AuraEffect const* aurEff, AuraType auraType, bool checkMiscValue /*= false*/, int32 miscValue /*= 0*/) const
{
    int32 val = 0;
    SpellSpellGroupMapBounds spellGroup = sSpellMgr->GetSpellSpellGroupMapBounds(aurEff->GetSpellInfo()->GetFirstRankSpell()->Id);
    for (SpellSpellGroupMap::const_iterator itr = spellGroup.first; itr != spellGroup.second; ++itr)
    {
        if (sSpellMgr->GetSpellGroupStackRule(itr->second) == SPELL_GROUP_STACK_RULE_EXCLUSIVE_SAME_EFFECT)
        {
            AuraEffectList const& auraEffList = GetAuraEffectsByType(auraType);
            for (AuraEffectList::const_iterator auraItr = auraEffList.begin(); auraItr != auraEffList.end(); ++auraItr)
            {
                if (aurEff != (*auraItr) && (!checkMiscValue || (*auraItr)->GetMiscValue() == miscValue) &&
                    sSpellMgr->IsSpellMemberOfSpellGroup((*auraItr)->GetSpellInfo()->Id, itr->second))
                {
                    // absolute value only
                    if (abs(val) < abs((*auraItr)->GetAmount()))
                        val = (*auraItr)->GetAmount();
                }
            }
        }
    }
    return val;
}

bool Unit::IsHighestExclusiveAura(Aura const* aura, bool removeOtherAuraApplications /*= false*/)
{
    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (AuraEffect const* aurEff = aura->GetEffect(i))
        {
            AuraType const auraType = AuraType(aura->GetSpellInfo()->Effects[i].ApplyAuraName);
            AuraEffectList const& auras = GetAuraEffectsByType(auraType);
            for (Unit::AuraEffectList::const_iterator itr = auras.begin(); itr != auras.end();)
            {
                AuraEffect const* existingAurEff = (*itr);
                ++itr;
                if (existingAurEff->GetEffIndex() != i) //sun: do not compare if effect idx are differents (19977 for exemple has two SPELL_AURA_DUMMY that would get compared)
                    continue;

                //sunstrider: exclude by default if same spell and can't stack
                auto exclusiveByDefault = [&]() 
                {
                    return aura->GetSpellInfo()->IsRankOf(existingAurEff->GetBase()->GetSpellInfo())
                        && !aura->CanStackWith(existingAurEff->GetBase());
                };

                if ( existingAurEff->GetBase()->GetCasterGUID() != aura->GetCasterGUID()  //allow replacing your own auras in all cases
                    && (   sSpellMgr->CheckSpellGroupStackRules(aura->GetSpellInfo(), existingAurEff->GetSpellInfo()) == SPELL_GROUP_STACK_RULE_EXCLUSIVE_HIGHEST
                        || exclusiveByDefault()  )
                   )
                {
                    int32 diff = abs(aurEff->GetAmount()) - abs(existingAurEff->GetAmount());
                    if (!diff)
                        diff = int32(aura->GetEffectMask()) - int32(existingAurEff->GetBase()->GetEffectMask());

                    if (diff > 0)
                    {
                        Aura const* base = existingAurEff->GetBase();
                        // no removing of area auras from the original owner, as that completely cancels them
                        if (removeOtherAuraApplications && (!base->IsArea() || base->GetOwner() != this))
                        {
                            if (AuraApplication* aurApp = existingAurEff->GetBase()->GetApplicationOfTarget(GetGUID()))
                            {
                                bool hasMoreThanOneEffect = base->HasMoreThanOneEffectForType(auraType);
                                uint32 removedAuras = m_removedAurasCount;
                                RemoveAura(aurApp);
                                if (hasMoreThanOneEffect || m_removedAurasCount > removedAuras + 1)
                                    itr = auras.begin();
                            }
                        }
                    }
                    else if (diff < 0)
                        return false;
                }
            }
        }
    }

    return true;
}

void Unit::_DeleteRemovedAuras()
{
    while(!m_removedAuras.empty())
    {
        delete m_removedAuras.front();
        m_removedAuras.pop_front();
    }

    m_removedAurasCount = 0;
}

void Unit::_UpdateSpells(uint32 diff)
{
    if(GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL))
        _UpdateAutoRepeatSpell();

    // remove finished spells from current pointers
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; i++)
    {
        if (GetCurrentSpell(i) && GetCurrentSpell(i)->getState() == SPELL_STATE_FINISHED)
        {
            GetCurrentSpell(i)->SetReferencedFromCurrent(false);
            m_currentSpells[i] = nullptr;                      // remove pointer
        }
    }

    // m_auraUpdateIterator can be updated in indirect called code at aura remove to skip next planned to update but removed auras
    for (m_auraUpdateIterator = m_ownedAuras.begin(); m_auraUpdateIterator != m_ownedAuras.end();)
    {
        Aura* i_aura = m_auraUpdateIterator->second;
        ++m_auraUpdateIterator;                            // need shift to next for allow update if need into aura update
        i_aura->UpdateOwner(diff, this);
    }

    // remove expired auras - do that after updates(used in scripts?)
    for (AuraMap::iterator i = m_ownedAuras.begin(); i != m_ownedAuras.end();)
    {
        if (i->second->IsExpired())
            RemoveOwnedAura(i, AURA_REMOVE_BY_EXPIRE);
        else
            ++i;
    }

    for (VisibleAuraMap::iterator itr = m_visibleAuras.begin(); itr != m_visibleAuras.end(); ++itr)
        if (itr->second->IsNeedClientUpdate())
            itr->second->ClientUpdate();

    _DeleteRemovedAuras();

    if(!m_gameObj.empty())
    {
        std::list<GameObject*>::iterator ite1, dnext1;
        for (ite1 = m_gameObj.begin(); ite1 != m_gameObj.end(); ite1 = dnext1)
        {
            dnext1 = ite1;
            //(*i)->Update( difftime );
            if( !(*ite1)->isSpawned() )
            {
                (*ite1)->SetOwnerGUID(ObjectGuid::Empty);
                (*ite1)->SetRespawnTime(0);
                (*ite1)->Delete();
                dnext1 = m_gameObj.erase(ite1);
            }
            else
                ++dnext1;
        }
    }

    m_spellHistory->Update();
}

void Unit::_UpdateAutoRepeatSpell()
{
    const SpellInfo* autoRepeatSpellInfo = m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo;

    //check "realtime" interrupts
    if ( (GetTypeId() == TYPEID_PLAYER && (this->ToPlayer())->isMoving()) || IsNonMeleeSpellCast(false,false,true) )
    {
        // cancel wand shoot
        if(autoRepeatSpellInfo->GetCategory() == 351)
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
        if(GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->CheckCast(true) != SPELL_CAST_OK)
        {
            InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
            return;
        }

        // we want to shoot
        auto spell = new Spell(this, autoRepeatSpellInfo, TRIGGERED_FULL_MASK);
        spell->prepare(GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->m_targets);

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
        && (withInstant || spell->GetCastTime() > 0 || spell->getState() == SPELL_STATE_CASTING))
    {
        // for example, do not let self-stun aura interrupt itself
        if(!spell->IsInterruptable())
            return;

        // send autorepeat cancel message for autorepeat spells
        if (spellType == CURRENT_AUTOREPEAT_SPELL)
        {
            if(GetTypeId()==TYPEID_PLAYER)
                (this->ToPlayer())->SendAutoRepeatCancel();
        }
        
        if (spell->getState() != SPELL_STATE_FINISHED)
            spell->cancel();
        else
        {
            m_currentSpells[spellType] = nullptr;
            spell->SetReferencedFromCurrent(false);
        }
    }
}

bool Unit::HasDelayedSpell()
{
    if ( m_currentSpells[CURRENT_GENERIC_SPELL] &&
        (m_currentSpells[CURRENT_GENERIC_SPELL]->getState() == SPELL_STATE_DELAYED) )
        return true;

    return false;
}

void Unit::FinishSpell(CurrentSpellTypes spellType, bool ok /*= true*/)
{
    Spell* spell = m_currentSpells[spellType];
    if (!spell)
        return;

    if (spellType == CURRENT_CHANNELED_SPELL)
        spell->SendChannelUpdate(0);

    spell->finish(ok);
}

bool Unit::IsNonMeleeSpellCast(bool withDelayed, bool skipChanneled, bool skipAutorepeat, bool isAutoshoot) const
{
    // We don't do loop here to explicitly show that melee spell is excluded.
    // Maybe later some special spells will be excluded too.

    // generic spells are casted when they are not finished and not delayed
    if (m_currentSpells[CURRENT_GENERIC_SPELL] &&
        (m_currentSpells[CURRENT_GENERIC_SPELL]->getState() != SPELL_STATE_FINISHED) &&
        (withDelayed || m_currentSpells[CURRENT_GENERIC_SPELL]->getState() != SPELL_STATE_DELAYED))
    {
        if (!isAutoshoot || !(m_currentSpells[CURRENT_GENERIC_SPELL]->m_spellInfo->HasAttribute(SPELL_ATTR2_NOT_RESET_AUTO_ACTIONS)))
            return true;
    }
    // channeled spells may be delayed, but they are still considered casted
    else if (!skipChanneled && GetCurrentSpell(CURRENT_CHANNELED_SPELL) &&
        (GetCurrentSpell(CURRENT_CHANNELED_SPELL)->getState() != SPELL_STATE_FINISHED))
    {
        if (!isAutoshoot || !(m_currentSpells[CURRENT_CHANNELED_SPELL]->m_spellInfo->HasAttribute(SPELL_ATTR2_NOT_RESET_AUTO_ACTIONS)))
            return true;
    }
    // autorepeat spells may be finished or delayed, but they are still considered casted
    else if ( !skipAutorepeat && GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL) )
        return true;

    return false;
}

void Unit::InterruptNonMeleeSpells(bool withDelayed, uint32 spell_id, bool withInstant)
{
    // generic spells are interrupted if they are not finished or delayed
    if (GetCurrentSpell(CURRENT_GENERIC_SPELL) && (!spell_id || GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id==spell_id))
        InterruptSpell(CURRENT_GENERIC_SPELL,withDelayed,withInstant);

    // autorepeat spells are interrupted if they are not finished or delayed
    if (GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL) && (!spell_id || GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->m_spellInfo->Id==spell_id))
        InterruptSpell(CURRENT_AUTOREPEAT_SPELL,withDelayed,withInstant);

    // channeled spells are interrupted if they are not finished, even if they are delayed
    if (GetCurrentSpell(CURRENT_CHANNELED_SPELL) && (!spell_id || GetCurrentSpell(CURRENT_CHANNELED_SPELL)->m_spellInfo->Id==spell_id))
        InterruptSpell(CURRENT_CHANNELED_SPELL,true,true);
}

Spell* Unit::FindCurrentSpellBySpellId(uint32 spell_id) const
{
    for (uint32 i = 0; i < CURRENT_MAX_SPELL; i++)
        if(GetCurrentSpell(i) && GetCurrentSpell(i)->m_spellInfo->Id==spell_id)
            return GetCurrentSpell(i);
    return nullptr;
}

bool Unit::IsMovementPreventedByCasting() const
{
    // can always move when not casting
    if (!HasUnitState(UNIT_STATE_CASTING))
        return false;

    // channeled spells during channel stage (after the initial cast timer) allow movement with a specific spell attribute
    if (Spell* spell = m_currentSpells[CURRENT_CHANNELED_SPELL])
        if (spell->getState() != SPELL_STATE_FINISHED && spell->IsChannelActive())
            if (spell->GetSpellInfo()->IsMoveAllowedChannel())
                return false;

    // prohibit movement for all other spell casts
    return true;
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
        if (c->GetOwnerGUID().IsPlayer() && !c->CanFly())
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
    //for creatures, we only update water position when IsInWater in queried
    if (GetExactDistSq(&_lastInWaterCheckPosition) > 2.5f*2.5f)
        const_cast<Unit*>(this)->UpdatePositionData(true);

    return _last_in_water_status;
}

bool Unit::IsUnderWater() const
{
    //for creatures, we only update water position when IsUnderWater in queried
    if (GetExactDistSq(&_lastInWaterCheckPosition) > 2.5f*2.5f)
        const_cast<Unit*>(this)->UpdatePositionData(true);

    return _last_isunderwater_status;
}

void Unit::ProcessPositionDataChanged(PositionFullTerrainStatus const& data, bool updateCreatureLiquid)
{
    WorldObject::ProcessPositionDataChanged(data);
    ProcessTerrainStatusUpdate(data.liquidStatus, data.liquidInfo, updateCreatureLiquid);
}

void Unit::ProcessTerrainStatusUpdate(ZLiquidStatus status, Optional<LiquidData> const& liquidData, bool updateCreatureLiquid)
{
    if (IsFlying() || (!updateCreatureLiquid && !IsControlledByPlayer()))
        return;

    // remove appropriate auras if we are swimming/not swimming respectively
    if (status & MAP_LIQUID_STATUS_SWIMMING)
        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_ABOVEWATER);
    else
        RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_NOT_UNDERWATER);

    // liquid aura handling
    LiquidTypeEntry const* curLiquid = nullptr;
    if ((status & MAP_LIQUID_STATUS_SWIMMING) && liquidData)
        curLiquid = sLiquidTypeStore.LookupEntry(liquidData->entry);
    if (curLiquid != _lastLiquid)
    {
        if (_lastLiquid && _lastLiquid->SpellId)
            RemoveAurasDueToSpell(_lastLiquid->SpellId);
        Player* player = GetCharmerOrOwnerPlayerOrPlayerItself();
        if (curLiquid && curLiquid->SpellId && (!player || !player->IsGameMaster()))
            CastSpell(this, curLiquid->SpellId, true);
        _lastLiquid = curLiquid;
    }

    if(liquidData)
    {
        bool enoughWater = (liquidData->level > INVALID_HEIGHT && liquidData->level > liquidData->depth_level && liquidData->level - liquidData->depth_level >= 1.5f); // also check if theres enough water - at least 2yd
        _last_in_water_status = status >= LIQUID_MAP_IN_WATER && enoughWater;
        _last_isunderwater_status = status == LIQUID_MAP_UNDER_WATER && enoughWater;
    }
    else {
        _last_in_water_status = false;
        _last_isunderwater_status = false;
    }
    _lastInWaterCheckPosition = GetPosition();
}

void Unit::DeMorph()
{
    SetDisplayId(GetNativeDisplayId());
}

void Unit::_RegisterDynObject(DynamicObject* dynObj)
{
    m_dynObj.push_back(dynObj);
}

void Unit::_UnregisterDynObject(DynamicObject* dynObj)
{
    m_dynObj.remove(dynObj);
}

DynamicObject* Unit::GetDynObject(uint32 spellId)
{
    if (m_dynObj.empty())
        return nullptr;
    for (DynObjectList::const_iterator i = m_dynObj.begin(); i != m_dynObj.end(); ++i)
    {
        DynamicObject* dynObj = *i;
        if (dynObj->GetSpellId() == spellId)
            return dynObj;
    }
    return nullptr;
}

void Unit::RemoveDynObject(uint32 spellId)
{
    if (m_dynObj.empty())
        return;
    for (DynObjectList::iterator i = m_dynObj.begin(); i != m_dynObj.end();)
    {
        DynamicObject* dynObj = *i;
        if (dynObj->GetSpellId() == spellId)
        {
            dynObj->Remove();
            i = m_dynObj.begin();
        }
        else
            ++i;
    }
}

void Unit::RemoveAllDynObjects()
{
    while (!m_dynObj.empty())
        m_dynObj.front()->Remove();
}

void Unit::AddGameObject(GameObject* gameObj)
{
    assert(gameObj && gameObj->GetOwnerGUID()==0);
    m_gameObj.push_back(gameObj);
    gameObj->SetOwnerGUID(GetGUID());

    if (gameObj->GetSpellId())
    {
        SpellInfo const* createBySpell = sSpellMgr->GetSpellInfo(gameObj->GetSpellId());
        // Need disable spell use for owner
        if (createBySpell && createBySpell->IsCooldownStartedOnEvent())
            // note: item based cooldowns and cooldown spell mods with charges ignored (unknown existing cases)
            GetSpellHistory()->StartCooldown(createBySpell, 0, nullptr, true);
    }
}

void Unit::RemoveGameObject(GameObject* gameObj, bool del)
{
    assert(gameObj && gameObj->GetOwnerGUID()==GetGUID());

    // GO created by some spell
    if ( GetTypeId()==TYPEID_PLAYER && gameObj->GetSpellId() )
    {
        SpellInfo const* createBySpell = sSpellMgr->GetSpellInfo(gameObj->GetSpellId());
        // Need activate spell use for owner
        if (createBySpell && createBySpell->IsCooldownStartedOnEvent())
            // note: item based cooldowns and cooldown spell mods with charges ignored (unknown existing cases)
            GetSpellHistory()->SendCooldownEvent(createBySpell);
    }
    gameObj->SetOwnerGUID(ObjectGuid::Empty);
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
            (*i)->SetOwnerGUID(ObjectGuid::Empty);
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
    for(auto i = m_gameObj.begin(); i != m_gameObj.end();)
    {
        (*i)->SetOwnerGUID(ObjectGuid::Empty);
        (*i)->SetRespawnTime(0);
        (*i)->Delete();
        i = m_gameObj.erase(i);
    }
}

void Unit::SendSpellNonMeleeDamageLog(SpellNonMeleeDamage *log)
{
    WorldPacket data(SMSG_SPELLNONMELEEDAMAGELOG, (16+4+4+1+4+4+1+1+4+4+1)); // we guess size (LK ok)
    data << log->target->GetPackGUID();
    data << log->attacker->GetPackGUID();
    data << uint32(log->SpellID);
    data << uint32(log->damage);                             //damage amount
#ifdef LICH_KING
    int32 overkill = log->damage - log->target->GetHealth();
    data << uint32(overkill > 0 ? overkill : 0);            // overkill
#endif
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
    SpellNonMeleeDamage log(this, target, SpellID, damageSchoolMask);
    log.damage = Damage - AbsorbedDamage - Resist - Blocked;
    log.absorb = AbsorbedDamage;
    log.resist = Resist;
    log.physicalLog = PhysicalDamage;
    log.blocked = Blocked;
    log.HitInfo = SPELL_HIT_TYPE_UNK1 | SPELL_HIT_TYPE_UNK3 | SPELL_HIT_TYPE_UNK6;
    if (CriticalHit)
        log.HitInfo |= SPELL_HIT_TYPE_CRIT;
    SendSpellNonMeleeDamageLog(&log);
}

/*static*/ void Unit::ProcSkillsAndAuras(Unit* actor, Unit* actionTarget, uint32 typeMaskActor, uint32 typeMaskActionTarget, uint32 spellTypeMask, uint32 spellPhaseMask, uint32 hitMask, Spell* spell, DamageInfo* damageInfo, HealInfo* healInfo)
{
    WeaponAttackType attType = damageInfo ? damageInfo->GetAttackType() : BASE_ATTACK;
    if (typeMaskActor && actor)
        actor->ProcSkillsAndReactives(false, actionTarget, typeMaskActor, hitMask, attType);

    if (typeMaskActionTarget && actionTarget)
        actionTarget->ProcSkillsAndReactives(true, actor, typeMaskActionTarget, hitMask, attType);

    if (actor)
        actor->TriggerAurasProcOnEvent(actionTarget, typeMaskActor, typeMaskActionTarget, spellTypeMask, spellPhaseMask, hitMask, spell, damageInfo, healInfo);
}

void Unit::SendAttackStateUpdate(CalcDamageInfo *damageInfo)
{
    uint32 count = 1;
    for (uint8 i = 1; i < MAX_ITEM_PROTO_DAMAGES; i++)
        if (damageInfo->Damages[i].Damage || damageInfo->Damages[i].Absorb || damageInfo->Damages[i].Resist)
            ++count;

    WorldPacket data(SMSG_ATTACKERSTATEUPDATE, (16+84));    // we guess size
    data << uint32(damageInfo->HitInfo);
    data << GetPackGUID();
    data << damageInfo->Target->GetPackGUID();
    uint32 totalDmg = 0;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
        totalDmg += damageInfo->Damages[i].Damage;
    data << uint32(totalDmg);
#ifdef LICH_KING
    int32 overkill = damageInfo->Damages[0].Damage + damageInfo->Damages[1].Damage - damageInfo->Target->GetHealth();
    data << uint32(overkill < 0 ? 0 : overkill);            // Overkill
#endif
    data << uint8(count);                                   // Sub damage count
#ifdef LICH_KING
    for (uint32 i = 0; i < count; ++i)
    {
        data << uint32(damageInfo->Damages[i].DamageSchoolMask);       // School of sub damage
        data << float(damageInfo->Damages[i].Damage);                  // sub damage
        data << uint32(damageInfo->Damages[i].Damage);                 // Sub Damage
    }

    if (damageInfo->HitInfo & (HITINFO_FULL_ABSORB | HITINFO_PARTIAL_ABSORB))
    {
        for (uint32 i = 0; i < count; ++i)
            data << uint32(damageInfo->Damages[i].Absorb);             // Absorb
    }

    if (damageInfo->HitInfo & (HITINFO_FULL_RESIST | HITINFO_PARTIAL_RESIST))
    {
        for (uint32 i = 0; i < count; ++i)
            data << uint32(damageInfo->Damages[i].Resist);             // Resist
    }
    data << uint8(damageInfo->TargetState);
    data << uint32(0);  // Unknown attackerstate
    data << uint32(0);  // Melee spellid

    if (damageInfo->HitInfo & HITINFO_BLOCK)
        data << uint32(damageInfo->Blocked);

    if (damageInfo->HitInfo & HITINFO_RAGE_GAIN)
        data << uint32(0);

    //! Probably used for debugging purposes, as it is not known to appear on retail servers
    if (damageInfo->HitInfo & HITINFO_UNK1)
    {
        data << uint32(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);
        data << float(0);       // Found in a loop with 1 iteration
        data << float(0);       // ditto ^
        data << uint32(0);
    }
#else
    //structure seems correct... but client does not show damage type for next damage types in damage log?
    for (uint32 i = 0; i < count; ++i)
    {
        data << uint32(damageInfo->Damages[i].DamageSchoolMask);       // School of sub damage
        data << float(damageInfo->Damages[i].Damage);                  // sub damage
        data << uint32(damageInfo->Damages[i].Damage);                 // Sub Damage
        data << uint32(damageInfo->Damages[i].Absorb);       // Absorb
        data << uint32(damageInfo->Damages[i].Resist);       // Resist
    }
    data << uint32(damageInfo->TargetState);
    data << uint32(0);  // Unknown attackerstate
    data << uint32(0);  // Melee spellid
    data << uint32(damageInfo->Blocked);
#endif
    SendMessageToSet( &data, true );
}

void Unit::SendAttackStateUpdate(uint32 HitInfo, Unit *target, uint8 SwingType, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, VictimState TargetState, uint32 BlockedAmount)
{
    CalcDamageInfo dmgInfo;
    dmgInfo.HitInfo = HitInfo;
    dmgInfo.Attacker = this;
    dmgInfo.Target = target;

    dmgInfo.Damages[0].Damage = Damage - AbsorbDamage - Resist - BlockedAmount;
    dmgInfo.Damages[0].DamageSchoolMask = damageSchoolMask;
    dmgInfo.Damages[0].Absorb = AbsorbDamage;
    dmgInfo.Damages[0].Resist = Resist;

    for (uint8 i = 1; i < MAX_ITEM_PROTO_DAMAGES; i++)
    {
        dmgInfo.Damages[i].Damage = 0;
        dmgInfo.Damages[i].DamageSchoolMask = 0;
        dmgInfo.Damages[i].Absorb = 0;
        dmgInfo.Damages[i].Resist = 0;
    }

    dmgInfo.TargetState = TargetState;
    dmgInfo.Blocked = BlockedAmount;
    SendAttackStateUpdate(&dmgInfo);
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

    // Update max power
    UpdateMaxPower(new_powertype);

    // Update current power
    switch (new_powertype)
    {
        case POWER_MANA: // Keep the same (druid form switching...)
        case POWER_ENERGY:
            break;
        case POWER_RAGE: // Reset to zero
            SetPower(POWER_RAGE, 0);
            break;
        case POWER_FOCUS: // Make it full
            SetFullPower(new_powertype);
            break;
        default:
            break;
    }
}

void Unit::UpdateDisplayPower()
{
    Powers displayPower = POWER_MANA;
    switch (GetShapeshiftForm())
    {
    case FORM_GHOUL:
    case FORM_CAT:
        displayPower = POWER_ENERGY;
        break;
    case FORM_BEAR:
    case FORM_DIREBEAR:
        displayPower = POWER_RAGE;
        break;
    case FORM_TRAVEL:
    case FORM_GHOSTWOLF:
        displayPower = POWER_MANA;
        break;
    default:
    {
        if (GetTypeId() == TYPEID_PLAYER)
        {
            ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(GetClass());
            if (cEntry && cEntry->PowerType < MAX_POWERS)
                displayPower = Powers(cEntry->PowerType);
        }
        else if (GetTypeId() == TYPEID_UNIT)
        {
#ifdef LICH_KING
            if (Vehicle* vehicle = GetVehicleKit())
            {
                if (PowerDisplayEntry const* powerDisplay = sPowerDisplayStore.LookupEntry(vehicle->GetVehicleInfo()->m_powerDisplayId))
                    displayPower = Powers(powerDisplay->PowerType);
                else if (getClass() == CLASS_ROGUE)
                    displayPower = POWER_ENERGY;
            }
            else 
#endif
                if (Pet* pet = ToPet())
            {
                if (pet->getPetType() == HUNTER_PET) // Hunter pets have focus
                    displayPower = POWER_FOCUS;
                else if (pet->IsPetGhoul() || pet->IsRisenAlly()) // DK pets have energy
                    displayPower = POWER_ENERGY;
            }
        }
        break;
    }
    }

    SetPowerType(displayPower);
}

void  Unit::_addAttacker(Unit *pAttacker)
{
    auto itr = m_attackers.find(pAttacker);
    if (itr == m_attackers.end())
        m_attackers.insert(pAttacker);
}
void  Unit::_removeAttacker(Unit *pAttacker)    
{
    auto itr = m_attackers.find(pAttacker);
    if (itr != m_attackers.end())
        m_attackers.erase(itr);
}

Unit* Unit::GetAttackerForHelper() const
{
    if (!IsEngaged())
        return nullptr;

    if (Unit* victim = GetVictim())
        if ((!IsPet() && !GetPlayerMovingMe()) || IsInCombatWith(victim))
            return victim;

    CombatManager const& mgr = GetCombatManager();
    // pick arbitrary targets; our pvp combat > owner's pvp combat > our pve combat > owner's pve combat
    Unit* owner = GetCharmerOrOwner();
    if (mgr.HasPvPCombat())
        return mgr.GetPvPCombatRefs().begin()->second->GetOther(this);
    if (owner && (owner->GetCombatManager().HasPvPCombat()))
        return owner->GetCombatManager().GetPvPCombatRefs().begin()->second->GetOther(owner);
    if (mgr.HasPvECombat())
        return mgr.GetPvECombatRefs().begin()->second->GetOther(this);
    if (owner && (owner->GetCombatManager().HasPvECombat()))
        return owner->GetCombatManager().GetPvECombatRefs().begin()->second->GetOther(owner);
    return nullptr;
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
        if (c && c->IsEvadingAttacks())
            return false;
    }

    // nobody can attack GM in GM-mode
    if (victim->GetTypeId() == TYPEID_PLAYER) {
        if ((victim->ToPlayer())->IsGameMaster() || (victim->ToPlayer())->isSpectator())
            return false;
    } else {
        if (victim->ToCreature()->IsEvadingAttacks())
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
    if(!HasUnitState(UNIT_STATE_CONTROLLED)) //sun, no setting target for stun, confused, ...
        SetTarget(victim->GetGUID());        

    m_attacking = victim;
    m_attacking->_addAttacker(this);

    if (GetTypeId() == TYPEID_UNIT && !IsControlledByPlayer())
    {
        EngageWithTarget(victim); // ensure that anything we're attacking has threat

        SendAIReaction(AI_REACTION_HOSTILE);
        ToCreature()->CallAssistance();
    }

    // delay offhand weapon attack to next attack time
    if (HaveOffhandWeapon() && GetTypeId() != TYPEID_PLAYER)
        ResetAttackTimer(OFF_ATTACK);

    if (meleeAttack) 
    {
        AddUnitState(UNIT_STATE_MELEE_ATTACKING);
        SendMeleeAttackStart(victim);
    }

    // Let the pet know we've started attacking someting. Handles melee attacks only
    // Spells such as auto-shot and others handled in WorldSession::HandleCastSpellOpcode
    if (GetTypeId() == TYPEID_PLAYER)
    {
        for (Unit* controlled : m_Controlled)
            if (Creature* cControlled = controlled->ToCreature())
                if (cControlled->IsAIEnabled)
                    cControlled->AI()->OwnerAttacked(victim);
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

    m_attacking->_removeAttacker(this);
    m_attacking = nullptr;

    //Clear our target
    SetTarget(ObjectGuid::Empty);

    ClearUnitState(UNIT_STATE_MELEE_ATTACKING);

    InterruptSpell(CURRENT_MELEE_SPELL);

    if (Creature* creature = ToCreature())
    {
        // reset call assistance
        creature->SetNoCallAssistance(false);
        /*TC
        if (creature->HasSearchedAssistance())
        {
            creature->SetNoSearchAssistance(false);
            UpdateSpeed(MOVE_RUN);
        }*/
    }

    SendMeleeAttackStop(victim);

    return true;
}

void Unit::CombatStop(bool includingCast, bool mutualPvP)
{
    if(includingCast && IsNonMeleeSpellCast(false))
        InterruptNonMeleeSpells(false);

    if (GetTypeId() == TYPEID_PLAYER)
        (this->ToPlayer())->SendAttackSwingCancelAttack();     // melee and ranged forced attack cancel

    AttackStop();
    RemoveAllAttackers();

    if (mutualPvP)
        ClearInCombat();
    else
    { // vanish and brethren are weird
        m_combatManager.EndAllPvECombat();
        m_combatManager.SuppressPvPCombat();
    }
}

void Unit::CombatStopWithPets(bool includingCast)
{
    CombatStop(includingCast);

    for (Unit* minion : m_Controlled)
        minion->CombatStop(includingCast);
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

    for (auto& i : m_SummonSlot)
    {
        if(i)
        {
            Creature *totem = ObjectAccessor::GetCreature(*this, i);
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
        auto iter = m_attackers.begin();
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
                for (const auto & itr : sp_list)
                {
                    if(itr.second->state == PLAYERSPELL_REMOVED) 
                        continue;
                    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(itr.first);
                    if (!spellInfo || !spellInfo->IsPassive()) 
                        continue;
                    if (spellInfo->CasterAuraState == flag)
                        CastSpell(this, itr.first, true);
                }
            }
        }
    }
    else
    {
        if (HasFlag(UNIT_FIELD_AURASTATE,1<<(flag-1)))
        {
            RemoveFlag(UNIT_FIELD_AURASTATE, 1<<(flag-1));
            Unit::AuraApplicationMap& tAuras = GetAppliedAuras();
            for (auto itr = tAuras.begin(); itr != tAuras.end();)
            {
                SpellInfo const* spellProto = itr->second->GetBase()->GetSpellInfo();
                if (itr->second->GetBase()->GetCasterGUID() == GetGUID() && spellProto->CasterAuraState == flag)
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
    for (const auto & m_auraStateAura : m_auraStateAuras)
        if ((1 << (m_auraStateAura.first - 1)) & PER_CASTER_AURA_STATE_MASK)
            if (m_auraStateAura.second->GetBase()->GetCasterGUID() == target->GetGUID())
                auraStates |= (1 << (m_auraStateAura.first - 1));

    return auraStates;
}

bool Unit::HasAuraState(AuraStateType flag, SpellInfo const* spellProto, Unit const* Caster) const
{
    if (Caster)
    {
#ifdef LICH_KING
        if (spellProto)
        {
            auto stateAuras = Caster->GetAuraEffectsByType(SPELL_AURA_ABILITY_IGNORE_AURASTATE);
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
            for (auto itr = range.first; itr != range.second; ++itr)
                if (itr->second->GetBase()->GetCasterGUID() == Caster->GetGUID())
                    return true;
            return false;
        }
    }

    return HasFlag(UNIT_FIELD_AURASTATE, 1 << (flag - 1));
}

Unit *Unit::GetCharmer() const
{
    if(ObjectGuid charmerid = GetCharmerGUID())
        return ObjectAccessor::GetUnit(*this, charmerid);
    return nullptr;
}

bool Unit::IsCharmedOwnedByPlayerOrPlayer() const 
{ 
    return GetCharmerOrOwnerOrOwnGUID().IsPlayer();
}

Pet* Unit::GetPet() const
{
    if(ObjectGuid pet_guid = GetMinionGUID())
    {
        if(!pet_guid.IsPet())
            return nullptr;

        Pet* pet = ObjectAccessor::GetPet(*this, pet_guid);

        if (!pet)
            return nullptr;

        if (IsInWorld() && pet)
            return pet;

        //there may be a guardian in slot
        //TC_LOG_ERROR("entities.pet","Unit::GetPet: Pet %u not exist.",pet_guid.GetCounter());
    }

    return nullptr;
}

Minion* Unit::GetFirstMinion() const
{
    if (ObjectGuid pet_guid = GetMinionGUID())
    {
        if (Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*this, pet_guid))
            if (pet->HasUnitTypeMask(UNIT_MASK_MINION))
                return (Minion*)pet;

        TC_LOG_ERROR("entities.unit", "Unit::GetFirstMinion: Minion %s not exist.", pet_guid.ToString().c_str());
        const_cast<Unit*>(this)->SetMinionGUID(ObjectGuid::Empty);
    }

    return NULL;
}

Guardian* Unit::GetGuardianPet() const
{
    if (ObjectGuid pet_guid = GetPetGUID())
    {
        if (Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*this, pet_guid))
            if (pet->HasUnitTypeMask(UNIT_MASK_GUARDIAN))
                return (Guardian*)pet;

        TC_LOG_FATAL("entities.unit", "Unit::GetGuardianPet: Guardian %u not exist.", pet_guid.GetCounter());
        const_cast<Unit*>(this)->SetPetGUID(ObjectGuid::Empty);
    }

    return nullptr;
}

Unit* Unit::GetCharm() const
{
    if(ObjectGuid charm_guid = GetCharmGUID())
    {
        if(Unit* pet = ObjectAccessor::GetUnit(*this, charm_guid))
            return pet;

        TC_LOG_ERROR("entities.pet","Unit::GetCharm: Charmed creature %u not exist.",charm_guid.GetCounter());
        const_cast<Unit*>(this)->SetGuidValue(UNIT_FIELD_CHARM, ObjectGuid::Empty);
    }

    return nullptr;
}

void Unit::SetMinion(Minion *minion, bool apply)
{
    TC_LOG_DEBUG("entities.unit", "SetMinion %u for %u, apply %u", minion->GetEntry(), GetEntry(), apply);

    if (apply)
    {
        if (minion->GetOwnerGUID())
        {
            TC_LOG_FATAL("entities.unit", "SetMinion: Minion %u is not the minion of owner %u", minion->GetEntry(), minion->GetOwnerGUID().GetCounter());
            return;
        }

        minion->SetOwnerGUID(GetGUID());

        m_Controlled.insert(minion);

        if (GetTypeId() == TYPEID_PLAYER)
        {
            minion->m_ControlledByPlayer = true;
            minion->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
        }

        // Can only have one pet. If a new one is summoned, dismiss the old one.
        if (minion->IsGuardianPet())
        {
            if (Guardian* oldPet = GetGuardianPet())
            {
                if (oldPet != minion && (oldPet->IsPet() || minion->IsPet() || oldPet->GetEntry() != minion->GetEntry()))
                {
                    // remove existing minion pet
                    if (oldPet->IsPet())
                        ((Pet*)oldPet)->Remove(PET_SAVE_AS_CURRENT);
                    else
                        oldPet->UnSummon();
                    SetPetGUID(minion->GetGUID());
                    SetMinionGUID(ObjectGuid::Empty);
                }
            }
            else
            {
                SetPetGUID(minion->GetGUID());
                SetMinionGUID(ObjectGuid::Empty);
            }
        }

        if (minion->HasUnitTypeMask(UNIT_MASK_CONTROLABLE_GUARDIAN))
        {
            if (AddGuidValue(UNIT_FIELD_SUMMON, minion->GetGUID()))
            {
            }
        }

        if (minion->m_Properties && minion->m_Properties->Type == SUMMON_TYPE_MINIPET)
            SetCritterGUID(minion->GetGUID());

        // PvP, FFAPvP
#ifdef LICH_KING
        minion->SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG));
#else
        minion->SetPvP(true);
#endif

        // FIXME: hack, speed must be set only at follow
        if (GetTypeId() == TYPEID_PLAYER && minion->IsPet())
            for (uint8 i = 0; i < MAX_MOVE_TYPE; ++i)
                minion->SetSpeedRate(UnitMoveType(i), m_speed_rate[i]);

        // Send infinity cooldown - client does that automatically but after relog cooldown needs to be set again
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(minion->GetUInt32Value(UNIT_CREATED_BY_SPELL));
        if (spellInfo && spellInfo->IsCooldownStartedOnEvent())
            GetSpellHistory()->StartCooldown(spellInfo, 0, nullptr, true);
    }
    else
    {
        if (minion->GetOwnerGUID() != GetGUID())
        {
            TC_LOG_FATAL("entities.unit", "SetMinion: Minion %u is not the minion of owner %u", minion->GetEntry(), GetEntry());
            return;
        }

        m_Controlled.erase(minion);

        if (minion->m_Properties && minion->m_Properties->Type == SUMMON_TYPE_MINIPET)
            if (GetCritterGUID() == minion->GetGUID())
                SetCritterGUID(ObjectGuid::Empty);

        if (minion->IsGuardianPet())
        {
            if (GetPetGUID() == minion->GetGUID())
                SetPetGUID(ObjectGuid::Empty);
        }
        else if (minion->IsTotem())
        {
            // All summoned by totem minions must disappear when it is removed.
            if (SpellInfo const* spInfo = sSpellMgr->GetSpellInfo(minion->ToTotem()->GetSpell()))
                for (int i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    if (spInfo->Effects[i].Effect != SPELL_EFFECT_SUMMON)
                        continue;

                    RemoveAllMinionsByEntry(spInfo->Effects[i].MiscValue);
                }
        }

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(minion->GetUInt32Value(UNIT_CREATED_BY_SPELL));
        // Remove infinity cooldown
        if (spellInfo && (spellInfo->IsCooldownStartedOnEvent()))
            GetSpellHistory()->SendCooldownEvent(spellInfo);

        //if (minion->HasUnitTypeMask(UNIT_MASK_GUARDIAN))
        {
            if (RemoveGuidValue(UNIT_FIELD_SUMMON, minion->GetGUID()))
            {
                // Check if there is another minion
                for (ControlList::iterator itr = m_Controlled.begin(); itr != m_Controlled.end(); ++itr)
                {
                    // do not use this check, creature do not have charm guid
                    //if (GetCharmGUID() == (*itr)->GetGUID())
                    if (GetGUID() == (*itr)->GetCharmerGUID())
                        continue;

                    //ASSERT((*itr)->GetOwnerGUID() == GetGUID());
                    if ((*itr)->GetOwnerGUID() != GetGUID())
                    {
                        OutDebugInfo();
                        (*itr)->OutDebugInfo();
                        ABORT();
                    }
                    ASSERT((*itr)->GetTypeId() == TYPEID_UNIT);

                    if (!(*itr)->HasUnitTypeMask(UNIT_MASK_CONTROLABLE_GUARDIAN))
                        continue;

                    if (AddGuidValue(UNIT_FIELD_SUMMON, (*itr)->GetGUID()))
                    {
                        // show another pet bar if there is no charm bar
                        if (GetTypeId() == TYPEID_PLAYER && !GetCharmGUID())
                        {
                            if ((*itr)->IsPet())
                                ToPlayer()->PetSpellInitialize();
                            else
                                ToPlayer()->CharmSpellInitialize();
                        }
                    }
                    break;
                }
            }
        }
    }
    UpdatePetCombatState();
}

void Unit::GetAllMinionsByEntry(std::list<Creature*>& Minions, uint32 entry)
{
    for (Unit::ControlList::iterator itr = m_Controlled.begin(); itr != m_Controlled.end();)
    {
        Unit* unit = *itr;
        ++itr;
        if (unit->GetEntry() == entry && unit->GetTypeId() == TYPEID_UNIT
            && unit->IsSummon()) // minion, actually
            Minions.push_back(unit->ToCreature());
    }
}

void Unit::RemoveAllMinionsByEntry(uint32 entry)
{
    for (Unit::ControlList::iterator itr = m_Controlled.begin(); itr != m_Controlled.end();)
    {
        Unit* unit = *itr;
        ++itr;
        if (unit->GetEntry() == entry && unit->GetTypeId() == TYPEID_UNIT
            && unit->IsSummon()) // minion, actually
            unit->ToTempSummon()->UnSummon();
        // i think this is safe because i have never heard that a despawned minion will trigger a same minion
    }
}

void Unit::SetCharm(Unit* charm, bool apply)
{
    if (apply)
    {
        if (GetTypeId() == TYPEID_PLAYER)
        {
            if (!AddGuidValue(UNIT_FIELD_CHARM, charm->GetGUID()))
                TC_LOG_FATAL("entities.unit", "Player %s is trying to charm unit %u, but it already has a charmed unit %s", GetName().c_str(), charm->GetEntry(), ObjectGuid(GetCharmGUID()).ToString().c_str());

            charm->m_ControlledByPlayer = true;
            /// @todo maybe we can use this flag to check if controlled by player
            charm->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
        }
        else
            charm->m_ControlledByPlayer = false;

        // PvP, FFAPvP
#ifdef LICH_KING
        charm->SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG));
#else
        charm->SetPvP(true);
#endif

        if (!charm->AddGuidValue(UNIT_FIELD_CHARMEDBY, GetGUID()))
            TC_LOG_FATAL("entities.unit", "Unit %u is being charmed, but it already has a charmer %s", charm->GetEntry(), ObjectGuid(charm->GetCharmerGUID()).ToString().c_str());

        /* TC
        _isWalkingBeforeCharm = charm->IsWalking();
        if (_isWalkingBeforeCharm)
        {
            charm->SetWalk(false);
            charm->SendMovementFlagUpdate();
        }
        */

        m_Controlled.insert(charm);
    }
    else
    {
        charm->ClearUnitState(UNIT_STATE_CHARMED);

        if (GetTypeId() == TYPEID_PLAYER)
        {
            if (!RemoveGuidValue(UNIT_FIELD_CHARM, charm->GetGUID()))
                TC_LOG_FATAL("entities.unit", "Player %s is trying to uncharm unit %u, but it has another charmed unit %s", GetName().c_str(), charm->GetEntry(), ObjectGuid(GetCharmGUID()).ToString().c_str());
        }

        if (!charm->RemoveGuidValue(UNIT_FIELD_CHARMEDBY, GetGUID()))
            TC_LOG_FATAL("entities.unit", "Unit %u is being uncharmed, but it has another charmer %s", charm->GetEntry(), ObjectGuid(charm->GetCharmerGUID()).ToString().c_str());

        if (charm->GetTypeId() == TYPEID_PLAYER)
        {
            charm->m_ControlledByPlayer = true;
            charm->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
            charm->ToPlayer()->UpdatePvPState();
        }
        else if (Player* player = charm->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            charm->m_ControlledByPlayer = true;
            charm->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
#ifdef LICH_KING
            charm->SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, player->GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG));
#else
            charm->SetPvP(true);
#endif    
        }
        else
        {
            charm->m_ControlledByPlayer = false;
            charm->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
#ifdef LICH_KING
            charm->SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, 0);
#else
            charm->SetPvP(false);
#endif    
        }

        /* TC
        if (charm->IsWalking() != _isWalkingBeforeCharm)
        {
            charm->SetWalk(_isWalkingBeforeCharm);
            charm->SendMovementFlagUpdate(true); // send packet to self, to update movement state on player.
        }
        */

        if (charm->GetTypeId() == TYPEID_PLAYER
            || !charm->ToCreature()->HasUnitTypeMask(UNIT_MASK_MINION)
            || charm->GetOwnerGUID() != GetGUID())
        {
            m_Controlled.erase(charm);
        }
    }
    UpdatePetCombatState();
}

// only called in Player::SetViewpoint
// so move it to Player?
void Unit::AddPlayerToVision(Player* plr)
{
    if(m_sharedVision.empty())
    {
        //set active so that creatures around in grid are active as well
        SetKeepActive(true);
        SetWorldObject(true);
    }
    m_sharedVision.push_back(plr);
}

// only called in Player::SetViewpoint
void Unit::RemovePlayerFromVision(Player* plr)
{
    m_sharedVision.remove(plr);
    if(m_sharedVision.empty())
    {
        SetKeepActive(false);
        SetWorldObject(false);
    }
}

void Unit::RemoveBindSightAuras()
{
    RemoveAurasByType(SPELL_AURA_BIND_SIGHT);
}

void Unit::RemoveCharmAuras(Aura* except /*= nullptr*/)
{
    RemoveAurasByType(SPELL_AURA_MOD_CHARM, ObjectGuid::Empty, except);
    RemoveAurasByType(SPELL_AURA_MOD_POSSESS_PET, ObjectGuid::Empty, except);
    RemoveAurasByType(SPELL_AURA_MOD_POSSESS, ObjectGuid::Empty, except);
}

void Unit::UnsummonAllTotems()
{
    for (ObjectGuid& i : m_SummonSlot)
    {
        if(!i)
            continue;

        Creature *OldTotem = ObjectAccessor::GetCreature(*this, i);
        if (OldTotem && OldTotem->IsTotem())
            ((Totem*)OldTotem)->UnSummon();
    }
}

bool RedirectSpellEvent::Execute(uint64 e_time, uint32 p_time)
{
    if (Unit* auraOwner = ObjectAccessor::GetUnit(_self, _auraOwnerGUID))
    {
        // sunwell: already removed
        if (!auraOwner->HasAuraType(SPELL_AURA_SPELL_MAGNET))
            return true;

        Unit::AuraEffectList const& magnetAuras = auraOwner->GetAuraEffectsByType(SPELL_AURA_SPELL_MAGNET);
        for (auto magnetAura : magnetAuras)
            if (magnetAura == _auraEffect)
            {
                magnetAura->GetBase()->DropCharge(AURA_REMOVE_BY_DEFAULT);
                return true;
            }
    }

    return true;
}

bool Unit::IsMagnet() const
{
    // Grounding Totem
    if (GetUInt32Value(UNIT_CREATED_BY_SPELL) == 8177) /// @todo: find a more generic solution
        return true;

    return false;
}

Unit* Unit::GetMeleeHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo)
{
    AuraEffectList const& hitTriggerAuras = victim->GetAuraEffectsByType(SPELL_AURA_ADD_CASTER_HIT_TRIGGER);
    for (auto hitTriggerAura : hitTriggerAuras)
    {
        if (Unit* magnet = hitTriggerAura->GetCaster())
            if (IsValidAttackTarget(magnet, spellInfo) && magnet->IsWithinLOSInMap(this, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2)
                && (!spellInfo || (spellInfo->CheckExplicitTarget(this, magnet) == SPELL_CAST_OK
                    && spellInfo->CheckTarget(this, magnet, false) == SPELL_CAST_OK)))
                if (roll_chance_i(hitTriggerAura->GetAmount()))
                {
                    hitTriggerAura->GetBase()->DropCharge(AURA_REMOVE_BY_EXPIRE);
                    return magnet;
                }
    }
    //no redirect
    return victim;
}

Unit* Unit::GetFirstControlled() const
{
    // Sequence: charmed, pet, other guardians
    Unit* unit = GetCharm();
    if (!unit)
        if (ObjectGuid guid = GetMinionGUID())
        {
            unit = ObjectAccessor::GetUnit(*this, guid);
        }


    return unit;
}


void Unit::RemoveAllControlled()
{
    // possessed pet and vehicle
    if (GetTypeId() == TYPEID_PLAYER)
        ToPlayer()->StopCastingCharm();

    while (!m_Controlled.empty())
    {
        Unit* target = *m_Controlled.begin();
        m_Controlled.erase(m_Controlled.begin());
        if (target->GetCharmerGUID() == GetGUID())
            target->RemoveCharmAuras();
        else if (target->GetOwnerGUID() == GetGUID() && target->IsSummon())
            target->ToTempSummon()->UnSummon();
        else
            TC_LOG_ERROR("entities.unit", "Unit %u is trying to release unit %u which is neither charmed nor owned by it", GetEntry(), target->GetEntry());
    }
    if (GetPetGUID())
        TC_LOG_FATAL("entities.unit", "Unit %u is not able to release its pet %s", GetEntry(), ObjectGuid(GetPetGUID()).ToString().c_str());
    if (GetMinionGUID())
        TC_LOG_FATAL("entities.unit", "Unit %u is not able to release its minion %s", GetEntry(), ObjectGuid(GetMinionGUID()).ToString().c_str());
    if (GetCharmGUID())
        TC_LOG_FATAL("entities.unit", "Unit %u is not able to release its charm %s", GetEntry(), ObjectGuid(GetCharmGUID()).ToString().c_str());
    if (!IsPet()) // pets don't use the flag for this
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT); // m_controlled is now empty, so we know none of our minions are in combat
}

void Unit::DealHeal(HealInfo& healInfo)
{
    int32 gain = 0;
    Unit* healer = healInfo.GetHealer();
    Unit* victim = healInfo.GetTarget();
    uint32 addhealth = healInfo.GetHeal();

    if (healer)
    {
        if (victim->IsAIEnabled)
            victim->GetAI()->HealReceived(healer, addhealth);

        if (healer->IsAIEnabled)
            healer->GetAI()->HealDone(victim, addhealth);
    }

    if (addhealth)
        gain = victim->ModifyHealth(int32(addhealth));

    // Hook for OnHeal Event
    //sScriptMgr->OnHeal(healer, victim, (uint32&)gain);

    Unit* unit = healer;

    if (healer && healer->GetTypeId() == TYPEID_UNIT && healer->IsTotem())
        unit = healer->GetOwner();

    if (unit)
    {
        if (Player* player = unit->ToPlayer())
        {
            if (Battleground* bg = player->GetBattleground())
                bg->UpdatePlayerScore(player, SCORE_HEALING_DONE, gain);

#ifdef LICH_KING
            // use the actual gain, as the overheal shall not be counted, skip gain 0 (it ignored anyway in to criteria)
            if (gain)
                player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE, gain, 0, victim);

            player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEAL_CAST, addhealth);
#endif
        }
    }

#ifdef LICH_KING
    if (Player* player = victim->ToPlayer())
    {
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_TOTAL_HEALING_RECEIVED, gain);
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED, addhealth);
    }
#endif

    if (gain)
        healInfo.SetEffectiveHeal(gain > 0 ? static_cast<uint32>(gain) : 0UL);
}

int32 Unit::HealBySpell(HealInfo& healInfo, bool critical /*= false*/, SpellMissInfo missInfo /* = SPELL_MISS_NONE */)
{
#ifdef LICH_KING
    // calculate heal absorb and reduce healing
    Unit::CalcHealAbsorb(healInfo);
#endif

    Unit::DealHeal(healInfo);
    SendHealSpellLog(healInfo, critical);

#ifdef TESTS
    if (Player* p = healInfo.GetHealer()->GetCharmerOrOwnerPlayerOrPlayerItself())
        if (auto AI = p->GetPlayerbotAI())
            AI->CastedHealingSpell(healInfo.GetTarget(), healInfo.GetHeal(), healInfo.GetEffectiveHeal(), healInfo.GetSpellInfo()->Id, missInfo, critical);
#endif

    return healInfo.GetEffectiveHeal();
}

void Unit::SendHealSpellLog(HealInfo& healInfo, bool critical /*= false*/)
{
    // we guess size
    WorldPacket data(SMSG_SPELLHEALLOG, 8 + 8 + 4 + 4 + 4 + 4 + 1 + 1);
    data << healInfo.GetTarget()->GetPackGUID();
    data << healInfo.GetHealer()->GetPackGUID();
    data << uint32(healInfo.GetSpellInfo()->Id);
    data << uint32(healInfo.GetHeal());
#ifdef LICH_KING
    data << uint32(healInfo.GetHeal() - healInfo.GetEffectiveHeal());
    data << uint32(healInfo.GetAbsorb()); // Absorb amount
#endif
    data << uint8(critical ? 1 : 0);
    data << uint8(0); // unused
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

void Unit::EnergizeBySpell(Unit* victim, uint32 spellId, int32 damage, Powers powerType)
{
    if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId))
        EnergizeBySpell(victim, info, damage, powerType);
}

void Unit::EnergizeBySpell(Unit* victim, SpellInfo const* spellInfo, int32 damage, Powers powerType, bool sendLog)
{
    if (sendLog)
        SendEnergizeSpellLog(victim, spellInfo->Id, damage, powerType);
    // needs to be called after sending spell log
    victim->ModifyPower(powerType, damage);
    victim->GetThreatManager().ForwardThreatForAssistingMe(this, float(damage) / 2, spellInfo, true);
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
    bool DirectDamage = false;
    bool AreaEffect = false;

    for (const auto & Effect : spellProto->Effects)
    {
        switch (Effect.Effect)
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
            switch (Effect.ApplyAuraName)
            {
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PERIODIC_HEAL:
            case SPELL_AURA_PERIODIC_LEECH:
                if (spellProto->GetDuration())
                    overTime = spellProto->GetDuration();
                break;
            default:
                /* From TC. Why? LK only?
                // -5% per additional effect
                ++effects;
                */
                break;
            }
        default:
            break;
        }

        if (Effect.IsTargetingArea())
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
    for (const auto & Effect : spellProto->Effects)
    {
        if (Effect.Effect == SPELL_EFFECT_HEALTH_LEECH ||
            (Effect.Effect == SPELL_EFFECT_APPLY_AURA && Effect.ApplyAuraName == SPELL_AURA_PERIODIC_LEECH))
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

void Unit::SetCantProc(bool apply)
{
    if (apply)
        ++m_procDeep;
    else
    {
        ASSERT(m_procDeep);
        --m_procDeep;
    }
}

float Unit::CalculateDefaultCoefficient(SpellInfo const *spellInfo, DamageEffectType damagetype) const
{
    int32 CastingTime = 0;

    switch (spellInfo->SpellFamilyName)
    {
    case SPELLFAMILY_WARRIOR:
    case SPELLFAMILY_ROGUE:
    case SPELLFAMILY_HUNTER:
        return 0.0f;
    case SPELLFAMILY_WARLOCK:
        // Dark Pact - Only if player has pet
        if ((spellInfo->SpellFamilyFlags & 0x80000000LL) && spellInfo->SpellIconID == 154 && GetMinionGUID())
        {
            CastingTime = 3360;                         // 96% from +shadow damage
        }
        break;
    case SPELLFAMILY_PALADIN:
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
        break;
    default:
        break;
    }

    // Damage over Time spells bonus calculation
    float DotFactor = 1.0f;
    if (damagetype == DOT)
    {
        int32 DotDuration = spellInfo->GetDuration();
        if (!spellInfo->IsChanneled() && DotDuration > 0)
            DotFactor = DotDuration / 15000.0f;

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

uint32 Unit::SpellDamageBonusTaken(Unit* caster, SpellInfo const *spellProto, uint32 pdamage, DamageEffectType damagetype)
{
    if (!spellProto || damagetype == DIRECT_DAMAGE)
        return pdamage;

    float TakenTotalMod = 1.0f;

#ifdef LICH_KING
    // Mod damage from spell mechanic
    if (uint32 mechanicMask = spellProto->GetAllEffectsMechanicMask())
    {
        TakenTotalMod *= GetTotalAuraMultiplier(SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT, [mechanicMask](AuraEffect const* aurEff) -> bool
        {
            if (mechanicMask & uint32(1 << (aurEff->GetMiscValue())))
                return true;
            return false;
        });
    }
#endif
    
    bool hasmangle = false;
    // .. taken pct: dummy auras
    AuraEffectList const& mDummyAuras = GetAuraEffectsByType(SPELL_AURA_DUMMY);

    for (auto mDummyAura : mDummyAuras)
    {
        switch (mDummyAura->GetSpellInfo()->SpellIconID)
        {
            //Cheat Death
        case 2109:
            if ((mDummyAura->GetMiscValue() & spellProto->GetSchoolMask()))
            {
                if (GetTypeId() != TYPEID_PLAYER)
                    continue;
                float mod = -1.0f * (ToPlayer())->GetRatingBonusValue(CR_CRIT_TAKEN_SPELL) * 2 * 4;
                AddPct(TakenTotalMod, std::max(mod, float(mDummyAura->GetAmount())));
                TakenTotalMod *= (mod + 100.0f) / 100.0f;
            }
            break;
            //This is changed in TLK, using aura 255
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
                    TakenTotalMod *= (100.0f + mDummyAura->GetAmount()) / 100.0f;
                    break;
                }
            }
            break;

        }
    }

    // Spells with SPELL_ATTR4_FIXED_DAMAGE should only benefit from mechanic damage mod auras.
    if (!spellProto->HasAttribute(SPELL_ATTR4_FIXED_DAMAGE))
    {
        // from positive and negative SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN
        // multiplicative bonus, for example Dispersion + Shadowform (0.10*0.85=0.085)
        TakenTotalMod *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, spellProto->GetSchoolMask());

#ifdef LICH_KING
        // From caster spells
        if (caster)
        {
            TakenTotalMod *= GetTotalAuraMultiplier(SPELL_AURA_MOD_DAMAGE_FROM_CASTER, [caster, spellProto](AuraEffect const* aurEff) -> bool
            {
                if (aurEff->GetCasterGUID() == caster->GetGUID() && aurEff->IsAffectedOnSpell(spellProto))
                    return true;
                return false;
            });
        }
#endif
    }

  
#ifdef LICH_KING
    // Sanctified Wrath (bypass damage reduction)
    if (caster && TakenTotalMod < 1.0f)
    {
        float damageReduction = 1.0f - TakenTotalMod;
        Unit::AuraEffectList const& casterIgnoreResist = caster->GetAuraEffectsByType(SPELL_AURA_MOD_IGNORE_TARGET_RESIST);
        for (AuraEffect const* aurEff : casterIgnoreResist)
        {
            if (!(aurEff->GetMiscValue() & spellProto->GetSchoolMask()))
                continue;

            AddPct(damageReduction, -aurEff->GetAmount());
        }

        TakenTotalMod = 1.0f - damageReduction;
    }
#endif

    float tmpDamage = pdamage * TakenTotalMod;
    return uint32(std::max(tmpDamage, 0.0f));
}

uint32 Unit::SpellDamageBonusDone(Unit* victim, SpellInfo const *spellProto, uint32 pdamage, DamageEffectType damagetype, uint8 effIndex, Optional<float> const& donePctTotal, uint32 stack)
{
    //HACK TIME
    switch (spellProto->SpellFamilyName)
    {
    case SPELLFAMILY_MAGE:
        // Ignite - do not modify, it is (8*Rank)% damage of procing Spell
        if (spellProto->Id == 12654)
            return pdamage;
        break;
    }

    if(!spellProto || !victim || damagetype == DIRECT_DAMAGE )
        return pdamage;
        
    if (spellProto->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS))
        return pdamage;

    // Done total percent damage auras
    float ApCoeffMod = 1.0f;
    int32 DoneTotal = 0;
    float DoneTotalMod = donePctTotal ? *donePctTotal : SpellDamagePctDone(victim, spellProto, damagetype);

    uint32 creatureTypeMask = victim->GetCreatureTypeMask();
    // Add flat bonus from spell damage versus
    DoneTotal += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_FLAT_SPELL_DAMAGE_VERSUS, creatureTypeMask);

    // done scripted mod (take it from owner)
    Unit* owner = GetOwner() ? GetOwner() : this;
    DoneTotal += owner->GetTotalAuraModifier(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS, [spellProto](AuraEffect const* aurEff) -> bool
    {
        if (!aurEff->IsAffectedOnSpell(spellProto))
            return false;

        switch (aurEff->GetMiscValue())
        {
        case 4418: // Increased Shock Damage
        case 4554: // Increased Lightning Damage
        case 4555: // Improved Moonfire
        case 5142: // Increased Lightning Damage
        case 5147: // Improved Consecration / Libram of Resurgence
        case 5148: // Idol of the Shooting Star
        case 6008: // Increased Lightning Damage
        case 8627: // Totem of Hex
            return true;
        default:
            break;
        }

        return false;
    });

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
    int32 DoneAdvertisedBenefit = SpellBaseDamageBonusDone(spellProto->GetSchoolMask());
    // modify spell power by victim's SPELL_AURA_MOD_DAMAGE_TAKEN auras (eg Amplify/Dampen Magic)
    //TC DoneAdvertisedBenefit += victim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, spellProto->GetSchoolMask());
    DoneAdvertisedBenefit += victim->SpellBaseDamageBonusTaken(spellProto);

    // Pets just add their bonus damage to their spell damage
    // note that their spell damage is just gain of their own auras
    if (HasUnitTypeMask(UNIT_MASK_GUARDIAN))
        DoneAdvertisedBenefit += static_cast<Guardian const*>(this)->GetBonusDamage();
    else if (IsTotem() && ToTotem()->GetTotemType() != TOTEM_STATUE)
        if(owner != this)
            return owner->SpellDamageBonusDone(victim, spellProto, pdamage, damagetype, effIndex, donePctTotal, stack);

#ifdef LICH_KING
    float coeff = spellProto->Effects[effIndex].BonusMultiplier;
#else
    //sun: -1.0f = use default coef. This is also the value used in db spell_bonus_data
    float coeff = -1.0f;
#endif
    if (SpellBonusEntry const* bonus = sSpellMgr->GetSpellBonusData(spellProto->Id))
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
    else 
    {
        // No bonus damage for SPELL_DAMAGE_CLASS_NONE class spells by default
        if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE)
            return uint32(std::max(pdamage * DoneTotalMod, 0.0f));
    }

    // Default calculation
    if (DoneAdvertisedBenefit)
    {
        if (coeff < 0.f)
            coeff = CalculateDefaultCoefficient(spellProto, damagetype);  // As wowwiki says: C = (Cast Time / 3.5)

        float factorMod = CalculateSpellpowerCoefficientLevelPenalty(spellProto) * stack;

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

float Unit::SpellDamagePctDone(Unit* victim, SpellInfo const *spellProto, DamageEffectType damagetype)
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
                return owner->SpellDamagePctDone(victim, spellProto, damagetype);
        }
    }

    // Done total percent damage auras
    float DoneTotalMod = 1.0f;

    // ..done
    AuraEffectList const& mModDamagePercentDone = GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE);

    for (auto i : mModDamagePercentDone)
    {
        //Some auras affect only weapons, like wand spec (6057) or 2H spec (12714)
        if (i->GetSpellInfo()->Attributes & SPELL_ATTR0_AFFECT_WEAPON && i->GetSpellInfo()->EquippedItemClass != -1)
            continue;

        if (i->GetMiscValue() & spellProto->GetSchoolMask())
        {
            if (i->GetSpellInfo()->EquippedItemClass == -1)
                AddPct(DoneTotalMod, i->GetAmount());
            else if (!i->GetSpellInfo()->HasAttribute(SPELL_ATTR5_SPECIAL_ITEM_CLASS_CHECK) && (i->GetSpellInfo()->EquippedItemSubClassMask == 0))
                AddPct(DoneTotalMod, i->GetAmount());
            else if (ToPlayer() && ToPlayer()->HasItemFitToSpellRequirements(i->GetSpellInfo()))
                AddPct(DoneTotalMod, i->GetAmount());
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
    AuraEffectList const& mDamageDoneVersus = GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS);

    for (auto mDamageDoneVersu : mDamageDoneVersus)
        if (creatureTypeMask & uint32(mDamageDoneVersu->GetMiscValue()))
            DoneTotalMod *= (mDamageDoneVersu->GetAmount() + 100.0f) / 100.0f;


    // .. taken pct: scripted (increases damage of * against targets *)
    AuraEffectList const& mOverrideClassScript = GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);

    for (auto i : mOverrideClassScript)
    {
        switch (i->GetMiscValue())
        {
            //Molten Fury (id 31679)
        case 4920:
        case 4919:
            if (HasAuraState(AURA_STATE_HEALTHLESS_20_PERCENT))
                AddPct(DoneTotalMod, i->GetAmount());
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
    AuraEffectList const& mDamageDone = GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_DONE);

    for(auto i : mDamageDone)
        if((i->GetMiscValue() & schoolMask) != 0 &&
        i->GetSpellInfo()->EquippedItemClass == -1 &&
                                                            // -1 == any item class (not wand then)
        i->GetSpellInfo()->EquippedItemInventoryTypeMask == 0 )
                                                            // 0 == any inventory type (not wand then)
            DoneAdvertisedBenefit += i->GetAmount();

    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Damage bonus from stats
        AuraEffectList const& mDamageDoneOfStatPercent = GetAuraEffectsByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT);

        for(auto i : mDamageDoneOfStatPercent)
        {
            if(i->GetMiscValue() & schoolMask)
            {
                SpellInfo const* iSpellProto = i->GetSpellInfo();
                uint8 eff = i->GetEffIndex();

                // stat used dependent from next effect aura SPELL_AURA_MOD_SPELL_HEALING presence and misc value (stat index)
                Stats usedStat = STAT_INTELLECT;
                if(eff < 2 && iSpellProto->Effects[eff+1].ApplyAuraName==SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT)
                    usedStat = Stats(iSpellProto->Effects[eff+1].MiscValue);

                DoneAdvertisedBenefit += int32(GetStat(usedStat) * i->GetAmount() / 100.0f);
            }
        }
        // ... and attack power
        AuraEffectList const& mDamageDonebyAP = GetAuraEffectsByType(SPELL_AURA_MOD_SPELL_DAMAGE_OF_ATTACK_POWER);

        for(auto i : mDamageDonebyAP)
            if (i->GetMiscValue() & schoolMask)
                DoneAdvertisedBenefit += int32(GetTotalAttackPowerValue(BASE_ATTACK, pVictim) * i->GetAmount() / 100.0f);

    }
    return DoneAdvertisedBenefit;
}

int32 Unit::SpellBaseDamageBonusTaken(SpellInfo const* spellInfo, bool isDoT)
{
    int32 TakenAdvertisedBenefit = 0;

    AuraEffectList const& mDamageTaken = GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_TAKEN);

    for(auto i : mDamageTaken)
        if ((i->GetMiscValue() & spellInfo->GetSchoolMask()) != 0)
        {
            /* SunWell core has this additional check. May be useful one day, can't investigate it right now. If not, replace this whole function with " return GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, schoolMask);"
            // sunwell: if we have DoT damage type and aura has charges, check if it affects DoTs
            // sunwell: required for hemorrhage & rupture / garrote
            if (isDoT && (*i)->IsUsingCharges() && !((*i)->GetSpellInfo()->ProcFlags & PROC_FLAG_TAKEN_PERIODIC))
                continue;
            */

            TakenAdvertisedBenefit += i->GetAmount();
        }

    return TakenAdvertisedBenefit;
}

float Unit::SpellCritChanceDone(SpellInfo const *spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType) const
{        
    // Mobs can't crit except for player controlled pets/guardians/totems (confirmed for those three)
    if (GetGUID().IsCreature())
    {
        if(Unit* owner = GetOwner())
            return owner->SpellCritChanceDone(spellProto, schoolMask, attackType);

        return 0.0f;
    }

    if (!spellProto->HasAttribute(SPELL_ATTR0_CU_CAN_CRIT))
        return 0.0f;
    
    if (spellProto->HasEffectByEffectMask(SPELL_EFFECT_HEALTH_LEECH))
        return 0.0f;

    float crit_chance = 0.0f;
    switch(spellProto->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_NONE:
            // mana potions, Demonic Rune, Alchemist's Stone, ...
            crit_chance = m_baseSpellCritChance;
            break;
        case SPELL_DAMAGE_CLASS_MAGIC:
        {
            /*  
            Implied spells: Potions, health stones, thunderclap, ... should crit
            SELECT * FROM spell_template WHERE dmgClass = 1 AND schoolMask = 1 AND (effect1 IN (2,10) OR effect2 IN (2,10) OR effect3 IN (2,10))
            */
            if (schoolMask & SPELL_SCHOOL_MASK_NORMAL)
                crit_chance = m_baseSpellCritChance;
            // For other schools
            else if (GetTypeId() == TYPEID_PLAYER)
                crit_chance = GetFloatValue( PLAYER_SPELL_CRIT_PERCENTAGE1 + GetFirstSchoolInMask(schoolMask));
            else
            {
                crit_chance = m_baseSpellCritChance;
                crit_chance += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL, schoolMask);
            }
          
            break;
        }
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
        {
            crit_chance = GetUnitCriticalChanceDone(attackType);
            crit_chance+= GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL, schoolMask);
            break;
        }
        default:
            return 0.0f;
    }
    // percent done
    // only players use intelligence for critical chance computations
    if(Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_CRITICAL_CHANCE, crit_chance);

    return std::max(crit_chance, 0.0f);
}

float Unit::SpellCritChanceTaken(Unit const* caster, SpellInfo const* spellInfo, SpellSchoolMask schoolMask, float doneChance, WeaponAttackType attackType /*= BASE_ATTACK*/) const
{
    // not critting spell
    if (!spellInfo->HasAttribute(SPELL_ATTR0_CU_CAN_CRIT))
        return 0.0f;

    float crit_chance = doneChance;
    switch (spellInfo->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_NONE:
        /* sun: At least some spells in here should crit, such as "Frenzied Regeneration",
        Other spells in this category: earth shield, Seal of Light, "Alchemist's Stone"
        I'm using the base crit chance for now. Note for earth shield:
        "Yesterday i checked my recount. For our warrior tank my ES had ZERO - 0 crits... For our pally it had around 6% crit."
        */
            return caster->m_baseSpellCritChance;
            //TC return 0.0f;
        case SPELL_DAMAGE_CLASS_MAGIC:
        {
            // taken
            if (!spellInfo->IsPositive())
            {
                // Modify critical chance by victim SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE
                crit_chance += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_ATTACKER_SPELL_CRIT_CHANCE, schoolMask);
                // Modify critical chance by victim SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE
                crit_chance += GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE);
                // Modify by player victim resilience
                if (GetTypeId() == TYPEID_PLAYER)
                    crit_chance -= (ToPlayer())->GetRatingBonusValue(CR_CRIT_TAKEN_SPELL);
                // scripted (increase crit chance ... against ... target by x%
                if (IsFrozen()) // Shatter
                {
                    AuraEffectList const& mOverrideClassScript = GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);

                    for (auto i : mOverrideClassScript)
                    {
                        switch (i->GetMiscValue())
                        {
                        case 849: crit_chance += 10.0f; break; //Shatter Rank 1
                        case 910: crit_chance += 20.0f; break; //Shatter Rank 2
                        case 911: crit_chance += 30.0f; break; //Shatter Rank 3
                        case 912: crit_chance += 40.0f; break; //Shatter Rank 4
                        case 913: crit_chance += 50.0f; break; //Shatter Rank 5
                        }
                    }
                }
                // arcane potency
                if (HasAura(12536) || HasAura(12043)) { // clearcasting or presence of mind
                    if (HasSpell(31571)) crit_chance += 10.0f;
                    if (HasSpell(31572)) crit_chance += 20.0f;
                    if (HasSpell(31573)) crit_chance += 30.0f;
                }

                // Spell crit suppression
                if (GetTypeId() == TYPEID_UNIT)
                {
                    int32 const levelDiff = static_cast<int32>(GetLevelForTarget(caster)) - caster->GetLevel();
                    //sun: added condition, only supress if target level is higher. Not 100% sure about this but couldn't find anything saying it should increase crit if target level is lower
                    if(levelDiff > 0)
                        crit_chance -= levelDiff * 0.7f;
                }
            }
        } break;
        case SPELL_DAMAGE_CLASS_MELEE:
        case SPELL_DAMAGE_CLASS_RANGED:
            if (caster)
                crit_chance = GetUnitCriticalChanceTaken(caster, attackType, crit_chance);

            break;
        default:
            return 0.0f;
    }

#ifdef LICH_KING
    // for this types the bonus was already added in GetUnitCriticalChanceAgainst, do not add twice
    if (caster && spellInfo->DmgClass != SPELL_DAMAGE_CLASS_MELEE && spellInfo->DmgClass != SPELL_DAMAGE_CLASS_RANGED)
    {
        crit_chance += GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_CHANCE_FOR_CASTER, [caster, spellInfo](AuraEffect const* aurEff) -> bool
        {
            if (aurEff->GetCasterGUID() == caster->GetGUID() && aurEff->IsAffectedOnSpell(spellInfo))
                return true;
            return false;
        });
    }
#endif

    return std::max(crit_chance, 0.0f);
}

uint32 Unit::SpellCriticalDamageBonus(Unit const* caster, SpellInfo const *spellProto, uint32 damage, Unit* victim)
{
    // Calculate critical bonus
    int32 crit_bonus = damage;
    float crit_mod = 0.0f;

    switch(spellProto->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:                      // for melee based spells is 100%
        case SPELL_DAMAGE_CLASS_RANGED:
            // TODO: write here full calculation for melee/ranged spells
            crit_bonus += damage;
            break;
        default:
            crit_bonus += damage / 2;                        // for spells is 50%
            break;
    }
    

    crit_mod += (caster->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_CRIT_DAMAGE_BONUS, spellProto->GetSchoolMask()) - 1.0f) * 100;
    if (victim)
        crit_mod += caster->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_CRIT_PERCENT_VERSUS, victim->GetCreatureTypeMask());

    if (crit_bonus != 0)
        AddPct(crit_bonus, crit_mod);


    crit_bonus -= damage;
    if (damage > uint32(crit_bonus))
    {
        // adds additional damage to crit_bonus (from talents)
        if(Player* modOwner = caster->GetSpellModOwner())
            modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_CRIT_DAMAGE_BONUS, crit_bonus);
    }
    crit_bonus += damage;

    return crit_bonus;
}

uint32 Unit::SpellCriticalHealingBonus(Unit const* caster, SpellInfo const* spellProto, uint32 damage, Unit* victim)
{
    // Calculate critical bonus
    int32 crit_bonus;
    switch (spellProto->DmgClass)
    {
    case SPELL_DAMAGE_CLASS_MELEE:                      // for melee based spells is 100%
    case SPELL_DAMAGE_CLASS_RANGED:
        /// @todo write here full calculation for melee/ranged spells
        crit_bonus = damage;
        break;
    default:
        crit_bonus = damage / 2;                        // for spells is 50%
        break;
    }

    if (caster)
    {
        if (victim)
        {
            uint32 creatureTypeMask = victim->GetCreatureTypeMask();
            crit_bonus = int32(crit_bonus * caster->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_CRIT_PERCENT_VERSUS, creatureTypeMask));
        }
    }

    if (crit_bonus > 0)
        damage += crit_bonus;

#ifdef LICH_KING
    if (caster)
        damage = int32(float(damage) * caster->GetTotalAuraMultiplier(SPELL_AURA_MOD_CRITICAL_HEALING_AMOUNT));
#endif

    return damage;
}

float Unit::SpellHealingPctDone(Unit* victim, SpellInfo const *spellProto)
{
    // For totems get healing bonus from owner (statue isn't totem in fact)
    if (GetTypeId() == TYPEID_UNIT && ToCreature()->IsTotem())
        if (Unit* owner = GetOwner())
            return owner->SpellHealingPctDone(victim, spellProto);

    // Some spells don't benefit from done mods
    if (spellProto->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS))
        return 1.0f;

#ifdef LICH_KING
    // sunwell: Some spells don't benefit from done mods
    if (spellProto->HasAttribute(SPELL_ATTR6_LIMIT_PCT_HEALING_MODS))
        return 1.0f;
#endif

    // No bonus healing for potion spells
    if (spellProto->SpellFamilyName == SPELLFAMILY_POTION)
        return 1.0f;

    float DoneTotalMod = 1.0f;

    // Healing done percent
    AuraEffectList const& mHealingDonePct = GetAuraEffectsByType(SPELL_AURA_MOD_HEALING_DONE_PERCENT);

    for (auto i : mHealingDonePct)
        AddPct(DoneTotalMod, i->GetAmount());

    // done scripted mod (take it from owner)
    Unit* owner = GetOwner() ? GetOwner() : this;
    AuraEffectList const& mOverrideClassScript = owner->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);

    for (auto i : mOverrideClassScript)
    {
        if (!i->IsAffectedOnSpell(spellProto))
            continue;

        switch (i->GetMiscValue())
        {
        case   21: // Test of Faith
        case 6935:
        case 6918:
            if (victim->HealthBelowPct(50))
                AddPct(DoneTotalMod, i->GetAmount());
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
            // sunwell: affected by any earth shield
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

uint32 Unit::SpellHealingBonusTaken(Unit* caster, SpellInfo const *spellProto, uint32 healamount, DamageEffectType damagetype)
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
    }

#ifdef LICH_KING
    // Tenacity increase healing % taken
    if (AuraEffect const* Tenacity = GetAuraEffect(58549, 0))
        AddPct(TakenTotalMod, Tenacity->GetAmount());
#endif

    //MIGHTY HACKS BLOCK
    {
        // Healing Wave cast (these are dummy auras)
        if (spellProto->SpellFamilyName == SPELLFAMILY_SHAMAN && spellProto->SpellFamilyFlags & 0x0000000000000040LL)
        {
            // Search for Healing Way on Victim (stack up to 3 time)
            Unit::AuraEffectList const& auraDummy = GetAuraEffectsByType(SPELL_AURA_DUMMY);

            for (auto itr : auraDummy)
            {
                if (itr->GetId() == 29203)
                {
                    uint32 percentIncrease = itr->GetAmount() * itr->GetBase()->GetStackAmount();
                    AddPct(TakenTotalMod, percentIncrease);
                    break;
                }
            }
        }
    }

#ifdef LICH_KING
    // Nourish cast
    if (spellProto->SpellFamilyName == SPELLFAMILY_DRUID && spellProto->SpellFamilyFlags[1] & 0x2000000)
    {
        // Rejuvenation, Regrowth, Lifebloom, or Wild Growth
        if (GetAuraEffect(SPELL_AURA_PERIODIC_HEAL, SPELLFAMILY_DRUID, 0x50, 0x4000010, 0))
            // increase healing by 20%
            TakenTotalMod *= 1.2f;
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


#ifdef LICH_KING
    if (caster)
    {
        TakenTotalMod *= GetTotalAuraMultiplier(SPELL_AURA_MOD_HEALING_RECEIVED, [caster, spellProto](AuraEffect const* aurEff) -> bool
        {
            if (caster->GetGUID() == aurEff->GetCasterGUID() && aurEff->IsAffectedOnSpell(spellProto))
                return true;
            return false;
        });
    }
#endif

    float heal = healamount * TakenTotalMod;
    return uint32(std::max(heal, 0.0f));
}

uint32 Unit::SpellHealingBonusDone(Unit* victim, SpellInfo const *spellProto, uint32 healamount, DamageEffectType damagetype, uint8 effIndex, Optional<float> const& donePctTotal, uint32 stack /* = 1 */)
{
    if (spellProto && spellProto->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS))
        return healamount;

    // For totems get healing bonus from owner (statue isn't totem in fact)
    if (GetTypeId() == TYPEID_UNIT && ToCreature()->IsTotem())
        if (Unit* owner = GetOwner())
            return owner->SpellHealingBonusDone(victim, spellProto, healamount, damagetype, effIndex, donePctTotal, stack);

    // No bonus healing for potion spells
    if (spellProto->SpellFamilyName == SPELLFAMILY_POTION)
        return healamount;

    float ApCoeffMod = 1.0f;
    float DoneTotalMod = donePctTotal ? *donePctTotal : SpellHealingPctDone(victim, spellProto);
    int32 DoneTotal = 0;

    // done scripted mod (take it from owner)
    Unit* owner = GetOwner() ? GetOwner() : this;
    AuraEffectList const& mOverrideClassScript = owner->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);

    for (auto i : mOverrideClassScript)
    {
        if (!i->IsAffectedOnSpell(spellProto))
            continue;
        switch (i->GetMiscValue())
        {
        case 4415: // Increased Rejuvenation Healing
        case 4953:
        case 3736: // Hateful Totem of the Third Wind / Increased Lesser Healing Wave / LK Arena (4/5/6) Totem of the Third Wind / Savage Totem of the Third Wind
            DoneTotal += i->GetAmount();
            break;
        }
    }

    // Done fixed damage bonus auras
    int32 DoneAdvertisedBenefit = SpellBaseHealingBonusDone(spellProto->GetSchoolMask());
    // modify spell power by victim's SPELL_AURA_MOD_HEALING auras (eg Amplify/Dampen Magic)
    //TC DoneAdvertisedBenefit += victim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_HEALING, spellProto->GetSchoolMask());
    DoneAdvertisedBenefit += victim->SpellBaseHealingBonusTaken(spellProto);

    // Pets just add their bonus damage to their spell damage
    // note that their spell damage is just gain of their own auras
    if (HasUnitTypeMask(UNIT_MASK_GUARDIAN))
        DoneAdvertisedBenefit += static_cast<Guardian const*>(this)->GetBonusDamage();

    // Check for table values
#ifdef LICH_KING
    float coeff = spellProto->Effects[effIndex].BonusMultiplier;
#else
    //sun: -1.0f = use default coef. This is also the value used in db spell_bonus_data
    float coeff = -1.0f;
#endif
    if (SpellBonusEntry const* bonus = sSpellMgr->GetSpellBonusData(spellProto->Id))
    {
        WeaponAttackType const attType = (spellProto->IsRangedWeaponSpell() && spellProto->DmgClass != SPELL_DAMAGE_CLASS_MELEE) ? RANGED_ATTACK : BASE_ATTACK;
        float APbonus = float(victim->GetTotalAuraModifier(attType == BASE_ATTACK ? SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS : SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS));
        APbonus += GetTotalAttackPowerValue(attType);

        if (damagetype == DOT)
        {
            coeff = bonus->dot_damage;
            if (bonus->ap_dot_bonus > 0)
                DoneTotal += int32(bonus->ap_dot_bonus * stack * ApCoeffMod * APbonus);
        }
        else
        {
            coeff = bonus->direct_damage;
            if (bonus->ap_bonus > 0)
                DoneTotal += int32(bonus->ap_dot_bonus * stack * ApCoeffMod * APbonus);
        }
    }
    else
    {
        // No bonus healing for SPELL_DAMAGE_CLASS_NONE class spells by default
        if (spellProto->DmgClass == SPELL_DAMAGE_CLASS_NONE)
            return uint32(std::max(healamount * DoneTotalMod, 0.0f));
    }

    // Default calculation
    if (DoneAdvertisedBenefit)
    {
        if (coeff < 0.f)
            coeff = CalculateDefaultCoefficient(spellProto, damagetype);

        float factorMod = CalculateSpellpowerCoefficientLevelPenalty(spellProto) * stack;
        if (Player* modOwner = GetSpellModOwner())
        {
            coeff *= 100.0f;
            modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_BONUS_MULTIPLIER, coeff);
            coeff /= 100.0f;
        }
        DoneTotal += int32(DoneAdvertisedBenefit * coeff * factorMod);
    }

    for (const auto & Effect : spellProto->Effects)
    {
        switch (Effect.ApplyAuraName)
        {
            // Bonus healing does not apply to these spells
        case SPELL_AURA_PERIODIC_LEECH:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
            DoneTotal = 0;
            break;
        }
        if (Effect.Effect == SPELL_EFFECT_HEALTH_LEECH)
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

    AuraEffectList const& mHealingDone = GetAuraEffectsByType(SPELL_AURA_MOD_HEALING_DONE);

    for(auto i : mHealingDone)
        if((i->GetMiscValue() & schoolMask) != 0)
            AdvertisedBenefit += i->GetAmount();

    // Healing bonus of spirit, intellect and strength
    if (GetTypeId() == TYPEID_PLAYER)
    {
        // Healing bonus from stats
        AuraEffectList const& mHealingDoneOfStatPercent = GetAuraEffectsByType(SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT);

        for(auto i : mHealingDoneOfStatPercent)
        {
            // stat used dependent from misc value (stat index)
            Stats usedStat = Stats(i->GetSpellInfo()->Effects[i->GetEffIndex()].MiscValue);
            AdvertisedBenefit += int32(GetStat(usedStat) * i->GetAmount() / 100.0f);
        }

        // ... and attack power
        AuraEffectList const& mHealingDonebyAP = GetAuraEffectsByType(SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER);

        for(auto i : mHealingDonebyAP)
            if (i->GetMiscValue() & schoolMask)
                AdvertisedBenefit += int32(GetTotalAttackPowerValue(BASE_ATTACK) * i->GetAmount() / 100.0f);
    }
    return AdvertisedBenefit;
}

int32 Unit::SpellBaseHealingBonusTaken(SpellInfo const* spellProto)
{
    int32 AdvertisedBenefit = 0;
    AuraEffectList const& mDamageTaken = GetAuraEffectsByType(SPELL_AURA_MOD_HEALING);

    for(auto i : mDamageTaken)
    {
        if((i->GetMiscValue() & spellProto->GetSchoolMask()) != 0)
            AdvertisedBenefit += i->GetAmount();

        //HACK
        if(i->GetId() == 34123) //tree of life "Increases healing received by 25% of the Tree of Life's total spirit." -> This means, add 25% drood spirit as healing bonus to healing spell taken
        {
            if(i->GetCaster() && i->GetCaster()->GetTypeId() == TYPEID_PLAYER)
                AdvertisedBenefit += int32(0.25f * (i->GetCaster()->ToPlayer())->GetStat(STAT_SPIRIT));
        }
    }


    // Blessing of Light dummy effects healing taken from Holy Light and Flash of Light
    if (spellProto->SpellFamilyName == SPELLFAMILY_PALADIN && (spellProto->SpellFamilyFlags & 0x00000000C0000000LL))
    {
        AuraEffectList const& mDummyAuras = GetAuraEffectsByType(SPELL_AURA_DUMMY);

        for (auto mDummyAura : mDummyAuras)
        {
            if (mDummyAura->GetSpellInfo()->HasVisual(9180))
            {
                // Flash of Light
                if ((spellProto->SpellFamilyFlags & 0x0000000040000000LL) && mDummyAura->GetEffIndex() == 1)
                    AdvertisedBenefit += mDummyAura->GetAmount();
                // Holy Light
                else if ((spellProto->SpellFamilyFlags & 0x0000000080000000LL) && mDummyAura->GetEffIndex() == 0)
                    AdvertisedBenefit += mDummyAura->GetAmount();
            }
            // Libram of the Lightbringer
            else if (mDummyAura->GetSpellInfo()->Id == 34231)
            {
                // Holy Light
                if ((spellProto->SpellFamilyFlags & 0x0000000080000000LL))
                    AdvertisedBenefit += mDummyAura->GetAmount();
            }
            // Blessed Book of Nagrand || Libram of Light || Libram of Divinity
            else if (mDummyAura->GetSpellInfo()->Id == 32403 || mDummyAura->GetSpellInfo()->Id == 28851 || mDummyAura->GetSpellInfo()->Id == 28853)
            {
                // Flash of Light
                if ((spellProto->SpellFamilyFlags & 0x0000000040000000LL))
                    AdvertisedBenefit += mDummyAura->GetAmount();
            }
        }
    }

    return AdvertisedBenefit;
}

uint32 Unit::GetSchoolImmunityMask() const
{
    uint32 mask = 0;
    //If m_immuneToSchool type contain this school type, IMMUNE damage.
    SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
    for (auto itr : schoolList)
        mask |= itr.type;

    return mask;
}

uint32 Unit::GetDamageImmunityMask() const
{
    uint32 mask = 0;

    //If m_immuneToDamage type contain magic, IMMUNE damage.
    SpellImmuneList const& damageList = m_spellImmune[IMMUNITY_DAMAGE];
    for (auto itr : damageList)
        mask |= itr.type;

    return mask;
}

uint32 Unit::GetMechanicImmunityMask() const
{
    uint32 mask = 0;

    SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
    for (auto itr : mechanicList)
        mask |= (1 << itr.type);

    return mask;
} 

bool Unit::IsImmunedToDamage(SpellSchoolMask schoolMask) const
{
    if (schoolMask == SPELL_SCHOOL_MASK_NONE)
        return false;

    // If m_immuneToSchool type contain this school type, IMMUNE damage.
    uint32 schoolImmunityMask = GetSchoolImmunityMask();
    if ((schoolImmunityMask & schoolMask) == schoolMask) // We need to be immune to all types
        return true;

    // If m_immuneToDamage type contain magic, IMMUNE damage.
    uint32 damageImmunityMask = GetDamageImmunityMask();
    if ((damageImmunityMask & schoolMask) == schoolMask) // We need to be immune to all types
        return true;

    return false;
}

bool Unit::IsImmunedToDamage(SpellInfo const* spellInfo) const
{
    if (!spellInfo)
        return false;

    // for example 40175
    if (spellInfo->HasAttribute(SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY) && spellInfo->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT))
        return false;

    if (spellInfo->HasAttribute(SPELL_ATTR1_UNAFFECTED_BY_SCHOOL_IMMUNE) || spellInfo->HasAttribute(SPELL_ATTR2_UNAFFECTED_BY_AURA_SCHOOL_IMMUNE))
        return false;

    if (uint32 schoolMask = spellInfo->GetSchoolMask())
    {
        // If m_immuneToSchool type contain this school type, IMMUNE damage.
        uint32 schoolImmunityMask = 0;
        SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
        for (auto itr = schoolList.begin(); itr != schoolList.end(); ++itr)
            if ((itr->type & schoolMask) && !spellInfo->CanPierceImmuneAura(sSpellMgr->GetSpellInfo(itr->spellId)))
                schoolImmunityMask |= itr->type;

        // // We need to be immune to all types
        if ((schoolImmunityMask & schoolMask) == schoolMask)
            return true;

        // If m_immuneToDamage type contain magic, IMMUNE damage.
        uint32 damageImmunityMask = GetDamageImmunityMask();
        if ((damageImmunityMask & schoolMask) == schoolMask) // We need to be immune to all types
            return true;
    }

    return false;
}
bool Unit::IsImmunedToSpell(SpellInfo const* spellInfo, WorldObject const* caster) const
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
    for(auto itr : idList)
        if(itr.type == spellInfo->Id)
            return true;

    if(spellInfo->Attributes & SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY)
        return false;

    SpellImmuneList const& dispelList = m_spellImmune[IMMUNITY_DISPEL];
    for(auto itr : dispelList)
        if(itr.type == spellInfo->Dispel)
            return true;

    // Spells that don't have effectMechanics.
    if (uint32 mechanic = spellInfo->Mechanic)
    {
        SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
        for (auto itr : mechanicList)
            if (itr.type == mechanic)
                return true;
    }

    bool immuneToAllEffects = true;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        // State/effect immunities applied by aura expect full spell immunity
        // Ignore effects with mechanic, they are supposed to be checked separately
        if (!spellInfo->Effects[i].IsEffect())
            continue;

        if (!IsImmunedToSpellEffect(spellInfo, i, caster))
        {
            immuneToAllEffects = false;
            break;
        }
    }

    if (immuneToAllEffects) //Return immune only if the target is immune to all spell effects.
        return true;

    if (uint32 schoolMask = spellInfo->GetSchoolMask())
    {
        uint32 schoolImmunityMask = 0;
        SpellImmuneList const& schoolList = m_spellImmune[IMMUNITY_SCHOOL];
        for (auto itr = schoolList.begin(); itr != schoolList.end(); ++itr)
        {
            if ((itr->type & schoolMask) == 0)
                continue;

            SpellInfo const* immuneSpellInfo = sSpellMgr->GetSpellInfo(itr->spellId);
            if (!(immuneSpellInfo && immuneSpellInfo->IsPositive() && spellInfo->IsPositive() && caster && IsFriendlyTo(caster)))
                if (!spellInfo->CanPierceImmuneAura(immuneSpellInfo))
                    schoolImmunityMask |= itr->type;
        }

        if ((schoolImmunityMask & schoolMask) == schoolMask)
            return true;
    }

    if(ToCreature() && ToCreature()->IsTotem())
        if(spellInfo->IsChanneled())
            return true;

    return false;
}

bool Unit::IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index, WorldObject const* caster) const
{
    if (!spellInfo || !spellInfo->Effects[index].IsEffect())
        return false;

    // sunwell: pet scaling auras
    if (spellInfo->HasAttribute(SPELL_ATTR4_IS_PET_SCALING))
        return false;

    if (spellInfo->HasAttribute(SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY) && !HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
        return false;

    //If m_immuneToEffect type contain this effect type, IMMUNE effect.
    uint32 effect = spellInfo->Effects[index].Effect;
    SpellImmuneList const& effectList = m_spellImmune[IMMUNITY_EFFECT];
    for (auto itr : effectList)
        if(itr.type == effect)
            return true;

    if (uint32 mechanic = spellInfo->Effects[index].Mechanic)
    {
        SpellImmuneList const& mechanicList = m_spellImmune[IMMUNITY_MECHANIC];
        for (auto itr : mechanicList)
            if (itr.type == mechanic)
                return true;
    }

    if (!spellInfo->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT))
    {
        if (uint32 aura = spellInfo->Effects[index].ApplyAuraName)
        {
            SpellImmuneList const& list = m_spellImmune[IMMUNITY_STATE];
            for (auto itr : list)
                if (itr.type == aura)
                    return true;

#ifdef LICH_KING
            if (!spellInfo->HasAttribute(SPELL_ATTR2_UNAFFECTED_BY_AURA_SCHOOL_IMMUNE))
            {
                // Check for immune to application of harmful magical effects
                AuraEffectList const& immuneAuraApply = GetAuraEffectsByType(SPELL_AURA_MOD_IMMUNE_AURA_APPLY_SCHOOL);
                for (AuraEffectList::const_iterator iter = immuneAuraApply.begin(); iter != immuneAuraApply.end(); ++iter)
                    if (((*iter)->GetMiscValue() & spellInfo->GetSchoolMask()) &&        // Check school
                        ((caster && !IsFriendlyTo(caster)) || !spellInfo->IsPositiveEffect(index))) // Harmful
                        return true;
            }
#endif
        }
    }

    return false;
}

uint32 Unit::MeleeDamageBonusTaken(Unit* attacker, uint32 pdamage, WeaponAttackType attType, SpellInfo const* spellProto /*= nullptr*/, SpellSchoolMask damageSchoolMask /*= SPELL_SCHOOL_MASK_NORMAL*/)
{
    if (pdamage == 0)
        return 0;

    int32 TakenFlatBenefit = 0;

    // ..taken
    TakenFlatBenefit += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_TAKEN, attacker->GetMeleeDamageSchoolMask());

    if (attType != RANGED_ATTACK)
        TakenFlatBenefit += GetTotalAuraModifier(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN);
    else
        TakenFlatBenefit += GetTotalAuraModifier(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN);

    // Taken total percent damage auras
    float TakenTotalMod = 1.0f;

    // ..taken
    TakenTotalMod *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, attacker->GetMeleeDamageSchoolMask());

#ifdef LICH_KING
    // .. taken pct (special attacks)
    if (spellProto)
    {
        // From caster spells
        TakenTotalMod *= GetTotalAuraMultiplier(SPELL_AURA_MOD_DAMAGE_FROM_CASTER, [attacker, spellProto](AuraEffect const* aurEff) -> bool
        {
            if (aurEff->GetCasterGUID() == attacker->GetGUID() && aurEff->IsAffectedOnSpell(spellProto))
                return true;
            return false;
        });

        // Mod damage from spell mechanic
        uint32 mechanicMask = spellProto->GetAllEffectsMechanicMask();

        // Shred, Maul - "Effects which increase Bleed damage also increase Shred damage"
        if (spellProto->SpellFamilyName == SPELLFAMILY_DRUID && spellProto->SpellFamilyFlags[0] & 0x00008800)
            mechanicMask |= (1 << MECHANIC_BLEED);

        if (mechanicMask)
        {
            TakenTotalMod *= GetTotalAuraMultiplier(SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT, [mechanicMask](AuraEffect const* aurEff) -> bool
            {
                if (mechanicMask & uint32(1 << (aurEff->GetMiscValue())))
                    return true;
                return false;
            });
        }
    }
#endif

    // .. taken pct: dummy auras
    AuraEffectList const& mDummyAuras = GetAuraEffectsByType(SPELL_AURA_DUMMY);
    for (AuraEffectList::const_iterator i = mDummyAuras.begin(); i != mDummyAuras.end(); ++i)
    {
        switch ((*i)->GetSpellInfo()->SpellIconID)
        {
        // Cheat Death
        case 2109:
            if ((*i)->GetMiscValue() & SPELL_SCHOOL_MASK_NORMAL)
            {
                if (GetTypeId() != TYPEID_PLAYER)
                    continue;
                float mod = ToPlayer()->GetRatingBonusValue(CR_CRIT_TAKEN_MELEE)*(-8.0f);
                AddPct(TakenTotalMod, std::max(mod, float((*i)->GetAmount())));
            }
            /* TC has
                // Patch 2.4.3: The resilience required to reach the 90% damage reduction cap
                //  is 22.5% critical strike damage reduction, or 444 resilience.
                // To calculate for 90%, we multiply the 100% by 4 (22.5% * 4 = 90%)
                float mod = -1.0f * GetMeleeCritDamageReduction(400);
            */
            break;
        //Mangle
        case 2312:
            if (spellProto == nullptr)
                break;
            // Should increase Shred (initial Damage of Lacerate and Rake handled in Spell::EffectSchoolDMG)
            if (spellProto->SpellFamilyName == SPELLFAMILY_DRUID && (spellProto->SpellFamilyFlags == 0x00008000LL))
                TakenTotalMod *= (100.0f + (*i)->GetAmount()) / 100.0f;
            break;
        }
    }

    // .. taken pct: class scripts
    //*AuraEffectList const& mclassScritAuras = GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    //for (AuraEffectList::const_iterator i = mclassScritAuras.begin(); i != mclassScritAuras.end(); ++i)
    //{
    //    switch ((*i)->GetMiscValue())
    //    {
    //    }
    //}*/

    if (attType != RANGED_ATTACK)
        TakenTotalMod *= GetTotalAuraMultiplier(SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT);
    else
        TakenTotalMod *= GetTotalAuraMultiplier(SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT);

#ifdef LICH_KING
    // Sanctified Wrath (bypass damage reduction)
    if (TakenTotalMod < 1.0f)
    {
        SpellSchoolMask const attackSchoolMask = spellProto ? spellProto->GetSchoolMask() : damageSchoolMask;

        float damageReduction = 1.0f - TakenTotalMod;
        Unit::AuraEffectList const& casterIgnoreResist = attacker->GetAuraEffectsByType(SPELL_AURA_MOD_IGNORE_TARGET_RESIST);
        for (AuraEffect const* aurEff : casterIgnoreResist)
        {
            if (!(aurEff->GetMiscValue() & attackSchoolMask))
                continue;

            AddPct(damageReduction, -aurEff->GetAmount());
        }

        TakenTotalMod = 1.0f - damageReduction;
    }
#endif

    float tmpDamage = float(pdamage + TakenFlatBenefit) * TakenTotalMod;
    return uint32(std::max(tmpDamage, 0.0f));
}

uint32 Unit::MeleeDamageBonusDone(Unit* victim, uint32 pdamage, WeaponAttackType attType, SpellInfo const* spellProto /*= nullptr*/, SpellSchoolMask damageSchoolMask /*= SPELL_SCHOOL_MASK_NORMAL*/)
{
    if (!victim || pdamage == 0)
        return 0;

    uint32 creatureTypeMask = victim->GetCreatureTypeMask();

    // Taken/Done fixed damage bonus auras
    int32 DoneFlatBenefit = 0;

    // ..done (for creature type by mask) in taken
    DoneFlatBenefit += GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_DAMAGE_DONE_CREATURE, creatureTypeMask);

    // ..done
    // SPELL_AURA_MOD_DAMAGE_DONE included in weapon damage

    // ..done (base at attack power for marked target and base at attack power for creature type)
    float APBonus = GetAPBonusVersus(attType, victim);
    if (APBonus != 0.0f)                                         // Can be negative
    {
        bool normalized = false;
        if(spellProto)
        {
            for (const auto & Effect : spellProto->Effects)
            {
                if (Effect.Effect == SPELL_EFFECT_NORMALIZED_WEAPON_DMG)
                {
                    normalized = true;
                    break;
                }
            }
        }

        DoneFlatBenefit += int32((APBonus/14.0f) * GetAPMultiplier(attType,normalized));
    }

    // Done total percent damage auras
    float DoneTotalMod = 1;

    SpellSchoolMask schoolMask = spellProto ? spellProto->GetSchoolMask() : damageSchoolMask;
    // mods for SPELL_SCHOOL_MASK_NORMAL are already factored in base melee damage calculation
    if (!(schoolMask & SPELL_SCHOOL_MASK_NORMAL))
    {
        // Some spells don't benefit from pct done mods
        if (!spellProto || !spellProto->HasAttribute(SPELL_ATTR6_LIMIT_PCT_DAMAGE_MODS))
        {
            float maxModDamagePercentSchool = 0.0f;
            if (GetTypeId() == TYPEID_PLAYER)
            {
                for (uint32 i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
                    if (schoolMask & (1 << i))
                        maxModDamagePercentSchool = std::max(maxModDamagePercentSchool, GetFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT + i));
            }
            else
                maxModDamagePercentSchool = GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, schoolMask);

            DoneTotalMod *= maxModDamagePercentSchool;
        }
    }

    DoneTotalMod *= GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS, creatureTypeMask);

#ifdef LICH_KING
    // bonus against aurastate
    DoneTotalMod *= GetTotalAuraMultiplier(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE, [victim](AuraEffect const* aurEff) -> bool
    {
        if (victim->HasAuraState(AuraStateType(aurEff->GetMiscValue())))
            return true;
        return false;
    });
#endif

    // done scripted mod (take it from owner)
    Unit* owner = GetOwner() ? GetOwner() : this;
    AuraEffectList const& mOverrideClassScript = owner->GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraEffectList::const_iterator i = mOverrideClassScript.begin(); i != mOverrideClassScript.end(); ++i)
    {
        if (!(*i)->IsAffectedOnSpell(spellProto))
            continue;

        switch ((*i)->GetMiscValue())
        {
#ifdef LICH_KING
        // Tundra Stalker
        // Merciless Combat
        case 7277:
        {
            // Merciless Combat
            if ((*i)->GetSpellInfo()->SpellIconID == 2656)
            {
                if (!victim->HealthAbovePct(35))
                    AddPct(DoneTotalMod, (*i)->GetAmount());
            }
            // Tundra Stalker
            else
            {
                // Frost Fever (target debuff)
                if (victim->HasAura(55095))
                    AddPct(DoneTotalMod, (*i)->GetAmount());
        }
            break;
        }
        // Rage of Rivendare
        case 7293:
        {
            if (victim->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_DEATHKNIGHT, 0, 0x02000000, 0))
                AddPct(DoneTotalMod, (*i)->GetSpellInfo()->GetRank() * 2.0f);
            break;
        }
        // Marked for Death
        case 7598:
        case 7599:
        case 7600:
        case 7601:
        case 7602:
        {
            if (victim->GetAuraEffect(SPELL_AURA_MOD_STALKED, SPELLFAMILY_HUNTER, 0x400, 0, 0))
                AddPct(DoneTotalMod, (*i)->GetAmount());
            break;
        }
#endif
        // Dirty Deeds
        case 6427:
        case 6428:
        {
            if (victim->HasAuraState(AURA_STATE_HEALTHLESS_35_PERCENT, spellProto, this))
            {
                // effect 0 has expected value but in negative state
                int32 bonus = -(*i)->GetBase()->GetEffect(0)->GetAmount();
                AddPct(DoneTotalMod, bonus);
            }
            break;
        }
    }
    }

    float tmpDamage = float(int32(pdamage) + DoneFlatBenefit) * DoneTotalMod;

    // bonus result can be negative
    return uint32(std::max(tmpDamage, 0.0f));
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
        for (auto itr = m_spellImmune[op].begin(); itr != m_spellImmune[op].end(); ++itr)
        {
            if(itr->spellId == spellId)
            {
                m_spellImmune[op].erase(itr);
                break;
            }
        }
    }

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

float Unit::GetPPMProcChance(uint32 WeaponSpeed, float PPM, SpellInfo const* /*spellProto*/) const
{
    // proc per minute chance calculation
    if (PPM <= 0) 
        return 0.0f;

#ifdef LICH_KING
    // Apply chance modifer aura
    if (spellProto)
        if (Player* modOwner = GetSpellModOwner())
            modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_PROC_PER_MINUTE, PPM);
#endif

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

    if(Player* player = ToPlayer())
    {
        // unsummon pet
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
                    player->SetTemporaryUnsummonedPetNumber(pet->GetCharmInfo()->GetPetNumber());
                    player->SetOldPetSpell(pet->GetUInt32Value(UNIT_CREATED_BY_SPELL));
                }
                player->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT);
                return;
            }
        }
        player->SetTemporaryUnsummonedPetNumber(0);
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


void Unit::EngageWithTarget(Unit* enemy)
{
    if (!enemy)
        return;

    if (IsEngagedBy(enemy))
        return;

    if (CanHaveThreatList())
        m_threatManager.AddThreat(enemy, 0.0f, nullptr, true, true);
    else
        SetInCombatWith(enemy);

    if (Creature* creature = ToCreature())
        if (CreatureGroup* formation = creature->GetFormation())
            formation->MemberEngagingTarget(creature, enemy);
}

void Unit::AttackedTarget(Unit* target, bool canInitialAggro)
{
    if (!target->IsEngaged() && !canInitialAggro)
        return;
    target->EngageWithTarget(this);
    if (Unit* targetOwner = target->GetCharmerOrOwner())
        targetOwner->EngageWithTarget(this);

    Player* myPlayerOwner = GetCharmerOrOwnerPlayerOrPlayerItself();
    Player* targetPlayerOwner = target->GetCharmerOrOwnerPlayerOrPlayerItself();
    if (myPlayerOwner && targetPlayerOwner && !(myPlayerOwner->duel && myPlayerOwner->duel->opponent == targetPlayerOwner))
    {
        myPlayerOwner->UpdatePvP(true);
        myPlayerOwner->SetContestedPvP(targetPlayerOwner);
        myPlayerOwner->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);
    }
}

void Unit::ValidateAttackersAndOwnTarget()
{
    // iterate attackers
    UnitVector toRemove;
    AttackerSet const& attackers = GetAttackers();
    for (Unit* attacker : attackers)
        if (!attacker->IsValidAttackTarget(this))
            toRemove.push_back(attacker);

    for (Unit* attacker : toRemove)
        attacker->AttackStop();

    // remove our own victim
    if (Unit* victim = GetVictim())
        if (!IsValidAttackTarget(victim))
            AttackStop();
}

void Unit::SetImmuneToAll(bool apply, bool keepCombat)
{
    if (apply)
    {
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        ValidateAttackersAndOwnTarget();
        if (keepCombat)
            m_threatManager.UpdateOnlineStates(true, true);
        else
            m_combatManager.EndAllCombat();
    }
    else
    {
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        m_threatManager.UpdateOnlineStates(true, true);
    }
}

void Unit::SetImmuneToPC(bool apply, bool keepCombat)
{
    if (apply)
    {
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        ValidateAttackersAndOwnTarget();
        if (keepCombat)
            m_threatManager.UpdateOnlineStates(true, true);
        else
        {
            std::list<CombatReference*> toEnd;
            for (auto const& pair : m_combatManager.GetPvECombatRefs())
                if (pair.second->GetOther(this)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                    toEnd.push_back(pair.second);
            for (auto const& pair : m_combatManager.GetPvPCombatRefs())
                if (pair.second->GetOther(this)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                    toEnd.push_back(pair.second);
            for (CombatReference* ref : toEnd)
                ref->EndCombat();
        }
    }
    else
    {
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        m_threatManager.UpdateOnlineStates(true, true);
    }
}

void Unit::SetImmuneToNPC(bool apply, bool keepCombat)
{
    if (apply)
    {
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        ValidateAttackersAndOwnTarget();
        if (keepCombat)
            m_threatManager.UpdateOnlineStates(true, true);
        else
        {
            std::list<CombatReference*> toEnd;
            for (auto const& pair : m_combatManager.GetPvECombatRefs())
                if (!pair.second->GetOther(this)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                    toEnd.push_back(pair.second);
            for (auto const& pair : m_combatManager.GetPvPCombatRefs())
                if (!pair.second->GetOther(this)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
                    toEnd.push_back(pair.second);
            for (CombatReference* ref : toEnd)
                ref->EndCombat();
        }
    }
    else
    {
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        m_threatManager.UpdateOnlineStates(true, true);
    }
}

bool Unit::IsThreatened() const
{
    return !m_threatManager.IsThreatListEmpty();
}

void Unit::OutDebugInfo() const
{
    TC_LOG_ERROR("entities.unit", "Unit::OutDebugInfo");
    TC_LOG_DEBUG("entities.unit", "%s name %s", GetGUID().ToString().c_str(), GetName().c_str());
    TC_LOG_DEBUG("entities.unit", "Owner %s, Minion %s, Charmer %s, Charmed %s", GetOwnerGUID().ToString().c_str(), GetMinionGUID().ToString().c_str(), GetCharmerGUID().ToString().c_str(), GetCharmGUID().ToString().c_str());
    TC_LOG_DEBUG("entities.unit", "In world %u, unit type mask %u", (uint32)(IsInWorld() ? 1 : 0), m_unitTypeMask);
    if (IsInWorld())
        TC_LOG_DEBUG("entities.unit", "Mapid %u", GetMapId());

    std::ostringstream o;
    o << "Summon Slot: ";
    for (uint32 i = 0; i < MAX_SUMMON_SLOT; ++i)
        o << m_SummonSlot[i].ToString() << ", ";

    TC_LOG_DEBUG("entities.unit", "%s", o.str().c_str());
    o.str("");

    o << "Controlled List: ";
    for (ControlList::const_iterator itr = m_Controlled.begin(); itr != m_Controlled.end(); ++itr)
        o << (*itr)->GetGUID().ToString() << ", ";
    TC_LOG_DEBUG("entities.unit", "%s", o.str().c_str());
    o.str("");

    o << "Aura List: ";
    for (AuraApplicationMap::const_iterator itr = m_appliedAuras.begin(); itr != m_appliedAuras.end(); ++itr)
        o << itr->first << ", ";
    TC_LOG_DEBUG("entities.unit", "%s", o.str().c_str());
    o.str("");

#ifdef LICH_KING
    if (IsVehicle())
    {
        o << "Passenger List: ";
        for (SeatMap::iterator itr = GetVehicleKit()->Seats.begin(); itr != GetVehicleKit()->Seats.end(); ++itr)
            if (Unit* passenger = ObjectAccessor::GetUnit(*GetVehicleBase(), itr->second.Passenger.Guid))
                o << passenger->GetGUID().ToString() << ", ";
        TC_LOG_DEBUG("entities.unit", "%s", o.str().c_str());
    }

    if (GetVehicle())
        TC_LOG_DEBUG("entities.unit", "On vehicle %u.", GetVehicleBase()->GetEntry());
#endif
}

void CharmInfo::SetIsCommandAttack(bool val)
{
    _isCommandAttack = val;
}

bool CharmInfo::IsCommandAttack()
{
    return _isCommandAttack;
}

void CharmInfo::SetIsCommandFollow(bool val)
{
    _isCommandFollow = val;
}

bool CharmInfo::IsCommandFollow()
{
    return _isCommandFollow;
}

void CharmInfo::SaveStayPosition()
{
    //! At this point a new spline destination is enabled because of Unit::StopMoving()
    G3D::Vector3 stayPos = _unit->movespline->FinalDestination();

    if (_unit->movespline->onTransport)
        if (TransportBase* transport = _unit->GetTransport())
            transport->CalculatePassengerPosition(stayPos.x, stayPos.y, stayPos.z);

    _stayX = stayPos.x;
    _stayY = stayPos.y;
    _stayZ = stayPos.z;
}

void CharmInfo::GetStayPosition(float &x, float &y, float &z)
{
    x = _stayX;
    y = _stayY;
    z = _stayZ;
}

void CharmInfo::SetIsAtStay(bool val)
{
    _isAtStay = val;
}

bool CharmInfo::IsAtStay()
{
    return _isAtStay;
}

void CharmInfo::SetIsFollowing(bool val)
{
    _isFollowing = val;
}

bool CharmInfo::IsFollowing()
{
    return _isFollowing;
}

void CharmInfo::SetIsReturning(bool val)
{
    _isReturning = val;
}

bool CharmInfo::IsReturning()
{
    return _isReturning;
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
           || !p->IsVisible()
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

bool Unit::IsTargetableForAttack(bool checkFakeDeath) const
{
    if (!IsAlive())
        return false;

    if (HasFlag(UNIT_FIELD_FLAGS,
        UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE))
        return false;

    if (GetTypeId() == TYPEID_PLAYER && ToPlayer()->IsGameMaster())
        return false;

    return !HasUnitState(UNIT_STATE_UNATTACKABLE) && (!checkFakeDeath || !HasUnitState(UNIT_STATE_DIED));
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
        SetLastDamagedTime(time(nullptr));

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

int32 Unit::GetHealthGain(int32 dVal)
{
    int32 gain = 0;

    if (dVal == 0)
        return 0;

    int32 curHealth = (int32)GetHealth();

    int32 val = dVal + curHealth;
    if (val <= 0)
    {
        return -curHealth;
    }

    int32 maxHealth = (int32)GetMaxHealth();

    if (val < maxHealth)
        gain = dVal;
    else if (curHealth != maxHealth)
        gain = maxHealth - curHealth;

    return gain;
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

/*
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
    
    AuraEffectList const& auras = u->GetAuraEffectsByType(SPELL_AURA_MOD_STALKED); // Hunter mark

    for (auto aura : auras)
        if (aura->GetCasterGUID() == GetGUID())
            return true;

    // Common invisibility mask
    Unit::AuraEffectList const& iAuras = u->GetAuraEffectsByType(SPELL_AURA_MOD_INVISIBILITY);

    Unit::AuraEffectList const& dAuras = GetAuraEffectsByType(SPELL_AURA_MOD_INVISIBILITY_DETECTION);

    if (uint32 mask = (m_detectInvisibilityMask & u->m_invisibilityMask)) {
        for (uint32 i = 0; i < 10; ++i) {
            if (((1 << i) & mask) == 0)
                continue;

            // find invisibility level
            uint32 invLevel = 0;
            for (auto iAura : iAuras)
                if ((iAura->GetMiscValue()) == i && invLevel < iAura->GetAmount())
                    invLevel = iAura->GetAmount();

            // find invisibility detect level
            uint32 detectLevel = 0;
            if (i == 6 && GetTypeId() == TYPEID_PLAYER) // special drunk detection case
                detectLevel = (this->ToPlayer())->GetDrunkValue();
            else {
                for (auto dAura : dAuras)
                    if ((dAura->GetMiscValue()) == i && detectLevel < dAura->GetAmount())
                        detectLevel = dAura->GetAmount();
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

    AuraEffectList const& auras = target->GetAuraEffectsByType(SPELL_AURA_MOD_STALKED); // Hunter mark

    for (auto aura : auras)
        if (aura->GetCasterGUID() == GetGUID())
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
    visibleDistance += (float)(GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_STEALTH_DETECT, 0) /2.0f); //spells like Track Hidden have 30 here, so you can see 15 yards further. Spells with miscvalue != 0 aren't meant to detect units but traps only
    
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

void Unit::SetVisibility(UnitVisibility x)
{
    m_Visibility = x;

    if (IsInWorld())
        UpdateObjectVisibility();

    if(x == VISIBILITY_GROUP_STEALTH)
        DestroyForNearbyPlayers();
}
*/

bool Unit::IsAlwaysVisibleFor(WorldObject const* seer) const
{
    if (WorldObject::IsAlwaysVisibleFor(seer))
        return true;

    // Always seen by owner
    if (ObjectGuid guid = GetCharmerOrOwnerGUID())
        if (seer->GetGUID() == guid)
            return true;

    if (Player const* seerPlayer = seer->ToPlayer())
        if (Unit* owner = GetOwner())
            if (Player* ownerPlayer = owner->ToPlayer())
                if (ownerPlayer->IsGroupVisibleFor(seerPlayer))
                    return true;

    return false;
}

bool Unit::IsAlwaysDetectableFor(WorldObject const* seer) const
{
    if (WorldObject::IsAlwaysDetectableFor(seer))
        return true;

    if (HasAuraTypeWithCaster(SPELL_AURA_MOD_STALKED, seer->GetGUID()))
        return true;

    return false;
}

bool Unit::IsVisible() const
{
    return (m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GM) > SEC_PLAYER) ? false : true;
}

void Unit::SetVisible(bool x)
{
    if (!x)
        m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_GAMEMASTER1);
    else
        m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GM, SEC_PLAYER);

    UpdateObjectVisibility();
}


void Unit::SetPhaseMask(uint32 newPhaseMask, bool update)
{
    if (newPhaseMask == GetPhaseMask())
        return;

    // Phase player, dont update
    WorldObject::SetPhaseMask(newPhaseMask, false);

    // Phase pets and summons
    if (IsInWorld())
    {
        for (ControlList::const_iterator itr = m_Controlled.begin(); itr != m_Controlled.end(); ++itr)
            if ((*itr)->GetTypeId() == TYPEID_UNIT)
                (*itr)->SetPhaseMask(newPhaseMask, true);

        for (uint8 i = 0; i < MAX_SUMMON_SLOT; ++i)
            if (m_SummonSlot[i])
                if (Creature* summon = GetMap()->GetCreature(m_SummonSlot[i]))
                    summon->SetPhaseMask(newPhaseMask, true);

        RemoveNotOwnSingleTargetAuras(newPhaseMask); // we can lost access to caster or target
    }

    // Update visibility after phasing pets and summons so they wont despawn
    if (update)
        UpdateObjectVisibility();
}


void Unit::UpdateObjectVisibility(bool forced)
{
    if (!forced)
        AddToNotify(NOTIFY_VISIBILITY_CHANGED);
    else
    {
        m_threatManager.UpdateOnlineStates(true, true);
        WorldObject::UpdateObjectVisibility(true);
        // call MoveInLineOfSight for nearby creatures
        Trinity::AIRelocationNotifier notifier(*this);
        Cell::VisitAllObjects(this, notifier, GetVisibilityRange());
    }
}

void Unit::UpdateSpeed(UnitMoveType mtype)
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
            TC_LOG_ERROR("entities.unit","Unit::UpdateSpeed: Unsupported move type (%d)", mtype);
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

    if (Creature* creature = ToCreature())
    {
        /* TC logic, why is this? (check Unit::AttackStop() in TC if enabling this)
        // for creature case, we check explicit if mob searched for assistance
        if (creature->HasSearchedAssistance())
            speed *= 0.66f;                                 // best guessed value, so this will be 33% reduction. Based off initial speed, mob can then "run", "walk fast" or "walk".
        */

        //adapt pet follow speed depending on distance from owner (if ooc)
        if (creature->HasUnitTypeMask(UNIT_MASK_MINION) && !creature->IsInCombat())
        {
            if (GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
            {
                Unit* followed = ASSERT_NOTNULL(dynamic_cast<AbstractFollower*>(GetMotionMaster()->GetCurrentMovementGenerator()))->GetTarget();
                if (followed && followed->GetGUID() == GetOwnerGUID() && !followed->IsInCombat())
                {
                    float ownerSpeed = followed->GetSpeedRate(mtype);
                    if (speed < ownerSpeed || creature->IsWithinDist3d(followed, 10.0f))
                        speed = ownerSpeed;
                    speed *= std::min(std::max(1.0f, float(0.75f + (GetDistance(followed) - PET_FOLLOW_DIST) * 0.05f)), 1.3f);
                }
            }
        }
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

    float oldSpeed = GetSpeedRate(mtype);
    SetSpeedRate(mtype, speed);
    if (Player* p = ToPlayer())
        p->GetSession()->anticheat->OnPlayerSpeedChanged(p, oldSpeed, speed);
}

/* return true speed */
float Unit::GetSpeed( UnitMoveType mtype ) const
{
    return m_speed_rate[mtype]*baseMoveSpeed[mtype];
}

/* Set speed rate of unit */
void Unit::SetSpeedRate(UnitMoveType mtype, float rate, bool sendUpdate /*= true*/)
{
    if (rate < 0)
        rate = 0.0f;

    // Update speed only on change
    if (m_speed_rate[mtype] == rate)
        return;

    m_speed_rate[mtype] = rate;

    PropagateSpeedChange();

    if (GetTypeId() == TYPEID_PLAYER)
    {
        // register forced speed changes for WorldSession::HandleForceSpeedChangeAck
        // and do it only for real sent packets and use run for run/mounted as client expected
        ++ToPlayer()->m_forced_speed_changes[mtype];

        /* TC logic, disabled, pets will now have their speed increased in Unit::UpdateSpeed when owner is far away, this this is not needed anymore.
        This was also breaking hunter talent "Bestial swiftness"
        if (m_speed_rate[mtype] >= 1.0f)
            if (!IsInCombat())
                if (Pet* pet = GetPet())
                    pet->SetSpeedRate(mtype, m_speed_rate[mtype], sendUpdate);*/
    }

    if (!sendUpdate)
        return;

    // Spline packets are for units controlled by AI. "Force speed change" (wrongly named opcodes) and "move set speed" packets are for units controlled by a player.
    static Opcodes const moveTypeToOpcode[MAX_MOVE_TYPE][3] =
    {
        { SMSG_SPLINE_SET_WALK_SPEED,        SMSG_FORCE_WALK_SPEED_CHANGE,           MSG_MOVE_SET_WALK_SPEED },
        { SMSG_SPLINE_SET_RUN_SPEED,         SMSG_FORCE_RUN_SPEED_CHANGE,            MSG_MOVE_SET_RUN_SPEED },
        { SMSG_SPLINE_SET_RUN_BACK_SPEED,    SMSG_FORCE_RUN_BACK_SPEED_CHANGE,       MSG_MOVE_SET_RUN_BACK_SPEED },
        { SMSG_SPLINE_SET_SWIM_SPEED,        SMSG_FORCE_SWIM_SPEED_CHANGE,           MSG_MOVE_SET_SWIM_SPEED },
        { SMSG_SPLINE_SET_SWIM_BACK_SPEED,   SMSG_FORCE_SWIM_BACK_SPEED_CHANGE,      MSG_MOVE_SET_SWIM_BACK_SPEED },
        { SMSG_SPLINE_SET_TURN_RATE,         SMSG_FORCE_TURN_RATE_CHANGE,            MSG_MOVE_SET_TURN_RATE },
        { SMSG_SPLINE_SET_FLIGHT_SPEED,      SMSG_FORCE_FLIGHT_SPEED_CHANGE,         MSG_MOVE_SET_FLIGHT_SPEED },
        { SMSG_SPLINE_SET_FLIGHT_BACK_SPEED, SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE,    MSG_MOVE_SET_FLIGHT_BACK_SPEED },
#ifdef LICH_KING
        { SMSG_SPLINE_SET_PITCH_RATE,        SMSG_FORCE_PITCH_RATE_CHANGE,           MSG_MOVE_SET_PITCH_RATE },
#endif
    };

    if (Player* playerMover = GetPlayerBeingMoved()) // unit controlled by a player.
    {
        // Send notification to self. this packet is only sent to one client (the client of the player concerned by the change).
        WorldPacket self;
        self.Initialize(moveTypeToOpcode[mtype][1], mtype != MOVE_RUN ? 8 + 4 + 4 : 8 + 4 + 1 + 4);
        self << GetPackGUID();
        self << (uint32)0;                                  // Movement counter. Unimplemented at the moment! NUM_PMOVE_EVTS = 0x39Z. 
        if (mtype == MOVE_RUN)
            self << uint8(1);                               // unknown byte added in 2.1.0
        self << float(GetSpeed(mtype));
        playerMover->GetSession()->SendPacket(&self);

        // Send notification to other players. sent to every clients (if in range) except one: the client of the player concerned by the change.
        /* Sunstrider: Bug with MSG_MOVE_SET_RUN_SPEED for players, either we shouldn't send this one, structure is incorrect, or info sent is wrong. Probably the latest.
        The bug was: fleeing players (first movement only) were seen going to another position for other players. BuildMovementPacket seems to be building information contradicting the previous SMSG_MONSTER_MOVE sent at move start.
        In the meanwhile, send SMSG_SPLINE_SET_RUN_SPEED seems to works flawlessly so... let's go with that
        **/
#ifdef LICH_KING
        WorldPacket data;
        data.Initialize(moveTypeToOpcode[mtype][2], 8 + 30 + 4);
        data << GetPackGUID();
        BuildMovementPacket(&data);
        data << float(GetSpeed(mtype));
        playerMover->SendMessageToSet(&data, false);
#else
        WorldPacket data;
        data.Initialize(moveTypeToOpcode[mtype][0], 8 + 4);
        data << GetPackGUID();
        data << float(GetSpeed(mtype));
        playerMover->SendMessageToSet(&data, false);
#endif
    }
    else // unit controlled by AI.
    {
        // send notification to every clients.
        WorldPacket data;
        data.Initialize(moveTypeToOpcode[mtype][0], 8 + 4);
        data << GetPackGUID();
        data << float(GetSpeed(mtype));
        SendMessageToSet(&data, false);
    }
}

void Unit::RemoveAllFollowers()
{
    while (!m_followingMe.empty())
        (*m_followingMe.begin())->SetTarget(nullptr);
}

void Unit::SetDeathState(DeathState s)
{
    if (s != ALIVE && s!= JUST_RESPAWNED)
    {
        CombatStop();
        GetThreatManager().ClearAllThreat();
        ClearComboPointHolders();                           // any combo points pointed to unit lost at it death

        if(IsNonMeleeSpellCast(false))
            InterruptNonMeleeSpells(false);

        RemoveAllAurasOnDeath();
        UnsummonAllTotems();
        RemoveAllControlled();
    }

    if (s == JUST_DIED)
    {
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
            GetMotionMaster()->Clear();
            GetMotionMaster()->MoveIdle();
        }
        StopMoving();
        DisableSpline();
        //without this when removing IncreaseMaxHealth aura player may stuck with 1 hp
        //dont know why since in IncreaseMaxHealth currenthealth is checked
        SetHealth(0);
        SetPower(GetPowerType(), 0);
        SetEmoteState(0);

        // players in instance don't have ZoneScript, but they have InstanceScript
        if (ZoneScript* zoneScript = GetZoneScript() ? GetZoneScript() : GetInstanceScript())
            zoneScript->OnUnitDeath(this);
    }
    else if(s == JUST_RESPAWNED)
    {
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE); // clear skinnable for creature and player (at battleground)
    }

    m_deathState = s;
}

void Unit::SetOwnerGUID(ObjectGuid owner)
{ 
    if (GetOwnerGUID() == owner)
        return;

    SetGuidValue(UNIT_FIELD_SUMMONEDBY, owner); 
    if (!owner)
        return;

    if (owner && owner.IsPlayer())
        m_ControlledByPlayer = true;
    else
        m_ControlledByPlayer = false;

    // Update owner dependent fields
    Player* player = ObjectAccessor::GetPlayer(*this, owner);
    if (!player || !player->HaveAtClient(this)) // if player cannot see this unit yet, he will receive needed data with create object
        return;
    
    SetFieldNotifyFlag(UF_FLAG_OWNER);

    UpdateData udata;
    WorldPacket packet;
    BuildValuesUpdateBlockForPlayer(&udata, player);
    udata.BuildPacket(&packet, player->GetSession()->GetClientBuild());
    player->SendDirectMessage(&packet);

    RemoveFieldNotifyFlag(UF_FLAG_OWNER);
}

void Unit::AtExitCombat()
{
#ifdef LICH_KING
    RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_LEAVE_COMBAT);
#endif
}

void Unit::UpdatePetCombatState()
{
    ASSERT(!IsPet()); // player pets do not use UNIT_FLAG_PET_IN_COMBAT for this purpose - but player pets should also never have minions of their own to call this

    bool state = false;
    for (Unit* minion : m_Controlled)
        if (minion->IsInCombat())
        {
            state = true;
            break;
        }

    if (state)
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);
    else
        RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);
}

//======================================================================

Aura* Unit::AddAura(uint32 spellId, Unit* target)
{
    if (!target)
        return nullptr;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return nullptr;

    return AddAura(spellInfo, MAX_EFFECT_MASK, target);
}

Aura* Unit::AddAura(SpellInfo const* spellInfo, uint8 effMask, Unit* target)
{
    if (!spellInfo)
        return nullptr;

    if (!target->IsAlive() && !spellInfo->IsPassive() && !spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_DEAD))
        return nullptr;

    if (target->IsImmunedToSpell(spellInfo, this))
        return nullptr;

    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (!(effMask & (1 << i)))
            continue;
        if (target->IsImmunedToSpellEffect(spellInfo, i, this))
            effMask &= ~(1 << i);
    }

    if (!effMask)
        return nullptr;

    AuraCreateInfo createInfo(spellInfo, effMask, target);
    createInfo.SetCaster(this);

    if (Aura* aura = Aura::TryRefreshStackOrCreate(createInfo))
    {
        aura->ApplyForTargets();
        return aura;
    }
    return nullptr;
}

void Unit::SetAuraStack(uint32 spellId, Unit* target, uint32 stack)
{
    Aura* aura = target->GetAura(spellId, GetGUID());
    if (!aura)
        aura = AddAura(spellId, target);
    if (aura && stack)
        aura->SetStackAmount(stack);
}


void Unit::SendPlaySpellVisual(uint32 id)
{
    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 8 + 4); //LK OK
    data << uint64(GetGUID());
    data << uint32(id); // SpellVisualKit.dbc index
    SendMessageToSet(&data, false);
}

int32 Unit::CalculateAOEAvoidance(int32 damage, uint32 schoolMask, ObjectGuid const& casterGuid) const
{
    damage = int32(float(damage) * GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_AOE_DAMAGE_AVOIDANCE, schoolMask));
#ifdef LICH_KING
    if (casterGuid.IsAnyTypeCreature())
        damage = int32(float(damage) * GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_CREATURE_AOE_DAMAGE_AVOIDANCE, schoolMask));
#endif
    return damage;
}

DiminishingLevels Unit::GetDiminishing(DiminishingGroup group) const
{
    DiminishingReturn const& diminish = m_Diminishing[group];
    if (!diminish.hitCount)
        return DIMINISHING_LEVEL_1;

    // If last spell was cast more than 15 seconds ago - reset level
    if (!diminish.stack && GetMSTimeDiffToNow(diminish.hitTime) > 15000)
        return DIMINISHING_LEVEL_1;

    return DiminishingLevels(diminish.hitCount);
}

void Unit::IncrDiminishing(SpellInfo const* auraSpellInfo, bool triggered)
{
    DiminishingGroup group = auraSpellInfo->GetDiminishingReturnsGroupForSpell(triggered);
    uint32 currentLevel = GetDiminishing(group);
    uint32 const maxLevel = auraSpellInfo->GetDiminishingReturnsMaxLevel(triggered);

    DiminishingReturn& diminish = m_Diminishing[group];
    if (currentLevel < maxLevel)
        diminish.hitCount = currentLevel + 1;
}

bool Unit::ApplyDiminishingToDuration(SpellInfo const* auraSpellInfo, bool triggered, int32 &duration, WorldObject* caster, DiminishingLevels previousLevel) const
{
    DiminishingGroup const group = auraSpellInfo->GetDiminishingReturnsGroupForSpell(triggered);
    if (duration == -1 || group == DIMINISHING_NONE)/*(caster->IsFriendlyTo(this) && caster != this)*/
        return true;

    //Hack to avoid incorrect diminishing on mind control
    if (group == DIMINISHING_CHARM && caster == this)
        return true;

    int32 const limitDuration = auraSpellInfo->GetDiminishingReturnsLimitDuration(triggered);

    // test pet/charm masters instead pets/charmedsz
    Unit const* targetOwner = GetCharmerOrOwner();
    Unit const* casterOwner = caster->GetCharmerOrOwner();

    // Duration of crowd control abilities on pvp target is limited by 10 sec. (2.2.0)
    if (limitDuration > 0 && duration > limitDuration)
    {
        Unit const* target = targetOwner ? targetOwner : this;
        WorldObject const* source = casterOwner ? casterOwner : caster;

        if(target->IsAffectedByDiminishingReturns() && source->GetTypeId() == TYPEID_PLAYER)
            duration = limitDuration;
    }

    float mod = 1.0f;

#ifdef LICH_KING
    TODO LK : diminishing returns on taunt;
#endif

    // Some diminishings applies to mobs too (for example, Stun)
    if(auraSpellInfo->GetDiminishingReturnsGroupType(group) == DRTYPE_ALL ||
               (auraSpellInfo->GetDiminishingReturnsGroupType(group) == DRTYPE_PLAYER
            && (targetOwner ? targetOwner->IsAffectedByDiminishingReturns() : IsAffectedByDiminishingReturns()))
      )
    {
        DiminishingLevels diminish = previousLevel;
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
    return (duration != 0);
}

void Unit::ClearDiminishings()
{
    for (DiminishingReturn& dim : m_Diminishing)
        dim.Clear();
}

void Unit::ApplyDiminishingAura(DiminishingGroup group, bool apply)
{
    // Checking for existing in the table
    DiminishingReturn& diminish = m_Diminishing[group];

    if (apply)
        ++diminish.stack;
    else if (diminish.stack)
    {
        --diminish.stack;

        // Remember time after last aura from group removed
        if (!diminish.stack)
            diminish.hitTime = GameTime::GetGameTimeMS();
    }
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

bool Unit::IsInSanctuary() const
{
    const AreaTableEntry *area = sAreaTableStore.LookupEntry(GetAreaId());
    if (area && (area->flags & AREA_FLAG_SANCTUARY || (sWorld->IsZoneSanctuary(area->ID)))) 
        return true;

    return false;
}

bool Unit::IsFFAPvP() const 
{ 
#ifdef LICH_KING
    return HasByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_1_UNK, UNIT_BYTE2_FLAG_FFA_PVP)
#else
    Unit const* checkUnit = GetOwner();
    if (!checkUnit)
        checkUnit = this;

    if(checkUnit->GetTypeId() == TYPEID_PLAYER)
        return checkUnit->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_FFA_PVP);

    return false;
#endif
}

/*#######################################
########                         ########
########       STAT SYSTEM       ########
########                         ########
#######################################*/

/*
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
*/

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

/*
float Unit::GetAmount(UnitMods unitMod, UnitModifierType modifierType) const
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
*/

float Unit::GetTotalStatValue(Stats stat) const
{
    UnitMods unitMod = UnitMods(UNIT_MOD_STAT_START + stat);

    if(GetPctModifierValue(unitMod, TOTAL_PCT) <= 0.0f)
        return 0.0f;

    // value = ((base_value * base_pct) + total_value) * total_pct
    float value = GetFlatModifierValue(unitMod, BASE_VALUE) + GetCreateStat(stat);
    value *= GetPctModifierValue(unitMod, BASE_PCT);
    value += GetFlatModifierValue(unitMod, TOTAL_VALUE);
    value *= GetPctModifierValue(unitMod, TOTAL_PCT);

    return value;
}

float Unit::GetTotalAuraModValue(UnitMods unitMod) const
{
    if(unitMod >= UNIT_MOD_END)
    {
        TC_LOG_ERROR("entities.unit","ERROR: trial to access non existed UnitMods in GetTotalAuraModValue()!");
        return 0.0f;
    }

    if(GetPctModifierValue(unitMod, BASE_PCT) <= 0.0f)
        return 0.0f;


    float value = GetFlatModifierValue(unitMod, BASE_VALUE);
    value *= GetPctModifierValue(unitMod, BASE_PCT);
    value += GetFlatModifierValue(unitMod, TOTAL_VALUE);
    
    //add dynamic flat mods
    if (unitMod == UNIT_MOD_ATTACK_POWER_RANGED && (GetClassMask() & CLASSMASK_WAND_USERS) == 0) {
        AuraEffectList const& mRAPbyIntellect = GetAuraEffectsByType(SPELL_AURA_MOD_RANGED_ATTACK_POWER_OF_STAT_PERCENT);

        for (auto i : mRAPbyIntellect)
            value += int32(GetStat(Stats(i->GetMiscValue())) * i->GetAmount() / 100.0f);
    }
    value *= GetPctModifierValue(unitMod, TOTAL_PCT);

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
    AuraEffectList const& mCreatureAttackPower = GetAuraEffectsByType(versusBonusType);

    for(auto itr : mCreatureAttackPower)
        if(creatureTypeMask & uint32(itr->GetMiscValue()))
            bonus += itr->GetAmount();

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

float Unit::GetWeaponDamageRange(WeaponAttackType attType, WeaponDamageRange type, uint8 damageIndex /*= 0*/) const
{
    if (attType == OFF_ATTACK && !HaveOffhandWeapon())
        return 0.0f;

    return m_weaponDamage[attType][type][damageIndex];
}

void Unit::SetLevel(uint32 lvl)
{
    SetUInt32Value(UNIT_FIELD_LEVEL, lvl);

    // group update
    if ((GetTypeId() == TYPEID_PLAYER) && (this->ToPlayer())->GetGroup())
        (this->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_LEVEL);

    if (GetTypeId() == TYPEID_PLAYER)
        sCharacterCache->UpdateCharacterLevel(ToPlayer()->GetGUID().GetCounter(), lvl);
}

void Unit::SetHealth(uint32 val)
{
    //sun: little addition to handle > max int32 values
    if (val > std::numeric_limits<int32>::max())
        val = std::numeric_limits<int32>::max();

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

float Unit::CountPctFromMaxHealth(float pct) const
{ 
    return CalculatePct<float, float>(GetMaxHealth(), pct);
}

uint32 Unit::CountPctFromMaxHealth(int32 pct) const 
{ 
    return CalculatePct<uint32, int32>(GetMaxHealth(), pct);
}

float Unit::CountPctFromCurHealth(float pct) const
{ 
    return CalculatePct<float, float>(GetHealth(), pct);
}

uint32 Unit::CountPctFromCurHealth(int32 pct) const 
{ 
    return CalculatePct<uint32, int32>(GetHealth(), pct);
}

void Unit::SetMaxHealth(uint32 val)
{
    //sun: little addition to handle > max int32 values
    if (val > std::numeric_limits<int32>::max())
        val = std::numeric_limits<int32>::max();

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


void Unit::UpdateResistanceBuffModsMod(SpellSchools school)
{
    float modPos = 0.0f;
    float modNeg = 0.0f;

    // these auras are always positive
    modPos = GetMaxPositiveAuraModifierByMiscMask(SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE, 1 << school);
    modPos += GetTotalAuraModifier(SPELL_AURA_MOD_RESISTANCE, [school](AuraEffect const* aurEff) -> bool
    {
        if ((aurEff->GetMiscValue() & (1 << school)) && aurEff->GetAmount() > 0)
            return true;
        return false;
    });

    modNeg = GetTotalAuraModifier(SPELL_AURA_MOD_RESISTANCE, [school](AuraEffect const* aurEff) -> bool
    {
        if ((aurEff->GetMiscValue() & (1 << school)) && aurEff->GetAmount() < 0)
            return true;
        return false;
    });

    float factor = GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_RESISTANCE_PCT, 1 << school);
    modPos *= factor;
    modNeg *= factor;

    SetFloatValue(UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE + school, modPos);
    SetFloatValue(UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE + school, modNeg);
}

void Unit::InitStatBuffMods()
{
    for (uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetFloatValue(UNIT_FIELD_POSSTAT0+i, 0);
    for (uint8 i = STAT_STRENGTH; i < MAX_STATS; ++i)
        SetFloatValue(UNIT_FIELD_NEGSTAT0+i, 0);
}

void Unit::UpdateStatBuffMod(Stats stat)
{
    float modPos = 0.0f;
    float modNeg = 0.0f;
    float factor = 0.0f;

    UnitMods const unitMod = static_cast<UnitMods>(UNIT_MOD_STAT_START + stat);

    // includes value from items and enchantments
    float modValue = GetFlatModifierValue(unitMod, BASE_VALUE);
    if (modValue > 0.f)
        modPos += modValue;
    else
        modNeg += modValue;

    if (IsGuardian())
    {
        modValue = static_cast<Guardian*>(this)->GetBonusStatFromOwner(stat);
        if (modValue > 0.f)
            modPos += modValue;
        else
            modNeg += modValue;
    }

    modPos += GetTotalAuraModifier(SPELL_AURA_MOD_STAT, [stat](AuraEffect const* aurEff) -> bool
    {
        if ((aurEff->GetMiscValue() < 0 || aurEff->GetMiscValue() == int32(stat)) && aurEff->GetAmount() > 0)
            return true;
        return false;
    });

    modNeg += GetTotalAuraModifier(SPELL_AURA_MOD_STAT, [stat](AuraEffect const* aurEff) -> bool
    {
        if ((aurEff->GetMiscValue() < 0 || aurEff->GetMiscValue() == int32(stat)) && aurEff->GetAmount() < 0)
            return true;
        return false;
    });

    factor = GetTotalAuraMultiplier(SPELL_AURA_MOD_PERCENT_STAT, [stat](AuraEffect const* aurEff) -> bool
    {
        if (aurEff->GetMiscValue() == -1 || aurEff->GetMiscValue() == int32(stat))
            return true;
        return false;
    });

    factor *= GetTotalAuraMultiplier(SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE, [stat](AuraEffect const* aurEff) -> bool
    {
        if (aurEff->GetMiscValue() == -1 || aurEff->GetMiscValue() == int32(stat))
            return true;
        return false;
    });

    modPos *= factor;
    modNeg *= factor;

    SetFloatValue(UNIT_FIELD_POSSTAT0 + stat, modPos);
    SetFloatValue(UNIT_FIELD_NEGSTAT0 + stat, modNeg);
}

void Unit::AddToWorld()
{
    if(!IsInWorld())
    {
        WorldObject::AddToWorld();
    }
}

void Unit::RemoveFromWorld()
{
    // cleanup
    if(IsInWorld())
    {
        m_duringRemoveFromWorld = true;
#ifdef LICH_KING
        if (IsVehicle())
            RemoveVehicleKit();
#endif
        RemoveCharmAuras();
        RemoveBindSightAuras();
        RemoveNotOwnSingleTargetAuras();

        RemoveAllGameObjects();
        RemoveAllDynObjects();

#ifdef LICH_KING
        ExitVehicle();  // Remove applied auras with SPELL_AURA_CONTROL_VEHICLE
#endif
        UnsummonAllTotems();
        RemoveAllControlled();

        //TC RemoveAreaAurasDueToLeaveWorld();

        RemoveAllFollowers();

        if (IsCharmed())
            RemoveCharmedBy(nullptr);

        if (GetCharmerGUID())
        {
            TC_LOG_FATAL("entities.unit", "Unit %u has charmer guid when removed from world", GetEntry());
            ABORT();
        }
        if (Unit* owner = GetOwner())
        {
            if (owner->m_Controlled.find(this) != owner->m_Controlled.end())
            {
                TC_LOG_FATAL("entities.unit", "Unit %u is in controlled list of %u when removed from world", GetEntry(), owner->GetEntry());
                ABORT();
            }
        }

        WorldObject::RemoveFromWorld();
        m_duringRemoveFromWorld = false;
    }
}
void Unit::CleanupBeforeRemoveFromMap(bool finalCleanup)
{
    if (GetTransport())
    {
        GetTransport()->RemovePassenger(this);
        SetTransport(nullptr);
        m_movementInfo.transport.Reset();
        m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
    }

    if (GetTypeId() == TYPEID_UNIT && (this->ToCreature())->IsAIEnabled) {
        (this->ToCreature())->AI()->OnRemove();
    }
    // This needs to be before RemoveFromWorld to make GetCaster() return a valid pointer on aura removal
    InterruptNonMeleeSpells(true);

    if (IsInWorld())
        RemoveFromWorld();

    ASSERT(GetGUID());

    assert(m_uint32Values);

    //A unit may be in removelist and not in world, but it is still in grid
    //and may have some references during delete
    RemoveAllAuras();
    RemoveAllGameObjects();
    RemoveAllDynObjects();
    GetMotionMaster()->Clear();                    // remove different non-standard movement generators.

    if (finalCleanup)
        m_cleanupDone = true;

    m_Events.KillAllEvents(false);                      // non-delatable (currently casted spells) will not deleted now but it will deleted at call in Map::RemoveAllObjectsInRemoveList
    CombatStop();
    ClearComboPointHolders();
}

void Unit::CleanupsBeforeDelete(bool finalCleanup)
{
    CleanupBeforeRemoveFromMap(finalCleanup);
    
    WorldObject::CleanupsBeforeDelete(finalCleanup);
}

void Unit::UpdateCharmAI()
{
    switch (GetTypeId())
    {
    case TYPEID_UNIT:
        if (i_disabledAI) // disabled AI must be primary AI
        {
            if (!IsCharmed())
            {
                delete i_AI;
                i_AI = i_disabledAI;
                i_disabledAI = nullptr;

                if (GetTypeId() == TYPEID_UNIT)
                    ToCreature()->AI()->OnCharmed(nullptr, false);
            }
        }
        else
        {
            if (IsCharmed())
            {
                i_disabledAI = i_AI;
                if (IsPossessed()
#ifdef LICH_KING
                    || IsVehicle()
#endif
                    )
                    i_AI = new PossessedAI(this->ToCreature());
                else
                    i_AI = new PetAI(this->ToCreature());
            }
        }
        break;
    case TYPEID_PLAYER:
        if (IsCharmed()) // if we are currently being charmed, then we should apply charm AI
        {
            i_disabledAI = i_AI;

            UnitAI* newAI = nullptr;
            // first, we check if the creature's own AI specifies an override playerai for its owned players
            Unit* charmer = GetCharmer();
            if (charmer)
            {
                if (Creature* creatureCharmer = charmer->ToCreature())
                {
                    if (PlayerAI* charmAI = creatureCharmer->IsAIEnabled ? creatureCharmer->AI()->GetAIForCharmedPlayer(ToPlayer()) : nullptr)
                        newAI = charmAI;
                }
                else
                {
                    TC_LOG_ERROR("misc", "Attempt to assign charm AI to player %u who is charmed by non-creature " UI64FMTD ".", GetGUID().GetCounter(), GetCharmerGUID().GetRawValue());
                }
            }
            if (!newAI) // otherwise, we default to the generic one
                newAI = new SimpleCharmedPlayerAI(ToPlayer());
            i_AI = newAI;
            newAI->OnCharmed(charmer, true);
        }
        else
        {
            if (i_AI)
            {
                // we allow the charmed PlayerAI to clean up
                i_AI->OnCharmed(GetCharmer(), false);
                // then delete it
                delete i_AI;
            }
            else
            {
                TC_LOG_ERROR("misc", "Attempt to remove charm AI from player %u who doesn't currently have charm AI.", GetGUID().GetCounter());
            }
            // and restore our previous PlayerAI (if we had one)
            i_AI = i_disabledAI;
            i_disabledAI = nullptr;
            // IsAIEnabled gets handled in the caller
        }
        break;
    default:
        TC_LOG_ERROR("misc", "Attempt to update charm AI for unit " UI64FMTD ", which is neither player nor creature.", GetGUID().GetRawValue());
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
    m_charmInfo = nullptr;
}

CharmInfo::CharmInfo(Unit* unit)
: _unit(unit), _CommandState(COMMAND_FOLLOW), _petnumber(0),
  _isCommandAttack(false), _isCommandFollow(false), _isAtStay(false), _isFollowing(false), _isReturning(false),
  _stayX(0.0f), _stayY(0.0f), _stayZ(0.0f)
{
    for (uint8 i = 0; i < MAX_SPELL_CHARM; ++i)
        _charmspells[i].SetActionAndType(0, ACT_DISABLED);

    if(_unit->GetTypeId() == TYPEID_UNIT)
    {
        _oldReactState = (_unit->ToCreature())->GetReactState();
        (_unit->ToCreature())->SetReactState(REACT_PASSIVE);
    }
}

CharmInfo::~CharmInfo()
{
    if(_unit->GetTypeId() == TYPEID_UNIT)
    {
        (_unit->ToCreature())->SetReactState(_oldReactState);
    }
}

void CharmInfo::InitPetActionBar()
{
    // the first 3 SpellOrActions are attack, follow and stay
    for (uint32 i = 0; i < ACTION_BAR_INDEX_PET_SPELL_START - ACTION_BAR_INDEX_START; ++i)
        SetActionBar(ACTION_BAR_INDEX_START + i, COMMAND_ATTACK - i, ACT_COMMAND);

    // middle 4 SpellOrActions are spells/special attacks/abilities
    for (uint32 i = 0; i < ACTION_BAR_INDEX_PET_SPELL_END - ACTION_BAR_INDEX_PET_SPELL_START; ++i)
        SetActionBar(ACTION_BAR_INDEX_PET_SPELL_START + i, 0, ACT_PASSIVE);

    // last 3 SpellOrActions are reactions
    for (uint32 i = 0; i < ACTION_BAR_INDEX_END - ACTION_BAR_INDEX_PET_SPELL_END; ++i)
        SetActionBar(ACTION_BAR_INDEX_PET_SPELL_END + i, COMMAND_ATTACK - i, ACT_REACTION);
}

void CharmInfo::InitEmptyActionBar(bool withAttack)
{
    if (withAttack)
        SetActionBar(ACTION_BAR_INDEX_START, COMMAND_ATTACK, ACT_COMMAND);
    else
        SetActionBar(ACTION_BAR_INDEX_START, 0, ACT_PASSIVE);
    for (uint32 x = ACTION_BAR_INDEX_START + 1; x < ACTION_BAR_INDEX_END; ++x)
        SetActionBar(x, 0, ACT_PASSIVE);
}

void CharmInfo::InitPossessCreateSpells()
{
    if(_unit->GetTypeId() == TYPEID_UNIT)
    {
        // Adding switch until better way is found. Malcrom
        // Adding entrys to this switch will prevent COMMAND_ATTACK being added to pet bar.
        switch (_unit->GetEntry())
        {
#ifdef LICH_KING
        case 23575: // Mindless Abomination
        case 24783: // Trained Rock Falcon
        case 27664: // Crashin' Thrashin' Racer
        case 40281: // Crashin' Thrashin' Racer
#endif
        case 25653: // Power of the Blue Flight (Kil'Jaeden fight)
        case 23109: // Vengeful Spirit (Teron Gorefiend fight)
            break;
        default:
            InitEmptyActionBar();
            break;
        }


        for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
        {
            uint32 spellId = _unit->ToCreature()->m_spells[i];
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (spellInfo)
            {
                if (spellInfo->IsPassive())
                    _unit->CastSpell(_unit, spellInfo->Id, true);
                else
                    AddSpellToActionBar(spellInfo, ACT_PASSIVE, i % MAX_UNIT_ACTION_BAR_INDEX);
            }
        }
    }
    else
        InitEmptyActionBar();
}

void CharmInfo::InitCharmCreateSpells()
{
    if(_unit->GetTypeId() == TYPEID_PLAYER)                //charmed players don't have spells
    {
        InitEmptyActionBar();
        return;
    }

    InitPetActionBar();

    for (uint32 x = 0; x < MAX_SPELL_CHARM; ++x)
    {
        uint32 spellId = _unit->ToCreature()->m_spells[x];
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);

        if (!spellInfo)
        {
            _charmspells[x].SetActionAndType(spellId, ACT_DISABLED);
            continue;
        }

        if (spellInfo->IsPassive())
        {
            _unit->CastSpell(_unit, spellInfo->Id, true);
            _charmspells[x].SetActionAndType(spellId, ACT_PASSIVE);
        }
        else
        {
            _charmspells[x].SetActionAndType(spellId, ACT_DISABLED);

            ActiveStates newstate = ACT_PASSIVE;

            if (!spellInfo->IsAutocastable())
                newstate = ACT_PASSIVE;
            else
            {
                if (spellInfo->NeedsExplicitUnitTarget())
                {
                    newstate = ACT_ENABLED;
                    ToggleCreatureAutocast(spellInfo, true);
                }
                else
                    newstate = ACT_DISABLED;
            }

            AddSpellToActionBar(spellInfo, newstate);
        }
    }
}

bool CharmInfo::AddSpellToActionBar(SpellInfo const* spellInfo, ActiveStates newstate, uint8 preferredSlot)
{
    uint32 spell_id = spellInfo->Id;
    uint32 first_id = spellInfo->GetFirstRankSpell()->Id;

    ASSERT(preferredSlot < MAX_UNIT_ACTION_BAR_INDEX);
    // new spell rank can be already listed
    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
    {
        if (uint32 action = PetActionBar[i].GetAction())
        {
            if (PetActionBar[i].IsActionBarForSpell() && sSpellMgr->GetFirstSpellInChain(action) == first_id)
            {
                PetActionBar[i].SetAction(spell_id);
                return true;
            }
        }
    }

    // or use empty slot in other case
    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
    {
        uint8 j = (preferredSlot + i) % MAX_UNIT_ACTION_BAR_INDEX;
        if (!PetActionBar[j].GetAction() && PetActionBar[j].IsActionBarForSpell())
        {
            SetActionBar(j, spell_id, newstate == ACT_DECIDE ? spellInfo->IsAutocastable() ? ACT_DISABLED : ACT_PASSIVE : newstate);
            return true;
        }
    }
    return false;
}

bool CharmInfo::RemoveSpellFromActionBar(uint32 spell_id)
{
    uint32 first_id = sSpellMgr->GetFirstSpellInChain(spell_id);

    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
    {
        if (uint32 action = PetActionBar[i].GetAction())
        {
            if (PetActionBar[i].IsActionBarForSpell() && sSpellMgr->GetFirstSpellInChain(action) == first_id)
            {
                SetActionBar(i, 0, ACT_PASSIVE);
                return true;
            }
        }
    }

    return false;
}

void CharmInfo::ToggleCreatureAutocast(SpellInfo const* spellInfo, bool apply)
{
    if (spellInfo->IsPassive())
        return;

    for (uint32 x = 0; x < MAX_SPELL_CHARM; ++x)
        if (spellInfo->Id == _charmspells[x].GetAction())
            _charmspells[x].SetType(apply ? ACT_ENABLED : ACT_DISABLED);
}

void CharmInfo::SetPetNumber(uint32 petnumber, bool statwindow)
{
    _petnumber = petnumber;
    if (statwindow)
        _unit->SetUInt32Value(UNIT_FIELD_PETNUMBER, _petnumber);
    else
        _unit->SetUInt32Value(UNIT_FIELD_PETNUMBER, 0);
}

void CharmInfo::LoadPetActionBar(const std::string& data)
{
    InitPetActionBar();

    Tokenizer tokens(data, ' ');

    if (tokens.size() != (ACTION_BAR_INDEX_END - ACTION_BAR_INDEX_START) * 2)
        return;                                             // non critical, will reset to default

    uint8 index = ACTION_BAR_INDEX_START;
    Tokenizer::const_iterator iter = tokens.begin();
    for (; index < ACTION_BAR_INDEX_END; ++iter, ++index)
    {
        // use unsigned cast to avoid sign negative format use at long-> ActiveStates (int) conversion
        ActiveStates type = ActiveStates(atol(*iter));
        ++iter;
        uint32 action = atoul(*iter);

        PetActionBar[index].SetActionAndType(action, type);

        // check correctness
        if (PetActionBar[index].IsActionBarForSpell())
        {
            SpellInfo const* spelInfo = sSpellMgr->GetSpellInfo(PetActionBar[index].GetAction());
            if (!spelInfo)
                SetActionBar(index, 0, ACT_PASSIVE);
            else if (!spelInfo->IsAutocastable())
                SetActionBar(index, PetActionBar[index].GetAction(), ACT_PASSIVE);
        }
    }
}

void CharmInfo::BuildActionBar(WorldPacket* data)
{
    for (uint32 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
        *data << uint32(PetActionBar[i].packedData);
}

void CharmInfo::SetSpellAutocast(SpellInfo const* spellInfo, bool state)
{
    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
    {
        if (spellInfo->Id == PetActionBar[i].GetAction() && PetActionBar[i].IsActionBarForSpell())
        {
            PetActionBar[i].SetType(state ? ACT_ENABLED : ACT_DISABLED);
            break;
        }
    }
}

Unit* Unit::GetUnitBeingMoved() const
{
    if (Player const* player = ToPlayer())
        return player->m_unitMovedByMe;
    return nullptr;
}

Player* Unit::GetPlayerBeingMoved() const
{
    if (Unit* mover = GetUnitBeingMoved())
        return mover->ToPlayer();
    return nullptr;
}

bool Unit::IsFrozen() const
{
    return HasAuraState(AURA_STATE_FROZEN);
}

// List of auras that CAN be trigger but may not exist in spell_proc_event
// in most case need for drop charges
// in some types of aura need do additional check
// for example SPELL_AURA_MECHANIC_IMMUNITY - need check for mechanic
static bool IsTriggerAura[TOTAL_AURAS];
static bool isNonTriggerAura[TOTAL_AURAS];
void InitTriggerAuraData()
{
    for (int i=0;i<TOTAL_AURAS;i++)
    {
      IsTriggerAura[i]=false;
      isNonTriggerAura[i] = false;
    }
    IsTriggerAura[SPELL_AURA_DUMMY] = true;
    IsTriggerAura[SPELL_AURA_MOD_CONFUSE] = true;
    IsTriggerAura[SPELL_AURA_MOD_THREAT] = true;
    IsTriggerAura[SPELL_AURA_MOD_STUN] = true; // Aura not have charges but need remove him on trigger
    IsTriggerAura[SPELL_AURA_MOD_DAMAGE_DONE] = true;
    IsTriggerAura[SPELL_AURA_MOD_DAMAGE_TAKEN] = true;
    IsTriggerAura[SPELL_AURA_MOD_RESISTANCE] = true;
    IsTriggerAura[SPELL_AURA_MOD_ROOT] = true;
    IsTriggerAura[SPELL_AURA_REFLECT_SPELLS] = true;
    IsTriggerAura[SPELL_AURA_DAMAGE_IMMUNITY] = true;
    IsTriggerAura[SPELL_AURA_PROC_TRIGGER_SPELL] = true;
    IsTriggerAura[SPELL_AURA_PROC_TRIGGER_DAMAGE] = true;
    IsTriggerAura[SPELL_AURA_MOD_CASTING_SPEED] = true;
    IsTriggerAura[SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT] = true;
    IsTriggerAura[SPELL_AURA_MOD_POWER_COST_SCHOOL] = true;
    IsTriggerAura[SPELL_AURA_REFLECT_SPELLS_SCHOOL] = true;
    IsTriggerAura[SPELL_AURA_MECHANIC_IMMUNITY] = true;
    IsTriggerAura[SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN] = true;
    IsTriggerAura[SPELL_AURA_SPELL_MAGNET] = true;
    IsTriggerAura[SPELL_AURA_MOD_ATTACK_POWER] = true;
    IsTriggerAura[SPELL_AURA_ADD_CASTER_HIT_TRIGGER] = true;
    IsTriggerAura[SPELL_AURA_OVERRIDE_CLASS_SCRIPTS] = true;
    IsTriggerAura[SPELL_AURA_MOD_MECHANIC_RESISTANCE] = true;
    IsTriggerAura[SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS] = true;
    IsTriggerAura[SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS] = true;
    IsTriggerAura[SPELL_AURA_MOD_MELEE_HASTE] = true;
    IsTriggerAura[SPELL_AURA_MOD_ATTACKER_MELEE_HIT_CHANCE]=true;
    IsTriggerAura[SPELL_AURA_PRAYER_OF_MENDING] = true;

    isNonTriggerAura[SPELL_AURA_MOD_POWER_REGEN]=true;
    isNonTriggerAura[SPELL_AURA_RESIST_PUSHBACK]=true;
}

SpellSchoolMask Unit::GetMeleeDamageSchoolMask(WeaponAttackType attackType, uint8 damageIndex) const
{
    return SPELL_SCHOOL_MASK_NORMAL;
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

MovementGeneratorType Unit::GetDefaultMovementType() const
{
    return IDLE_MOTION_TYPE;
}

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

void Unit::StopMovingOnCurrentPos() // sunwell
{
    ClearUnitState(UNIT_STATE_MOVING);

    // not need send any packets if not in world
    if (!IsInWorld())
        return;

    DisableSpline(); // sunwell: required so Launch() won't recalculate position from previous spline
    Movement::MoveSplineInit init(this);
    init.MoveTo(GetPositionX(), GetPositionY(), GetPositionZ());
    init.SetFacing(GetOrientation());
    init.Launch();
}

void Unit::PauseMovement(uint32 timer/* = 0*/, uint8 slot/* = 0*/, bool forced/* = true*/)
{
    if (IsInvalidMovementSlot(slot))
        return;

    if (MovementGenerator* movementGenerator = GetMotionMaster()->GetCurrentMovementGenerator(MovementSlot(slot)))
        movementGenerator->Pause(timer);

    if (forced && GetMotionMaster()->GetCurrentSlot() == MovementSlot(slot))
        StopMoving();
}

void Unit::ResumeMovement(uint32 timer/* = 0*/, uint8 slot/* = 0*/)
{
    if (IsInvalidMovementSlot(slot))
        return;

    if (MovementGenerator* movementGenerator = GetMotionMaster()->GetCurrentMovementGenerator(MovementSlot(slot)))
        movementGenerator->Resume(timer);
}

void Unit::SendMovementFlagUpdate()
{
    WorldPacket data;
    BuildHeartBeatMsg(&data);
    SendMessageToSet(&data, false);
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
    SetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_STAND_STATE, state);

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

    return spellInfo->GetSpellSpecific() == SPELL_SPECIFIC_MAGE_POLYMORPH;
}

void Unit::SetDisplayId(uint32 modelId)
{
    SetUInt32Value(UNIT_FIELD_DISPLAYID, modelId);
 
}

void Unit::ClearComboPointHolders()
{
    while(!m_ComboPointHolders.empty())
    {
        uint32 lowguid = *m_ComboPointHolders.begin();

        Player* plr = ObjectAccessor::FindPlayer(ObjectGuid(HighGuid::Player, lowguid));
        if(plr && plr->GetComboTarget()==GetGUID())         // recheck for safe
            plr->ClearComboPoints();                        // remove also guid from m_ComboPointHolders;
        else
            m_ComboPointHolders.erase(lowguid);             // or remove manually
    }
}

void Unit::ClearAllReactives()
{

    for(uint32 & i : m_reactiveTimer)
        i = 0;

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

bool Unit::HasReactiveTimerActive(ReactiveType reactive) const
{
    return m_reactiveTimer[reactive] != 0;
}

Unit* Unit::SelectNearbyTarget(Unit* exclude, float dist) const
{
    std::list<Unit *> targets;
    Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(this, this, dist);
    Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(this, targets, u_check);
    Cell::VisitAllObjects(this, searcher, dist);

    // remove current target
    if(GetVictim())
        targets.remove(GetVictim());

    // remove not LoS targets
    for(auto tIter = targets.begin(); tIter != targets.end();)
    {
        if(!IsWithinLOSInMap(*tIter, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
        {
            auto tIter2 = tIter;
            ++tIter;
            targets.erase(tIter2);
        }
        else
            ++tIter;
    }

    // no appropriate targets
    if(targets.empty())
        return nullptr;

    // select random
    uint32 rIdx = urand(0,targets.size()-1);
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

void Unit::ApplyCastTimePercentMod(float val, bool apply)
{
    float amount = GetFloatValue(UNIT_MOD_CAST_SPEED);

    if (val > 0.f)
        ApplyPercentModFloatVar(amount, val, !apply);
    else
        ApplyPercentModFloatVar(amount, -val, apply);

    SetFloatValue(UNIT_MOD_CAST_SPEED, amount);
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

float Unit::GetAPMultiplier(WeaponAttackType attType, bool normalized) const
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

bool Unit::IsUnderLastManaUseEffect() const
{
    return  GetMSTimeDiff(m_lastManaUse, GameTime::GetGameTimeMS()) < 5000;
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

Pet* Unit::CreateTamedPetFrom(Creature* creatureTarget, uint32 spell_id)
{
    if (GetTypeId() != TYPEID_PLAYER)
        return NULL;

    Pet* pet = new Pet(ToPlayer(), HUNTER_PET);

    if(!pet->CreateBaseAtCreature(creatureTarget))
    {
        delete pet;
        return nullptr;
    }

    InitTamedPet(pet, creatureTarget->GetLevel(), spell_id);

    return pet;
}

Pet* Unit::CreateTamedPetFrom(uint32 creatureEntry, uint32 spell_id)
{
    if (GetTypeId() != TYPEID_PLAYER)
        return nullptr;

    CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(creatureEntry);
    if (!creatureInfo)
        return nullptr;

    Pet* pet = new Pet(ToPlayer(), HUNTER_PET);

    if (!pet->CreateBaseAtCreatureInfo(creatureInfo, this) || !InitTamedPet(pet, GetLevel(), spell_id))
    {
        delete pet;
        return nullptr;
    }

    return pet;
}

bool Unit::InitTamedPet(Pet* pet, uint8 level, uint32 spell_id)
{
    pet->SetCreatorGUID(GetGUID());
    pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, GetFaction());
    pet->SetUInt32Value(UNIT_CREATED_BY_SPELL, spell_id);

    if (GetTypeId() == TYPEID_PLAYER)
        pet->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    if (!pet->InitStatsForLevel(level))
    {
        TC_LOG_ERROR("entities.pet", "ERROR: InitStatsForLevel() failed for creature (Entry: %u)!", pet->GetEntry());
        return false;
    }

    pet->GetCharmInfo()->SetPetNumber(sObjectMgr->GeneratePetNumber(), true);
    // this enables pet details window (Shift+P)
    pet->AIM_Initialize();
    pet->InitPetCreateSpells();
    pet->SetFullHealth();
    return true;
}

/*-----------------------TRINITY-----------------------------*/

/*static*/ void Unit::Kill(Unit* attacker, Unit *pVictim, bool durabilityLoss)
{
    pVictim->SetHealth(0);

    // find player: owner of controlled `this` or `this` itself maybe
    Player* player = attacker ? attacker->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
    Creature* creature = pVictim->ToCreature();

    bool bRewardIsAllowed = true;
    if(pVictim->GetTypeId() == TYPEID_UNIT)
    {
        bRewardIsAllowed = (pVictim->ToCreature())->IsDamageEnoughForLootingAndReward();
        if(!bRewardIsAllowed)
            (pVictim->ToCreature())->SetLootRecipient(nullptr);
    }
    
    if(bRewardIsAllowed && pVictim->GetTypeId() == TYPEID_UNIT && (pVictim->ToCreature())->GetLootRecipient())
        player = (pVictim->ToCreature())->GetLootRecipient();

    // Exploit fix
    if (creature && creature->IsPet() && creature->GetOwnerGUID().IsPlayer())
        bRewardIsAllowed = false;

    // Reward player, his pets, and group/raid members
    // call kill spell proc event (before real die and combat stop to triggering auras removed at death/combat stop)
    if(bRewardIsAllowed && player && player != pVictim)
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
                loot->FillLoot(lootid, LootTemplates_Creature, looter, false, false, creature->GetLootMode());

            if (creature->GetLootMode() > 0)
                loot->generateMoneyLoot(creature->GetCreatureTemplate()->mingold, creature->GetCreatureTemplate()->maxgold);

            if (group)
            {
                if (hasLooterGuid)
                    group->SendLooter(creature, looter);
                else
                    group->SendLooter(creature, nullptr);

                // Update round robin looter only if the creature had loot
                if (!loot->empty())
                    group->UpdateLooterGuid(creature);
            }
        }

        player->RewardPlayerAndGroupAtKill(pVictim);
    }

    // Do KILL and KILLED procs. KILL proc is called only for the unit who landed the killing blow (and its owner - for pets and totems) regardless of who tapped the victim
    if (attacker && (attacker->IsPet() || attacker->IsTotem()))
    {
        // proc only once for victim
        if (Unit* owner = attacker->GetOwner())
            Unit::ProcSkillsAndAuras(owner, pVictim, PROC_FLAG_KILL, PROC_FLAG_NONE, PROC_SPELL_TYPE_MASK_ALL, PROC_SPELL_PHASE_NONE, PROC_HIT_NONE, nullptr, nullptr, nullptr);
    }

    if (!pVictim->IsCritter())
        Unit::ProcSkillsAndAuras(attacker, pVictim, PROC_FLAG_KILL, PROC_FLAG_KILLED, PROC_SPELL_TYPE_MASK_ALL, PROC_SPELL_PHASE_NONE, PROC_HIT_NONE, nullptr, nullptr, nullptr);

    // Proc auras on death - must be before aura/combat remove
    Unit::ProcSkillsAndAuras(pVictim, pVictim, PROC_FLAG_NONE, PROC_FLAG_DEATH, PROC_SPELL_TYPE_MASK_ALL, PROC_SPELL_PHASE_NONE, PROC_HIT_NONE, nullptr, nullptr, nullptr);

    // if talent known but not triggered (check priest class for speedup check)
    bool SpiritOfRedemption = false;
    if(pVictim->GetTypeId()==TYPEID_PLAYER && pVictim->GetClass()==CLASS_PRIEST )
    {
        AuraEffectList const& vDummyAuras = pVictim->GetAuraEffectsByType(SPELL_AURA_DUMMY);

        for(auto vDummyAura : vDummyAuras)
        {
            if(vDummyAura->GetSpellInfo()->SpellIconID==1654)
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
                pVictim->CastSpell(pVictim, 27827, vDummyAura);
                //just aesthetics, SoR appears full health and full mana on retail
                pVictim->SetFullHealth(); 
                pVictim->SetFullPower(POWER_MANA);

                SpiritOfRedemption = true;
                if(attacker)
                    (pVictim->ToPlayer())->SetSpiritRedeptionKiller(attacker->GetGUID());
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
        (pVictim->ToPlayer())->SetPvPDeath(player!=nullptr);

        // only if not player and not controlled by player pet. And not at BG
        if (durabilityLoss && !player && !(pVictim->ToPlayer())->InBattleground())
        {
            (pVictim->ToPlayer())->DurabilityLossAll(0.10f,false);
            // durability lost message
            WorldPacket data(SMSG_DURABILITY_DAMAGE_DEATH, 0);
            (pVictim->ToPlayer())->SendDirectMessage(&data);
        }
        // Call KilledUnit for creatures
        if (attacker && attacker->GetTypeId() == TYPEID_UNIT && (attacker->ToCreature())->IsAIEnabled) 
        {
            (attacker->ToCreature())->AI()->KilledUnit(pVictim);

            auto attackers = pVictim->GetAttackers();
            for(auto attacker : attackers)
            {
                if(Creature* c = attacker->ToCreature())
                    if(c->IsAIEnabled)
                        c->AI()->VictimDied(pVictim);
            }
        }
            
        if (attacker && attacker->GetTypeId() == TYPEID_PLAYER) 
        {
            if (ObjectGuid minipet_guid = attacker->GetCritterGUID())
            {
                if (Creature* minipet = ObjectAccessor::GetCreature(*attacker, minipet_guid))
                    if (minipet->IsAIEnabled)
                        minipet->AI()->MasterKilledUnit(pVictim);
            }
            if (Pet* pet = attacker->ToPlayer()->GetPet()) {
                if (pet->IsAIEnabled)
                    pet->AI()->MasterKilledUnit(pVictim);
            }
            for (ObjectGuid slot : attacker->m_SummonSlot) {
                if (Creature* totem = ObjectAccessor::GetCreature(*attacker, slot))
                    totem->AI()->MasterKilledUnit(pVictim);
            }
        }

        // last damage from non duel opponent or opponent controlled creature
        if((pVictim->ToPlayer())->duel)
        {
            (pVictim->ToPlayer())->duel->opponent->CombatStopWithPets(true);
            (pVictim->ToPlayer())->CombatStopWithPets(true);
            (pVictim->ToPlayer())->DuelComplete(DUEL_INTERRUPTED);
        }
        
        if (InstanceScript* instance = ((InstanceScript*)pVictim->GetInstanceScript()))
            instance->PlayerDied(pVictim->ToPlayer());
    }
    else                                                // creature died
    {
        Creature* cVictim = pVictim->ToCreature();
        
        if(!cVictim->IsPet())
        {
            //save threat list before deleting it
            if(cVictim->IsWorldBoss())
                cVictim->ConvertThreatListIntoPlayerListAtDeath();

            cVictim->GetThreatManager().ClearAllThreat();
            if(!cVictim->GetFormation() || !cVictim->GetFormation()->isLootLinked(cVictim)) //the flag is set when whole group is dead for those with linked loot 
                cVictim->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
        }

        // Call KilledUnit for creatures, this needs to be called after the lootable flag is set
        if (attacker && attacker->GetTypeId() == TYPEID_UNIT && attacker->IsAIEnabled) 
        {
            (attacker->ToCreature())->AI()->KilledUnit(pVictim);
        }
            
        if (attacker && attacker->GetTypeId() == TYPEID_PLAYER)
        {
            if (ObjectGuid minipet_guid = attacker->GetCritterGUID())
            {
                if (Creature* minipet = ObjectAccessor::GetCreature(*attacker, minipet_guid))
                    if (minipet->IsAIEnabled)
                        minipet->AI()->MasterKilledUnit(pVictim);
            }
            if (Pet* pet = attacker->ToPlayer()->GetPet()) 
            {
                if (pet->IsAIEnabled)
                    pet->AI()->MasterKilledUnit(pVictim);
            }
            for (ObjectGuid slot : attacker->m_SummonSlot) 
            {
                if (Creature* totem = ObjectAccessor::GetCreature(*attacker, slot))
                    totem->AI()->MasterKilledUnit(pVictim);
            }
        }

        // Call creature just died function
        if (cVictim->IsAIEnabled) 
            cVictim->AI()->JustDied(attacker);

        if (TempSummon* summon = creature->ToTempSummon())
            if (Unit* summoner = summon->GetSummoner())
                if (summoner->ToCreature() && summoner->IsAIEnabled)
                    summoner->ToCreature()->AI()->SummonedCreatureDies(creature, attacker);

        cVictim->WarnDeathToFriendly();
        
        // Despawn creature pet if alive
        if (Pet* pet = cVictim->GetPet()) {
            if (pet->IsAlive())
                pet->DisappearAndDie();
        }
        
        // Log down if worldboss
        if (attacker && cVictim->IsWorldBoss() && (cVictim->GetMap()->IsRaid() || cVictim->GetMap()->IsWorldMap()))
        {
            attacker->LogBossDown(cVictim);
        }

        // Dungeon specific stuff, only applies to players killing creatures
        if(cVictim->GetInstanceId())
        {
            InstanceScript *pInstance = ((InstanceScript*)cVictim->GetInstanceScript());
            if (pInstance)
                pInstance->OnCreatureKill(cVictim);

            Map* instanceMap = cVictim->GetMap();
            // TODO: do instance binding anyway if the charmer/owner is offline
            if(instanceMap->IsDungeon() && ((attacker && attacker->GetCharmerOrOwnerPlayerOrPlayerItself()) || attacker == cVictim))
            {
                if (instanceMap->IsRaidOrHeroicDungeon())
                {
                    if(cVictim->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_INSTANCE_BIND)
                        instanceMap->ToInstanceMap()->PermBindAllPlayers();
                }
                else
                {
                    // the reset time is set but not added to the scheduler
                    // until the players leave the instance
                    time_t resettime = cVictim->GetRespawnTimeEx() + 2 * HOUR;
                    if(InstanceSave *save = sInstanceSaveMgr->GetInstanceSave(cVictim->GetInstanceId()))
                        if(save->GetResetTime() < resettime) 
                            save->SetResetTime(resettime);
                }
            }
        }
    }

    // outdoor pvp things, do these after setting the death state, else the player activity notify won't work... doh...
    // handle player kill only if not suicide (spirit of redemption for example)
    if(player && attacker != pVictim)
        if(OutdoorPvP * pvp = player->GetOutdoorPvP())
            pvp->HandleKill(player, pVictim);

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

                ClearUnitState(state);
                SetStunned(false);
                break;
            case UNIT_STATE_ROOT:
                if (HasAuraType(SPELL_AURA_MOD_ROOT))
                    return;

                ClearUnitState(state);
                SetRooted(false);
                break;
            case UNIT_STATE_CONFUSED:
                if (HasAuraType(SPELL_AURA_MOD_CONFUSE))
                    return;

                ClearUnitState(state);
                SetConfused(false);
                break;
            case UNIT_STATE_FLEEING:
                if (HasAuraType(SPELL_AURA_MOD_FEAR))
                    return;

                ClearUnitState(state);
                SetFeared(false);
                break;
            default:
                return;
        }

        ApplyControlStatesIfNeeded();
    }
}

void Unit::ApplyControlStatesIfNeeded()
{
    if (HasUnitState(UNIT_STATE_STUNNED) || HasAuraType(SPELL_AURA_MOD_STUN))
        SetStunned(true);
    if (HasUnitState(UNIT_STATE_ROOT) || HasAuraType(SPELL_AURA_MOD_ROOT))
        SetRooted(true);
    if (HasUnitState(UNIT_STATE_CONFUSED) || HasAuraType(SPELL_AURA_MOD_CONFUSE))
        SetConfused(true);
    if (HasUnitState(UNIT_STATE_FLEEING) || HasAuraType(SPELL_AURA_MOD_FEAR))
        SetFeared(true);
}

void Unit::SetStunned(bool apply)
{
    if (apply)
    {
        // We need to stop fear on stun and root or we will get teleport to destination issue as MVMGEN for fear keeps going on        
        if (HasUnitState(UNIT_STATE_FLEEING))
            SetFeared(false);

        SetTarget(ObjectGuid::Empty);
        SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);

        // MOVEMENTFLAG_ROOT cannot be used in conjunction with MOVEMENTFLAG_MASK_MOVING (tested 3.3.5a)
        // this will freeze clients. That's why we remove MOVEMENTFLAG_MASK_MOVING before
        // setting MOVEMENTFLAG_ROOT
        RemoveUnitMovementFlag(MOVEMENTFLAG_MASK_MOVING);
        AddUnitMovementFlag(MOVEMENTFLAG_ROOT);
        StopMoving();

        if (GetTypeId() == TYPEID_PLAYER)
            SetStandState(UNIT_STAND_STATE_STAND);

        WorldPacket data(SMSG_FORCE_MOVE_ROOT, 8 + 4);
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
        Unit* owner = GetCharmerOrOwner();
        if (!owner || owner->GetTypeId() != TYPEID_PLAYER || !owner->ToPlayer()->IsMounted())
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
            WorldPacket data(SMSG_FORCE_MOVE_ROOT, 8 + 4);
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
        SetTarget(ObjectGuid::Empty);

        Unit* caster = nullptr;
        Unit::AuraEffectList const& fearAuras = GetAuraEffectsByType(SPELL_AURA_MOD_FEAR);

        if (!fearAuras.empty())
            caster = ObjectAccessor::GetUnit(*this, fearAuras.front()->GetCasterGUID());
        if (!caster)
            caster = GetAttackerForHelper();

       GetMotionMaster()->MoveFleeing(caster, fearAuras.empty() ? sWorld->getConfig(CONFIG_CREATURE_FAMILY_FLEE_DELAY) : 0);             // caster == NULL processed in MoveFleeing
    }
    else
    {
        AttackStop();  //sun: This will reupdate current victim. patch 2.4.3 : When a stun wears off, the creature that was stunned will prefer the last target with the highest threat, versus the current target. I'm not sure this should apply to fear but this seems logical.

        if (IsAlive())
        {
            GetMotionMaster()->Remove(FLEEING_MOTION_TYPE);
            if (GetVictim())
                SetTarget(EnsureVictim()->GetGUID());
        }
    }

    // block / allow control to real player in control (eg charmer)
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (m_playerMovingMe)
            m_playerMovingMe->SetClientControl(this, !apply);
    }
}

void Unit::SetConfused(bool apply)
{
    if (apply)
    {
        SetTarget(ObjectGuid::Empty);
        GetMotionMaster()->MoveConfused();
    }
    else
    {
        AttackStop();  //This will reupdate current victim. patch 2.4.3 : When a stun wears off, the creature that was stunned will prefer the last target with the highest threat, versus the current target. I'm not sure this should apply to fear but this seems logical.

        if (IsAlive())
        {
            GetMotionMaster()->Remove(CONFUSED_MOTION_TYPE);
            if (GetVictim())
                SetTarget(EnsureVictim()->GetGUID());
        }
    }

    // block / allow control to real player in control (eg charmer)
    if (GetTypeId() == TYPEID_PLAYER)
    {
        if (m_playerMovingMe)
            m_playerMovingMe->SetClientControl(this, !apply);
    }
}

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

    TC_LOG_DEBUG("entities.unit", "SetCharmedBy: charmer %u (GUID %u), charmed %u (GUID %u), type %u.", charmer->GetEntry(), ObjectGuid(charmer->GetGUID()).GetCounter(), GetEntry(), ObjectGuid(GetGUID()).GetCounter(), uint32(type));

    if (this == charmer)
    {
        TC_LOG_FATAL("entities.unit", "Unit::SetCharmedBy: Unit %u (GUID %u) is trying to charm itself!", GetEntry(), ObjectGuid(GetGUID()).GetCounter());
        return false;
    }

    //if (HasUnitState(UNIT_STATE_UNATTACKABLE))
    //    return false;

    if (GetTypeId() == TYPEID_PLAYER && ToPlayer()->GetTransport())
    {
        TC_LOG_FATAL("entities.unit", "Unit::SetCharmedBy: Player on transport is trying to charm %u (GUID %u)", GetEntry(), ObjectGuid(GetGUID()).GetCounter());
        return false;
    }

    if (IsInFlight())
        return false;

    // Already charmed
    if (GetCharmerGUID())
    {
        TC_LOG_FATAL("entities.unit", "Unit::SetCharmedBy: %u (GUID %u) has already been charmed but %u (GUID %u) is trying to charm it!", GetEntry(), ObjectGuid(GetGUID()).GetCounter(), charmer->GetEntry(), ObjectGuid(charmer->GetGUID()).GetCounter());
        return false;
    }

    if (ToCreature())
        ToCreature()->SetWalk(false);
    else
        RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);

    CastStop();
    CombatStop(); /// @todo CombatStop(true) may cause crash (interrupt spells)
    SetEmoteState(0);
    HandleEmoteCommand(0);

    Player* playerCharmer = charmer->ToPlayer();

    // Charmer stop charming
    if (playerCharmer)
    {
        playerCharmer->StopCastingCharm(aurApp->GetBase()); //sun, exclude this aura, else spell such as 30019 (Control Piece) will delete itself
        playerCharmer->StopCastingBindSight(aurApp->GetBase());
    }

    // Charmed stop charming
    if (GetTypeId() == TYPEID_PLAYER)
    {
        ToPlayer()->StopCastingCharm(aurApp->GetBase()); //sun, exclude this aura, else spell such as 30019 (Control Piece) will delete itself
        ToPlayer()->StopCastingBindSight(aurApp->GetBase());
    }

    // StopCastingCharm may remove a possessed pet?
    if (!IsInWorld())
    {
        TC_LOG_FATAL("entities.unit", "Unit::SetCharmedBy: %u (GUID %u) is not in world but %u (GUID %u) is trying to charm it!", GetEntry(), ObjectGuid(GetGUID()).GetCounter(), charmer->GetEntry(), ObjectGuid(charmer->GetGUID()).GetCounter());
        return false;
    }

    // charm is set by aura, and aura effect remove handler was called during apply handler execution
    // prevent undefined behaviour
    if (aurApp && aurApp->GetRemoveMode())
        return false;

    //TC _oldFactionId = GetFaction();
    SetFaction(charmer->GetFaction());

    // Set charmed
    charmer->SetCharm(this, true);

    if (GetTypeId() == TYPEID_UNIT)
    {
        PauseMovement(0, 0, false);
        GetMotionMaster()->Clear(MOTION_PRIORITY_NORMAL);

        StopMoving();

        ToCreature()->AI()->OnCharmed(charmer, true);
    }
    else if (Player* player = ToPlayer())
    {
        if (player->IsAFK())
            player->ToggleAFK();

        if (charmer->GetTypeId() == TYPEID_UNIT) // we are charmed by a creature
        {
            // change AI to charmed AI on next Update tick
            NeedChangeAI = true;
            if (IsAIEnabled)
            {
                IsAIEnabled = false;
                player->AI()->OnCharmed(charmer, true);
            }
        }
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
#ifdef LICH_KING
            case CHARM_TYPE_VEHICLE:
                SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED);
                playerCharmer->SetClientControl(this, true);
                playerCharmer->VehicleSpellInitialize();
                break;
#endif
            case CHARM_TYPE_POSSESS:
                AddUnitState(UNIT_STATE_POSSESSED);
                SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED);
                charmer->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                playerCharmer->SetClientControl(this, true);
                playerCharmer->PossessSpellInitialize();
                AddUnitState(UNIT_STATE_POSSESSED);
                break;
            case CHARM_TYPE_CHARM:
                if (GetTypeId() == TYPEID_UNIT && charmer->GetClass() == CLASS_WARLOCK)
                {
                    CreatureTemplate const* cinfo = ToCreature()->GetCreatureTemplate();
                    if (cinfo && cinfo->type == CREATURE_TYPE_DEMON)
                    {
                        // to prevent client crash
                        SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS, (uint8)CLASS_MAGE);

                        // just to enable stat window
                        if (GetCharmInfo())
                            GetCharmInfo()->SetPetNumber(sObjectMgr->GeneratePetNumber(), true);

                        // if charmed two demons the same session, the 2nd gets the 1st one's name
                        SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(nullptr))); // cast can't be helped
                    }
                }
                playerCharmer->CharmSpellInitialize();
                break;
            default:
            case CHARM_TYPE_CONVERT:
                break;
        }
    }

    AddUnitState(UNIT_STATE_CHARMED);
    return true;
}

void Unit::RemoveCharmedBy(Unit* charmer)
{
    if(!IsCharmed())
        return;

    if(!charmer)
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
#ifdef LICH_KING
    else if (charmer && charmer->IsOnVehicle(this))
        type = CHARM_TYPE_VEHICLE;
#endif
    else
        type = CHARM_TYPE_CHARM;

    CastStop();
    AttackStop();
    CombatStop(); //TODO: CombatStop(true) may cause crash (interrupt spells)
    RestoreFaction();

    GetMotionMaster()->InitializeDefault();
    if(Creature* c = ToCreature())
        c->ResetCreatureEmote();
    
    HandleEmoteCommand(0);

    if (Creature* creature = ToCreature())
    {
        // Creature will restore its old AI on next update
        if (creature->AI())
            creature->AI()->OnCharmed(charmer, false);

#ifdef LICH_KING
        // Vehicle should not attack its passenger after he exists the seat
        if (type != CHARM_TYPE_VEHICLE)
            LastCharmerGUID = ASSERT_NOTNULL(charmer)->GetGUID();
#endif
    }

    // If charmer still exists
    if (!charmer)
        return;

    ASSERT(type != CHARM_TYPE_POSSESS || charmer->GetTypeId() == TYPEID_PLAYER);
#ifdef LICH_KING
    ASSERT(type != CHARM_TYPE_VEHICLE || (GetTypeId() == TYPEID_UNIT && IsVehicle()));
#endif

    charmer->SetCharm(this, false);
    Player* playerCharmer = charmer->ToPlayer();

    if (playerCharmer)
    {
        switch (type)
        {
#ifdef LICH_KING
        case CHARM_TYPE_VEHICLE:
            playerCharmer->SetClientControl(this, false);
            playerCharmer->SetClientControl(charmer, true);
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED);
            break;
#endif
        case CHARM_TYPE_POSSESS:
            ClearUnitState(UNIT_STATE_POSSESSED);
            playerCharmer->SetClientControl(this, false);
            playerCharmer->SetClientControl(charmer, true);
            charmer->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_POSSESSED);
            break;
        case CHARM_TYPE_CHARM:
            if (GetTypeId() == TYPEID_UNIT && charmer->GetClass() == CLASS_WARLOCK)
            {
                CreatureTemplate const* cinfo = ToCreature()->GetCreatureTemplate();
                if (cinfo && cinfo->type == CREATURE_TYPE_DEMON)
                {
                    SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS, uint8(cinfo->unit_class));
                    if (GetCharmInfo())
                        GetCharmInfo()->SetPetNumber(0, true);
                    else
                        TC_LOG_ERROR("entities.unit", "Unit::RemoveCharmedBy: " UI64FMTD " has a charm aura but no charm info!", GetGUID().GetRawValue());
                }
            }
            break;
        case CHARM_TYPE_CONVERT:
            break;
        }
    }

    if(GetTypeId() == TYPEID_UNIT)
    {
        if(!(this->ToCreature())->IsPet())
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

        if(IsAlive() && (this->ToCreature())->IsAIEnabled)
        {
            if(!IsFriendlyTo(charmer))
                EngageWithTarget(charmer);
            else
                (this->ToCreature())->AI()->EnterEvadeMode();
        }
    }

    if (Player* player = ToPlayer())
    {
        if (charmer->GetTypeId() == TYPEID_UNIT) // charmed by a creature, this means we had PlayerAI
        {
            NeedChangeAI = true;
            IsAIEnabled = false;
        }

        player->SetClientControl(this, true);
    }

    // a guardian should always have charminfo
    if(playerCharmer && this != charmer->GetFirstControlled())
        playerCharmer->SendRemoveControlBar();
    else if (GetTypeId() == TYPEID_PLAYER || (GetTypeId() == TYPEID_UNIT && !IsGuardian()))
        DeleteCharmInfo();

    // reset confused movement for example
    ApplyControlStatesIfNeeded();
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
    
    //consider creature in same party when they have the same faction
    if(u1->GetTypeId() == TYPEID_UNIT && u2->GetTypeId() == TYPEID_UNIT)
        return u1->GetFaction() == u2->GetFaction();

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

    //consider creature in same raid when they have the same faction
    if (u1->GetTypeId() == TYPEID_UNIT && u2->GetTypeId() == TYPEID_UNIT)
        return u1->GetFaction() == u2->GetFaction();

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

    for(GroupReference *itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
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
    Group *pGroup = nullptr;
    if (owner->GetTypeId() == TYPEID_PLAYER)
        pGroup = (owner->ToPlayer())->GetGroup();

    if(pGroup)
    {
        uint8 subgroup = (owner->ToPlayer())->GetSubGroup();

        for(GroupReference *itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* Target = itr->GetSource();

            // IsHostileTo check duel and controlled by enemy
            if( Target && Target->GetSubGroup()==subgroup && !IsHostileTo(Target) )
            {
                if(Target->IsAlive() && IsWithinDistInMap(Target, radius) )
                    TagUnitMap.push_back(Target);

                if (Guardian* pet = Target->GetGuardianPet())
                    if(pet->IsAlive() && IsWithinDistInMap(pet, radius) )
                        TagUnitMap.push_back(pet);
            }
        }
    }
    else
    {
        if(owner->IsAlive() && (owner == this || IsWithinDistInMap(owner, radius)))
            TagUnitMap.push_back(owner);
        if (Guardian* pet = owner->GetGuardianPet())
            if(pet->IsAlive() && (pet == this && IsWithinDistInMap(pet, radius)))
                TagUnitMap.push_back(pet);
    }
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

void Unit::RestoreDisplayId()
{
    AuraEffect* handledAura = nullptr;
    // try to receive model from transform auras
    Unit::AuraEffectList const& transforms = GetAuraEffectsByType(SPELL_AURA_TRANSFORM);

    if (!transforms.empty())
    {
        // iterate over already applied transform auras - from newest to oldest
        for (auto i = transforms.rbegin(); i != transforms.rend(); ++i)
        {
            if (AuraApplication const* aurApp = (*i)->GetBase()->GetApplicationOfTarget(GetGUID()))
            {
                if (!handledAura)
                    handledAura = (*i);
                // prefer negative auras
                if (!aurApp->IsPositive())
                {
                    handledAura = (*i);
                    break;
                }
            }
        }
    }
    
    // transform aura was found
    if (handledAura)
    {
        //unapply (this is still active so can't be reapplied without unapplying first) then re apply
        handledAura->HandleEffect(this, AURA_EFFECT_HANDLE_SEND_FOR_CLIENT, true);
        return;
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
    time_t t = time(nullptr);
    tm* timePtr = localtime(&t);
    bool firstApril = timePtr->tm_mon == 3 && timePtr->tm_mday == 1;

    uint32 modelid = 0;
    switch(form)
    {
        case FORM_CAT:
            if(Player::TeamForRace(GetRace()) == ALLIANCE)
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
            if(Player::TeamForRace(GetRace()) == ALLIANCE)
                modelid = 10045;
            break;
        case FORM_BEAR:
        case FORM_DIREBEAR:
            if(Player::TeamForRace(GetRace()) == ALLIANCE)
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
            if(Player::TeamForRace(GetRace()) == ALLIANCE)
                modelid = firstApril ? 9345 : 20857;
            else
                modelid = firstApril ? 9345 : 20872;
            break;
        case FORM_MOONKIN:
            if(Player::TeamForRace(GetRace()) == ALLIANCE)
                modelid = firstApril ? 17034 : 15374;
            else
                modelid = firstApril ? 17034 : 15375;
            break;
        case FORM_FLIGHT_EPIC:
            if(Player::TeamForRace(GetRace()) == ALLIANCE)
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

uint32 Unit::GetModelForTotem(PlayerTotemType totemType)
{
    switch (GetRace())
    {
#ifdef LICH_KING
    case RACE_ORC:
    {
        switch (totemType)
        {
        case SUMMON_TYPE_TOTEM_FIRE:    // fire
            return 30758;
        case SUMMON_TYPE_TOTEM_EARTH:   // earth
            return 30757;
        case SUMMON_TYPE_TOTEM_WATER:   // water
            return 30759;
        case SUMMON_TYPE_TOTEM_AIR:     // air
            return 30756;
        }
        break;
    }
    case RACE_DWARF:
    {
        switch (totemType)
        {
        case SUMMON_TYPE_TOTEM_FIRE:    // fire
            return 30754;
        case SUMMON_TYPE_TOTEM_EARTH:   // earth
            return 30753;
        case SUMMON_TYPE_TOTEM_WATER:   // water
            return 30755;
        case SUMMON_TYPE_TOTEM_AIR:     // air
            return 30736;
        }
        break;
    }
    case RACE_TROLL:
    {
        switch (totemType)
        {
        case SUMMON_TYPE_TOTEM_FIRE:    // fire
            return 30762;
        case SUMMON_TYPE_TOTEM_EARTH:   // earth
            return 30761;
        case SUMMON_TYPE_TOTEM_WATER:   // water
            return 30763;
        case SUMMON_TYPE_TOTEM_AIR:     // air
            return 30760;
        }
        break;
    }
#endif
#ifndef LICH_KING
    //BC does not have model per race
    case RACE_TROLL:
    case RACE_ORC:
#endif
    case RACE_TAUREN:
    default:
    {
        switch (totemType)
        {
        case SUMMON_TYPE_TOTEM_FIRE:    // fire
            return 4589;
        case SUMMON_TYPE_TOTEM_EARTH:   // earth
            return 4588;
        case SUMMON_TYPE_TOTEM_WATER:   // water
            return 4587;
        case SUMMON_TYPE_TOTEM_AIR:     // air
            return 4590;
        }
        break;
    }
    case RACE_DRAENEI:
    {
        switch (totemType)
        {
        case SUMMON_TYPE_TOTEM_FIRE:    // fire
            return 19074;
        case SUMMON_TYPE_TOTEM_EARTH:   // earth
            return 19073;
        case SUMMON_TYPE_TOTEM_WATER:   // water
            return 19075;
        case SUMMON_TYPE_TOTEM_AIR:     // air
            return 19071;
        }
        break;
    }
    }
    return 0;
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

    if (Player* p = ToPlayer())
        p->GetSession()->anticheat->OnPlayerWaterWalk(p);
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

    if (Player* p = ToPlayer())
        p->GetSession()->anticheat->OnPlayerSlowfall(p);

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
        //! Dying creatures will MoveFall from setDeathState
        if (hoverHeight && (!IsDying() || GetTypeId() != TYPEID_UNIT))
        {
            float newZ = GetPositionZ() - hoverHeight;
            UpdateAllowedPositionZ(GetPositionX(), GetPositionY(), newZ);
            UpdateHeight(newZ);
        }
    }

    if (Player* p = ToPlayer())
        p->GetSession()->anticheat->OnPlayerSlowfall(p);

    return true;
}

void Unit::SetInFront(WorldObject const* target)
{
    if (!HasUnitState(UNIT_STATE_CANNOT_TURN))
        SetOrientation(GetAbsoluteAngle(target));
}

void Unit::SetInFront(float x, float y)
{
    if(!HasUnitState(UNIT_STATE_CANNOT_TURN) && !IsUnitRotating()) 
        SetOrientation(GetAbsoluteAngle(x,y));
}

bool Unit::HandleSpellClick(Unit* clicker, int8 seatId)
{
#ifdef LICH_KING
    - "TODO LK vehicules";
#endif

    bool result = true;

    Creature* creature = ToCreature();
    if (creature && creature->IsAIEnabled)
        creature->AI()->OnSpellClick(clicker, result);

    return result;
}

TransportBase* Unit::GetDirectTransport() const
{
#ifdef LICH_KING
    if (Vehicle* veh = GetVehicle())
        return veh;
#endif
    return GetTransport();
}

void Unit::BuildMovementPacket(ByteBuffer *data) const
{
    Unit::BuildMovementPacket(Position(GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation()), m_movementInfo.transport.pos, m_movementInfo, data);
}

void Unit::BuildMovementPacket(Position const& pos, Position const& transportPos, MovementInfo const& movementInfo, ByteBuffer* data)
{
    *data << uint32(movementInfo.GetMovementFlags());            // movement flags
#ifdef LICH_KING
    *data << uint16(movementInfo.GetExtraMovementFlags());
#else
    *data << uint8(0);                                  // 2.3.0, always set to 0
#endif
    *data << uint32(GetMSTime());                       // time / counter
    *data << float(pos.GetPositionX());
    *data << float(pos.GetPositionY());
    *data << float(pos.GetPositionZ());
    *data << float(pos.GetOrientation());

    // 0x00000200
    if (movementInfo.HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
    {
#ifdef LICH_KING
        *data << movementInfo.transport.guid.WriteAsPacked();
#else
        *data << uint64(movementInfo.transport.guid);
#endif
        *data << float(transportPos.GetPositionX());
        *data << float(transportPos.GetPositionY());
        *data << float(transportPos.GetPositionZ());
        *data << float(transportPos.GetOrientation());
        *data << uint32(movementInfo.transport.time);
#ifdef LICH_KING
        *data << int8(movementInfo.transport.seat);
#endif
    }

    // 0x02200000
    if (movementInfo.HasMovementFlag(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_PLAYER_FLYING))
        *data << float(movementInfo.pitch);

    *data << uint32(movementInfo.fallTime);

    // 0x00001000
    if (movementInfo.HasMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING))
    {
        *data << float(movementInfo.jump.zspeed);
        *data << float(movementInfo.jump.sinAngle);
        *data << float(movementInfo.jump.cosAngle);
        *data << float(movementInfo.jump.xyspeed);
    }

    // 0x04000000
    if (movementInfo.HasMovementFlag(MOVEMENTFLAG_SPLINE_ELEVATION))
        *data << float(movementInfo.splineElevation);
}

void Unit::KnockbackFrom(float x, float y, float speedXY, float speedZ)
{
    Player* player = ToPlayer();
    if (!player)
    {
        if (Unit* charmer = GetCharmer())
        {
            player = charmer->ToPlayer();
            if (player && player->m_unitMovedByMe != this)
                player = nullptr;
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

bool Unit::CanSwim() const
{
    // Mirror client behavior, if this method returns false then client will not use swimming animation and for players will apply gravity as if there was no water
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CANNOT_SWIM))
        return false;
    if (HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED)) // is player
        return true;
    if (HasFlag(UNIT_FIELD_FLAGS_2, 0x1000000))
        return false;
    if (IsPet() && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT))
        return true;
    return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_RENAME | UNIT_FLAG_UNK_15);
}

bool Unit::IsFalling() const
{
    return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING | MOVEMENTFLAG_FALLING_FAR) || movespline->isFalling();
}

void Unit::NearTeleportTo(Position const& pos, bool casting /*= false*/)
{
    DisableSpline();
    if (GetTypeId() == TYPEID_PLAYER)
    {
        WorldLocation target(GetMapId(), pos);
        ToPlayer()->TeleportTo(target, TELE_TO_NOT_LEAVE_TRANSPORT | TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET | (casting ? TELE_TO_SPELL : 0));
    }
    else
    {
        SendTeleportPacket(pos);
        UpdatePosition(pos, true);
        UpdateObjectVisibility();
    }
}

void Unit::SendTeleportPacket(Position const& pos, bool teleportingTransport /*= false*/)
{
    // MSG_MOVE_TELEPORT is sent to nearby players to signal the teleport
    // MSG_MOVE_TELEPORT_ACK is sent to self in order to trigger ACK and update the position server side

    MovementInfo teleportMovementInfo = m_movementInfo;
    teleportMovementInfo.pos.Relocate(pos);
    Position transportPos = m_movementInfo.transport.pos;
    if (TransportBase* transportBase = GetDirectTransport())
    {
        // if its the transport that is teleported then we have old transport position here and cannot use it to calculate offsets
        // assume that both transport teleport and teleport within transport cannot happen at the same time
        if (!teleportingTransport)
        {
            float x, y, z, o;
            pos.GetPosition(x, y, z, o);
            transportBase->CalculatePassengerOffset(x, y, z, &o);
            transportPos.Relocate(x, y, z, o);
        }
    }

    WorldPacket moveUpdateTeleport(MSG_MOVE_TELEPORT, 38);
    moveUpdateTeleport << GetPackGUID();
    Unit* broadcastSource = this;

    if (Player* playerMover = GetPlayerBeingMoved())
    {
        WorldPacket moveTeleport(MSG_MOVE_TELEPORT_ACK, 41);
        moveTeleport << GetPackGUID();
        moveTeleport << uint32(0);                                     // this value increments every time
        Unit::BuildMovementPacket(pos, transportPos, teleportMovementInfo, &moveTeleport);
        playerMover->SendDirectMessage(&moveTeleport);

        broadcastSource = playerMover;
    }

    Unit::BuildMovementPacket(pos, transportPos, teleportMovementInfo, &moveUpdateTeleport);

    // Broadcast the packet to everyone except self.
    broadcastSource->SendMessageToSet(&moveUpdateTeleport, false);
}

bool Unit::UpdatePosition(float x, float y, float z, float orientation, bool teleport)
{
    // prevent crash when a bad coord is sent by the client
    if (!Trinity::IsValidMapCoord(x, y, z, orientation))
    {
        TC_LOG_DEBUG("entities.unit", "Unit::UpdatePosition(%f, %f, %f) .. bad coordinates!", x, y, z);
        return false;
    }

    bool const turn = (GetOrientation() != orientation);

    // G3D::fuzzyEq won't help here, in some cases magnitudes differ by a little more than G3D::eps, but should be considered equal
    bool const relocated = (teleport ||
        std::fabs(GetPositionX() - x) > 0.001f ||
        std::fabs(GetPositionY() - y) > 0.001f ||
        std::fabs(GetPositionZ() - z) > 0.001f);

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

    UpdatePositionData();

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
#ifdef LICH_KING
    if (IsVehicle())
        GetVehicleKit()->RelocatePassengers();
#endif
}

//! Only server-side height update, does not broadcast to client
void Unit::UpdateHeight(float newZ)
{
    Relocate(GetPositionX(), GetPositionY(), newZ);
#ifdef LICH_KING
    if (IsVehicle())
        GetVehicleKit()->RelocatePassengers();
#endif
}

class SplineHandler
{
public:
    SplineHandler(Unit* unit) : _unit(unit) { }

    bool operator()(Movement::MoveSpline::UpdateResult result)
    {
        MovementGeneratorType motionType = _unit->GetMotionMaster()->GetCurrentMovementGeneratorType();
        if ((result & (Movement::MoveSpline::Result_NextSegment | Movement::MoveSpline::Result_JustArrived | Movement::MoveSpline::Result_Arrived))
            && _unit->GetTypeId() == TYPEID_UNIT 
            && (motionType == WAYPOINT_MOTION_TYPE)
            && _unit->movespline->GetId() == _unit->GetMotionMaster()->GetCurrentSplineId())
        {
            Creature* creature = _unit->ToCreature();
            if (creature)
            {
                _unit->GetMotionMaster()->AddFlag(MOTIONMASTER_FLAG_UPDATE);
                MovementGenerator* baseGenerator = creature->GetMotionMaster()->GetCurrentMovementGenerator();
                WaypointMovementGenerator<Creature>* moveGenerator = static_cast<WaypointMovementGenerator<Creature>*>(baseGenerator);
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
                _unit->GetMotionMaster()->RemoveFlag(MOTIONMASTER_FLAG_UPDATE);
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

    // sunwell: update always! not every 400ms, because movement generators need the actual position
    /*m_movesplineTimer.Update(t_diff);
    if (m_movesplineTimer.Passed() || arrived) */
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

    // sunwell: this is bullcrap, if we had spline running update orientation along with position
    //if (HasUnitState(UNIT_STATE_CANNOT_TURN))
    //    loc.orientation = GetOrientation();

    UpdatePosition(loc.x, loc.y, loc.z, loc.orientation);
}

void Unit::DisableSpline()
{
    m_movementInfo.RemoveMovementFlag(MovementFlags(MOVEMENTFLAG_SPLINE_ENABLED|MOVEMENTFLAG_FORWARD));
    movespline->_Interrupt();
}

bool Unit::IsPossessedByPlayer() const
{
    return HasUnitState(UNIT_STATE_POSSESSED) && GetCharmerGUID().IsPlayer();
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

Unit* Unit::GetNextRandomRaidMemberOrPet(float radius)
{
    Player* player = nullptr;
    if (GetTypeId() == TYPEID_PLAYER)
        player = ToPlayer();
    // Should we enable this also for charmed units?
    else if (GetTypeId() == TYPEID_UNIT && IsPet())
        player = GetOwner()->ToPlayer();

    if (!player)
        return nullptr;
    Group* group = player->GetGroup();
    // When there is no group check pet presence
    if (!group)
    {
        // We are pet now, return owner
        if (player != this)
            return IsWithinDistInMap(player, radius) ? player : nullptr;
        Unit* pet = GetGuardianPet();
        // No pet, no group, nothing to return
        if (!pet)
            return nullptr;
        // We are owner now, return pet
        return IsWithinDistInMap(pet, radius) ? pet : nullptr;
    }

    std::vector<Unit*> nearMembers;
    // reserve place for players and pets because resizing vector every unit push is unefficient (vector is reallocated then)
    nearMembers.reserve(group->GetMembersCount() * 2);

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        if (Player* Target = itr->GetSource())
        {
            // IsHostileTo check duel and controlled by enemy
            if (Target != this && IsWithinDistInMap(Target, radius) && Target->IsAlive() && !IsHostileTo(Target))
                nearMembers.push_back(Target);

            // Push player's pet to vector
            if (Unit* pet = Target->GetGuardianPet())
                if (pet != this && IsWithinDistInMap(pet, radius) && pet->IsAlive() && !IsHostileTo(pet))
                    nearMembers.push_back(pet);
        }

    if (nearMembers.empty())
        return nullptr;

    uint32 randTarget = urand(0, nearMembers.size() - 1);
    return nearMembers[randTarget];
}

bool Unit::CanFreeMove() const
{
    return !HasUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING | UNIT_STATE_IN_FLIGHT |
        UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED) && GetOwnerGUID() == 0;
}

void Unit::SetFacingTo(float ori, bool force)
{
    // do not face when already moving
    if (!force && (!IsStopped() || !movespline->Finalized()))
        return;

    Movement::MoveSplineInit init(this);
    init.MoveTo(GetPositionX(), GetPositionY(), GetPositionZ(), false);
    if (HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) && GetTransGUID())
        init.DisableTransportPathTransformations(); // It makes no sense to target global orientation
    init.SetFacing(ori);

    //GetMotionMaster()->LaunchMoveSpline(std::move(init), EVENT_FACE, MOTION_PRIORITY_HIGHEST);
    init.Launch();
}

void Unit::SetFacingToObject(WorldObject const* object, bool force)
{
    // do not face when already moving
    if (!force && (!IsStopped() || !movespline->Finalized()))
        return;

    /// @todo figure out under what conditions creature will move towards object instead of facing it where it currently is.
    Movement::MoveSplineInit init(this);
    init.MoveTo(GetPositionX(), GetPositionY(), GetPositionZ(), false);
    init.SetFacing(GetAbsoluteAngle(object));   // when on transport, GetAbsoluteAngle will still return global coordinates (and angle) that needs transforming
    //GetMotionMaster()->LaunchMoveSpline(std::move(init), EVENT_FACE, MOTION_PRIORITY_HIGHEST);
    init.Launch();
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
    AuraEffect const* aura = pInfo->auraEff;

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
            for (GroupReference* gr = group->GetFirstMember(); gr != nullptr; gr = gr->next())
            {
                if (Player* groupGuy = gr->GetSource())
                    guildOccurs[groupGuy->GetGuildId()]++;
            }
        }

        if (groupSize) {
            for (auto & guildOccur : guildOccurs) 
            {
                guildPercentage = ((float)guildOccur.second / groupSize) * 100;
                if (guildPercentage >= 67.0f) {
                    downByGuildId = guildOccur.first;
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
    Cell::VisitWorldObjects(this, worker, textRange);
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
        for(const auto & player : players) {
            if (Player* plr = player.GetSource())
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
    Cell::VisitWorldObjects(this, worker, textRange);
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

void Unit::old_Talk(uint32 textId, ChatMsg msgType, float textRange, ObjectGuid targetGUID, uint32 language)
{
    Unit* target = nullptr;
    if(targetGUID)
    {
        target = ObjectAccessor::GetUnit(*this, targetGUID);
        if(!target)
            TC_LOG_ERROR("entities.unit", "WorldObject::old_Talk: unit with guid " UI64FMTD " was not found. Defaulting to no target.", targetGUID.GetRawValue());
    }

    Trinity::OldScriptTextBuilder builder(this, msgType, textId, Language(language), target);
    Trinity::LocalizedPacketDo<Trinity::OldScriptTextBuilder> localizer(builder);
    Trinity::PlayerDistWorker<Trinity::LocalizedPacketDo<Trinity::OldScriptTextBuilder> > worker(this, textRange, localizer);
    Cell::VisitWorldObjects(this, worker, textRange);
}

void Unit::old_Say(int32 textId, uint32 language, ObjectGuid TargetGuid)
{
    old_Talk(textId, CHAT_MSG_MONSTER_SAY, sWorld->getConfig(CONFIG_LISTEN_RANGE_SAY), TargetGuid, language);
}

void Unit::old_Yell(int32 textId, uint32 language, ObjectGuid TargetGuid)
{
    old_Talk(textId, CHAT_MSG_MONSTER_YELL, sWorld->getConfig(CONFIG_LISTEN_RANGE_YELL), TargetGuid, language);
}

void Unit::old_TextEmote(int32 textId, ObjectGuid TargetGuid, bool IsBossEmote)
{
    old_Talk(textId, IsBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, sWorld->getConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), TargetGuid, LANG_UNIVERSAL);
}

void Unit::old_Whisper(int32 textId, ObjectGuid receiverGUID, bool IsBossWhisper)
{
    if (!receiverGUID)
        return;

    Player* target = ObjectAccessor::FindPlayer(receiverGUID);
    if(!target)
    {
        TC_LOG_ERROR("entities.unit", "WorldObject::old_Whisper: player with guid " UI64FMTD " was not found", receiverGUID.GetRawValue());
        return;
    }

    LocaleConstant locale = target->GetSession()->GetSessionDbLocaleIndex();

    char const* text = sObjectMgr->GetTrinityString(textId,locale);

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, IsBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_WHISPER, LANG_UNIVERSAL, this, target, text, 0, "", locale);
    target->SendDirectMessage(&data);
}

// Returns collisionheight of the unit. If it is 0, it returns DEFAULT_COLLISION_HEIGHT.
float Unit::GetCollisionHeight() const
{
    float scaleMod = GetObjectScale(); // 99% sure about this

    if (IsMounted())
    {
        if (CreatureDisplayInfoEntry const* mountDisplayInfo = sCreatureDisplayInfoStore.LookupEntry(GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID)))
        {
            if (CreatureModelDataEntry const* mountModelData = sCreatureModelDataStore.LookupEntry(mountDisplayInfo->ModelId))
            {
                CreatureDisplayInfoEntry const* displayInfo = sCreatureDisplayInfoStore.AssertEntry(GetNativeDisplayId());
                CreatureModelDataEntry const* modelData = sCreatureModelDataStore.AssertEntry(displayInfo->ModelId);
                float const collisionHeight = scaleMod * (mountModelData->MountHeight + modelData->CollisionHeight * modelData->Scale * displayInfo->scale * 0.5f);
                return collisionHeight == 0.0f ? DEFAULT_COLLISION_HEIGHT : collisionHeight;
            }
        }
    }

    //! Dismounting case - use basic default model data
    CreatureDisplayInfoEntry const* displayInfo = sCreatureDisplayInfoStore.AssertEntry(GetNativeDisplayId());
    CreatureModelDataEntry const* modelData = sCreatureModelDataStore.AssertEntry(displayInfo->ModelId);

    float const collisionHeight = scaleMod * modelData->CollisionHeight * modelData->Scale * displayInfo->scale;
    return collisionHeight == 0.0f ? DEFAULT_COLLISION_HEIGHT : collisionHeight;
}

//might slightly move targetPos
bool _IsLeapAccessibleByPath(Unit* unit, Position& targetPos)
{
    float distToTarget = unit->GetExactDistance(targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ());

    PathGenerator path(unit);
    path.ExcludeSteepSlopes();
    //here we'd want to allow just a bit longer than a straight line, but SetPathLengthLimit is really imprecise so we just set limit to distToTarget and it'll allow some minor detours
    path.SetPathLengthLimit(distToTarget);
    if (path.CalculatePath(targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ(), false, false)
        && ((path.GetPathType() & (PATHFIND_SHORT | PATHFIND_NOPATH | PATHFIND_INCOMPLETE | PATHFIND_NOT_USING_PATH)) == 0)
        ) 
    {
        //additional internal validity hack to compensate for mmap imprecision. Check if path last two points and two first points are in Los
        Movement::PointsArray points = path.GetPath();
        if (points.size() >= 2) //if shorter than that the LoS check from before should be valid anyway, noneed for else
        {
            G3D::Vector3 firstPoint = points[0];
            Position firstPointPos(firstPoint.x, firstPoint.y, firstPoint.z + 1.5f);

            G3D::Vector3 lastPoint = points[points.size() - 1];
            G3D::Vector3 beforeLastPoint = points[points.size() - 2];
            Position beforeLastPointPos(beforeLastPoint.x, beforeLastPoint.y, beforeLastPoint.z + 1.5f);
            if (!unit->GetCollisionPosition(beforeLastPointPos, lastPoint.x, lastPoint.y, lastPoint.z + 1.5f, targetPos)
                && !unit->GetCollisionPosition(unit->GetPosition(), firstPoint.x, firstPoint.y, firstPoint.z + 1.5f, targetPos)
                )
            {
                targetPos.Relocate(lastPoint.x, lastPoint.y, lastPoint.z);
                //TC_LOG_TRACE("vmap", "WorldObject::GetLeapPosition Found valid target point, %f %f %f was accessible by path.", targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ());
                return true;
            }
        }
    }
    return false;
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
        float maxSearchDist = (dist - i * stepLength) / 1.8f + GetCombatReach()*2; //allow smaller z diff at close range, greater z diff at long range (linear reduction)
        //TC_LOG_TRACE("vmap", "WorldObject::GetLeapPosition Searching for valid target, step %i. maxSearchDist = %f.", i, maxSearchDist);

        //start with higher check then lower
        for (int8 j = 1; j >= 0; j--)
        {
            //search at given z then at z + maxSearchDist
            float mapHeight = GetMap()->GetHeight(PHASEMASK_NORMAL, destx, desty, destz + j * (maxSearchDist / 2.0f), true, maxSearchDist / 2.0f, GetCollisionHeight(), true);
            if (mapHeight != INVALID_HEIGHT)
            {
                //if no collision
                if (!GetCollisionPosition(currentPos, destx, desty, mapHeight, targetPos, GetCombatReach()) && GetMap()->IsPlayerWalkable(Position(destx, desty, mapHeight)))
                {
                    //TC_LOG_TRACE("vmap", "WorldObject::GetLeapPositionFound valid target point, %f %f %f was in LoS", targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ());
                    //GetCollisionPosition already set targetPos to destx, desty, destz
                    goto exitloopfounddest;
                }

                //else if is accessible by path
                targetPos.Relocate(destx, desty, mapHeight);
                float distToTarget = GetExactDistance(targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ());
                if (distToTarget < 4.0f)
                    continue; //dont even check for very short distance

                if (_IsLeapAccessibleByPath(this, targetPos)) 
                {
                    //TC_LOG_TRACE("vmap", "WorldObject::GetLeapPosition Found valid target point, %f %f %f was accessible by path.", targetPos.GetPositionX(), targetPos.GetPositionY(), targetPos.GetPositionZ());
                    goto exitloopfounddest;
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
        float mapHeight = GetMap()->GetHeight(PHASEMASK_NORMAL, currentPos.GetPositionX(), currentPos.GetPositionY(), currentPos.GetPositionZ(), true, 15.0f, GetCollisionHeight(), true);
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

bool Unit::HasBreakableByDamageAuraType(AuraType type, uint32 excludeAura) const
{
    AuraEffectList const& auras = GetAuraEffectsByType(type);
    for (auto aura : auras)
        if ((!excludeAura || excludeAura != aura->GetSpellInfo()->Id) && //Avoid self interrupt of channeled Crowd Control spells like Seduction
            (aura->GetSpellInfo()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_TAKE_DAMAGE)) 
            return true;
    return false;
}

bool Unit::HasBreakableByDamageCrowdControlAura(Unit* excludeCasterChannel) const
{
    uint32 excludeAura = 0;
    if (Spell* currentChanneledSpell = excludeCasterChannel ? excludeCasterChannel->GetCurrentSpell(CURRENT_CHANNELED_SPELL) : nullptr)
        excludeAura = currentChanneledSpell->GetSpellInfo()->Id; //Avoid self interrupt of channeled Crowd Control spells like Seduction

    return (HasBreakableByDamageAuraType(SPELL_AURA_MOD_CONFUSE, excludeAura)
        || HasBreakableByDamageAuraType(SPELL_AURA_MOD_FEAR, excludeAura)
        || HasBreakableByDamageAuraType(SPELL_AURA_MOD_STUN, excludeAura)
        || HasBreakableByDamageAuraType(SPELL_AURA_MOD_ROOT, excludeAura)
        || HasBreakableByDamageAuraType(SPELL_AURA_TRANSFORM, excludeAura));
}

void Unit::SetShapeshiftForm(ShapeshiftForm form)
{
    SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHAPESHIFT_FORM, form);
}

void Unit::SetCombatDistance(float dist)
{
    bool changed = m_CombatDistance != dist;
    m_CombatDistance = dist;
    //create new targeted movement gen
    if (i_AI && changed && GetVictim())
    {
        AttackStop();
        i_AI->AttackStart(GetVictim());
    }
};

UnitActionBarEntry::UnitActionBarEntry() : packedData(uint32(ACT_DISABLED) << 24) { }

ActiveStates UnitActionBarEntry::GetType() const { return ActiveStates(UNIT_ACTION_BUTTON_TYPE(packedData)); }
uint32 UnitActionBarEntry::GetAction() const { return UNIT_ACTION_BUTTON_ACTION(packedData); }
bool UnitActionBarEntry::IsActionBarForSpell() const
{
    ActiveStates Type = GetType();
    return Type == ACT_DISABLED || Type == ACT_ENABLED || Type == ACT_PASSIVE;
}

void UnitActionBarEntry::SetActionAndType(uint32 action, ActiveStates type)
{
    packedData = MAKE_UNIT_ACTION_BUTTON(action, type);
}

void UnitActionBarEntry::SetType(ActiveStates type)
{
    packedData = MAKE_UNIT_ACTION_BUTTON(UNIT_ACTION_BUTTON_ACTION(packedData), type);
}

void UnitActionBarEntry::SetAction(uint32 action)
{
    packedData = (packedData & 0xFF000000) | UNIT_ACTION_BUTTON_ACTION(action);
}

HealInfo::HealInfo(Unit* healer, Unit* target, uint32 heal, SpellInfo const* spellInfo, SpellSchoolMask schoolMask)
    : _healer(healer), _target(target), _heal(heal), _effectiveHeal(0), _absorb(0), _spellInfo(spellInfo), _schoolMask(schoolMask), _hitMask(0)
{
}

void HealInfo::AbsorbHeal(uint32 amount)
{
#ifdef LICH_KING
    amount = std::min(amount, GetHeal());
    _absorb += amount;
    _heal -= amount;
    amount = std::min(amount, GetEffectiveHeal());
    _effectiveHeal -= amount;
    _hitMask |= PROC_HIT_ABSORB;
#endif
}

uint32 HealInfo::GetHitMask() const
{
    return _hitMask;
}

ProcEventInfo::ProcEventInfo(Unit* actor, Unit* actionTarget, Unit* procTarget,
    uint32 typeMask, uint32 spellTypeMask,
    uint32 spellPhaseMask, uint32 hitMask,
    Spell* spell, DamageInfo* damageInfo,
    HealInfo* healInfo) :
    _actor(actor), _actionTarget(actionTarget), _procTarget(procTarget),
    _typeMask(typeMask), _spellTypeMask(spellTypeMask),
    _spellPhaseMask(spellPhaseMask), _hitMask(hitMask), _spell(spell),
    _damageInfo(damageInfo), _healInfo(healInfo)
{ }

SpellInfo const* ProcEventInfo::GetSpellInfo() const
{
    if (_spell)
        return _spell->GetSpellInfo();
    if (_damageInfo)
        return _damageInfo->GetSpellInfo();
    if (_healInfo)
        return _healInfo->GetSpellInfo();
    return nullptr;
}

SpellSchoolMask ProcEventInfo::GetSchoolMask() const
{
    if (_spell)
        return _spell->GetSpellInfo()->GetSchoolMask();
    if (_damageInfo)
        return _damageInfo->GetSchoolMask();
    if (_healInfo)
        return _healInfo->GetSchoolMask();
    return SPELL_SCHOOL_MASK_NONE;
}

Aura* Unit::_TryStackingOrRefreshingExistingAura(AuraCreateInfo& createInfo)
{
    ASSERT(createInfo.CasterGUID || createInfo.Caster);

    // Check if these can stack anyway
    if (!createInfo.CasterGUID && !createInfo.GetSpellInfo()->IsStackableOnOneSlotWithDifferentCasters())
        createInfo.CasterGUID = createInfo.Caster->GetGUID();

    // world gameobjects can't own auras and they send empty casterguid
    // checked on sniffs with spell 22247
    if (createInfo.CasterGUID.IsGameObject())
    {
        createInfo.Caster = nullptr;
        createInfo.CasterGUID.Clear();
    }

    // passive and Incanter's Absorption and auras with different type can stack with themselves any number of times
    if (!createInfo.GetSpellInfo()->IsMultiSlotAura())
    {
        // check if cast item changed
        ObjectGuid castItemGUID;
        if (createInfo.CastItem)
            castItemGUID = createInfo.CastItem->GetGUID();

        // find current aura from spell and change it's stackamount, or refresh it's duration
        if (Aura* foundAura = GetOwnedAura(createInfo.GetSpellInfo()->Id, createInfo.CasterGUID, createInfo.GetSpellInfo()->HasAttribute(SPELL_ATTR0_CU_ENCHANT_PROC) ? castItemGUID : ObjectGuid::Empty, 0))
        {
            // effect masks do not match
            // extremely rare case
            // let's just recreate aura
            if (createInfo.GetAuraEffectMask() != foundAura->GetEffectMask())
                return nullptr;

            // update basepoints with new values - effect amount will be recalculated in ModStackAmount
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (!foundAura->HasEffect(i))
                    continue;

                int bp;
                if (createInfo.BaseAmount)
                    bp = *(createInfo.BaseAmount + i);
                else
                    bp = foundAura->GetSpellInfo()->Effects[i].BasePoints;

                int32* oldBP = const_cast<int32*>(&(foundAura->GetEffect(i)->m_baseAmount));
                *oldBP = bp;
            }

            // correct cast item guid if needed
            if (castItemGUID != foundAura->GetCastItemGUID())
            {
                ObjectGuid* oldGUID = const_cast<ObjectGuid*>(&foundAura->m_castItemGuid);
                *oldGUID = castItemGUID;
            }

            // try to increase stack amount
            foundAura->ModStackAmount(1, AURA_REMOVE_BY_DEFAULT, createInfo.ResetPeriodicTimer);
            return foundAura;
        }
    }

    return nullptr;
}

void Unit::_AddAura(UnitAura* aura, Unit* caster)
{
    ASSERT(!m_cleanupDone);
    m_ownedAuras.emplace(aura->GetId(), aura);

    _RemoveNoStackAurasDueToAura(aura);

    if (aura->IsRemoved())
        return;

    aura->SetIsSingleTarget(caster && (aura->GetSpellInfo()->IsSingleTarget() 
#ifdef LICH_KING
        || aura->HasEffectType(SPELL_AURA_CONTROL_VEHICLE)
#endif
        ));
    if (aura->IsSingleTarget())
    {
#ifdef LICH_KING
        bool loadingVehicle = IsLoading() && aura->HasEffectType(SPELL_AURA_CONTROL_VEHICLE);
#else
        bool loadingVehicle = false;
#endif
        ASSERT((IsInWorld() && !IsDuringRemoveFromWorld()) || (aura->GetCasterGUID() == GetGUID()) || loadingVehicle);
        /* @HACK: Player is not in world during loading auras.
        *        Single target auras are not saved or loaded from database
        *        but may be created as a result of aura links (player mounts with passengers)
        */

        // register single target aura
        caster->GetSingleCastAuras().push_back(aura);
        // remove other single target auras
        Unit::AuraList& scAuras = caster->GetSingleCastAuras();
        for (Unit::AuraList::iterator itr = scAuras.begin(); itr != scAuras.end();)
        {
            if ((*itr) != aura &&
                (*itr)->IsSingleTargetWith(aura))
            {
                (*itr)->Remove();
                itr = scAuras.begin();
            }
            else
                ++itr;
        }
    }
}

// creates aura application instance and registers it in lists
// aura application effects are handled separately to prevent aura list corruption
AuraApplication* Unit::_CreateAuraApplication(Aura* aura, uint8 effMask)
{
    // can't apply aura on unit which is going to be deleted - to not create a memory leak
    ASSERT(!m_cleanupDone);
    // aura musn't be removed
    ASSERT(!aura->IsRemoved());

    // aura mustn't be already applied on target
    ASSERT(!aura->IsAppliedOnTarget(GetGUID()) && "Unit::_CreateAuraApplication: aura musn't be applied on target");

    SpellInfo const* aurSpellInfo = aura->GetSpellInfo();
    uint32 aurId = aurSpellInfo->Id;

    // ghost spell check, allow apply any auras at player loading in ghost mode (will be cleanup after load)
    if (!IsAlive() && !aurSpellInfo->IsDeathPersistent() && !aurSpellInfo->IsAllowingDeadTarget() && //sun: added IsAllowingDeadTarget, else we won't see new aura for those on dead targets
        (GetTypeId() != TYPEID_PLAYER || !ToPlayer()->GetSession()->PlayerLoading()))
        return nullptr;

    Unit* caster = aura->GetCaster();

    AuraApplication * aurApp = new AuraApplication(this, caster, aura, effMask);
    m_appliedAuras.insert(AuraApplicationMap::value_type(aurId, aurApp));

    if (aurSpellInfo->AuraInterruptFlags)
    {
        m_interruptableAuras.push_back(aurApp);
        AddInterruptMask(aurSpellInfo->AuraInterruptFlags);
    }
    if ((aura->GetSpellInfo()->Attributes & SPELL_ATTR0_HEARTBEAT_RESIST_CHECK)
        && !(aura->GetSpellInfo()->HasAuraEffect(SPELL_AURA_MOD_POSSESS))) //only dummy aura is breakable
    {
        m_ccAuras.push_back(aurApp);
    }

    if (AuraStateType aState = aura->GetSpellInfo()->GetAuraState())
        m_auraStateAuras.insert(AuraStateAurasMap::value_type(aState, aurApp));

    aura->_ApplyForTarget(this, caster, aurApp);
    return aurApp;
}

void Unit::_ApplyAuraEffect(Aura* aura, uint8 effIndex)
{
    ASSERT(aura);
    ASSERT(aura->HasEffect(effIndex));
    AuraApplication * aurApp = aura->GetApplicationOfTarget(GetGUID());
    ASSERT(aurApp);
    if (!aurApp->GetEffectMask())
        _ApplyAura(aurApp, 1 << effIndex);
    else
        aurApp->_HandleEffect(effIndex, true);
}

// handles effects of aura application
// should be done after registering aura in lists
void Unit::_ApplyAura(AuraApplication* aurApp, uint8 effMask)
{
    Aura* aura = aurApp->GetBase();

    _RemoveNoStackAurasDueToAura(aura);

    if (aurApp->GetRemoveMode())
        return;

    // Update target aura state flag
    if (AuraStateType aState = aura->GetSpellInfo()->GetAuraState())
    {
        uint32 aStateMask = (1 << (aState - 1));
        // force update so the new caster registers it
        if ((aStateMask & PER_CASTER_AURA_STATE_MASK) && HasFlag(UNIT_FIELD_AURASTATE, aStateMask))
            ForceValuesUpdateAtIndex(UNIT_FIELD_AURASTATE);
        else
            ModifyAuraState(aState, true);
    }

    if (aurApp->GetRemoveMode())
        return;

    // Sitdown on apply aura req seated
    if (aura->GetSpellInfo()->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED && !IsSitState())
        SetStandState(UNIT_STAND_STATE_SIT);

    Unit* caster = aura->GetCaster();

    if (aurApp->GetRemoveMode())
        return;

    aura->HandleAuraSpecificMods(aurApp, caster, true, false);

    // apply effects of the aura
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (effMask & 1 << i && (!aurApp->GetRemoveMode()))
            aurApp->_HandleEffect(i, true);
    }
}

// removes aura application from lists and unapplies effects
void Unit::_UnapplyAura(AuraApplicationMap::iterator& i, AuraRemoveMode removeMode)
{
    AuraApplication * aurApp = i->second;
    ASSERT(aurApp);
    ASSERT(!aurApp->GetRemoveMode());
    ASSERT(aurApp->GetTarget() == this);

    aurApp->SetRemoveMode(removeMode);
    Aura* aura = aurApp->GetBase();
    TC_LOG_DEBUG("spells", "Aura %u now is remove mode %d", aura->GetId(), removeMode);

    // dead loop is killing the server probably
    ASSERT(m_removedAurasCount < 0xFFFFFFFF);

    ++m_removedAurasCount;

    Unit* caster = aura->GetCaster();

    // Remove all pointers from lists here to prevent possible pointer invalidation on spellcast/auraapply/auraremove
    m_appliedAuras.erase(i);

    if (aura->GetSpellInfo()->AuraInterruptFlags)
    {
        m_interruptableAuras.remove(aurApp);
        UpdateInterruptMask();
    }
    if ((aura->GetSpellInfo()->Attributes & SPELL_ATTR0_HEARTBEAT_RESIST_CHECK)
        && !(aura->HasEffectType(SPELL_AURA_MOD_POSSESS))) //only dummy aura is breakable
    {
        m_ccAuras.remove(aurApp);
    }

    bool auraStateFound = false;
    AuraStateType auraState = aura->GetSpellInfo()->GetAuraState();
    if (auraState)
    {
        bool canBreak = false;
        // Get mask of all aurastates from remaining auras
        for (AuraStateAurasMap::iterator itr = m_auraStateAuras.lower_bound(auraState); itr != m_auraStateAuras.upper_bound(auraState) && !(auraStateFound && canBreak);)
        {
            if (itr->second == aurApp)
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

    aurApp->_Remove();
    aura->_UnapplyForTarget(this, caster, aurApp);

    // remove effects of the spell - needs to be done after removing aura from lists
    for (uint8 itr = 0; itr < MAX_SPELL_EFFECTS; ++itr)
    {
        if (aurApp->HasEffect(itr))
            aurApp->_HandleEffect(itr, false);
    }

    // all effect mustn't be applied
    ASSERT(!aurApp->GetEffectMask());

    // Remove totem at next update if totem loses its aura
    if (aurApp->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE && GetTypeId() == TYPEID_UNIT && IsTotem())
    {
        if (ToTotem()->GetSpell() == aura->GetId() && ToTotem()->GetTotemType() == TOTEM_PASSIVE)
            ToTotem()->SetDeathState(JUST_DIED);
    }

    // Remove aurastates only if needed and were not found
    if (auraState)
    {
        if (!auraStateFound)
            ModifyAuraState(auraState, false);
        else
        {
            // update for casters, some shouldn't 'see' the aura state
            uint32 aStateMask = (1 << (auraState - 1));
            if ((aStateMask & PER_CASTER_AURA_STATE_MASK) != 0)
                ForceValuesUpdateAtIndex(UNIT_FIELD_AURASTATE);
        }
    }

    aura->HandleAuraSpecificMods(aurApp, caster, false, false);

    // only way correctly remove all auras from list
    //if (removedAuras != m_removedAurasCount) new aura may be added
    i = m_appliedAuras.begin();
}

void Unit::_UnapplyAura(AuraApplication* aurApp, AuraRemoveMode removeMode)
{
    // aura can be removed from unit only if it's applied on it, shouldn't happen
    ASSERT(aurApp->GetBase()->GetApplicationOfTarget(GetGUID()) == aurApp);

    uint32 spellId = aurApp->GetBase()->GetId();
    AuraApplicationMapBoundsNonConst range = m_appliedAuras.equal_range(spellId);

    for (AuraApplicationMap::iterator iter = range.first; iter != range.second;)
    {
        if (iter->second == aurApp)
        {
            _UnapplyAura(iter, removeMode);
            return;
        }
        else
            ++iter;
    }
    ABORT();
}

void Unit::_RemoveNoStackAurasDueToAura(Aura* aura, bool checkStrongerAura /*= false*/)
{
    SpellInfo const* spellProto = aura->GetSpellInfo();

    // passive spell special case (only non stackable with ranks)
    if (spellProto->IsPassiveStackableWithRanks())
        return;

    //sun: this is now also checked in Spell::CheckCast instead, and disabled here by default
    if (checkStrongerAura && !IsHighestExclusiveAura(aura))
    {
        aura->Remove();
        return;
    }

    bool remove = false;
    for (AuraApplicationMap::iterator i = m_appliedAuras.begin(); i != m_appliedAuras.end(); ++i)
    {
        if (remove)
        {
            remove = false;
            i = m_appliedAuras.begin();
        }

        if (aura->CanStackWith(i->second->GetBase()))
            continue;

        RemoveAura(i, AURA_REMOVE_BY_DEFAULT);
        if (i == m_appliedAuras.end())
            break;
        remove = true;
    }
}

void Unit::_RegisterAuraEffect(AuraEffect* aurEff, bool apply)
{
    if (apply)
        m_modAuras[aurEff->GetAuraType()].push_back(aurEff);
    else
        m_modAuras[aurEff->GetAuraType()].remove(aurEff);
}

// All aura base removes should go through this function!
void Unit::RemoveOwnedAura(AuraMap::iterator& i, AuraRemoveMode removeMode)
{
    Aura* aura = i->second;
    ASSERT(!aura->IsRemoved());

    // if unit currently update aura list then make safe update iterator shift to next
    if (m_auraUpdateIterator == i)
        ++m_auraUpdateIterator;

    m_ownedAuras.erase(i);
    m_removedAuras.push_back(aura);

    // Unregister single target aura
    if (aura->IsSingleTarget())
        aura->UnregisterSingleTarget();

    aura->_Remove(removeMode);

    i = m_ownedAuras.begin();
}

void Unit::RemoveOwnedAura(uint32 spellId, ObjectGuid casterGUID, uint8 reqEffMask, AuraRemoveMode removeMode)
{
    for (AuraMap::iterator itr = m_ownedAuras.lower_bound(spellId); itr != m_ownedAuras.upper_bound(spellId);)
        if (((itr->second->GetEffectMask() & reqEffMask) == reqEffMask) && (!casterGUID || itr->second->GetCasterGUID() == casterGUID))
        {
            RemoveOwnedAura(itr, removeMode);
            itr = m_ownedAuras.lower_bound(spellId);
        }
        else
            ++itr;
}

void Unit::RemoveOwnedAura(Aura* aura, AuraRemoveMode removeMode)
{
    if (aura->IsRemoved())
        return;

    ASSERT(aura->GetOwner() == this);

    if (removeMode == AURA_REMOVE_NONE)
    {
        TC_LOG_ERROR("spells", "Unit::RemoveOwnedAura() called with unallowed removeMode AURA_REMOVE_NONE, spellId %u", aura->GetId());
        return;
    }

    uint32 spellId = aura->GetId();
    AuraMapBoundsNonConst range = m_ownedAuras.equal_range(spellId);

    for (AuraMap::iterator itr = range.first; itr != range.second; ++itr)
    {
        if (itr->second == aura)
        {
            RemoveOwnedAura(itr, removeMode);
            return;
        }
    }

    ABORT();
}

Aura* Unit::GetOwnedAura(uint32 spellId, ObjectGuid casterGUID, ObjectGuid itemCasterGUID, uint8 reqEffMask, Aura* except) const
{
    AuraMapBounds range = m_ownedAuras.equal_range(spellId);
    for (AuraMap::const_iterator itr = range.first; itr != range.second; ++itr)
    {
        if (((itr->second->GetEffectMask() & reqEffMask) == reqEffMask)
            && (!casterGUID || itr->second->GetCasterGUID() == casterGUID)
            && (!itemCasterGUID || itr->second->GetCastItemGUID() == itemCasterGUID)
            && (!except || except != itr->second))
        {
            return itr->second;
        }
    }
    return nullptr;
}

void Unit::RemoveAura(AuraApplicationMap::iterator &i, AuraRemoveMode mode)
{
    AuraApplication * aurApp = i->second;
    // Do not remove aura which is already being removed
    if (aurApp->GetRemoveMode())
        return;
    Aura* aura = aurApp->GetBase();
    _UnapplyAura(i, mode);
    // Remove aura - for Area and Target auras
    if (aura->GetOwner() == this)
        aura->Remove(mode);
}

void Unit::RemoveAura(uint32 spellId, ObjectGuid caster, uint8 reqEffMask, AuraRemoveMode removeMode)
{
    AuraApplicationMapBoundsNonConst range = m_appliedAuras.equal_range(spellId);
    for (AuraApplicationMap::iterator iter = range.first; iter != range.second;)
    {
        Aura const* aura = iter->second->GetBase();
        if (((aura->GetEffectMask() & reqEffMask) == reqEffMask)
            && (!caster || aura->GetCasterGUID() == caster))
        {
            RemoveAura(iter, removeMode);
            return;
        }
        else
            ++iter;
    }
}

void Unit::RemoveAura(AuraApplication * aurApp, AuraRemoveMode mode)
{
    // we've special situation here, RemoveAura called while during aura removal
    // this kind of call is needed only when aura effect removal handler
    // or event triggered by it expects to remove
    // not yet removed effects of an aura
    if (aurApp->GetRemoveMode())
    {
        // remove remaining effects of an aura
        for (uint8 itr = 0; itr < MAX_SPELL_EFFECTS; ++itr)
        {
            if (aurApp->HasEffect(itr))
                aurApp->_HandleEffect(itr, false);
        }
        return;
    }
    // no need to remove
    if (aurApp->GetBase()->GetApplicationOfTarget(GetGUID()) != aurApp || aurApp->GetBase()->IsRemoved())
        return;

    uint32 spellId = aurApp->GetBase()->GetId();
    AuraApplicationMapBoundsNonConst range = m_appliedAuras.equal_range(spellId);

    for (AuraApplicationMap::iterator iter = range.first; iter != range.second;)
    {
        if (aurApp == iter->second)
        {
            RemoveAura(iter, mode);
            return;
        }
        else
            ++iter;
    }
}

void Unit::RemoveAura(Aura* aura, AuraRemoveMode mode)
{
    if (aura->IsRemoved())
        return;
    if (AuraApplication * aurApp = aura->GetApplicationOfTarget(GetGUID()))
        RemoveAura(aurApp, mode);
}

void Unit::RemoveAppliedAuras(std::function<bool(AuraApplication const*)> const& check)
{
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        if (check(iter->second))
        {
            RemoveAura(iter);
            continue;
        }
        ++iter;
    }
}

void Unit::RemoveOwnedAuras(std::function<bool(Aura const*)> const& check)
{
    for (AuraMap::iterator iter = m_ownedAuras.begin(); iter != m_ownedAuras.end();)
    {
        if (check(iter->second))
        {
            RemoveOwnedAura(iter);
            continue;
        }
        ++iter;
    }
}

void Unit::RemoveAppliedAuras(uint32 spellId, std::function<bool(AuraApplication const*)> const& check)
{
    for (AuraApplicationMap::iterator iter = m_appliedAuras.lower_bound(spellId); iter != m_appliedAuras.upper_bound(spellId);)
    {
        if (check(iter->second))
        {
            RemoveAura(iter);
            continue;
        }
        ++iter;
    }
}

void Unit::RemoveOwnedAuras(uint32 spellId, std::function<bool(Aura const*)> const& check)
{
    for (AuraMap::iterator iter = m_ownedAuras.lower_bound(spellId); iter != m_ownedAuras.upper_bound(spellId);)
    {
        if (check(iter->second))
        {
            RemoveOwnedAura(iter);
            continue;
        }
        ++iter;
    }
}

void Unit::RemoveAurasByType(AuraType auraType, std::function<bool(AuraApplication const*)> const& check)
{
    for (AuraEffectList::iterator iter = m_modAuras[auraType].begin(); iter != m_modAuras[auraType].end();)
    {
        Aura* aura = (*iter)->GetBase();
        AuraApplication * aurApp = aura->GetApplicationOfTarget(GetGUID());
        ASSERT(aurApp);

        ++iter;
        if (check(aurApp))
        {
            uint32 removedAuras = m_removedAurasCount;
            RemoveAura(aurApp);
            if (m_removedAurasCount > removedAuras + 1)
                iter = m_modAuras[auraType].begin();
        }
    }
}

void Unit::RemoveAurasDueToSpell(uint32 spellId, ObjectGuid casterGUID, uint8 reqEffMask, AuraRemoveMode removeMode)
{
    for (AuraApplicationMap::iterator iter = m_appliedAuras.lower_bound(spellId); iter != m_appliedAuras.upper_bound(spellId);)
    {
        Aura const* aura = iter->second->GetBase();
        if (((aura->GetEffectMask() & reqEffMask) == reqEffMask)
            && (!casterGUID || aura->GetCasterGUID() == casterGUID))
        {
            RemoveAura(iter, removeMode);
            iter = m_appliedAuras.lower_bound(spellId);
        }
        else
            ++iter;
    }
}

void Unit::RemoveAuraFromStack(uint32 spellId, ObjectGuid casterGUID, AuraRemoveMode removeMode)
{
    AuraMapBoundsNonConst range = m_ownedAuras.equal_range(spellId);
    for (AuraMap::iterator iter = range.first; iter != range.second;)
    {
        Aura* aura = iter->second;
        if ((aura->GetType() == UNIT_AURA_TYPE)
            && (!casterGUID || aura->GetCasterGUID() == casterGUID))
        {
            aura->ModStackAmount(-1, removeMode);
            return;
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasDueToSpellByDispel(uint32 spellId, uint32 dispellerSpellId, ObjectGuid casterGUID, WorldObject* dispeller, uint8 chargesRemoved/*= 1*/)
{
    AuraMapBoundsNonConst range = m_ownedAuras.equal_range(spellId);
    for (AuraMap::iterator iter = range.first; iter != range.second;)
    {
        Aura* aura = iter->second;
        if (aura->GetCasterGUID() == casterGUID)
        {
            DispelInfo dispelInfo(dispeller, dispellerSpellId, chargesRemoved);

            // Call OnDispel hook on AuraScript
            aura->CallScriptDispel(&dispelInfo);

#ifdef LICH_KING
            if (aura->GetSpellInfo()->HasAttribute(SPELL_ATTR7_DISPEL_CHARGES))
                aura->ModCharges(-dispelInfo.GetRemovedCharges(), AURA_REMOVE_BY_ENEMY_SPELL);
            else
#endif
            aura->ModStackAmount(-dispelInfo.GetRemovedCharges(), AURA_REMOVE_BY_ENEMY_SPELL);

            // Call AfterDispel hook on AuraScript
            aura->CallScriptAfterDispel(&dispelInfo);

            return;
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasDueToSpellBySteal(uint32 spellId, ObjectGuid casterGUID, WorldObject* stealer)
{
    AuraMapBoundsNonConst range = m_ownedAuras.equal_range(spellId);
    for (AuraMap::iterator iter = range.first; iter != range.second;)
    {
        Aura* aura = iter->second;
        if (aura->GetCasterGUID() == casterGUID)
        {
            int32 damage[MAX_SPELL_EFFECTS];
            int32 baseDamage[MAX_SPELL_EFFECTS];
            uint8 effMask = 0;
            uint8 recalculateMask = 0;
            Unit* caster = aura->GetCaster();
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (aura->GetEffect(i))
                {
                    baseDamage[i] = aura->GetEffect(i)->GetBaseAmount();
                    damage[i] = aura->GetEffect(i)->GetAmount();
                    effMask |= (1 << i);
                    if (aura->GetEffect(i)->CanBeRecalculated())
                        recalculateMask |= (1 << i);
                }
                else
                {
                    baseDamage[i] = 0;
                    damage[i] = 0;
                }
            }

#ifdef LICH_KING
            bool stealCharge = aura->GetSpellInfo()->HasAttribute(SPELL_ATTR7_DISPEL_CHARGES);
#else
            bool stealCharge = false;
#endif
            // Cast duration to unsigned to prevent permanent aura's such as Righteous Fury being permanently added to caster
            uint32 dur = std::min(2u * MINUTE * IN_MILLISECONDS, uint32(aura->GetDuration()));

            if (Unit* unitStealer = stealer->ToUnit())
            {
                if (Aura* oldAura = unitStealer->GetAura(aura->GetId(), aura->GetCasterGUID()))
                {
                    if (stealCharge)
                        oldAura->ModCharges(1);
                    else
                        oldAura->ModStackAmount(1);
                    oldAura->SetDuration(int32(dur));
                }
                else
                {
                    // single target state must be removed before aura creation to preserve existing single target aura
                    if (aura->IsSingleTarget())
                        aura->UnregisterSingleTarget();

                    AuraCreateInfo createInfo(aura->GetSpellInfo(), effMask, unitStealer);
                    createInfo
                        .SetCasterGUID(aura->GetCasterGUID())
                        .SetBaseAmount(baseDamage);

                    if (Aura* newAura = Aura::TryRefreshStackOrCreate(createInfo))
                    {
                        // created aura must not be single target aura,, so stealer won't loose it on recast
                        if (newAura->IsSingleTarget())
                        {
                            newAura->UnregisterSingleTarget();
                            // bring back single target aura status to the old aura
                            aura->SetIsSingleTarget(true);
                            caster->GetSingleCastAuras().push_back(aura);
                        }
                        // FIXME: using aura->GetMaxDuration() maybe not blizzlike but it fixes stealing of spells like Innervate
                        newAura->SetLoadedState(aura->GetMaxDuration(), int32(dur), stealCharge ? 1 : aura->GetCharges(), 1, recalculateMask, aura->GetCritChance(), aura->CanApplyResilience(), &damage[0]);
                        newAura->ApplyForTargets();
                    }
                }
            }

            if (stealCharge)
                aura->ModCharges(-1, AURA_REMOVE_BY_ENEMY_SPELL);
            else
                aura->ModStackAmount(-1, AURA_REMOVE_BY_ENEMY_SPELL);

            return;
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasDueToItemSpell(uint32 spellId, ObjectGuid castItemGuid)
{
    for (AuraApplicationMap::iterator iter = m_appliedAuras.lower_bound(spellId); iter != m_appliedAuras.upper_bound(spellId);)
    {
        if (iter->second->GetBase()->GetCastItemGUID() == castItemGuid)
        {
            RemoveAura(iter);
            iter = m_appliedAuras.lower_bound(spellId);
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasByType(AuraType auraType, ObjectGuid casterGUID, Aura* except, bool negative, bool positive)
{
    for (AuraEffectList::iterator iter = m_modAuras[auraType].begin(); iter != m_modAuras[auraType].end();)
    {
        Aura* aura = (*iter)->GetBase();
        AuraApplication * aurApp = aura->GetApplicationOfTarget(GetGUID());
        ASSERT(aurApp);

        ++iter;
        if (aura != except && (!casterGUID || aura->GetCasterGUID() == casterGUID)
            && ((negative && !aurApp->IsPositive()) || (positive && aurApp->IsPositive())))
        {
            uint32 removedAuras = m_removedAurasCount;
            RemoveAura(aurApp);
            if (m_removedAurasCount > removedAuras + 1)
                iter = m_modAuras[auraType].begin();
        }
    }
}

void Unit::RemoveAurasWithAttribute(uint32 flags)
{
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        SpellInfo const* spell = iter->second->GetBase()->GetSpellInfo();
        if (spell->Attributes & flags)
            RemoveAura(iter);
        else
            ++iter;
    }
}

void Unit::RemoveAurasWithCustomAttribute(SpellCustomAttributes flags)
{
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        SpellInfo const* spell = iter->second->GetBase()->GetSpellInfo();
        if (spell->HasAttribute(flags))
            RemoveAura(iter);
        else
            ++iter;
    }
}

void Unit::RemoveNotOwnSingleTargetAuras(uint32 newPhase)
{
    // single target auras from other casters
    // Iterate m_ownedAuras - aura is marked as single target in Unit::AddAura (and pushed to m_ownedAuras).
    // m_appliedAuras will NOT contain the aura before first Unit::Update after adding it to m_ownedAuras.
    // Quickly removing such an aura will lead to it not being unregistered from caster's single cast auras container
    // leading to assertion failures if the aura was cast on a player that can
    // (and is changing map at the point where this function is called).
    // Such situation occurs when player is logging in inside an instance and fails the entry check for any reason.
    // The aura that was loaded from db (indirectly, via linked casts) gets removed before it has a chance
    // to register in m_appliedAuras
    for (AuraMap::iterator iter = m_ownedAuras.begin(); iter != m_ownedAuras.end();)
    {
        Aura const* aura = iter->second;

        if (aura->GetCasterGUID() != GetGUID() && aura->IsSingleTarget())
        {
            if (!newPhase)
                RemoveOwnedAura(iter);
            else
            {
                Unit* caster = aura->GetCaster();
                if (!caster || !caster->InSamePhase(newPhase))
                    RemoveOwnedAura(iter);
                else
                    ++iter;
            }
        }
        else
            ++iter;
    }

    // single target auras at other targets
    AuraList& scAuras = GetSingleCastAuras();
    for (AuraList::iterator iter = scAuras.begin(); iter != scAuras.end();)
    {
        Aura* aura = *iter;
        if (aura->GetUnitOwner() != this && !aura->GetUnitOwner()->InSamePhase(newPhase))
        {
            aura->Remove();
            iter = scAuras.begin();
        }
        else
            ++iter;
    }
}

void Unit::RemoveAurasWithInterruptFlags(uint32 flag, uint32 except, bool withChanneled)
{
    if (!(m_interruptMask & flag))
        return;

    // interrupt auras
    for (AuraApplicationList::iterator iter = m_interruptableAuras.begin(); iter != m_interruptableAuras.end();)
    {
        Aura* aura = (*iter)->GetBase();
        ++iter;
        if ((aura->GetSpellInfo()->AuraInterruptFlags & flag) && (!except || aura->GetId() != except))
        {
            uint32 removedAuras = m_removedAurasCount;
            RemoveAura(aura);
            if (m_removedAurasCount > removedAuras + 1)
                iter = m_interruptableAuras.begin();
        }
    }

    if (withChanneled)
    {
        // interrupt channeled spell
        if (Spell* spell = m_currentSpells[CURRENT_CHANNELED_SPELL])
            if (spell->getState() == SPELL_STATE_CASTING
                && (spell->m_spellInfo->ChannelInterruptFlags & flag)
                && spell->m_spellInfo->Id != except)
                InterruptNonMeleeSpells(false);
    }

    UpdateInterruptMask();
}
#ifdef LICH_KING
void Unit::RemoveAurasWithFamily(SpellFamilyNames family, uint32 familyFlag1, uint32 familyFlag2, uint32 familyFlag3, ObjectGuid casterGUID)
#else
void Unit::RemoveAurasWithFamily(SpellFamilyNames family, uint64 familyFlags, ObjectGuid casterGUID)
#endif
{
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        Aura const* aura = iter->second->GetBase();
        if (!casterGUID || aura->GetCasterGUID() == casterGUID)
        {
            SpellInfo const* spell = aura->GetSpellInfo();
            if (spell->SpellFamilyName == uint32(family) 
#ifdef LICH_KING
                && spell->SpellFamilyFlags.HasFlag(familyFlag1, familyFlag2, familyFlag3)
#else
                && spell->SpellFamilyFlags & familyFlags
#endif
                )
            {
                RemoveAura(iter);
                continue;
            }
        }
        ++iter;
    }
}

void Unit::RemoveMovementImpairingAuras(bool withRoot)
{
    if (withRoot)
        RemoveAurasWithMechanic(1 << MECHANIC_ROOT);

    // Snares
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        Aura const* aura = iter->second->GetBase();
        if (aura->GetSpellInfo()->Mechanic == MECHANIC_SNARE)
        {
            RemoveAura(iter);
            continue;
        }

        // turn off snare auras by setting amount to 0
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (((1 << i) & iter->second->GetEffectMask()) && aura->GetSpellInfo()->Effects[i].Mechanic == MECHANIC_SNARE)
                aura->GetEffect(i)->ChangeAmount(0);

        ++iter;
    }
}

void Unit::RemoveAurasWithMechanic(uint32 mechanic_mask, AuraRemoveMode removemode, uint32 except)
{
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        Aura const* aura = iter->second->GetBase();
        if (!except || aura->GetId() != except)
        {
            if (aura->GetSpellInfo()->GetAllEffectsMechanicMask() & mechanic_mask)
            {
                RemoveAura(iter, removemode);
                continue;
            }
        }
        ++iter;
    }
}

void Unit::RemoveAurasByShapeShift()
{
    uint32 mechanic_mask = (1 << MECHANIC_SNARE) | (1 << MECHANIC_ROOT);
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        Aura const* aura = iter->second->GetBase();
        if ((aura->GetSpellInfo()->GetAllEffectsMechanicMask() & mechanic_mask) && !aura->GetSpellInfo()->HasAttribute(SPELL_ATTR0_CU_AURA_CC))
        {
            RemoveAura(iter);
            continue;
        }
        ++iter;
    }
}

void Unit::RemoveAreaAurasDueToLeaveWorld()
{
    // make sure that all area auras not applied on self are removed - prevent access to deleted pointer later
    for (AuraMap::iterator iter = m_ownedAuras.begin(); iter != m_ownedAuras.end();)
    {
        Aura* aura = iter->second;
        ++iter;
        Aura::ApplicationMap const& appMap = aura->GetApplicationMap();
        for (Aura::ApplicationMap::const_iterator itr = appMap.begin(); itr != appMap.end();)
        {
            AuraApplication * aurApp = itr->second;
            ++itr;
            Unit* target = aurApp->GetTarget();
            if (target == this)
                continue;
            target->RemoveAura(aurApp);
            // things linked on aura remove may apply new area aura - so start from the beginning
            iter = m_ownedAuras.begin();
        }
    }

    // remove area auras owned by others
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        if (iter->second->GetBase()->GetOwner() != this)
        {
            RemoveAura(iter);
        }
        else
            ++iter;
    }
}

void Unit::RemoveAllAuras()
{
    // this may be a dead loop if some events on aura remove will continiously apply aura on remove
    // we want to have all auras removed, so use your brain when linking events
    while (!m_appliedAuras.empty() || !m_ownedAuras.empty())
    {
        AuraApplicationMap::iterator aurAppIter;
        for (aurAppIter = m_appliedAuras.begin(); aurAppIter != m_appliedAuras.end();)
            _UnapplyAura(aurAppIter, AURA_REMOVE_BY_DEFAULT);

        AuraMap::iterator aurIter;
        for (aurIter = m_ownedAuras.begin(); aurIter != m_ownedAuras.end();)
            RemoveOwnedAura(aurIter);
    }
}

void Unit::RemoveArenaAuras(bool onleave)
{
    // in join, remove positive buffs, on end, remove negative
    // used to remove positive visible auras in arenas
    RemoveAppliedAuras([&](AuraApplication const* aurApp)
    {
        Aura const* aura = aurApp->GetBase();
        return (!aura->GetSpellInfo()->HasAttribute(SPELL_ATTR4_UNK21)                                         // don't remove stances, shadowform, pally/hunter auras
            && (!aura->GetSpellInfo()->HasAttribute(SPELL_ATTR2_PRESERVE_ENCHANT_IN_ARENA))
            && !aura->IsPassive()                                                                              // don't remove passive auras
            && (!onleave || !aurApp->IsPositive())                                                             // remove all buffs on enter, negative buffs on leave
            && (aurApp->IsPositive() || !aura->GetSpellInfo()->HasAttribute(SPELL_ATTR3_DEATH_PERSISTENT))) || // not negative death persistent auras
            aura->GetSpellInfo()->HasAttribute(SPELL_ATTR5_REMOVE_ON_ARENA_ENTER);                             // special marker, always remove
    });

    if (Player* plr = ToPlayer()) {
        if (Pet* pet = GetPet())
            pet->RemoveArenaAuras(onleave);
        else
            plr->RemoveAllCurrentPetAuras(); //still remove auras if the players hasnt called his pet yet
    }
}

void Unit::RemoveAurasOnEvade()
{
    if (IsCharmedOwnedByPlayerOrPlayer()) // if it is a player owned creature it should not remove the aura
        return;

#ifdef LICH_KING
    // don't remove vehicle auras, passengers aren't supposed to drop off the vehicle
    RemoveAllAurasExceptType(SPELL_AURA_CONTROL_VEHICLE, SPELL_AURA_CLONE_CASTER);
#else
    // don't remove clone caster on evade (to be verified)
    RemoveAllAurasExceptType(SPELL_AURA_CLONE_CASTER);
#endif
}

void Unit::RemoveAllAurasOnDeath()
{
    // used just after dieing to remove all visible auras
    // and disable the mods for the passive ones
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        Aura const* aura = iter->second->GetBase();
        if (!aura->IsPassive() && !aura->IsDeathPersistent())
            _UnapplyAura(iter, AURA_REMOVE_BY_DEATH);
        else
            ++iter;
    }

    for (AuraMap::iterator iter = m_ownedAuras.begin(); iter != m_ownedAuras.end();)
    {
        Aura* aura = iter->second;
        if (!aura->IsPassive() && !aura->IsDeathPersistent())
            RemoveOwnedAura(iter, AURA_REMOVE_BY_DEATH);
        else
            ++iter;
    }
}

void Unit::RemoveAllAurasRequiringDeadTarget()
{
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        Aura const* aura = iter->second->GetBase();
        if (!aura->IsPassive() && aura->GetSpellInfo()->IsRequiringDeadTarget())
            _UnapplyAura(iter, AURA_REMOVE_BY_DEFAULT);
        else
            ++iter;
    }

    for (AuraMap::iterator iter = m_ownedAuras.begin(); iter != m_ownedAuras.end();)
    {
        Aura* aura = iter->second;
        if (!aura->IsPassive() && aura->GetSpellInfo()->IsRequiringDeadTarget())
            RemoveOwnedAura(iter, AURA_REMOVE_BY_DEFAULT);
        else
            ++iter;
    }
}

void Unit::RemoveAllAurasExceptType(AuraType type)
{
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        Aura const* aura = iter->second->GetBase();
        if (aura->GetSpellInfo()->HasAura(type))
            ++iter;
        else
            _UnapplyAura(iter, AURA_REMOVE_BY_DEFAULT);
    }

    for (AuraMap::iterator iter = m_ownedAuras.begin(); iter != m_ownedAuras.end();)
    {
        Aura* aura = iter->second;
        if (aura->GetSpellInfo()->HasAura(type))
            ++iter;
        else
            RemoveOwnedAura(iter, AURA_REMOVE_BY_DEFAULT);
    }
}

void Unit::RemoveAllAurasExceptType(AuraType type1, AuraType type2)
{
    for (AuraApplicationMap::iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end();)
    {
        Aura const* aura = iter->second->GetBase();
        if (aura->GetSpellInfo()->HasAura(type1) || aura->GetSpellInfo()->HasAura(type2))
            ++iter;
        else
            _UnapplyAura(iter, AURA_REMOVE_BY_DEFAULT);
    }

    for (AuraMap::iterator iter = m_ownedAuras.begin(); iter != m_ownedAuras.end();)
    {
        Aura* aura = iter->second;
        if (aura->GetSpellInfo()->HasAura(type1) || aura->GetSpellInfo()->HasAura(type2))
            ++iter;
        else
            RemoveOwnedAura(iter, AURA_REMOVE_BY_DEFAULT);
    }
}

void Unit::RemoveAllGroupBuffsFromCaster(ObjectGuid casterGUID)
{
    for (AuraMap::iterator iter = m_ownedAuras.begin(); iter != m_ownedAuras.end();)
    {
        Aura* aura = iter->second;
        if (aura->GetCasterGUID() == casterGUID && aura->GetSpellInfo()->IsGroupBuff())
        {
            RemoveOwnedAura(iter);
            continue;
        }
        ++iter;
    }
}

void Unit::DelayOwnedAuras(uint32 spellId, ObjectGuid caster, int32 delaytime)
{
    AuraMapBoundsNonConst range = m_ownedAuras.equal_range(spellId);
    for (; range.first != range.second; ++range.first)
    {
        Aura* aura = range.first->second;
        if (!caster || aura->GetCasterGUID() == caster)
        {
            if (aura->GetDuration() < delaytime)
                aura->SetDuration(0);
            else
                aura->SetDuration(aura->GetDuration() - delaytime);

            // update for out of range group members (on 1 slot use)
            aura->SetNeedClientUpdateForTargets();
        }
    }
}

void Unit::_RemoveAllAuraStatMods()
{
    for (AuraApplicationMap::iterator i = m_appliedAuras.begin(); i != m_appliedAuras.end(); ++i)
        (*i).second->GetBase()->HandleAllEffects(i->second, AURA_EFFECT_HANDLE_STAT, false);
}

void Unit::_ApplyAllAuraStatMods()
{
    for (AuraApplicationMap::iterator i = m_appliedAuras.begin(); i != m_appliedAuras.end(); ++i)
        (*i).second->GetBase()->HandleAllEffects(i->second, AURA_EFFECT_HANDLE_STAT, true);
}

AuraEffect* Unit::GetAuraEffect(uint32 spellId, uint8 effIndex, ObjectGuid caster) const
{
    AuraApplicationMapBounds range = m_appliedAuras.equal_range(spellId);
    for (AuraApplicationMap::const_iterator itr = range.first; itr != range.second; ++itr)
    {
        if (itr->second->HasEffect(effIndex)
            && (!caster || itr->second->GetBase()->GetCasterGUID() == caster))
        {
            return itr->second->GetBase()->GetEffect(effIndex);
        }
    }
    return nullptr;
}

AuraEffect* Unit::GetAuraEffectOfRankedSpell(uint32 spellId, uint8 effIndex, ObjectGuid caster) const
{
    uint32 rankSpell = sSpellMgr->GetFirstSpellInChain(spellId);
    while (rankSpell)
    {
        if (AuraEffect* aurEff = GetAuraEffect(rankSpell, effIndex, caster))
            return aurEff;
        rankSpell = sSpellMgr->GetNextSpellInChain(rankSpell);
    }
    return nullptr;
}

AuraEffect* Unit::GetAuraEffect(AuraType type, SpellFamilyNames name, uint32 iconId, uint8 effIndex) const
{
    AuraEffectList const& auras = GetAuraEffectsByType(type);
    for (Unit::AuraEffectList::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
    {
        if (effIndex != (*itr)->GetEffIndex())
            continue;
        SpellInfo const* spell = (*itr)->GetSpellInfo();
        if (spell->SpellIconID == iconId && spell->SpellFamilyName == uint32(name) && !spell->SpellFamilyFlags)
            return *itr;
    }
    return nullptr;
}

#ifdef LICH_KING
AuraEffect* Unit::GetAuraEffect(AuraType type, SpellFamilyNames family, uint32 familyFlag1, uint32 familyFlag2, uint32 familyFlag3, ObjectGuid casterGUID) const
#else
AuraEffect* Unit::GetAuraEffect(AuraType type, SpellFamilyNames family, uint64 familyFlags, ObjectGuid casterGUID) const
#endif
{
    AuraEffectList const& auras = GetAuraEffectsByType(type);
    for (AuraEffectList::const_iterator i = auras.begin(); i != auras.end(); ++i)
    {
        SpellInfo const* spell = (*i)->GetSpellInfo();
        if (spell->SpellFamilyName == uint32(family) && 
#ifdef LICH_KING
            spell->SpellFamilyFlags.HasFlag(familyFlag1, familyFlag2, familyFlag3)
#else
            spell->SpellFamilyFlags & familyFlags
#endif
            )
        {
            if (casterGUID && (*i)->GetCasterGUID() != casterGUID)
                continue;
            return (*i);
        }
    }
    return nullptr;
}

AuraEffect* Unit::GetDummyAuraEffect(SpellFamilyNames name, uint32 iconId, uint8 effIndex) const
{
    return GetAuraEffect(SPELL_AURA_DUMMY, name, iconId, effIndex);
}

AuraEffect* Unit::GetDummyAura(uint32 spell_id) const
{
    Unit::AuraEffectList const& mDummy = GetAuraEffectsByType(SPELL_AURA_DUMMY);

    for (auto itr : mDummy)
        if (itr->GetId() == spell_id)
            return itr;

    return nullptr;
}

AuraApplication * Unit::GetAuraApplication(uint32 spellId, ObjectGuid casterGUID, ObjectGuid itemCasterGUID, uint8 reqEffMask, AuraApplication * except) const
{
    AuraApplicationMapBounds range = m_appliedAuras.equal_range(spellId);
    for (; range.first != range.second; ++range.first)
    {
        AuraApplication* app = range.first->second;
        Aura const* aura = app->GetBase();

        if (((aura->GetEffectMask() & reqEffMask) == reqEffMask)
            && (!casterGUID || aura->GetCasterGUID() == casterGUID)
            && (!itemCasterGUID || aura->GetCastItemGUID() == itemCasterGUID)
            && (!except || except != app))
        {
            return app;
        }
    }
    return nullptr;
}

Aura* Unit::GetAura(uint32 spellId, ObjectGuid casterGUID, ObjectGuid itemCasterGUID, uint8 reqEffMask) const
{
    AuraApplication * aurApp = GetAuraApplication(spellId, casterGUID, itemCasterGUID, reqEffMask);
    return aurApp ? aurApp->GetBase() : nullptr;
}

AuraApplication * Unit::GetAuraApplicationOfRankedSpell(uint32 spellId, ObjectGuid casterGUID, ObjectGuid itemCasterGUID, uint8 reqEffMask, AuraApplication* except) const
{
    uint32 rankSpell = sSpellMgr->GetFirstSpellInChain(spellId);
    while (rankSpell)
    {
        if (AuraApplication * aurApp = GetAuraApplication(rankSpell, casterGUID, itemCasterGUID, reqEffMask, except))
            return aurApp;
        rankSpell = sSpellMgr->GetNextSpellInChain(rankSpell);
    }
    return nullptr;
}

Aura* Unit::GetAuraOfRankedSpell(uint32 spellId, ObjectGuid casterGUID, ObjectGuid itemCasterGUID, uint8 reqEffMask) const
{
    AuraApplication * aurApp = GetAuraApplicationOfRankedSpell(spellId, casterGUID, itemCasterGUID, reqEffMask);
    return aurApp ? aurApp->GetBase() : nullptr;
}

void Unit::GetDispellableAuraList(WorldObject const* caster, uint32 dispelMask, DispelChargesList& dispelList, bool isReflect /*= false*/)  const
{
#ifdef LICH_KING
    // we should not be able to dispel diseases if the target is affected by unholy blight
    if (dispelMask & (1 << DISPEL_DISEASE) && HasAura(50536))
        dispelMask &= ~(1 << DISPEL_DISEASE);
#endif

    AuraMap const& auras = GetOwnedAuras();
    for (auto itr = auras.begin(); itr != auras.end(); ++itr)
    {
        Aura* aura = itr->second;
        AuraApplication const* aurApp = aura->GetApplicationOfTarget(GetGUID());
        if (!aurApp)
            continue;

        // don't try to remove passive auras
        if (aura->IsPassive())
            continue;

        if (aura->GetSpellInfo()->GetDispelMask() & dispelMask)
        {
            // do not remove positive auras if friendly target
            //               negative auras if non-friendly
            // unless we're reflecting (dispeller eliminates one of it's benefitial buffs)
            if (isReflect != (aurApp->IsPositive() == IsFriendlyTo(caster)))
                continue;

            // 2.4.3 Patch Notes: "Dispel effects will no longer attempt to remove effects that have 100% dispel resistance."
            int32 chance = aura->CalcDispelChance(this, !IsFriendlyTo(caster));
            if (!chance)
                continue;

            // The charges / stack amounts don't count towards the total number of auras that can be dispelled.
            // Ie: A dispel on a target with 5 stacks of Winters Chill and a Polymorph has 1 / (1 + 1) -> 50% chance to dispell
            // Polymorph instead of 1 / (5 + 1) -> 16%.
#ifdef LICH_KING
            bool const dispelCharges = aura->GetSpellInfo()->HasAttribute(SPELL_ATTR7_DISPEL_CHARGES);
            uint8 charges = dispelCharges ? aura->GetCharges() : aura->GetStackAmount();
#else
            uint8 charges = aura->GetStackAmount();
#endif
            if (charges > 0)
                dispelList.emplace_back(aura, chance, charges);
        }
    }
}

bool Unit::HasAuraEffect(uint32 spellId, uint8 effIndex, ObjectGuid caster) const
{
    AuraApplicationMapBounds range = m_appliedAuras.equal_range(spellId);
    for (AuraApplicationMap::const_iterator itr = range.first; itr != range.second; ++itr)
    {
        if (itr->second->HasEffect(effIndex)
            && (!caster || itr->second->GetBase()->GetCasterGUID() == caster))
        {
            return true;
        }
    }
    return false;
}

uint32 Unit::GetAuraCount(uint32 spellId) const
{
    uint32 count = 0;
    AuraApplicationMapBounds range = m_appliedAuras.equal_range(spellId);

    for (AuraApplicationMap::const_iterator itr = range.first; itr != range.second; ++itr)
    {
        if (itr->second->GetBase()->GetStackAmount() == 0)
            ++count;
        else
            count += (uint32)itr->second->GetBase()->GetStackAmount();
    }

    return count;
}

bool Unit::HasAura(uint32 spellId, ObjectGuid casterGUID, ObjectGuid itemCasterGUID, uint8 reqEffMask) const
{
    if (GetAuraApplication(spellId, casterGUID, itemCasterGUID, reqEffMask))
        return true;
    return false;
}

bool Unit::HasAuraType(AuraType auraType) const
{
    return (!m_modAuras[auraType].empty());
}

bool Unit::HasAuraTypeWithCaster(AuraType auraType, ObjectGuid caster) const
{
    for (AuraEffect const* eff : GetAuraEffectsByType(auraType))
        if (caster == eff->GetCasterGUID())
            return true;

    return false;
}

bool Unit::HasAuraTypeWithMiscvalue(AuraType auraType, int32 miscvalue) const
{
    AuraEffectList const& mTotalAuraList = GetAuraEffectsByType(auraType);
    for (AuraEffectList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
        if (miscvalue == (*i)->GetMiscValue())
            return true;

    return false;
}

bool Unit::HasAuraTypeWithAffectMask(AuraType auraType, SpellInfo const* affectedSpell) const
{
    AuraEffectList const& mTotalAuraList = GetAuraEffectsByType(auraType);
    for (AuraEffectList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
        if ((*i)->IsAffectedOnSpell(affectedSpell))
            return true;
    return false;
}

bool Unit::HasAuraTypeWithValue(AuraType auraType, int32 value) const
{
    AuraEffectList const& mTotalAuraList = GetAuraEffectsByType(auraType);
    for (AuraEffectList::const_iterator i = mTotalAuraList.begin(); i != mTotalAuraList.end(); ++i)
        if (value == (*i)->GetAmount())
            return true;
    return false;
}

bool Unit::HasNegativeAuraWithInterruptFlag(uint32 flag, ObjectGuid guid) const
{
    if (!(m_interruptMask & flag))
        return false;
    for (AuraApplicationList::const_iterator iter = m_interruptableAuras.begin(); iter != m_interruptableAuras.end(); ++iter)
    {
        if (!(*iter)->IsPositive() && (*iter)->GetBase()->GetSpellInfo()->AuraInterruptFlags & flag && (!guid || (*iter)->GetBase()->GetCasterGUID() == guid))
            return true;
    }
    return false;
}

bool Unit::HasAuraWithMechanic(Mechanics mechanicMask) const
{
    for (AuraApplicationMap::const_iterator iter = m_appliedAuras.begin(); iter != m_appliedAuras.end(); ++iter)
    {
        SpellInfo const* spellInfo = iter->second->GetBase()->GetSpellInfo();
        if (spellInfo->Mechanic && (mechanicMask & (1 << spellInfo->Mechanic)))
            return true;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (iter->second->HasEffect(i) && spellInfo->Effects[i].Effect && spellInfo->Effects[i].Mechanic)
                if (mechanicMask & (1 << spellInfo->Effects[i].Mechanic))
                    return true;
    }

    return false;
}

bool Unit::HasStrongerAuraWithDR(SpellInfo const* auraSpellInfo, Unit* caster, bool triggered) const
{
    DiminishingGroup diminishGroup = auraSpellInfo->GetDiminishingReturnsGroupForSpell(triggered);
    DiminishingLevels level = GetDiminishing(diminishGroup);
    for (auto itr = m_appliedAuras.begin(); itr != m_appliedAuras.end(); ++itr)
    {
        SpellInfo const* spellInfo = itr->second->GetBase()->GetSpellInfo();
        if (spellInfo->GetDiminishingReturnsGroupForSpell(triggered) != diminishGroup)
            continue;

        int32 existingDuration = itr->second->GetBase()->GetDuration();
        int32 newDuration = auraSpellInfo->GetMaxDuration();
        ApplyDiminishingToDuration(auraSpellInfo, triggered, newDuration, caster, level);
        if (newDuration > 0 && newDuration < existingDuration)
            return true;
    }

    return false;
}

AuraEffect* Unit::IsScriptOverriden(SpellInfo const* spell, int32 script) const
{
    AuraEffectList const& auras = GetAuraEffectsByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
    for (AuraEffectList::const_iterator i = auras.begin(); i != auras.end(); ++i)
    {
        if ((*i)->GetMiscValue() == script)
            if ((*i)->IsAffectedOnSpell(spell))
                return (*i);
    }
    return nullptr;
}

int32 Unit::GetTotalAuraModifier(AuraType auraType, std::function<bool(AuraEffect const*)> const& predicate) const
{
    AuraEffectList const& mTotalAuraList = GetAuraEffectsByType(auraType);
    if (mTotalAuraList.empty())
        return 0;

    std::map<SpellGroup, int32> sameEffectSpellGroup;
    int32 modifier = 0;

    for (AuraEffect const* aurEff : mTotalAuraList)
    {
        if (predicate(aurEff))
        {
            // Check if the Aura Effect has a the Same Effect Stack Rule and if so, use the highest amount of that SpellGroup
            // If the Aura Effect does not have this Stack Rule, it returns false so we can add to the multiplier as usual
            if (!sSpellMgr->AddSameEffectStackRuleSpellGroups(aurEff->GetSpellInfo(), static_cast<uint32>(auraType), aurEff->GetAmount(), sameEffectSpellGroup))
                modifier += aurEff->GetAmount();
        }
    }

    // Add the highest of the Same Effect Stack Rule SpellGroups to the accumulator
    for (auto itr = sameEffectSpellGroup.begin(); itr != sameEffectSpellGroup.end(); ++itr)
        modifier += itr->second;

    return modifier;
}

float Unit::GetTotalAuraMultiplier(AuraType auraType, std::function<bool(AuraEffect const*)> const& predicate) const
{
    AuraEffectList const& mTotalAuraList = GetAuraEffectsByType(auraType);
    if (mTotalAuraList.empty())
        return 1.0f;

    std::map<SpellGroup, int32> sameEffectSpellGroup;
    float multiplier = 1.0f;

    for (AuraEffect const* aurEff : mTotalAuraList)
    {
        if (predicate(aurEff))
        {
            // Check if the Aura Effect has a the Same Effect Stack Rule and if so, use the highest amount of that SpellGroup
            // If the Aura Effect does not have this Stack Rule, it returns false so we can add to the multiplier as usual
            if (!sSpellMgr->AddSameEffectStackRuleSpellGroups(aurEff->GetSpellInfo(), static_cast<uint32>(auraType), aurEff->GetAmount(), sameEffectSpellGroup))
                AddPct(multiplier, aurEff->GetAmount());
        }
    }

    // Add the highest of the Same Effect Stack Rule SpellGroups to the multiplier
    for (auto itr = sameEffectSpellGroup.begin(); itr != sameEffectSpellGroup.end(); ++itr)
        AddPct(multiplier, itr->second);

    return multiplier;
}

int32 Unit::GetMaxPositiveAuraModifier(AuraType auraType, std::function<bool(AuraEffect const*)> const& predicate) const
{
    AuraEffectList const& mTotalAuraList = GetAuraEffectsByType(auraType);
    if (mTotalAuraList.empty())
        return 0;

    int32 modifier = 0;
    for (AuraEffect const* aurEff : mTotalAuraList)
    {
        if (predicate(aurEff))
            modifier = std::max(modifier, aurEff->GetAmount());
    }

    return modifier;
}

int32 Unit::GetMaxNegativeAuraModifier(AuraType auraType, std::function<bool(AuraEffect const*)> const& predicate) const
{
    AuraEffectList const& mTotalAuraList = GetAuraEffectsByType(auraType);
    if (mTotalAuraList.empty())
        return 0;

    int32 modifier = 0;
    for (AuraEffect const* aurEff : mTotalAuraList)
    {
        if (predicate(aurEff))
            modifier = std::min(modifier, aurEff->GetAmount());
    }

    return modifier;
}

int32 Unit::GetTotalAuraModifier(AuraType auraType) const
{
    return GetTotalAuraModifier(auraType, [](AuraEffect const* /*aurEff*/) { return true; });
}

float Unit::GetTotalAuraMultiplier(AuraType auraType) const
{
    return GetTotalAuraMultiplier(auraType, [](AuraEffect const* /*aurEff*/) { return true; });
}

int32 Unit::GetMaxPositiveAuraModifier(AuraType auraType) const
{
    return GetMaxPositiveAuraModifier(auraType, [](AuraEffect const* /*aurEff*/) { return true; });
}

int32 Unit::GetMaxNegativeAuraModifier(AuraType auraType) const
{
    return GetMaxNegativeAuraModifier(auraType, [](AuraEffect const* /*aurEff*/) { return true; });
}

int32 Unit::GetTotalAuraModifierByMiscMask(AuraType auraType, uint32 miscMask) const
{
    return GetTotalAuraModifier(auraType, [miscMask](AuraEffect const* aurEff) -> bool
    {
        if ((aurEff->GetMiscValue() & miscMask) != 0)
            return true;
        return false;
    });
}

float Unit::GetTotalAuraMultiplierByMiscMask(AuraType auraType, uint32 miscMask) const
{
    return GetTotalAuraMultiplier(auraType, [miscMask](AuraEffect const* aurEff) -> bool
    {
        if ((aurEff->GetMiscValue() & miscMask) != 0)
            return true;
        return false;
    });
}

int32 Unit::GetMaxPositiveAuraModifierByMiscMask(AuraType auraType, uint32 miscMask, AuraEffect const* except /*= nullptr*/) const
{
    return GetMaxPositiveAuraModifier(auraType, [miscMask, except](AuraEffect const* aurEff) -> bool
    {
        if (except != aurEff && (aurEff->GetMiscValue() & miscMask) != 0)
            return true;
        return false;
    });
}

int32 Unit::GetMaxNegativeAuraModifierByMiscMask(AuraType auraType, uint32 miscMask) const
{
    return GetMaxNegativeAuraModifier(auraType, [miscMask](AuraEffect const* aurEff) -> bool
    {
        if ((aurEff->GetMiscValue() & miscMask) != 0)
            return true;
        return false;
    });
}

int32 Unit::GetTotalAuraModifierByMiscValue(AuraType auraType, int32 miscValue) const
{
    return GetTotalAuraModifier(auraType, [miscValue](AuraEffect const* aurEff) -> bool
    {
        if (aurEff->GetMiscValue() == miscValue)
            return true;
        return false;
    });
}

float Unit::GetTotalAuraMultiplierByMiscValue(AuraType auraType, int32 miscValue) const
{
    return GetTotalAuraMultiplier(auraType, [miscValue](AuraEffect const* aurEff) -> bool
    {
        if (aurEff->GetMiscValue() == miscValue)
            return true;
        return false;
    });
}

int32 Unit::GetMaxPositiveAuraModifierByMiscValue(AuraType auraType, int32 miscValue) const
{
    return GetMaxPositiveAuraModifier(auraType, [miscValue](AuraEffect const* aurEff) -> bool
    {
        if (aurEff->GetMiscValue() == miscValue)
            return true;
        return false;
    });
}

int32 Unit::GetMaxNegativeAuraModifierByMiscValue(AuraType auraType, int32 miscValue) const
{
    return GetMaxNegativeAuraModifier(auraType, [miscValue](AuraEffect const* aurEff) -> bool
    {
        if (aurEff->GetMiscValue() == miscValue)
            return true;
        return false;
    });
}

int32 Unit::GetTotalAuraModifierByAffectMask(AuraType auraType, SpellInfo const* affectedSpell) const
{
    return GetTotalAuraModifier(auraType, [affectedSpell](AuraEffect const* aurEff) -> bool
    {
        if (aurEff->IsAffectedOnSpell(affectedSpell))
            return true;
        return false;
    });
}

float Unit::GetTotalAuraMultiplierByAffectMask(AuraType auraType, SpellInfo const* affectedSpell) const
{
    return GetTotalAuraMultiplier(auraType, [affectedSpell](AuraEffect const* aurEff) -> bool
    {
        if (aurEff->IsAffectedOnSpell(affectedSpell))
            return true;
        return false;
    });
}

int32 Unit::GetMaxPositiveAuraModifierByAffectMask(AuraType auraType, SpellInfo const* affectedSpell) const
{
    return GetMaxPositiveAuraModifier(auraType, [affectedSpell](AuraEffect const* aurEff) -> bool
    {
        if (aurEff->IsAffectedOnSpell(affectedSpell))
            return true;
        return false;
    });
}

int32 Unit::GetMaxNegativeAuraModifierByAffectMask(AuraType auraType, SpellInfo const* affectedSpell) const
{
    return GetMaxNegativeAuraModifier(auraType, [affectedSpell](AuraEffect const* aurEff) -> bool
    {
        if (aurEff->IsAffectedOnSpell(affectedSpell))
            return true;
        return false;
    });
}

void Unit::RemoveSingleAuraFromStack(uint32 spellId)
{
    auto iter = m_appliedAuras.find(spellId);
    if (iter != m_appliedAuras.end())
    {
        if (iter->second->GetBase()->GetStackAmount() > 1)
        {
            iter->second->GetBase()->ModStackAmount(-1);
            return; // not remove aura if stack amount > 1
        }
        RemoveAura(iter);
    }
}

uint32 createProcHitMask(SpellNonMeleeDamage* damageInfo, SpellMissInfo missCondition)
{
    uint32 hitMask = PROC_HIT_NONE;
    // Check victim state
    if (missCondition != SPELL_MISS_NONE)
    {
        switch (missCondition)
        {
        case SPELL_MISS_MISS:
            hitMask |= PROC_HIT_MISS;
            break;
        case SPELL_MISS_DODGE:
            hitMask |= PROC_HIT_DODGE;
            break;
        case SPELL_MISS_PARRY:
            hitMask |= PROC_HIT_PARRY;
            break;
        case SPELL_MISS_BLOCK:
            // spells can't be partially blocked (it's damage can though)
            hitMask |= PROC_HIT_BLOCK | PROC_HIT_FULL_BLOCK;
            break;
        case SPELL_MISS_EVADE:
            hitMask |= PROC_HIT_EVADE;
            break;
        case SPELL_MISS_IMMUNE:
        case SPELL_MISS_IMMUNE2:
            hitMask |= PROC_HIT_IMMUNE;
            break;
        case SPELL_MISS_DEFLECT:
            hitMask |= PROC_HIT_DEFLECT;
            break;
        case SPELL_MISS_ABSORB:
            hitMask |= PROC_HIT_ABSORB;
            break;
        case SPELL_MISS_REFLECT:
            hitMask |= PROC_HIT_REFLECT;
            break;
        case SPELL_MISS_RESIST:
            hitMask |= PROC_HIT_FULL_RESIST;
            break;
        default:
            break;
        }
    }
    else
    {
        // On block
        if (damageInfo->blocked)
        {
            hitMask |= PROC_HIT_BLOCK;
            if (damageInfo->fullBlock)
                hitMask |= PROC_HIT_FULL_BLOCK;
        }
        // On absorb
        if (damageInfo->absorb)
            hitMask |= PROC_HIT_ABSORB;

        // Don't set hit/crit hitMask if damage is nullified
        bool const damageNullified =
#ifdef LICH_KING
            (damageInfo->HitInfo & (HITINFO_FULL_ABSORB | HITINFO_FULL_RESIST)) != 0 || 
#else
            damageInfo->fullAbsorb || damageInfo->fullResist ||
#endif
            (hitMask & PROC_HIT_FULL_BLOCK) != 0;
        if (!damageNullified)
        {
            // On crit
            if (damageInfo->HitInfo & SPELL_HIT_TYPE_CRIT)
                hitMask |= PROC_HIT_CRITICAL;
            else
                hitMask |= PROC_HIT_NORMAL;
        }
#ifdef LICH_KING
        else if ((damageInfo->HitInfo & HITINFO_FULL_RESIST) != 0)
#else
        else if (damageInfo->fullResist)
#endif
            hitMask |= PROC_HIT_FULL_RESIST;
    }

    return hitMask;
}

void Unit::ProcSkillsAndReactives(bool isVictim, Unit* procTarget, uint32 typeMask, uint32 hitMask, WeaponAttackType attType)
{
    // Player is loaded now - do not allow passive spell casts to proc
    if (GetTypeId() == TYPEID_PLAYER && ToPlayer()->GetSession()->PlayerLoading())
        return;

    // For melee/ranged based attack need update skills and set some Aura states if victim present
    if (typeMask & MELEE_BASED_TRIGGER_MASK && procTarget)
    {
        // Update skills here for players
        if (GetTypeId() == TYPEID_PLAYER)
        {
            // On melee based hit/miss/resist need update skill (for victim and attacker)
            if (hitMask & (PROC_HIT_NORMAL | PROC_HIT_MISS | PROC_HIT_FULL_RESIST))
            {
                if (procTarget->GetTypeId() != TYPEID_PLAYER && !procTarget->IsCritter())
                    ToPlayer()->UpdateCombatSkills(procTarget, attType, isVictim);
            }
            // Update defense if player is victim and parry/dodge/block
            else if (isVictim && (hitMask & (PROC_HIT_DODGE | PROC_HIT_PARRY | PROC_HIT_BLOCK)))
                ToPlayer()->UpdateCombatSkills(procTarget, attType, true);
        }
        // If exist crit/parry/dodge/block need update aura state (for victim and attacker)
        if (hitMask & (PROC_HIT_CRITICAL | PROC_HIT_PARRY | PROC_HIT_DODGE | PROC_HIT_BLOCK))
        {
            // for victim
            if (isVictim)
            {
                // if victim and dodge attack
                if (hitMask & PROC_HIT_DODGE)
                {
                    // Update AURA_STATE on dodge
                    if (GetClass() != CLASS_ROGUE) // skip Rogue Riposte
                    {
                        ModifyAuraState(AURA_STATE_DEFENSE, true);
                        StartReactiveTimer(REACTIVE_DEFENSE);
                    }
                }
                // if victim and parry attack
                if (hitMask & PROC_HIT_PARRY)
                {
                    // For Hunters only Counterattack (skip Mongoose bite)
                    if (GetClass() == CLASS_HUNTER)
                    {
                        ModifyAuraState(AURA_STATE_HUNTER_PARRY, true);
                        StartReactiveTimer(REACTIVE_HUNTER_PARRY);
                    }
                    else
                    {
                        ModifyAuraState(AURA_STATE_DEFENSE, true);
                        StartReactiveTimer(REACTIVE_DEFENSE);
                    }
                }
                // if and victim block attack
                if (hitMask & PROC_HIT_BLOCK)
                {
                    ModifyAuraState(AURA_STATE_DEFENSE, true);
                    StartReactiveTimer(REACTIVE_DEFENSE);
                }
            }
            else // For attacker
            {
                // Overpower on victim dodge
                if ((hitMask & PROC_HIT_DODGE) && GetTypeId() == TYPEID_PLAYER && GetClass() == CLASS_WARRIOR)
                {
                    ToPlayer()->AddComboPoints(procTarget, 1);
                    StartReactiveTimer(REACTIVE_OVERPOWER);
                }
                else if ((hitMask & PROC_HIT_CRITICAL))
                {
                    ModifyAuraState(AURA_STATE_CRIT, true);
                    StartReactiveTimer(REACTIVE_CRIT);
                    if (GetClass() == CLASS_HUNTER)
                    {
                        ModifyAuraState(AURA_STATE_HUNTER_CRIT_STRIKE, true);
                        StartReactiveTimer(REACTIVE_HUNTER_CRIT);
                    }
#ifdef LICH_KING
                    if (IsHunterPet())
                    {
                        AddComboPoints(procTarget, 1);
                        StartReactiveTimer(REACTIVE_WOLVERINE_BITE);
                    }
#endif
                }
            }
        }
    }
}

void Unit::GetProcAurasTriggeredOnEvent(AuraApplicationProcContainer& aurasTriggeringProc, AuraApplicationList* procAuras, ProcEventInfo& eventInfo)
{
    std::chrono::steady_clock::time_point now = GameTime::GetGameTimeSteadyPoint();

    // use provided list of auras which can proc
    if (procAuras)
    {
        for (AuraApplication* aurApp : *procAuras)
        {
            ASSERT(aurApp->GetTarget() == this);
            if (uint8 procEffectMask = aurApp->GetBase()->GetProcEffectMask(aurApp, eventInfo, now))
            {
                aurApp->GetBase()->PrepareProcToTrigger(aurApp, eventInfo, now);
                aurasTriggeringProc.emplace_back(procEffectMask, aurApp);
            }
        }
    }
    // or generate one on our own
    else
    {
        for (AuraApplicationMap::iterator itr = GetAppliedAuras().begin(); itr != GetAppliedAuras().end(); ++itr)
        {
            if (uint8 procEffectMask = itr->second->GetBase()->GetProcEffectMask(itr->second, eventInfo, now))
            {
                itr->second->GetBase()->PrepareProcToTrigger(itr->second, eventInfo, now);
                aurasTriggeringProc.emplace_back(procEffectMask, itr->second);
            }
        }
    }
}

void Unit::TriggerAurasProcOnEvent(Unit* actionTarget, uint32 typeMaskActor, uint32 typeMaskActionTarget, uint32 spellTypeMask, uint32 spellPhaseMask, uint32 hitMask, Spell* spell, DamageInfo* damageInfo, HealInfo* healInfo)
{
    // prepare data for self trigger
    ProcEventInfo myProcEventInfo(this, actionTarget, actionTarget, typeMaskActor, spellTypeMask, spellPhaseMask, hitMask, spell, damageInfo, healInfo);
    if (typeMaskActor)
    {
        AuraApplicationProcContainer myAurasTriggeringProc;
        GetProcAurasTriggeredOnEvent(myAurasTriggeringProc, nullptr, myProcEventInfo);

        // needed for example for Cobra Strikes, pet does the attack, but aura is on owner
        if (Player* modOwner = GetSpellModOwner())
        {
            if (modOwner != this && spell)
            {
                AuraApplicationList modAuras;
                for (auto itr = modOwner->GetAppliedAuras().begin(); itr != modOwner->GetAppliedAuras().end(); ++itr)
                {
                    if (spell->m_appliedMods.count(itr->second->GetBase()) != 0)
                        modAuras.push_back(itr->second);
                }
                modOwner->GetProcAurasTriggeredOnEvent(myAurasTriggeringProc, &modAuras, myProcEventInfo);
            }
        }
        TriggerAurasProcOnEvent(myProcEventInfo, myAurasTriggeringProc);
    }

    // prepare data for target trigger
    ProcEventInfo targetProcEventInfo(this, actionTarget, this, typeMaskActionTarget, spellTypeMask, spellPhaseMask, hitMask, spell, damageInfo, healInfo);
    if (typeMaskActionTarget && actionTarget)
    {
        AuraApplicationProcContainer targetAurasTriggeringProc;
        actionTarget->GetProcAurasTriggeredOnEvent(targetAurasTriggeringProc, nullptr, targetProcEventInfo);
        actionTarget->TriggerAurasProcOnEvent(targetProcEventInfo, targetAurasTriggeringProc);
    }
}

void Unit::TriggerAurasProcOnEvent(ProcEventInfo& eventInfo, AuraApplicationProcContainer& aurasTriggeringProc)
{
    Spell const* triggeringSpell = eventInfo.GetProcSpell();
    bool const disableProcs = triggeringSpell && triggeringSpell->IsProcDisabled();
    if (disableProcs)
        SetCantProc(true);

    for (auto const& aurAppProc : aurasTriggeringProc)
    {
        AuraApplication* aurApp;
        uint8 procEffectMask;
        std::tie(procEffectMask, aurApp) = aurAppProc;

        if (aurApp->GetRemoveMode())
            continue;

        SpellInfo const* spellInfo = aurApp->GetBase()->GetSpellInfo();
        if (spellInfo->HasAttribute(SPELL_ATTR3_DISABLE_PROC))
            SetCantProc(true);

        aurApp->GetBase()->TriggerProcOnEvent(procEffectMask, aurApp, eventInfo);

        if (spellInfo->HasAttribute(SPELL_ATTR3_DISABLE_PROC))
            SetCantProc(false);
    }

    if (disableProcs)
        SetCantProc(false);
}

void Unit::FeignDeathDetected(Unit const* by)
{
    m_feignDeathDetectedBy.emplace(by->GetGUID());
}

bool Unit::IsFeighDeathDetected(Unit const* by) const
{
    return m_feignDeathDetectedBy.find(by->GetGUID()) != m_feignDeathDetectedBy.end();
}

void Unit::ResetFeignDeathDetected()
{
    m_feignDeathDetectedBy.clear();
}
