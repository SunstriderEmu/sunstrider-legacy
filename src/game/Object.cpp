/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "Common.h"
#include "SharedDefines.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "Object.h"
#include "Creature.h"
#include "Player.h"
#include "Totem.h"
#include "ObjectMgr.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "UpdateMask.h"
#include "Util.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Log.h"
#include "Transport.h"
#include "TargetedMovementGenerator.h"
#include "WaypointMovementGenerator.h"
#include "Management/VMapFactory.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "MovementPacketBuilder.h"
#include "MapInstanced.h"
#include "UpdateFieldFlags.h"

#include "TemporarySummon.h"
#include "DynamicTree.h"

uint32 GuidHigh2TypeId(uint32 guid_hi)
{
    switch(guid_hi)
    {
        case HIGHGUID_ITEM:         return TYPEID_ITEM;
        //case HIGHGUID_CONTAINER:    return TYPEID_CONTAINER; HIGHGUID_CONTAINER==HIGHGUID_ITEM currently
        case HIGHGUID_UNIT:         return TYPEID_UNIT;
        case HIGHGUID_PET:          return TYPEID_UNIT;
        case HIGHGUID_PLAYER:       return TYPEID_PLAYER;
        case HIGHGUID_GAMEOBJECT:   return TYPEID_GAMEOBJECT;
        case HIGHGUID_DYNAMICOBJECT:return TYPEID_DYNAMICOBJECT;
        case HIGHGUID_CORPSE:       return TYPEID_CORPSE;
        case HIGHGUID_MO_TRANSPORT: return TYPEID_GAMEOBJECT;
        case HIGHGUID_TRANSPORT:    return TYPEID_GAMEOBJECT;
    }
    return 10;                                              // unknown
}

Object::Object() : 
    m_PackGUID(sizeof(uint64)+1)
{
    m_objectTypeId      = TYPEID_OBJECT;
    m_objectType        = TYPEMASK_OBJECT;
    m_updateFlag        = UPDATEFLAG_NONE;

    m_uint32Values      = nullptr;
    m_valuesCount       = 0;
    _fieldNotifyFlags   = UF_FLAG_DYNAMIC;

    m_inWorld           = false;
    m_objectUpdated     = false;
}

Object::~Object( )
{
    //if(m_objectUpdated)
    //    sObjectAccessor->RemoveUpdateObject(this);

    if(m_uint32Values)
    {
        if(IsInWorld())
        {
            ///- Do NOT call RemoveFromWorld here, if the object is a player it will crash
            TC_LOG_ERROR("misc","Object::~Object - guid=" UI64FMTD ", typeid=%d deleted but still in world!!", GetGUID(), GetTypeId());
            ABORT();
        }

        if (m_objectUpdated)
        {
            TC_LOG_FATAL("misc", "Object::~Object " UI64FMTD " deleted but still in update list!!", GetGUID());
            ABORT();
        }

        delete [] m_uint32Values;
        m_uint32Values = NULL;
    }
}

void Object::_InitValues()
{
    m_uint32Values = new uint32[ m_valuesCount ];
    memset(m_uint32Values, 0, m_valuesCount*sizeof(uint32));

    _changesMask.SetCount(m_valuesCount);

    m_objectUpdated = false;
}

void Object::_Create( uint32 guidlow, uint32 entry, HighGuid guidhigh )
{
    if(!m_uint32Values) _InitValues();

    uint64 guid = MAKE_NEW_GUID(guidlow, entry, guidhigh);  // required more changes to make it working
    SetUInt64Value( OBJECT_FIELD_GUID, guid );
    SetUInt32Value( OBJECT_FIELD_TYPE, m_objectType );
    m_PackGUID.Set(guid);
}

std::string Object::_ConcatFields(uint16 startIndex, uint16 size) const
{
    std::ostringstream ss;
    for (uint16 index = 0; index < size; ++index)
        ss << GetUInt32Value(index + startIndex) << ' ';
    return ss.str();
}

void Object::_LoadIntoDataField(std::string const& data, uint32 startOffset, uint32 count)
{
    if (data.empty())
        return;

    Tokenizer tokens(data, ' ', count);

    if (tokens.size() != count)
        return;

    for (uint32 index = 0; index < count; ++index)
    {
        m_uint32Values[startOffset + index] = atoul(tokens[index]);
        _changesMask.SetBit(startOffset + index);
    }
}

void Object::BuildMovementUpdateBlock(UpdateData* data, uint32 flags ) const
{
    ByteBuffer buf(500);

    buf << uint8( UPDATETYPE_MOVEMENT );
#ifdef LICH_KING
    buf << GetPackGUID();
#else
    buf << GetGUID();
#endif

    BuildMovementUpdate(&buf, flags);

    data->AddUpdateBlock(buf);
}

void Object::SendUpdateToPlayer(Player* player)
{
    // send create update to player
    UpdateData upd;
    WorldPacket packet;

    BuildCreateUpdateBlockForPlayer(&upd, player);
    upd.BuildPacket(&packet);
    player->SendDirectMessage(&packet);
}

void Object::BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    if(!target)
        return;

    uint8  updatetype = UPDATETYPE_CREATE_OBJECT;
    uint8  flags      = m_updateFlag;

    // lower flag1
    if(target == this)                                      // building packet for oneself
        flags |= UPDATEFLAG_SELF;

    if(flags & UPDATEFLAG_STATIONARY_POSITION)
    {
        // UPDATETYPE_CREATE_OBJECT2 dynamic objects, corpses...
        if(isType(TYPEMASK_DYNAMICOBJECT) || isType(TYPEMASK_CORPSE) || isType(TYPEMASK_PLAYER))
            updatetype = UPDATETYPE_CREATE_OBJECT2;

        // UPDATETYPE_CREATE_OBJECT2 for pets...
        if(target->GetPetGUID() == GetGUID())
            updatetype = UPDATETYPE_CREATE_OBJECT2;

        // UPDATETYPE_CREATE_OBJECT2 for some gameobject types...
        if(isType(TYPEMASK_GAMEOBJECT))
        {
            switch(((GameObject*)this)->GetGoType())
            {
                case GAMEOBJECT_TYPE_TRAP:
                case GAMEOBJECT_TYPE_DUEL_ARBITER:
                case GAMEOBJECT_TYPE_FLAGSTAND:
                case GAMEOBJECT_TYPE_FLAGDROP:
                    updatetype = UPDATETYPE_CREATE_OBJECT2;
                    break;
                case GAMEOBJECT_TYPE_TRANSPORT:
                    updatetype |= UPDATEFLAG_TRANSPORT;
                    break;
                default:
                    break;
            }
        }

        if (isType(TYPEMASK_UNIT))
        {
            if (ToUnit()->GetVictim())
                flags |= UPDATEFLAG_HAS_TARGET;
        }
    }

    //TC_LOG_DEBUG("misc","BuildCreateUpdate: update-type: %u, object-type: %u got flags: %X, flags2: %X", updatetype, m_objectTypeId, flags, flags2);

    ByteBuffer buf(500);
    buf << (uint8)updatetype;
#ifdef LICH_KING
    buf << GetPackGUID();
#else
    buf << (uint8)0xFF << GetGUID();
#endif
    buf << (uint8)m_objectTypeId;

    BuildMovementUpdate(&buf, flags);
    BuildValuesUpdate(updatetype, &buf, target );
    data->AddUpdateBlock(buf);
}

void Object::BuildValuesUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    ByteBuffer buf(500);

    buf << (uint8) UPDATETYPE_VALUES;
    if(target->GetSession()->GetClientBuild() == BUILD_335)
        buf << GetPackGUID();
    else
        buf << (uint8)0xFF << GetGUID();

    BuildValuesUpdate(UPDATETYPE_VALUES, &buf, target );

    data->AddUpdateBlock(buf);
}

void Object::BuildFieldsUpdate(Player* player, UpdateDataMapType& data_map) const
{
    UpdateDataMapType::iterator iter = data_map.find(player);
    if (iter == data_map.end())
    {
        std::pair<UpdateDataMapType::iterator, bool> p = data_map.emplace(player, UpdateData());
        ASSERT(p.second);
        iter = p.first;
    }

    BuildValuesUpdateBlockForPlayer(&iter->second, iter->first);
}

