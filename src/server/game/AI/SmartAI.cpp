#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "ObjectDefines.h"
#include "GridDefines.h"
#include "GridNotifiers.h"
#include "SpellMgr.h"
#include "GridNotifiersImpl.h"
#include "Cell.h"
#include "CellImpl.h"
#include "Unit.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "WaypointManager.h"

#include "SmartAI.h"

SmartAI::SmartAI(Creature* c) : CreatureAI(c), mEscortQuestID(0), mIsCharmed(false), 
    _escortState(SMART_ESCORT_NONE), _escortNPCFlags(0), _escortInvokerCheckTimer(1000), _currentWaypointNode(0), _waypointReached(false), _waypointPauseTimer(0), _waypointPauseForced(false), _repeatWaypointPath(false),
    _OOCReached(false), _waypointPathEnded(false), mRun(true), mEvadeDisabled(false), mCanAutoAttack(true), mCanCombatMove(true), mInvincibilityHpLevel(0), mDespawnTime(0), mDespawnState(0), _gossipReturn(false)
{
    // spawn in run mode
    // me->SetWalk(false); //disabled this, movement generator may have altered this already

    mEscortInvokerCheckTimer = 1000;
    mFollowGuid = 0;
    mFollowDist = 0;
    mFollowAngle = 0;
    mFollowCredit = 0;
    mFollowArrivedEntry = 0;
    mFollowCreditType = 0;
    mFollowArrivedTimer = 0;
}

bool SmartAI::IsAIControlled() const
{
    if (me->IsControlledByPlayer())
        return false;
    if (mIsCharmed)
         return false;
    return true;
}

void SmartAI::UpdateDespawn(const uint32 diff)
{
    if (mDespawnState <= 1 || mDespawnState > 3)
        return;

    if (mDespawnTime < diff)
    {
        if (mDespawnState == 2)
        {
            me->SetVisible(false);
            mDespawnTime = 1000;
            mDespawnState++;
        }
        else
            me->DespawnOrUnsummon();
    } else mDespawnTime -= diff;
}

void SmartAI::UpdateFollow(uint32 diff)
{
    if (mFollowGuid)
    {
        if (mFollowArrivedTimer < diff)
        {
            if (me->FindNearestCreature(mFollowArrivedEntry, INTERACTION_DISTANCE, true))
            {
                StopFollow(true);
                return;
            }

            mFollowArrivedTimer = 1000;
        }
        else
            mFollowArrivedTimer -= diff;
    }
}

void SmartAI::StartPath(bool run, uint32 path, bool repeat, Unit* invoker, uint32 nodeId/* = 1*/)
{
    if (me->IsInCombat())// no wp movement in combat
    {
        TC_LOG_ERROR("FIXME","SmartAI::StartPath: Creature entry %u wanted to start waypoint movement while in combat, ignoring.", me->GetEntry());
        return;
    }
    if (HasEscortState(SMART_ESCORT_ESCORTING))
        StopPath();

    SetRun(run);

    if (path)
    {
        if (!LoadPath(path))
            return;
    }

    if (_path.nodes.empty())
        return;

    _currentWaypointNode = nodeId;
    _waypointPathEnded = false;

    _repeatWaypointPath = repeat;

    // Do not use AddEscortState, removing everything from previous
    _escortState = SMART_ESCORT_ESCORTING;

    if (invoker && invoker->GetTypeId() == TYPEID_PLAYER)
    {
        _escortNPCFlags = me->GetUInt32Value(UNIT_NPC_FLAGS);
        me->SetFlag(UNIT_NPC_FLAGS, 0);
    }

    GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_START, nullptr, _currentWaypointNode, GetScript()->GetPathId());

    me->GetMotionMaster()->MovePath(_path, _repeatWaypointPath);
}

bool SmartAI::LoadPath(uint32 entry)
{
    if (HasEscortState(SMART_ESCORT_ESCORTING))
        return false;

    auto path = sSmartWaypointMgr->GetPath(entry);
    if (!path || path->nodes.empty())
    {
        GetScript()->SetPathId(0);
        return false;
    }

    _path.id = path->id;
    _path.nodes = path->nodes;
    for (WaypointNode& waypoint : _path.nodes)
    {
        Trinity::NormalizeMapCoord(waypoint.x);
        Trinity::NormalizeMapCoord(waypoint.y);
        waypoint.moveType = mRun ? WAYPOINT_MOVE_TYPE_RUN : WAYPOINT_MOVE_TYPE_WALK;
    }

    GetScript()->SetPathId(entry);
    return true;
}

