#include "EscortAI.h"
#include "MovementGenerator.h"

enum Points
{
    POINT_LAST_POINT    = 0xFFFFFF,
    POINT_HOME          = 0xFFFFFE
};

EscortAI::EscortAI(Creature *c) : ScriptedAI(c),
_playerCheckTimer(1000), _maxPlayerDistance(DEFAULT_MAX_PLAYER_DISTANCE), _pauseTimer(2500),
_despawnAtEnd(true), _despawnAtFar(true), _escortState(STATE_ESCORT_NONE), _escortQuest(nullptr),
_hasImmuneToNPCFlags(false), _started(false), _ended(false), _resume(false), _manualPath(false),
_running(false), _instantRespawn(false), _returnToStart(false), _activeAttacker(true)
{
}

void EscortAI::MoveInLineOfSight(Unit *who)
{
    if (HasEscortState(STATE_ESCORT_ESCORTING) && AssistPlayerInCombatAgainst(who))
        return;

    if (HasEscortState(STATE_ESCORT_ESCORTING) && !_activeAttacker)
        return;

    ScriptedAI::MoveInLineOfSight(who);
}

void EscortAI::FailQuest()
{
    if (!_escortQuest)
        return;

    if (Player* player = GetPlayerForEscort())
    {
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
            {
                if (Player* member = groupRef->GetSource())
                    if (member->IsInMap(player))
                        member->FailQuest(_escortQuest->GetQuestId());
            }
        }
        else
            player->FailQuest(_escortQuest->GetQuestId());
    }
}

void EscortAI::JustDied(Unit* /*killer*/)
{
    if (!HasEscortState(STATE_ESCORT_ESCORTING))
        return;

    RemoveEscortState(STATE_ESCORT_ESCORTING); //sun: remove escort state on death

    if (!_playerGUID || !_escortQuest)
        return;

    FailQuest();
}

void EscortAI::InitializeAI()
{
    _escortState = STATE_ESCORT_NONE;

    if (!IsCombatMovementAllowed())
        SetCombatMovementAllowed(true);

    // add a small delay before going to first waypoint, normal in near all cases
    _pauseTimer = 2000;

    if (me->GetFaction() != me->GetCreatureTemplate()->faction)
        me->RestoreFaction();

    //Re-Enable questgiver flag
    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
}

void EscortAI::ReturnToLastPoint()
{
    me->GetMotionMaster()->MovePoint(POINT_LAST_POINT, me->GetHomePosition());
}

void EscortAI::EnterEvadeMode(EvadeReason /* why */)
{
    me->RemoveAllAuras();
    me->GetThreatManager().ClearAllThreat();
    me->CombatStop();
    me->SetLootRecipient(nullptr);

    if (HasEscortState(STATE_ESCORT_ESCORTING) || !_despawnAtEnd)
    {
        AddEscortState(STATE_ESCORT_RETURNING);
        ReturnToLastPoint();

    } 
    else
    {
        if (_hasImmuneToNPCFlags)
            me->SetImmuneToNPC(true);
        me->GetMotionMaster()->MoveTargetedHome();
        Reset();
        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }
}

