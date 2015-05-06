/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Common.h"
#include "Transport.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Path.h"
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
#include "TransportBase.h"

Transport::Transport() : GameObject(),
    _transportInfo(nullptr), _isMoving(true),
    _triggeredArrivalEvent(false), _triggeredDepartureEvent(false), 
    _isDocked(false), _passengerTeleportItr(_passengers.begin()),
     _delayedAddModel(false)
{
    m_updateFlag = UPDATEFLAG_TRANSPORT | UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_STATIONARY_POSITION;
}

Transport::~Transport()
{
    UnloadStaticPassengers();
    while (!_passengers.empty())
    {
        WorldObject* obj = *_passengers.begin();
        RemovePassenger(obj);
    }
}

bool Transport::Create(uint32 guidlow, uint32 entry, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress)
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
        TC_LOG_ERROR("sql.sql", "Transport not created: entry in `gameobject_template` not found, guidlow: %u map: %u  (X: %f Y: %f Z: %f) ang: %f", guidlow, mapid, x, y, z, ang);
        return false;
    }

    m_goInfo = goinfo;

    TransportTemplate const* tInfo = sTransportMgr->GetTransportTemplate(entry);
    if (!tInfo)
    {
        TC_LOG_ERROR("sql.sql", "Transport %u (name: %s) will not be created, missing `transport_template` entry.", entry, goinfo->name.c_str());
        return false;
    }

    _transportInfo = tInfo;

    // initialize waypoints
    _nextFrame = tInfo->keyFrames.begin();
    _currentFrame = _nextFrame++;
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    m_goValue.Transport.PathProgress = 0;
    SetObjectScale(goinfo->size);
    SetFaction(goinfo->faction);
    SetUInt32Value(GAMEOBJECT_FLAGS, goinfo->flags);
    SetPeriod(tInfo->pathTime);
    SetEntry(goinfo->entry);
    SetDisplayId(goinfo->displayId);
    SetGoState(GO_STATE_READY);
    SetGoType(GAMEOBJECT_TYPE_MO_TRANSPORT);
    SetGoAnimProgress(animprogress);
    SetName(goinfo->name);
    UpdateRotationFields(0.0f, 1.0f);

    m_model = GameObjectModel::Create(*this);
    return true;
}

void Transport::Update(uint32 diff)
{
    uint32 const positionUpdateDelay = 200;

    if (AI())
        AI()->UpdateAI(diff);
    else if (!AIM_Initialize())
        TC_LOG_ERROR("entities.transport", "Could not initialize GameObjectAI for Transport");

    if (GetKeyFrames().size() <= 1)
        return;

    if (IsMoving())
        m_goValue.Transport.PathProgress += diff;

    uint32 timer = GetMSTime() % GetPeriod(); //m_goValue.Transport.PathProgress % GetPeriod();

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
                if(!_isDocked)
                {
                    _isDocked = true;
                    JustDocked();
                }
                SetMoving(false);
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

        if (timer >= _currentFrame->DepartureTime && timer < _currentFrame->NextArriveTime)
            break;  // found current waypoint

        MoveToNextWaypoint();

        sScriptMgr->OnRelocate(this, _currentFrame->Node->index, _currentFrame->Node->mapid, _currentFrame->Node->x, _currentFrame->Node->y, _currentFrame->Node->z);

        _isDocked = false;

        TC_LOG_DEBUG("entities.transport", "Transport %u (%s) moved to node %u %u %f %f %f", GetEntry(), GetName().c_str(), _currentFrame->Node->index, _currentFrame->Node->mapid, _currentFrame->Node->x, _currentFrame->Node->y, _currentFrame->Node->z);

        // Departure event
        if (_currentFrame->IsTeleportFrame())
            if (TeleportTransport(_nextFrame->Node->mapid, _nextFrame->Node->x, _nextFrame->Node->y, _nextFrame->Node->z, _nextFrame->InitialOrientation))
                return; // Update more in new map thread
    }

    // Add model to map after we are fully done with moving maps
    if (_delayedAddModel)
    {
        _delayedAddModel = false;
        if (m_model)
            GetMap()->InsertGameObjectModel(*m_model);
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
            UpdatePosition(pos.x, pos.y, pos.z, atan2(dir.y, dir.x) + M_PI);
        }
        else
        {
            /* There are four possible scenarios that trigger loading/unloading passengers:
              1. transport moves from inactive to active grid
              2. the grid that transport is currently in becomes active
              3. transport moves from active to inactive grid
              4. the grid that transport is currently in unloads
            */
            bool gridActive = GetMap()->IsGridLoadedAt(GetPositionX(), GetPositionY());

            if (_staticPassengers.empty() && gridActive) // 2.
                LoadStaticPassengers();
            else if (!_staticPassengers.empty() && !gridActive)
                // 4. - if transports stopped on grid edge, some passengers can remain in active grids
                //      unload all static passengers otherwise passengers won't load correctly when the grid that transport is currently in becomes active
                UnloadStaticPassengers();
        }
    }

    sScriptMgr->OnTransportUpdate(this, diff);
}

