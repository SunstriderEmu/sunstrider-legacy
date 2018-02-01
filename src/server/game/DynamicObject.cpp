
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
#include "GameTime.h"
#include "DynamicObject.h"

DynamicObject::DynamicObject(bool isWorldObject) : WorldObject(isWorldObject),
    _isViewpoint(false), _duration(0), _caster(nullptr), _aura(nullptr), _removedAura(nullptr)
{

    m_objectType |= TYPEMASK_DYNAMICOBJECT;
    m_objectTypeId = TYPEID_DYNAMICOBJECT;
                                                            
#ifdef LICH_KING
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_STATIONARY_POSITION | UPDATEFLAG_POSITION);
#else
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_STATIONARY_POSITION); // 2.3.2 - 0x58
#endif

    m_valuesCount = DYNAMICOBJECT_END;
}

DynamicObject::~DynamicObject()
{
    // make sure all references were properly removed
    ASSERT(!_aura);
    ASSERT(!_caster);
    ASSERT(!_isViewpoint);
    ASSERT(!_isViewpoint);
}


void DynamicObject::AddToWorld()
{
    ///- Register the dynamicObject for guid lookup
    if(!IsInWorld())
    {
        GetMap()->GetObjectsStore().Insert<DynamicObject>(GetGUID(), this);
        WorldObject::AddToWorld();
        BindToCaster();
    }
}

void DynamicObject::RemoveFromWorld()
{
    ///- Remove the dynamicObject from the accessor
    if(IsInWorld())
    {
        if (_isViewpoint)
            RemoveCasterViewpoint();

        if (_aura)
            RemoveAura();

        // dynobj could get removed in Aura::RemoveAura
        if (!IsInWorld())
            return;

        UnbindFromCaster();
        WorldObject::RemoveFromWorld();
        GetMap()->GetObjectsStore().Remove<DynamicObject>(GetGUID());
        if(GetTransport())
            GetTransport()->RemovePassenger(this);
    }
}

bool DynamicObject::CreateDynamicObject(ObjectGuid::LowType guidlow, Unit *caster, uint32 spellId, Position const& pos, float radius, DynamicObjectType type)
{
    SetMap(caster->GetMap());
    Relocate(pos);
    if(!IsPositionValid())
    {
        TC_LOG_ERROR("misc","ERROR: DynamicObject (spell %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)", spellId, GetPositionX(), GetPositionY());
        return false;
    }

    WorldObject::_Create(guidlow, HighGuid::DynamicObject, caster->GetPhaseMask());
    /*
    float visualRadius = radius;
    // For some reason visual size in client seems incorrect for some spells. Can't seem to find the proper rule.
    if(SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId))
    {
        if (spellInfo->GetMaxRange() == 0 || spellInfo->HasVisual(10383))  //Personal range. Ice trap, consecration... + (Flamestrike : 10383)
            visualRadius = radius * 2.2;
        
        else if (spellInfo->Id == 45848)
            visualRadius = radius * 0.4;
    }
    */

    SetEntry(spellId);
    SetObjectScale(1.0f);
    SetUInt64Value( DYNAMICOBJECT_CASTER, caster->GetGUID() );

    // The lower word of DYNAMICOBJECT_BYTES must be 0x0001. This value means that the visual radius will be overriden
    // by client for most of the "ground patch" visual effect spells and a few "skyfall" ones like Hurricane.
    // If any other value is used, the client will _always_ use the radius provided in DYNAMICOBJECT_RADIUS, but
    // precompensation is necessary (eg radius *= 2) for many spells. Anyway, blizz sends 0x0001 for all the spells
    // I saw sniffed...
    SetByteValue(DYNAMICOBJECT_BYTES, 0, type);
    SetUInt32Value( DYNAMICOBJECT_SPELLID, spellId );
    SetFloatValue( DYNAMICOBJECT_RADIUS, radius);
    SetFloatValue( DYNAMICOBJECT_POS_X, pos.GetPositionX() );
    SetFloatValue( DYNAMICOBJECT_POS_Y, pos.GetPositionY() );
    SetFloatValue( DYNAMICOBJECT_POS_Z, pos.GetPositionZ() );
    SetUInt32Value( DYNAMICOBJECT_CASTTIME, GameTime::GetGameTimeMS()); 

    if (IsWorldObject())
        SetKeepActive(true);    //must before add to map to be put in world container

    Transport* transport = caster->GetTransport();
    if (transport)
    {
        float x, y, z, o;
        pos.GetPosition(x, y, z, o);
        transport->CalculatePassengerOffset(x, y, z, &o);
        m_movementInfo.transport.pos.Relocate(x, y, z, o);

        // This object must be added to transport before adding to map for the client to properly display it
        transport->AddPassenger(this);
    }

    if (!GetMap()->AddToMap(this))
    {
        // Returning false will cause the object to be deleted - remove from transport
        if (transport)
            transport->RemovePassenger(this);

        return false;
    }

    return true;
}