uint32 Object::GetUpdateFieldData(Player const* target, uint32*& flags) const
{
    uint32 visibleFlag = UF_FLAG_PUBLIC;

    if (target == this)
        visibleFlag |= UF_FLAG_PRIVATE;

    switch (GetTypeId())
    {
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
            flags = ItemUpdateFieldFlags;
            if (((Item const*)this)->GetOwnerGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER | UF_FLAG_ITEM_OWNER;
            break;
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
        {
            Player* plr = ToUnit()->GetCharmerOrOwnerPlayerOrPlayerItself();
            flags = UnitUpdateFieldFlags;
            if (ToUnit()->GetOwnerGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;

            if (HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_SPECIALINFO))
                if (ToUnit()->HasAuraTypeWithCaster(SPELL_AURA_EMPATHY, target->GetGUID()))
                    visibleFlag |= UF_FLAG_SPECIAL_INFO;

            if (plr && plr->IsInSameRaidWith(target))
                visibleFlag |= UF_FLAG_PARTY_MEMBER;
            break;
        }
        case TYPEID_GAMEOBJECT:
            flags = GameObjectUpdateFieldFlags;
            if (ToGameObject()->GetOwnerGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
            break;
        case TYPEID_DYNAMICOBJECT:
            flags = DynamicObjectUpdateFieldFlags;
            if (ToDynObject()->GetCasterGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
            break;
        case TYPEID_CORPSE:
            flags = CorpseUpdateFieldFlags;
            if (ToCorpse()->GetOwnerGUID() == target->GetGUID())
                visibleFlag |= UF_FLAG_OWNER;
            break;
        case TYPEID_OBJECT:
            break;
    }

    return visibleFlag;
}


void Object::BuildOutOfRangeUpdateBlock(UpdateData * data) const
{
    data->AddOutOfRangeGUID(GetGUID());
}

void Object::DestroyForPlayer(Player *target, bool onDeath /*= false*/) const
{
    ASSERT(target);

    WorldPacket data(SMSG_DESTROY_OBJECT, 8);
    data << GetGUID();
        /* LK
        //! If the following bool is true, the client will call "void CGUnit_C::OnDeath()" for this object.
        //! OnDeath() does for eg trigger death animation and interrupts certain spells/missiles/auras/sounds...
        data << uint8(onDeath ? 1 : 0);
        */
    target->SendDirectMessage( &data );
}

int32 Object::GetInt32Value(uint16 index) const
{
    ASSERT(index < m_valuesCount || PrintIndexError(index, false));
    return m_int32Values[index];
}

uint32 Object::GetUInt32Value(uint16 index) const
{
    ASSERT(index < m_valuesCount || PrintIndexError(index, false));
    return m_uint32Values[index];
}

uint64 Object::GetUInt64Value(uint16 index) const
{
    ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, false));
    return *((uint64*)&(m_uint32Values[index]));
}

float Object::GetFloatValue(uint16 index) const
{
    ASSERT(index < m_valuesCount || PrintIndexError(index, false));
    return m_floatValues[index];
}

uint8 Object::GetByteValue(uint16 index, uint8 offset) const
{
    ASSERT(index < m_valuesCount || PrintIndexError(index, false));
    ASSERT(offset < 4);
    return *(((uint8*)&m_uint32Values[index]) + offset);
}

uint16 Object::GetUInt16Value(uint16 index, uint8 offset) const
{
    ASSERT(index < m_valuesCount || PrintIndexError(index, false));
    ASSERT(offset < 2);
    return *(((uint16*)&m_uint32Values[index]) + offset);
}

uint64 Object::GetGuidValue(uint16 index) const
{
    ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, false));
    return *((uint64*)&(m_uint32Values[index]));
}

void Object::BuildValuesUpdate(uint8 updateType, ByteBuffer * data, Player *target) const
{
    if(!target)
        return;

    if (!target)
        return;

    ByteBuffer fieldBuffer;
    UpdateMask updateMask;
    updateMask.SetCount(m_valuesCount);

    uint32* flags = NULL;
    uint32 visibleFlag = GetUpdateFieldData(target, flags);
    ASSERT(flags);

    for (uint16 index = 0; index < m_valuesCount; ++index)
    {
        if (_fieldNotifyFlags & flags[index] ||
            ((updateType == UPDATETYPE_VALUES ? _changesMask.GetBit(index) : m_uint32Values[index]) && (flags[index] & visibleFlag)))
        {
            updateMask.SetBit(index);
            fieldBuffer << m_uint32Values[index];
        }
    }

    *data << uint8(updateMask.GetBlockCount());
    updateMask.AppendToPacket(data);
    data->append(fieldBuffer);
}

void Object::ClearUpdateMask(bool remove)
{
    _changesMask.Clear();

    if(m_objectUpdated)
    {
        if(remove)
            sObjectAccessor->RemoveUpdateObject(this);
        m_objectUpdated = false;
    }
}

bool Object::LoadValues(const char* data)
{
    if(!m_uint32Values) _InitValues();

    Tokens tokens = StrSplit(data, " ");

    if(tokens.size() != m_valuesCount)
        return false;

    Tokens::iterator iter;
    int index;
    for (iter = tokens.begin(), index = 0; index < m_valuesCount; ++iter, ++index)
    {
        m_uint32Values[index] = atol((*iter).c_str());
    }

    return true;
}

