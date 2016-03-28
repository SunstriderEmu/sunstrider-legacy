/*
Current bugs :
- MotionTransports are at 0,0,0 on client until first teleport
    to try : dumping all fields at first send then after first teleport, see what changed for client
- MotionTransports have some strange behavior at client after a while ? (transport randomly stopping, broken player movement when aboard)
- No MMaps handling on transports : Not so important, creatures force destination for now
 */

#include "Common.h"
#include "Transport.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "WorldPacket.h"
#include "DBCStores.h"
#include "World.h"
#include "GameObjectAI.h"
#include "MapReference.h"
#include "Player.h"
#include "Cell.h"
#include "CellImpl.h"
#include "Totem.h"
#include "Spell.h"

uint32 Transport::GetPathProgress() const 
{
    return GetGOValue()->Transport.PathProgress;
}

MotionTransport::MotionTransport() : Transport(), _transportInfo(NULL), _isMoving(true), _pendingStop(false), _triggeredArrivalEvent(false), _triggeredDepartureEvent(false), _passengersLoaded(false), _delayedTeleport(false)
{
#ifdef LICH_KING
    m_updateFlag = UPDATEFLAG_TRANSPORT | UPDATEFLAG_LOWGUID | UPDATEFLAG_STATIONARY_POSITION | UPDATEFLAG_ROTATION;
#else
    m_updateFlag = UPDATEFLAG_TRANSPORT | UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_STATIONARY_POSITION;
#endif
}

MotionTransport::~MotionTransport()
{
    ASSERT(_passengers.empty());
    UnloadStaticPassengers();
}

bool MotionTransport::CreateMoTrans(uint32 guidlow, uint32 entry, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress)
{
    Relocate(x, y, z, ang);

    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "Transport (GUID: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)",
            guidlow, x, y);
        return false;
    }

    Object::_Create(guidlow, 0, HIGHGUID_MO_TRANSPORT);

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);

    if (!goinfo)
    {
        TC_LOG_ERROR("entities.transport", "Transport not created: entry in `gameobject_template` not found, guidlow: %u map: %u  (X: %f Y: %f Z: %f) ang: %f", guidlow, mapid, x, y, z, ang);
        return false;
    }

    m_goInfo = goinfo;

    TransportTemplate const* tInfo = sTransportMgr->GetTransportTemplate(entry);
    if (!tInfo)
    {
        TC_LOG_ERROR("entities.transport", "Transport %u (name: %s) will not be created, missing `transport_template` entry.", entry, goinfo->name.c_str());
        return false;
    }

    _transportInfo = tInfo;

    // initialize waypoints
    _nextFrame = tInfo->keyFrames.begin();
    _currentFrame = _nextFrame++;
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    SetObjectScale(goinfo->size);
    SetUInt32Value(GAMEOBJECT_FACTION, goinfo->faction);
    SetUInt32Value(GAMEOBJECT_FLAGS, goinfo->flags);
    SetPathProgress(0);
    SetPeriod(tInfo->pathTime);
    SetEntry(goinfo->entry);
    SetDisplayId(goinfo->displayId);
    //Ships on BC cannot be paused and are always GO_STATE_READY
#ifdef LICH_KING
    SetGoState(!goinfo->moTransport.canBeStopped ? GO_STATE_READY : GO_STATE_ACTIVE);
#else
    SetGoState(GO_STATE_READY);
#endif
    SetGoType(GAMEOBJECT_TYPE_MO_TRANSPORT);
    SetGoAnimProgress(animprogress);
    SetName(goinfo->name);

    // pussywizard: no WorldRotation for MotionTransports
#ifdef LICH_KING
    SetWorldRotation(G3D::Quat());
#endif
    // pussywizard: no PathRotation for MotionTransports
    SetTransportPathRotation(0.0f, 0.0f, 0.0f, 1.0f);

    m_model = CreateModel();
    return true;
}

void MotionTransport::CleanupsBeforeDelete(bool finalCleanup /*= true*/)
{
    UnloadStaticPassengers();
    while (!_passengers.empty())
    {
        WorldObject* obj = *_passengers.begin();
        RemovePassenger(obj);
        obj->SetTransport(NULL);
        obj->m_movementInfo.transport.Reset();
        obj->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
    }

    GameObject::CleanupsBeforeDelete(finalCleanup);
}

void MotionTransport::BuildUpdate(UpdateDataMapType& data_map, UpdatePlayerSet&)
{
    Map::PlayerList const& players = GetMap()->GetPlayers();
    if (players.isEmpty())
        return;

    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        BuildFieldsUpdate(itr->GetSource(), data_map);

    ClearUpdateMask(true);
}