void Transport::AddPassenger(WorldObject* passenger)
{
    if (!IsInWorld())
        return;

    if (_passengers.insert(passenger).second)
    {
        passenger->SetTransport(this);
        passenger->m_movementInfo.AddMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
        passenger->m_movementInfo.transport.guid = GetGUID();
        TC_LOG_DEBUG("entities.transport", "Object %s boarded transport %s.", passenger->GetName().c_str(), GetName().c_str());

        if (Player* plr = passenger->ToPlayer())
            sScriptMgr->OnAddPassenger(this, plr);
    }
}

void Transport::RemovePassenger(WorldObject* passenger)
{
    bool erased = false;
    if (_passengerTeleportItr != _passengers.end())
    {
        PassengerSet::iterator itr = _passengers.find(passenger);
        if (itr != _passengers.end())
        {
            if (itr == _passengerTeleportItr)
                ++_passengerTeleportItr;

            _passengers.erase(itr);
            erased = true;
        }
    }
    else
        erased = _passengers.erase(passenger) > 0;

    if (erased || _staticPassengers.erase(passenger)) // static passenger can remove itself in case of grid unload
    {
        passenger->SetTransport(NULL);
        passenger->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
        passenger->m_movementInfo.transport.Reset();
        TC_LOG_DEBUG("entities.transport", "Object %s removed from transport %s.", passenger->GetName().c_str(), GetName().c_str());

        if (Player* plr = passenger->ToPlayer())
            sScriptMgr->OnRemovePassenger(this, plr);
    }
}

//this won't add creature to map, you still got to do it afterwards. This also add passenger to _staticPassengers
Creature* Transport::CreateNPCPassenger(uint32 guid, CreatureData const* data)
{
    Map* map = GetMap();
    Creature* creature = new Creature();

    if (!creature->LoadFromDB(guid, map))
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

    creature->InitCreatureAddon(true);
    /// @HACK - transport models are not added to map's dynamic LoS calculations
    ///         because the current GameObjectModel cannot be moved without recreating
    creature->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
    //non blizz but better than npc doing totally random things :
    creature->SetReactState(REACT_PASSIVE);
    creature->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);

    if (!creature->IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "Creature (guidlow %d, entry %d) not created. Suggested coordinates aren't valid (X: %f Y: %f)",creature->GetGUIDLow(),creature->GetEntry(),creature->GetPositionX(),creature->GetPositionY());
        delete creature;
        return NULL;
    }

    _staticPassengers.insert(creature);
    sScriptMgr->OnAddCreaturePassenger(this, creature);
    return creature;
}

#ifdef LICH_KING
GameObject* Transport::CreateGOPassenger(uint32 guid, GameObjectData const* data)
{
    return NULL;

    Map* map = GetMap();
    GameObject* go = new GameObject();

    if (!go->LoadFromDB(guid, map))
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

    _staticPassengers.insert(go);
    return go;
}
#endif