void Object::SetInt32Value( uint16 index, int32 value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(m_int32Values[ index ] != value)
    {
        m_int32Values[ index ] = value;
        _changesMask.SetBit(index);

        if(m_inWorld && !m_objectUpdated)
        {
            sObjectAccessor->AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

void Object::SetUInt32Value( uint16 index, uint32 value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(m_uint32Values[ index ] != value)
    {
        m_uint32Values[ index ] = value;
        _changesMask.SetBit(index);

        if(m_inWorld && !m_objectUpdated)
        {
            sObjectAccessor->AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

void Object::UpdateUInt32Value(uint16 index, uint32 value)
{
    ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    m_uint32Values[index] = value;
    _changesMask.SetBit(index);
}

void Object::SetUInt64Value(uint16 index, uint64 value)
{
    ASSERT( index + 1 < m_valuesCount || PrintIndexError( index , true ) );
    if(*((uint64*)&(m_uint32Values[ index ])) != value)
    {
        m_uint32Values[ index ] = PAIR64_LOPART(value);
        m_uint32Values[ index + 1 ] = PAIR64_HIPART(value);
        _changesMask.SetBit(index);
        _changesMask.SetBit(index + 1);

        if(m_inWorld && !m_objectUpdated)
        {
            sObjectAccessor->AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

void Object::SetFloatValue( uint16 index, float value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(m_floatValues[ index ] != value)
    {
        m_floatValues[ index ] = value;
        _changesMask.SetBit(index);

        if(m_inWorld && !m_objectUpdated)
        {
            sObjectAccessor->AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

void Object::SetByteValue( uint16 index, uint8 offset, uint8 value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(offset > 3)
    {
        TC_LOG_ERROR("misc","Object::SetByteValue: wrong offset %u", offset);
        return;
    }

    if(uint8(m_uint32Values[ index ] >> (offset * 8)) != value)
    {
        m_uint32Values[ index ] &= ~uint32(uint32(0xFF) << (offset * 8));
        m_uint32Values[ index ] |= uint32(uint32(value) << (offset * 8));
        _changesMask.SetBit(index);

        if(m_inWorld && !m_objectUpdated)
        {
            sObjectAccessor->AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

void Object::SetUInt16Value( uint16 index, uint8 offset, uint16 value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(offset > 1)
    {
        TC_LOG_ERROR("misc","Object::SetUInt16Value: wrong offset %u", offset);
        return;
    }

    if(uint8(m_uint32Values[ index ] >> (offset * 16)) != value)
    {
        m_uint32Values[ index ] &= ~uint32(uint32(0xFFFF) << (offset * 16));
        m_uint32Values[ index ] |= uint32(uint32(value) << (offset * 16));
        _changesMask.SetBit(index);

        if(m_inWorld && !m_objectUpdated)
        {
            sObjectAccessor->AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

void Object::SetGuidValue(uint16 index, uint64 value)
{
    ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, true));
    if (*((uint64*)&(m_uint32Values[index])) != value)
    {
        *((uint64*)&(m_uint32Values[index])) = value;
        _changesMask.SetBit(index);
        _changesMask.SetBit(index + 1);

        if (m_inWorld && !m_objectUpdated)
        {
            sObjectAccessor->AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

void Object::SetStatFloatValue( uint16 index, float value)
{
    if(value < 0)
        value = 0.0f;

    SetFloatValue(index, value);
}

void Object::SetStatInt32Value( uint16 index, int32 value)
{
    if(value < 0)
        value = 0;

    SetUInt32Value(index, uint32(value));
}

void Object::ApplyModUInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetUInt32Value(index);
    cur += (apply ? val : -val);
    if(cur < 0)
        cur = 0;
    SetUInt32Value(index,cur);
}

void Object::ApplyModInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetInt32Value(index);
    cur += (apply ? val : -val);
    SetInt32Value(index,cur);
}

void Object::ApplyModSignedFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    SetFloatValue(index,cur);
}

void Object::ApplyPercentModFloatValue(uint16 index, float val, bool apply)
{
    float value = GetFloatValue(index);
    ApplyPercentModFloatVar(value, val, apply);
    SetFloatValue(index, value);
}

void Object::ApplyModPositiveFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    if(cur < 0)
        cur = 0;
    SetFloatValue(index,cur);
}

void Object::SetFlag( uint16 index, uint32 newFlag )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );
    uint32 oldval = m_uint32Values[ index ];
    uint32 newval = oldval | newFlag;

    if(oldval != newval)
    {
        m_uint32Values[ index ] = newval;
        _changesMask.SetBit(index);

        if(m_inWorld && !m_objectUpdated)
        {
            sObjectAccessor->AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

void Object::RemoveFlag( uint16 index, uint32 oldFlag )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );
    uint32 oldval = m_uint32Values[ index ];
    uint32 newval = oldval & ~oldFlag;

    if(oldval != newval)
    {
        m_uint32Values[ index ] = newval;
        _changesMask.SetBit(index);

        if(m_inWorld && !m_objectUpdated)
        {
            sObjectAccessor->AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

void Object::ToggleFlag( uint16 index, uint32 flag)
{
    if(HasFlag(index, flag))
        RemoveFlag(index, flag);
    else
        SetFlag(index, flag);
}

bool Object::HasFlag( uint16 index, uint32 flag ) const
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , false ) );
    return (m_uint32Values[ index ] & flag) != 0;
}

void Object::SetByteFlag( uint16 index, uint8 offset, uint8 newFlag )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(offset > 3)
    {
        TC_LOG_ERROR("misc","Object::SetByteFlag: wrong offset %u", offset);
        return;
    }

    if(!(uint8(m_uint32Values[ index ] >> (offset * 8)) & newFlag))
    {
        m_uint32Values[ index ] |= uint32(uint32(newFlag) << (offset * 8));
        _changesMask.SetBit(index);

        if(m_inWorld && !m_objectUpdated)
        {
            sObjectAccessor->AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

void Object::RemoveByteFlag( uint16 index, uint8 offset, uint8 oldFlag )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(offset > 3)
    {
        TC_LOG_ERROR("FIXME","Object::RemoveByteFlag: wrong offset %u", offset);
        return;
    }

    if(uint8(m_uint32Values[ index ] >> (offset * 8)) & oldFlag)
    {
        m_uint32Values[ index ] &= ~uint32(uint32(oldFlag) << (offset * 8));
        _changesMask.SetBit(index);

        if(m_inWorld && !m_objectUpdated)
        {
            sObjectAccessor->AddUpdateObject(this);
            m_objectUpdated = true;
        }
    }
}

void Object::ToggleFlag( uint16 index, uint8 offset, uint8 flag )
{
    if(HasByteFlag(index, offset, flag))
        RemoveByteFlag(index, offset, flag);
    else
        SetByteFlag(index, offset, flag);
}

bool Object::HasByteFlag( uint16 index, uint8 offset, uint8 flag ) const
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , false ) );
    ASSERT( offset < 4 );
    return (((uint8*)&m_uint32Values[index])[offset] & flag) != 0;
}

void Object::ApplyModFlag( uint16 index, uint32 flag, bool apply)
{
    if(apply) SetFlag(index,flag); else RemoveFlag(index,flag);
}

void Object::SetFlag64( uint16 index, uint64 newFlag )
{
    uint64 oldval = GetUInt64Value(index);
    uint64 newval = oldval | newFlag;
    SetUInt64Value(index,newval);
}

void Object::RemoveFlag64( uint16 index, uint64 oldFlag )
{
    uint64 oldval = GetUInt64Value(index);
    uint64 newval = oldval & ~oldFlag;
    SetUInt64Value(index,newval);
}

void Object::ToggleFlag64( uint16 index, uint64 flag)
{
    if(HasFlag64(index, flag))
        RemoveFlag64(index, flag);
    else
        SetFlag64(index, flag);
}

bool Object::HasFlag64( uint16 index, uint64 flag ) const
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , false ) );
    return (GetUInt64Value( index ) & flag) != 0;
}

void Object::ApplyModFlag64( uint16 index, uint64 flag, bool apply)
{
    if(apply) SetFlag64(index,flag); else RemoveFlag64(index,flag);
}

bool Object::PrintIndexError(uint32 index, bool set) const
{
    TC_LOG_ERROR("misc", "Attempt %s non-existed value field: %u (count: %u) for object typeid: %u type mask: %u",
        (set ? "set value to" : "get value from"),index,m_valuesCount,GetTypeId(),m_objectType);

    // assert must fail after function call
    return false;
}

WorldObject::WorldObject() :
    LastUsedScriptID(0),
    lootingGroupLeaderGUID(0)
{
    m_positionX         = 0.0f;
    m_positionY         = 0.0f;
    m_positionZ         = 0.0f;
    m_orientation       = 0.0f;

    m_mapId             = 0;
    m_InstanceId        = 0;
    m_map               = NULL;

    m_name = "";

    m_groupLootTimer    = 0;
    lootingGroupLeaderGUID = 0;

    mSemaphoreTeleport  = false;

    m_isActive          = false;
    IsTempWorldObject   = false;

    m_transport = nullptr;
}

void WorldObject::SetWorldObject(bool on)
{
    if(!IsInWorld())
        return;
    
    GetMap()->AddObjectToSwitchList(this, on); 
}

void WorldObject::SetKeepActive( bool on )
{
    if(m_isActive == on)
        return;

    if(GetTypeId() == TYPEID_PLAYER)
        return;

    m_isActive = on;

    if(!IsInWorld())
        return;

    Map *map = FindMap();
    if(!map)
        return;

    if(on)
    {
        if(GetTypeId() == TYPEID_UNIT)
            map->AddToForceActive(this->ToCreature());
        else if(GetTypeId() == TYPEID_DYNAMICOBJECT)
            map->AddToForceActive((DynamicObject*)this);
    }
    else
    {
        if(GetTypeId() == TYPEID_UNIT)
            map->RemoveFromForceActive(this->ToCreature());
        else if(GetTypeId() == TYPEID_DYNAMICOBJECT)
            map->RemoveFromForceActive((DynamicObject*)this);
    }
}

void WorldObject::CleanupsBeforeDelete(bool /*finalCleanup*/)
{
    if (IsInWorld())
        RemoveFromWorld();
}

void WorldObject::_Create( uint32 guidlow, HighGuid guidhigh, uint32 mapid )
{
    Object::_Create(guidlow, 0, guidhigh);

    m_mapId = mapid;
}

uint32 WorldObject::GetZoneId() const
{
    return GetBaseMap()->GetZoneId(m_positionX,m_positionY,m_positionZ);
}

uint32 WorldObject::GetAreaId() const
{
    return GetBaseMap()->GetAreaId(m_positionX,m_positionY,m_positionZ);
}

InstanceScript* WorldObject::GetInstanceScript()
{
    Map *map = GetMap();
    return map->IsDungeon() ? ((InstanceMap*)map)->GetInstanceScript() : NULL;
}

                                                            //slow
float WorldObject::GetDistance(const WorldObject* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistance(const Position &pos) const
{
    float d = GetExactDist(&pos) - GetObjectSize();
    return d > 0.0f ? d : 0.0f;
}

float WorldObject::GetDistance2d(float x, float y) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float sizefactor = GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistanceSqr(float x, float y, float z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float sizefactor = GetObjectSize();
    float dist = dx*dx+dy*dy+dz*dz-sizefactor;
    return (dist > 0 ? dist : 0);
}

float WorldObject::GetExactDistance2d(const WorldObject* obj) const
{
    return GetExactDistance2d(obj->GetPositionX(),obj->GetPositionY());
}

float WorldObject::GetExactDistance2d(const float x, const float y) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    return sqrt((dx*dx) + (dy*dy));
}

float WorldObject::GetDistance(const float x, const float y, const float z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float sizefactor = GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetExactDistance(const float x, const float y, const float z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz));
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistanceSq(const float &x, const float &y, const float &z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    return dx*dx + dy*dy + dz*dz;
}

float WorldObject::GetDistance2d(const WorldObject* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistanceZ(const WorldObject* obj) const
{
    float dz = fabs(GetPositionZ() - obj->GetPositionZ());
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float dist = dz - sizefactor;
    return ( dist > 0 ? dist : 0);
}

bool WorldObject::IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D /*= true*/) const
{
    return obj && _IsWithinDist(obj, dist2compare, is3D);
}

bool WorldObject::IsWithinDistInMap(const WorldObject* obj, const float dist2compare, const bool is3D) const
{
    return obj && IsInMap(obj) && InSamePhase(obj) && _IsWithinDist(obj, dist2compare, is3D);
}


bool WorldObject::IsWithinDist3d(float x, float y, float z, float dist) const
{
    return IsInDist(x, y, z, dist + GetObjectSize());
}

bool WorldObject::IsWithinDist3d(const Position* pos, float dist) const
{
    return IsInDist(pos, dist + GetObjectSize());
}

bool WorldObject::IsWithinDist2d(float x, float y, float dist) const
{
    return IsInDist2d(x, y, dist + GetObjectSize());
}

bool WorldObject::IsWithinDist2d(const Position* pos, float dist) const
{
    return IsInDist2d(pos, dist + GetObjectSize());
}

bool WorldObject::IsWithinLOSInMap(const WorldObject* obj) const
{
    if (!IsInMap(obj)) 
        return false;

    float ox,oy,oz;
    obj->GetPosition(ox,oy,oz);
    return IsWithinLOS(ox, oy, oz);
}

bool WorldObject::IsWithinLOS(const float ox, const float oy, const float oz ) const
{
    /*float x,y,z;
    GetPosition(x,y,z);
    VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
    return vMapManager->isInLineOfSight(GetMapId(), x, y, z+2.0f, ox, oy, oz+2.0f);*/
    if (IsInWorld())
        return GetMap()->isInLineOfSight(GetPositionX(), GetPositionY(), GetPositionZ()+2.f, ox, oy, oz+2.f);
    
    return true;
}

bool WorldObject::_IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D) const
{
    float sizefactor = GetObjectSize() + obj->GetObjectSize();
    float maxdist = dist2compare + sizefactor;

    if (GetTransport() && obj->GetTransport() && obj->GetTransport()->GetGUID() == GetTransport()->GetGUID())
    {
        float dtx = m_movementInfo.transport.pos.m_positionX - obj->m_movementInfo.transport.pos.m_positionX;
        float dty = m_movementInfo.transport.pos.m_positionY - obj->m_movementInfo.transport.pos.m_positionY;
        float disttsq = dtx * dtx + dty * dty;
        if (is3D)
        {
            float dtz = m_movementInfo.transport.pos.m_positionZ - obj->m_movementInfo.transport.pos.m_positionZ;
            disttsq += dtz * dtz;
        }
        return disttsq < (maxdist * maxdist);
    }

    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float distsq = dx*dx + dy*dy;
    if (is3D)
    {
        float dz = GetPositionZ() - obj->GetPositionZ();
        distsq += dz*dz;
    }

    return distsq < maxdist * maxdist;
}

bool WorldObject::GetDistanceOrder(WorldObject const* obj1, WorldObject const* obj2, bool is3D /* = true */) const
{
    float dx1 = GetPositionX() - obj1->GetPositionX();
    float dy1 = GetPositionY() - obj1->GetPositionY();
    float distsq1 = dx1*dx1 + dy1*dy1;
    if(is3D)
    {
        float dz1 = GetPositionZ() - obj1->GetPositionZ();
        distsq1 += dz1*dz1;
    }

    float dx2 = GetPositionX() - obj2->GetPositionX();
    float dy2 = GetPositionY() - obj2->GetPositionY();
    float distsq2 = dx2*dx2 + dy2*dy2;
    if(is3D)
    {
        float dz2 = GetPositionZ() - obj2->GetPositionZ();
        distsq2 += dz2*dz2;
    }

    return distsq1 < distsq2;
}

bool WorldObject::IsInRange(WorldObject const* obj, float minRange, float maxRange, bool is3D /* = true */) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float distsq = dx*dx + dy*dy;
    if (is3D)
    {
        float dz = GetPositionZ() - obj->GetPositionZ();
        distsq += dz*dz;
    }

    float sizefactor = GetObjectSize() + obj->GetObjectSize();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

void WorldObject::GetRandomPoint( const Position &pos, float distance, float &rand_x, float &rand_y, float &rand_z) const
{
    if (distance == 0)
    {
        pos.GetPosition(rand_x, rand_y, rand_z);
        return;
    }

    // angle to face `obj` to `this`
    float angle = (float)rand_norm()*static_cast<float>(2*M_PI);
    float new_dist = (float)rand_norm()*static_cast<float>(distance);

    rand_x = pos.m_positionX + new_dist * cos(angle);
    rand_y = pos.m_positionY + new_dist * sin(angle);
    rand_z = pos.m_positionZ;

    Trinity::NormalizeMapCoord(rand_x);
    Trinity::NormalizeMapCoord(rand_y);
    UpdateGroundPositionZ(rand_x,rand_y,rand_z);            // update to LOS height if available
}

void WorldObject::UpdateGroundPositionZ(float x, float y, float &z) const
{
    float new_z = sMapMgr->GetBaseMap(GetMapId())->GetHeight(x,y,z,true);
    if(new_z > INVALID_HEIGHT)
        z = new_z+ 0.05f;                                   // just to be sure that we are not a few pixel under the surface
}

void WorldObject::UpdateAllowedPositionZ(float x, float y, float &z, float maxDist) const
{
    bool waterWalk = false;
    bool canSwim = true;
    bool canFly = true;
    if(Unit const* u = ToUnit())
    {
        waterWalk = u->HasAuraType(SPELL_AURA_WATER_WALK);
        canFly = u->CanFly();
        canSwim = u->CanSwim();
    }

    Position pos = GetPosition();
    WorldObject::UpdateAllowedPositionZ(GetMapId(),x,y,z,canSwim,canFly,waterWalk,maxDist);
}

void WorldObject::UpdateAllowedPositionZ(uint32 mapId, float x, float y, float &z, bool canSwim, bool canFly, bool waterWalk, float maxDist)
{
    // non fly unit don't must be in air
    // non swim unit must be at ground (mostly speedup, because it don't must be in water and water level check less fast
    Map const* baseMap = (MapInstanced*)sMapMgr->GetBaseMap(mapId);
    if (!canFly)
    {
        float ground_z = z;
        if(canSwim || waterWalk)
            baseMap->GetWaterOrGroundLevel(x, y, z, &ground_z, true);
        else
            ground_z = baseMap->GetHeight(PhaseMask(1), x, y, z, true, DEFAULT_HEIGHT_SEARCH);

        if (ground_z == INVALID_HEIGHT)
            return;

        if (fabs(z - ground_z) < maxDist) //if difference is within max dist
            z = ground_z;
        //else, no ground found, keep the z position as is
    }
    else
    {
        float ground_z = baseMap->GetHeight(PhaseMask(1), x, y, z, true, DEFAULT_HEIGHT_SEARCH);
        if (z < ground_z && std::fabs(z - ground_z) <= maxDist)
            z = ground_z;
    }
}

bool WorldObject::IsPositionValid() const
{
    return Trinity::IsValidMapCoord(m_positionX,m_positionY,m_positionZ,m_orientation);
}

void WorldObject::SendPlaySound(uint32 Sound, bool OnlySelf)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << Sound;
    if (OnlySelf && GetTypeId() == TYPEID_PLAYER )
        (this->ToPlayer())->SendDirectMessage( &data );
    else
        SendMessageToSet( &data, true ); // ToSelf ignored in this case
}

void Object::ForceValuesUpdateAtIndex(uint32 i)
{
    _changesMask.SetBit(i);
    if(m_inWorld && !m_objectUpdated)
    {
        sObjectAccessor->AddUpdateObject(this);
        m_objectUpdated = true;
    }
}

void WorldObject::BuildHeartBeatMsg(WorldPacket *data) const
{
    //Heartbeat message cannot be used for non-units
    if (!isType(TYPEMASK_UNIT))
        return;

    data->Initialize(MSG_MOVE_HEARTBEAT, 32);
    *data << GetPackGUID();
    *data << uint32(((Unit*)this)->GetUnitMovementFlags()); // movement flags
    *data << uint8(0);                                      // 2.3.0
    *data << GetMSTime();                                   // time
    *data << m_positionX;
    *data << m_positionY;
    *data << m_positionZ;
    *data << m_orientation;
    *data << uint32(0);
}

void WorldObject::SendMessageToSet(WorldPacket *data, bool /*fake*/, bool bToPossessor)
{
    Map* map = sMapMgr->CreateMap(m_mapId, this);
    if (!map)
        return;

    sMapMgr->CreateMap(m_mapId, this)->MessageBroadcast(this, data, bToPossessor);
}

void WorldObject::SendMessageToSet(WorldPacket* data, Player* skipped_rcvr)
{
    assert(skipped_rcvr);
    sMapMgr->CreateMap(m_mapId, this)->MessageBroadcast(skipped_rcvr, data, false, false);
}

void WorldObject::SendMessageToSetInRange(WorldPacket *data, float dist, bool /*bToSelf*/, bool bToPossessor)
{
    sMapMgr->CreateMap(m_mapId, this)->MessageDistBroadcast(this, data, dist, bToPossessor);
}

void WorldObject::SendObjectDeSpawnAnim(uint64 guid)
{
    WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
    data << guid;
    SendMessageToSet(&data, true);
}

Map* WorldObject::_getMap()
{
    return m_map = sMapMgr->CreateMap(GetMapId(), this);
}

Map* WorldObject::FindBaseMap()
{
    return m_map = sMapMgr->FindMap(GetMapId(), GetInstanceId());
}

Map const* WorldObject::GetBaseMap() const
{
    return sMapMgr->GetBaseMap(GetMapId());
}

void WorldObject::AddObjectToRemoveList()
{
    assert(m_uint32Values);

    Map* map = FindMap();
    if(!map)
    {
        TC_LOG_ERROR("FIXME","Object (TypeId: %u Entry: %u GUID: %u) at attempt add to move list not have valid map (Id: %u).",GetTypeId(),GetEntry(),GetGUIDLow(),GetMapId());
        return;
    }

    map->AddObjectToRemoveList(this);
}

float WorldObject::GetVisibilityRange() const
{
    if (isActiveObject() && !ToPlayer())
        return MAX_VISIBILITY_DISTANCE;
    else if (GetTypeId() == TYPEID_GAMEOBJECT)
#ifdef LICH_KING
        return IsInWintergrasp() ? VISIBILITY_DIST_WINTERGRASP + VISIBILITY_INC_FOR_GOBJECTS : GetMap()->GetVisibilityRange() + VISIBILITY_INC_FOR_GOBJECTS;
    else
        return IsInWintergrasp() ? VISIBILITY_DIST_WINTERGRASP : GetMap()->GetVisibilityRange();
#else
        return GetMap()->GetVisibilityRange() + VISIBILITY_INC_FOR_GOBJECTS;
    else
        return GetMap()->GetVisibilityRange();
#endif
}

Creature* WorldObject::SummonCreature(uint32 id, float x, float y, float z, float ang, TempSummonType spwtype,uint32 despwtime) const
{
    TemporarySummon* pCreature = new TemporarySummon(GetGUID());

    if (!pCreature->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT,true), GetMap(), id))
    {
        delete pCreature;
        return NULL;
    }

    if (x == 0.0f && y == 0.0f && z == 0.0f)
        GetClosePoint(x, y, z, pCreature->GetObjectSize());

    pCreature->Relocate(x, y, z, ang);

    if(!pCreature->IsPositionValid())
    {
        TC_LOG_ERROR("FIXME","ERROR: Creature (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)",pCreature->GetGUIDLow(),pCreature->GetEntry(),pCreature->GetPositionX(),pCreature->GetPositionY());
        delete pCreature;
        return NULL;
    }

    pCreature->SetHomePosition(x, y, z, ang);
    pCreature->Summon(spwtype, despwtime);
    
    //script hooks
    if(pCreature->AI())
        pCreature->AI()->IsSummonedBy(((Unit*)this)->ToUnit());

    if(GetTypeId()==TYPEID_UNIT && (this->ToCreature())->IsAIEnabled) 
        (((Unit*)this)->ToCreature())->AI()->JustSummoned(pCreature);
    // --

    if((pCreature->GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER) && pCreature->m_spells[0])
    {
        if(GetTypeId() == TYPEID_UNIT || GetTypeId() == TYPEID_PLAYER)
            pCreature->SetFaction(((Unit*)this)->GetFaction());
        pCreature->CastSpell(pCreature, pCreature->m_spells[0], false, 0, 0, GetGUID());
    }

    // allow summoned creatures to keep grids active for 2 minutes, so that we may use AI summoning creatures far away and coming to them
    pCreature->SetKeepActiveTimer(2 * MINUTE * IN_MILLISECONDS);

    //return the creature therewith the summoner has access to it
    return pCreature;
}

Pet* Player::SummonPet(uint32 entry, float x, float y, float z, float ang, PetType petType, uint32 duration)
{
    Pet* pet = new Pet(petType);

    if(petType == SUMMON_PET && pet->LoadPetFromDB(this, entry))
    {
        // Remove Demonic Sacrifice auras (known pet)
        Unit::AuraList const& auraClassScripts = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for(Unit::AuraList::const_iterator itr = auraClassScripts.begin();itr!=auraClassScripts.end();)
        {
            if((*itr)->GetModifier()->m_miscvalue==2228)
            {
                RemoveAurasDueToSpell((*itr)->GetId());
                itr = auraClassScripts.begin();
            }
            else
                ++itr;
        }

        if(duration > 0)
            pet->SetDuration(duration);

        return NULL;
    }

    // petentry==0 for hunter "call pet" (current pet summoned if any)
    if(!entry)
    {
        delete pet;
        return NULL;
    }

    Map *map = GetMap();
    uint32 pet_number = sObjectMgr->GeneratePetNumber();
    if(!pet->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_PET), map, entry, pet_number))
    {
        TC_LOG_ERROR("FIXME","no such creature entry %u", entry);
        delete pet;
        return NULL;
    }

    pet->Relocate(x, y, z, ang);

    if(!pet->IsPositionValid())
    {
        TC_LOG_ERROR("FIXME","ERROR: Pet (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)",pet->GetGUIDLow(),pet->GetEntry(),pet->GetPositionX(),pet->GetPositionY());
        delete pet;
        return NULL;
    }

    pet->SetOwnerGUID(GetGUID());
    pet->SetCreatorGUID(GetGUID());
    pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, GetFaction());

    // this enables pet details window (Shift+P)
    pet->GetCharmInfo()->SetPetNumber(pet_number, false);

    pet->AIM_Initialize();

    map->Add(pet->ToCreature(), true);

    pet->SetPowerType(POWER_MANA);
    pet->SetUInt32Value(UNIT_NPC_FLAGS , 0);
    pet->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    pet->InitStatsForLevel(GetLevel());

    switch(petType)
    {
        case GUARDIAN_PET:
        case POSSESSED_PET:
            pet->SetUInt32Value(UNIT_FIELD_FLAGS,0);
            AddGuardian(pet);
            break;
        case SUMMON_PET:
            pet->SetUInt32Value(UNIT_FIELD_BYTES_0, 2048);
            pet->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
            pet->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, 1000);
            pet->SetHealth(pet->GetMaxHealth());
            pet->SetPower(POWER_MANA, pet->GetMaxPower(POWER_MANA));
            pet->InitPetCreateSpells();
            pet->SavePetToDB(PET_SAVE_AS_CURRENT);
            SetPet(pet);
            PetSpellInitialize();
            break;
        default:
            break;
    }

    if(petType == SUMMON_PET)
    {
        // Remove Demonic Sacrifice auras (known pet)
        Unit::AuraList const& auraClassScripts = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for(Unit::AuraList::const_iterator itr = auraClassScripts.begin();itr!=auraClassScripts.end();)
        {
            if((*itr)->GetModifier()->m_miscvalue==2228)
            {
                RemoveAurasDueToSpell((*itr)->GetId());
                itr = auraClassScripts.begin();
            }
            else
                ++itr;
        }
    }

    if(duration > 0)
        pet->SetDuration(duration);
    
    //scripts hooks
    if(pet->AI())
        pet->AI()->IsSummonedBy(this->ToUnit());

    if(GetTypeId()==TYPEID_UNIT && (this->ToCreature())->IsAIEnabled) 
        (this->ToCreature())->AI()->JustSummoned(pet);

    return pet;
}

Pet* Unit::SummonPet(uint32 entry, float x, float y, float z, float ang, uint32 duration)
{
    PetType petType = SUMMON_PET;
    Pet* pet = new Pet(petType);

    // petentry==0 for hunter "call pet" (current pet summoned if any)
    if(!entry)
    {
        delete pet;
        return NULL;
    }

    Map *map = GetMap();
    uint32 pet_number = sObjectMgr->GeneratePetNumber();
    if(!pet->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_PET), map, entry, pet_number))
    {
        TC_LOG_ERROR("FIXME","no such creature entry %u", entry);
        delete pet;
        return NULL;
    }

    pet->Relocate(x, y, z, ang);

    if(!pet->IsPositionValid())
    {
        TC_LOG_ERROR("FIXME","ERROR: Pet (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)",pet->GetGUIDLow(),pet->GetEntry(),pet->GetPositionX(),pet->GetPositionY());
        delete pet;
        return NULL;
    }

    pet->SetOwnerGUID(GetGUID());
    pet->SetCreatorGUID(GetGUID());
    pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, GetFaction());

    pet->AIM_Initialize();

    map->Add(pet->ToCreature());

    pet->SetPowerType(POWER_MANA);
    pet->SetUInt32Value(UNIT_NPC_FLAGS , 0);
    pet->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    pet->InitStatsForLevel(GetLevel());

    switch(petType)
    {
        case SUMMON_PET:
            pet->SetUInt32Value(UNIT_FIELD_BYTES_0, 2048);
            pet->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
            pet->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, 1000);
            pet->SetHealth(pet->GetMaxHealth());
            pet->SetPower(POWER_MANA, pet->GetMaxPower(POWER_MANA));
            pet->InitPetCreateSpells();
            SetPet(pet);
            break;
        default:
            break;
    }

    if(duration > 0)
        pet->SetDuration(duration);

    return pet;
}