void MotionTransport::Update(uint32 diff)
{
    uint32 const positionUpdateDelay = 1;

    if (AI())
        AI()->UpdateAI(diff);
    else if (!AIM_Initialize())
        TC_LOG_ERROR("entities.transport", "Could not initialize GameObjectAI for Transport");

    if (GetKeyFrames().size() <= 1)
        return;

    if (IsMoving() || !_pendingStop)
        SetPathProgress(GetPathProgress() + diff);

    uint32 timer = GetPathProgress() % GetPeriod();

    // Set current waypoint
    // Desired outcome: _currentFrame->DepartureTime < timer < _nextFrame->ArriveTime
    // ... arrive | ... delay ... | departure
    //      event /         event /
    for (;;)
    {
        if (timer >= _currentFrame->ArriveTime)
        {
            if (!_triggeredArrivalEvent)
            {
                DoEventIfAny(*_currentFrame, false);
                _triggeredArrivalEvent = true;
            }

            if (timer < _currentFrame->DepartureTime)
            {
                SetMoving(false);
#ifdef LICH_KING
                if (_pendingStop && GetGoState() != GO_STATE_READY)
                {
                    SetGoState(GO_STATE_READY);
                    SetPathProgress(GetPathProgress() / GetPeriod());
                    SetPathProgress(GetPathProgress() * GetPeriod());
                    SetPathProgress(GetPathProgress() + _currentFrame->ArriveTime);
                }
#endif
                break;  // its a stop frame and we are waiting
            }
        }

        if (timer >= _currentFrame->DepartureTime && !_triggeredDepartureEvent)
        {
            DoEventIfAny(*_currentFrame, true); // departure event
            _triggeredDepartureEvent = true;
        }

        // not waiting anymore
        SetMoving(true);

        // Enable movement //No pause for ships on BC
#ifdef LICH_KING
        if (GetGOInfo()->moTransport.canBeStopped)
            SetGoState(GO_STATE_ACTIVE);
#endif

        if (timer >= _currentFrame->DepartureTime && timer < _currentFrame->NextArriveTime)
            break;  // found current waypoint

        MoveToNextWaypoint();

        // sScriptMgr->OnRelocate(this, _currentFrame->Node->NodeIndex, _currentFrame->Node->MapID, _currentFrame->Node->LocX, _currentFrame->Node->LocY, _currentFrame->Node->LocZ);

        //TC_LOG_DEBUG("entities.transport", "Transport %u (%s) moved to node %u %u %f %f %f", GetEntry(), GetName().c_str(), _currentFrame->Node->index, _currentFrame->Node->mapid, _currentFrame->Node->x, _currentFrame->Node->y, _currentFrame->Node->z);

        // Departure event
        if (_currentFrame->IsTeleportFrame())
            if (TeleportTransport(_nextFrame->Node->MapID, _nextFrame->Node->LocX, _nextFrame->Node->LocY, _nextFrame->Node->LocZ, _nextFrame->InitialOrientation))
                return; // Update more in new map thread
    }

    // Set position
    _positionChangeTimer.Update(diff);
    if (_positionChangeTimer.Passed())
    {
        _positionChangeTimer.Reset(positionUpdateDelay);
        if (IsMoving())
        {
            float t = CalculateSegmentPos(float(timer) * 0.001f);
            G3D::Vector3 pos, dir;
            _currentFrame->Spline->evaluate_percent(_currentFrame->Index, t, pos);
            _currentFrame->Spline->evaluate_derivative(_currentFrame->Index, t, dir);
            UpdatePosition(pos.x, pos.y, pos.z, NormalizeOrientation(atan2(dir.y, dir.x) + M_PI));
        }
        else
        {
            /* There are four possible scenarios that trigger loading/unloading passengers:
            1. transport moves from inactive to active grid
            2. the grid that transport is currently in becomes active
            3. transport moves from active to inactive grid
            4. the grid that transport is currently in unloads
            */
            if (_staticPassengers.empty() && GetMap()->IsGridLoaded(GetPositionX(), GetPositionY())) // 2.
                LoadStaticPassengers();
        }
    }

    sScriptMgr->OnTransportUpdate(this, diff);
}

void MotionTransport::DelayedUpdate(uint32 diff)
{
    if (GetKeyFrames().size() <= 1)
        return;

    DelayedTeleportTransport();
}

void MotionTransport::UpdatePosition(float x, float y, float z, float o)
{
    if (!GetMap()->IsGridLoaded(x, y)) // pussywizard: should not happen, but just in case
        GetMap()->LoadGrid(x, y);

    Relocate(x, y, z, o);
    UpdateModelPosition();

    UpdatePassengerPositions(_passengers);

    if (_staticPassengers.empty())
        LoadStaticPassengers();
    else
        UpdatePassengerPositions(_staticPassengers);
}

