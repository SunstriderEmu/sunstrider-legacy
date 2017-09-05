
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
#include <G3D/Vector3.h>
#include "ZoneScript.h"
#include "OutdoorPvpMgr.h"

#include "TemporarySummon.h"
#include "DynamicTree.h"

uint32 GuidHigh2TypeId(uint32 guid_hi)
{
    switch(HighGuid(guid_hi))
    {
        case HighGuid::Item:         return TYPEID_ITEM;
        //case HighGuid::Container:    return TYPEID_CONTAINER; HighGuid::Container==HighGuid::Item currently
        case HighGuid::Unit:         return TYPEID_UNIT;
        case HighGuid::Pet:          return TYPEID_UNIT;
        case HighGuid::Player:       return TYPEID_PLAYER;
        case HighGuid::GameObject:   return TYPEID_GAMEOBJECT;
        case HighGuid::DynamicObject:return TYPEID_DYNAMICOBJECT;
        case HighGuid::Corpse:       return TYPEID_CORPSE;
        case HighGuid::Mo_Transport: return TYPEID_GAMEOBJECT;
        case HighGuid::Transport:    return TYPEID_GAMEOBJECT;
    }
    return 10;                                              // unknown
}

Object::Object() : 
    m_PackGUID(sizeof(uint64)+1)
{
    m_objectTypeId      = TYPEID_OBJECT;
    m_objectType        = TYPEMASK_OBJECT;
    m_updateFlag        = UPDATEFLAG_NONE;
    m_updateFlagLK      = LK_UPDATEFLAG_NONE;

    m_uint32Values      = nullptr;
    m_valuesCount       = 0;
    _fieldNotifyFlags   = UF_FLAG_DYNAMIC;

    m_inWorld           = false;
    m_objectUpdated     = false;
}

Object::~Object( )
{
    //if(m_objectUpdated)
    //    ObjectAccessor::RemoveUpdateObject(this);

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
        m_uint32Values = nullptr;
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

void Object::SendUpdateToPlayer(Player* player)
{
    // send create update to player
    UpdateData upd;
    WorldPacket packet;

    BuildCreateUpdateBlockForPlayer(&upd, player);
    upd.BuildPacket(&packet, player->GetSession()->GetClientBuild());
    player->SendDirectMessage(&packet);
}

void Object::BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    if(!target)
        return;

    ClientBuild targetBuild = target->GetSession()->GetClientBuild();

    uint8  updatetype = UPDATETYPE_CREATE_OBJECT;
    uint8  flags      = targetBuild == BUILD_335 ? m_updateFlagLK : m_updateFlag;

    // lower flag1
    if(target == this)                                      // building packet for oneself
        flags |=  targetBuild == BUILD_335 ? LK_UPDATEFLAG_SELF : UPDATEFLAG_SELF;

    if(flags & (targetBuild == BUILD_335 ? LK_UPDATEFLAG_STATIONARY_POSITION : UPDATEFLAG_STATIONARY_POSITION))
    {
        // UPDATETYPE_CREATE_OBJECT2 dynamic objects, corpses...
        if(isType(TYPEMASK_DYNAMICOBJECT) || isType(TYPEMASK_CORPSE) || isType(TYPEMASK_PLAYER))
            updatetype = UPDATETYPE_CREATE_OBJECT2;

        // UPDATETYPE_CREATE_OBJECT2 for pets...
        if(target->GetMinionGUID() == GetGUID())
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
                    if(targetBuild == BUILD_243)
                        updatetype |= UPDATEFLAG_TRANSPORT;
                    break;
                default:
                    break;
            }
        }

        if (isType(TYPEMASK_UNIT))
        {
            if (ToUnit()->GetVictim())
                flags |= (targetBuild == BUILD_335 ? LK_UPDATEFLAG_HAS_TARGET : UPDATEFLAG_HAS_TARGET);
        }
    }

    //TC_LOG_DEBUG("misc","BuildCreateUpdate: update-type: %u, object-type: %u got flags: %X, flags2: %X", updatetype, m_objectTypeId, flags, flags2);

    ByteBuffer buf(500);
    buf << (uint8)updatetype;
    if(targetBuild == BUILD_335)
        buf << GetPackGUID();
    else
        buf << (uint8)0xFF << GetGUID();

    buf << (uint8)m_objectTypeId;

    BuildMovementUpdate(&buf, flags, targetBuild);
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
    auto iter = data_map.find(player);
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

    WorldPacket data(SMSG_DESTROY_OBJECT, 8 + 1);
    data << GetGUID();
    if(target->GetSession()->GetClientBuild() == BUILD_335)
    {
        //! If the following bool is true, the client will call "void CGUnit_C::OnDeath()" for this object.
        //! OnDeath() does for eg trigger death animation and interrupts certain spells/missiles/auras/sounds...
        data << uint8(onDeath ? 1 : 0);
    }
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

    uint32* flags = nullptr;
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

void Object::AddToObjectUpdateIfNeeded()
{
	if (m_inWorld && !m_objectUpdated)
	{
		AddToObjectUpdate();
		m_objectUpdated = true;
	}
}

void Object::ClearUpdateMask(bool remove)
{
    _changesMask.Clear();

    if(m_objectUpdated)
    {
        if(remove)
			RemoveFromObjectUpdate();

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

		AddToObjectUpdateIfNeeded();
    }
}

void Object::SetUInt32Value( uint16 index, uint32 value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(m_uint32Values[ index ] != value)
    {
        m_uint32Values[ index ] = value;
        _changesMask.SetBit(index);

		AddToObjectUpdateIfNeeded();
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

		AddToObjectUpdateIfNeeded();
    }
}

void Object::SetFloatValue( uint16 index, float value )
{
    ASSERT( index < m_valuesCount || PrintIndexError( index , true ) );

    if(m_floatValues[ index ] != value)
    {
        m_floatValues[ index ] = value;
        _changesMask.SetBit(index);

		AddToObjectUpdateIfNeeded();
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

		AddToObjectUpdateIfNeeded();
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

		AddToObjectUpdateIfNeeded();
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

		AddToObjectUpdateIfNeeded();
    }
}


bool Object::AddGuidValue(uint16 index, ObjectGuid value)
{
	ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, true));
	if (value && !*((ObjectGuid*)&(m_uint32Values[index])))
	{
		*((ObjectGuid*)&(m_uint32Values[index])) = value;
		_changesMask.SetBit(index);
		_changesMask.SetBit(index + 1);

		AddToObjectUpdateIfNeeded();

		return true;
	}

	return false;
}