void SmartAI::PausePath(uint32 delay, bool forced)
{
    if (!HasEscortState(SMART_ESCORT_ESCORTING))
    {
        me->PauseMovement(delay, MOTION_SLOT_IDLE, forced);
        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
        {
            std::pair<uint32, uint32> waypointInfo = me->GetCurrentWaypointInfo();
            GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_PAUSED, nullptr, waypointInfo.first, waypointInfo.second);
        }
        return;
    }

    if (HasEscortState(SMART_ESCORT_PAUSED))
    {
        TC_LOG_ERROR("misc", "SmartAI::PausePath: Creature entry %u wanted to pause waypoint movement while already paused, ignoring.", me->GetEntry());
        return;
    }

    _waypointPauseTimer = delay;

    if (forced)
    {
        _waypointPauseForced = forced;
        SetRun(mRun);
        me->PauseMovement();
        me->SetHomePosition(me->GetPosition());
    }
    else
        _waypointReached = false;

    AddEscortState(SMART_ESCORT_PAUSED);
    GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_PAUSED, nullptr, _currentWaypointNode, GetScript()->GetPathId());
}

void SmartAI::StopPath(uint32 DespawnTime, uint32 quest, bool fail)
{
    if (!HasEscortState(SMART_ESCORT_ESCORTING))
    {
        std::pair<uint32, uint32> waypointInfo = { 0, 0 };
        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
            waypointInfo = me->GetCurrentWaypointInfo();

        if (mDespawnState != 2)
            SetDespawnTime(DespawnTime);

        me->GetMotionMaster()->MoveIdle();

        if (waypointInfo.first)
            GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_STOPPED, nullptr, waypointInfo.first, waypointInfo.second);

        if (!fail)
        {
            if (waypointInfo.first)
                GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_ENDED, nullptr, waypointInfo.first, waypointInfo.second);
            if (mDespawnState == 1)
                StartDespawn();
        }
        return;
    }

    if (quest)
        mEscortQuestID = quest;

    if (mDespawnState != 2)
        SetDespawnTime(DespawnTime);

    me->GetMotionMaster()->MoveIdle();

    GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_STOPPED, nullptr, _currentWaypointNode, GetScript()->GetPathId());

    EndPath(fail);
}

void SmartAI::EndPath(bool fail)
{
    RemoveEscortState(SMART_ESCORT_ESCORTING | SMART_ESCORT_PAUSED | SMART_ESCORT_RETURNING);
    _path.nodes.clear();
    _waypointPauseTimer = 0;

    if (_escortNPCFlags)
    {
        me->SetFlag(UNIT_NPC_FLAGS, _escortNPCFlags);
        _escortNPCFlags = 0;
    }

    ObjectVector const* targets = GetScript()->GetStoredTargetVector(SMART_ESCORT_TARGETS, *me);
    if (targets && mEscortQuestID)
    {
        if (targets->size() == 1 && GetScript()->IsPlayer((*targets->begin())))
        {
            Player* player = targets->front()->ToPlayer();
            if (!fail && player->IsAtGroupRewardDistance(me) && !player->HasCorpse())
                player->GroupEventHappens(mEscortQuestID, me);

            if (fail)
                player->FailQuest(mEscortQuestID);

            if (Group* group = player->GetGroup())
            {
                for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
                {
                    Player* groupGuy = groupRef->GetSource();
                    if (!groupGuy->IsInMap(player))
                        continue;

                    if (!fail && groupGuy->IsAtGroupRewardDistance(me) && !groupGuy->HasCorpse())
                        groupGuy->AreaExploredOrEventHappens(mEscortQuestID);
                    else if (fail)
                        groupGuy->FailQuest(mEscortQuestID);
                }
            }
        }
        else
        {
            for (WorldObject* target : *targets)
            {
                if (GetScript()->IsPlayer(target))
                {
                    Player* player = target->ToPlayer();
                    if (!fail && player->IsAtGroupRewardDistance(me) && !player->HasCorpse())
                        player->AreaExploredOrEventHappens(mEscortQuestID);
                    else if (fail)
                        player->FailQuest(mEscortQuestID);
                }
            }
        }
    }

    // End Path events should be only processed if it was SUCCESSFUL stop or stop called by SMART_ACTION_WAYPOINT_STOP
    if (fail)
        return;

    GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_ENDED, nullptr, _currentWaypointNode, GetScript()->GetPathId());

    if (_repeatWaypointPath)
    {
        if (IsAIControlled())
            StartPath(mRun, GetScript()->GetPathId(), _repeatWaypointPath);
    }
    else
        GetScript()->SetPathId(0);

    if (mDespawnState == 1)
        StartDespawn();
}