void MotionTransport::AddPassenger(WorldObject* passenger, bool withAll)
{
    std::lock_guard<std::mutex> guard(Lock);
    if (_passengers.insert(passenger).second)
    {
        if (Player* plr = passenger->ToPlayer())
            sScriptMgr->OnAddPassenger(ToTransport(), plr);

        if (withAll)
        {
            if (Transport* t = passenger->GetTransport()) // SHOULD NEVER HAPPEN
                t->RemovePassenger(passenger, false);

            float x, y, z, o;
            passenger->GetPosition(x, y, z, o);
            CalculatePassengerOffset(x, y, z, &o);

            passenger->SetTransport(this);
            passenger->m_movementInfo.flags |= MOVEMENTFLAG_ONTRANSPORT;
            passenger->m_movementInfo.transport.guid = GetGUID();
            passenger->m_movementInfo.transport.pos.Relocate(x, y, z, o);
        }
    }
}

void MotionTransport::RemovePassenger(WorldObject* passenger, bool withAll)
{
    std::lock_guard<std::mutex> guard(Lock);
    if (_passengers.erase(passenger) || _staticPassengers.erase(passenger))
    {
        if (Player* plr = passenger->ToPlayer())
        {
            sScriptMgr->OnRemovePassenger(ToTransport(), plr);
            plr->SetFallInformation(time(NULL), plr->GetPositionZ());
        }

        if (withAll)
        {
            passenger->SetTransport(NULL);
            passenger->m_movementInfo.flags &= ~MOVEMENTFLAG_ONTRANSPORT;
            passenger->m_movementInfo.transport.guid = 0;
            passenger->m_movementInfo.transport.pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }
}

Creature* MotionTransport::CreateNPCPassenger(uint32 guid, CreatureData const* data)
{
    Map* map = GetMap();
    Creature* creature = new Creature();

    if (!creature->LoadFromDB(guid, map)) //do not add to map yet
    {
        delete creature;
        return NULL;
    }

    float x = data->posX;
    float y = data->posY;
    float z = data->posZ;
    float o = data->orientation;

    creature->SetTransport(this);
    creature->AddUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
    creature->m_movementInfo.transport.guid = GetGUID();
    creature->m_movementInfo.transport.pos.Relocate(x, y, z, o);
    CalculatePassengerPosition(x, y, z, &o);
    creature->Relocate(x, y, z, o);
    creature->SetHomePosition(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());
    creature->SetTransportHomePosition(creature->m_movementInfo.transport.pos);

#ifndef LICH_KING
    //keep these mobs as purely aesthetic for BC as the ship crews should not even be there on BC
    creature->SetReactState(REACT_PASSIVE);
    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    //also prevent using vendors
    creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_VENDOR_REAGENT | UNIT_NPC_FLAG_VENDOR_AMMO | UNIT_NPC_FLAG_VENDOR_FOOD | UNIT_NPC_FLAG_VENDOR_POISON | UNIT_NPC_FLAG_REPAIR);
#endif

    /// @HACK - transport models are not added to map's dynamic LoS calculations
    ///         because the current GameObjectModel cannot be moved without recreating
    creature->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);

    if (!creature->IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "Creature (guidlow %d, entry %d) not created. Suggested coordinates aren't valid (X: %f Y: %f)", creature->GetGUIDLow(), creature->GetEntry(), creature->GetPositionX(), creature->GetPositionY());
        delete creature;
        return NULL;
    }

    if (!map->Add(creature))
    {
        delete creature;
        return NULL;
    }

    _staticPassengers.insert(creature);
    sScriptMgr->OnAddCreaturePassenger(this, creature);
    return creature;
}

GameObject* MotionTransport::CreateGOPassenger(uint32 guid, GameObjectData const* data)
{
#ifdef LICH_KING
    Map* map = GetMap();
    GameObject* go = new GameObject();
    ASSERT(!sObjectMgr->IsGameObjectStaticTransport(data->id));

    if (!go->LoadGameObjectFromDB(guid, map, false))
    {
        delete go;
        return NULL;
    }

    float x = data->posX;
    float y = data->posY;
    float z = data->posZ;
    float o = data->orientation;

    go->SetTransport(this);
    go->m_movementInfo.transport.guid = GetGUID();
    go->m_movementInfo.transport.pos.Relocate(x, y, z, o);
    CalculatePassengerPosition(x, y, z, &o);
    go->Relocate(x, y, z, o);

    if (!go->IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "GameObject (guidlow %d, entry %d) not created. Suggested coordinates aren't valid (X: %f Y: %f)", go->GetGUIDLow(), go->GetEntry(), go->GetPositionX(), go->GetPositionY());
        delete go;
        return NULL;
    }

    if (!map->AddToMap(go))
    {
        delete go;
        return NULL;
    }

    _staticPassengers.insert(go);
    return go;