void EscortAI::UpdateAI(uint32 diff)
{
    // Waypoint Updating
    if (HasEscortState(STATE_ESCORT_ESCORTING) && !me->IsEngaged() && !HasEscortState(STATE_ESCORT_RETURNING))
    {
        if (_pauseTimer <= diff)
        {
            if (!HasEscortState(STATE_ESCORT_PAUSED))
            {
                _pauseTimer = 0;

                if (_ended)
                {
                    _ended = false;
                    me->GetMotionMaster()->MoveIdle();

                    if (_despawnAtEnd)
                    {
                        TC_LOG_DEBUG("scripts", "EscortAI::UpdateAI: reached end of waypoints, despawning at end");
                        if (_returnToStart)
                        {
                            Position respawnPosition;
                            float orientation = 0.f;
                            me->GetRespawnPosition(respawnPosition.m_positionX, respawnPosition.m_positionY, respawnPosition.m_positionZ, &orientation);
                            respawnPosition.SetOrientation(orientation);
                            me->GetMotionMaster()->MovePoint(POINT_HOME, respawnPosition);
                            TC_LOG_DEBUG("scripts", "EscortAI::UpdateAI: returning to spawn location: %s", respawnPosition.ToString().c_str());
                        }
                        else if (_instantRespawn)
                            me->Respawn(true);
                        else
                            me->DespawnOrUnsummon(5000);
                    }
                    TC_LOG_DEBUG("scripts", "EscortAI::UpdateAI: reached end of waypoints");
                    RemoveEscortState(STATE_ESCORT_ESCORTING);
                    return;
                }

                if (!_started)
                {
                    _started = true;
                    me->GetMotionMaster()->MovePath(_path, false);
                }
                else if (_resume)
                {
                    _resume = false;
                    if (MovementGenerator* movementGenerator = me->GetMotionMaster()->GetCurrentMovementGenerator(MOTION_SLOT_DEFAULT))
                        movementGenerator->Resume(0);
                }
            }
        }
        else
            _pauseTimer -= diff;
    }

    // Check if player or any member of his group is within range
    if (_despawnAtFar && HasEscortState(STATE_ESCORT_ESCORTING) && _playerGUID && !me->IsEngaged() && !HasEscortState(STATE_ESCORT_RETURNING))
    {
        if (_playerCheckTimer <= diff)
        {
            if (!IsPlayerOrGroupInRange())
            {
                TC_LOG_DEBUG("scripts", "EscortAI::UpdateAI: failed because player/group was to far away or not found");

                bool isEscort = false;
                if (CreatureData const* creatureData = me->GetCreatureData())
                    isEscort = (sWorld->getBoolConfig(CONFIG_RESPAWN_DYNAMIC_ESCORTNPC) && (creatureData->spawnGroupData->flags & SPAWNGROUP_FLAG_ESCORTQUESTNPC));

                if (_instantRespawn)
                {
                    if (!isEscort)
                        me->DespawnOrUnsummon(0, 1s);
                    else
                        me->GetMap()->RemoveRespawnTime(SPAWN_TYPE_CREATURE, me->GetSpawnId(), true);
                }
                else
                    me->DespawnOrUnsummon();

                FailQuest();
                return;
            }

            _playerCheckTimer = 1000;
        }
        else
            _playerCheckTimer -= diff;
    }

    UpdateEscortAI(diff);
}

void EscortAI::UpdateEscortAI(uint32 /*diff*/)
{
    if (!UpdateVictim())
        return;

    DoMeleeAttackIfReady();
}

void EscortAI::MovementInform(uint32 type, uint32 id)
{
    // no action allowed if there is no escort
    if (!HasEscortState(STATE_ESCORT_ESCORTING))
        return;

    if (type == POINT_MOTION_TYPE)
    {
        if (!_pauseTimer)
            _pauseTimer = 2000;

        // continue waypoint movement
        if (id == POINT_LAST_POINT)
        {
            TC_LOG_DEBUG("scripts", "EscortAI::MovementInform: returned to before combat position");
            me->SetWalk(!_running);
            RemoveEscortState(STATE_ESCORT_RETURNING);
        }
        else if (id == POINT_HOME)
        {
            TC_LOG_DEBUG("scripts", "EscortAI::MovementInform: returned to home location and restarting waypoint path");
            _started = false;
        }
    }
    else if (type == WAYPOINT_MOTION_TYPE)
    {
        //sunstrider: diff with tc... we send db node and not node index in MovementInform, so less work to do here
        TC_LOG_DEBUG("scripts", "EscortAI::MovementInform: waypoint node %u reached", id);

        auto lastNode = _path.nodes[_path.nodes.size() - 1];
        // last point
        if (id == lastNode.id)
        {
            _started = false;
            _ended = true;
            _pauseTimer = 1000;
        }
    }
}

Player* EscortAI::GetPlayerForEscort() 
{ 
    return ObjectAccessor::GetPlayer(*me, _playerGUID);
}

