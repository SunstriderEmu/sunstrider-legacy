
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
#include "ChaseMovementGenerator.h"
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
#include "OutdoorPvPMgr.h"
#include "CinematicMgr.h"
#include "SpellAuraEffects.h"
#include "TotemAI.h"
#include "ReputationMgr.h"

#include "TemporarySummon.h"
#include "DynamicTree.h"

uint32 GuidHigh2TypeId(HighGuid guid_hi)
{
    switch(guid_hi)
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

    m_uint32Values      = nullptr;
    m_valuesCount       = 0;
    _fieldNotifyFlags   = UF_FLAG_DYNAMIC;

    m_inWorld           = false;
    m_isNewObject       = false;
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
            TC_LOG_ERROR("misc","Object::~Object - guid=%u, typeid=%d deleted but still in world!!", GetGUID().GetCounter(), GetTypeId());
            ABORT();
        }

        if (m_objectUpdated)
        {
            TC_LOG_FATAL("misc", "Object::~Object %u deleted but still in update list!!", GetGUID().GetCounter());
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

void Object::_Create(ObjectGuid::LowType guidlow, uint32 entry, HighGuid guidhigh )
{
    if(!m_uint32Values)
        _InitValues();

    ObjectGuid guid(guidhigh, entry, guidlow);
    SetGuidValue( OBJECT_FIELD_GUID, guid );
    SetUInt32Value(OBJECT_FIELD_TYPE, m_objectType);
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
    upd.BuildPacket(&packet, false);
    player->SendDirectMessage(&packet);
}

void Object::BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const
{
    if(!target)
        return;

    uint8  updateType = UPDATETYPE_CREATE_OBJECT;
    uint8  flags = m_updateFlag;

    // lower flag1
    if(target == this)                                      // building packet for oneself
        flags |=  UPDATEFLAG_SELF;


    if (m_isNewObject)
    {
        switch (ObjectGuid(GetGUID()).GetHigh())
        {
        case HighGuid::Player:
        case HighGuid::Pet:
        case HighGuid::Corpse:
        case HighGuid::DynamicObject:
        case HighGuid::GameObject: //sun: diff with TC, we send this for all gobjects, not just player ones. Not 100% sure about this but this seems to make more sense
            updateType = UPDATETYPE_CREATE_OBJECT2;
            break;
        case HighGuid::Unit:
        case HighGuid::Vehicle:
        {
            if (ToUnit()->IsSummon())
                updateType = UPDATETYPE_CREATE_OBJECT2;
            break;
        }
        default:
            break;
        }
    }

    if(flags & UPDATEFLAG_STATIONARY_POSITION)
    {
        // UPDATETYPE_CREATE_OBJECT2 for some gameobject types...
        if(isType(TYPEMASK_GAMEOBJECT))
        {
            switch(((GameObject*)this)->GetGoType())
            {
                case GAMEOBJECT_TYPE_TRAP:
                case GAMEOBJECT_TYPE_DUEL_ARBITER:
                case GAMEOBJECT_TYPE_FLAGSTAND:
                case GAMEOBJECT_TYPE_FLAGDROP:
                    updateType = UPDATETYPE_CREATE_OBJECT2;
                    break;
                case GAMEOBJECT_TYPE_TRANSPORT:
#ifndef LICH_KING
                    updateType |= UPDATEFLAG_TRANSPORT;
#endif
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
    buf << (uint8)updateType;
#ifdef LICH_KING
    buf << GetPackGUID();
#else
    buf << (uint8)0xFF << GetGUID();
#endif

    buf << (uint8)m_objectTypeId;

    BuildMovementUpdate(&buf, flags);
    BuildValuesUpdate(updateType, &buf, target);
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
#ifdef LICH_KING
    //! If the following bool is true, the client will call "void CGUnit_C::OnDeath()" for this object.
    //! OnDeath() does for eg trigger death animation and interrupts certain spells/missiles/auras/sounds...
    data << uint8(onDeath ? 1 : 0);
#endif
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

ObjectGuid Object::GetGuidValue(uint16 index) const
{
    ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, false));
    return *((ObjectGuid*)&(m_uint32Values[index]));
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

void Object::SetGuidValue(uint16 index, ObjectGuid value)
{
    ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, true));
    if (*((ObjectGuid*)&(m_uint32Values[index])) != value)
    {
        *((ObjectGuid*)&(m_uint32Values[index])) = value;
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
    Object(), WorldLocation(),
    LastUsedScriptID(0),
    m_isWorldObject(isWorldObject),
    m_InstanceId(0),
    m_currMap(nullptr),
    m_zoneScript(nullptr),
    m_movementInfo(),
    m_name(),
    m_zoneId(0),
    m_areaId(0),
    m_staticFloorZ(VMAP_INVALID_HEIGHT),
    m_notifyflags(0),
    m_executed_notifies(0),
    mSemaphoreTeleport(false),
    m_isActive(false),
    m_isFarVisible(false),
    m_isTempWorldObject(false),
    m_transport(nullptr),
    m_phaseMask(PHASEMASK_NORMAL),
    _forceHitResultOverride(SPELL_FORCE_HIT_DEFAULT)
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

        //sun: load grid if not done. Else unit won't update properly if grid wasn't loaded beforehand
        if (!map->IsGridLoaded(GetPositionX(), GetPositionY()))
            map->LoadGrid(GetPositionX(), GetPositionY());
    }
    else
    {
        if(GetTypeId() == TYPEID_UNIT)
            map->RemoveFromForceActive(this->ToCreature());
        else if(GetTypeId() == TYPEID_DYNAMICOBJECT)
            map->RemoveFromForceActive((DynamicObject*)this);
    }
}

void WorldObject::SetFarVisible(bool on)
{
    if (GetTypeId() == TYPEID_PLAYER)
        return;

    m_isFarVisible = on;
}

void WorldObject::CleanupsBeforeDelete(bool /*finalCleanup*/)
{
    if (IsInWorld())
        RemoveFromWorld();

    if (Transport* transport = GetTransport())
        transport->RemovePassenger(this);
}

void WorldObject::_Create(ObjectGuid::LowType guidlow, HighGuid guidhigh, uint32 phaseMask)
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


void WorldObject::UpdatePositionData(bool updateCreatureLiquid)
{
    PositionFullTerrainStatus data;
    GetMap()->GetFullTerrainStatusForPosition(GetPositionX(), GetPositionY(), GetPositionZ(), data, MAP_ALL_LIQUIDS);
    ProcessPositionDataChanged(data, updateCreatureLiquid);
}

void WorldObject::ProcessPositionDataChanged(PositionFullTerrainStatus const& data, bool /*updateCreatureLiquid*/)
{
    m_zoneId = m_areaId = data.areaId;
    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(m_areaId))
        if (area->zone)
            m_zoneId = area->zone;

    m_outdoors = data.outdoors;
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

bool WorldObject::IsSelfOrInSameMap(WorldObject const* obj) const
{
    if (this == obj)
        return true;
    return IsInMap(obj);
}

bool WorldObject::IsWithinLOSInMap(const WorldObject* obj, LineOfSightChecks checks, VMAP::ModelIgnoreFlags ignoreFlags) const
{
    if (!IsInMap(obj)) 
        return false;

    float ox, oy, oz;
    if (obj->GetTypeId() == TYPEID_PLAYER)
    {
        obj->GetPosition(ox, oy, oz);
        oz += GetCollisionHeight();
    }
    else
        obj->GetHitSpherePointFor({ GetPositionX(), GetPositionY(), GetPositionZ() + GetCollisionHeight() }, ox, oy, oz);
    
    float x, y, z;
    if (GetTypeId() == TYPEID_PLAYER)
    {
        GetPosition(x, y, z);
        z += GetCollisionHeight();
    }
    else
        GetHitSpherePointFor({ obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ() + obj->GetCollisionHeight() }, x, y, z);

    return GetMap()->isInLineOfSight(x, y, z, ox, oy, oz, GetPhaseMask(), checks, ignoreFlags);
}

bool WorldObject::IsWithinLOS(float ox, float oy, float oz, LineOfSightChecks checks, VMAP::ModelIgnoreFlags ignoreFlags) const
{
    if(IsInWorld())
    {
        oz += GetCollisionHeight();
        float x, y, z;
        if (GetTypeId() == TYPEID_PLAYER)
        {
            GetPosition(x, y, z);
            z += GetCollisionHeight();
        }
        else
            GetHitSpherePointFor({ ox, oy, oz }, x, y, z);
        
        return GetMap()->isInLineOfSight(x, y, z + 2.0f, ox, oy, oz + 2.0f, GetPhaseMask(), checks, ignoreFlags);
   }
    
    return true;
}

Position WorldObject::GetHitSpherePointFor(Position const& dest) const
{
    G3D::Vector3 vThis(GetPositionX(), GetPositionY(), GetPositionZ() + GetCollisionHeight());
    G3D::Vector3 vObj(dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ());
    G3D::Vector3 contactPoint = vThis + (vObj - vThis).directionOrZero() * std::min(dest.GetExactDist(GetPosition()), GetCombatReach());
    
    return Position(contactPoint.x, contactPoint.y, contactPoint.z, GetAbsoluteAngle(contactPoint.x, contactPoint.y));
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
    float new_z = GetMapHeight(x, y, z);
    if (new_z > INVALID_HEIGHT)
    {
        z = new_z;
#ifdef LICH_KING
        if (isType(TYPEMASK_UNIT))
            z += static_cast<Unit const*>(this)->GetHoverOffset();
#endif
    }
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
    float newZ = z + GetCollisionHeight();
    WorldObject::UpdateAllowedPositionZ(GetPhaseMask(), GetMapId(), x, y, newZ, canSwim, canFly, waterWalk, maxDist);
    if (newZ != z + GetCollisionHeight())
        z = newZ;
}

void WorldObject::UpdateAllowedPositionZ(uint32 phaseMask, uint32 mapId, float x, float y, float &z, bool canSwim, bool canFly, bool waterWalk, float maxDist)
{
    // non fly unit don't must be in air
    // non swim unit must be at ground (mostly speedup, because it don't must be in water and water level check less fast
    Map const* baseMap = (MapInstanced*)sMapMgr->CreateBaseMap(mapId);
    if (!canFly)
    {
        float ground_z = z;
        float max_z = canSwim
            ? baseMap->GetWaterOrGroundLevel(phaseMask, x, y, z, &ground_z, !waterWalk)
            : ((ground_z = baseMap->GetHeight(phaseMask, x, y, z, true)));

        if (max_z > INVALID_HEIGHT)
        {
            if (z > max_z && fabs(z - max_z) < maxDist)
                z = max_z;
            else if (z < ground_z && fabs(z - ground_z) < maxDist)
                z = ground_z;
            //else, no ground found, keep the z position as is
        }
    }
    else
    {
        float ground_z = baseMap->GetHeight(phaseMask, x, y, z, true);
        if (z < ground_z && std::fabs(z - ground_z) <= maxDist)
            z = ground_z;
    }
}

// Only call this function when you already checked for collision with vmap/gameobject.
// You should check for collision again after this one has been called.
// excludeCollisionHeight should only be true if you had collision, it wont add it to raycasts for dest position.
float WorldObject::SelectBestZForDestination(float x, float y, float z, bool excludeCollisionHeight) const
{
    if (Unit const* unit = ToUnit())
    {
        float const ground = GetFloorZ();
        bool const isInAir = (G3D::fuzzyGt(unit->GetPositionZ(), ground + GROUND_HEIGHT_TOLERANCE) || G3D::fuzzyLt(unit->GetPositionZ(), ground - GROUND_HEIGHT_TOLERANCE));
       if (isInAir)
       {
            // creatures never get MOVEMENTFLAG_PLAYER_FLYING, check it additionally for them
            if (Creature const* creature = ToCreature())
                if (creature->CanFly())
                    return z;

            if (unit->IsFlying())
                return z;
        }
    }

    float myX, myY, myZ;
    GetPosition(myX, myY, myZ);

    float const myCollisionHeight = GetCollisionHeight();
    float const destCollisionHeight = excludeCollisionHeight ? 0.0f : myCollisionHeight;

    float const myGridHeight = GetMap()->GetGridMapHeight(myX, myY);
    float const myVmapFloor = std::max(GetMap()->GetVMapFloor(myX, myY, myZ, 150.0f, myCollisionHeight),
        GetMap()->GetGameObjectFloor(GetPhaseMask(), myX, myY, myZ, 150.0f, myCollisionHeight));

    // which of these 3 do I want ?
    float const destGridHeight = GetMap()->GetGridMapHeight(x, y);
    float const destCeil = GetMap()->GetCeil(GetPhaseMask(), x, y, z, 150.0f, destCollisionHeight);
    float const destVmapFloor = std::max(GetMap()->GetVMapFloor(x, y, z, 150.0f, destCollisionHeight),
        GetMap()->GetGameObjectFloor(GetPhaseMask(), x, y, z, 150.0f, destCollisionHeight));

    bool const hasVmapFloor = myVmapFloor > INVALID_HEIGHT;
    bool const hasDestGridHeight = destGridHeight > INVALID_HEIGHT;
    bool const hasDestVmapCeil = destCeil < VMAP_INVALID_CEIL && destCeil != destVmapFloor;
    bool const hasDestVmapFloor = destVmapFloor > INVALID_HEIGHT;
    bool const destBetweenVmaps = hasDestVmapCeil && hasDestVmapFloor;
    bool const noVmap = !hasDestVmapFloor && !hasDestVmapCeil;

    // It is possible that while moving, our feet are slightly moving under the ground. Jumping / reconnecting fixes this issue but we don't want to rely on that.
    myZ += myCollisionHeight;
    bool const isOnVmap = hasVmapFloor &&
        ((myZ < myGridHeight && std::fabs(myVmapFloor - myZ) < std::fabs(myGridHeight - myZ)) ||
        (myZ > myGridHeight && myVmapFloor > myGridHeight));

    bool const hasToFollowGridHeight = hasDestGridHeight && (noVmap ||
        (z > destGridHeight && destGridHeight > destVmapFloor) ||
        (z < destGridHeight && hasDestVmapFloor && !hasDestVmapCeil) || //sun: basically, if we're underground and not in a cavern like place, follow grid
        (z < destGridHeight && !hasDestVmapFloor) ||
        (destBetweenVmaps && !isOnVmap && destGridHeight > destVmapFloor && destGridHeight < destCeil));

    float result = INVALID_HEIGHT;
    if (hasToFollowGridHeight)
    {
        result = destGridHeight;
        if (hasDestVmapFloor)
            if (std::fabs(destVmapFloor - destGridHeight) < myCollisionHeight)
                result = std::max(destVmapFloor, destGridHeight);

        if (hasDestVmapCeil)
            if (std::fabs(destCeil - destGridHeight) < myCollisionHeight)
                result = std::max(destCeil, destGridHeight);
    }
    else if (hasDestVmapFloor)
        result = destVmapFloor;
    else if (hasDestVmapCeil)
        result = destCeil;

    if (Unit const* unit = ToUnit())
        if (!unit->CanSwim())
            return result;

    LiquidData liquidData;
    ZLiquidStatus const liquidStatus = GetMap()->GetLiquidStatus(x, y, z, MAP_ALL_LIQUIDS, &liquidData, destCollisionHeight);
    switch (liquidStatus)
    {
    case LIQUID_MAP_ABOVE_WATER:
        return std::max<float>(liquidData.level, result);
    case LIQUID_MAP_IN_WATER:
    case LIQUID_MAP_UNDER_WATER:
        return std::max<float>(z, result);
    default:
        return result;
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

void WorldObject::PlayDistanceSound(uint32 sound_id, Player* target /*= nullptr*/)
{
    WorldPacket data(SMSG_PLAY_OBJECT_SOUND, 4 + 8);
    data << uint32(sound_id);
    data << uint64(GetGUID());
    if (target)
        target->SendDirectMessage(&data);
    else
        SendMessageToSet(&data, true);
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

void WorldObject::BuildHeartBeatMsg(WorldPacket* data) const
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

void WorldObject::SendMessageToSet(WorldPacket const* data, bool self)
{
    if (IsInWorld()) 
        SendMessageToSetInRange(data, GetVisibilityRange(), self, true);
}

void WorldObject::SendMessageToSet(WorldPacket const* data, Player* skipped_rcvr)
{
    if (IsInWorld()) 
        SendMessageToSetInRange(data, GetVisibilityRange(), false, true, skipped_rcvr);
}

void WorldObject::SendMessageToSetInRange(WorldPacket const* data, float dist, bool self, bool includeMargin /*= false*/, Player const* skipped_rcvr /*= nullptr*/)
{
    dist += GetCombatReach();
    if (includeMargin)
        dist += VISIBILITY_COMPENSATION; // sunwell: to ensure everyone receives all important packets
    Trinity::MessageDistDeliverer notifier(this, data, dist, false, skipped_rcvr);
    Cell::VisitWorldObjects(this, notifier, dist);
}

void WorldObject::SendObjectDeSpawnAnim(ObjectGuid guid)
{
    WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
    data << uint64(guid);
    SendMessageToSet(&data, true);
}

void WorldObject::AddObjectToRemoveList()
{
    assert(m_uint32Values);

    Map* map = FindMap();
    if(!map)
    {
        TC_LOG_ERROR("misc","Object (TypeId: %u Entry: %u GUID: %u) at attempt add to move list not have valid map (Id: %u).", GetTypeId(), GetEntry(), GetGUID().GetCounter(),GetMapId());
        return;
    }

    map->AddObjectToRemoveList(this);
}

float WorldObject::GetVisibilityRange() const
{
    if (IsFarVisible() && !ToPlayer())
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
        if (GetTypeId() == TYPEID_PLAYER && ToPlayer()->GetCinematicMgr()->IsOnCinematic())
            return std::max(MAX_VISIBILITY_DISTANCE, GetMap()->GetVisibilityRange());
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
            if (target && target->IsFarVisible() && !target->ToPlayer())
                return MAX_VISIBILITY_DISTANCE;
            else if (ToPlayer()->GetCinematicMgr()->IsOnCinematic())
                return MAX_VISIBILITY_DISTANCE;
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

bool WorldObject::CanSeeOrDetect(WorldObject const* obj, bool ignoreStealth, bool distanceCheck, bool checkAlert, float tolerance /*= 0.0f*/) const
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
                    if (corpse->IsWithinDist(thisPlayer, GetSightRange(obj) + tolerance, false))
                        if (corpse->IsWithinDist(obj, GetSightRange(obj) + tolerance, false))
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

        if (!corpseCheck && !viewpoint->IsWithinDist(obj, GetSightRange(obj) + tolerance, false))
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

    if (!CanDetect(obj, ignoreStealth, checkAlert, tolerance))
        return false;

    return true;
}


bool WorldObject::CanNeverSee(WorldObject const* obj) const
{
    return GetMap() != obj->GetMap() || !InSamePhase(obj);
}

bool WorldObject::CanDetect(WorldObject const* obj, bool ignoreStealth, bool checkAlert, float tolerance /*= 0.0f*/) const
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

    if (!ignoreStealth && !seer->CanDetectStealthOf(obj, checkAlert, tolerance))
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
    /* Sunstrider: Disabled. This is wrong in some cases such as creature 20882 which should aggro players even though then cant detect her
    if (obj->ToUnit())
        if ((m_invisibility.GetFlags() & obj->m_invisibilityDetect.GetFlags()) != m_invisibility.GetFlags())
            return false;
            */

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

bool WorldObject::CanDetectStealthOf(WorldObject const* obj, bool checkAlert, float tolerance /* = 0.0f */) const
{
    // custom sunstrider rules, based on http://wolfendonkane.pagesperso-orange.fr/furtivite.html
    // One stealth point increases the visibility range by 0.3 yard.

    if (!obj->m_stealth.GetFlags())
        return true;

    float distance = GetDistance(obj); // This uses this unit and target combat reach

    Unit const* detector = ToUnit();
    if (detector && !detector->IsAlive())
        return false;

    Unit const* unitTarget = obj->ToUnit();
    if (unitTarget)
    {
        if (unitTarget->HasAura(18461)) //vanish dummy spell, 2.5s duration after vanish
            return false;
    }

    if (distance <= 0.0f) //collision
        return true;

    if (!HasInArc(M_PI / 2.0f*3.0f, obj)) // can't see 90° behind
        return false;

    for (uint32 i = 0; i < TOTAL_STEALTH_TYPES; ++i)
    {
        if (!(obj->m_stealth.GetFlags() & (1 << i)))
            continue;

        if (detector && detector->HasAuraTypeWithMiscvalue(SPELL_AURA_DETECT_STEALTH, i))
            return true;

        float visibleDistance = 0.0f;
        switch(i)
        {
        default:
        case STEALTH_GENERAL:
            visibleDistance = 17.5;
            break;
        case STEALTH_TRAP:
            //according to some sources, only stealth units can see traps
            if (!m_stealth.GetFlags())
                break;

            visibleDistance = 0.0f;
            break;
        }

        // General rule is : one level difference = 1 yard of visibility
        // SPELL_AURA_MOD_STEALTH and SPELL_AURA_MOD_STEALTH_LEVEL are both affecting m_stealth. 
        // SPELL_AURA_MOD_STEALTH are the base stealth spells while SPELL_AURA_MOD_STEALTH_LEVEL are bonus auras and talents
        // Max level stealth spell have 350 SPELL_AURA_MOD_STEALTH
        // So for this next line will equal 0 for the same level and no talent/items boost
        // Talent such as "Master of Deception" (= 15 stealth level) will decrease the detect range by 15y when maxed out
        visibleDistance += float(GetLevelForTarget(obj)) - obj->m_stealth.GetValue(StealthType(i)) / 5.0f;
        // spells like Track Hidden have 30 here, so you can see 30 yards further. Rogue trap detects has 70.
        visibleDistance += (float)m_stealthDetect.GetValue(StealthType(i));

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
        {
            visibleDistance *= 1.08f;
            visibleDistance += 1.5f;
        }

        visibleDistance += tolerance;

        if (visibleDistance <= 0.0f)
            break; //in this case we can already stop here

        if (detector)
        {
            // If this unit is an NPC then player detect range doesn't apply
            if (detector->GetTypeId() == TYPEID_PLAYER && visibleDistance > MAX_PLAYER_STEALTH_DETECT_RANGE)
                visibleDistance = MAX_PLAYER_STEALTH_DETECT_RANGE;

            if (checkAlert && detector->GetTypeId() == TYPEID_UNIT)
            {
                float combatRange = detector->ToCreature()->GetCombatRange() ? detector->ToCreature()->GetCombatRange()->MaxRange : 0.0f;
                if (visibleDistance <= detector->ToCreature()->GetAggroRange(unitTarget) + combatRange)
                    return true;
            }
        }


        if (distance <= visibleDistance)
            return true;
    }

    return false;
}

/**
* Summons group of creatures.
*
* @param group Id of group to summon.
* @param list  List to store pointers to summoned creatures.
*/

void Map::SummonCreatureGroup(uint8 group, std::list<TempSummon*>* list /*= nullptr*/)
{
    std::vector<TempSummonData> const* data = sObjectMgr->GetSummonGroup(GetId(), SUMMONER_TYPE_MAP, group);
    if (!data)
        return;

    for (std::vector<TempSummonData>::const_iterator itr = data->begin(); itr != data->end(); ++itr)
        if (TempSummon* summon = SummonCreature(itr->entry, itr->pos, nullptr, itr->time))
            if (list)
                list->push_back(summon);
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
}

GameObject* WorldObject::SummonGameObject(uint32 entry, Position const& pos, G3D::Quat const& rot, uint32 respawnTime, GOSummonType summonType) const
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
    GameObject* go = sObjectMgr->CreateGameObject(entry);

    if(!go->Create(map->GenerateLowGuid<HighGuid::GameObject>(), entry, map, GetPhaseMask(), pos, rot, 255, GO_STATE_READY))
    {
        delete go;
        return nullptr;
    }
    go->SetRespawnTime(respawnTime);
    if (GetTypeId() == TYPEID_PLAYER || (GetTypeId() == TYPEID_UNIT && summonType == GO_SUMMON_TIMED_OR_CORPSE_DESPAWN)) //not sure how to handle this
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

/**
* Summons group of creatures. Should be called only by instances of Creature and GameObject classes.
*
* @param group Id of group to summon.
* @param list  List to store pointers to summoned creatures.
*/
void WorldObject::SummonCreatureGroup(uint8 group, std::list<TempSummon*>* list /*= nullptr*/)
{
    ASSERT((GetTypeId() == TYPEID_GAMEOBJECT || GetTypeId() == TYPEID_UNIT) && "Only GOs and creatures can summon npc groups!");

    std::vector<TempSummonData> const* data = sObjectMgr->GetSummonGroup(GetEntry(), GetTypeId() == TYPEID_GAMEOBJECT ? SUMMONER_TYPE_GAMEOBJECT : SUMMONER_TYPE_CREATURE, group);
    if (!data)
    {
        TC_LOG_WARN("scripts", "%s (%s) tried to summon non-existing summon group %u.", GetName().c_str(), GetGUID().ToString().c_str(), group);
        return;
    }

    for (std::vector<TempSummonData>::const_iterator itr = data->begin(); itr != data->end(); ++itr)
        if (TempSummon* summon = SummonCreature(itr->entry, itr->pos, itr->type, itr->time))
            if (list)
                list->push_back(summon);
}

Creature* WorldObject::FindNearestCreature(uint32 entry, float range, bool alive) const
{
       Creature *creature = nullptr;
       Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck checker(*this, entry, alive, range);
       Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(this, creature, checker);
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

void WorldObject::GetNearPoint2D(WorldObject const* searcher, float &x, float &y, float distance2d, float absAngle ) const
{
    x = GetPositionX() + (GetCombatReach() + distance2d) * cos(absAngle);
    y = GetPositionY() + (GetCombatReach() + distance2d) * sin(absAngle);

    Trinity::NormalizeMapCoord(x);
    Trinity::NormalizeMapCoord(y);
}

void WorldObject::GetNearPoint(WorldObject const* searcher, float &x, float &y, float &z, float distance2d, float absAngle) const
{
    GetNearPoint2D(searcher, x, y, distance2d, absAngle);
    z = GetPositionZ();
    (searcher ? searcher : this)->UpdateAllowedPositionZ(x, y, z);

    // if detection disabled, return first point
    if (!sWorld->getBoolConfig(CONFIG_DETECT_POS_COLLISION))
        return;

    // return if the point is already in LoS
    if (IsWithinLOS(x, y, z))
        return;

    // remember first point
    float first_x = x;
    float first_y = y;
    float first_z = z;

    // loop in a circle to look for a point in LoS using small steps
    for (float angle = float(M_PI) / 8; angle < float(M_PI) * 2; angle += float(M_PI) / 8)
    {
        GetNearPoint2D(searcher, x, y, distance2d, absAngle + angle);
        z = GetPositionZ();
        (searcher ? searcher : this)->UpdateAllowedPositionZ(x, y, z);
        if (IsWithinLOS(x, y, z))
            return;
    }

    // still not in LoS, give up and return first position found
    x = first_x;
    y = first_y;
    z = first_z;
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

void WorldObject::GetChasePoint(float &x, float &y, float &z, float distance2d, float angle) const
{
    //NYI implemented, use regular GetClosePoint for now
    //Maybe look for Nost or Sunwell impl
    GetClosePoint(x, y, z, distance2d, angle);
}

void WorldObject::GetClosePoint(float &x, float &y, float &z, float distance2d, float angle) const
{
    // angle calculated from current orientation
    GetNearPoint(nullptr, x, y, z, distance2d, GetOrientation() + angle);
}

void WorldObject::GetContactPoint(const WorldObject* obj, float &x, float &y, float &z, float distance2d) const
{
    //on retail, creature follow approximatively at half the max melee distance
    float offset = (GetCombatReach() + obj->GetCombatReach()) / 2.0f + distance2d;
    float angle = GetAbsoluteAngle(obj);
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
Position WorldObject::GetFirstWalkableCollisionPosition(float dist, float angle)
{
    Position pos = GetPosition();
    MovePositionToFirstWalkableCollision(pos, dist, angle);
    return pos;
}

Position WorldObject::GetNearPosition(float dist, float angle)
{
    Position pos = GetPosition();
    MovePosition(pos, dist, angle);
    return pos;
}

void WorldObject::MovePositionToFirstWalkableCollision(Position &pos, float dist, float angle)
{
    angle += GetOrientation();
    float destx, desty, destz;
    destx = pos.m_positionX + dist * std::cos(angle);
    desty = pos.m_positionY + dist * std::sin(angle);
    destz = pos.m_positionZ;

    // Prevent invalid coordinates here, position is unchanged
    if (!Trinity::IsValidMapCoord(destx, desty))
    {
        TC_LOG_ERROR("vmap","WorldObject::MovePositionToFirstWalkableCollision invalid coordinates X: %f and Y: %f were passed!", destx, desty);
        return;
    }
    
    bool const col = ComputeCollisionPosition(pos, { destx, desty, destz }, destx, desty, destz);
    if (col)
        dist = std::sqrt((pos.m_positionX - destx)*(pos.m_positionX - destx) + (pos.m_positionY - desty)*(pos.m_positionY - desty));

    destz = SelectBestZForDestination(destx, desty, destz, col);

    TC_LOG_DEBUG("vmap", "WorldObject::MovePositionToFirstWalkableCollision: Called with %f,%f. Target Z set to %f.", destx, desty, destz);
    
    float step = dist / 10.0f;

    for (uint8 j = 0; j < 10; ++j)
    {
        // do not allow too big z changes. I too much changes, try again to get a position a little bit closer.
        if (fabs(pos.m_positionZ - destz) > 7.5f)
        {
            destx -= step * std::cos(angle);
            desty -= step * std::sin(angle);
            destz = pos.m_positionZ; //reset destz at each step before updating it
            // There should not be any collision between our position and destx, desty, pos.m_positionZ at this point.
            // Use pos.m_positionZ here because destz was not good.
            destz = SelectBestZForDestination(destx, desty, destz, col);
        }
        // we have correct destz now
        else
        {
            pos.Relocate(destx, desty, destz);
            pos.SetOrientation(GetOrientation());

            Trinity::NormalizeMapCoord(pos.m_positionX);
            Trinity::NormalizeMapCoord(pos.m_positionY);
            TC_LOG_TRACE("vmap", "WorldObject::MovePositionToFirstWalkableCollision: Found a suitable position after %u steps", j);
            break;
        }
    }

    TC_LOG_TRACE("vmap", "WorldObject::MovePositionToFirstWalkableCollision: Final target: %f, %f, %f", destx, desty, destz);
}

bool WorldObject::ComputeCollisionPosition(Position const& startPosition, Position const& endPosition, float& x, float& y, float& z) const
{
    Position vmapCollisionPos;
    bool const vmapCollision = VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(GetMapId(), startPosition.m_positionX, startPosition.m_positionY, startPosition.m_positionZ + GetCollisionHeight(), endPosition.m_positionX, endPosition.m_positionY, endPosition.m_positionZ + GetCollisionHeight(), vmapCollisionPos.m_positionX, vmapCollisionPos.m_positionY, vmapCollisionPos.m_positionZ, -CONTACT_DISTANCE * 2.0f);
    Position gameObjectCollisionPos;
    bool const gameObjectCollision = GetMap()->getObjectHitPos(GetPhaseMask(), startPosition.m_positionX, startPosition.m_positionY, startPosition.m_positionZ + GetCollisionHeight(), endPosition.m_positionX, endPosition.m_positionY, endPosition.m_positionZ + GetCollisionHeight(), gameObjectCollisionPos.m_positionX, gameObjectCollisionPos.m_positionY, gameObjectCollisionPos.m_positionZ, -CONTACT_DISTANCE * 2.0f);

    // Both collision occures, check which one is closest to start.
    if (vmapCollision && gameObjectCollision)
    {
        if (startPosition.GetExactDist(vmapCollision) < startPosition.GetExactDist(gameObjectCollisionPos))
        {
            x = vmapCollisionPos.m_positionX;
            y = vmapCollisionPos.m_positionY;
            z = vmapCollisionPos.m_positionZ;
            return true;
        }

        x = gameObjectCollisionPos.m_positionX;
        y = gameObjectCollisionPos.m_positionY;
        z = gameObjectCollisionPos.m_positionZ;
        return true;
    }

    if (vmapCollision)
    {
        x = vmapCollisionPos.m_positionX;
        y = vmapCollisionPos.m_positionY;
        z = vmapCollisionPos.m_positionZ;
        return true;
    }

    if (gameObjectCollision)
    {
        x = gameObjectCollisionPos.m_positionX;
        y = gameObjectCollisionPos.m_positionY;
        z = gameObjectCollisionPos.m_positionZ;
        return true;
    }

    return false;
}

bool WorldObject::isInFront(WorldObject const* target,  float arc) const
{
    return HasInArc(arc, target);
}

bool WorldObject::isInBack(WorldObject const* target, float arc) const
{
    return !HasInArc(2 * M_PI - arc, target);
}

Position WorldObject::GetRandomNearPosition(float radius)
{
    Position pos = GetPosition();
    MovePosition(pos, radius * (float)rand_norm(), (float)rand_norm() * static_cast<float>(2 * M_PI));
    return pos;
}

void Object::BuildMovementUpdate(ByteBuffer* data, uint16 flags) const
{
    Unit const* unit = nullptr;
    WorldObject const* object = nullptr;

    if (isType(TYPEMASK_UNIT))
        unit = ToUnit();
    else
        object = ((WorldObject*)this);

#ifdef LICH_KING
    *data << uint16(flags);                                  // update flags
#else
    *data << uint8(flags);                                   // update flags
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
        *data << unit->GetSpeed(MOVE_PITCH_RATE);
#endif

        // 0x08000000
        if (unit->m_movementInfo.GetMovementFlags() & MOVEMENTFLAG_SPLINE_ENABLED)
            Movement::PacketBuilder::WriteCreate(*unit->movespline, *data);
    }
    else
    {
#ifdef LICH_KING
        bool LK_has_position = false;
        {
            if (flags & UPDATEFLAG_POSITION)
            {
                LK_has_position = true;

                Transport* transport = object->GetTransport();

                if (transport)
                    *data << transport->GetPackGUID();
                else
                    *data << uint8(0);

                *data << object->GetPositionX();
                *data << object->GetPositionY();
                *data << object->GetPositionZ();

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
                    *data << object->GetPositionZ();
                }

                *data << object->GetOrientation();

                if (GetTypeId() == TYPEID_CORPSE)
                    *data << float(object->GetOrientation());
                else
                    *data << float(0);
            }
        }
#endif
        
        // 0x40
        if (flags & UPDATEFLAG_STATIONARY_POSITION
#ifdef LICH_KING
            && LK_has_position == false //LK wont send UPDATEFLAG_STATIONARY_POSITION if UPDATEFLAG_POSITION was present
#endif
           )
        {
            ASSERT(object);
            // 0x02
            //X Y Z are at 0 for transports on bc for transports but whatever, it does not seem used at all by client for them
            *data << object->GetStationaryX();
            *data << object->GetStationaryY();
            *data << object->GetStationaryZ();
            *data << object->GetStationaryO();
        }
    }

    // 0x8
#ifdef LICH_KING
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
                *data << uint32(GetGUID().GetCounter());              // GetGUID().GetCounter()
                break;
            //! Unit, Player and default here are sending wrong values.
            /// @todo Research the proper formula
            case TYPEID_UNIT:
                *data << uint32(0x0000000B);                // unk
                break;
            case TYPEID_PLAYER:
                if (flags & UPDATEFLAG_SELF)
                {
#ifdef LICH_KING
                    *data << uint32(0x0000002F);            // unk
#else
                    *data << uint32(0x00000015);            // unk, can be 0x15 or 0x22
#endif
                } else
                    *data << uint32(0x00000008);            // unk
                break;
            default:
                *data << uint32(0x00000000);                // unk
                break;
        }
    }

    // 0x10
#ifndef LICH_KING
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

                *data << uint32(GetGUID().GetHigh());             // GetGUID().GetHigh()
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

    // 0x80
#ifdef LICH_KING
    if ((build == BUILD_335) && flags & UPDATEFLAG_VEHICLE)
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
        *data << int64(0 /*ToGameObject()->GetPackedWorldRotation()*/); //TODO
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
            GetTypeId(), GetEntry(), GetGUID().GetCounter(), destx, desty);
        return;
    }

    ground = GetMapHeight(destx, desty, MAX_HEIGHT);
    floor = GetMapHeight(destx, desty, pos.m_positionZ);
    destz = fabs(ground - pos.m_positionZ) <= fabs(floor - pos.m_positionZ) ? ground : floor;

    float step = dist/10.0f;

    for (uint8 j = 0; j < 10; ++j)
    {
        // do not allow too big z changes
        if (fabs(pos.m_positionZ - destz) > 6)
        {
            destx -= step * std::cos(angle);
            desty -= step * std::sin(angle);
            ground = GetMapHeight(destx, desty, MAX_HEIGHT);
            floor = GetMapHeight(destx, desty, pos.m_positionZ);
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
            ObjectGuid guid = source->GetCasterGUID();

            if (guid.IsPlayer())
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
        if (i_playerSet.find(player->GetGUID().GetCounter()) == i_playerSet.end() && player->HaveAtClient(&i_object))
        {
            i_object.BuildFieldsUpdate(player, i_updateDatas);
            i_playerSet.insert(player->GetGUID().GetCounter());
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

ObjectGuid WorldObject::GetTransGUID() const
{
    if (GetTransport())
        return GetTransport()->GetGUID();
    return ObjectGuid::Empty;
}

float WorldObject::GetFloorZ() const
{
    if (!IsInWorld())
        return m_staticFloorZ;
    //this is the same as Map::GetHeight but we use our cached value for the map height instead for performance
    return std::max<float>(m_staticFloorZ, GetMap()->GetGameObjectFloor(GetPhaseMask(), GetPositionX(), GetPositionY(), GetPositionZ() + GetCollisionHeight()));
}

float WorldObject::GetMapWaterOrGroundLevel(float x, float y, float z, float* ground/* = nullptr*/) const
{
    return GetMap()->GetWaterOrGroundLevel(GetPhaseMask(), x, y, z, ground,
        isType(TYPEMASK_UNIT) ? !static_cast<Unit const*>(this)->HasAuraType(SPELL_AURA_WATER_WALK) : false,
        GetCollisionHeight());
}

float WorldObject::GetMapHeight(float x, float y, float z, bool vmap/* = true*/, float distanceToSearch/* = DEFAULT_HEIGHT_SEARCH*/) const
{
    return GetMap()->GetHeight(GetPhaseMask(), x, y, z, vmap, distanceToSearch, GetCollisionHeight());
}

ObjectGuid WorldObject::GetCharmerOrOwnerOrOwnGUID() const
{
    if (ObjectGuid guid = GetCharmerOrOwnerGUID())
        return guid;
    return GetGUID();
}

Unit* WorldObject::GetOwner() const
{
    return ObjectAccessor::GetUnit(*this, GetOwnerGUID());
}

Unit* WorldObject::GetCharmerOrOwner() const
{
    if (Unit const* unit = ToUnit())
        return unit->GetCharmerOrOwner();
    else if (GameObject const* go = ToGameObject())
        return go->GetOwner();

    return nullptr;
}

Unit* WorldObject::GetCharmerOrOwnerOrSelf() const
{
    if (Unit* u = GetCharmerOrOwner())
        return u;

    return const_cast<WorldObject*>(this)->ToUnit();
}

Player* WorldObject::GetCharmerOrOwnerPlayerOrPlayerItself() const
{
    ObjectGuid guid = GetCharmerOrOwnerGUID();
    if (guid.IsPlayer())
        return ObjectAccessor::GetPlayer(*this, guid);

    return const_cast<WorldObject*>(this)->ToPlayer();
}

Player* WorldObject::GetAffectingPlayer() const
{
    if (!GetCharmerOrOwnerGUID())
        return const_cast<WorldObject*>(this)->ToPlayer();

    if (Unit* owner = GetCharmerOrOwner())
        return owner->GetCharmerOrOwnerPlayerOrPlayerItself();

    return nullptr;
}

Player* WorldObject::GetSpellModOwner() const
{
    if (Player* player = const_cast<WorldObject*>(this)->ToPlayer())
        return player;

    if (GetTypeId() == TYPEID_UNIT)
    {
        Creature const* creature = ToCreature();
        if (creature->IsPet() || creature->IsTotem())
        {
            if (Unit* owner = creature->GetOwner())
                return owner->ToPlayer();
        }
    }
    else if (GetTypeId() == TYPEID_GAMEOBJECT)
    {
        GameObject const* go = ToGameObject();
        if (Unit* owner = go->GetOwner())
            return owner->ToPlayer();
    }

    return nullptr;
}

int32 WorldObject::CalculateSpellDamage(SpellInfo const* spellProto, uint8 effect_index, int32 const* basePoints /*= nullptr*/) const
{
    return spellProto->Effects[effect_index].CalcValue(this, basePoints);
}

float WorldObject::GetSpellMaxRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const
{
    if (!spellInfo->RangeEntry)
        return 0.0f;
#ifdef LICH_KING
    if (spellInfo->RangeEntry->maxRangeFriend == spellInfo->RangeEntry->maxRangeHostile)
        return spellInfo->GetMaxRange();
#else
    //sun: aoe trap spells such as "Frost Trap Aura" (13810) have self range only, while they do have an extended rage on LK. We need to make an exception for them to keep this TC code compatible.
    if (GetTypeId() == TYPEID_GAMEOBJECT)
        if (spellInfo->RangeEntry->ID == 1)
            return 50000.0f;
#endif
    if (!target)
        return spellInfo->GetMaxRange(true);

    return spellInfo->GetMaxRange(!IsHostileTo(target));
}

float WorldObject::GetSpellMinRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const
{
    if (!spellInfo->RangeEntry)
        return 0.0f;
#ifdef LICH_KING
    if (spellInfo->RangeEntry->minRangeFriend == spellInfo->RangeEntry->minRangeHostile)
        return spellInfo->GetMinRange();

    return spellInfo->GetMinRange(!IsHostileTo(target));
#else
    return spellInfo->GetMinRange();
#endif
}

float WorldObject::ApplyEffectModifiers(SpellInfo const* spellProto, uint8 effect_index, float value) const
{
    if (Player* modOwner = GetSpellModOwner())
    {
        modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_ALL_EFFECTS, value);
        switch (effect_index)
        {
        case EFFECT_0:
            modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_EFFECT1, value);
            break;
        case EFFECT_1:
            modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_EFFECT2, value);
            break;
        case EFFECT_2:
            modOwner->ApplySpellMod(spellProto->Id, SPELLMOD_EFFECT3, value);
            break;
        }
    }
    return value;
}

int32 WorldObject::CalcSpellDuration(SpellInfo const* spellProto) const
{
    uint8 comboPoints = (GetTypeId() == TYPEID_PLAYER) ? ToPlayer()->GetComboPoints() : 0;
    //TC uint8 comboPoints = GetComboPoints();

    int32 minduration = spellProto->GetDuration();
    int32 maxduration = spellProto->GetMaxDuration();

    int32 duration;

    if (comboPoints && minduration != -1 && minduration != maxduration)
        duration = minduration + int32((maxduration - minduration) * comboPoints / 5);
    else
        duration = minduration;

    return duration;
}

int32 WorldObject::ModSpellDuration(SpellInfo const* spellProto, WorldObject const* target, int32 duration, bool positive, uint32 effectMask) const
{
    // don't mod permanent auras duration
    if (duration < 0)
        return duration;

#ifdef LICH_KING
    // some auras are not affected by duration modifiers
    if (spellProto->HasAttribute(SPELL_ATTR7_IGNORE_DURATION_MODS))
        return duration;
#endif

    Unit const* unitTarget = target->ToUnit();
    if (!unitTarget)
        return duration;

    // cut duration only of negative effects
    if (!positive)
    {
        int32 mechanicMask = spellProto->GetSpellMechanicMaskByEffectMask(SpellEffectMask(effectMask));
        auto mechanicCheck = [mechanicMask](AuraEffect const* aurEff) -> bool
        {
            if (mechanicMask & (1 << aurEff->GetMiscValue()))
                return true;
            return false;
        };

        // Find total mod value (negative bonus)
        int32 durationMod_always = unitTarget->GetTotalAuraModifier(SPELL_AURA_MECHANIC_DURATION_MOD, mechanicCheck);
        // Find max mod (negative bonus)
        int32 durationMod_not_stack = unitTarget->GetMaxNegativeAuraModifier(SPELL_AURA_MECHANIC_DURATION_MOD_NOT_STACK, mechanicCheck);

        // Select strongest negative mod
        int32 durationMod = std::min(durationMod_always, durationMod_not_stack);
        if (durationMod != 0)
            AddPct(duration, durationMod);

        // there are only negative mods currently
#ifdef LICH_KING
        durationMod_always = target->GetTotalAuraModifierByMiscValue(SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL, spellProto->Dispel);
        durationMod_not_stack = target->GetMaxNegativeAuraModifierByMiscValue(SPELL_AURA_MOD_AURA_DURATION_BY_DISPEL_NOT_STACK, spellProto->Dispel);

        durationMod = 0;
        if (durationMod_always > durationMod_not_stack)
            durationMod += durationMod_not_stack;
        else
            durationMod += durationMod_always;

        if (durationMod != 0)
            AddPct(duration, durationMod);
#endif
    }
    else
    {
        // else positive mods here, there are no currently
        // when there will be, change GetTotalAuraModifierByMiscValue to GetTotalPositiveAuraModifierByMiscValue

#ifdef LICH_KING
        // Mixology - duration boost
        if (target->GetTypeId() == TYPEID_PLAYER)
        {
            if (spellProto->SpellFamilyName == SPELLFAMILY_POTION && (
                sSpellMgr->IsSpellMemberOfSpellGroup(spellProto->Id, SPELL_GROUP_ELIXIR_BATTLE) ||
                sSpellMgr->IsSpellMemberOfSpellGroup(spellProto->Id, SPELL_GROUP_ELIXIR_GUARDIAN)))
            {
                if (target->HasAura(53042) && target->HasSpell(spellProto->Effects[0].TriggerSpell))
                    duration *= 2;
            }
        }
#endif
    }
#ifdef LICH_KING
    // Glyphs which increase duration of selfcast buffs
    if (target == this)
    {
        switch (spellProto->SpellFamilyName)
        {
        case SPELLFAMILY_DRUID:
            if (spellProto->SpellFamilyFlags[0] & 0x100)
            {
                // Glyph of Thorns
                if (AuraEffect* aurEff = GetAuraEffect(57862, 0))
                    duration += aurEff->GetAmount() * MINUTE * IN_MILLISECONDS;
            }
            break;
        case SPELLFAMILY_PALADIN:
            if ((spellProto->SpellFamilyFlags[0] & 0x00000002) && spellProto->SpellIconID == 298)
            {
                // Glyph of Blessing of Might
                if (AuraEffect* aurEff = GetAuraEffect(57958, 0))
                    duration += aurEff->GetAmount() * MINUTE * IN_MILLISECONDS;
            }
            else if ((spellProto->SpellFamilyFlags[0] & 0x00010000) && spellProto->SpellIconID == 306)
            {
                // Glyph of Blessing of Wisdom
                if (AuraEffect* aurEff = GetAuraEffect(57979, 0))
                    duration += aurEff->GetAmount() * MINUTE * IN_MILLISECONDS;
            }
            break;
        }
    }
#endif
    return std::max(duration, 0);
}

void WorldObject::ModSpellCastTime(SpellInfo const* spellInfo, int32& castTime, Spell* spell /*= nullptr*/) const
{
    if (!spellInfo || castTime < 0)
        return;

    // called from caster
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_CASTING_TIME, castTime, spell);

    Unit const* unitCaster = ToUnit();
    if (!unitCaster)
        return;

    if (!(spellInfo->HasAttribute(SPELL_ATTR0_ABILITY) || spellInfo->HasAttribute(SPELL_ATTR0_TRADESPELL) || spellInfo->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS)) &&
        ((GetTypeId() == TYPEID_PLAYER && spellInfo->SpellFamilyName) || GetTypeId() == TYPEID_UNIT))
        castTime = unitCaster->CanInstantCast() ? 0 : int32(float(castTime) * unitCaster->GetFloatValue(UNIT_MOD_CAST_SPEED));
    else if (spellInfo->HasAttribute(SPELL_ATTR0_RANGED) && !spellInfo->HasAttribute(SPELL_ATTR2_AUTOREPEAT_FLAG))
        castTime = int32(float(castTime) * unitCaster->m_modAttackSpeedPct[RANGED_ATTACK]);
#ifdef LICH_KING
    else if (spellInfo->SpellVisual[0] == 3881 && unitCaster->HasAura(67556)) // cooking with Chef Hat.
        castTime = 500;
#endif
}

void WorldObject::ModSpellDurationTime(SpellInfo const* spellInfo, int32& castTime, Spell* spell /*= nullptr*/) const
{
    if (!spellInfo || castTime < 0)
        return;

    if (spellInfo->IsChanneled() && !spellInfo->HasAttribute(SPELL_ATTR5_HASTE_AFFECT_DURATION))
        return;

    //called from caster
    if (Player* modOwner = GetSpellModOwner())
        modOwner->ApplySpellMod(spellInfo->Id, SPELLMOD_CASTING_TIME, castTime, spell);

    Unit const* unitCaster = ToUnit();
    if (!unitCaster)
        return;

    //magic spells
    if (!(spellInfo->HasAttribute(SPELL_ATTR0_ABILITY) || spellInfo->HasAttribute(SPELL_ATTR0_TRADESPELL) || spellInfo->HasAttribute(SPELL_ATTR3_NO_DONE_BONUS)) &&
        ((GetTypeId() == TYPEID_PLAYER && spellInfo->SpellFamilyName) || GetTypeId() == TYPEID_UNIT))
        castTime = int32(float(castTime) * GetFloatValue(UNIT_MOD_CAST_SPEED));
    //ranged attacks
    else if (spellInfo->HasAttribute(SPELL_ATTR0_RANGED) && !spellInfo->HasAttribute(SPELL_ATTR2_AUTOREPEAT_FLAG))
        castTime = int32(float(castTime) * unitCaster->m_modAttackSpeedPct[RANGED_ATTACK]);
}

float WorldObject::MeleeSpellMissChance(Unit const* /*victim*/, WeaponAttackType /*attType*/, int32 /*skillDiff*/, uint32 /*spellId*/) const
{
    return 0.0f;
}

SpellMissInfo WorldObject::MeleeSpellHitResult(Unit* /*victim*/, SpellInfo const* /*spellInfo*/) const
{
    return SPELL_MISS_NONE;
}

SpellMissInfo WorldObject::MagicSpellHitResult(Unit *pVictim, SpellInfo const *spell, Item* castItem) const
{
    // Can`t miss on dead target (on skinning for example)
    if ((!pVictim->IsAlive() && pVictim->GetTypeId() != TYPEID_PLAYER))
        return SPELL_MISS_NONE;
        
    // Always 1% resist chance. Send this as SPELL_MISS_MISS (note that this is not BC blizzlike, this was changed in WotLK).
    uint32 rand = urand(0, 9999);
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
    if (Unit const* unit = ToUnit())
        modHitChance += unit->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT, schoolMask);

    // Chance hit from victim SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE auras
    modHitChance += pVictim->GetTotalAuraModifierByMiscMask(SPELL_AURA_MOD_ATTACKER_SPELL_HIT_CHANCE, schoolMask);

    // Reduce spell hit chance for Area of effect spells from victim SPELL_AURA_MOD_AOE_AVOIDANCE aura
    if(spell->IsAffectingArea())
        modHitChance -= pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_AOE_AVOIDANCE);

    // Reduce spell hit chance for dispel mechanic spells from victim SPELL_AURA_MOD_DISPEL_RESIST
    if (IsDispelSpell(spell))
        modHitChance -= pVictim->GetTotalAuraModifier(SPELL_AURA_MOD_DISPEL_RESIST);

    // Chance resist mechanic (select max value from every mechanic spell effect)
    int32 resist_chance = pVictim->GetMechanicResistChance(spell);
    modHitChance -= resist_chance;

    // Chance resist debuff - SPELL_AURA_MOD_DEBUFF_RESISTANCE handling. Only affects spells that debuffs.
    /* Only two spells involved for BC: 
    - Master Poisoner -increase resist to poison (dispel type 4)
    - Pure of Heart - increase resist to curse and disease (2 and 3)
    */
    if (!spell->IsPositive() && !spell->HasAttribute(SPELL_ATTR4_IGNORE_RESISTANCES)) {
        bool hasAura = false;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (spell->Effects[i].IsAura())
            {
                hasAura = true;
                break;
            }
        }

        if (hasAura)
        {
            //TODO LK: LK spells seems to have different values in the spells (I believe: * -100)
            modHitChance -= pVictim->GetMaxPositiveAuraModifierByMiscValue(SPELL_AURA_MOD_DEBUFF_RESISTANCE, static_cast<int32>(spell->Dispel)); 
        }

        // resistance chance for binary spells, equals to average damage reduction of non-binary spell
        if (spell->IsBinarySpell() && (spell->GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC))
            modHitChance -= int32(Unit::CalculateAverageResistReduction(this, spell->GetSchoolMask(), pVictim, spell) * 100.f); 
    }

    int32 HitChance = modHitChance * 100;
    // Increase hit chance from attacker SPELL_AURA_MOD_SPELL_HIT_CHANCE and attacker ratings
    if (Unit const* unit = ToUnit())
        HitChance += int32(unit->m_modSpellHitChance*100.0f);

    // Decrease hit chance from victim rating bonus
    if (pVictim->GetTypeId() == TYPEID_PLAYER)
        HitChance -= int32((pVictim->ToPlayer())->GetRatingBonusValue(CR_HIT_TAKEN_SPELL)*100.0f);

    // Hack - Always have 99% on taunts for Nalorakk & Brutallus.
    if ((spell->Effects[0].ApplyAuraName == SPELL_AURA_MOD_TAUNT || spell->Effects[1].ApplyAuraName == SPELL_AURA_MOD_TAUNT || spell->Effects[2].ApplyAuraName == SPELL_AURA_MOD_TAUNT)
        && (pVictim->GetEntry() == 24882 || pVictim->GetEntry() == 23576)) 
    {
        HitChance = 9900;
    }

    // Always have a minimal 1% chance
    if (HitChance < 100) 
        HitChance = 100;

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
SpellMissInfo WorldObject::SpellHitResult(Unit *pVictim, SpellInfo const* spellInfo, bool canReflect, Item* castItem) const
{
    if (ToCreature() && ToCreature()->IsTotem())
        if (Unit *owner = GetOwner())
            return owner->SpellHitResult(pVictim, spellInfo, canReflect, castItem);

    // Return evade for units in evade mode
    if (pVictim->GetTypeId()==TYPEID_UNIT && pVictim->ToCreature()->IsEvadingAttacks())
        return SPELL_MISS_EVADE;

    // All positive spells can`t miss (unless immune)
    // TODO: client not show miss log for this spells - so need find info for this in dbc and use it!
    if (spellInfo->IsPositive())
        return SPELL_MISS_NONE;

    if(this == pVictim)
        return SPELL_MISS_NONE;
        
    //TC_LOG_INFO("SpellHitResult1 %u", spell->Id);

    // Try victim reflect spell
    if (canReflect)
    {
        int32 reflectchance = pVictim->GetTotalAuraModifier(SPELL_AURA_REFLECT_SPELLS);
        reflectchance += pVictim->GetTotalAuraModifierByMiscMask(SPELL_AURA_REFLECT_SPELLS_SCHOOL, spellInfo->GetSchoolMask());

        if (reflectchance > 0 && roll_chance_i(reflectchance))
            return SPELL_MISS_REFLECT;
    }

    if (spellInfo->HasAttribute(SPELL_ATTR3_IGNORE_HIT_RESULT))
        return SPELL_MISS_NONE;

    // Check for immune
    if (pVictim->IsImmunedToSpell(spellInfo, this))
        return SPELL_MISS_IMMUNE;

    // Check for immune (use charges)
    if (!spellInfo->HasAttribute(SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY) && spellInfo->HasOnlyDamageEffects() && pVictim->IsImmunedToDamage(spellInfo))
        return SPELL_MISS_IMMUNE;

    switch (spellInfo->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_RANGED:
        case SPELL_DAMAGE_CLASS_MELEE:
            return MeleeSpellHitResult(pVictim, spellInfo);
        case SPELL_DAMAGE_CLASS_NONE:
            if (spellInfo->SchoolMask & SPELL_SCHOOL_MASK_SPELL)
                return MagicSpellHitResult(pVictim, spellInfo, castItem);
            else
                return SPELL_MISS_NONE;
        case SPELL_DAMAGE_CLASS_MAGIC:
            return MagicSpellHitResult(pVictim, spellInfo, castItem);
    }

    return SPELL_MISS_NONE;
}

