
#include "Common.h"
#include "Corpse.h"
#include "Player.h"
#include "CharacterCache.h"
#include "UpdateMask.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Database/DatabaseEnv.h"
#include "Opcodes.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "GossipDef.h"
#include "World.h"
#ifdef TESTS
#include "TestPlayer.h"
#endif

Corpse::Corpse(CorpseType type) : WorldObject(type != CORPSE_BONES)
{
    m_objectType |= TYPEMASK_CORPSE;
    m_objectTypeId = TYPEID_CORPSE;
                                                            
#ifdef LICH_KING
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_STATIONARY_POSITION | UPDATEFLAG_POSITION);
#else
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_STATIONARY_POSITION); // 2.3.2 - 0x58
#endif

    m_valuesCount = CORPSE_END;

    m_type = type;

    m_time = time(nullptr);

    lootRecipient = nullptr;
}

Corpse::~Corpse()
{
}

void Corpse::AddToWorld()
{
    ///- Register the corpse for guid lookup
    if(!IsInWorld()) 
        GetMap()->GetObjectsStore().Insert<Corpse>(GetGUID(), this);

    Object::AddToWorld();
}

void Corpse::RemoveFromWorld()
{
    ///- Remove the corpse from the accessor
    if(IsInWorld()) 
        GetMap()->GetObjectsStore().Remove<Corpse>(GetGUID());

    Object::RemoveFromWorld();
}

bool Corpse::Create(ObjectGuid::LowType guidlow)
{
    Object::_Create(guidlow, 0, HighGuid::Corpse);
    return true;
}

bool Corpse::Create(ObjectGuid::LowType guidlow, Player *owner)
{
    WorldObject::_Create(guidlow, HighGuid::Corpse, owner->GetPhaseMask());

    Relocate(owner->GetPositionX(), owner->GetPositionY(), owner->GetPositionZ(), owner->GetOrientation());

    if(!IsPositionValid())
    {
        TC_LOG_ERROR("FIXME","ERROR: Corpse (guidlow %d, owner %s) not created. Suggested coordinates isn't valid (X: %f Y: %f)",
            guidlow,owner->GetName().c_str(), owner->GetPositionX(), owner->GetPositionY());
        return false;
    }

    SetObjectScale(1.0f);
    SetFloatValue( CORPSE_FIELD_POS_X, owner->GetPositionX());
    SetFloatValue( CORPSE_FIELD_POS_Y, owner->GetPositionY());
    SetFloatValue( CORPSE_FIELD_POS_Z, owner->GetPositionZ());
    SetFloatValue( CORPSE_FIELD_FACING, owner->GetOrientation());
    SetUInt64Value( CORPSE_FIELD_OWNER, owner->GetGUID() );

    _cellCoord = Trinity::ComputeCellCoord(GetPositionX(), GetPositionY());

#ifdef TESTS
    if (dynamic_cast<TestPlayer*>(owner))
        _noDatabaseSave = true;
#endif
    return true;
}

void Corpse::SaveToDB()
{
    if (_noDatabaseSave)
        return;

    // prevent DB data inconsistance problems and duplicates
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    DeleteFromDB(trans);

    uint16 index = 0;
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CORPSE);
    stmt->setUInt32(index++, GetOwnerGUID().GetCounter());                            // guid
    stmt->setFloat(index++, GetPositionX());                                         // posX
    stmt->setFloat(index++, GetPositionY());                                         // posY
    stmt->setFloat(index++, GetPositionZ());                                         // posZ
    stmt->setFloat(index++, GetOrientation());                                       // orientation
    stmt->setUInt16(index++, GetMapId());                                             // mapId
    stmt->setUInt32(index++, GetUInt32Value(CORPSE_FIELD_DISPLAY_ID));                // displayId
    stmt->setString(index++, _ConcatFields(CORPSE_FIELD_ITEM, EQUIPMENT_SLOT_END));   // itemCache
    stmt->setUInt32(index++, GetUInt32Value(CORPSE_FIELD_BYTES_1));                   // bytes1
    stmt->setUInt32(index++, GetUInt32Value(CORPSE_FIELD_BYTES_2));                   // bytes2
    stmt->setUInt32(index++, GetUInt32Value(CORPSE_FIELD_GUILD));                     // guildId
    stmt->setUInt8(index++, GetUInt32Value(CORPSE_FIELD_FLAGS));                     // flags
    stmt->setUInt8(index++, GetUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS));             // dynFlags
    stmt->setUInt32(index++, uint32(m_time));                                         // time
    stmt->setUInt8(index++, GetType());                                              // corpseType
    stmt->setUInt32(index++, GetInstanceId());                                        // instanceId
    stmt->setUInt32(index++, GetPhaseMask());                                         // phaseMask
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