void EscortAI::AddWaypoint(uint32 id, float x, float y, float z, float orientation/* = 0*/, uint32 waitTime/* = 0*/)
{
    Trinity::NormalizeMapCoord(x);
    Trinity::NormalizeMapCoord(y);

    WaypointNode waypoint;
    waypoint.id = id;
    waypoint.x = x;
    waypoint.y = y;
    waypoint.z = z;
    waypoint.orientation = orientation;
    // waypoint.moveType = _running ? WAYPOINT_MOVE_TYPE_RUN : WAYPOINT_MOVE_TYPE_WALK; //sunstrider: this is wrong, _running is defined at start time and we're obviously running this function before thatµ
    waypoint.moveType = WAYPOINT_MOVE_TYPE_USE_UNIT_MOVEMENT_FLAG;
    waypoint.delay = waitTime;
    waypoint.eventId = 0;
    waypoint.eventChance = 100;
    _path.nodes.push_back(std::move(waypoint));

    _manualPath = true;
}

void EscortAI::FillPointMovementListForCreature()
{
    /*WaypointPath const* path = sScriptSystemMgr->GetPath(me->GetEntry());
    if (!path)
        return;

    for (WaypointNode const& value : path->nodes)
    {
        WaypointNode node = value;
        Trinity::NormalizeMapCoord(node.x);
        Trinity::NormalizeMapCoord(node.y);
        node.moveType = _running ? WAYPOINT_MOVE_TYPE_RUN : WAYPOINT_MOVE_TYPE_WALK;

        _path.nodes.push_back(std::move(node));
    }
    */ 
    _path.nodes.clear();
    uint32 entry = me->GetEntry();
    if (entry == 0)
        return;

    QueryResult result = WorldDatabase.PQuery("SELECT pointid, location_x, location_y, location_z, waittime FROM escort_waypoints WHERE entry = '%u' ORDER BY pointid", entry);

    if (!result)
    {
        TC_LOG_ERROR("scripts", "SD2 ERROR: EscortAI: Attempt to GetWaypointListFromDB for creature entry %u, but no record found in DB !", entry);
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        WaypointNode node;
        node.id =  fields[0].GetUInt16();
        node.x = fields[1].GetFloat();
        node.y = fields[2].GetFloat();
        node.z = fields[3].GetFloat();
        node.delay = fields[4].GetUInt32();

        Trinity::NormalizeMapCoord(node.x);
        Trinity::NormalizeMapCoord(node.y);
        node.moveType = _running ? WAYPOINT_MOVE_TYPE_RUN : WAYPOINT_MOVE_TYPE_WALK;

        _path.nodes.push_back(std::move(node));
    } while (result->NextRow());
}

void EscortAI::SetRun(bool on)
{
    if (on == _running)
        return;

    for (auto& node : _path.nodes)
        node.moveType = on ? WAYPOINT_MOVE_TYPE_RUN : WAYPOINT_MOVE_TYPE_WALK;

    me->SetWalk(!on);
    _running = on;
}

bool EscortAI::IsPlayerOrGroupInRange()
{
    if (Player* player = GetPlayerForEscort())
    {
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* groupRef = group->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
            {
                if (Player* member = groupRef->GetSource())
                    if (me->IsWithinDistInMap(member, GetMaxPlayerDistance()))
                        return true;
            }
        }
        else if (me->IsWithinDistInMap(player, GetMaxPlayerDistance()))
            return true;
    }

    return false;
}