bool Object::RemoveGuidValue(uint16 index, ObjectGuid value)
{
	ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, true));
	if (value && *((ObjectGuid*)&(m_uint32Values[index])) == value)
	{
		m_uint32Values[index] = 0;
		m_uint32Values[index + 1] = 0;
		_changesMask.SetBit(index);
		_changesMask.SetBit(index + 1);

		AddToObjectUpdateIfNeeded();

		return true;
	}

	return false;
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

		AddToObjectUpdateIfNeeded();
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

		AddToObjectUpdateIfNeeded();
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

		AddToObjectUpdateIfNeeded();
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

		AddToObjectUpdateIfNeeded();
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

WorldObject::WorldObject(bool isWorldObject) :
    LastUsedScriptID(0),
    lootingGroupLeaderGUID(0),
	m_isWorldObject(isWorldObject),
	m_InstanceId(0),
	m_currMap(nullptr),
	m_zoneScript(nullptr),
	m_name(""),
    m_zoneId(0),
    m_areaId(0),
    m_staticFloorZ(VMAP_INVALID_HEIGHT),
	m_groupLootTimer(0),
	m_notifyflags(0),
	m_executed_notifies(0),
	mSemaphoreTeleport(false),
	m_isActive(false),
	m_isTempWorldObject(false),
	m_transport(nullptr),
	m_phaseMask(PHASEMASK_NORMAL)
{
    m_positionX         = 0.0f;
    m_positionY         = 0.0f;
    m_positionZ         = 0.0f;
    m_orientation       = 0.0f;

	m_serverSideVisibility.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE | GHOST_VISIBILITY_GHOST);
	m_serverSideVisibilityDetect.SetValue(SERVERSIDE_VISIBILITY_GHOST, GHOST_VISIBILITY_ALIVE);
}

void WorldObject::SetWorldObject(bool on)
{
    if(!IsInWorld())
        return;
    
    GetMap()->AddObjectToSwitchList(this, on); 
}

bool WorldObject::IsWorldObject() const
{
	if (m_isWorldObject)
		return true;

	if (ToCreature() && ToCreature()->m_isTempWorldObject)
		return true;

	return false;
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

void WorldObject::_Create( uint32 guidlow, HighGuid guidhigh, uint32 phaseMask)
{
    Object::_Create(guidlow, 0, guidhigh);
	m_phaseMask = phaseMask;
}

WorldObject::~WorldObject()
{
	if (IsWorldObject() && m_currMap)
	{
		if (GetTypeId() == TYPEID_CORPSE)
		{
			TC_LOG_FATAL("misc", "WorldObject::~WorldObject Corpse Type: %d (%s) deleted but still in map!!",
				ToCorpse()->GetType(), ObjectGuid(GetGUID()).ToString().c_str());
			ABORT();
		}
		ResetMap();
	}
}


void WorldObject::UpdatePositionData()
{
    PositionFullTerrainStatus data;
    GetMap()->GetFullTerrainStatusForPosition(GetPositionX(), GetPositionY(), GetPositionZ(), data);
    ProcessPositionDataChanged(data);
}

void WorldObject::ProcessPositionDataChanged(PositionFullTerrainStatus const& data)
{
    m_zoneId = m_areaId = data.areaId;
    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(m_areaId))
        if (area->zone)
            m_zoneId = area->zone;
    m_staticFloorZ = data.floorZ;
}

void WorldObject::AddToWorld()
{
    Object::AddToWorld();
    GetBaseMap()->GetZoneAndAreaId(m_zoneId, m_areaId, GetPositionX(), GetPositionY(), GetPositionZ());
}

void WorldObject::RemoveFromWorld()
{
	if (!IsInWorld())
		return;

	DestroyForNearbyPlayers();

	Object::RemoveFromWorld();
}

void WorldObject::SetMap(Map* map)
{
	ASSERT(map);
	ASSERT(!IsInWorld());
	if (m_currMap == map) // command add npc: first create, than loadfromdb
		return;
	if (m_currMap)
	{
		TC_LOG_FATAL("misc", "WorldObject::SetMap: obj %u new map %u %u, old map %u %u", (uint32)GetTypeId(), map->GetId(), map->GetInstanceId(), m_currMap->GetId(), m_currMap->GetInstanceId());
		ABORT();
	}
	m_currMap = map;
	m_mapId = map->GetId();
	m_InstanceId = map->GetInstanceId();
	if (IsWorldObject())
		m_currMap->AddWorldObject(this);
}

void WorldObject::ResetMap()
{
	ASSERT(m_currMap);
	ASSERT(!IsInWorld());
	if (IsWorldObject())
		m_currMap->RemoveWorldObject(this);
	m_currMap = nullptr;
}

Map const* WorldObject::GetBaseMap() const
{
	ASSERT(m_currMap);
	return m_currMap->GetParent();
}

/*
uint32 WorldObject::GetZoneId() const
{
    return GetBaseMap()->GetZoneId(m_positionX,m_positionY,m_positionZ);
}

uint32 WorldObject::GetAreaId() const
{
    return GetBaseMap()->GetAreaId(m_positionX,m_positionY,m_positionZ);
}

void WorldObject::GetZoneAndAreaId(uint32& zoneid, uint32& areaid) const
{
    GetBaseMap()->GetZoneAndAreaId(zoneid, areaid, m_positionX, m_positionY, m_positionZ);
}
*/

InstanceScript* WorldObject::GetInstanceScript()
{
    Map *map = GetMap();
    return map->IsDungeon() ? ((InstanceMap*)map)->GetInstanceScript() : nullptr;
}

                                                            //slow