#else
    //no gameobject on transports before LK ?
    return nullptr;
#endif
}

void MotionTransport::LoadStaticPassengers()
{
    if (PassengersLoaded())
        return;
    SetPassengersLoaded(true);
    if (uint32 mapId = GetGOInfo()->moTransport.mapID)
    {
        CellObjectGuidsMap const& cells = sObjectMgr->GetMapObjectGuids(mapId, GetMap()->GetSpawnMode());
        CellGuidSet::const_iterator guidEnd;
        for (CellObjectGuidsMap::const_iterator cellItr = cells.begin(); cellItr != cells.end(); ++cellItr)
        {
            // Creatures on transport
            guidEnd = cellItr->second.creatures.end();
            for (CellGuidSet::const_iterator guidItr = cellItr->second.creatures.begin(); guidItr != guidEnd; ++guidItr)
                CreateNPCPassenger(*guidItr, sObjectMgr->GetCreatureData(*guidItr));

            // GameObjects on transport
            guidEnd = cellItr->second.gameobjects.end();
            for (CellGuidSet::const_iterator guidItr = cellItr->second.gameobjects.begin(); guidItr != guidEnd; ++guidItr)
                CreateGOPassenger(*guidItr, sObjectMgr->GetGOData(*guidItr));
        }
    }
}

void MotionTransport::UnloadStaticPassengers()
{
    SetPassengersLoaded(false);
    while (!_staticPassengers.empty())
    {
        WorldObject* obj = *_staticPassengers.begin();
        obj->AddObjectToRemoveList();   // also removes from _staticPassengers
    }
}

void MotionTransport::UnloadNonStaticPassengers()
{
    for (PassengerSet::iterator itr = _passengers.begin(); itr != _passengers.end(); )
    {
        if ((*itr)->GetTypeId() == TYPEID_PLAYER)
        {
            ++itr;
            continue;
        }
        PassengerSet::iterator itr2 = itr++;
        (*itr2)->AddObjectToRemoveList();
    }
}

void MotionTransport::EnableMovement(bool enabled)
{
#ifdef LICH_KING
    if (!GetGOInfo()->moTransport.canBeStopped)
        return;

    _pendingStop = !enabled;
#endif
}

void MotionTransport::MoveToNextWaypoint()
{
    // Clear events flagging
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    // Set frames
    _currentFrame = _nextFrame++;
    if (_nextFrame == GetKeyFrames().end())
        _nextFrame = GetKeyFrames().begin();
}

float MotionTransport::CalculateSegmentPos(float now)
{
    KeyFrame const& frame = *_currentFrame;
    const float speed = float(m_goInfo->moTransport.moveSpeed);
    const float accel = float(m_goInfo->moTransport.accelRate);
    float timeSinceStop = frame.TimeFrom + (now - (1.0f / IN_MILLISECONDS) * frame.DepartureTime);
    float timeUntilStop = frame.TimeTo - (now - (1.0f / IN_MILLISECONDS) * frame.DepartureTime);
    float segmentPos, dist;
    float accelTime = _transportInfo->accelTime;
    float accelDist = _transportInfo->accelDist;
    // calculate from nearest stop, less confusing calculation...
    if (timeSinceStop < timeUntilStop)
    {
        if (timeSinceStop < accelTime)
            dist = 0.5f * accel * timeSinceStop * timeSinceStop;
        else
            dist = accelDist + (timeSinceStop - accelTime) * speed;
        segmentPos = dist - frame.DistSinceStop;
    }
    else
    {
        if (timeUntilStop < _transportInfo->accelTime)
            dist = 0.5f * accel * timeUntilStop * timeUntilStop;
        else
            dist = accelDist + (timeUntilStop - accelTime) * speed;
        segmentPos = frame.DistUntilStop - dist;
    }

    return segmentPos / frame.NextDistFromPrev;
}

bool MotionTransport::TeleportTransport(uint32 newMapid, float x, float y, float z, float o)
{
    Map const* oldMap = GetMap();

    if (oldMap->GetId() != newMapid)
    {
        _delayedTeleport = true;
        UnloadStaticPassengers();
        return true;
    }
    else
    {
        /* Disabled, dunno why some transports have strange teleport frames (Grom'gol/Undercity)

        // Teleport players, they need to know it
        for (PassengerSet::iterator itr = _passengers.begin(); itr != _passengers.end(); ++itr)
        {
            if ((*itr)->GetTypeId() == TYPEID_PLAYER)
            {
                float destX, destY, destZ, destO;
                (*itr)->m_movementInfo.transport.pos.GetPosition(destX, destY, destZ, destO);
                TransportBase::CalculatePassengerPosition(destX, destY, destZ, &destO, x, y, z, o);

                (*itr)->ToUnit()->NearTeleportTo(destX, destY, destZ, destO);
            }
        }

        */
        UpdatePosition(x, y, z, o);

        return false;
    }
}