FactionTemplateEntry const* WorldObject::GetFactionTemplateEntry() const
{
    FactionTemplateEntry const* entry = sFactionTemplateStore.LookupEntry(GetFaction());
    if (!entry)
    {
        if (Player const* player = ToPlayer())
            TC_LOG_ERROR("entities", "Player %s has invalid faction (faction template id) #%u", player->GetName().c_str(), GetFaction());
        else if (Creature const* creature = ToCreature())
            TC_LOG_ERROR("entities", "Creature (template id: %u) has invalid faction (faction template id) #%u", creature->GetCreatureTemplate()->Entry, GetFaction());
        else if (GameObject const* go = ToGameObject())
            TC_LOG_ERROR("entities", "GameObject (template id: %u) has invalid faction (faction template id) #%u", go->GetGOInfo()->entry, GetFaction());
        else
            TC_LOG_ERROR("entities", "WorldObject (name: %s, type: %u) has invalid faction (faction template id) #%u", GetName().c_str(), uint32(GetTypeId()), GetFaction());
    }

    return entry;
}

ReputationRank WorldObject::GetReactionTo(WorldObject const* target) const
{
    // always friendly to self
    if (this == target)
        return REP_FRIENDLY;

    // always friendly to charmer or owner
    if (GetCharmerOrOwnerOrSelf() == target->GetCharmerOrOwnerOrSelf())
        return REP_FRIENDLY;

    { //HACK TIME
      // Karazhan chess exception
        if (GetFaction() == 1689 && target->GetFaction() == 1690)
            return REP_HOSTILE;
        if (GetFaction() == 1690 && target->GetFaction() == 1689)
            return REP_HOSTILE;
    }

    Player const* selfPlayerOwner = GetAffectingPlayer();
    Player const* targetPlayerOwner = target->GetAffectingPlayer();

    // check forced reputation to support SPELL_AURA_FORCE_REACTION
    if (selfPlayerOwner)
    {
        if (FactionTemplateEntry const* targetFactionTemplateEntry = target->GetFactionTemplateEntry())
            if (ReputationRank const* repRank = selfPlayerOwner->GetReputationMgr().GetForcedRankIfAny(targetFactionTemplateEntry))
                return *repRank;
    }
    else if (targetPlayerOwner)
    {
        if (FactionTemplateEntry const* selfFactionTemplateEntry = GetFactionTemplateEntry())
            if (ReputationRank const* repRank = targetPlayerOwner->GetReputationMgr().GetForcedRankIfAny(selfFactionTemplateEntry))
                return *repRank;
    }

    Unit const* unit = ToUnit();
    Unit const* targetUnit = target->ToUnit();
    if (unit && unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
    {
        if (targetUnit && targetUnit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        {
            if (selfPlayerOwner && targetPlayerOwner)
            {
                // always friendly to other unit controlled by player, or to the player himself
                if (selfPlayerOwner == targetPlayerOwner)
                    return REP_FRIENDLY;

                // duel - always hostile to opponent
                if (selfPlayerOwner->duel && selfPlayerOwner->duel->Opponent == targetPlayerOwner && selfPlayerOwner->duel->State == DUEL_STATE_IN_PROGRESS)
                    return REP_HOSTILE;

                // Duel area case. check after duel
                if ((selfPlayerOwner->IsInDuelArea())
                    || (targetPlayerOwner->IsInDuelArea())
                    )
                    return REP_FRIENDLY;


                // same group - checks dependant only on our faction - skip FFA_PVP for example
                if (selfPlayerOwner->IsInRaidWith(targetPlayerOwner))
                    return REP_FRIENDLY; // return true to allow config option AllowTwoSide.Interaction.Group to work
                                         // however client seems to allow mixed group parties, because in 13850 client it works like:
                                         // return GetFactionReactionTo(GetFactionTemplateEntry(), target);
            }

            // check FFA_PVP
            if (unit->IsFFAPvP() && targetUnit->IsFFAPvP())
                return REP_HOSTILE;

            if (selfPlayerOwner)
            {
                if (FactionTemplateEntry const* targetFactionTemplateEntry = target->GetFactionTemplateEntry())
                {
                    if (ReputationRank const* repRank = selfPlayerOwner->GetReputationMgr().GetForcedRankIfAny(targetFactionTemplateEntry))
                        return *repRank;

                    if (!selfPlayerOwner->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_IGNORE_REPUTATION))
                    {
                        if (FactionEntry const* targetFactionEntry = sFactionStore.LookupEntry(targetFactionTemplateEntry->faction))
                        {
                            if (targetFactionEntry->CanHaveReputation())
                            {
                                // check contested flags
                                if (targetFactionTemplateEntry->factionFlags & FACTION_TEMPLATE_FLAG_CONTESTED_GUARD
                                    && selfPlayerOwner->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP))
                                    return REP_HOSTILE;

                                // if faction has reputation, hostile state depends only from AtWar state
                                if (selfPlayerOwner->GetReputationMgr().IsAtWar(targetFactionEntry))
                                    return REP_HOSTILE;

                                return REP_FRIENDLY;
                            }
                        }
                    }
                }
            }
        }
    }
    // do checks dependant only on our faction
    return GetFactionReactionTo(GetFactionTemplateEntry(), target);
}