float WorldObject::GetDistance(const WorldObject* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float sizefactor = GetCombatReach() + obj->GetCombatReach();
    float dist = sqrt((dx*dx) + (dy*dy) + (dz*dz)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistance(const Position &pos) const
{
    float d = GetExactDist(&pos) - GetCombatReach();
    return d > 0.0f ? d : 0.0f;
}

float WorldObject::GetDistance2d(float x, float y) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float sizefactor = GetCombatReach();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistanceSqr(float x, float y, float z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float sizefactor = GetCombatReach();
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
    float sizefactor = GetCombatReach();
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
    float sizefactor = GetCombatReach() + obj->GetCombatReach();
    float dist = sqrt((dx*dx) + (dy*dy)) - sizefactor;
    return ( dist > 0 ? dist : 0);
}

float WorldObject::GetDistanceZ(const WorldObject* obj) const
{
    float dz = fabs(GetPositionZ() - obj->GetPositionZ());
    float sizefactor = GetCombatReach() + obj->GetCombatReach();
    float dist = dz - sizefactor;
    return ( dist > 0 ? dist : 0);
}

bool WorldObject::IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D /*= true*/) const
{
    return obj && _IsWithinDist(obj, dist2compare, is3D);
}

bool WorldObject::IsWithinDistInMap(WorldObject const* obj, float dist2compare, bool is3D /*= true*/, bool incOwnRadius /*= true*/, bool incTargetRadius /*= true*/) const
{
    return obj && IsInMap(obj) && InSamePhase(obj) && _IsWithinDist(obj, dist2compare, is3D, incOwnRadius, incTargetRadius);
}


bool WorldObject::IsWithinDist3d(float x, float y, float z, float dist) const
{
    return IsInDist(x, y, z, dist + GetCombatReach());
}

bool WorldObject::IsWithinDist3d(const Position* pos, float dist) const
{
    return IsInDist(pos, dist + GetCombatReach());
}

bool WorldObject::IsWithinDist2d(float x, float y, float dist) const
{
    return IsInDist2d(x, y, dist + GetCombatReach());
}

bool WorldObject::IsWithinDist2d(const Position* pos, float dist) const
{
    return IsInDist2d(pos, dist + GetCombatReach());
}

bool WorldObject::IsWithinLOSInMap(const WorldObject* obj, VMAP::ModelIgnoreFlags ignoreFlags) const
{
    if (!IsInMap(obj)) 
        return false;

    float x, y, z;
    if (obj->GetTypeId() == TYPEID_PLAYER)
        obj->GetPosition(x, y, z);
    else
        obj->GetHitSpherePointFor(GetPosition(), x, y, z);
    
    return IsWithinLOS(x, y, z, ignoreFlags);
}

bool WorldObject::IsWithinLOS(const float ox, const float oy, const float oz, VMAP::ModelIgnoreFlags ignoreFlags) const
{
    if(IsInWorld())
    {
        float x, y, z;
        if (GetTypeId() == TYPEID_PLAYER)
            GetPosition(x, y, z);
        else
            GetHitSpherePointFor({ ox, oy, oz }, x, y, z);
        
        return GetMap()->isInLineOfSight(x, y, z + 2.0f, ox, oy, oz + 2.0f, GetPhaseMask(), ignoreFlags);
   }
    
    return true;
}

Position WorldObject::GetHitSpherePointFor(Position const& dest) const
{
    G3D::Vector3 vThis(GetPositionX(), GetPositionY(), GetPositionZ());
    G3D::Vector3 vObj(dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ());
    G3D::Vector3 contactPoint = vThis + (vObj - vThis).directionOrZero() * std::min(dest.GetExactDist(GetPosition()), GetCombatReach());
    
    return Position(contactPoint.x, contactPoint.y, contactPoint.z, GetAngle(contactPoint.x, contactPoint.y));
}

void WorldObject::GetHitSpherePointFor(Position const& dest, float& x, float& y, float& z) const
 {
    Position pos = GetHitSpherePointFor(dest);
    x = pos.GetPositionX();
    y = pos.GetPositionY();
    z = pos.GetPositionZ();
}

bool WorldObject::_IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D, bool incOwnRadius, bool incTargetRadius) const
{
    float sizefactor = 0;
    sizefactor += incOwnRadius ? GetCombatReach() : 0.0f;
    sizefactor += incTargetRadius ? obj->GetCombatReach() : 0.0f;
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

    float sizefactor = GetCombatReach() + obj->GetCombatReach();

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

Position WorldObject::GetRandomPoint(const Position &srcPos, float distance) const
{
	float x, y, z;
	GetRandomPoint(srcPos, distance, x, y, z);
	return Position(x, y, z, GetOrientation());
}

void WorldObject::UpdateGroundPositionZ(float x, float y, float &z) const
{
    float new_z = GetMap()->GetHeight(x,y,z,true);
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
    WorldObject::UpdateAllowedPositionZ(GetPhaseMask(), GetMapId(), x, y, z, canSwim, canFly, waterWalk, maxDist);
}

void WorldObject::UpdateAllowedPositionZ(uint32 phaseMask, uint32 mapId, float x, float y, float &z, bool canSwim, bool canFly, bool waterWalk, float maxDist)
{
    // non fly unit don't must be in air
    // non swim unit must be at ground (mostly speedup, because it don't must be in water and water level check less fast
    Map const* baseMap = (MapInstanced*)sMapMgr->CreateBaseMap(mapId);
    if (!canFly)
    {
        float ground_z = z;
        if(canSwim || waterWalk)
            baseMap->GetWaterOrGroundLevel(x, y, z, &ground_z, true);
        else
            ground_z = baseMap->GetHeight(phaseMask, x, y, z, true, DEFAULT_HEIGHT_SEARCH);

        if (ground_z == INVALID_HEIGHT)
            return;

        if (fabs(z - ground_z) < maxDist) //if difference is within max dist
            z = ground_z;
        //else, no ground found, keep the z position as is
    }
    else
    {
        float ground_z = baseMap->GetHeight(phaseMask, x, y, z, true, DEFAULT_HEIGHT_SEARCH);
        if (z < ground_z && std::fabs(z - ground_z) <= maxDist)
            z = ground_z;
    }
}

bool WorldObject::IsPositionValid() const
{
    return Trinity::IsValidMapCoord(m_positionX,m_positionY,m_positionZ,m_orientation);
}   

void WorldObject::SetPhaseMask(uint32 newPhaseMask, bool update)
{
	m_phaseMask = newPhaseMask;

	if (update && IsInWorld())
		UpdateObjectVisibility();
}

bool WorldObject::InSamePhase(WorldObject const* obj) const
{
	return InSamePhase(obj->GetPhaseMask());
}

void WorldObject::PlayDirectSound(uint32 sound_id, Player* target /*= NULL*/)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << sound_id;
    if (target)
        target->SendDirectMessage(&data);
    else
        SendMessageToSet( &data, true ); // ToSelf ignored in this case
}