void MotionTransport::DelayedTeleportTransport()
{
    if (!_delayedTeleport)
        return;

    _delayedTeleport = false;

    uint32 newMapId = _nextFrame->Node->MapID;
    float x = _nextFrame->Node->LocX,
        y = _nextFrame->Node->LocY,
        z = _nextFrame->Node->LocZ,
        o = _nextFrame->InitialOrientation;

    PassengerSet _passengersCopy = _passengers;
    for (PassengerSet::iterator itr = _passengersCopy.begin(); itr != _passengersCopy.end(); )
    {
        WorldObject* obj = (*itr++);

        if (_passengers.find(obj) == _passengers.end())
            continue;

        switch (obj->GetTypeId())
        {
        case TYPEID_UNIT:
            _passengers.erase(obj);
            if (!obj->ToCreature()->IsPet())
                obj->ToCreature()->DespawnOrUnsummon();
            break;
        case TYPEID_GAMEOBJECT:
            _passengers.erase(obj);
            obj->ToGameObject()->Delete();
            break;
        case TYPEID_DYNAMICOBJECT:
            _passengers.erase(obj);
            if (Unit* caster = obj->ToDynObject()->GetCaster())
                if (Spell* s = caster->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
                    if (obj->ToDynObject()->GetSpellId() == s->GetSpellInfo()->Id)
                    {
                        s->SendChannelUpdate(0);
                        s->SendInterrupted(0);
                        caster->RemoveAurasByCasterSpell(s->GetSpellInfo()->Id, caster->GetGUID());
                        //caster->RemoveOwnedAura(s->GetSpellInfo()->Id, caster->GetGUID());
                    }
            obj->AddObjectToRemoveList();
            break;
        case TYPEID_PLAYER:
        {
            float destX, destY, destZ, destO;
            obj->m_movementInfo.transport.pos.GetPosition(destX, destY, destZ, destO);
            TransportBase::CalculatePassengerPosition(destX, destY, destZ, &destO, x, y, z, o);
            if (!obj->ToPlayer()->TeleportTo(newMapId, destX, destY, destZ, destO, TELE_TO_NOT_LEAVE_TRANSPORT))
                _passengers.erase(obj);
        }
        break;
        default:
            break;
        }
    }

    Map* newMap = sMapMgr->CreateBaseMap(newMapId);
    GetMap()->Remove<MotionTransport>(this, false);
    newMap->LoadGrid(x, y); // xinef: load before adding passengers to new map
    //sunwell SetMap(newMap);
    SetMapId(newMap->GetId());

    Relocate(x, y, z, o);
    GetMap()->Add<MotionTransport>(this);

    LoadStaticPassengers();
}

void MotionTransport::UpdatePassengerPositions(PassengerSet& passengers)
{
    for (PassengerSet::iterator itr = passengers.begin(); itr != passengers.end(); ++itr)
    {
        WorldObject* passenger = *itr;
        // transport teleported but passenger not yet (can happen for players)
        if (passenger->GetMap() != GetMap())
            continue;

#ifdef LICH_KING
        // if passenger is on vehicle we have to assume the vehicle is also on transport and its the vehicle that will be updating its passengers
        if (Unit* unit = passenger->ToUnit())
            if (unit->GetVehicle())
                continue;
#endif

        // Do not use Unit::UpdatePosition here, we don't want to remove auras as if regular movement occurred
        float x, y, z, o;
        passenger->m_movementInfo.transport.pos.GetPosition(x, y, z, o);
        CalculatePassengerPosition(x, y, z, &o);

        // check if position is valid
        if (!Trinity::IsValidMapCoord(x, y, z))
            continue;

        switch (passenger->GetTypeId())
        {
        case TYPEID_UNIT:
        {
            Creature* creature = passenger->ToCreature();
            GetMap()->CreatureRelocation(creature, x, y, z, o);

            creature->GetTransportHomePosition(x, y, z, o);
            CalculatePassengerPosition(x, y, z, &o);
            creature->SetHomePosition(x, y, z, o);
        }
        break;
        case TYPEID_PLAYER:
            if (passenger->IsInWorld())
                GetMap()->PlayerRelocation(passenger->ToPlayer(), x, y, z, o);
            break;
        case TYPEID_GAMEOBJECT:
            GetMap()->GameObjectRelocation(passenger->ToGameObject(), x, y, z, o);
            break;
        case TYPEID_DYNAMICOBJECT:
           // TODO GetMap()->DynamicObjectRelocation(passenger->ToDynObject(), x, y, z, o);
            break;
        default:
            break;
        }
    }
}

void MotionTransport::DoEventIfAny(KeyFrame const& node, bool departure)
{
    if (uint32 eventid = departure ? node.Node->departureEventID : node.Node->arrivalEventID)
    {
        //TC_LOG_DEBUG("maps.script", "Taxi %s event %u of node %u of %s path", departure ? "departure" : "arrival", eventid, node.Node->index, GetName().c_str());
        //GetMap()->ScriptsStart(sEventScripts, eventid, this, this);
        sWorld->ScriptsStart(sEventScripts, eventid, this, this);
        EventInform(eventid);
    }
}

// pussywizard: StaticTransport below

StaticTransport::StaticTransport() : Transport(), _needDoInitialRelocation(false)
{
#ifdef LICH_KING
    m_updateFlag = UPDATEFLAG_TRANSPORT | UPDATEFLAG_LOWGUID | UPDATEFLAG_STATIONARY_POSITION | UPDATEFLAG_ROTATION;
#else
    m_updateFlag = UPDATEFLAG_TRANSPORT | UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_STATIONARY_POSITION;
#endif
}

StaticTransport::~StaticTransport()
{
    ASSERT(_passengers.empty());
}

bool StaticTransport::Create(uint32 guidlow, uint32 name_id, Map* map, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 animprogress, GOState go_state, uint32 artKit)
{
   // return GameObject::Create(guidlow, name_id, map, x, y, z, ang, rotation0, rotation1, rotation2, rotation3, animprogress, go_state, artKit);

    ASSERT(map);
    //sunwell SetMap(map);
    SetMapId(map->GetId());
    SetInstanceId(map->GetInstanceId());

    Relocate(x, y, z, ang);
    m_stationaryPosition.Relocate(x, y, z, ang);

    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.gameobject", "Gameobject (GUID: %u Entry: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)", guidlow, name_id, x, y);
        return false;
    }

#ifdef LICH_KING
    SetPhaseMask(phaseMask, false);
#endif
    /* TODO zonescript
    SetZoneScript();
    if (m_zoneScript)
    {
        name_id = m_zoneScript->GetGameObjectEntry(guidlow, name_id);
        if (!name_id)
            return false;
    }
    */

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(name_id);
    if (!goinfo)
    {
        TC_LOG_ERROR("sql.sql", "Gameobject (GUID: %u Entry: %u) not created: non-existing entry in `gameobject_template`. Map: %u (X: %f Y: %f Z: %f)", guidlow, name_id, map->GetId(), x, y, z);
        return false;
    }

#ifdef LICH_KING
    Object::_Create(guidlow, 0, HIGHGUID_TRANSPORT);
#else
    //Object::_Create(guidlow, goinfo->entry, HIGHGUID_GAMEOBJECT); //entry doesn't seem necessary, but keeping this in comment in case this causes problem. If you change this, you'll also need to change it in layer::LoadFromDB (search for "MAKE_NEW_GUID(transGUIDLow, 0, HIGHGUID_GAMEOBJECT)")
    Object::_Create(guidlow, 0, HIGHGUID_GAMEOBJECT);
#endif

    m_goInfo = goinfo;

    if (goinfo->type >= MAX_GAMEOBJECT_TYPE)
    {
        TC_LOG_ERROR("sql.sql", "Gameobject (GUID: %u Entry: %u) not created: non-existing GO type '%u' in `gameobject_template`. It will crash client if created.", guidlow, name_id, goinfo->type);
        return false;
    }

    // pussywizard: temporarily calculate WorldRotation from orientation, do so until values in db are correct
    //SetWorldRotation( /*for StaticTransport we need 2 rotation Quats in db for World- and Path- Rotation*/ );
#ifdef LICH_KING
    SetWorldRotationAngles(NormalizeOrientation(GetOrientation()), 0.0f, 0.0f);
#else
    SetFloatValue(GAMEOBJECT_POS_X, x);
    SetFloatValue(GAMEOBJECT_POS_Y, y);
    SetFloatValue(GAMEOBJECT_POS_Z, z);
    SetFloatValue(GAMEOBJECT_FACING, ang);                  //this is not facing angle
#endif
    // pussywizard: PathRotation for StaticTransport (only StaticTransports have PathRotation)
    SetTransportPathRotation(rotation0, rotation1, rotation2, rotation3);

    SetObjectScale(goinfo->size);

    SetUInt32Value(GAMEOBJECT_FACTION, goinfo->faction);
    SetUInt32Value(GAMEOBJECT_FLAGS, goinfo->flags);

    SetEntry(goinfo->entry);
    SetName(goinfo->name);

    SetDisplayId(goinfo->displayId);

    if (!m_model)
        m_model = CreateModel();

    SetGoType(GameobjectTypes(goinfo->type));
    SetGoState(GOState(go_state));
    //SetGoState(goinfo->transport.startOpen ? GO_STATE_ACTIVE : GO_STATE_READY);
    SetGoAnimProgress(animprogress);

    SetGoArtKit(artKit);
    
    m_goValue.Transport.AnimationInfo = sTransportMgr->GetTransportAnimInfo(goinfo->entry);
    //ASSERT(m_goValue.Transport.AnimationInfo);
    //ASSERT(m_goValue.Transport.AnimationInfo->TotalTime > 0);
    SetPauseTime(goinfo->transport.pauseAtTime);
    if (goinfo->transport.startOpen && goinfo->transport.pauseAtTime)
    {
        SetPathProgress(goinfo->transport.pauseAtTime);
        _needDoInitialRelocation = true;
    }
    else
        SetPathProgress(0);

/* sunwell
    if (GameObjectAddon const* addon = sObjectMgr->GetGameObjectAddon(guidlow))
    {
        if (addon->InvisibilityValue)
        {
            m_invisibility.AddFlag(addon->invisibilityType);
            m_invisibility.AddValue(addon->invisibilityType, addon->InvisibilityValue);
        }
    }
    */

    LastUsedScriptID = GetGOInfo()->ScriptId;

    this->SetKeepActive(true);
    return true;
}