ReputationRank WorldObject::GetFactionReactionTo(FactionTemplateEntry const* factionTemplateEntry, WorldObject const* target) const
{
    // always neutral when no template entry found
    if (!factionTemplateEntry)
        return REP_NEUTRAL;

    FactionTemplateEntry const* targetFactionTemplateEntry = target->GetFactionTemplateEntry();
    if (!targetFactionTemplateEntry)
        return REP_NEUTRAL;

    // sunwell: check forced reputation for self also
    if (Player const* selfPlayerOwner = GetAffectingPlayer())
    {
        if (ReputationRank const* repRank = selfPlayerOwner->GetReputationMgr().GetForcedRankIfAny(target->GetFactionTemplateEntry()))
            return *repRank;
    }

    if (Player const* targetPlayerOwner = target->GetAffectingPlayer())
    {
        // check contested flags
        if (factionTemplateEntry->factionFlags & FACTION_TEMPLATE_FLAG_CONTESTED_GUARD
            && targetPlayerOwner->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP))
            return REP_HOSTILE;
        if (ReputationRank const* repRank = targetPlayerOwner->GetReputationMgr().GetForcedRankIfAny(factionTemplateEntry))
            return *repRank;

        if (target->ToUnit() && !target->HasFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_IGNORE_REPUTATION))
        {
            if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionTemplateEntry->faction))
            {
                if (factionEntry->CanHaveReputation())
                {
                    // CvP case - check reputation, don't allow state higher than neutral when at war
                    ReputationRank repRank = targetPlayerOwner->GetReputationMgr().GetRank(factionEntry);
                    if (targetPlayerOwner->GetReputationMgr().IsAtWar(factionEntry))
                        repRank = std::min(REP_NEUTRAL, repRank);
                    return repRank;
                }
            }
        }
    }

    // common faction based check
    if (factionTemplateEntry->IsHostileTo(*targetFactionTemplateEntry))
        return REP_HOSTILE;
    if (factionTemplateEntry->IsFriendlyTo(*targetFactionTemplateEntry))
        return REP_FRIENDLY;
    if (targetFactionTemplateEntry->IsFriendlyTo(*factionTemplateEntry))
        return REP_FRIENDLY;
    if (factionTemplateEntry->factionFlags & FACTION_TEMPLATE_FLAG_HOSTILE_BY_DEFAULT)
        return REP_HOSTILE;
    // neutral by default
    return REP_NEUTRAL;
}