void DynamicObject::Update(uint32 p_time)
{
    // caster has to be always available and in the same map
    ASSERT(_caster);
    ASSERT(_caster->GetMap() == GetMap());

    bool expired = false;

    if (_aura)
    {
        if (!_aura->IsRemoved())
            _aura->UpdateOwner(p_time, this);

        // _aura may be set to null in Aura::UpdateOwner call
        if (_aura && (_aura->IsRemoved() || _aura->IsExpired()))
            expired = true;
    }
    else
    {
        if (GetDuration() > int32(p_time))
            _duration -= p_time;
        else
            expired = true;
    }

    if (expired)
        Remove();
    /* TC else
    sScriptMgr->OnDynamicObjectUpdate(this, p_time);*/
}

void DynamicObject::Remove()
{
    if (IsInWorld())
    {
        SendObjectDeSpawnAnim(GetGUID());
        RemoveFromWorld();
        AddObjectToRemoveList();
    }
}

int32 DynamicObject::GetDuration() const
{
    if (!_aura)
        return _duration;
    else
        return _aura->GetDuration();
}

void DynamicObject::SetDuration(int32 newDuration)
{
    if (!_aura)
        _duration = newDuration;
    else
        _aura->SetDuration(newDuration);
}

void DynamicObject::Delay(int32 delaytime)
{
    SetDuration(GetDuration() - delaytime);
}

void DynamicObject::SetAura(Aura* aura)
{
    ASSERT(!_aura && aura);
    _aura = aura;
}

void DynamicObject::RemoveAura()
{
    ASSERT(_aura && !_removedAura);
    _removedAura = _aura;
    _aura = nullptr;
    if (!_removedAura->IsRemoved())
        _removedAura->_Remove(AURA_REMOVE_BY_DEFAULT);
}

void DynamicObject::SetCasterViewpoint()
{
    if (Player* caster = _caster->ToPlayer())
    {
        caster->ToPlayer()->SetViewpoint(this, true);
        _isViewpoint = true;
    }
}

void DynamicObject::RemoveCasterViewpoint()
{
    if (Player* caster = _caster->ToPlayer())
    {
        caster->ToPlayer()->SetViewpoint(this, false);
        _isViewpoint = false;
    }
}

void DynamicObject::BindToCaster()
{
    ASSERT(!_caster);
    _caster = ObjectAccessor::GetUnit(*this, GetCasterGUID());
    ASSERT(_caster);
    ASSERT(_caster->GetMap() == GetMap());
    _caster->_RegisterDynObject(this);
}

void DynamicObject::UnbindFromCaster()
{
    ASSERT(_caster);
    _caster->_UnregisterDynObject(this);
    _caster = nullptr;
}

SpellInfo const* DynamicObject::GetSpellInfo() const
{
    return sSpellMgr->GetSpellInfo(GetSpellId());
}