void StaticTransport::CleanupsBeforeDelete(bool finalCleanup /*= true*/)
{
    while (!_passengers.empty())
    {
        WorldObject* obj = *_passengers.begin();
        RemovePassenger(obj);
        obj->SetTransport(NULL);
        obj->m_movementInfo.transport.Reset();
        obj->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
    }

    GameObject::CleanupsBeforeDelete(finalCleanup);
}

void StaticTransport::BuildUpdate(UpdateDataMapType& data_map, UpdatePlayerSet&)
{
    Map::PlayerList const& players = GetMap()->GetPlayers();
    if (players.isEmpty())
        return;

    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        BuildFieldsUpdate(itr->GetSource(), data_map);

    ClearUpdateMask(true);
}

void StaticTransport::Update(uint32 diff)
{
    GameObject::Update(diff);

    if (!IsInWorld())
        return;

    if (!m_goValue.Transport.AnimationInfo)
        return;

    if (_needDoInitialRelocation)
    {
        _needDoInitialRelocation = false;
        RelocateToProgress(GetPathProgress());
    }

    if (GetPauseTime())
    {
        if (GetGoState() == GO_STATE_READY)
        {
            if (GetPathProgress() == 0) // waiting at it's destination for state change, do nothing
                return;

            if (GetPathProgress() < GetPauseTime()) // GOState has changed before previous state was reached, move to new destination immediately
                SetPathProgress(0);
            else if (GetPathProgress() + diff < GetPeriod())
                SetPathProgress(GetPathProgress() + diff);
            else
                SetPathProgress(0);
        }
        else
        {
            if (GetPathProgress() == GetPauseTime()) // waiting at it's destination for state change, do nothing
                return;

            if (GetPathProgress() > GetPauseTime()) // GOState has changed before previous state was reached, move to new destination immediately
                SetPathProgress(GetPauseTime());
            else if (GetPathProgress() + diff < GetPauseTime())
                SetPathProgress(GetPathProgress() + diff);
            else
                SetPathProgress(GetPauseTime());
        }
    }
    else
    {
        SetPathProgress(GetPathProgress() + diff);
        if (GetPathProgress() >= GetPeriod())
            SetPathProgress(GetPathProgress() % GetPeriod());
    }

    RelocateToProgress(GetPathProgress());
}