GameObject* WorldObject::SummonGameObject(uint32 entry, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 respawnTime) const
{
    if(!IsInWorld())
        return NULL;

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);
    if(!goinfo)
    {
        TC_LOG_ERROR("FIXME","Gameobject template %u not found in database!", entry);
        return NULL;
    }
    Map *map = GetMap();
    GameObject* go = sObjectMgr->IsGameObjectStaticTransport(entry) ? new StaticTransport() : new GameObject();

    if(!go->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_GAMEOBJECT,true),entry,map,x,y,z,ang,rotation0,rotation1,rotation2,rotation3,100, GO_STATE_READY))
    {
        delete go;
        return NULL;
    }
    go->SetRespawnTime(respawnTime);
    if(GetTypeId()==TYPEID_PLAYER || GetTypeId()==TYPEID_UNIT) //not sure how to handle this
        ((Unit*)this)->AddGameObject(go);
    else
        go->SetSpawnedByDefault(false);
    map->Add(go);

    return go;
}

Creature* WorldObject::SummonTrigger(float x, float y, float z, float ang, uint32 duration, CreatureAI* (*GetAI)(Creature*))
{
    TempSummonType summonType = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;
    Creature* summon = SummonCreature(WORLD_TRIGGER, x, y, z, ang, summonType, duration);
    if(!summon)
        return NULL;

    //summon->SetName(GetName());
    if(GetTypeId()==TYPEID_PLAYER || GetTypeId()==TYPEID_UNIT)
    {
        summon->SetFaction(((Unit*)this)->GetFaction());
        summon->SetLevel(((Unit*)this)->GetLevel());
    }

    if(GetAI)
        summon->AIM_Initialize(GetAI(summon));
    return summon;
}