void Object::ForceValuesUpdateAtIndex(uint32 i)
{
    _changesMask.SetBit(i);
	AddToObjectUpdateIfNeeded();
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

void WorldObject::SendMessageToSet(WorldPacket *data, bool self)
{
	if (IsInWorld()) 
		SendMessageToSetInRange(data, GetVisibilityRange(), self, true);
}

void WorldObject::SendMessageToSet(WorldPacket* data, Player* skipped_rcvr)
{
	if (IsInWorld()) 
		SendMessageToSetInRange(data, GetVisibilityRange(), false, true, skipped_rcvr);
}

void WorldObject::SendMessageToSetInRange(WorldPacket *data, float dist, bool self, bool includeMargin /*= false*/, Player const* skipped_rcvr /*= nullptr*/)
{
	dist += GetCombatReach();
	if (includeMargin)
		dist += VISIBILITY_COMPENSATION; // pussywizard: to ensure everyone receives all important packets
	Trinity::MessageDistDeliverer notifier(this, data, dist, false, skipped_rcvr);
    Cell::VisitWorldObjects(this, notifier, dist);
}

void WorldObject::SendObjectDeSpawnAnim(uint64 guid)
{
    WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
    data << guid;
    SendMessageToSet(&data, true);
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
	//+ Todo: creatures like the hellfire peninsula walker?
}

float WorldObject::GetGridActivationRange() const
{
	if (isActiveObject())
	{
		/*TC
		if (GetTypeId() == TYPEID_PLAYER && ToPlayer()->GetCinematicMgr()->IsOnCinematic())
			return std::max(DEFAULT_VISIBILITY_INSTANCE, GetMap()->GetVisibilityRange());
			*/
		return GetMap()->GetVisibilityRange();
	}

	if (Creature const* thisCreature = ToCreature())
		return thisCreature->m_SightDistance;

	return 0.0f;
}


float WorldObject::GetSightRange(const WorldObject* target) const
{
	if (ToUnit())
	{
		if (ToPlayer())
		{
			if (target && target->isActiveObject() && !target->ToPlayer())
				return MAX_VISIBILITY_DISTANCE;
			/* TC
			else if (ToPlayer()->GetCinematicMgr()->IsOnCinematic())
				return DEFAULT_VISIBILITY_INSTANCE;
				*/
			else
				return GetMap()->GetVisibilityRange();
		}
		else if (ToCreature())
			return ToCreature()->m_SightDistance;
		else
			return SIGHT_RANGE_UNIT;
	}

	if (ToDynObject() && isActiveObject())
	{
		return GetMap()->GetVisibilityRange();
	}

	return 0.0f;
}

bool WorldObject::CanSeeOrDetect(WorldObject const* obj, bool ignoreStealth, bool distanceCheck, bool checkAlert) const
{
	if (this == obj)
		return true;

	if (obj->IsNeverVisible() || CanNeverSee(obj))
		return false;

	if (obj->IsAlwaysVisibleFor(this) || CanAlwaysSee(obj))
		return true;

	bool corpseVisibility = false;
	if (distanceCheck)
	{
		bool corpseCheck = false;
		if (Player const* thisPlayer = ToPlayer())
		{
			if (thisPlayer->IsDead() && thisPlayer->GetHealth() > 0 && // Cheap way to check for ghost state
				!(obj->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GHOST) & m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GHOST) & GHOST_VISIBILITY_GHOST))
			{
				if (Corpse* corpse = thisPlayer->GetCorpse())
				{
					corpseCheck = true;
					if (corpse->IsWithinDist(thisPlayer, GetSightRange(obj), false))
						if (corpse->IsWithinDist(obj, GetSightRange(obj), false))
							corpseVisibility = true;
				}
			}

#ifdef LICH_KING
			if (Unit const* target = obj->ToUnit())
			{
				// Don't allow to detect vehicle accessories if you can't see vehicle
				if (Unit const* vehicle = target->GetVehicleBase())
					if (!thisPlayer->HaveAtClient(vehicle))
						return false;
			}
#endif
		}

		WorldObject const* viewpoint = this;
		if (Player const* player = this->ToPlayer())
			viewpoint = player->GetViewpoint();

		if (!viewpoint)
			viewpoint = this;

		if (!corpseCheck && !viewpoint->IsWithinDist(obj, GetSightRange(obj), false))
			return false;
	}

	// GM visibility off or hidden NPC
	if (!obj->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GM))
	{
		//target is not gm hidden
		// Stop checking other things for GMs
		if (m_serverSideVisibilityDetect.GetValue(SERVERSIDE_VISIBILITY_GM))
			return true;
	}
	else {
		//target is gm hidden
		Player const* me = ToPlayer();
		Player const* targetP = obj->ToPlayer();
		if (me && targetP)
		{
			//both units involved are players :
			//custom sunstrider gm visibility rules
			//gm with rank 1 can only see other gm with rank 1, not higher gm's
			if (me->GetSession()->GetSecurity() <= SEC_GAMEMASTER1
				&& targetP->GetSession()->GetSecurity() > SEC_GAMEMASTER1
				&& !me->IsInSameGroupWith(targetP)) //still visible if in same group
				return false;
		}
		return m_serverSideVisibilityDetect.GetValue(SERVERSIDE_VISIBILITY_GM) >= obj->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GM);
	}

	// Ghost players, Spirit Healers, and some other NPCs
	if (!corpseVisibility && !(obj->m_serverSideVisibility.GetValue(SERVERSIDE_VISIBILITY_GHOST) & m_serverSideVisibilityDetect.GetValue(SERVERSIDE_VISIBILITY_GHOST)))
	{
		// Alive players can see dead players in some cases, but other objects can't do that
		if (Player const* thisPlayer = ToPlayer())
		{
			if (Player const* objPlayer = obj->ToPlayer())
			{
				if (thisPlayer->GetTeam() != objPlayer->GetTeam() || !thisPlayer->IsGroupVisibleFor(objPlayer))
					return false;
			}
			else
				return false;
		}
		else
			return false;
	}

	if (obj->IsInvisibleDueToDespawn())
		return false;

	if (!CanDetect(obj, ignoreStealth, checkAlert))
		return false;

	return true;
}


bool WorldObject::CanNeverSee(WorldObject const* obj) const
{
	return GetMap() != obj->GetMap() || !InSamePhase(obj);
}

bool WorldObject::CanDetect(WorldObject const* obj, bool ignoreStealth, bool checkAlert) const
{
	const WorldObject* seer = this;

	// Pets don't have detection, they use the detection of their masters
	if (Unit const* thisUnit = ToUnit())
		if (Unit* controller = thisUnit->GetCharmerOrOwner())
			seer = controller;

	if (obj->IsAlwaysDetectableFor(seer))
		return true;

	if (!ignoreStealth && !seer->CanDetectInvisibilityOf(obj))
		return false;

	if (!ignoreStealth && !seer->CanDetectStealthOf(obj, checkAlert))
		return false;

	return true;
}

bool WorldObject::CanDetectInvisibilityOf(WorldObject const* obj) const
{
	uint32 mask = obj->m_invisibility.GetFlags() & m_invisibilityDetect.GetFlags();

	// Check for not detected types
	if (mask != obj->m_invisibility.GetFlags())
		return false;

	// It isn't possible in invisibility to detect something that can't detect the invisible object
	// (it's at least true for spell: 66)
	// It seems like that only Units are affected by this check (couldn't see arena doors with preparation invisibility)
	if (obj->ToUnit())
		if ((m_invisibility.GetFlags() & obj->m_invisibilityDetect.GetFlags()) != m_invisibility.GetFlags())
			return false;

	for (uint32 i = 0; i < TOTAL_INVISIBILITY_TYPES; ++i)
	{
		if (!(mask & (1 << i)))
			continue;

		int32 objInvisibilityValue = obj->m_invisibility.GetValue(InvisibilityType(i));
		int32 ownInvisibilityDetectValue = m_invisibilityDetect.GetValue(InvisibilityType(i));

		// Too low value to detect
		if (ownInvisibilityDetectValue < objInvisibilityValue)
			return false;
	}

	return true;
}