void SmartAI::ResumePath()
{
    GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_RESUMED, nullptr, _currentWaypointNode, GetScript()->GetPathId());

    RemoveEscortState(SMART_ESCORT_PAUSED);

    _waypointPauseForced = false;
    _waypointReached = false;
    _waypointPauseTimer = 0;

    SetRun(mRun);
    me->ResumeMovement();
}

void SmartAI::ReturnToLastOOCPos()
{
    if (!IsAIControlled())
        return;

    me->SetWalk(false);
    me->GetMotionMaster()->MovePoint(SMART_ESCORT_LAST_OOC_POINT, me->GetHomePosition());
}

void SmartAI::MovePointInPath(bool run, uint32 id, float x, float y, float z, float o)
{
    me->GetMotionMaster()->MovePoint(id, x, y, z);

    if (me->GetFormation() && me->GetFormation()->getLeader() == me)
        me->GetFormation()->LeaderMoveTo(x, y, z, run);
}

bool SmartAI::IsEscortInvokerInRange()
{
    if (ObjectVector const* targets = GetScript()->GetStoredTargetVector(SMART_ESCORT_TARGETS, *me))
    {
        float checkDist = me->GetInstanceScript() ? SMART_ESCORT_MAX_PLAYER_DIST * 2 : SMART_ESCORT_MAX_PLAYER_DIST;
        if (targets->size() == 1 && GetScript()->IsPlayer((*targets->begin())))
        {
            Player* player = (*targets->begin())->ToPlayer();
            if (me->GetDistance(player) <= checkDist)
                return true;

            if (Group* group = player->GetGroup())
            {
                for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
                {
                    Player* groupGuy = groupRef->GetSource();
                    if (groupGuy->IsInMap(player) && me->GetDistance(groupGuy) <= checkDist)
                        return true;
                }
            }
        }
        else
        {
            for (WorldObject* target : *targets)
            {
                if (GetScript()->IsPlayer(target))
                {
                    if (me->GetDistance(target->ToPlayer()) <= checkDist)
                        return true;
                }
            }
        }

        // no valid target found
        return false;
    }

    // no player invoker was stored, just ignore range check
    return true;
}

void SmartAI::UpdatePath(const uint32 diff)
{
    if (!HasEscortState(SMART_ESCORT_ESCORTING))
        return;

    if (_escortInvokerCheckTimer < diff)
    {
        if (!IsEscortInvokerInRange())
        {
            StopPath(0, mEscortQuestID, true);

            // allow to properly hook out of range despawn action, which in most cases should perform the same operation as dying
            GetScript()->ProcessEventsFor(SMART_EVENT_DEATH, me);
            me->DespawnOrUnsummon();
            return;
        }
        _escortInvokerCheckTimer = 1000;
    }
    else
        _escortInvokerCheckTimer -= diff;

    // handle pause
    if (HasEscortState(SMART_ESCORT_PAUSED) && (_waypointReached || _waypointPauseForced))
    {
        if (_waypointPauseTimer <= diff)
        {
            if (!me->IsInCombat() && !HasEscortState(SMART_ESCORT_RETURNING))
                ResumePath();
        }
        else
            _waypointPauseTimer -= diff;
    }
    else if (_waypointPathEnded) // end path
    {
        _waypointPathEnded = false;
        StopPath();
        return;
    }

    if (HasEscortState(SMART_ESCORT_RETURNING))
    {
        if (_OOCReached) // reached OOC WP
        {
            _OOCReached = false;
            RemoveEscortState(SMART_ESCORT_RETURNING);
            if (!HasEscortState(SMART_ESCORT_PAUSED))
                ResumePath();
        }
    }
}