Creature* WorldObject::FindNearestCreature(uint32 entry, float range, bool alive) const
{
       Creature *creature = NULL;
       Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck checker(*this, entry, alive, range);
       Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(creature, checker);
       VisitNearbyObject(range, searcher);
       return creature;
}

GameObject* WorldObject::FindNearestGameObject(uint32 entry, float range) const
{
    GameObject *go = NULL;
    Trinity::NearestGameObjectEntryInObjectRangeCheck checker(*this, entry, range);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> searcher(this, go, checker);
    VisitNearbyGridObject(range, searcher);
    return go;
}

GameObject* WorldObject::FindNearestGameObjectOfType(GameobjectTypes type, float range) const
{
    GameObject* go = NULL;
    Trinity::NearestGameObjectTypeInObjectRangeCheck checker(*this, type, range);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectTypeInObjectRangeCheck> searcher(this, go, checker);
    VisitNearbyGridObject(range, searcher);
    return go;
}

Player* WorldObject::FindNearestPlayer(float range) const
{
       Player* pl = NULL;
       Trinity::AnyPlayerInObjectRangeCheck checker(this, range);
       Trinity::PlayerSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(this, pl, checker);
       VisitNearbyObject(range, searcher);
       return pl;
}

void WorldObject::GetNearPoint2D(float &x, float &y, float distance2d, float absAngle ) const
{
    x = GetPositionX() + (GetObjectSize() + distance2d) * cos(absAngle);
    y = GetPositionY() + (GetObjectSize() + distance2d) * sin(absAngle);

    Trinity::NormalizeMapCoord(x);
    Trinity::NormalizeMapCoord(y);
}