bool WorldObject::IsHostileTo(WorldObject const* unit) const
{
    return GetReactionTo(unit) <= REP_HOSTILE;
}

bool WorldObject::IsFriendlyTo(WorldObject const* unit) const
{
    return GetReactionTo(unit) >= REP_FRIENDLY;
}

bool WorldObject::IsHostileToPlayers() const
{
    FactionTemplateEntry const* my_faction = GetFactionTemplateEntry();
    if (!my_faction || !my_faction->faction)
        return false;

    FactionEntry const* raw_faction = sFactionStore.LookupEntry(my_faction->faction);
    if (raw_faction && raw_faction->reputationListID >= 0)
        return false;

    return my_faction->IsHostileToPlayers();
}

bool WorldObject::IsNeutralToAll() const
{
    FactionTemplateEntry const* my_faction = GetFactionTemplateEntry();
    if (!my_faction || !my_faction->faction)
        return true;

    FactionEntry const* raw_faction = sFactionStore.LookupEntry(my_faction->faction);
    if (raw_faction && raw_faction->reputationListID >= 0)
        return false;

    return my_faction->IsNeutralToAll();
}

uint32 WorldObject::CastSpell(SpellCastTargets const& targets, uint32 spellId, CastSpellExtraArgs const& args)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
    {
        TC_LOG_ERROR("entities.unit", "CastSpell: unknown spell %u by caster %s", spellId, GetGUID().ToString().c_str());
        return SPELL_FAILED_UNKNOWN;
    }

    Spell* spell = new Spell(this, info, args.TriggerFlags, args.OriginalCaster, nullptr, args.SkipHit);
    for (auto const& pair : args.SpellValueOverrides)
        spell->SetSpellValue(pair.first, pair.second);

    spell->SetForceHitResult(args.ForceHitResult);

    spell->m_CastItem = args.CastItem;
    return spell->prepare(targets, args.TriggeringAura);
}