void SmartAI::UpdateAI(const uint32 diff)
{
    GetScript()->OnUpdate(diff);
    UpdatePath(diff);
    UpdateFollow(diff);
    UpdateDespawn(diff);
    
    if (!IsAIControlled())
        return;

    if (!UpdateVictim())
        return;

    if (mCanAutoAttack)
        DoMeleeAttackIfReady();
}

/*
void SmartAI::MovepointReached(uint32 id)
{
    if (id != SMART_ESCORT_LAST_OOC_POINT && mLastWPIDReached != id)
        GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_REACHED, nullptr, id);

    mLastWPIDReached = id;
    mWPReached = true;
}
*/

void SmartAI::MovementInform(uint32 MovementType, uint32 id)
{
    if ((MovementType == POINT_MOTION_TYPE && id == SMART_ESCORT_LAST_OOC_POINT) || MovementType == FOLLOW_MOTION_TYPE)
        me->ClearUnitState(UNIT_STATE_EVADE);

    GetScript()->ProcessEventsFor(SMART_EVENT_MOVEMENTINFORM, nullptr, MovementType, id);

    if (MovementType == POINT_MOTION_TYPE && id == SMART_ESCORT_LAST_OOC_POINT)
        _OOCReached = true;
}

///@todo move escort related logic
void SmartAI::WaypointPathStarted(uint32 nodeId, uint32 pathId)
{
    if (!HasEscortState(SMART_ESCORT_ESCORTING))
    {
        GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_START, nullptr, nodeId, pathId);
        return;
    }
}

///@todo Implement new smart event SMART_EVENT_WAYPOINT_STARTED
void SmartAI::WaypointStarted(uint32 /*nodeId*/, uint32 /*pathId*/)
{
}

void SmartAI::WaypointReached(uint32 nodeId, uint32 pathId)
{
    if (!HasEscortState(SMART_ESCORT_ESCORTING))
    {
        GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_REACHED, nullptr, nodeId, pathId);
        return;
    }

    _currentWaypointNode = nodeId;

    GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_REACHED, nullptr, _currentWaypointNode, pathId);

    if (_waypointPauseTimer && !_waypointPauseForced)
    {
        _waypointReached = true;
        me->PauseMovement();
        me->SetHomePosition(me->GetPosition());
    }
    else if (HasEscortState(SMART_ESCORT_ESCORTING) && me->GetMotionMaster()->GetCurrentMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
    {
        if (_currentWaypointNode == _path.nodes.size())
            _waypointPathEnded = true;
        else
            SetRun(mRun);
    }
}

///@todo move escort related logic
void SmartAI::WaypointPathEnded(uint32 nodeId, uint32 pathId)
{
    if (!HasEscortState(SMART_ESCORT_ESCORTING))
    {
        GetScript()->ProcessEventsFor(SMART_EVENT_WAYPOINT_ENDED, nullptr, nodeId, pathId);
        return;
    }
}


void SmartAI::RemoveAuras()
{
    Unit::AuraMap& auras = me->GetAuras();
    for(auto itr = auras.begin(); itr != auras.end();)
    {
        if (!itr->second->IsPassive() && itr->second->GetCasterGUID() != me->GetGUID())
            me->RemoveAura(itr);
        else
            itr++;
    }
    /*
    /// @fixme: duplicated logic in CreatureAI::_EnterEvadeMode (could use RemoveAllAurasExceptType)
    Unit::AuraApplicationMap& appliedAuras = me->GetAppliedAuras();
    for (Unit::AuraApplicationMap::iterator iter = appliedAuras.begin(); iter != appliedAuras.end();)
    {
        Aura const* aura = iter->second->GetBase();
        if (!aura->IsPassive() && !aura->HasEffectType(SPELL_AURA_CLONE_CASTER) && aura->GetCasterGUID() != me->GetGUID())
            me->RemoveAura(iter);
        else
            ++iter;
    }
    */
}