ObjectGuid Corpse::GetOwnerGUID() const { return GetGuidValue(CORPSE_FIELD_OWNER); }

void Corpse::DeleteFromDB(SQLTransaction& trans)
{
    DeleteFromDB(GetOwnerGUID(), trans);
}

void Corpse::DeleteFromDB(ObjectGuid const& ownerGuid, SQLTransaction& trans)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CORPSE);
    stmt->setUInt32(0, ownerGuid.GetCounter());
    CharacterDatabase.ExecuteOrAppend(trans, stmt);
}

uint32 Corpse::GetFaction() const
{
    // inherit faction from player race
    uint32 const race = GetByteValue(CORPSE_FIELD_BYTES_1, 1);

    ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
    return rEntry ? rEntry->FactionID : 0;
}

bool Corpse::LoadCorpseFromDB(ObjectGuid::LowType guid, Field* fields)
{
    //        0     1     2     3            4      5          6          7       8       9        10     11        12    13          14          15         16
    // SELECT posX, posY, posZ, orientation, mapId, displayId, itemCache, bytes1, bytes2, guildId, flags, dynFlags, time, corpseType, instanceId, phaseMask, guid FROM corpse WHERE mapId = ? AND instanceId = ?


    ObjectGuid::LowType ownerGuid = fields[16].GetUInt32();
    float posX = fields[0].GetFloat();
    float posY = fields[1].GetFloat();
    float posZ = fields[2].GetFloat();
    float o = fields[3].GetFloat();
    uint32 mapId = fields[4].GetUInt16();

    Object::_Create(guid, 0, HighGuid::Corpse);

    SetObjectScale(1.0f);
    SetUInt32Value(CORPSE_FIELD_DISPLAY_ID, fields[5].GetUInt32());
    _LoadIntoDataField(fields[6].GetString(), CORPSE_FIELD_ITEM, EQUIPMENT_SLOT_END);
    SetUInt32Value(CORPSE_FIELD_BYTES_1, fields[7].GetUInt32());
    SetUInt32Value(CORPSE_FIELD_BYTES_2, fields[8].GetUInt32());
    SetUInt32Value(CORPSE_FIELD_GUILD, fields[9].GetUInt32());
    SetUInt32Value(CORPSE_FIELD_FLAGS, fields[10].GetUInt8());
    SetUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS, fields[11].GetUInt8());
    SetGuidValue(CORPSE_FIELD_OWNER, ObjectGuid(HighGuid::Player, ownerGuid));

    m_time = time_t(fields[12].GetUInt32());

    uint32 instanceId = fields[14].GetUInt32();
    uint32 phaseMask = fields[15].GetUInt32();

    // place
    SetLocationInstanceId(instanceId);
    SetLocationMapId(mapId);
    SetPhaseMask(phaseMask, false);
    Relocate(posX, posY, posZ, o);

    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.player", "Corpse (%s, owner: %s) is not created, given coordinates are not valid (X: %f, Y: %f, Z: %f)",
            ObjectGuid(GetGUID()).ToString().c_str(), ObjectGuid(GetOwnerGUID()).ToString().c_str(), posX, posY, posZ);
        return false;
    }

    _cellCoord = Trinity::ComputeCellCoord(GetPositionX(), GetPositionY());
    return true;
}

bool Corpse::IsExpired(time_t t) const
{
    // Deleted character
    if (!sCharacterCache->HasCharacterCacheEntry(GetOwnerGUID()))
        return true;

    if (m_type == CORPSE_BONES)
        return m_time < t - 60 * MINUTE;
    else
        return m_time < t - 3 * DAY;
}