void WorldObject::GetNearPoint(WorldObject const* searcher, float &x, float &y, float &z, float searcher_size, float distance2d, float absAngle ) const
{
    GetNearPoint2D(x,y,distance2d+searcher_size,absAngle);
    z = GetPositionZ();
    UpdateAllowedPositionZ(x, y, z, searcher_size);
}

void WorldObject::GetGroundPoint(float &x, float &y, float &z, float dist, float angle)
{
    angle += GetOrientation();
    x += dist * cos(angle);
    y += dist * sin(angle);
    Trinity::NormalizeMapCoord(x);
    Trinity::NormalizeMapCoord(y);
    UpdateGroundPositionZ(x, y, z);
}

bool WorldObject::GetCollisionPosition(Position from, float x, float y, float z, Position& resultPos, float modifyDist)
{
    float vmapHit_x, vmapHit_y, vmapHit_z;
    float mapHit_x, mapHit_y, mapHit_z;

    bool colVMap = VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetMapId(), from.m_positionX, from.m_positionY, from.m_positionZ + 0.5f, x, y, z, vmapHit_x, vmapHit_y, vmapHit_z, modifyDist);
    bool colMap = GetMap()->getObjectHitPos(GetPhaseMask(), from.m_positionX, from.m_positionY, from.m_positionZ + 0.5f, x, y, z, mapHit_x, mapHit_y, mapHit_z, modifyDist);

    if (colVMap && !colMap)
    {
        resultPos.Relocate(vmapHit_x, vmapHit_y, vmapHit_z);
        return true;
    }
    if (!colVMap && colMap)
    {
        resultPos.Relocate(mapHit_x, mapHit_y, mapHit_z);
        return true;
    }
    if (colVMap && colMap)
    {
        if(GetDistance(vmapHit_x, vmapHit_y, vmapHit_z) < GetDistance(mapHit_x, mapHit_y, mapHit_z))
            resultPos.Relocate(vmapHit_x, vmapHit_y, vmapHit_z);
        else
            resultPos.Relocate(mapHit_x, mapHit_y, mapHit_z);

        return true;
    }

    resultPos.Relocate(x, y, z);
    //no collision at all
    return false;
}

// angle = relative angle from current orientation
Position WorldObject::GetFirstWalkableCollisionPosition(float dist, float angle, bool keepZ)
{
    Position pos = GetPosition();
    MovePositionToFirstWalkableCollision(pos, dist, angle, keepZ);
    return pos;
}

Position WorldObject::GetNearPosition(float dist, float angle)
{
    Position pos = GetPosition();
    MovePosition(pos, dist, angle);
    return pos;
}

// @todo: replace with WorldObject::UpdateAllowedPositionZ
float NormalizeZforCollision(WorldObject* obj, float x, float y, float z)
{
    float ground = obj->GetMap()->GetHeight(obj->GetPhaseMask(), x, y, z, true, MAX_HEIGHT);
    float floor = obj->GetMap()->GetHeight(obj->GetPhaseMask(), x, y, z + 2.0f, true);
    float helper = fabs(ground - z) <= fabs(floor - z) ? ground : floor;
    if (z > helper) // must be above ground
    {
        if (Unit* unit = obj->ToUnit())
        {
            if (unit->CanFly())
                return z;
        }
        LiquidData liquid_status;
        ZLiquidStatus res = obj->GetMap()->getLiquidStatus(x, y, z, BASE_LIQUID_TYPE_MASK_ALL, &liquid_status);
        if (res && liquid_status.level > helper) // water must be above ground
        {
            if (liquid_status.level > z) // z is underwater
                return z;
            else
                return fabs(liquid_status.level - z) <= fabs(helper - z) ? liquid_status.level : helper;
        }
    }
    return helper;
}

void WorldObject::MovePositionToFirstWalkableCollision(Position &pos, float dist, float angle, bool keepZ)
{
    angle += GetOrientation();
    float destx, desty, destz;
    destx = pos.m_positionX + dist * std::cos(angle);
    desty = pos.m_positionY + dist * std::sin(angle);
    
    // Prevent invalid coordinates here, position is unchanged
    if (!Trinity::IsValidMapCoord(destx, desty))
    {
        TC_LOG_ERROR("vmap","WorldObject::MovePositionToFirstWalkableCollision invalid coordinates X: %f and Y: %f were passed!", destx, desty);
        return;
    }
    
    destz = pos.m_positionZ;
    if(!keepZ)
        UpdateAllowedPositionZ(destx, desty, destz, dist);

    TC_LOG_DEBUG("vmap", "WorldObject::MovePositionToFirstWalkableCollision: Called with %f,%f. Target Z set to %f.", destx, desty, destz);
    
    // check static collision (terrain + WMO + MDX ?)
    bool col = VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetMapId(), pos.m_positionX, pos.m_positionY, pos.m_positionZ + 0.5f, destx, desty, destz + 0.5f, destx, desty, destz, -0.5f);

    // collision occured
    if (col)
    {
        // move back a bit
        destx -= CONTACT_DISTANCE * std::cos(angle);
        desty -= CONTACT_DISTANCE * std::sin(angle);
        dist = sqrt((pos.m_positionX - destx)*(pos.m_positionX - destx) + (pos.m_positionY - desty)*(pos.m_positionY - desty));
        TC_LOG_TRACE("vmap", "WorldObject::MovePositionToFirstWalkableCollision: Static collision occured at %f, %f, %f", destx, desty, destz);
    }

    // check dynamic collision
    col = GetMap()->getObjectHitPos(GetPhaseMask(), pos.m_positionX, pos.m_positionY, pos.m_positionZ+0.5f, destx, desty, destz+0.5f, destx, desty, destz, -0.5f);

    // Collided with a gameobject
    if (col)
    {
        destx -= CONTACT_DISTANCE * std::cos(angle);
        desty -= CONTACT_DISTANCE * std::sin(angle);
        dist = sqrt((pos.m_positionX - destx)*(pos.m_positionX - destx) + (pos.m_positionY - desty)*(pos.m_positionY - desty));
        TC_LOG_TRACE("vmap", "WorldObject::MovePositionToFirstWalkableCollision: Dynamic collision occured at %f, %f, %f", destx, desty, destz);
    }

    float step = dist / 10.0f;

    for (uint8 j = 0; j < 10; ++j)
    {
        // do not allow too big z changes. I too much changes, try again to get a position a little bit closer.
        if (fabs(pos.m_positionZ - destz) > 7.5f)
        {
            destx -= step * std::cos(angle);
            desty -= step * std::sin(angle);
            destz = pos.m_positionZ; //reset destz at each step before updating it
            if(!keepZ)
                UpdateAllowedPositionZ(destx, desty, destz, 50.0f);
        }
        // we have correct destz now
        else
        {
            pos.Relocate(destx, desty, destz);
            TC_LOG_TRACE("vmap", "WorldObject::MovePositionToFirstWalkableCollision: Found a suitable position after %u steps", j);
            break;
        }
    }

    Trinity::NormalizeMapCoord(pos.m_positionX);
    Trinity::NormalizeMapCoord(pos.m_positionY);
    if (!keepZ)
        UpdateAllowedPositionZ(destx, desty, pos.m_positionZ, dist);

    pos.SetOrientation(GetOrientation());

    TC_LOG_TRACE("vmap", "WorldObject::MovePositionToFirstWalkableCollision: Final target: %f, %f, %f", destx, desty, destz);
}

bool Position::HasInArc(float arc, const Position *obj, float border) const
{
    // always have self in arc
    if (obj == this)
        return true;

    // move arc to range 0.. 2*pi
    arc = Trinity::NormalizeOrientation(arc);

    float angle = GetAngle(obj);
    angle -= m_orientation;

    // move angle to range -pi ... +pi
    angle = Trinity::NormalizeOrientation(angle);
    if (angle > M_PI)
        angle -= 2.0f*M_PI;

    float lborder = -1 * (arc/border);                        // in range -pi..0
    float rborder = (arc/border);                             // in range 0..pi
    return ((angle >= lborder) && (angle <= rborder));
}

bool Position::operator==(Position const &a)
{
    return (G3D::fuzzyEq(a.m_positionX, m_positionX) &&
        G3D::fuzzyEq(a.m_positionY, m_positionY) &&
        G3D::fuzzyEq(a.m_positionZ, m_positionZ) &&
        G3D::fuzzyEq(a.m_orientation, m_orientation));
}

bool Position::HasInLine(const WorldObject* const target, float width) const
{
    if (!HasInArc(float(M_PI), target))
        return false;
    width += target->GetObjectSize();
    float angle = GetRelativeAngle(target);
    return fabs(sin(angle)) * GetExactDist2d(target->GetPositionX(), target->GetPositionY()) < width;
}

bool WorldObject::isInFront(WorldObject const* target,  float arc) const
{
    return HasInArc(arc, target);
}

bool WorldObject::isInBack(WorldObject const* target, float arc) const
{
    return !HasInArc(2 * M_PI - arc, target);
}