void SmartAI::EnterEvadeMode(EvadeReason why)
{
    if (mEvadeDisabled)
    {
        GetScript()->ProcessEventsFor(SMART_EVENT_EVADE);
        return;
    }

    if (!_EnterEvadeMode())
        return;

    GetScript()->ProcessEventsFor(SMART_EVENT_EVADE);//must be after aura clear so we can cast spells from db

    SetRun(mRun);
    
    if (HasEscortState(SMART_ESCORT_ESCORTING))
    {
        AddEscortState(SMART_ESCORT_RETURNING);
        ReturnToLastOOCPos();
    }
    else if (mFollowGuid)
    {
        if (Unit* target = ObjectAccessor::GetUnit(*me, mFollowGuid))
            me->GetMotionMaster()->MoveFollow(target, mFollowDist, mFollowAngle);
    }
    else if (!me->IsHomeless()) {
        me->GetMotionMaster()->MoveTargetedHome();
    }

    if (!HasEscortState(SMART_ESCORT_ESCORTING))//dont mess up escort movement after combat
        SetRun(mRun);
}

void SmartAI::MoveInLineOfSight(Unit* who)
{
    if (!who)
        return;

    GetScript()->OnMoveInLineOfSight(who);

    if (!IsAIControlled())
        return;

    CreatureAI::MoveInLineOfSight(who);
}

void SmartAI::JustAppeared()
{
    mDespawnTime = 0;
    mDespawnState = 0;
    _escortState = SMART_ESCORT_NONE;

    me->SetVisible(true);

    if (me->GetFaction() != me->GetCreatureTemplate()->faction)
        me->RestoreFaction();

    GetScript()->OnReset();
    GetScript()->ProcessEventsFor(SMART_EVENT_RESPAWN);

    mFollowGuid = 0;//do not reset follower on Reset(), we need it after combat evade
    mFollowDist = 0;
    mFollowAngle = 0;
    mFollowCredit = 0;
    mFollowArrivedTimer = 1000;
    mFollowArrivedEntry = 0;
    mFollowCreditType = 0;
}

int SmartAI::Permissible(const Creature* creature)
{
    if (creature->GetAIName() == SMARTAI_AI_NAME)
        return PERMIT_BASE_SPECIAL;
    return PERMIT_BASE_NO;
}

void SmartAI::JustReachedHome()
{
    GetScript()->OnReset();

    GetScript()->ProcessEventsFor(SMART_EVENT_REACHED_HOME);

    if (me->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_IDLE) != WAYPOINT_MOTION_TYPE && me->GetWaypointPath())
        me->GetMotionMaster()->MovePath(me->GetWaypointPath(), true);
    else
        me->ResumeMovement();
}

void SmartAI::EnterCombat(Unit* enemy)
{
    if (IsAIControlled())
        me->InterruptNonMeleeSpells(false); // must be before ProcessEvents

    GetScript()->ProcessEventsFor(SMART_EVENT_AGGRO, enemy);
}

void SmartAI::JustDied(Unit* killer)
{
    if (HasEscortState(SMART_ESCORT_ESCORTING))
        EndPath(true);

    GetScript()->ProcessEventsFor(SMART_EVENT_DEATH, killer);
}

void SmartAI::KilledUnit(Unit* victim)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_KILL, victim);
}

void SmartAI::VictimDied(Unit* attacked)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_VICTIM_DIED, attacked);
}

void SmartAI::JustSummoned(Creature* creature)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_SUMMONED_UNIT, creature);
}

void SmartAI::AttackStart(Unit* who)
{
    // sunwell: dont allow charmed npcs to act on their own
    if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
    {
        if (who && mCanAutoAttack)
            me->Attack(who, true);
        return;
    }

    if (who && me->Attack(who, me->IsWithinMeleeRange(who)))
    {
        if (mCanCombatMove)
        {
            SetRun(mRun);
            me->GetMotionMaster()->MoveChase(who);
        }
    }
}

void SmartAI::SpellHit(Unit* unit, const SpellInfo* spellInfo)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_SPELLHIT, unit, 0, 0, false, spellInfo);
}

void SmartAI::SpellHitTarget(Unit* target, const SpellInfo* spellInfo)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_SPELLHIT_TARGET, target, 0, 0, false, spellInfo);
}