uint32 WorldObject::CastSpell(WorldObject* target, uint32 spellId, CastSpellExtraArgs const& args)
{
    SpellCastTargets targets;
    if (target)
    {
        if (Unit* unitTarget = target->ToUnit())
            targets.SetUnitTarget(unitTarget);
        else if (GameObject* goTarget = target->ToGameObject())
            targets.SetGOTarget(goTarget);
        else
        {
            TC_LOG_ERROR("entities.unit", "CastSpell: Invalid target %s passed to spell cast by %s", target->GetGUID().ToString().c_str(), GetGUID().ToString().c_str());
            return SPELL_FAILED_BAD_TARGETS;
        }
    }
    return CastSpell(targets, spellId, args);
}

uint32 WorldObject::CastSpell(Position const& dest, uint32 spellId, CastSpellExtraArgs const& args)
{
    SpellCastTargets targets;
    targets.SetDst(dest);
    return CastSpell(targets, spellId, args);
}

// function based on function Unit::CanAttack from 13850 client
bool WorldObject::IsValidAttackTarget(WorldObject const* target, SpellInfo const* bySpell /*= nullptr*/) const
{
    ASSERT(target);

    // some positive spells can be casted at hostile target
    bool isPositiveSpell = bySpell && bySpell->IsPositive();

    // can't attack self (spells can, attribute check)
    if (!bySpell && this == target)
        return false;

    // can't attack unattackable units
    Unit const* unitTarget = target->ToUnit();
    if (unitTarget && unitTarget->HasUnitState(UNIT_STATE_UNATTACKABLE))
        return false;

    // can't attack GMs
    if (target->GetTypeId() == TYPEID_PLAYER && target->ToPlayer()->IsGameMaster())
        return false;

    Unit const* unit = ToUnit();
    // visibility checks (only units)
    if (unit)
    {
        // can't attack invisible
        if (!bySpell || !bySpell->HasAttribute(SPELL_ATTR6_CAN_TARGET_INVISIBLE))
        {
            if (!unit->CanSeeOrDetect(target, bySpell && bySpell->IsAffectingArea()))
                return false;
        }
    }

    // can't attack dead
    if ((!bySpell || !bySpell->IsAllowingDeadTarget()) && unitTarget && !unitTarget->IsAlive())
        return false;

    // can't attack untargetable 
    // sunstrider: only apply this to player, allow easier scripting
    if (unit && unit->IsControlledByPlayer()
        && (!bySpell || !bySpell->HasAttribute(SPELL_ATTR6_CAN_TARGET_UNTARGETABLE))
        && unitTarget && unitTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
        return false;

    Player const* playerAttacker = ToPlayer();
#ifdef LICH_KING
    if (playerAttacker)
    {
        if (playerAttacker->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_UBER))
            return false;
    }
#endif

    //sunstrider: for arena spectators
    if (playerAttacker && playerAttacker->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_COMMENTATOR) /*|| playerAttacker->IsSpectator()*/)
        return false;

    // check flags
    if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_TAXI_FLIGHT | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NON_ATTACKABLE_2))
        return false;

    // ignore immunity flags when assisting
    if (unit && unitTarget && !(isPositiveSpell && bySpell->HasAttribute(SPELL_ATTR6_ASSIST_IGNORE_IMMUNE_FLAG)))
    {
        if (!unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && unitTarget->IsImmuneToNPC())
            return false;

        if (!unitTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && unit->IsImmuneToNPC())
            return false;

        if (unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && unitTarget->IsImmuneToPC())
            return false;

        if (unitTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && unit->IsImmuneToPC())
            return false;
    }

    // CvC case - can attack each other only when one of them is hostile
    if (ToUnit() && !HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && target->ToUnit() && !target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        return IsHostileTo(target) || target->IsHostileTo(this);

    // PvP, PvC, CvP case
    // can't attack friendly targets
    if (IsFriendlyTo(target) || target->IsFriendlyTo(this))
        return false;

    Player const* playerAffectingAttacker = ToUnit() && HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) ? GetAffectingPlayer() : nullptr;
    Player const* playerAffectingTarget = target->ToUnit() && target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) ? target->GetAffectingPlayer() : nullptr;

    // Not all neutral creatures can be attacked (even some unfriendly faction does not react aggresive to you, like Sporaggar)
    if ((playerAffectingAttacker && !playerAffectingTarget) || (!playerAffectingAttacker && playerAffectingTarget))
    {
        Player const* player = playerAffectingAttacker ? playerAffectingAttacker : playerAffectingTarget;

        if (Unit const* creature = playerAffectingAttacker ? target->ToUnit() : ToUnit())
        {
            if (creature->IsContestedGuard() && player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_CONTESTED_PVP))
                return true;

            if (FactionTemplateEntry const* factionTemplate = creature->GetFactionTemplateEntry())
            {
                if (!(player->GetReputationMgr().GetForcedRankIfAny(factionTemplate)))
                    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(factionTemplate->faction))
                        if (FactionState const* repState = player->GetReputationMgr().GetState(factionEntry))
                            if (!(repState->Flags & FACTION_FLAG_AT_WAR))
                                return false;

            }
        }
    }