std::string Position::ToString() const
{
    std::stringstream sstr;
    sstr << "X: " << m_positionX << " Y: " << m_positionY << " Z: " << m_positionZ << " O: " << m_orientation;
    return sstr.str();
}

ByteBuffer &operator>>(ByteBuffer& buf, Position::PositionXYZOStreamer const & streamer)
{
    float x, y, z, o;
    buf >> x >> y >> z >> o;
    streamer.m_pos->Relocate(x, y, z, o);
    return buf;
}
ByteBuffer & operator<<(ByteBuffer& buf, Position::PositionXYZStreamer const & streamer)
{
    float x, y, z;
    streamer.m_pos->GetPosition(x, y, z);
    buf << x << y << z;
    return buf;
}

ByteBuffer &operator>>(ByteBuffer& buf, Position::PositionXYZStreamer const & streamer)
{
    float x, y, z;
    buf >> x >> y >> z;
    streamer.m_pos->Relocate(x, y, z);
    return buf;
}

ByteBuffer & operator<<(ByteBuffer& buf, Position::PositionXYZOStreamer const & streamer)
{
    float x, y, z, o;
    streamer.m_pos->GetPosition(x, y, z, o);
    buf << x << y << z << o;
    return buf;
}


void MovementInfo::Read(ByteBuffer& data)
{
    data >> flags;
    data >> flags2;
    data >> time;
    data >> pos.m_positionX;
    data >> pos.m_positionY;
    data >> pos.m_positionZ;
    data >> pos.m_orientation;

    if (HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
    {
        data >> transport.guid;
        data >> transport.pos.m_positionX;
        data >> transport.pos.m_positionY;
        data >> transport.pos.m_positionZ;
        data >> transport.pos.m_orientation;
        data >> transport.time;
    }

    if (HasMovementFlag(MovementFlags(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_PLAYER_FLYING)))
        data >> pitch;

    data >> fallTime;

    if (HasMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING))
    {
        data >> jump.zspeed;
        data >> jump.sinAngle;
        data >> jump.cosAngle;
        data >> jump.xyspeed;
    }

    if (HasMovementFlag(MOVEMENTFLAG_SPLINE_ELEVATION))
        data >> splineElevation;
}

void MovementInfo::Write(ByteBuffer& data) const
{
    data << flags;
    data << flags2;
    data << time;
    data << pos.GetPositionX();
    data << pos.GetPositionY();
    data << pos.GetPositionZ();
    data << pos.GetOrientation();

    if (HasMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
    {
        data << transport.guid;
        data << transport.pos.GetPositionX();
        data << transport.pos.GetPositionY();
        data << transport.pos.GetPositionZ();
        data << transport.pos.GetOrientation();
        data << transport.time;
    }

    if (HasMovementFlag(MovementFlags(MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_PLAYER_FLYING)))
        data << pitch;

    data << fallTime;

    if (HasMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING))
    {
        data << jump.zspeed;
        data << jump.sinAngle;
        data << jump.cosAngle;
        data << jump.xyspeed;
    }

    if (HasMovementFlag(MOVEMENTFLAG_SPLINE_ELEVATION))
        data << splineElevation;
}

Position WorldObject::GetRandomNearPosition(float radius)
{
    Position pos = GetPosition();
    MovePosition(pos, radius * (float)rand_norm(), (float)rand_norm() * static_cast<float>(2 * M_PI));
    return pos;
}

void Position::RelocateOffset(const Position & offset)
{
    m_positionX = GetPositionX() + (offset.GetPositionX() * cos(GetOrientation()) + offset.GetPositionY() * sin(GetOrientation() + M_PI));
    m_positionY = GetPositionY() + (offset.GetPositionY() * cos(GetOrientation()) + offset.GetPositionX() * sin(GetOrientation()));
    m_positionZ = GetPositionZ() + offset.GetPositionZ();
    m_orientation = GetOrientation() + offset.GetOrientation();
}

void Position::GetPositionOffsetTo(const Position & endPos, Position & retOffset) const
{
    float dx = endPos.GetPositionX() - GetPositionX();
    float dy = endPos.GetPositionY() - GetPositionY();

    retOffset.m_positionX = dx * cos(GetOrientation()) + dy * sin(GetOrientation());
    retOffset.m_positionY = dy * cos(GetOrientation()) - dx * sin(GetOrientation());
    retOffset.m_positionZ = endPos.GetPositionZ() - GetPositionZ();
    retOffset.m_orientation = endPos.GetOrientation() - GetOrientation();
}

float Position::GetAngle(const Position *obj) const
{
    if (!obj) 
        return 0;

    return GetAngle(obj->GetPositionX(), obj->GetPositionY());
}

// Return angle in range 0..2*pi
float Position::GetAngle(const float x, const float y) const
{
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();

    float ang = atan2(dy, dx);
    ang = (ang >= 0) ? ang : 2 * M_PI + ang;
    return ang;
}

void Position::GetSinCos(const float x, const float y, float &vsin, float &vcos) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;

    if (dx < 0.001f && dy < 0.001f)
    {
        float angle = (float)rand_norm()*static_cast<float>(2*M_PI);
        vcos = cos(angle);
        vsin = sin(angle);
    }
    else
    {
        float dist = sqrt((dx*dx) + (dy*dy));
        vcos = dx / dist;
        vsin = dy / dist;
    }
}

bool Position::IsPositionValid() const
{
    return Trinity::IsValidMapCoord(m_positionX, m_positionY, m_positionZ, m_orientation);
}

float WorldObject::GetObjectSize() const
{
    return ( m_valuesCount > UNIT_FIELD_COMBATREACH ) ? m_floatValues[UNIT_FIELD_COMBATREACH] : DEFAULT_WORLD_OBJECT_SIZE;
}

void WorldObject::GetCreatureListWithEntryInGrid(std::list<Creature*>& lList, uint32 uiEntry, float fMaxSearchRange) const
{
    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Trinity::AllCreaturesOfEntryInRange check((Unit const*)this, uiEntry, fMaxSearchRange);
    Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(this, lList, check);
    TypeContainerVisitor<Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange>, GridTypeMapContainer> visitor(searcher);

    cell.Visit(pair, visitor, *(this->GetMap()));
}

void WorldObject::GetGameObjectListWithEntryInGrid(std::list<GameObject*>& lList, uint32 uiEntry, float fMaxSearchRange) const
{
    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Trinity::AllGameObjectsWithEntryInRange check(this, uiEntry, fMaxSearchRange);
    Trinity::GameObjectListSearcher<Trinity::AllGameObjectsWithEntryInRange> searcher(this, lList, check);
    TypeContainerVisitor<Trinity::GameObjectListSearcher<Trinity::AllGameObjectsWithEntryInRange>, GridTypeMapContainer> visitor(searcher);

    cell.Visit(pair, visitor, *(this->GetMap()));
}