void SmartAI::DamageTaken(Unit* doneBy, uint32& damage)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DAMAGED, doneBy, damage);

    // don't allow players to use unkillable units
    if (IsAIControlled() && mInvincibilityHpLevel && (damage >= me->GetHealth() - mInvincibilityHpLevel))
    {
        damage = 0;
        me->SetHealth(mInvincibilityHpLevel);
    }
}

void SmartAI::HealReceived(Unit* doneBy, uint32& addhealth)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_RECEIVE_HEAL, doneBy, addhealth);
}

void SmartAI::ReceiveEmote(Player* player, uint32 textEmote)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_RECEIVE_EMOTE, player, textEmote);
}

void SmartAI::IsSummonedBy(Unit* summoner)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_JUST_SUMMONED, summoner);
}

void SmartAI::DamageDealt(Unit* doneTo, uint32& damage, DamageEffectType /*damagetype*/)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DAMAGED_TARGET, doneTo, damage);
}

void SmartAI::SummonedCreatureDespawn(Creature* unit)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_SUMMON_DESPAWNED, unit);
}

void SmartAI::CorpseRemoved(uint32& respawnDelay)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_CORPSE_REMOVED, nullptr, respawnDelay);
}

void SmartAI::PassengerBoarded(Unit* who, int8 seatId, bool apply)
{
#ifndef LICH_KING
    TC_LOG_ERROR("misc","SmartAI::PassengerBoarded was called while core isn't compiled for LK");
#endif
    GetScript()->ProcessEventsFor(apply ? SMART_EVENT_PASSENGER_BOARDED : SMART_EVENT_PASSENGER_REMOVED, who, uint32(seatId), 0, apply);
}

void SmartAI::InitializeAI()
{
    GetScript()->OnInitialize(me);

    JustReachedHome();
}

void SmartAI::OnCharmed(Unit* charmer, bool apply)
{
    if (apply) // do this before we change charmed state, as charmed state might prevent these things from processing
    {
        if (HasEscortState(SMART_ESCORT_ESCORTING | SMART_ESCORT_PAUSED | SMART_ESCORT_RETURNING))
            EndPath(true);
        me->StopMoving();
    }
    mIsCharmed = apply;

    me->NeedChangeAI = true;

    if (!apply && !me->IsInEvadeMode())
    {
        if (_repeatWaypointPath)
            StartPath(mRun, GetScript()->GetPathId(), true);
        else
            me->SetWalk(!mRun);

        if (charmer)
            AttackStart(charmer);
    }

    GetScript()->ProcessEventsFor(SMART_EVENT_CHARMED, nullptr, 0, 0, apply);
}

bool SmartAI::CanAIAttack(Unit const* /*who*/) const
{
    return !(me->HasReactState(REACT_PASSIVE));
}

void SmartAI::DoAction(int32 param)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_ACTION_DONE, nullptr, param);
}

uint32 SmartAI::GetData(uint32 /*id*/) const
{
    return 0;
}

void SmartAI::SetData(uint32 id, uint32 value, Unit* setter)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DATA_SET, setter, id, value);
}

void SmartAI::SetGUID(uint64 /*guid*/, int32 /*id*/) { }

uint64 SmartAI::GetGUID(int32 /*id*/) const
{
    return 0;
}

void SmartAI::SetRun(bool run)
{
    me->SetWalk(!run);
    mRun = run;
}

void SmartAI::SetDisableGravity(bool fly)
{
    me->SetDisableGravity(fly);
}

void SmartAI::SetCanFly(bool fly)
{
    me->SetCanFly(fly);
}

void SmartAI::SetSwim(bool swim)
{
    if (swim)
        me->AddUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
    else
        me->RemoveUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
}

void SmartAI::SetEvadeDisabled(bool disable)
{
    mEvadeDisabled = disable;
}

bool SmartAI::GossipHello(Player* player)
{
    _gossipReturn = false;
    GetScript()->ProcessEventsFor(SMART_EVENT_GOSSIP_HELLO, player);
    return _gossipReturn;
}

bool SmartAI::GossipSelect(Player* player, uint32 sender, uint32 action)
{
    _gossipReturn = false;
    GetScript()->ProcessEventsFor(SMART_EVENT_GOSSIP_SELECT, player, sender, action);
    return _gossipReturn;
}