#ifdef LICH_KING
    Creature const* creatureAttacker = ToCreature();
    if (creatureAttacker && (creatureAttacker->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_TREAT_AS_RAID_UNIT))
        return false;
#endif

    // check duel - before sanctuary checks
    if (playerAffectingAttacker && playerAffectingTarget)
        if (playerAffectingAttacker->duel && playerAffectingAttacker->duel->Opponent == playerAffectingTarget && playerAffectingAttacker->duel->State == DUEL_STATE_IN_PROGRESS)
            return true;

    // PvP case - can't attack when attacker or target are in sanctuary
    // however, 13850 client doesn't allow to attack when one of the unit's has sanctuary flag and is pvp
    if (unitTarget && unitTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && unit && unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) && (unitTarget->IsInSanctuary() || unit->IsInSanctuary()))
        return false;

    // additional checks - only PvP case
    if (playerAffectingAttacker && playerAffectingTarget)
    {
        if (unitTarget->IsPvP())
            return true;

        if (unit->IsFFAPvP() && unitTarget->IsFFAPvP())
            return true;

#ifdef LICH_KING
        return unit->HasByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_UNK1) ||
        unitTarget->HasByteFlag(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PVP_FLAG, UNIT_BYTE2_FLAG_UNK1);
#else
        return false; //not 100% sure about this, is there any condition where IsPvP should be bypassed?