bool WorldObject::CanDetectStealthOf(WorldObject const* obj, bool checkAlert) const
{
	// custom sunstrider rules, based on http://wolfendonkane.pagesperso-orange.fr/furtivite.html

	// Combat reach is the minimal distance (both in front and behind),
	//   and it is also used in the range calculation.
	// One stealth point increases the visibility range by 0.3 yard.

	if (!obj->m_stealth.GetFlags())
		return true;

	float distance = GetExactDist(obj);
	float combatReach = 0.0f;

	Unit const* unit = ToUnit();
	if (unit && !unit->IsAlive())
		return false;

	Unit const* unitTarget = obj->ToUnit();
	if (unitTarget)
	{
		if (unitTarget->HasAuraEffect(18461, 0)) //vanish dummy spell, 2.5s duration after vanish
			return false;

		//use combat reach of target unit instead of our own, else rogue won't be able to approach some big units
		combatReach = unit->GetCombatReach();
	}

	if (distance < combatReach) //collision
		return true;

	if (!HasInArc(M_PI / 2.0f*3.0f, obj)) // can't see 90° behind
		return false;

	for (uint32 i = 0; i < TOTAL_STEALTH_TYPES; ++i)
	{
		if (!(obj->m_stealth.GetFlags() & (1 << i)))
			continue;

		if (unit && unit->HasAuraTypeWithMiscvalue(SPELL_AURA_DETECT_STEALTH, i))
			return true;

		float visibleDistance = 0.0f;
		switch(i)
		{
		default:
		case STEALTH_GENERAL:
			visibleDistance = 17.5 + combatReach;
			//SPELL_AURA_MOD_STEALTH and SPELL_AURA_MOD_STEALTH_LEVEL are both affecting m_stealth. 
			//SPELL_AURA_MOD_STEALTH is the base stealth spell while SPELL_AURA_MOD_STEALTH_LEVEL are bonus auras and talents
			//max level stealth spell have 350 SPELL_AURA_MOD_STEALTH
			//so for this next line will equal 0 if for the same level and no talent/items boost
			//Talent such as "Master of Deception" will descrease the detect range by 15 when maxed out
			visibleDistance += float(GetLevelForTarget(obj)) - obj->m_stealth.GetValue(StealthType(i)) / 5.0f;
			//spells like Track Hidden have 30 here, so you can see 30 yards further. 
			visibleDistance += (float)m_stealthDetect.GetValue(StealthType(i));
			break;
		case STEALTH_TRAP:
			//according to some sources, only stealth units can see traps
			if (!m_stealth.GetFlags())
				break;

			visibleDistance = 0.0f;
			visibleDistance += float(GetLevelForTarget(obj)) - obj->m_stealth.GetValue(StealthType(i)) / 5.0f;
			//Rogue trap detects also have 70. Dunno how to use this, let's divide it by 5
			visibleDistance += (float)m_stealthDetect.GetValue(StealthType(i)) / 5.0f;
			break;
		}

		if (visibleDistance <= 0.0f)
			break; //in this case we can already stop here

		/* Reduce range depending on angle. Logic here is :
		- Full range in 90° in front
		- else /1.5 range if in 180° front
		- else /2 range if behind (somewhere between 180° and 270°)
		- else can't see if 90° back (already handled at this point)
		*/
		if (!HasInArc(M_PI, obj)) //not in front (180°)
			visibleDistance = visibleDistance / 2;
		else if (!HasInArc(M_PI / 2, obj)) //not in 90° cone in front
			visibleDistance = visibleDistance / 1.5;

		if (checkAlert)
			visibleDistance += (visibleDistance * 0.08f) + 1.5f;

		// If this unit is an NPC then player detect range doesn't apply
		if (unit && unit->GetTypeId() == TYPEID_PLAYER && visibleDistance > MAX_PLAYER_STEALTH_DETECT_RANGE)
			visibleDistance = MAX_PLAYER_STEALTH_DETECT_RANGE;

		if (checkAlert && unit && unit->ToCreature() && visibleDistance <= unit->ToCreature()->GetAggroRange(unitTarget) + unit->ToCreature()->m_CombatDistance)
			return true;

		if (distance <= visibleDistance)
			return true;
	}

	return false;
}

void WorldObject::SetZoneScript()
{
	if (Map* map = FindMap())
	{
		if (map->IsDungeon())
			m_zoneScript = (ZoneScript*)((InstanceMap*)map)->GetInstanceScript();
		else if (!map->IsBattlegroundOrArena())
		{
#ifdef LICH_KING
			if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(GetZoneId()))
				m_zoneScript = bf;
			else
#endif
				m_zoneScript = sOutdoorPvPMgr->GetZoneScript(GetZoneId());
		}
	}
}

void WorldObject::ClearZoneScript()
{
	m_zoneScript = NULL;
}

TempSummon* WorldObject::SummonCreature(uint32 entry, Position const& pos, TempSummonType spwtype /*= TEMPSUMMON_MANUAL_DESPAWN*/, uint32 duration /*= 0*/) const
{
	if (Map* map = FindMap())
	{
		if (TempSummon* summon = map->SummonCreature(entry, pos, NULL, duration, isType(TYPEMASK_UNIT) ? (Unit*)this : NULL))
		{
			summon->SetTempSummonType(spwtype);
			return summon;
		}
	}

	return nullptr;
}

TempSummon* WorldObject::SummonCreature(uint32 id, float x, float y, float z, float ang, TempSummonType spwtype,uint32 despwtime) const
{
	if (!x && !y && !z)
	{
		GetClosePoint(x, y, z, GetCombatReach());
		ang = GetOrientation();
	}

	Position pos;
	pos.Relocate(x, y, z, ang);
	return SummonCreature(id, pos, spwtype, despwtime);

	/*
    auto pCreature = new TempSummon(GetGUID());

    if (!pCreature->Create(GetMap()->GenerateLowGuid<HighGuid::Unit>(), GetMap(), GetPhaseMask(), id, x, y, z, ang))
    {
        delete pCreature;
        return nullptr;
    }

    if (x == 0.0f && y == 0.0f && z == 0.0f)
        GetClosePoint(x, y, z, pCreature->GetCombatReach());

    pCreature->Relocate(x, y, z, ang);

    if(!pCreature->IsPositionValid())
    {
        TC_LOG_ERROR("FIXME","ERROR: Creature (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)",pCreature->GetGUIDLow(),pCreature->GetEntry(),pCreature->GetPositionX(),pCreature->GetPositionY());
        delete pCreature;
        return nullptr;
    }

    pCreature->SetHomePosition(x, y, z, ang);
	pCreature->Summon(spwtype, despwtime, GetMap());
    
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
        pCreature->CastSpell(pCreature, pCreature->m_spells[0], false, nullptr, nullptr, GetGUID());
    }

    // allow summoned creatures to keep grids active for 2 minutes, so that we may use AI summoning creatures far away and coming to them
    pCreature->SetKeepActiveTimer(2 * MINUTE * IN_MILLISECONDS);

    //return the creature therewith the summoner has access to it
    return pCreature;
	*/
}