void Transport::UpdatePosition(float x, float y, float z, float o)
{
    bool newActive = GetMap()->IsGridLoadedAt(x,y);

    Cell oldCell(CellPair(GetPositionX(), GetPositionY()));

    Relocate(x, y, z, o);
    UpdateModelPosition();

    UpdatePassengerPositions(_passengers);

    /* There are four possible scenarios that trigger loading/unloading passengers:
      1. transport moves from inactive to active grid
      2. the grid that transport is currently in becomes active
      3. transport moves from active to inactive grid
      4. the grid that transport is currently in unloads
    */
    if (_staticPassengers.empty() && newActive) // 1.
        LoadStaticPassengers();
    else if (!_staticPassengers.empty() && !newActive && oldCell.DiffGrid(Cell(CellPair(GetPositionX(), GetPositionY())))) // 3.
        UnloadStaticPassengers();
    else
        UpdatePassengerPositions(_staticPassengers);
    // 4. is handed by grid unload
}

void Transport::LoadStaticPassengers()
{
    if (uint32 mapId = GetGOInfo()->moTransport.mapID)
    {
        CellObjectGuidsMap const& cells = sObjectMgr->GetMapObjectGuids(mapId, GetMap()->GetSpawnMode());
        CellGuidSet::const_iterator guidEnd;
        for (CellObjectGuidsMap::const_iterator cellItr = cells.begin(); cellItr != cells.end(); ++cellItr)
        {
            // Creatures on transport
            guidEnd = cellItr->second.creatures.end();
            for (CellGuidSet::const_iterator guidItr = cellItr->second.creatures.begin(); guidItr != guidEnd; ++guidItr)
            {
                if(const CreatureData* data =  sObjectMgr->GetCreatureData(*guidItr))
                    if(Creature* creature = CreateNPCPassenger(*guidItr, data))
                    {
                        GetMap()->Add(creature);
                        sObjectMgr->AddCreatureToGrid(*guidItr, data);
                    }
            }
#ifdef LICH_KING
            // GameObjects on transport
            guidEnd = cellItr->second.gameobjects.end();
            for (CellGuidSet::const_iterator guidItr = cellItr->second.gameobjects.begin(); guidItr != guidEnd; ++guidItr)
                CreateGOPassenger(*guidItr, sObjectMgr->GetGOData(*guidItr)); 
#endif
        }
    }
}

void Transport::UnloadStaticPassengers()
{
    while (!_staticPassengers.empty())
    {
        WorldObject* obj = *_staticPassengers.begin();
        obj->AddObjectToRemoveList();   // also removes from _staticPassengers
    }
}

void Transport::MoveToNextWaypoint()
{
    // Clear events flagging
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    // Set frames
    _currentFrame = _nextFrame++;
    if (_nextFrame == GetKeyFrames().end())
        _nextFrame = GetKeyFrames().begin();
}