void Object::BuildMovementUpdate(ByteBuffer* data, uint16 flags) const
{
    Unit const* unit = NULL;
    WorldObject const* object = NULL;

    if (isType(TYPEMASK_UNIT))
        unit = ToUnit();
    else
        object = ((WorldObject*)this);

#ifdef LICH_KING
    *data << uint16(flags);                                  // update flags
#else
    *data << uint8(flags);                                  // update flags
#endif

    // 0x20
    if (flags & UPDATEFLAG_LIVING)
    {
        ASSERT(unit);
        unit->BuildMovementPacket(data);

        *data << unit->GetSpeed(MOVE_WALK)
              << unit->GetSpeed(MOVE_RUN)
              << unit->GetSpeed(MOVE_RUN_BACK)
              << unit->GetSpeed(MOVE_SWIM)
              << unit->GetSpeed(MOVE_SWIM_BACK)
              << unit->GetSpeed(MOVE_FLIGHT)
              << unit->GetSpeed(MOVE_FLIGHT_BACK)
              << unit->GetSpeed(MOVE_TURN_RATE);
#ifdef LICH_KING
              << unit->GetSpeed(MOVE_PITCH_RATE);
#endif

        // 0x08000000
        if (unit->m_movementInfo.GetMovementFlags() & MOVEMENTFLAG_SPLINE_ENABLED)
            Movement::PacketBuilder::WriteCreate(*unit->movespline, *data);
    }
    else
    {
#ifdef LICH_KING
        if (flags & UPDATEFLAG_POSITION)
        {
            Transport* transport = object->GetTransport();

            if (transport)
                data->append(transport->GetPackGUID());
            else
                *data << uint8(0);

            *data << object->GetPositionX();
            *data << object->GetPositionY();
            *data << object->GetPositionZ() + (unit ? unit->GetHoverHeight() : 0.0f);

            if (transport)
            {
                *data << object->GetTransOffsetX();
                *data << object->GetTransOffsetY();
                *data << object->GetTransOffsetZ();
            }
            else
            {
                *data << object->GetPositionX();
                *data << object->GetPositionY();
                *data << object->GetPositionZ() + (unit ? unit->GetHoverHeight() : 0.0f);
            }

            *data << object->GetOrientation();

            if (GetTypeId() == TYPEID_CORPSE)
                *data << float(object->GetOrientation());
            else
                *data << float(0);
        }
        else
#endif
        // 0x40
        if (flags & UPDATEFLAG_STATIONARY_POSITION)
        {
            ASSERT(object);
            // 0x02
            //X Y Z are at 0 for transports on bc for transports but whatever
            *data << object->GetStationaryX();
            *data << object->GetStationaryY();
            *data << object->GetStationaryZ() + (unit ? unit->GetHoverHeight() : 0.0f);
            *data << object->GetStationaryO();
        }
    }

#ifdef LICH_KING
    // 0x8
    if (flags & UPDATEFLAG_UNKNOWN)
    {
        *data << uint32(0);
    }
#endif


    // 0x08
    if (flags & UPDATEFLAG_LOWGUID)
    {
        switch (GetTypeId())
        {
            case TYPEID_OBJECT:
            case TYPEID_ITEM:
            case TYPEID_CONTAINER:
            case TYPEID_GAMEOBJECT:
            case TYPEID_DYNAMICOBJECT:
            case TYPEID_CORPSE:
                *data << uint32(GetGUIDLow());              // GetGUIDLow()
                break;
            //! Unit, Player and default here are sending wrong values.
            /// @todo Research the proper formula
            case TYPEID_UNIT:
                *data << uint32(0x0000000B);                // unk
                break;
            case TYPEID_PLAYER:
                if (flags & UPDATEFLAG_SELF)
#ifdef LICH_KING //not sure this change is needed (since we dunno what it does)
                    *data << uint32(0x0000002F);            // unk
#else
                    *data << uint32(0x00000015);            // unk, can be 0x15 or 0x22
#endif
                else
                    *data << uint32(0x00000008);            // unk
                break;
            default:
                *data << uint32(0x00000000);                // unk
                break;
        }
    }

#ifndef LICH_KING
    // 0x10
    if(flags & UPDATEFLAG_HIGHGUID)
    {
        switch(GetTypeId())
        {
            case TYPEID_OBJECT:
            case TYPEID_ITEM:
            case TYPEID_CONTAINER:
            case TYPEID_GAMEOBJECT:
            case TYPEID_DYNAMICOBJECT:
            case TYPEID_CORPSE:

                *data << uint32(GetGUIDHigh());             // GetGUIDHigh()
                break;
            default:
                *data << uint32(0x00000000);                // unk
                break;
        }
    }
#endif

    // 0x4
    if (flags & UPDATEFLAG_HAS_TARGET)
    {
        ASSERT(unit);
        if (Unit* victim = unit->GetVictim())
            *data << victim->GetPackGUID();
        else
            *data << uint8(0);
    }

    // 0x2
    if (flags & UPDATEFLAG_TRANSPORT)
    {
        GameObject const* go = ToGameObject();
        if (go && go->ToTransport())
            *data << uint32(go->ToTransport()->GetPathProgress());
        else 
            *data << uint32(0);
    }

#ifdef LICH_KING
    // 0x80
    if (flags & UPDATEFLAG_VEHICLE)
    {
        /// @todo Allow players to aquire this updateflag.
        *data << uint32(unit->GetVehicleKit()->GetVehicleInfo()->m_ID);
        if (unit->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
            *data << float(unit->GetTransOffsetO());
        else
            *data << float(unit->GetOrientation());
    }

    // 0x200
    if (flags & UPDATEFLAG_ROTATION)
        *data << int64(ToGameObject()->GetPackedWorldRotation());
#endif
}

void WorldObject::MovePosition(Position &pos, float dist, float angle)
{
    angle += GetOrientation();
    float destx, desty, destz, ground, floor;
    destx = pos.m_positionX + dist * std::cos(angle);
    desty = pos.m_positionY + dist * std::sin(angle);

    // Prevent invalid coordinates here, position is unchanged
    if (!Trinity::IsValidMapCoord(destx, desty, pos.m_positionZ))
    {
        TC_LOG_FATAL("misc", "WorldObject::MovePosition: Object (TypeId: %u Entry: %u GUID: %u) has invalid coordinates X: %f and Y: %f were passed!",
            GetTypeId(), GetEntry(), GetGUIDLow(), destx, desty);
        return;
    }

    ground = GetMap()->GetHeight(GetPhaseMask(), destx, desty, MAX_HEIGHT, true);
    floor = GetMap()->GetHeight(GetPhaseMask(), destx, desty, pos.m_positionZ, true);
    destz = fabs(ground - pos.m_positionZ) <= fabs(floor - pos.m_positionZ) ? ground : floor;

    float step = dist/10.0f;

    for (uint8 j = 0; j < 10; ++j)
    {
        // do not allow too big z changes
        if (fabs(pos.m_positionZ - destz) > 6)
        {
            destx -= step * std::cos(angle);
            desty -= step * std::sin(angle);
            ground = GetMap()->GetHeight(GetPhaseMask(), destx, desty, MAX_HEIGHT, true);
            floor = GetMap()->GetHeight(GetPhaseMask(), destx, desty, pos.m_positionZ, true);
            destz = fabs(ground - pos.m_positionZ) <= fabs(floor - pos.m_positionZ) ? ground : floor;
        }
        // we have correct destz now
        else
        {
            pos.Relocate(destx, desty, destz);
            break;
        }
    }

    Trinity::NormalizeMapCoord(pos.m_positionX);
    Trinity::NormalizeMapCoord(pos.m_positionY);
    UpdateGroundPositionZ(pos.m_positionX, pos.m_positionY, pos.m_positionZ);
    pos.SetOrientation(GetOrientation());
}

Position::Position(const WorldObject* obj) 
{ 
    Relocate(obj->GetPosition()); 
}

/** Fill UpdateData's for each player in range of given object */
struct WorldObjectChangeAccumulator
{
    UpdateDataMapType& i_updateDatas;
    UpdatePlayerSet& i_playerSet;
    WorldObject& i_object;
    WorldObjectChangeAccumulator(WorldObject &obj, UpdateDataMapType &d, UpdatePlayerSet &p) : i_updateDatas(d), i_object(obj), i_playerSet(p) 
    { 
        i_playerSet.clear();
    }

    void Visit(PlayerMapType &m)
    {
        Player* source = NULL;
        for (PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        {
            source = iter->GetSource();
            BuildPacket(source);

            //also add vision to player if he has shared vision with us
            if (!source->GetSharedVisionList().empty())
            {
                //has player shared vision with us ?
                 for (auto it : source->GetSharedVisionList())
                     if(Player* p = ObjectAccessor::GetPlayer(*source, it))
                         BuildPacket(p);
            }
        }
    }

    //add vision from creatures whom we share vision with
    void Visit(CreatureMapType &m)
    {
        Creature* source = NULL;
        for (CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        {
            source = iter->GetSource();
            if (!source->GetSharedVisionList().empty())
            {
                //has player shared vision with us ?
                for (auto it : source->GetSharedVisionList())
                     if(Player* p = ObjectAccessor::GetPlayer(*source, it))
                         BuildPacket(p);
            }
        }
    }

    //add vision from far sight dynamic objects
    void Visit(DynamicObjectMapType &m)
    {
        DynamicObject* source = NULL;
        for (DynamicObjectMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        {
            source = iter->GetSource();
            uint64 guid = source->GetCasterGUID();

            if (IS_PLAYER_GUID(guid))
            {
                //Caster may be NULL if DynObj is in removelist
                if (Player* caster = ObjectAccessor::FindPlayer(guid))
                    if (caster->GetGuidValue(PLAYER_FARSIGHT) == source->GetGUID())
                        BuildPacket(caster);
            }
        }
    }

    void BuildPacket(Player* player)
    {
        // Only send update once to a player
        if (i_playerSet.find(player->GetGUIDLow()) == i_playerSet.end() && player->HaveAtClient(&i_object))
        {
            i_object.BuildFieldsUpdate(player, i_updateDatas);
            i_playerSet.insert(player->GetGUIDLow());
        }
    }

    template<class SKIP> void Visit(GridRefManager<SKIP> &) { }
};

void WorldObject::BuildUpdate(UpdateDataMapType& data_map, UpdatePlayerSet& player_set)
{
    CellCoord p = Trinity::ComputeCellCoord(GetPositionX(), GetPositionY());
    Cell cell(p);
    cell.SetNoCreate();

    WorldObjectChangeAccumulator notifier(*this, data_map, player_set);
    TypeContainerVisitor<WorldObjectChangeAccumulator, WorldTypeMapContainer > player_notifier(notifier);
    Map& map = *GetMap();
    //we must build packets for all visible players
    cell.Visit(p, player_notifier, map, *this, GetVisibilityRange() + VISIBILITY_COMPENSATION);

    ClearUpdateMask(false);
}

uint64 WorldObject::GetTransGUID() const
{
    if (GetTransport())
        return GetTransport()->GetGUID();
    return 0;
}

uint64 Object::GetGUID() const { return GetUInt64Value(OBJECT_FIELD_GUID); }
uint32 Object::GetGUIDLow() const { return GUID_LOPART(GetUInt64Value(OBJECT_FIELD_GUID)); }
uint32 Object::GetGUIDMid() const { return GUID_ENPART(GetUInt64Value(OBJECT_FIELD_GUID)); }
uint32 Object::GetGUIDHigh() const { return GUID_HIPART(GetUInt64Value(OBJECT_FIELD_GUID)); }
PackedGuid const& Object::GetPackGUID() const { return m_PackGUID; }
uint32 Object::GetEntry() const { return GetUInt32Value(OBJECT_FIELD_ENTRY); }
void Object::SetEntry(uint32 entry) { SetUInt32Value(OBJECT_FIELD_ENTRY, entry); }

float Object::GetObjectScale() const { return GetFloatValue(OBJECT_FIELD_SCALE_X); }
void Object::SetObjectScale(float scale) { SetFloatValue(OBJECT_FIELD_SCALE_X, scale); }