void EscortAI::Start(bool isActiveAttacker /* = true*/, bool run /* = false */, ObjectGuid playerGUID /* = 0 */, Quest const* quest /* = nullptr */, bool instantRespawn /* = false */, bool canLoopPath /* = false */, bool resetWaypoints /* = true */)
{
    if (Map* map = me->GetMap())
    {
        if (CreatureData const* cdata = me->GetCreatureData())
        {
            if (SpawnGroupTemplateData const* groupdata = cdata->spawnGroupData)
            {
                if (sWorld->getBoolConfig(CONFIG_RESPAWN_DYNAMIC_ESCORTNPC) && (groupdata->flags & SPAWNGROUP_FLAG_ESCORTQUESTNPC) && !map->GetCreatureRespawnTime(me->GetSpawnId()))
                {
                    me->SetRespawnTime(me->GetRespawnDelay());
                    me->SaveRespawnTime();
                }
            }
        }
    }

    if (me->GetVictim())
    {
        TC_LOG_ERROR("scripts", "EscortAI::Start: (script: %s, creature entry: %u) attempts to Start while in combat", me->GetScriptName().c_str(), me->GetEntry());
        return;
    }

    if (HasEscortState(STATE_ESCORT_ESCORTING))
    {
        TC_LOG_ERROR("scripts", "EscortAI::Start: (script: %s, creature entry: %u) attempts to Start while already escorting", me->GetScriptName().c_str(), me->GetEntry());
        return;
    }

    _running = run;

    if (!_manualPath && resetWaypoints)
        FillPointMovementListForCreature();

    if (_path.nodes.empty())
    {
        TC_LOG_ERROR("scripts", "EscortAI::Start: (script: %s, creature entry: %u) starts with 0 waypoints (possible missing entry in script_waypoint. Quest: %u).", me->GetScriptName().c_str(), me->GetEntry(), quest ? quest->GetQuestId() : 0);
        return;
    }

    // set variables
    _activeAttacker = isActiveAttacker;
    _playerGUID = playerGUID;
    _escortQuest = quest;
    _instantRespawn = instantRespawn;
    _returnToStart = canLoopPath;

    if (_returnToStart && _instantRespawn)
        TC_LOG_DEBUG("scripts", "EscortAI::Start: (script: %s, creature entry: %u) is set to return home after waypoint end and instant respawn at waypoint end. Creature will never despawn.", me->GetScriptName().c_str(), me->GetEntry());

    me->GetMotionMaster()->MoveIdle();
    me->GetMotionMaster()->Clear(MOTION_PRIORITY_NORMAL);

    // disable npcflags
    me->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
    if (me->IsImmuneToNPC())
    {
        _hasImmuneToNPCFlags = true;
        me->SetImmuneToNPC(false);
    }

    TC_LOG_DEBUG("scripts", "EscortAI::Start: (script: %s, creature entry: %u) started with %u waypoints. ActiveAttacker = %d, Run = %d, Player = %u", me->GetScriptName().c_str(), me->GetEntry(), uint32(_path.nodes.size()), _activeAttacker, _running, _playerGUID.GetCounter());

    // set initial speed
    me->SetWalk(!_running);
    //sunstrider: Improve compat with older scripts... if we used manual point, lets always use unit move flags
    if (_manualPath)
        _useUnitWalkFlag = true;

    _started = false;
    AddEscortState(STATE_ESCORT_ESCORTING);
}

void EscortAI::SetEscortPaused(bool on)
{
    if (!HasEscortState(STATE_ESCORT_ESCORTING))
        return;

    if (on)
    {
        AddEscortState(STATE_ESCORT_PAUSED);
        if (MovementGenerator* movementGenerator = me->GetMotionMaster()->GetCurrentMovementGenerator(MOTION_SLOT_DEFAULT))
            movementGenerator->Pause(0);
    }
    else
    {
        RemoveEscortState(STATE_ESCORT_PAUSED);
        _resume = true;
    }
}

bool EscortAI::AssistPlayerInCombatAgainst(Unit* who)
{
    if (!who || !who->GetVictim())
        return false;

    if (me->HasReactState(REACT_PASSIVE))
        return false;

    //experimental (unknown) flag not present
    if (!(me->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_CAN_ASSIST))
        return false;

    //not a player
    if (!who->GetVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
        return false;

    if (!who->isInAccessiblePlaceFor(me))
        return false;

    if (!CanAIAttack(who))
        return false;

    // we cannot attack in evade mode
    if (me->IsInEvadeMode())
        return false;

    // or if enemy is in evade mode
    if (who->GetTypeId() == TYPEID_UNIT && who->ToCreature()->IsInEvadeMode())
        return false;

    if (!me->IsValidAssistTarget(who->GetVictim()))
        return false;

    //too far away and no free sight?
    if (me->IsWithinDistInMap(who, GetMaxPlayerDistance()) && me->IsWithinLOSInMap(who, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
    {
        me->EngageWithTarget(who);
        return true;
    }

    return false;
}

bool EscortAI::IsEscortNPC(bool onlyIfActive) const
{
    if (!onlyIfActive)
        return true;

    if (GetEventStarterGUID())
        return true;

    return false;
}