Pet* Player::SummonPet(uint32 entry, float x, float y, float z, float ang, PetType petType, uint32 duration)
{
    auto pet = new Pet(this, petType);

    if(petType == SUMMON_PET && pet->LoadPetFromDB(this, entry))
    {
        // Remove Demonic Sacrifice auras (known pet)
        Unit::AuraList const& auraClassScripts = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for(auto itr = auraClassScripts.begin();itr!=auraClassScripts.end();)
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

        return nullptr;
    }

    // petentry==0 for hunter "call pet" (current pet summoned if any)
    if(!entry)
    {
        delete pet;
        return nullptr;
    }

    pet->Relocate(x, y, z, ang);

    if(!pet->IsPositionValid())
    {
        TC_LOG_ERROR("FIXME","ERROR: Pet (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)",pet->GetGUIDLow(),pet->GetEntry(),pet->GetPositionX(),pet->GetPositionY());
        delete pet;
        return nullptr;
    }

	Map *map = GetMap();
	uint32 pet_number = sObjectMgr->GeneratePetNumber();
	if (!pet->Create(map->GenerateLowGuid<HighGuid::Pet>(), map, GetPhaseMask(), entry, pet_number))
	{
		TC_LOG_ERROR("FIXME", "no such creature entry %u", entry);
		delete pet;
		return nullptr;
	}

	pet->SetCreatorGUID(GetGUID());
	pet->SetFaction(GetFaction());

    // this enables pet details window (Shift+P)
    pet->GetCharmInfo()->SetPetNumber(pet_number, false);

    //pet->AIM_Initialize();

    pet->SetPowerType(POWER_MANA);
    pet->SetUInt32Value(UNIT_NPC_FLAGS , 0);
    pet->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    pet->InitStatsForLevel(GetLevel());

	SetMinion(pet, true);

    switch(petType)
    {
        case SUMMON_PET:
			// this enables pet details window (Shift+P)
			pet->GetCharmInfo()->SetPetNumber(pet_number, true);
			pet->SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS, CLASS_MAGE);
            pet->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
            pet->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, 1000);
			pet->SetFullHealth();
            pet->SetPower(POWER_MANA, pet->GetMaxPower(POWER_MANA));
            break;
        default:
            break;
    }

	map->AddToMap(pet->ToCreature(), true);

	switch (petType)
	{
		case SUMMON_PET:
			pet->InitPetCreateSpells();
			//TC pet->InitTalentForLevel();
			pet->SavePetToDB(PET_SAVE_AS_CURRENT);
			PetSpellInitialize();
			break;
		default:
			break;
	}

    if(petType == SUMMON_PET)
    {
        // Remove Demonic Sacrifice auras (known pet)
        Unit::AuraList const& auraClassScripts = GetAurasByType(SPELL_AURA_OVERRIDE_CLASS_SCRIPTS);
        for(auto itr = auraClassScripts.begin();itr!=auraClassScripts.end();)
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
    
    return pet;
}

/*
Pet* Unit::SummonPet(uint32 entry, float x, float y, float z, float ang, uint32 duration)
{
    PetType petType = SUMMON_PET;
    Pet* pet = new Pet(this, petType);

    // petentry==0 for hunter "call pet" (current pet summoned if any)
    if(!entry)
    {
        delete pet;
        return nullptr;
    }

    Map *map = GetMap();
    uint32 pet_number = sObjectMgr->GeneratePetNumber();
    if(!pet->Create(map->GenerateLowGuid<HighGuid::Pet>(), map, GetPhaseMask(), entry, pet_number))
    {
        TC_LOG_ERROR("FIXME","no such creature entry %u", entry);
        delete pet;
        return nullptr;
    }

    pet->Relocate(x, y, z, ang);

    if(!pet->IsPositionValid())
    {
        TC_LOG_ERROR("FIXME","ERROR: Pet (guidlow %d, entry %d) not summoned. Suggested coordinates isn't valid (X: %f Y: %f)",pet->GetGUIDLow(),pet->GetEntry(),pet->GetPositionX(),pet->GetPositionY());
        delete pet;
        return nullptr;
    }

    pet->SetOwnerGUID(GetGUID());
    pet->SetCreatorGUID(GetGUID());
    pet->SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, GetFaction());

    pet->AIM_Initialize();

    pet->SetPowerType(POWER_MANA);
    pet->SetUInt32Value(UNIT_NPC_FLAGS , 0);
    pet->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
    pet->InitStatsForLevel(GetLevel());

	SetMinion(pet, true);

    switch(petType)
    {
        case SUMMON_PET:
            pet->SetUInt32Value(UNIT_FIELD_BYTES_0, 2048);
            pet->SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
            pet->SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, 1000);
			pet->SetFullHealth();
            pet->SetPower(POWER_MANA, pet->GetMaxPower(POWER_MANA));
            break;
        default:
            break;
    }

	map->AddToMap(pet->ToCreature());

	switch (petType)
	{
	case SUMMON_PET:
		pet->InitPetCreateSpells();
		//TC pet->InitTalentForLevel();
		pet->SavePetToDB(PET_SAVE_AS_CURRENT);
		PetSpellInitialize();
		break;
	default:
		break;
	}


    if(duration > 0)
        pet->SetDuration(duration);

    return pet;
}
*/

GameObject* WorldObject::SummonGameObject(uint32 entry, Position const& pos, G3D::Quat const& rot, uint32 respawnTime) const
{
    if(!IsInWorld())
        return nullptr;

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);
    if(!goinfo)
    {
        TC_LOG_ERROR("sql.sql","Gameobject template %u not found in database!", entry);
        return nullptr;
    }
    Map *map = GetMap();
    GameObject* go = sObjectMgr->IsGameObjectStaticTransport(entry) ? new StaticTransport() : new GameObject();

    if(!go->Create(map->GenerateLowGuid<HighGuid::GameObject>(), entry, map, GetPhaseMask(), pos, rot, 255, GO_STATE_READY))
    {
        delete go;
        return nullptr;
    }
    go->SetRespawnTime(respawnTime);
    if(GetTypeId()==TYPEID_PLAYER || GetTypeId()==TYPEID_UNIT) //not sure how to handle this
        ((Unit*)this)->AddGameObject(go);
    else
        go->SetSpawnedByDefault(false);
    map->AddToMap(go);

    return go;
}