#endif
    }

    return true;
}

// function based on function Unit::CanAssist from 13850 client
bool WorldObject::IsValidAssistTarget(WorldObject const* target, SpellInfo const* bySpell /*= nullptr*/) const
{
    ASSERT(target);

    // can assist to self
    if (this == target)
        return true;

    // can't assist unattackable units
    Unit const* unitTarget = target->ToUnit();
    if (unitTarget && unitTarget->HasUnitState(UNIT_STATE_UNATTACKABLE))
        return false;

    // can't assist GMs
    if (target->GetTypeId() == TYPEID_PLAYER && target->ToPlayer()->IsGameMaster())
        return false;

    Unit const* unit = ToUnit();
#ifdef LICH_KING
    // can't assist own vehicle or passenger
    if (unit && unitTarget && unit->GetVehicle())
    {
        if (unit->IsOnVehicle(unitTarget))
            return false;

        if (unit->GetVehicleBase()->IsOnVehicle(unitTarget))
            return false;
    }
#endif

    // can't assist invisible
    if ((!bySpell || !bySpell->HasAttribute(SPELL_ATTR6_CAN_TARGET_INVISIBLE)) && !CanSeeOrDetect(target, bySpell && bySpell->IsAffectingArea()))
        return false;

    // can't assist dead
    if ((!bySpell || !bySpell->IsAllowingDeadTarget()) && unitTarget && !unitTarget->IsAlive())
        return false;

    // can't assist untargetable
    if ((!bySpell || !bySpell->HasAttribute(SPELL_ATTR6_CAN_TARGET_UNTARGETABLE)) && unitTarget && unitTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
        return false;

    // some negative spells can be casted at friendly target
    bool isNegativeSpell = bySpell && !bySpell->IsPositive();
    // check flags for negative spells
    if (isNegativeSpell && unitTarget && unitTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_TAXI_FLIGHT | UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NON_ATTACKABLE_2))
        return false;

    if (isNegativeSpell || !bySpell || !bySpell->HasAttribute(SPELL_ATTR6_ASSIST_IGNORE_IMMUNE_FLAG))
    {
        if (unit && unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        {
            if (unitTarget && unitTarget->IsImmuneToPC())
                return false;
        }
        else
        {
            if (unitTarget && unitTarget->IsImmuneToNPC())
                return false;
        }
    }

    // can't assist non-friendly targets
    if (GetReactionTo(target) < REP_NEUTRAL && target->GetReactionTo(this) < REP_NEUTRAL
#ifdef LICH_KING
        && (!ToCreature() || !(ToCreature()->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_TREAT_AS_RAID_UNIT))
#endif
        )
        return false;

    // PvP case
    if (unitTarget && unitTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
    {
        Player const* targetPlayerOwner = unitTarget->GetAffectingPlayer();
        if (unit && unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED))
        {
            Player const* selfPlayerOwner = GetAffectingPlayer();
            if (selfPlayerOwner && targetPlayerOwner)
            {
                // can't assist player which is dueling someone
                if (selfPlayerOwner != targetPlayerOwner
                    && targetPlayerOwner->duel)
                    return false;
            }
            // can't assist player in ffa_pvp zone from outside
            if (unitTarget->IsFFAPvP() && unit && !unit->IsFFAPvP())
                return false;

            // can't assist player out of sanctuary from sanctuary if has pvp enabled
            if (unitTarget->IsPvP())
                if (unit && unit->IsInSanctuary() && !unitTarget->IsInSanctuary())
                    return false;
        }
    }
    // PvC case - player can assist creature only if has specific type flags or if player is charmed by it
    // !target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED) &&
    else if (unit && unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED)
        && (!bySpell || !bySpell->HasAttribute(SPELL_ATTR6_ASSIST_IGNORE_IMMUNE_FLAG))
        && (unitTarget && !unitTarget->IsPvP())
        && (unit && unitTarget && unit->GetCharmerGUID() != unitTarget->GetGUID())
        )
    {
        if (Creature const* creatureTarget = target->ToCreature())
            return
#ifdef LICH_KING
            creatureTarget->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_TREAT_AS_RAID_UNIT ||
#endif
            creatureTarget->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_CAN_ASSIST;
    }

    return true;
}