bool SmartAI::GossipSelectCode(Player* /*player*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/) 
{ 
    return false;
}

void SmartAI::QuestAccept(Player* player, Quest const* quest)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_ACCEPTED_QUEST, player, quest->GetQuestId());
}

void SmartAI::QuestReward(Player* player, Quest const* quest, uint32 opt)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_REWARD_QUEST, player, quest->GetQuestId(), opt);
}

void SmartAI::SetCombatMove(bool on)
{
    if (mCanCombatMove == on)
        return;
    mCanCombatMove = on;
    if (!IsAIControlled())
        return;

    if (!HasEscortState(SMART_ESCORT_ESCORTING))
    {
        if (on && me->GetVictim())
        {
            if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == IDLE_MOTION_TYPE)
            {
                SetRun(mRun);
                me->GetMotionMaster()->MoveChase(me->GetVictim());
                me->CastStop();
            }
        }
        else if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != WAYPOINT_MOTION_TYPE)
        {
            if (me->HasUnitState(UNIT_STATE_CONFUSED_MOVE | UNIT_STATE_FLEEING_MOVE))
                return;

            me->GetMotionMaster()->MovementExpired();
            me->GetMotionMaster()->Clear(true);
            me->StopMoving();
            me->GetMotionMaster()->MoveIdle();
        }
    }
}

void SmartAI::SetFollow(Unit* target, float dist, float angle, uint32 credit, uint32 end, uint32 creditType)
{
    if (!target)
    {
        StopFollow(false);
        return;
    }

    mFollowGuid = target->GetGUID();
    mFollowDist = dist >= 0.0f ? dist : PET_FOLLOW_DIST;
    mFollowAngle = angle >= 0.0f ? angle : me->GetFollowAngle();
    mFollowArrivedTimer = 1000;
    mFollowCredit = credit;
    mFollowArrivedEntry = end;
    mFollowCreditType = creditType;
    SetRun(mRun);
    me->GetMotionMaster()->MoveFollow(target, mFollowDist, mFollowAngle);
}

void SmartAI::StopFollow(bool complete)
{
    mFollowGuid = 0;
    mFollowDist = 0;
    mFollowAngle = 0;
    mFollowCredit = 0;
    mFollowArrivedTimer = 1000;
    mFollowArrivedEntry = 0;
    mFollowCreditType = 0;
    SetDespawnTime(5000);
    me->StopMoving();
    me->GetMotionMaster()->MoveIdle();

    if (!complete)
        return;

    if (Player* player = ObjectAccessor::GetPlayer(*me, mFollowGuid))
    {
        if (!mFollowCreditType)
            player->RewardPlayerAndGroupAtEvent(mFollowCredit, me);
        else
            player->GroupEventHappens(mFollowCredit, me);
    }

    StartDespawn();
    GetScript()->ProcessEventsFor(SMART_EVENT_FOLLOW_COMPLETED);
}

void SmartAI::SetScript9(SmartScriptHolder& e, uint32 entry, Unit* invoker)
{
    if (invoker)
        GetScript()->mLastInvoker = invoker->GetGUID();
    GetScript()->SetScript9(e, entry);
}

/*
void SmartAI::sOnGameEvent(bool start, uint16 eventId)
{
    GetScript()->ProcessEventsFor(start ? SMART_EVENT_GAME_EVENT_START : SMART_EVENT_GAME_EVENT_END, NULL, eventId);
}*/

void SmartAI::OnSpellClick(Unit* clicker, bool& result)
{
    if (!result)
        return;

    GetScript()->ProcessEventsFor(SMART_EVENT_ON_SPELLCLICK, clicker);
}

void SmartAI::FriendlyKilled(Creature const* c, float range)
{
    GetScript()->ProcessEventsFor( SMART_EVENT_FRIENDLY_KILLED, (Unit*)c );
}

int SmartGameObjectAI::Permissible(const GameObject* g)
{
    if (g->GetAIName() == SMARTAI_GOBJECT_AI_NAME)
        return PERMIT_BASE_SPECIAL;

    return PERMIT_BASE_NO;
}

void SmartGameObjectAI::UpdateAI(uint32 diff)
{
    GetScript()->OnUpdate(diff);
}

void SmartGameObjectAI::InitializeAI()
{
    GetScript()->OnInitialize(me);
    //Reset();
}

void SmartGameObjectAI::Reset()
{
    GetScript()->OnReset();
}

// Called when a player opens a gossip dialog with the gameobject.
bool SmartGameObjectAI::GossipHello(Player* player)
{
    TC_LOG_DEBUG("scripts.ai","SmartGameObjectAI::GossipHello");
    _gossipReturn = false;
    GetScript()->ProcessEventsFor(SMART_EVENT_GOSSIP_HELLO, player, 0, 0, false, nullptr, me);
    return _gossipReturn;
}

bool SmartGameObjectAI::OnReportUse(Player* player)
{
    TC_LOG_DEBUG("scripts.ai", "SmartGameObjectAI::OnReportUse");
    _gossipReturn = false;
    GetScript()->ProcessEventsFor(SMART_EVENT_GOSSIP_HELLO, player, 1, 0, false, nullptr, me);
    return _gossipReturn;
}

// Called when a player selects a gossip item in the gameobject's gossip menu.
bool SmartGameObjectAI::GossipSelect(Player* player, uint32 sender, uint32 action)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_GOSSIP_SELECT, player, sender, action, false, nullptr, me);
    return false;
}

