
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
	_isViewpoint(false), m_updateTimer(0), m_effIndex(0), m_aliveDuration(0), _duration(0), m_type(DYNAMIC_OBJECT_AREA_SPELL)
{

    m_objectType |= TYPEMASK_DYNAMICOBJECT;
    m_objectTypeId = TYPEID_DYNAMICOBJECT;
                                                            // 2.3.2 - 0x58
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_STATIONARY_POSITION);
    m_updateFlagLK = (LK_UPDATEFLAG_LOWGUID | LK_UPDATEFLAG_STATIONARY_POSITION | LK_UPDATEFLAG_POSITION);

    m_valuesCount = DYNAMICOBJECT_END;
}

DynamicObject::~DynamicObject()
{
	// make sure all references were properly removed
	ASSERT(!_isViewpoint);
}


void DynamicObject::AddToWorld()
{
    ///- Register the dynamicObject for guid lookup
    if(!IsInWorld())
    {
		GetMap()->GetObjectsStore().Insert<DynamicObject>(GetGUID(), this);
        WorldObject::AddToWorld();
    }
}

void DynamicObject::RemoveFromWorld()
{
    ///- Remove the dynamicObject from the accessor
    if(IsInWorld())
    {
		if (_isViewpoint)
			RemoveCasterViewpoint();

		// dynobj could get removed in Aura::RemoveAura
		if (!IsInWorld())
			return;

		WorldObject::RemoveFromWorld();
		GetMap()->GetObjectsStore().Remove<DynamicObject>(GetGUID());
        if(GetTransport())
            GetTransport()->RemovePassenger(this);
    }
}

bool DynamicObject::Create( uint32 guidlow, Unit *caster, uint32 spellId, uint32 effIndex, Position const& pos, int32 duration, float radius, DynamicObjectType type)
{
	SetMap(caster->GetMap());
    Relocate(pos);
    if(!IsPositionValid())
    {
        TC_LOG_ERROR("misc","ERROR: DynamicObject (spell %u eff %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)",spellId,GetPositionX(),GetPositionY());
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

	m_aliveDuration = duration;
	m_effIndex = effIndex;
	m_updateTimer = 0;
	m_type = type;

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

#ifdef LICH_KING
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
#endif

	if (!GetMap()->AddToMap(this))
	{
		// Returning false will cause the object to be deleted - remove from transport
#ifdef LICH_KING
		if (transport)
			transport->RemovePassenger(this);
#endif
		return false;
	}

    return true;
}

Unit* DynamicObject::GetCaster() const
{
	// can be not found in some cases
	return ObjectAccessor::GetUnit(*this, GetCasterGUID());
}

void DynamicObject::Update(uint32 p_time)
{
	// caster has to be always available and in the same map
	Unit* caster = GetCaster();
	if (!caster)
	{
		Remove();
		return;
	}

	bool expired = false;

	if (m_aliveDuration > int32(p_time))
		m_aliveDuration -= p_time;
	else
		expired = true;

	if (m_type != DYNAMIC_OBJECT_FARSIGHT_FOCUS)
	{
		if (m_updateTimer < p_time)
		{
			Trinity::DynamicObjectUpdater notifier(*this, caster);
            Cell::VisitAllObjects(this, notifier, GetRadius());
			m_updateTimer = 500; // is this blizzlike?
		}
		else m_updateTimer -= p_time;
	}

	if (expired) {
		caster->RemoveDynObjectWithGUID(GetGUID());
		Remove();
	}
	/* TC
	else
		sScriptMgr->OnDynamicObjectUpdate(this, p_time);
		*/
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

void DynamicObject::AddAffected(Unit *unit)
{
    m_affected.insert(unit);

    // Hacky way
    switch (GetSpellId())
    {
        case 45848:    // Shield of the Blue
            unit->RemoveAurasDueToSpell(45641);
            unit->RemoveAurasDueToSpell(45737);
            break;
    }
}

void DynamicObject::Delay(int32 delaytime)
{
	m_aliveDuration -= delaytime;
	for (auto iunit : m_affected)
		if (iunit)
			iunit->DelayAura(GetSpellId(), m_effIndex, delaytime);
}

void DynamicObject::SetCasterViewpoint()
{
	Unit* caster = GetCaster();
	if (caster && caster->GetTypeId() == TYPEID_PLAYER)
	{
		caster->ToPlayer()->SetViewpoint(this, true);
		_isViewpoint = true;
	}
}

void DynamicObject::RemoveCasterViewpoint()
{
	Unit* caster = GetCaster();
	if(caster && caster->GetTypeId() == TYPEID_PLAYER)
	{
		caster->ToPlayer()->SetViewpoint(this, false);
		_isViewpoint = false;
	}
}