GameObject* WorldObject::SummonGameObject(uint32 entry, float x, float y, float z, float ang, G3D::Quat const& rot, uint32 respawnTime)
{
    if (!x && !y && !z)
    {
        GetClosePoint(x, y, z, GetCombatReach());
        ang = GetOrientation();
    }

    Position pos(x, y, z, ang);
    return SummonGameObject(entry, pos, rot, respawnTime);
}

Creature* WorldObject::SummonTrigger(float x, float y, float z, float ang, uint32 duration, CreatureAI* (*GetAI)(Creature*))
{
    TempSummonType summonType = (duration == 0) ? TEMPSUMMON_DEAD_DESPAWN : TEMPSUMMON_TIMED_DESPAWN;
    Creature* summon = SummonCreature(WORLD_TRIGGER, x, y, z, ang, summonType, duration);
    if(!summon)
        return nullptr;

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
       Creature *creature = nullptr;
       Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck checker(*this, entry, alive, range);
       Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(creature, checker);
       Cell::VisitAllObjects(this, searcher, range);
       return creature;
}

GameObject* WorldObject::FindNearestGameObject(uint32 entry, float range) const
{
    GameObject *go = nullptr;
    Trinity::NearestGameObjectEntryInObjectRangeCheck checker(*this, entry, range);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> searcher(this, go, checker);
    Cell::VisitGridObjects(this, searcher, range);

    return go;
}

GameObject* WorldObject::FindNearestGameObjectOfType(GameobjectTypes type, float range) const
{
    GameObject* go = nullptr;
    Trinity::NearestGameObjectTypeInObjectRangeCheck checker(*this, type, range);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectTypeInObjectRangeCheck> searcher(this, go, checker);
    Cell::VisitGridObjects(this, searcher, range);
    return go;
}

Player* WorldObject::SelectNearestPlayer(float distance, bool alive) const
{
    Player* target = nullptr;

    Trinity::NearestPlayerInObjectRangeCheck checker(*this, alive, distance);
    Trinity::PlayerLastSearcher<Trinity::NearestPlayerInObjectRangeCheck> searcher(this, target, checker);
    Cell::VisitAllObjects(this, searcher, distance);

    return target;
}