// Called when a player selects a gossip with a code in the gameobject's gossip menu.
bool SmartGameObjectAI::GossipSelectCode(Player* /*player*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/)
{
    return false;
}

// Called when a player accepts a quest from the gameobject.
void SmartGameObjectAI::QuestAccept(Player* player, Quest const* quest)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_ACCEPTED_QUEST, player, quest->GetQuestId(), 0, false, nullptr, me);
}

// Called when a player selects a quest reward.
void SmartGameObjectAI::QuestReward(Player* player, Quest const* quest, uint32 opt)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_REWARD_QUEST, player, quest->GetQuestId(), opt, false, nullptr, me);
}

// Called when the gameobject is destroyed (destructible buildings only).
void SmartGameObjectAI::Destroyed(Player* player, uint32 eventId)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DEATH, player, eventId, 0, false, nullptr, me);
}

void SmartGameObjectAI::SetData(uint32 id, uint32 value, Unit* setter)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_DATA_SET, setter, id, value);
}

void SmartGameObjectAI::SetScript9(SmartScriptHolder& e, uint32 entry, Unit* invoker)
{
    if (invoker)
        GetScript()->mLastInvoker = invoker->GetGUID();
    GetScript()->SetScript9(e, entry);
}

void SmartGameObjectAI::OnGameEvent(bool start, uint16 eventId)
{
    GetScript()->ProcessEventsFor(start ? SMART_EVENT_GAME_EVENT_START : SMART_EVENT_GAME_EVENT_END, nullptr, eventId);
}

void SmartGameObjectAI::OnStateChanged(GOState state, Unit* unit)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_GO_STATE_CHANGED, unit, state);
}

void SmartGameObjectAI::OnLootStateChanged(LootState state, Unit* unit)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_GO_LOOT_STATE_CHANGED, unit, state);
}

void SmartGameObjectAI::EventInform(uint32 eventId)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_GO_EVENT_INFORM, nullptr, eventId);
}

void SmartGameObjectAI::SpellHit(Unit* unit, const SpellInfo* spellInfo)
{
    GetScript()->ProcessEventsFor(SMART_EVENT_SPELLHIT, unit, 0, 0, false, spellInfo);
}

class SmartTrigger : public AreaTriggerScript
{
    public:

        SmartTrigger() : AreaTriggerScript("SmartTrigger") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* trigger) override
        {
            if (!player->IsAlive())
                return false;

            TC_LOG_DEBUG("scripts.ai", "AreaTrigger %u is using SmartTrigger script", trigger->id);
            SmartScript script;
            script.OnInitialize(nullptr, trigger);
            script.ProcessEventsFor(SMART_EVENT_AREATRIGGER_ONTRIGGER, player, trigger->id);
            return true;
        }
};

void AddSC_SmartScripts()
{
    new SmartTrigger();
}