void StaticTransport::RelocateToProgress(uint32 progress)
{
    TransportAnimationEntry const *curr = NULL, *next = NULL;
    float percPos;
    if (m_goValue.Transport.AnimationInfo->GetAnimNode(progress, curr, next, percPos))
    {
        // curr node offset
        G3D::Vector3 pos = G3D::Vector3(curr->X, curr->Y, curr->Z);

        // move by percentage of segment already passed
        pos += G3D::Vector3(percPos * (next->X - curr->X), percPos * (next->Y - curr->Y), percPos * (next->Z - curr->Z));

        // rotate path by PathRotation
        // pussywizard: PathRotation in db is only simple orientation rotation, so don't use sophisticated and not working code
        // reminder: WorldRotation only influences model rotation, not the path
        float sign = GetFloatValue(GAMEOBJECT_PARENTROTATION + 2) >= 0.0f ? 1.0f : -1.0f;
        float pathRotAngle = sign * 2.0f * acos(GetFloatValue(GAMEOBJECT_PARENTROTATION + 3));
        float cs = cos(pathRotAngle), sn = sin(pathRotAngle);
        float nx = pos.x * cs - pos.y * sn;
        float ny = pos.x * sn + pos.y * cs;
        pos.x = nx;
        pos.y = ny;

        // add stationary position to the calculated offset
        pos += G3D::Vector3(GetStationaryX(), GetStationaryY(), GetStationaryZ());

        // rotate by AnimRotation at current segment
        // pussywizard: AnimRotation in dbc is only simple orientation rotation, so don't use sophisticated and not working code
#ifdef LICH_KING
        G3D::Quat currRot, nextRot;
        float percRot;
        m_goValue.Transport.AnimationInfo->GetAnimRotation(progress, currRot, nextRot, percRot);
        float signCurr = currRot.z >= 0.0f ? 1.0f : -1.0f;
        float oriRotAngleCurr = signCurr * 2.0f * acos(currRot.w);
        float signNext = nextRot.z >= 0.0f ? 1.0f : -1.0f;
        float oriRotAngleNext = signNext * 2.0f * acos(nextRot.w);
        float oriRotAngle = oriRotAngleCurr + percRot * (oriRotAngleNext - oriRotAngleCurr);
#else
        float oriRotAngle = 0.0f;
#endif
        // check if position is valid
        if (!Trinity::IsValidMapCoord(pos.x, pos.y, pos.z))
            return;

        // update position to new one
        // also adding simplified orientation rotation here
        UpdatePosition(pos.x, pos.y, pos.z, NormalizeOrientation(GetStationaryO() + oriRotAngle));
    }
}