void WorldObject::GetNearPoint2D(float &x, float &y, float distance2d, float absAngle ) const
{
    x = GetPositionX() + (GetCombatReach() + distance2d) * cos(absAngle);
    y = GetPositionY() + (GetCombatReach() + distance2d) * sin(absAngle);

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

void WorldObject::GetClosePoint(float &x, float &y, float &z, float searcherSize, float distance2d, float angle) const
{
    // angle calculated from current orientation
    GetNearPoint(nullptr, x, y, z, searcherSize, distance2d, GetOrientation() + angle);
}

void WorldObject::GetContactPoint(const WorldObject* obj, float &x, float &y, float &z, float distance2d) const
{
    //on retail, creature follow approximatively at half the max melee distance
    float offset = (GetCombatReach() + obj->GetCombatReach()) / 2.0f + distance2d;
    float angle = GetAngle(obj);
    x = GetPositionX() + offset * cos(angle);
    y = GetPositionY() + offset * sin(angle);

    Trinity::NormalizeMapCoord(x);
    Trinity::NormalizeMapCoord(y);

    z = GetPositionZ();
    UpdateAllowedPositionZ(x, y, z, GetCombatReach());
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
        ZLiquidStatus res = obj->GetMap()->GetLiquidStatus(x, y, z, MAP_ALL_LIQUIDS, &liquid_status);
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

bool Position::IsWithinDoubleVerticalCylinder(Position const* center, float radius, float height) const
{
    float verticalDelta = GetPositionZ() - center->GetPositionZ();
    return IsInDist2d(center, radius) && std::abs(verticalDelta) <= height;
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

Position::Position(G3D::Vector3 const& vect)
{
    Relocate(vect.x, vect.y, vect.z, 0.f);
}

bool Position::operator==(Position const &a)
{
    return (G3D::fuzzyEq(a.m_positionX, m_positionX) &&
        G3D::fuzzyEq(a.m_positionY, m_positionY) &&
        G3D::fuzzyEq(a.m_positionZ, m_positionZ) &&
        G3D::fuzzyEq(a.m_orientation, m_orientation));
}

bool Position::HasInLine(const WorldObject* const target, float objSize, float width) const
{
    if (!HasInArc(float(M_PI), target))
        return false;

    width += objSize;
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

Position Position::GetPositionWithOffset(Position const& offset) const
{
    Position ret(*this);
    ret.RelocateOffset(offset);
    return ret;
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

void WorldObject::GetCreatureListWithEntryInGrid(std::list<Creature*>& lList, uint32 uiEntry, float fMaxSearchRange) const
{
    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.SetNoCreate();

    Trinity::AllCreaturesOfEntryInRange check((Unit const*)this, uiEntry, fMaxSearchRange);
    Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(this, lList, check);
    TypeContainerVisitor<Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange>, GridTypeMapContainer> visitor(searcher);

	cell.Visit(pair, visitor, *(this->GetMap()), *this, fMaxSearchRange);
}

void WorldObject::GetGameObjectListWithEntryInGrid(std::list<GameObject*>& lList, uint32 uiEntry, float fMaxSearchRange) const
{
    CellCoord pair(Trinity::ComputeCellCoord(this->GetPositionX(), this->GetPositionY()));
    Cell cell(pair);
    cell.SetNoCreate();

    Trinity::AllGameObjectsWithEntryInRange check(this, uiEntry, fMaxSearchRange);
    Trinity::GameObjectListSearcher<Trinity::AllGameObjectsWithEntryInRange> searcher(this, lList, check);
    TypeContainerVisitor<Trinity::GameObjectListSearcher<Trinity::AllGameObjectsWithEntryInRange>, GridTypeMapContainer> visitor(searcher);

    cell.Visit(pair, visitor, *(this->GetMap()), *this, fMaxSearchRange);
}

void Object::BuildMovementUpdate(ByteBuffer* data, uint16 flags, ClientBuild build) const
{
    Unit const* unit = nullptr;
    WorldObject const* object = nullptr;

    if (isType(TYPEMASK_UNIT))
        unit = ToUnit();
    else
        object = ((WorldObject*)this);

    if(build == BUILD_335)
        *data << uint16(flags);                                  // update flags
    else
        *data << uint8(flags);                                  // update flags

    // 0x20
    if (flags & (build == BUILD_335 ? LK_UPDATEFLAG_LIVING : UPDATEFLAG_LIVING))
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
        if(build == BUILD_335)
#ifdef LICH_KING
            *data << unit->GetSpeed(MOVE_PITCH_RATE);
#else
            *data << 3.14f; //default pitch rate
#endif

        // 0x08000000
        if (unit->m_movementInfo.GetMovementFlags() & MOVEMENTFLAG_SPLINE_ENABLED)
            Movement::PacketBuilder::WriteCreate(*unit->movespline, *data, build);
    }
    else
    {
        bool LK_has_position = false;
        if(build == BUILD_335)
        {
            if (flags & LK_UPDATEFLAG_POSITION)
            {
                LK_has_position = true;

                Transport* transport = object->GetTransport();

                if (transport)
                    *data << transport->GetPackGUID();
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
        }
        
        // 0x40
        if ((build != BUILD_335 || LK_has_position == false) //LK wont send LK_UPDATEFLAG_STATIONARY_POSITION if LK_UPDATEFLAG_POSITION was present
            && flags & (build == BUILD_335 ? LK_UPDATEFLAG_STATIONARY_POSITION : UPDATEFLAG_STATIONARY_POSITION))
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

    // 0x8
    if ((build == BUILD_335) && flags & LK_UPDATEFLAG_UNKNOWN)
    {
        *data << uint32(0);
    }

    // 0x08
    if (flags & (build == BUILD_335 ? LK_UPDATEFLAG_LOWGUID : UPDATEFLAG_LOWGUID))
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
                {
                    if(build == BUILD_335) //not sure this change is needed (since we dunno what it does)
                        *data << uint32(0x0000002F);            // unk
                    else
                        *data << uint32(0x00000015);            // unk, can be 0x15 or 0x22
                } else
                    *data << uint32(0x00000008);            // unk
                break;
            default:
                *data << uint32(0x00000000);                // unk
                break;
        }
    }

    // 0x10
    if((build == BUILD_243) && flags & UPDATEFLAG_HIGHGUID)
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

    // 0x4
    if (flags & (build == BUILD_335 ? LK_UPDATEFLAG_HAS_TARGET : UPDATEFLAG_HAS_TARGET))
    {
        ASSERT(unit);
        if (Unit* victim = unit->GetVictim())
            *data << victim->GetPackGUID();
        else
            *data << uint8(0);
    }

    // 0x2
    if (flags & (build == BUILD_335 ? LK_UPDATEFLAG_TRANSPORT : UPDATEFLAG_TRANSPORT))
    {
        GameObject const* go = ToGameObject();
        if (go && go->ToTransport())
            *data << uint32(go->ToTransport()->GetPathProgress());
        else 
            *data << uint32(0);
    }

    // 0x80
    if ((build == BUILD_335) && flags & LK_UPDATEFLAG_VEHICLE)
    {
#ifdef LICH_KING
        /// @todo Allow players to aquire this updateflag.
        *data << uint32(unit->GetVehicleKit()->GetVehicleInfo()->m_ID);
        if (unit->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT))
            *data << float(unit->GetTransOffsetO());
        else
            *data << float(unit->GetOrientation());
#else
        DEBUG_ASSERT(false); //this flag not supposed to be set in BC
#endif
    }

    // 0x200
    if ((build == BUILD_335) && flags & LK_UPDATEFLAG_ROTATION)
        *data << int64(0 /*ToGameObject()->GetPackedWorldRotation()*/); //TODO
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

void WorldObject::UpdateObjectVisibility(bool /*forced*/)
{
	//updates object's visibility for nearby players
	Trinity::VisibleChangesNotifier notifier(*this);
    Cell::VisitWorldObjects(this, notifier, GetVisibilityRange());
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
        Player* source = nullptr;
        for (auto & iter : m)
        {
            source = iter.GetSource();
            BuildPacket(source);

            //also add vision to player if he has shared vision with us
            if (!source->GetSharedVisionList().empty())
            {
                //has player shared vision with us ?
                 for (auto p : source->GetSharedVisionList())
					BuildPacket(p);
            }
        }
    }

    //add vision from creatures whom we share vision with
    void Visit(CreatureMapType &m)
    {
        Creature* source = nullptr;
        for (auto & iter : m)
        {
            source = iter.GetSource();
            if (!source->GetSharedVisionList().empty())
            {
                //has player shared vision with us ?
                for (auto p : source->GetSharedVisionList())
                    BuildPacket(p);
            }
        }
    }

    //add vision from far sight dynamic objects
    void Visit(DynamicObjectMapType &m)
    {
        DynamicObject* source = nullptr;
        for (auto & iter : m)
        {
            source = iter.GetSource();
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

    /*todo perf: As I understand it, what changes from player to player is visibility, we build almost the same packet for every players here, this could be vastly improved.
    Maybe we could:
    - Do a first pass where we just count players in range that should receive the packet
    - If that count passes a certain threshold (maybe a very low one like 2-3?), send all those players a packet containing all the changes
    I think that a client ignore "privates" field it shouldn't receive anyway (like those with UF_OWNER) so that shouldn't cause problems
    */
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

void WorldObject::AddToObjectUpdate()
{
	GetMap()->AddUpdateObject(this);
}

void WorldObject::RemoveFromObjectUpdate()
{
	GetMap()->RemoveUpdateObject(this);
}


void WorldObject::DestroyForNearbyPlayers()
{
	if (!IsInWorld())
		return;

	std::list<Player*> targets;
	Trinity::AnyPlayerInObjectRangeCheck check(this, GetMap()->GetVisibilityRange());
	Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(this, targets, check);
    Cell::VisitWorldObjects(this, searcher, GetMap()->GetVisibilityRange());
	for (auto& player : targets) {
		if (player == this)
			continue;

		if (!player->HaveAtClient(this))
			continue;

		/*
		if (isType(TYPEMASK_UNIT) && ToUnit()->GetCharmerGUID() == player->GetGUID()) /// @todo this is for puppet
			continue;
		*/

		if(GetTypeId() == TYPEID_UNIT)
			DestroyForPlayer(player, ToUnit()->IsDuringRemoveFromWorld() && ToCreature()->IsDead()); // at remove from world (destroy) show kill animation
		else
			DestroyForPlayer(player);

		player->m_clientGUIDs.erase(GetGUID());
	}
}

uint64 WorldObject::GetTransGUID() const
{
    if (GetTransport())
        return GetTransport()->GetGUID();
    return 0;
}

float WorldObject::GetFloorZ() const
{
    if (!IsInWorld())
        return m_staticFloorZ;
    //this is the same as Map::GetHeight but we use our cached value for the map height instead for performance
    return std::max<float>(m_staticFloorZ, GetMap()->GetGameObjectFloor(GetPhaseMask(), GetPositionX(), GetPositionY(), GetPositionZ()));
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