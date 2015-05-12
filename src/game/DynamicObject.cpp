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
#include "GameObject.h"
#include "UpdateMask.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectAccessor.h"
#include "Database/DatabaseEnv.h"
#include "SpellAuras.h"
#include "MapManager.h"
#include "GridNotifiers.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Transport.h"

DynamicObject::DynamicObject() : WorldObject()
{
    m_aliveDuration = 0;
    m_casterGuid = 0;
    m_updateTimer = 0;
    m_spellId = 0;
    m_nextThinkTime = 0;
    m_radius = 0.0f;
    m_effIndex = 0;

    m_objectType |= TYPEMASK_DYNAMICOBJECT;
    m_objectTypeId = TYPEID_DYNAMICOBJECT;
                                                            // 2.3.2 - 0x58
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_STATIONARY_POSITION);

    m_valuesCount = DYNAMICOBJECT_END;
}

void DynamicObject::AddToWorld()
{
    ///- Register the dynamicObject for guid lookup
    if(!IsInWorld())
    {
        sObjectAccessor->AddObject(this);
        WorldObject::AddToWorld();
    }
}

void DynamicObject::RemoveFromWorld()
{
    ///- Remove the dynamicObject from the accessor
    if(IsInWorld())
    {
        sObjectAccessor->RemoveObject(this);
        WorldObject::RemoveFromWorld();
        if(GetTransport())
            GetTransport()->RemovePassenger(this);
    }
}

bool DynamicObject::Create( uint32 guidlow, Unit *caster, uint32 spellId, uint32 effIndex, float x, float y, float z, int32 duration, float radius )
{
    Position pos { x,y,z,0.0f };
    SetInstanceId(caster->GetInstanceId());

    WorldObject::_Create(guidlow, HIGHGUID_DYNAMICOBJECT, caster->GetMapId());
    Relocate(pos);

    if(!IsPositionValid())
    {
        TC_LOG_ERROR("FIXME","ERROR: DynamicObject (spell %u eff %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)",spellId,effIndex,GetPositionX(),GetPositionY());
        return false;
    }

    float visualRadius = radius;
    // For some reason visual size in client seems incorrect for some spells. Can't seem to find the proper rule.
    if(SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId))
    {
        if (spellInfo->GetMaxRange() == 0 || spellInfo->HasVisual(10383))  //Personal range. Ice trap, consecration... + (Flamestrike : 10383)
            visualRadius = radius * 2.2;
        
        else if (spellInfo->Id == 45848)
            visualRadius = radius * 0.4;
    }

    SetEntry(spellId);
    SetFloatValue( OBJECT_FIELD_SCALE_X, 1 );
    SetUInt64Value( DYNAMICOBJECT_CASTER, caster->GetGUID() );
    SetUInt32Value( DYNAMICOBJECT_BYTES, 0x00000001 );
    SetUInt32Value( DYNAMICOBJECT_SPELLID, spellId );
    SetFloatValue( DYNAMICOBJECT_RADIUS, visualRadius);
    SetFloatValue( DYNAMICOBJECT_POS_X, x );
    SetFloatValue( DYNAMICOBJECT_POS_Y, y );
    SetFloatValue( DYNAMICOBJECT_POS_Z, z );
    SetUInt32Value( DYNAMICOBJECT_CASTTIME, GetMSTime() );  // new 2.4.0

    if (Transport* transport = caster->GetTransport())
    {
        SetTransport(transport);
        float x, y, z, o;
        pos.GetPosition(x, y, z, o);
        transport->CalculatePassengerOffset(x, y, z, &o);
        m_movementInfo.transport.pos.Relocate(x, y, z, o);

        // This object must be added to transport before adding to map for the client to properly display it
        transport->AddPassenger(this);
    }

    m_aliveDuration = duration;
    m_radius = radius;
    m_effIndex = effIndex;
    m_spellId = spellId;
    m_casterGuid = caster->GetGUID();
    m_updateTimer = 0;
    return true;
}

Unit* DynamicObject::GetCaster() const
{
    // can be not found in some cases
    return ObjectAccessor::GetUnit(*this,m_casterGuid);
}

void DynamicObject::Update(uint32 p_time)
{
    // caster can be not in world at time dynamic object update, but dynamic object not yet deleted in Unit destructor
    Unit* caster = GetCaster();
    if(!caster)
    {
        Delete();
        return;
    }

    bool deleteThis = false;

    if(m_aliveDuration > int32(p_time))
        m_aliveDuration -= p_time;
    else
        deleteThis = true;

    if(m_effIndex < 4)
    {
        if(m_updateTimer < p_time)
        {
            Trinity::DynamicObjectUpdater notifier(*this,caster);
            VisitNearbyObject(GetRadius(), notifier);
            m_updateTimer = 500; // is this official-like?
        }else m_updateTimer -= p_time;
    }

    if(deleteThis)
    {
        caster->RemoveDynObjectWithGUID(GetGUID());
        Delete();
    }
}

void DynamicObject::Delete()
{
    SendObjectDeSpawnAnim(GetGUID());
    AddObjectToRemoveList();
}

void DynamicObject::Delay(int32 delaytime)
{
    m_aliveDuration -= delaytime;
    for(AffectedSet::iterator iunit= m_affected.begin();iunit != m_affected.end();++iunit)
        if (*iunit)
            (*iunit)->DelayAura(m_spellId, m_effIndex, delaytime);
}

bool DynamicObject::IsVisibleForInState(Player const* u, bool inVisibleList) const
{
    return IsInWorld() && u->IsInWorld()
        && (IsWithinDistInMap(u,World::GetMaxVisibleDistanceForObject()+(inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f), false)
        || GetCasterGUID() == u->GetGUID());
}

void DynamicObject::AddAffected(Unit *unit)
{
    m_affected.insert(unit);

    // Hacky way
    switch (m_spellId)
    {
        case 45848:    // Shield of the Blue
            unit->RemoveAurasDueToSpell(45641);
            unit->RemoveAurasDueToSpell(45737);
            break;
    }
}