float Transport::CalculateSegmentPos(float now)
{
    KeyFrame const& frame = *_currentFrame;
    const float speed = float(m_goInfo->moTransport.moveSpeed);
    const float accel = float(m_goInfo->moTransport.accelRate);
    float timeSinceStop = frame.TimeFrom + (now - (1.0f/IN_MILLISECONDS) * frame.DepartureTime);
    float timeUntilStop = frame.TimeTo - (now - (1.0f/IN_MILLISECONDS) * frame.DepartureTime);
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

bool Transport::TeleportTransport(uint32 newMapid, float x, float y, float z, float o)
{
    Map const* oldMap = GetMap();

    if (oldMap->GetId() != newMapid)
    {
        UnloadStaticPassengers();
        GetMap()->Remove(this,false);
        SetMapId(newMapid);
        Map* newMap = GetMap();
        if(!newMap)
        {
            TC_LOG_ERROR("FIXME","Transport::TeleportTransport : Could not get map %u",newMapid);
            return false;
        }

        for (_passengerTeleportItr = _passengers.begin(); _passengerTeleportItr != _passengers.end();)
        {
            WorldObject* obj = (*_passengerTeleportItr++);

            float destX, destY, destZ, destO;
            obj->m_movementInfo.transport.pos.GetPosition(destX, destY, destZ, destO);
            TransportBase::CalculatePassengerPosition(destX, destY, destZ, &destO, x, y, z, o);

            switch (obj->GetTypeId())
            {
                case TYPEID_UNIT:
                    if (!IS_PLAYER_GUID(obj->ToUnit()->GetOwnerGUID()))  // pets should be teleported with player
                    {
                        obj->ToCreature()->FarTeleportTo(newMap, destX, destY, destZ, destO);
                        AddPassenger(obj->ToUnit()); //removed in FarTeleportTo
                    }
                    break;
                case TYPEID_GAMEOBJECT:
                {
                    GameObject* go = obj->ToGameObject();
                    go->GetMap()->Remove(go,false);
                    go->Relocate(destX, destY, destZ, destO);
                    go->GetMap()->Add(go);
                    go->SetMapId(newMapid);
                    break;
                }
                case TYPEID_PLAYER:
                    if (!obj->ToPlayer()->TeleportTo(newMapid, destX, destY, destZ, destO, TELE_TO_NOT_LEAVE_TRANSPORT))
                        RemovePassenger(obj);
                    break;
                case TYPEID_DYNAMICOBJECT:
                    obj->AddObjectToRemoveList();
                    break;
                default:
                    break;
            }
        }
        Relocate(x, y, z, o);
        GetMap()->Add(this);
        return true;
    }
    else
    {
//BC clients don't need this, if used the client leave the transport at teleport.
#ifdef LICH_KING
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
#endif

        UpdatePosition(x, y, z, o);
        return false;
    }
}

void Transport::UpdatePassengerPositions(PassengerSet& passengers)
{
    for (PassengerSet::iterator itr = passengers.begin(); itr != passengers.end(); ++itr)
    {
        WorldObject* passenger = *itr;
        // transport teleported but passenger not yet (can happen for players)
        if (passenger->GetMap() != GetMap())
            continue;

        // Do not use Unit::UpdatePosition here, we don't want to remove auras
        // as if regular movement occurred
        float x, y, z, o;
        passenger->m_movementInfo.transport.pos.GetPosition(x, y, z, o);
        CalculatePassengerPosition(x, y, z, &o);
        switch (passenger->GetTypeId())
        {
            case TYPEID_UNIT:
            {
                Creature* creature = passenger->ToCreature();
                GetMap()->CreatureRelocation(creature, x, y, z, o);
                creature->GetTransportHomePosition(x, y, z, o);
                CalculatePassengerPosition(x, y, z, &o);
                creature->SetHomePosition(x, y, z, o);
                break;
            }
            case TYPEID_PLAYER:
                //relocate only passengers in world and skip any player that might be still logging in/teleporting
                if (passenger->IsInWorld())
                    GetMap()->PlayerRelocation(passenger->ToPlayer(), x, y, z, o);
                break;
            case TYPEID_GAMEOBJECT:
                GetMap()->GameObjectRelocation(passenger->ToGameObject(), x, y, z, o);
                break;
            case TYPEID_DYNAMICOBJECT:
                //TODO GetMap()->DynamicObjectRelocation(passenger->ToDynObject(), x, y, z, o);
                break;
            default:
                break;
        }
    }
}

void Transport::DoEventIfAny(KeyFrame const& node, bool departure)
{
    if (uint32 eventid = departure ? node.Node->departureEventID : node.Node->arrivalEventID)
    {
        TC_LOG_DEBUG("maps.script", "Taxi %s event %u of node %u of %s path", departure ? "departure" : "arrival", eventid, node.Node->index, GetName().c_str());
//        GetMap()->ScriptsStart(sEventScripts, eventid, this, this);
//        EventInform(eventid);
    }
}

void Transport::CleanupsBeforeDelete(bool finalCleanup /*= true*/)
{
    UnloadStaticPassengers();
    while (!_passengers.empty())
    {
        WorldObject* obj = *_passengers.begin();
        RemovePassenger(obj);
    }

    GameObject::CleanupsBeforeDelete(finalCleanup);
}

void Transport::JustDocked()
{
    switch (GetEntry())
    {
        case 176495:
        case 164871:
        case 175080:
            SendPlaySound(11804, false); break;     // ZeppelinDocked
        case 20808:
        case 181646:
        case 176231:
        case 176244:
        case 176310:
        case 177233:
            //SendPlaySound(5495, false);break;       // BoatDockingWarning
        default:
            SendPlaySound(5154, false); break;      // ShipDocked
    }
}

void Transport::BuildUpdate(UpdateDataMapType& data_map)
{
    Map::PlayerList const& players = GetMap()->GetPlayers();
    if (players.isEmpty())
        return;

    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
        BuildFieldsUpdate(itr->GetSource(), data_map);

    ClearUpdateMask(true);
}