void StaticTransport::UpdatePosition(float x, float y, float z, float o)
{
    if (!GetMap()->IsGridLoaded(x, y)) // pussywizard: should not happen, but just in case
        GetMap()->LoadGrid(x, y);

    GetMap()->GameObjectRelocation(this, x, y, z, o); // this also relocates the model
    UpdatePassengerPositions();
}

void StaticTransport::UpdatePassengerPositions()
{
    for (PassengerSet::iterator itr = _passengers.begin(); itr != _passengers.end(); ++itr)
    {
        WorldObject* passenger = *itr;

#ifdef LICH_KING
        // if passenger is on vehicle we have to assume the vehicle is also on transport and its the vehicle that will be updating its passengers
        if (Unit* unit = passenger->ToUnit())
            if (unit->GetVehicle())
                continue;
#endif

        // Do not use Unit::UpdatePosition here, we don't want to remove auras as if regular movement occurred
        float x, y, z, o;
        passenger->m_movementInfo.transport.pos.GetPosition(x, y, z, o);
        CalculatePassengerPosition(x, y, z, &o);

        // check if position is valid
        if (!Trinity::IsValidMapCoord(x, y, z))
            continue;

        switch (passenger->GetTypeId())
        {
        case TYPEID_UNIT:
            GetMap()->CreatureRelocation(passenger->ToCreature(), x, y, z, o);
            break;
        case TYPEID_PLAYER:
            if (passenger->IsInWorld())
                GetMap()->PlayerRelocation(passenger->ToPlayer(), x, y, z, o);
            break;
        case TYPEID_GAMEOBJECT:
            GetMap()->GameObjectRelocation(passenger->ToGameObject(), x, y, z, o);
            break;
        case TYPEID_DYNAMICOBJECT:
            // TODO GetMap()->DynamicObjectRelocation(passenger->ToDynObject(), x, y, z, o);
            break;
        default:
            break;
        }
    }
}

void StaticTransport::AddPassenger(WorldObject* passenger, bool withAll)
{
    if (_passengers.insert(passenger).second)
    {
        if (Player* plr = passenger->ToPlayer())
            sScriptMgr->OnAddPassenger(ToTransport(), plr);

        if (withAll)
        {
            if (Transport* t = passenger->GetTransport()) // SHOULD NEVER HAPPEN
                t->RemovePassenger(passenger, false);

            float x, y, z, o;
            passenger->GetPosition(x, y, z, o);
            CalculatePassengerOffset(x, y, z, &o);

            passenger->SetTransport(this);
            passenger->m_movementInfo.flags |= MOVEMENTFLAG_ONTRANSPORT;
            passenger->m_movementInfo.transport.guid = GetGUID();
            passenger->m_movementInfo.transport.pos.Relocate(x, y, z, o);
        }
    }
}

void StaticTransport::RemovePassenger(WorldObject* passenger, bool withAll)
{
    if (_passengers.erase(passenger))
    {
        if (Player* plr = passenger->ToPlayer())
        {
            sScriptMgr->OnRemovePassenger(ToTransport(), plr);
            plr->SetFallInformation(time(NULL), plr->GetPositionZ());
        }

        if (withAll)
        {
            passenger->SetTransport(NULL);
            passenger->m_movementInfo.flags &= ~MOVEMENTFLAG_ONTRANSPORT;
            passenger->m_movementInfo.transport.guid = 0;
            passenger->m_movementInfo.transport.pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }
}