Unit* WorldObject::GetMagicHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo)
{
    // Patch 1.2 notes: Spell Reflection no longer reflects abilities
    if (spellInfo->HasAttribute(SPELL_ATTR0_ABILITY) || spellInfo->HasAttribute(SPELL_ATTR1_CANT_BE_REDIRECTED) || spellInfo->HasAttribute(SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY))
        return victim;

    Unit::AuraEffectList const& magnetAuras = victim->GetAuraEffectsByType(SPELL_AURA_SPELL_MAGNET);
    for (auto magnetAura : magnetAuras)
    {
        if (Unit* magnet = magnetAura->GetBase()->GetUnitOwner())
            if (spellInfo->CheckExplicitTarget(this, magnet) == SPELL_CAST_OK
                && IsValidAttackTarget(magnet, spellInfo))
            {
                // sunwell: We should choose minimum between flight time and queue time as in reflect, however we dont know flight time at this point, use arbitrary small number
                magnet->m_Events.AddEvent(new RedirectSpellEvent(*magnet, victim->GetGUID(), magnetAura), magnet->m_Events.CalculateQueueTime(100));

                if (magnet->ToCreature() && magnet->ToCreature()->IsTotem())
                {
                    uint64 queueTime = magnet->m_Events.CalculateQueueTime(100);
                    if (spellInfo->Speed > 0.0f)
                    {
                        float dist = GetDistance(magnet->GetPositionX(), magnet->GetPositionY(), magnet->GetPositionZ());
                        dist = std::max<float>(dist, 5.0f);
                        uint32 delay = (uint64)floor(dist / spellInfo->Speed * 1000.0f);
                        queueTime = magnet->m_Events.CalculateTime(delay);
                        // Schedule charge drop
                        magnetAura->GetBase()->DropChargeDelayed(delay, AURA_REMOVE_BY_EXPIRE);
                    }
                    else
                        magnetAura->GetBase()->DropCharge(AURA_REMOVE_BY_EXPIRE);

                    magnet->m_Events.AddEvent(new KillMagnetEvent(*magnet), queueTime);
                }

                return magnet;
            }
    }
    return victim;
}

void WorldObject::SendSpellMiss(Unit *target, uint32 spellID, SpellMissInfo missInfo)
{
    WorldPacket data(SMSG_SPELLLOGMISS, (4 + 8 + 1 + 4 + 8 + 1)); //LK OK
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

ObjectGuid Object::GetGUID() const { return GetGuidValue(OBJECT_FIELD_GUID); }
PackedGuid const& Object::GetPackGUID() const { return m_PackGUID; }
uint32 Object::GetEntry() const { return GetUInt32Value(OBJECT_FIELD_ENTRY); }
void Object::SetEntry(uint32 entry) { SetUInt32Value(OBJECT_FIELD_ENTRY, entry); }

float Object::GetObjectScale() const { return GetFloatValue(OBJECT_FIELD_SCALE_X); }
void Object::SetObjectScale(float scale) { SetFloatValue(OBJECT_FIELD_SCALE_X, scale); }

template <typename Container>
void WorldObject::GetGameObjectListWithEntryInGrid(Container& gameObjectContainer, uint32 entry, float maxSearchRange /*= 250.0f*/) const
{
    Trinity::AllGameObjectsWithEntryInRange check(this, entry, maxSearchRange);
    Trinity::GameObjectListSearcher<Trinity::AllGameObjectsWithEntryInRange> searcher(this, gameObjectContainer, check);
    Cell::VisitGridObjects(this, searcher, maxSearchRange);
}

template <typename Container>
void WorldObject::GetCreatureListWithEntryInGrid(Container& creatureContainer, uint32 entry, float maxSearchRange /*= 250.0f*/) const
{
    Trinity::AllCreaturesOfEntryInRange check(this, entry, maxSearchRange);
    Trinity::CreatureListSearcher<Trinity::AllCreaturesOfEntryInRange> searcher(this, creatureContainer, check);
    Cell::VisitGridObjects(this, searcher, maxSearchRange);
}

template <typename Container>
void WorldObject::GetPlayerListInGrid(Container& playerContainer, float maxSearchRange) const
{
    Trinity::AnyPlayerInObjectRangeCheck checker(this, maxSearchRange);
    Trinity::PlayerListSearcher<Trinity::AnyPlayerInObjectRangeCheck> searcher(this, playerContainer, checker);
    Cell::VisitWorldObjects(this, searcher, maxSearchRange);
}

template TC_GAME_API void WorldObject::GetGameObjectListWithEntryInGrid(std::list<GameObject*>&, uint32, float) const;
template TC_GAME_API void WorldObject::GetGameObjectListWithEntryInGrid(std::deque<GameObject*>&, uint32, float) const;
template TC_GAME_API void WorldObject::GetGameObjectListWithEntryInGrid(std::vector<GameObject*>&, uint32, float) const;

template TC_GAME_API void WorldObject::GetCreatureListWithEntryInGrid(std::list<Creature*>&, uint32, float) const;
template TC_GAME_API void WorldObject::GetCreatureListWithEntryInGrid(std::deque<Creature*>&, uint32, float) const;
template TC_GAME_API void WorldObject::GetCreatureListWithEntryInGrid(std::vector<Creature*>&, uint32, float) const;

template TC_GAME_API void WorldObject::GetPlayerListInGrid(std::list<Player*>&, float) const;
template TC_GAME_API void WorldObject::GetPlayerListInGrid(std::deque<Player*>&, float) const;
template TC_GAME_API void WorldObject::GetPlayerListInGrid(std::vector<Player*>&, float) const;