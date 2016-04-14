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
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Opcodes.h"
#include "Log.h"
#include "UpdateMask.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Pet.h"
#include "Unit.h"
#include "Totem.h"
#include "Spell.h"
#include "DynamicObject.h"
#include "SpellAuras.h"
#include "Group.h"
#include "UpdateData.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "CellImpl.h"
#include "SharedDefines.h"
#include "LootMgr.h"
#include "Management/VMapFactory.h"
#include "BattleGround.h"
#include "Util.h"
#include "TemporarySummon.h"
#include "ScriptMgr.h"
#include "Containers.h"

#define SPELL_CHANNEL_UPDATE_INTERVAL 1000

extern pEffect SpellEffects[TOTAL_SPELL_EFFECTS];

bool IsQuestTameSpell(uint32 spellId)
{
    SpellInfo const *spellproto = sSpellMgr->GetSpellInfo(spellId);
    if (!spellproto) return false;

    return spellproto->Effects[0].Effect == SPELL_EFFECT_THREAT
        && spellproto->Effects[1].Effect == SPELL_EFFECT_APPLY_AURA && spellproto->Effects[1].ApplyAuraName == SPELL_AURA_DUMMY;
}


SpellDestination::SpellDestination()
{
    _position.Relocate(0, 0, 0, 0);
#ifdef LICH_KING
    _transportGUID = 0;
    _transportOffset.Relocate(0, 0, 0, 0);
#endif
}

SpellDestination::SpellDestination(float x, float y, float z, float orientation, uint32 mapId)
{
    _position.Relocate(x, y, z, orientation);
    _position.m_mapId = mapId;
#ifdef LICH_KING
    _transportGUID = 0;
    _transportOffset.Relocate(0, 0, 0, 0);
#endif
}

SpellDestination::SpellDestination(Position const& pos)
{
    _position.Relocate(pos);
#ifdef LICH_KING
    _transportGUID = 0;
    _transportOffset.Relocate(0, 0, 0, 0);
#endif
}

SpellDestination::SpellDestination(WorldObject const& wObj)
{
#ifdef LICH_KING
    _transportGUID = wObj.GetTransGUID();
    _transportOffset.Relocate(wObj.GetTransOffsetX(), wObj.GetTransOffsetY(), wObj.GetTransOffsetZ(), wObj.GetTransOffsetO());
#endif
    _position.Relocate(wObj);
}

void SpellDestination::Relocate(Position const& pos)
{
#ifdef LICH_KING
    if (_transportGUID)
    {
        Position offset;
        _position.GetPositionOffsetTo(pos, offset);
        _transportOffset.RelocateOffset(offset);
    }
#endif LICH_KING
    _position.Relocate(pos);
}

void SpellDestination::RelocateOffset(Position const& offset)
{
#ifdef LICH_KING
    if (_transportGUID)
        _transportOffset.RelocateOffset(offset);
#endif
    _position.RelocateOffset(offset);
}

SpellCastTargets::SpellCastTargets() : m_elevation(0), m_speed(0), m_strTarget()
{
    m_objectTarget = NULL;
    m_itemTarget = NULL;

    m_objectTargetGUID = 0;
    m_itemTargetGUID = 0;
    m_itemTargetEntry = 0;

    m_targetMask = 0;

    // Xinef: Channel data
    m_objectTargetGUIDChannel = 0;
}

SpellCastTargets::~SpellCastTargets()
{
}

void SpellCastTargets::Read(ByteBuffer& data, Unit* caster)
{
    data >> m_targetMask;

    if (m_targetMask == TARGET_FLAG_NONE)
        return;

    //BC has corpse at the end
#ifdef LICH_KING
    if (m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_UNIT_MINIPET | TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_CORPSE_ENEMY | TARGET_FLAG_CORPSE_ALLY))
#else
    if (m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_UNIT_MINIPET | TARGET_FLAG_GAMEOBJECT))
#endif
        data.readPackGUID(m_objectTargetGUID);

    if (m_targetMask & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM))
        data.readPackGUID(m_itemTargetGUID);

    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
    {
#ifdef LICH_KING
        data.readPackGUID(m_src._transportGUID);
        if (m_src._transportGUID)
            data >> m_src._transportOffset.PositionXYZStream();
        else
#endif
            data >> m_src._position.PositionXYZStream();
    }
    else
    {
#ifdef LICH_KING
        m_src._transportGUID = caster->GetTransGUID();
        if (m_src._transportGUID)
            m_src._transportOffset.Relocate(caster->GetTransOffsetX(), caster->GetTransOffsetY(), caster->GetTransOffsetZ(), caster->GetTransOffsetO());
        else
#endif
            m_src._position.Relocate(caster);
    }

    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
#ifdef LICH_KING
        data.readPackGUID(m_dst._transportGUID);
        if (m_dst._transportGUID)
            data >> m_dst._transportOffset.PositionXYZStream();
        else
#endif
            data >> m_dst._position.PositionXYZStream();
    }
    else
    {
#ifdef LICH_KING
        m_dst._transportGUID = caster->GetTransGUID();
        if (m_dst._transportGUID)
            m_dst._transportOffset.Relocate(caster->GetTransOffsetX(), caster->GetTransOffsetY(), caster->GetTransOffsetZ(), caster->GetTransOffsetO());
        else
#endif
            m_dst._position.Relocate(caster);
    }

    if (m_targetMask & TARGET_FLAG_STRING)
        data >> m_strTarget;

    //LK has corpse at packet begin with other objects types
#ifndef LICH_KING
    if (m_targetMask & (TARGET_FLAG_CORPSE_ENEMY | TARGET_FLAG_CORPSE_ALLY))
#endif
        data.readPackGUID(m_objectTargetGUID);


    Update(caster);
}

void SpellCastTargets::Write(ByteBuffer& data)
{
    data << uint32(m_targetMask);

    if (m_targetMask & (TARGET_FLAG_UNIT | TARGET_FLAG_CORPSE_ALLY | TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_CORPSE_ENEMY | TARGET_FLAG_UNIT_MINIPET))
        data.appendPackGUID(m_objectTargetGUID);

    if (m_targetMask & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM))
    {
        if (m_itemTarget)
            data << m_itemTarget->GetPackGUID();
        else
            data << uint8(0);
    }

    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
    {
#ifdef LICH_KING
        data.appendPackGUID(m_src._transportGUID); // relative position guid here - transport for example
        if (m_src._transportGUID)
            data << m_src._transportOffset.PositionXYZStream();
        else
#endif
            data << m_src._position.PositionXYZStream();
    }

    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
#ifdef LICH_KING
        data.appendPackGUID(m_dst._transportGUID); // relative position guid here - transport for example
        if (m_dst._transportGUID)
            data << m_dst._transportOffset.PositionXYZStream();
        else
#endif
            data << m_dst._position.PositionXYZStream();
    }

    if (m_targetMask & TARGET_FLAG_STRING)
        data << m_strTarget;
}

uint64 SpellCastTargets::GetUnitTargetGUID() const
{
    switch (GUID_HIPART(m_objectTargetGUID))
    {
    case HIGHGUID_PLAYER:
    case HIGHGUID_VEHICLE:
    case HIGHGUID_UNIT:
    case HIGHGUID_PET:
        return m_objectTargetGUID;
    default:
        return 0LL;
    }
}

Unit* SpellCastTargets::GetUnitTarget() const
{
    if (m_objectTarget)
        return m_objectTarget->ToUnit();
    return NULL;
}

void SpellCastTargets::SetUnitTarget(Unit* target)
{
    if (!target)
        return;

    m_objectTarget = target;
    m_objectTargetGUID = target->GetGUID();
    m_targetMask |= TARGET_FLAG_UNIT;
}

uint64 SpellCastTargets::GetGOTargetGUID() const
{
    switch (GUID_HIPART(m_objectTargetGUID))
    {
    case HIGHGUID_TRANSPORT:
    case HIGHGUID_MO_TRANSPORT:
    case HIGHGUID_GAMEOBJECT:
        return m_objectTargetGUID;
    default:
        return 0LL;
    }
}

GameObject* SpellCastTargets::GetGOTarget() const
{
    if (m_objectTarget)
        return m_objectTarget->ToGameObject();
    return NULL;
}


void SpellCastTargets::SetGOTarget(GameObject* target)
{
    if (!target)
        return;

    m_objectTarget = target;
    m_objectTargetGUID = target->GetGUID();
    m_targetMask |= TARGET_FLAG_GAMEOBJECT;
}

uint64 SpellCastTargets::GetCorpseTargetGUID() const
{
    switch (GUID_HIPART(m_objectTargetGUID))
    {
    case HIGHGUID_CORPSE:
        return m_objectTargetGUID;
    default:
        return 0LL;
    }
}

Corpse* SpellCastTargets::GetCorpseTarget() const
{
    if (m_objectTarget)
        return m_objectTarget->ToCorpse();
    return NULL;
}

WorldObject* SpellCastTargets::GetObjectTarget() const
{
    return m_objectTarget;
}

uint64 SpellCastTargets::GetObjectTargetGUID() const
{
    return m_objectTargetGUID;
}

void SpellCastTargets::RemoveObjectTarget()
{
    m_objectTarget = NULL;
    m_objectTargetGUID = 0LL;
    m_targetMask &= ~(TARGET_FLAG_UNIT_MASK | TARGET_FLAG_CORPSE_MASK | TARGET_FLAG_GAMEOBJECT_MASK);
}

void SpellCastTargets::SetItemTarget(Item* item)
{
    if (!item)
        return;

    m_itemTarget = item;
    m_itemTargetGUID = item->GetGUID();
    m_itemTargetEntry = item->GetEntry();
    m_targetMask |= TARGET_FLAG_ITEM;
}

void SpellCastTargets::SetTradeItemTarget(Player* caster)
{
    m_itemTargetGUID = uint64(TRADE_SLOT_NONTRADED);
    m_itemTargetEntry = 0;
    m_targetMask |= TARGET_FLAG_TRADE_ITEM;

    Update(caster);
}

void SpellCastTargets::UpdateTradeSlotItem()
{
    if (m_itemTarget && (m_targetMask & TARGET_FLAG_TRADE_ITEM))
    {
        m_itemTargetGUID = m_itemTarget->GetGUID();
        m_itemTargetEntry = m_itemTarget->GetEntry();
    }
}

SpellDestination const* SpellCastTargets::GetSrc() const
{
    return &m_src;
}

Position const* SpellCastTargets::GetSrcPos() const
{
    return &m_src._position;
}

void SpellCastTargets::SetSrc(float x, float y, float z)
{
    m_src = SpellDestination(x, y, z);
    m_targetMask |= TARGET_FLAG_SOURCE_LOCATION;
}

void SpellCastTargets::SetSrc(Position const& pos)
{
    m_src = SpellDestination(pos);
    m_targetMask |= TARGET_FLAG_SOURCE_LOCATION;
}

void SpellCastTargets::SetSrc(WorldObject const& wObj)
{
    m_src = SpellDestination(wObj);
    m_targetMask |= TARGET_FLAG_SOURCE_LOCATION;
}

void SpellCastTargets::ModSrc(Position const& pos)
{
    ASSERT(m_targetMask & TARGET_FLAG_SOURCE_LOCATION);
    m_src.Relocate(pos);
}

void SpellCastTargets::RemoveSrc()
{
    m_targetMask &= ~(TARGET_FLAG_SOURCE_LOCATION);
}

SpellDestination const* SpellCastTargets::GetDst() const
{
    return &m_dst;
}

WorldLocation const* SpellCastTargets::GetDstPos() const
{
    return &m_dst._position;
}

void SpellCastTargets::SetDst(float x, float y, float z, float orientation, uint32 mapId)
{
    m_dst = SpellDestination(x, y, z, orientation, mapId);
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
}

void SpellCastTargets::SetDst(Position const& pos)
{
    m_dst = SpellDestination(pos);
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
}

void SpellCastTargets::SetDst(WorldObject const& wObj)
{
    m_dst = SpellDestination(wObj);
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
}

void SpellCastTargets::SetDst(SpellDestination const& spellDest)
{
    m_dst = spellDest;
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
}

void SpellCastTargets::SetDst(SpellCastTargets const& spellTargets)
{
    m_dst = spellTargets.m_dst;
    m_targetMask |= TARGET_FLAG_DEST_LOCATION;
}

void SpellCastTargets::ModDst(Position const& pos)
{
    ASSERT(m_targetMask & TARGET_FLAG_DEST_LOCATION);
    m_dst.Relocate(pos);
}

void SpellCastTargets::ModDst(SpellDestination const& spellDest)
{
    ASSERT(m_targetMask & TARGET_FLAG_DEST_LOCATION);
    m_dst = spellDest;
}

void SpellCastTargets::RemoveDst()
{
    m_targetMask &= ~(TARGET_FLAG_DEST_LOCATION);
}

// Xinef: Channel Data
void SpellCastTargets::SetObjectTargetChannel(uint64 targetGUID)
{
    m_objectTargetGUIDChannel = targetGUID;
}

void SpellCastTargets::SetDstChannel(SpellDestination const& spellDest)
{
    m_dstChannel = spellDest;
}

WorldObject* SpellCastTargets::GetObjectTargetChannel(Unit* caster) const
{
    return m_objectTargetGUIDChannel ? ((m_objectTargetGUIDChannel == caster->GetGUID()) ? caster : ObjectAccessor::GetObjectInWorld(m_objectTargetGUIDChannel, caster)) : NULL;
}

bool SpellCastTargets::HasDstChannel() const
{
    return m_dstChannel._position.GetExactDist(0, 0, 0) > 0.001f;
}

SpellDestination const* SpellCastTargets::GetDstChannel() const
{
    return &m_dstChannel;
}

void SpellCastTargets::Update(Unit* caster)
{
    m_objectTarget = m_objectTargetGUID ? ((m_objectTargetGUID == caster->GetGUID()) ? caster : ObjectAccessor::GetObjectInWorld(m_objectTargetGUID, caster)) : NULL;

    m_itemTarget = NULL;
    if (caster->GetTypeId() == TYPEID_PLAYER)
    {
        Player* player = caster->ToPlayer();
        if (m_targetMask & TARGET_FLAG_ITEM)
            m_itemTarget = player->GetItemByGuid(m_itemTargetGUID);
        else if (m_targetMask & TARGET_FLAG_TRADE_ITEM)
        {
            /* Sunwell core (kept here if we use TradeData one day. Old code instead below.
            if (m_itemTargetGUID == TRADE_SLOT_NONTRADED) // here it is not guid but slot. Also prevents hacking slots
                if (TradeData* pTrade = player->GetTradeData())
                    m_itemTarget = pTrade->GetTraderData()->GetItem(TRADE_SLOT_NONTRADED);
            */

            // here it is not guid but slot
            Player* pTrader = (caster->ToPlayer())->GetTrader();
            if (pTrader && m_itemTargetGUID < TRADE_SLOT_COUNT)
                m_itemTarget = pTrader->GetItemByPos(pTrader->GetItemPosByTradeSlot(m_itemTargetGUID));
        }

        if (m_itemTarget)
            m_itemTargetEntry = m_itemTarget->GetEntry();
    }

#ifdef LICH_KING
    // update positions by transport move
    if (HasSrc() && m_src._transportGUID)
    {
        if (WorldObject* transport = ObjectAccessor::GetWorldObject(*caster, m_src._transportGUID))
        {
            m_src._position.Relocate(transport);
            m_src._position.RelocateOffset(m_src._transportOffset);
        }
    }

    if (HasDst() && m_dst._transportGUID)
    {
        if (WorldObject* transport = ObjectAccessor::GetWorldObject(*caster, m_dst._transportGUID))
        {
            m_dst._position.Relocate(transport);
            m_dst._position.RelocateOffset(m_dst._transportOffset);
        }
    }
#endif
}

void SpellCastTargets::OutDebug() const
{
    if (!m_targetMask)
        TC_LOG_DEBUG("spells", "No targets");

    TC_LOG_DEBUG("spells", "target mask: %u", m_targetMask);
    if (m_targetMask & (TARGET_FLAG_UNIT_MASK | TARGET_FLAG_CORPSE_MASK | TARGET_FLAG_GAMEOBJECT_MASK))
        TC_LOG_DEBUG("spells", "Object target: " UI64FMTD, m_objectTargetGUID);
    if (m_targetMask & TARGET_FLAG_ITEM)
        TC_LOG_DEBUG("spells", "Item target: " UI64FMTD, m_itemTargetGUID);
    if (m_targetMask & TARGET_FLAG_TRADE_ITEM)
        TC_LOG_DEBUG("spells", "Trade item target: " UI64FMTD, m_itemTargetGUID);
#ifdef LICH_KING
    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
        TC_LOG_DEBUG("spells", "Source location: transport guid: " UI64FMTD " trans offset: %s position: %s", m_src._transportGUID, m_src._transportOffset.ToString().c_str(), m_src._position.ToString().c_str());
    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
        TC_LOG_DEBUG("spells", "Destination location: transport guid: "  UI64FMTD " trans offset: %s position: %s", m_dst._transportGUID, m_dst._transportOffset.ToString().c_str(), m_dst._position.ToString().c_str());
#else
    if (m_targetMask & TARGET_FLAG_SOURCE_LOCATION)
        TC_LOG_DEBUG("spells", "Source location: position: %s", m_src._position.ToString().c_str());
    if (m_targetMask & TARGET_FLAG_DEST_LOCATION)
        TC_LOG_DEBUG("spells", "Destination location: position: %s", m_dst._position.ToString().c_str());
#endif
    if (m_targetMask & TARGET_FLAG_STRING)
        TC_LOG_DEBUG("spells", "String: %s", m_strTarget.c_str());
    TC_LOG_DEBUG("spells", "speed: %f", m_speed);
    TC_LOG_DEBUG("spells", "elevation: %f", m_elevation);
}

Spell::Spell(Unit* Caster, SpellInfo const *info, bool triggered, uint64 originalCasterGUID, Spell** triggeringContainer, bool skipCheck) :
    m_spellInfo(info), 
    m_spellValue(new SpellValue(m_spellInfo)),
    m_caster(Caster),
    m_preGeneratedPath(nullptr)
{
    m_skipHitCheck = skipCheck;
    m_selfContainer = NULL;
    m_triggeringContainer = triggeringContainer;
    m_referencedFromCurrentSpell = false;
    m_executedCurrently = false;
    m_delayStart = 0;
    m_immediateHandled = false;
    m_delayAtDamageCount = 0;

    m_applyMultiplierMask = 0;
    m_auraScaleMask = 0;

    // Get data for type of attack
    switch (m_spellInfo->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
            if (m_spellInfo->HasAttribute(SPELL_ATTR3_REQ_OFFHAND))
                m_attackType = OFF_ATTACK;
            else
                m_attackType = BASE_ATTACK;
            break;
        case SPELL_DAMAGE_CLASS_RANGED:
            m_attackType = RANGED_ATTACK;
            break;
        default:
                                                            // Wands
            if (m_spellInfo->HasAttribute(SPELL_ATTR3_REQ_WAND))
                m_attackType = RANGED_ATTACK;
            else
                m_attackType = BASE_ATTACK;
            break;
    }

    m_spellSchoolMask = info->GetSchoolMask();           // Can be override for some spell (wand shoot for example)

    if(m_attackType == RANGED_ATTACK)
    {
        // wand case
        if((m_caster->GetClassMask() & CLASSMASK_WAND_USERS) != 0 && m_caster->GetTypeId()==TYPEID_PLAYER)
        {
            if(Item* pItem = (m_caster->ToPlayer())->GetWeaponForAttack(RANGED_ATTACK))
                m_spellSchoolMask = SpellSchoolMask(1 << pItem->GetTemplate()->Damage->DamageType);
        }
    }
    // Set health leech amount to zero
    m_healthLeech = 0;

    if(originalCasterGUID)
        m_originalCasterGUID = originalCasterGUID;
    else
        m_originalCasterGUID = m_caster->GetGUID();

    if(m_originalCasterGUID==m_caster->GetGUID())
        m_originalCaster = m_caster;
    else
    {
        m_originalCaster = ObjectAccessor::GetUnit(*m_caster,m_originalCasterGUID);
        if(m_originalCaster && !m_originalCaster->IsInWorld()) m_originalCaster = NULL;
    }

    for(int i=0; i <3; ++i)
        m_currentBasePoints[i] = m_spellValue->EffectBasePoints[i];

    m_spellState = SPELL_STATE_NULL;

    m_castPositionX = m_castPositionY = m_castPositionZ = 0;
    m_TriggerSpells.clear();
    m_IsTriggeredSpell = triggered;
    //m_AreaAura = false;
    m_CastItem = NULL;

    unitTarget = NULL;
    itemTarget = NULL;
    gameObjTarget = NULL;
    focusObject = NULL;
    m_cast_count = 0;
    m_triggeredByAuraSpell  = NULL;

    effectHandleMode = SPELL_EFFECT_HANDLE_LAUNCH;

    //Auto Shot & Shoot
    if( m_spellInfo->AttributesEx2 == SPELL_ATTR2_AUTOREPEAT_FLAG && !triggered )
        m_autoRepeat = true;
    else
        m_autoRepeat = false;

    m_powerCost = 0;                                        // setup to correct value in Spell::prepare, don't must be used before.
    m_casttime = 0;                                         // setup to correct value in Spell::prepare, don't must be used before.
    m_timer = 0;                                            // will set to castime in prepare

    m_channelTargetEffectMask = 0;

    // determine reflection
    m_canReflect = false;
    m_removeReflect = false;

    if(   m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC 
       && !m_spellInfo->HasAttribute(SPELL_ATTR1_CANT_BE_REDIRECTED)
       && !m_spellInfo->HasAttribute(SPELL_ATTR1_CANT_BE_REFLECTED)
       && !m_spellInfo->HasAttribute(SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY)
       && !m_spellInfo->HasAttribute(SPELL_ATTR0_ABILITY)
       && !m_spellInfo->IsPassive()
       && (!m_spellInfo->IsPositive() || m_spellInfo->HasEffect(SPELL_EFFECT_DISPEL))
      )
        m_canReflect = true;

    CleanupTargetList();
    //TC memset(m_effectExecuteData, 0, MAX_SPELL_EFFECTS * sizeof(ByteBuffer*));

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        m_destTargets[i] = SpellDestination(*m_caster);

    // xinef:
    _spellTargetsSelected = false;
}

Spell::~Spell()
{
    delete m_spellValue;
    delete m_preGeneratedPath;
}

void Spell::InitExplicitTargets(SpellCastTargets const& targets)
{
    m_targets = targets;
    // this function tries to correct spell explicit targets for spell
    // client doesn't send explicit targets correctly sometimes - we need to fix such spells serverside
    // this also makes sure that we correctly send explicit targets to client (removes redundant data)
    uint32 neededTargets = m_spellInfo->GetExplicitTargetMask();

    if (WorldObject* target = m_targets.GetObjectTarget())
    {
        // check if object target is valid with needed target flags
        // for unit case allow corpse target mask because player with not released corpse is a unit target
        if ((target->ToUnit() && !(neededTargets & (TARGET_FLAG_UNIT_MASK | TARGET_FLAG_CORPSE_MASK)))
            || (target->ToGameObject() && !(neededTargets & TARGET_FLAG_GAMEOBJECT_MASK))
            || (target->ToCorpse() && !(neededTargets & TARGET_FLAG_CORPSE_MASK)))
            m_targets.RemoveObjectTarget();
    }
    else
    {
        // try to select correct unit target if not provided by client or by serverside cast
        if (neededTargets & (TARGET_FLAG_UNIT_MASK))
        {
            Unit* unit = NULL;
            // try to use player selection as a target
            if (Player* playerCaster = m_caster->ToPlayer())
            {
                // selection has to be found and to be valid target for the spell
                if (Unit* selectedUnit = ObjectAccessor::GetUnit(*m_caster, playerCaster->GetTarget()))
                    if (m_spellInfo->CheckExplicitTarget(m_caster, selectedUnit) == SPELL_CAST_OK)
                        unit = selectedUnit;
            }
            // try to use attacked unit as a target
            else if ((m_caster->GetTypeId() == TYPEID_UNIT) && neededTargets & (TARGET_FLAG_UNIT_ENEMY | TARGET_FLAG_UNIT))
                unit = m_caster->GetVictim();

            // didn't find anything - let's use self as target
            if (!unit && neededTargets & (TARGET_FLAG_UNIT_RAID | TARGET_FLAG_UNIT_PARTY | TARGET_FLAG_UNIT_ALLY))
                unit = m_caster;

            m_targets.SetUnitTarget(unit);
        }
    }

    // check if spell needs dst target
    if (neededTargets & TARGET_FLAG_DEST_LOCATION)
    {
        // and target isn't set
        if (!m_targets.HasDst())
        {
            // try to use unit target if provided
            if (WorldObject* target = targets.GetObjectTarget())
                m_targets.SetDst(*target);
            // or use self if not available
            else
                m_targets.SetDst(*m_caster);
        }
    }
    else
        m_targets.RemoveDst();

    if (neededTargets & TARGET_FLAG_SOURCE_LOCATION)
    {
        if (!targets.HasSrc())
            m_targets.SetSrc(*m_caster);
    }
    else
        m_targets.RemoveSrc();
}

void Spell::SelectExplicitTargets()
{
    // here go all explicit target changes made to explicit targets after spell prepare phase is finished
    if (Unit* target = m_targets.GetUnitTarget())
    {
        // check for explicit target redirection, for Grounding Totem for example
        if (m_spellInfo->GetExplicitTargetMask() & TARGET_FLAG_UNIT_ENEMY
            || (m_spellInfo->GetExplicitTargetMask() & TARGET_FLAG_UNIT
                && (!m_spellInfo->IsPositive() || (!m_caster->IsFriendlyTo(target) && m_spellInfo->HasEffect(SPELL_EFFECT_DISPEL)))))
        {
            Unit* redirect;
            switch (m_spellInfo->DmgClass)
            {
            case SPELL_DAMAGE_CLASS_MAGIC:
                redirect = m_caster->GetMagicHitRedirectTarget(target, m_spellInfo);
                break;
            case SPELL_DAMAGE_CLASS_MELEE:
            case SPELL_DAMAGE_CLASS_RANGED:
                redirect = m_caster->GetMeleeHitRedirectTarget(target, m_spellInfo);
                break;
            default:
                redirect = NULL;
                break;
            }
            if (redirect && (redirect != target))
                m_targets.SetUnitTarget(redirect);
        }
    }
}

void Spell::SelectSpellTargets()
{
    // select targets for cast phase
    SelectExplicitTargets();

    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        switch (m_spellInfo->Effects[i].Effect)
        {
        case SPELL_EFFECT_DUMMY:
        {
            switch (m_spellInfo->Id)
            {
            case 20577:                         // Cannibalize
            {
                // non-standard target selection
                float max_range = m_spellInfo->GetMaxRange(false, m_caster->GetSpellModOwner(), this);
                WorldObject* result = NULL;

                Trinity::CannibalizeObjectCheck u_check(m_caster, max_range);
                Trinity::WorldObjectSearcher<Trinity::CannibalizeObjectCheck > searcher(m_caster, result, u_check);
                m_caster->VisitNearbyGridObject(max_range, searcher);
                if (!result)
                    m_caster->VisitNearbyWorldObject(max_range, searcher);

                if (result)
                {
                    switch (result->GetTypeId())
                    {
                    case TYPEID_UNIT:
                    case TYPEID_PLAYER:
                        AddUnitTarget((Unit*)result, i);
                        break;
                    case TYPEID_CORPSE:
                        /* todo spelltargets m_targets.SetCorpseTarget((Corpse*)result); */
                        if (Player* owner = ObjectAccessor::FindPlayer(((Corpse*)result)->GetOwnerGUID()))
                            AddUnitTarget(owner, i);
                        break;
                    }
                }
                else
                {
                    // clear cooldown at fail
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        (m_caster->ToPlayer())->RemoveSpellCooldown(m_spellInfo->Id);

                        m_caster->ToPlayer()->SendClearCooldown(m_spellInfo->Id, m_caster);
                    }

                    SendCastResult(SPELL_FAILED_NO_EDIBLE_CORPSES);
                    finish(false);
                }
                break;
            }
            case 40160: // Throw Bomb
            {
                GameObject* go = NULL;

                CellCoord pair(Trinity::ComputeCellCoord(m_targets.GetDstPos()->GetPositionX(), m_targets.GetDstPos()->GetPositionY()));
                Cell cell(pair);
                cell.data.Part.reserved = ALL_DISTRICT;
                cell.SetNoCreate();

                Trinity::NearestGameObjectEntryInObjectRangeCheck go_check(*m_caster, 185861, 100.0f);
                Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> searcher(m_caster, go, go_check);

                TypeContainerVisitor<Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck>, GridTypeMapContainer> go_searcher(searcher);

                cell.Visit(pair, go_searcher, *m_caster->GetMap(), *m_caster, 100.0f);

                if (go && go->GetDistance2d(m_targets.GetDstPos()->GetPositionX(), m_targets.GetDstPos()->GetPositionY()) <= 4.0f) {
                    go->SetLootState(GO_JUST_DEACTIVATED);
                    m_caster->ToPlayer()->KilledMonsterCredit(23118, go->GetGUID());
                }

                break;
            }
            case 33655: // Dropping the Nether Modulator
            {
                GameObject* go = NULL;

                CellCoord pair(Trinity::ComputeCellCoord(m_targets.GetDstPos()->GetPositionX(), m_targets.GetDstPos()->GetPositionY()));
                Cell cell(pair);
                cell.data.Part.reserved = ALL_DISTRICT;
                cell.SetNoCreate();

                Trinity::NearestGameObjectEntryInObjectRangeCheck go_check(*m_caster, 183350, 100.0f);
                Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> searcher(m_caster, go, go_check);

                TypeContainerVisitor<Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck>, GridTypeMapContainer> go_searcher(searcher);

                cell.Visit(pair, go_searcher, *m_caster->GetMap());

                if (go && go->GetDistance2d(m_targets.GetDstPos()->GetPositionX(), m_targets.GetDstPos()->GetPositionY()) <= 17.0f) {
                    go->SetLootState(GO_JUST_DEACTIVATED);
                    m_caster->ToPlayer()->KilledMonsterCredit(19291, go->GetGUID());
                    break;
                }

                go = NULL;

                CellCoord pair2(Trinity::ComputeCellCoord(m_targets.GetDstPos()->GetPositionX(), m_targets.GetDstPos()->GetPositionY()));
                Cell cell2(pair2);
                cell2.data.Part.reserved = ALL_DISTRICT;
                cell2.SetNoCreate();

                Trinity::NearestGameObjectEntryInObjectRangeCheck go_check2(*m_caster, 183351, 100.0f);
                Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> searcher2(m_caster, go, go_check2);

                TypeContainerVisitor<Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck>, GridTypeMapContainer> go_searcher2(searcher2);

                cell2.Visit(pair2, go_searcher2, *m_caster->GetMap());

                if (go && go->GetDistance2d(m_targets.GetDstPos()->GetPositionX(), m_targets.GetDstPos()->GetPositionY()) <= 17.0f) {
                    go->SetLootState(GO_JUST_DEACTIVATED);
                    m_caster->ToPlayer()->KilledMonsterCredit(19292, go->GetGUID());
                    break;
                }

                break;
            }

            }
        }
        }
    }

    uint32 processedAreaEffectsMask = 0;
    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        // not call for empty effect.
        // Also some spells use not used effect targets for store targets for dummy effect in triggered spells
        if (!m_spellInfo->Effects[i].IsEffect())
            continue;

        // set expected type of implicit targets to be sent to client
        uint32 implicitTargetMask = GetTargetFlagMask(m_spellInfo->Effects[i].TargetA.GetObjectType()) | GetTargetFlagMask(m_spellInfo->Effects[i].TargetB.GetObjectType());
        if (implicitTargetMask & TARGET_FLAG_UNIT)
            m_targets.SetTargetFlag(TARGET_FLAG_UNIT);
        if (implicitTargetMask & (TARGET_FLAG_GAMEOBJECT | TARGET_FLAG_GAMEOBJECT_ITEM))
            m_targets.SetTargetFlag(TARGET_FLAG_GAMEOBJECT);

        SelectEffectImplicitTargets(SpellEffIndex(i), m_spellInfo->Effects[i].TargetA, processedAreaEffectsMask);
        SelectEffectImplicitTargets(SpellEffIndex(i), m_spellInfo->Effects[i].TargetB, processedAreaEffectsMask);

        // Select targets of effect based on effect type
        // those are used when no valid target could be added for spell effect based on spell target type
        // some spell effects use explicit target as a default target added to target map (like SPELL_EFFECT_LEARN_SPELL)
        // some spell effects add target to target map only when target type specified (like SPELL_EFFECT_WEAPON)
        // some spell effects don't add anything to target map (confirmed with sniffs) (like SPELL_EFFECT_DESTROY_ALL_TOTEMS)
        SelectEffectTypeImplicitTargets(i);

        if (m_targets.HasDst())
            AddDestTarget(*m_targets.GetDst(), i);

        if (m_spellInfo->IsChanneled())
        {
            uint8 mask = (1 << i);
            for (std::list<TargetInfo>::iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
            {
                if (ihit->effectMask & mask)
                {
                    m_channelTargetEffectMask |= mask;
                    break;
                }
            }
        }
        else if (m_auraScaleMask)
        {
            bool checkLvl = !m_UniqueTargetInfo.empty();
            for (std::list<TargetInfo>::iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end();)
            {
                // remove targets which did not pass min level check
                if (m_auraScaleMask && ihit->effectMask == m_auraScaleMask)
                {
                    // Do not check for selfcast
                    if (!ihit->scaleAura && ihit->targetGUID != m_caster->GetGUID())
                    {
                        m_UniqueTargetInfo.erase(ihit++);
                        continue;
                    }
                }
                ++ihit;
            }
            if (checkLvl && m_UniqueTargetInfo.empty())
            {
                SendCastResult(SPELL_FAILED_LOWLEVEL);
                finish(false);
            }
        }
    }

    if (m_targets.HasDst())
    {
        if (m_targets.HasTraj())
        {
            float speed = m_targets.GetSpeedXY();
            if (speed > 0.0f)
                m_delayTrajectory = (uint64)floor(m_targets.GetDist2d() / speed * 1000.0f);
        }
        else if (m_spellInfo->Speed > 0.0f)
        {
            float dist = m_caster->GetExactDist(m_targets.GetDstPos());
            m_delayTrajectory = (uint64)floor(dist / m_spellInfo->Speed * 1000.0f);
        }
    }
}


void Spell::SelectEffectImplicitTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32& processedEffectMask)
{
    if (!targetType.GetTarget())
        return;

    uint32 effectMask = 1 << effIndex;
    // set the same target list for all effects
    // some spells appear to need this, however this requires more research
    switch (targetType.GetSelectionCategory())
    {
    case TARGET_SELECT_CATEGORY_NEARBY:
    case TARGET_SELECT_CATEGORY_CONE:
    case TARGET_SELECT_CATEGORY_AREA:
        // targets for effect already selected
        if (effectMask & processedEffectMask)
            return;
        // choose which targets we can select at once
        for (uint32 j = effIndex + 1; j < MAX_SPELL_EFFECTS; ++j)
        {
            SpellEffectInfo const* effects = GetSpellInfo()->Effects;
            if (effects[effIndex].TargetA.GetTarget() == effects[j].TargetA.GetTarget() &&
                effects[effIndex].TargetB.GetTarget() == effects[j].TargetB.GetTarget() &&
                effects[effIndex].ImplicitTargetConditions == effects[j].ImplicitTargetConditions &&
                effects[effIndex].CalcRadius(m_caster) == effects[j].CalcRadius(m_caster)/* &&
 todo spellscript                CheckScriptEffectImplicitTargets(effIndex, j) */
                )
            {
                effectMask |= 1 << j;
            }
        }
        processedEffectMask |= effectMask;
        break;
    default:
        break;
    }

    switch (targetType.GetSelectionCategory())
    {
    case TARGET_SELECT_CATEGORY_CHANNEL:
        SelectImplicitChannelTargets(effIndex, targetType);
        break;
    case TARGET_SELECT_CATEGORY_NEARBY:
        SelectImplicitNearbyTargets(effIndex, targetType, effectMask);
        break;
    case TARGET_SELECT_CATEGORY_CONE:
        SelectImplicitConeTargets(effIndex, targetType, effectMask);
        break;
    case TARGET_SELECT_CATEGORY_AREA:
        SelectImplicitAreaTargets(effIndex, targetType, effectMask);
        break;
    case TARGET_SELECT_CATEGORY_TRAJ:
        // xinef: just in case there is no dest, explanation in SelectImplicitDestDestTargets
        if (!m_targets.HasDst())
            m_targets.SetDst(*m_caster);

        SelectImplicitTrajTargets(effIndex, targetType);
        break;
    case TARGET_SELECT_CATEGORY_DEFAULT:
        switch (targetType.GetObjectType())
        {
        case TARGET_OBJECT_TYPE_SRC:
            switch (targetType.GetReferenceType())
            {
            case TARGET_REFERENCE_TYPE_CASTER:
                m_targets.SetSrc(*m_caster);
                break;
            default:
                ASSERT(false && "Spell::SelectEffectImplicitTargets: received not implemented select target reference type for TARGET_TYPE_OBJECT_SRC");
                break;
            }
            break;
        case TARGET_OBJECT_TYPE_DEST:
            switch (targetType.GetReferenceType())
            {
            case TARGET_REFERENCE_TYPE_CASTER:
                SelectImplicitCasterDestTargets(effIndex, targetType);
                break;
            case TARGET_REFERENCE_TYPE_TARGET:
                SelectImplicitTargetDestTargets(effIndex, targetType);
                break;
            case TARGET_REFERENCE_TYPE_DEST:
                SelectImplicitDestDestTargets(effIndex, targetType);
                break;
            default:
                ASSERT(false && "Spell::SelectEffectImplicitTargets: received not implemented select target reference type for TARGET_TYPE_OBJECT_DEST");
                break;
            }
            break;
        default:
            switch (targetType.GetReferenceType())
            {
            case TARGET_REFERENCE_TYPE_CASTER:
                SelectImplicitCasterObjectTargets(effIndex, targetType);
                break;
            case TARGET_REFERENCE_TYPE_TARGET:
                SelectImplicitTargetObjectTargets(effIndex, targetType);
                break;
            default:
                ASSERT(false && "Spell::SelectEffectImplicitTargets: received not implemented select target reference type for TARGET_TYPE_OBJECT");
                break;
            }
            break;
        }
        break;
    case TARGET_SELECT_CATEGORY_NYI:
        ;//sLog->outDebug(LOG_FILTER_SPELLS_AURAS, "SPELL: target type %u, found in spellID %u, effect %u is not implemented yet!", m_spellInfo->Id, effIndex, targetType.GetTarget());
        break;
    default:
        ASSERT(false && "Spell::SelectEffectImplicitTargets: received not implemented select target category");
        break;
    }
}


void Spell::SelectImplicitChannelTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    if (targetType.GetReferenceType() != TARGET_REFERENCE_TYPE_CASTER)
    {
        ASSERT(false && "Spell::SelectImplicitChannelTargets: received not implemented target reference type");
        return;
    }

    switch (targetType.GetTarget())
    {
    case TARGET_UNIT_CHANNEL_TARGET:
    {
        // Xinef: All channel selectors have needed data passed in m_targets structure
        WorldObject* target = m_targets.GetObjectTargetChannel(m_caster);
        if (target)
        {
            CallScriptObjectTargetSelectHandlers(target, effIndex, targetType);
            // unit target may be no longer avalible - teleported out of map for example
            if (target && target->ToUnit())
                AddUnitTarget(target->ToUnit(), 1 << effIndex);
        }
        else
            ;//sLog->outDebug(LOG_FILTER_SPELLS_AURAS, "SPELL: cannot find channel spell target for spell ID %u, effect %u", m_spellInfo->Id, effIndex);
        break;
    }
    case TARGET_DEST_CHANNEL_TARGET:
        if (m_targets.HasDstChannel())
            m_targets.SetDst(*m_targets.GetDstChannel());
        else if (WorldObject* target = m_targets.GetObjectTargetChannel(m_caster))
        {
            CallScriptObjectTargetSelectHandlers(target, effIndex, targetType);
            if (target)
                m_targets.SetDst(*target);
        }
        else //if (!m_targets.HasDst())
            ;//sLog->outDebug(LOG_FILTER_SPELLS_AURAS, "SPELL: cannot find channel spell destination for spell ID %u, effect %u", m_spellInfo->Id, effIndex);
        break;
#ifdef LICH_KING
    case TARGET_DEST_CHANNEL_CASTER:
        if (GetOriginalCaster())
            m_targets.SetDst(*GetOriginalCaster());
        break;
#endif
    default:
        ASSERT(false && "Spell::SelectImplicitChannelTargets: received not implemented target type");
        break;
    }
}

void Spell::SelectImplicitNearbyTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask)
{
    if (targetType.GetReferenceType() != TARGET_REFERENCE_TYPE_CASTER)
    {
        ASSERT(false && "Spell::SelectImplicitNearbyTargets: received not implemented target reference type");
        return;
    }

    float range = 0.0f;
    switch (targetType.GetCheckType())
    {
    case TARGET_CHECK_ENEMY:
        range = m_spellInfo->GetMaxRange(false, m_caster, this);
        break;
    case TARGET_CHECK_ALLY:
    case TARGET_CHECK_PARTY:
    case TARGET_CHECK_RAID:
    case TARGET_CHECK_RAID_CLASS:
        range = m_spellInfo->GetMaxRange(true, m_caster, this);
        break;
    case TARGET_CHECK_ENTRY:
    case TARGET_CHECK_DEFAULT:
        range = m_spellInfo->GetMaxRange(m_spellInfo->IsPositive(), m_caster, this);
        break;
    default:
        ASSERT(false && "Spell::SelectImplicitNearbyTargets: received not implemented selection check type");
        break;
    }

    ConditionList* condList = m_spellInfo->Effects[effIndex].ImplicitTargetConditions;

    // handle emergency case - try to use other provided targets if no conditions provided
    if (targetType.GetCheckType() == TARGET_CHECK_ENTRY && (!condList || condList->empty()))
    {
        ;//sLog->outDebug(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitNearbyTargets: no conditions entry for target with TARGET_CHECK_ENTRY of spell ID %u, effect %u - selecting default targets", m_spellInfo->Id, effIndex);
        switch (targetType.GetObjectType())
        {
        case TARGET_OBJECT_TYPE_GOBJ:
            if (m_spellInfo->RequiresSpellFocus)
            {
                if (focusObject)
                    AddGOTarget(focusObject, effMask);
                return;
            }
            break;
        case TARGET_OBJECT_TYPE_DEST:
            if (m_spellInfo->RequiresSpellFocus)
            {
                if (focusObject)
                    m_targets.SetDst(*focusObject);
                return;
            }
            break;
        default:
            break;
        }
    }

    WorldObject* target = SearchNearbyTarget(range, targetType.GetObjectType(), targetType.GetCheckType(), condList);
    if (!target)
    {
        ;//sLog->outDebug(LOG_FILTER_SPELLS_AURAS, "Spell::SelectImplicitNearbyTargets: cannot find nearby target for spell ID %u, effect %u", m_spellInfo->Id, effIndex);
        return;
    }

    CallScriptObjectTargetSelectHandlers(target, effIndex, targetType);
    if (!target)
    {
        //TC_LOG_DEBUG("spells", "Spell::SelectImplicitNearbyTargets: OnObjectTargetSelect script hook for spell Id %u set NULL target, effect %u", m_spellInfo->Id, effIndex);
        return;
    }

    switch (targetType.GetObjectType())
    {
    case TARGET_OBJECT_TYPE_UNIT:
    {
        if (Unit* unitTarget = target->ToUnit())
        {
            AddUnitTarget(unitTarget, effMask, true, false);
            // xinef: important! if channeling spell have nearby entry, it has no unitTarget by default
            // and if channeled spell has target 77, it requires unitTarget, set it here!
            // xinef: if we have NO unit target
            if (!m_targets.GetUnitTarget())
                m_targets.SetUnitTarget(unitTarget);
        }
        else
        {
            //TC_LOG_DEBUG("spells", "Spell::SelectImplicitNearbyTargets: OnObjectTargetSelect script hook for spell Id %u set object of wrong type, expected unit, got %s, effect %u", m_spellInfo->Id, GetLogNameForGuid(target->GetGUID()), effMask);
            return;
        }
        break;
    }
    case TARGET_OBJECT_TYPE_GOBJ:
        if (GameObject* gobjTarget = target->ToGameObject())
            AddGOTarget(gobjTarget, effMask);
        else
        {
            //TC_LOG_DEBUG("spells", "Spell::SelectImplicitNearbyTargets: OnObjectTargetSelect script hook for spell Id %u set object of wrong type, expected gameobject, got %s, effect %u", m_spellInfo->Id, GetLogNameForGuid(target->GetGUID()), effMask);
            return;
        }
        break;
    case TARGET_OBJECT_TYPE_DEST:
        m_targets.SetDst(*target);
        break;
    default:
        ASSERT(false && "Spell::SelectImplicitNearbyTargets: received not implemented target object type");
        break;
    }

    SelectImplicitChainTargets(effIndex, targetType, target, effMask);
}

void Spell::SelectImplicitConeTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask)
{
    if (targetType.GetReferenceType() != TARGET_REFERENCE_TYPE_CASTER)
    {
        ASSERT(false && "Spell::SelectImplicitConeTargets: received not implemented target reference type");
        return;
    }
    std::list<WorldObject*> targets;
    SpellTargetObjectTypes objectType = targetType.GetObjectType();
    SpellTargetCheckTypes selectionType = targetType.GetCheckType();
    ConditionList* condList = m_spellInfo->Effects[effIndex].ImplicitTargetConditions;
    float coneAngle = M_PI / 2;
    float radius = m_spellInfo->Effects[effIndex].CalcRadius(m_caster) * m_spellValue->RadiusMod;

    if (uint32 containerTypeMask = GetSearcherTypeMask(objectType, condList))
    {
        Trinity::WorldObjectSpellConeTargetCheck check(coneAngle, radius, m_caster, m_spellInfo, selectionType, condList);
        Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellConeTargetCheck> searcher(m_caster, targets, check, containerTypeMask);
        SearchTargets<Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellConeTargetCheck> >(searcher, containerTypeMask, m_caster, m_caster, radius);

        CallScriptObjectAreaTargetSelectHandlers(targets, effIndex, targetType);

        if (!targets.empty())
        {
            // Other special target selection goes here
            if (uint32 maxTargets = m_spellValue->MaxAffectedTargets)
            {
#ifdef LICH_KING
                Unit::AuraEffectList const& Auras = m_caster->GetAuraEffectsByType(SPELL_AURA_MOD_MAX_AFFECTED_TARGETS);
                for (Unit::AuraEffectList::const_iterator j = Auras.begin(); j != Auras.end(); ++j)
                    if ((*j)->IsAffectedOnSpell(m_spellInfo))
                        maxTargets += (*j)->GetAmount();
#endif
                Trinity::Containers::RandomResizeList(targets, maxTargets);
            }

            for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
            {
                if (Unit* unitTarget = (*itr)->ToUnit())
                    AddUnitTarget(unitTarget, effMask, false);
                else if (GameObject* gObjTarget = (*itr)->ToGameObject())
                    AddGOTarget(gObjTarget, effMask);
            }
        }
    }
}

void Spell::SelectImplicitAreaTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask)
{
    Unit* referer = NULL;
    switch (targetType.GetReferenceType())
    {
    case TARGET_REFERENCE_TYPE_SRC:
    case TARGET_REFERENCE_TYPE_DEST:
    case TARGET_REFERENCE_TYPE_CASTER:
        referer = m_caster;
        break;
    case TARGET_REFERENCE_TYPE_TARGET:
        referer = m_targets.GetUnitTarget();
        break;
    case TARGET_REFERENCE_TYPE_LAST:
    {
        // find last added target for this effect
        for (std::list<TargetInfo>::reverse_iterator ihit = m_UniqueTargetInfo.rbegin(); ihit != m_UniqueTargetInfo.rend(); ++ihit)
        {
            if (ihit->effectMask & (1 << effIndex))
            {
                referer = ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID);
                break;
            }
        }
        break;
    }
    default:
        ASSERT(false && "Spell::SelectImplicitAreaTargets: received not implemented target reference type");
        return;
    }
    if (!referer)
        return;

    Position const* center = NULL;
    switch (targetType.GetReferenceType())
    {
    case TARGET_REFERENCE_TYPE_SRC:
        center = m_targets.GetSrcPos();
        break;
    case TARGET_REFERENCE_TYPE_DEST:
        center = m_targets.GetDstPos();
        break;
    case TARGET_REFERENCE_TYPE_CASTER:
    case TARGET_REFERENCE_TYPE_TARGET:
    case TARGET_REFERENCE_TYPE_LAST:
        center = referer;
        break;
    default:
        ASSERT(false && "Spell::SelectImplicitAreaTargets: received not implemented target reference type");
        return;
    }

    // Xinef: the distance should be increased by caster size, it is neglected in latter calculations
    std::list<WorldObject*> targets;
    float radius = m_spellInfo->Effects[effIndex].CalcRadius(m_caster) * m_spellValue->RadiusMod;
    SearchAreaTargets(targets, radius, center, referer, targetType.GetObjectType(), targetType.GetCheckType(), m_spellInfo->Effects[effIndex].ImplicitTargetConditions);

    CallScriptObjectAreaTargetSelectHandlers(targets, effIndex, targetType);

    if (!targets.empty())
    {
        // Other special target selection goes here
        if (uint32 maxTargets = m_spellValue->MaxAffectedTargets)
        {
#ifdef LICH_KING
            Unit::AuraEffectList const& Auras = m_caster->GetAuraEffectsByType(SPELL_AURA_MOD_MAX_AFFECTED_TARGETS);
            for (Unit::AuraEffectList::const_iterator j = Auras.begin(); j != Auras.end(); ++j)
                if ((*j)->IsAffectedOnSpell(m_spellInfo))
                    maxTargets += (*j)->GetAmount();
#endif
            Trinity::Containers::RandomResizeList(targets, maxTargets);
        }

        for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
        {
            if (Unit* unitTarget = (*itr)->ToUnit())
                AddUnitTarget(unitTarget, effMask, false);
            else if (GameObject* gObjTarget = (*itr)->ToGameObject())
                AddGOTarget(gObjTarget, effMask);
        }
    }
}

void Spell::SelectImplicitCasterDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    SpellDestination dest(*m_caster);

    switch (targetType.GetTarget())
    {
    case TARGET_DEST_CASTER:
    case TARGET_DEST_CASTER_UNK_36:
        break;
    case TARGET_DEST_HOME:
        if (Player* playerCaster = m_caster->ToPlayer())
            dest = SpellDestination(playerCaster->m_homebindX, playerCaster->m_homebindY, playerCaster->m_homebindZ, playerCaster->GetOrientation(), playerCaster->m_homebindMapId);
        break;
    case TARGET_DEST_DB:
        if (SpellTargetPosition const* st = sSpellMgr->GetSpellTargetPosition(m_spellInfo->Id, effIndex))
        {
            /// @todo fix this check
            if (m_spellInfo->HasEffect(SPELL_EFFECT_TELEPORT_UNITS) || m_spellInfo->HasEffect(SPELL_EFFECT_BIND))
                dest = SpellDestination(st->target_X, st->target_Y, st->target_Z, st->target_Orientation, (int32)st->target_mapId);
            else if (st->target_mapId == m_caster->GetMapId())
                dest = SpellDestination(st->target_X, st->target_Y, st->target_Z, st->target_Orientation);
        }
        else
        {
            ;//sLog->outDebug(LOG_FILTER_SPELLS_AURAS, "SPELL: unknown target coordinates for spell ID %u", m_spellInfo->Id);
            if (WorldObject* target = m_targets.GetObjectTarget())
                dest = SpellDestination(*target);
        }
        break;
    case TARGET_DEST_CASTER_FISHING:
    {
        float min_dis = m_spellInfo->GetMinRange(true);
        float max_dis = m_spellInfo->GetMaxRange(true);
        float dis = (float)rand_norm() * (max_dis - min_dis) + min_dis;
        float x, y, z, angle;
        angle = (float)rand_norm() * static_cast<float>(M_PI * 35.0f / 180.0f) - static_cast<float>(M_PI * 17.5f / 180.0f);
        m_caster->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE, dis, angle);

        float ground = m_caster->GetMap()->GetHeight(m_caster->GetPhaseMask(), x, y, z, true, 120.0f);
        float liquidLevel = VMAP_INVALID_HEIGHT_VALUE;
        LiquidData liquidData;
        if (m_caster->GetMap()->getLiquidStatus(x, y, z, BASE_LIQUID_TYPE_MASK_ALL, &liquidData))
            liquidLevel = liquidData.level;

        if (liquidLevel <= ground) // When there is no liquid Map::GetWaterOrGroundLevel returns ground level
        {
            SendCastResult(SPELL_FAILED_NOT_HERE);
            SendChannelUpdate(0);
            finish(false);
            return;
        }

        if (ground + 0.75 > liquidLevel)
        {
            SendCastResult(SPELL_FAILED_TOO_SHALLOW);
            SendChannelUpdate(0);
            finish(false);
            return;
        }

        dest = SpellDestination(x, y, liquidLevel, m_caster->GetOrientation());
        break;
    }
    default:
    {
        float dist;
        float angle = targetType.CalcDirectionAngle();
        float objSize = m_caster->GetObjectSize();
        if (targetType.GetTarget() == TARGET_DEST_CASTER_SUMMON)
            dist = PET_FOLLOW_DIST;
        else
            dist = m_spellInfo->Effects[effIndex].CalcRadius(m_caster);

        if (dist < objSize)
        {
            dist = objSize;
            // xinef: give the summon some space (eg. totems)
            if (m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo->Effects[effIndex].IsEffect(SPELL_EFFECT_SUMMON))
                dist += objSize;
        }
        else if (targetType.GetTarget() == TARGET_DEST_CASTER_RANDOM)
            dist = objSize + (dist - objSize) * (float)rand_norm();

        Position pos;
        bool totemCollision = false;
        if (m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_SUMMON)
        {
#ifdef LICH_KING
            SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(m_spellInfo->Effects[effIndex].MiscValueB);
            if (properties && (properties->Type == SUMMON_TYPE_TOTEM || properties->Type == SUMMON_TYPE_LIGHTWELL))
#else
            if (m_spellInfo->Effects[effIndex].MiscValueB == SUMMON_TYPE_TOTEM)
#endif
            {
                totemCollision = true;
                /* sunwell m_caster->GetFirstCollisionPositionForTotem(pos, dist, angle, false); */
                pos = m_caster->GetFirstWalkableCollisionPosition(dist, angle, false);
            }
        }
        else if (m_spellInfo->Effects[effIndex].Effect >= SPELL_EFFECT_SUMMON_OBJECT_SLOT1 && m_spellInfo->Effects[effIndex].Effect <= SPELL_EFFECT_SUMMON_OBJECT_SLOT4)
        {
            totemCollision = true;
            /*sunwell m_caster->GetFirstCollisionPositionForTotem(pos, dist, angle, true); */
            pos = m_caster->GetFirstWalkableCollisionPosition(dist, angle, false);
        }

        if (!totemCollision)
        {
            if (m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_LEAP 
                || m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_TELEPORT_UNITS 
#ifdef LICH_KING
                || m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_JUMP_DEST 
#endif
                || (m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_SUMMON))
               /*sunwell m_caster->GetFirstCollisionPosition(pos, dist, angle); */
                pos = m_caster->GetFirstWalkableCollisionPosition(dist, angle, false);
            else
                /*sunwell m_caster->GetNearPosition(pos, dist, angle); */
                pos = m_caster->GetNearPosition(dist, angle);
        }
        dest.Relocate(pos);
        break;
    }
    }

    CallScriptDestinationTargetSelectHandlers(dest, effIndex, targetType);
    m_targets.SetDst(dest);
}

void Spell::SelectImplicitTargetDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    WorldObject* target = m_targets.GetObjectTarget();

    SpellDestination dest(*target);

    switch (targetType.GetTarget())
    {
    case TARGET_DEST_TARGET_ENEMY:
    case TARGET_DEST_TARGET_ANY:
        break;
    default:
    {
        float angle = targetType.CalcDirectionAngle();
        float objSize = target->GetObjectSize();
        float dist = m_spellInfo->Effects[effIndex].CalcRadius(m_caster);
        if (dist < objSize)
            dist = objSize;
        else if (targetType.GetTarget() == TARGET_DEST_TARGET_RANDOM)
            dist = objSize + (dist - objSize) * (float)rand_norm();

        Position pos;
        if (m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_LEAP 
            || m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_TELEPORT_UNITS 
#ifdef LICH_KING
            || m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_JUMP_DEST 
#endif
            || (m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_SUMMON))
            pos = target->GetFirstWalkableCollisionPosition(dist, angle);
        else
            pos = target->GetNearPosition(dist, angle);

        dest.Relocate(pos);
        break;;
    }
    }

    CallScriptDestinationTargetSelectHandlers(dest, effIndex, targetType);
    m_targets.SetDst(dest);
}

void Spell::SelectImplicitDestDestTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    // set destination to caster if no dest provided
    // can only happen if previous destination target could not be set for some reason
    // (not found nearby target, or channel target for example
    // maybe we should abort the spell in such case?
    CheckDst();

    SpellDestination dest(*m_targets.GetDst());

    switch (targetType.GetTarget())
    {
    case TARGET_DEST_DYNOBJ_ENEMY:
    case TARGET_DEST_DYNOBJ_ALLY:
    case TARGET_DEST_DYNOBJ_NONE:
    case TARGET_DEST_DEST:
        return;
    case TARGET_DEST_TRAJ:
        SelectImplicitTrajTargets(effIndex, targetType);
        return;
    default:
    {
        float angle = targetType.CalcDirectionAngle();
        float dist = m_spellInfo->Effects[effIndex].CalcRadius(m_caster);
        if (targetType.GetTarget() == TARGET_DEST_DEST_RANDOM)
            dist *= float(rand_norm());

        Position pos = dest._position;
        m_caster->MovePosition(pos, dist, angle);

        dest.Relocate(pos);
        break;
    }
    }

    CallScriptDestinationTargetSelectHandlers(dest, effIndex, targetType);
    m_targets.ModDst(dest);
}

void Spell::SelectImplicitCasterObjectTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    WorldObject* target = NULL;
    bool checkIfValid = true;

    switch (targetType.GetTarget())
    {
    case TARGET_UNIT_CASTER:
        target = m_caster;
        checkIfValid = false;
        break;
    case TARGET_UNIT_MASTER:
        target = m_caster->GetCharmerOrOwner();
        break;
    case TARGET_UNIT_PET:
        /* TC
        target = m_caster->GetGuardianPet(); */
        target = m_caster->GetPet();
        if (!target)
            target = m_caster->GetCharm();
        break;
    case TARGET_UNIT_SUMMONER:
        /* TC 
        if (m_caster->IsSummon())
            target = m_caster->ToTempSummon()->GetSummoner(); */
        if (Unit* summoner = m_caster->GetSummoner())
            target = summoner;
        break;
#ifdef LICH_KING
    case TARGET_UNIT_VEHICLE:
        target = m_caster->GetVehicleBase();
        break;
    case TARGET_UNIT_PASSENGER_0:
    case TARGET_UNIT_PASSENGER_1:
    case TARGET_UNIT_PASSENGER_2:
    case TARGET_UNIT_PASSENGER_3:
    case TARGET_UNIT_PASSENGER_4:
    case TARGET_UNIT_PASSENGER_5:
    case TARGET_UNIT_PASSENGER_6:
    case TARGET_UNIT_PASSENGER_7:
        if (m_caster->GetTypeId() == TYPEID_UNIT && m_caster->ToCreature()->IsVehicle())
            target = m_caster->GetVehicleKit()->GetPassenger(targetType.GetTarget() - TARGET_UNIT_PASSENGER_0);
        break;
#endif
    default:
        break;
    }

    CallScriptObjectTargetSelectHandlers(target, effIndex, targetType);

    if (target && target->ToUnit())
        AddUnitTarget(target->ToUnit(), 1 << effIndex, checkIfValid);
}

void Spell::SelectImplicitTargetObjectTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    ASSERT((m_targets.GetObjectTarget() || m_targets.GetItemTarget()) && "Spell::SelectImplicitTargetObjectTargets - no explicit object or item target available!");

    WorldObject* target = m_targets.GetObjectTarget();

    CallScriptObjectTargetSelectHandlers(target, effIndex, targetType);

    if (target)
    {
        if (Unit* unit = target->ToUnit())
            AddUnitTarget(unit, 1 << effIndex, true, false);
        else if (GameObject* gobj = target->ToGameObject())
            AddGOTarget(gobj, 1 << effIndex);

        SelectImplicitChainTargets(effIndex, targetType, target, 1 << effIndex);
    }
    // Script hook can remove object target and we would wrongly land here
    else if (Item* item = m_targets.GetItemTarget())
        AddItemTarget(item, 1 << effIndex);
}

void Spell::SelectImplicitChainTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, WorldObject* target, uint32 effMask)
{
    uint32 maxTargets = m_spellInfo->Effects[effIndex].ChainTarget;
    if (Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_JUMP_TARGETS, maxTargets, this);

    if (maxTargets > 1)
    {
        // mark damage multipliers as used
        for (uint32 k = effIndex; k < MAX_SPELL_EFFECTS; ++k)
            if (effMask & (1 << k))
                m_damageMultipliers[k] = 1.0f;
        m_applyMultiplierMask |= effMask;

        std::list<WorldObject*> targets;
        SearchChainTargets(targets, maxTargets - 1, target, targetType.GetObjectType(), targetType.GetCheckType(), targetType.GetSelectionCategory()
            , m_spellInfo->Effects[effIndex].ImplicitTargetConditions, targetType.GetTarget() == TARGET_UNIT_TARGET_CHAINHEAL_ALLY);

        // Chain primary target is added earlier
        CallScriptObjectAreaTargetSelectHandlers(targets, effIndex, targetType);

        for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); ++itr)
            if (Unit* unitTarget = (*itr)->ToUnit())
                AddUnitTarget(unitTarget, effMask, false);
    }
}

float tangent(float x)
{
    x = tan(x);
    //if (x < std::numeric_limits<float>::max() && x > -std::numeric_limits<float>::max()) return x;
    //if (x >= std::numeric_limits<float>::max()) return std::numeric_limits<float>::max();
    //if (x <= -std::numeric_limits<float>::max()) return -std::numeric_limits<float>::max();
    if (x < 100000.0f && x > -100000.0f) return x;
    if (x >= 100000.0f) return 100000.0f;
    if (x <= 100000.0f) return -100000.0f;
    return 0.0f;
}

void Spell::SelectImplicitTrajTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    if (!m_targets.HasTraj())
        return;

    float dist2d = m_targets.GetDist2d();
    if (!dist2d)
        return;

    float srcToDestDelta = m_targets.GetDstPos()->m_positionZ - m_targets.GetSrcPos()->m_positionZ;


    // xinef: supply correct target type, DEST_DEST and similar are ALWAYS undefined
    // xinef: correct target is stored in TRIGGERED SPELL, however as far as i noticed, all checks are ENTRY, ENEMY
    std::list<WorldObject*> targets;
    Trinity::WorldObjectSpellTrajTargetCheck check(dist2d, m_targets.GetSrcPos(), m_caster, m_spellInfo, TARGET_CHECK_ENEMY /*targetCheckType*/, m_spellInfo->Effects[effIndex].ImplicitTargetConditions);
    Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellTrajTargetCheck> searcher(m_caster, targets, check, GRID_MAP_TYPE_MASK_ALL);
    SearchTargets<Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellTrajTargetCheck> >(searcher, GRID_MAP_TYPE_MASK_ALL, m_caster, m_targets.GetSrcPos(), dist2d);
    if (targets.empty())
        return;

    targets.sort(Trinity::ObjectDistanceOrderPred(m_caster));

    float b = tangent(m_targets.GetElevation());
    float a = (srcToDestDelta - dist2d * b) / (dist2d * dist2d);
    if (a > -0.0001f)
        a = 0;
    //DEBUG_TRAJ(sLog->outError("Spell::SelectTrajTargets: a %f b %f", a, b);)

    // Xinef: hack for distance, many trajectory spells have RangeEntry 1 (self)
    float bestDist = m_spellInfo->GetMaxRange(false) * 2;
    if (bestDist < 1.0f)
        bestDist = 300.0f;

    std::list<WorldObject*>::const_iterator itr = targets.begin();
    for (; itr != targets.end(); ++itr)
    {
        if (Unit* unitTarget = (*itr)->ToUnit())
            if (m_caster == *itr 
#ifdef LICH_KING
                || m_caster->IsOnVehicle(unitTarget) || (unitTarget)->GetVehicle())//(*itr)->IsOnVehicle(m_caster)
#endif
                )
                continue;

        const float size = std::max((*itr)->GetObjectSize() * 0.7f, 1.0f); // 1/sqrt(3)
                                                                           // TODO: all calculation should be based on src instead of m_caster
        const float objDist2d = fabs(m_targets.GetSrcPos()->GetExactDist2d(*itr) * cos(m_targets.GetSrcPos()->GetRelativeAngle(*itr)));
        const float dz = fabs((*itr)->GetPositionZ() - m_targets.GetSrcPos()->m_positionZ);

        //DEBUG_TRAJ(sLog->outError("Spell::SelectTrajTargets: check %u, dist between %f %f, height between %f %f.", (*itr)->GetEntry(), objDist2d - size, objDist2d + size, dz - size, dz + size);)

            float dist = objDist2d - size;
        float height = dist * (a * dist + b);
        //DEBUG_TRAJ(sLog->outError("Spell::SelectTrajTargets: dist %f, height %f.", dist, height);)
        if (dist < bestDist && height < dz + size && height > dz - size)
        {
            bestDist = dist > 0 ? dist : 0;
            break;
        }

#define CHECK_DIST {\
            /*DEBUG_TRAJ(TC_LOG_ERROR("spells", "Spell::SelectTrajTargets: dist %f, height %f.", dist, height);) */\
            if (dist > bestDist)\
                continue;\
            if (dist < objDist2d + size && dist > objDist2d - size)\
            {\
                bestDist = dist;\
                break;\
            }\
        }

        // RP-GG only, search in straight line, as item have no trajectory
        if (m_CastItem)
        {
            if (dist < bestDist && fabs(dz) < 6.0f) // closes target, also check Z difference)
            {
                bestDist = dist;
                break;
            }

            continue;
        }

        if (!a)
        {
            // Xinef: everything remade
            dist = m_targets.GetSrcPos()->GetExactDist(*itr);
            height = m_targets.GetSrcPos()->GetExactDist2d(*itr)*b;

            if (height < dz + size*(b + 1) && height > dz - size*(b + 1) && dist < bestDist)
            {
                bestDist = dist;
                break;
            }

            continue;
        }

        height = dz - size;
        float sqrt1 = b * b + 4 * a * height;
        if (sqrt1 > 0)
        {
            sqrt1 = sqrt(sqrt1);
            dist = (sqrt1 - b) / (2 * a);
            CHECK_DIST;
        }

        height = dz + size;
        float sqrt2 = b * b + 4 * a * height;
        if (sqrt2 > 0)
        {
            sqrt2 = sqrt(sqrt2);
            dist = (sqrt2 - b) / (2 * a);
            CHECK_DIST;

            dist = (-sqrt2 - b) / (2 * a);
            CHECK_DIST;
        }

        if (sqrt1 > 0)
        {
            dist = (-sqrt1 - b) / (2 * a);
            CHECK_DIST;
        }
    }

    if (m_targets.GetSrcPos()->GetExactDist2d(m_targets.GetDstPos()) > bestDist)
    {
        float x = m_targets.GetSrcPos()->m_positionX + cos(m_caster->GetOrientation()) * bestDist;
        float y = m_targets.GetSrcPos()->m_positionY + sin(m_caster->GetOrientation()) * bestDist;
        float z = m_targets.GetSrcPos()->m_positionZ + bestDist * (a * bestDist + b);

        if (itr != targets.end())
        {
            float distSq = (*itr)->GetExactDistSq(x, y, z);
            float sizeSq = (*itr)->GetObjectSize();
            sizeSq *= sizeSq;
            //DEBUG_TRAJ(sLog->outError("Initial %f %f %f %f %f", x, y, z, distSq, sizeSq);)
            if (distSq > sizeSq)
            {
                float factor = 1 - sqrt(sizeSq / distSq);
                x += factor * ((*itr)->GetPositionX() - x);
                y += factor * ((*itr)->GetPositionY() - y);
                z += factor * ((*itr)->GetPositionZ() - z);

                distSq = (*itr)->GetExactDistSq(x, y, z);
                //DEBUG_TRAJ(sLog->outError("Initial %f %f %f %f %f", x, y, z, distSq, sizeSq);)
            }
        }

        Position trajDst;
        trajDst.Relocate(x, y, z, m_caster->GetOrientation());
        SpellDestination dest(*m_targets.GetDst());
        dest.Relocate(trajDst);

        CallScriptDestinationTargetSelectHandlers(dest, effIndex, targetType);
        m_targets.ModDst(dest);
    }
}

void Spell::SelectEffectTypeImplicitTargets(uint8 effIndex)
{
    // special case for SPELL_EFFECT_SUMMON_RAF_FRIEND and SPELL_EFFECT_SUMMON_PLAYER
    // TODO: this is a workaround - target shouldn't be stored in target map for those spells
    switch (m_spellInfo->Effects[effIndex].Effect)
    {
#ifdef LICH_KING
    case SPELL_EFFECT_SUMMON_RAF_FRIEND:
#endif
    case SPELL_EFFECT_SUMMON_PLAYER:
        if (m_caster->GetTypeId() == TYPEID_PLAYER && m_caster->ToPlayer()->GetTarget())
        {
            WorldObject* target = ObjectAccessor::FindPlayer(m_caster->ToPlayer()->GetTarget());

            //todo spellscript CallScriptObjectTargetSelectHandlers(target, SpellEffIndex(effIndex), SpellImplicitTargetInfo());

            if (target && target->ToPlayer())
                AddUnitTarget(target->ToUnit(), 1 << effIndex, false);
        }
        return;
    default:
        break;
    }

    // select spell implicit targets based on effect type
    if (!m_spellInfo->Effects[effIndex].GetImplicitTargetType())
        return;

    uint32 targetMask = m_spellInfo->Effects[effIndex].GetMissingTargetMask();

    if (!targetMask)
        return;

    WorldObject* target = NULL;

    switch (m_spellInfo->Effects[effIndex].GetImplicitTargetType())
    {
        // add explicit object target or self to the target map
    case EFFECT_IMPLICIT_TARGET_EXPLICIT:
        // player which not released his spirit is Unit, but target flag for it is TARGET_FLAG_CORPSE_MASK
        if (targetMask & (TARGET_FLAG_UNIT_MASK | TARGET_FLAG_CORPSE_MASK))
        {
            if (Unit* unitTarget = m_targets.GetUnitTarget())
                target = unitTarget;
            else if (targetMask & TARGET_FLAG_CORPSE_MASK)
            {
                if (Corpse* corpseTarget = m_targets.GetCorpseTarget())
                {
                    // TODO: this is a workaround - corpses should be added to spell target map too, but we can't do that so we add owner instead
                    if (Player* owner = ObjectAccessor::FindPlayer(corpseTarget->GetOwnerGUID()))
                        target = owner;
                }
            }
            else //if (targetMask & TARGET_FLAG_UNIT_MASK)
                target = m_caster;
        }
        if (targetMask & TARGET_FLAG_ITEM_MASK)
        {
            if (Item* itemTarget = m_targets.GetItemTarget())
                AddItemTarget(itemTarget, 1 << effIndex);
            return;
        }
        if (targetMask & TARGET_FLAG_GAMEOBJECT_MASK)
            target = m_targets.GetGOTarget();
        break;
        // add self to the target map
    case EFFECT_IMPLICIT_TARGET_CASTER:
        if (targetMask & TARGET_FLAG_UNIT_MASK)
            target = m_caster;
        break;
    default:
        break;
    }

    //todo spellscript CallScriptObjectTargetSelectHandlers(target, SpellEffIndex(effIndex), SpellImplicitTargetInfo());

    if (target)
    {
        if (target->ToUnit())
            AddUnitTarget(target->ToUnit(), 1 << effIndex, false);
        else if (target->ToGameObject())
            AddGOTarget(target->ToGameObject(), 1 << effIndex);
    }
}

uint32 Spell::GetSearcherTypeMask(SpellTargetObjectTypes objType, ConditionList* condList)
{
    // this function selects which containers need to be searched for spell target
    uint32 retMask = GRID_MAP_TYPE_MASK_ALL;

    // filter searchers based on searched object type
    switch (objType)
    {
    case TARGET_OBJECT_TYPE_UNIT:
    case TARGET_OBJECT_TYPE_UNIT_AND_DEST:
    case TARGET_OBJECT_TYPE_CORPSE:
    case TARGET_OBJECT_TYPE_CORPSE_ENEMY:
    case TARGET_OBJECT_TYPE_CORPSE_ALLY:
        retMask &= GRID_MAP_TYPE_MASK_PLAYER | GRID_MAP_TYPE_MASK_CORPSE | GRID_MAP_TYPE_MASK_CREATURE;
        break;
    case TARGET_OBJECT_TYPE_GOBJ:
    case TARGET_OBJECT_TYPE_GOBJ_ITEM:
        retMask &= GRID_MAP_TYPE_MASK_GAMEOBJECT;
        break;
    default:
        break;
    }
    if (!m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_DEAD))
        retMask &= ~GRID_MAP_TYPE_MASK_CORPSE;
    if (m_spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_PLAYERS))
        retMask &= GRID_MAP_TYPE_MASK_CORPSE | GRID_MAP_TYPE_MASK_PLAYER;
    if (m_spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_GHOSTS))
        retMask &= GRID_MAP_TYPE_MASK_PLAYER;

    if (condList)
        retMask &= sConditionMgr->GetSearcherTypeMaskForConditionList(*condList);
    return retMask;
}

template<class SEARCHER>
void Spell::SearchTargets(SEARCHER& searcher, uint32 containerMask, Unit* referer, Position const* pos, float radius)
{
    if (!containerMask)
        return;

    // search world and grid for possible targets
    bool searchInGrid = containerMask & (GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_GAMEOBJECT);
    bool searchInWorld = containerMask & (GRID_MAP_TYPE_MASK_CREATURE | GRID_MAP_TYPE_MASK_PLAYER | GRID_MAP_TYPE_MASK_CORPSE);

    if (searchInGrid || searchInWorld)
    {
        float x, y;
        x = pos->GetPositionX();
        y = pos->GetPositionY();

        CellCoord p(Trinity::ComputeCellCoord(x, y));
        Cell cell(p);
        cell.SetNoCreate();

        Map& map = *(referer->GetMap());

        if (searchInWorld)
        {
            TypeContainerVisitor<SEARCHER, WorldTypeMapContainer> world_object_notifier(searcher);
            cell.Visit(p, world_object_notifier, map, radius + SPELL_SEARCHER_COMPENSATION, x, y);
        }
        if (searchInGrid)
        {
            TypeContainerVisitor<SEARCHER, GridTypeMapContainer >  grid_object_notifier(searcher);
            cell.Visit(p, grid_object_notifier, map, radius + SPELL_SEARCHER_COMPENSATION, x, y);
        }
    }
}

WorldObject* Spell::SearchNearbyTarget(float range, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionList* condList)
{
    WorldObject* target = NULL;
    uint32 containerTypeMask = GetSearcherTypeMask(objectType, condList);
    if (!containerTypeMask)
        return NULL;
    Trinity::WorldObjectSpellNearbyTargetCheck check(range, m_caster, m_spellInfo, selectionType, condList);
    Trinity::WorldObjectLastSearcher<Trinity::WorldObjectSpellNearbyTargetCheck> searcher(m_caster, target, check, containerTypeMask);
    SearchTargets<Trinity::WorldObjectLastSearcher<Trinity::WorldObjectSpellNearbyTargetCheck> >(searcher, containerTypeMask, m_caster, m_caster, range);
    return target;
}

void Spell::SearchAreaTargets(std::list<WorldObject*>& targets, float range, Position const* position, Unit* referer, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionList* condList)
{
    uint32 containerTypeMask = GetSearcherTypeMask(objectType, condList);
    if (!containerTypeMask)
        return;
    Trinity::WorldObjectSpellAreaTargetCheck check(range, position, m_caster, referer, m_spellInfo, selectionType, condList);
    Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellAreaTargetCheck> searcher(m_caster, targets, check, containerTypeMask);
    SearchTargets<Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellAreaTargetCheck> >(searcher, containerTypeMask, m_caster, position, range);
}

void Spell::SearchChainTargets(std::list<WorldObject*>& targets, uint32 chainTargets, WorldObject* target, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectType, SpellTargetSelectionCategories selectCategory, ConditionList* condList, bool isChainHeal)
{
    // max dist for jump target selection
    float jumpRadius = 0.0f;
    switch (m_spellInfo->DmgClass)
    {
    case SPELL_DAMAGE_CLASS_RANGED:
        // 7.5y for multi shot
        jumpRadius = 7.5f;
        break;
    case SPELL_DAMAGE_CLASS_MELEE:
        // 5y for swipe, cleave and similar
        jumpRadius = 5.0f;
        break;
    case SPELL_DAMAGE_CLASS_NONE:
    case SPELL_DAMAGE_CLASS_MAGIC:
        // 12.5y for chain heal spell since 3.2 patch
        if (isChainHeal)
            jumpRadius = 12.5f;
        // 10y as default for magic chain spells
        else
            jumpRadius = 10.0f;
        break;
    }

    // chain lightning/heal spells and similar - allow to jump at larger distance and go out of los
    bool isBouncingFar = (m_spellInfo->HasAttribute(SPELL_ATTR4_AREA_TARGET_CHAIN)
        || m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_NONE
        || m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC);

    // max dist which spell can reach
    float searchRadius = jumpRadius;
    if (isBouncingFar && !isChainHeal)
        searchRadius *= chainTargets;

    // Xinef: the distance should be increased by caster size, it is neglected in latter calculations
    std::list<WorldObject*> tempTargets;
    SearchAreaTargets(tempTargets, searchRadius, (m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MELEE ? m_caster : target), m_caster, objectType, selectType, condList);
    tempTargets.remove(target);

    // xinef: if we have select category nearby and checktype entry, select random of what we have, not by distance
    if (selectCategory == TARGET_SELECT_CATEGORY_NEARBY && selectType == TARGET_CHECK_ENTRY)
    {
        Trinity::Containers::RandomResizeList(tempTargets, chainTargets);
        targets = tempTargets;
        return;
    }

    // remove targets which are always invalid for chain spells
    // for some spells allow only chain targets in front of caster (swipe for example)
    if (!isBouncingFar)
    {
        float allowedArc = 0.0f;
        if (m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MELEE)
            allowedArc = (M_PI*7.0f) / 18.0f; // 70 degrees
        else if (m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_RANGED)
            allowedArc = M_PI*0.5f; // 90 degrees

        for (std::list<WorldObject*>::iterator itr = tempTargets.begin(); itr != tempTargets.end();)
        {
            std::list<WorldObject*>::iterator checkItr = itr++;
            if (!m_caster->HasInArc(static_cast<float>(M_PI), *checkItr))
                tempTargets.erase(checkItr);
            else if (allowedArc > 0.0f && !m_caster->HasInArc(allowedArc, *checkItr, (*checkItr)->GetObjectSize()))
                tempTargets.erase(checkItr);
        }
    }

    while (chainTargets)
    {
        // try to get unit for next chain jump
        std::list<WorldObject*>::iterator foundItr = tempTargets.end();
        // get unit with highest hp deficit in dist
        if (isChainHeal)
        {
            uint32 maxHPDeficit = 0;
            for (std::list<WorldObject*>::iterator itr = tempTargets.begin(); itr != tempTargets.end(); ++itr)
            {
                if (Unit* itrTarget = (*itr)->ToUnit())
                {
                    uint32 deficit = itrTarget->GetMaxHealth() - itrTarget->GetHealth();
                    // xinef: chain should not heal targets with max health
                    if (deficit == 0)
                        continue;

                    if ((deficit > maxHPDeficit || foundItr == tempTargets.end()) && target->IsWithinDist(itrTarget, jumpRadius) && target->IsWithinLOSInMap(itrTarget))
                    {
                        foundItr = itr;
                        maxHPDeficit = deficit;
                    }
                }
            }
        }
        // get closest object
        else
        {
            for (std::list<WorldObject*>::iterator itr = tempTargets.begin(); itr != tempTargets.end(); ++itr)
            {
                if (foundItr == tempTargets.end())
                {
                    if ((!isBouncingFar || target->IsWithinDist(*itr, jumpRadius)) && target->IsWithinLOSInMap(*itr))
                        foundItr = itr;
                }
                else if (target->GetDistanceOrder(*itr, *foundItr) && target->IsWithinLOSInMap(*itr))
                    foundItr = itr;
            }
        }
        // not found any valid target - chain ends
        if (foundItr == tempTargets.end())
            break;
        target = *foundItr;
        tempTargets.erase(foundItr);
        targets.push_back(target);
        --chainTargets;
    }
}


void Spell::prepareHitProcData(uint32& procAttacker, uint32& procVictim, bool hostileTarget)
{
    // Get data for type of attack and fill base info for trigger
    switch (m_spellInfo->DmgClass)
    {
        case SPELL_DAMAGE_CLASS_MELEE:
            procAttacker = PROC_FLAG_SUCCESSFUL_MELEE_SPELL_HIT;
            if (m_attackType == OFF_ATTACK)
                procAttacker |= PROC_FLAG_SUCCESSFUL_OFFHAND_HIT;

            procVictim   = PROC_FLAG_TAKEN_MELEE_SPELL_HIT;
            if (IsNextMeleeSwingSpell())
                procVictim   |= PROC_FLAG_TAKEN_MELEE_HIT;
            break;
        case SPELL_DAMAGE_CLASS_RANGED:
            procAttacker = PROC_FLAG_SUCCESSFUL_RANGED_SPELL_HIT;
            procVictim   = PROC_FLAG_TAKEN_RANGED_SPELL_HIT;
            break;
        default:
            if(m_spellInfo->IsPositive(hostileTarget))          // Check for positive spell
            {
                procAttacker = PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL;
                procVictim   = PROC_FLAG_TAKEN_POSITIVE_SPELL;
            }
            else if(m_spellInfo->Id == 5019) // Wands
            {
                procAttacker = PROC_FLAG_SUCCESSFUL_RANGED_SPELL_HIT;
                procVictim   = PROC_FLAG_TAKEN_RANGED_SPELL_HIT;
            }
            else
            {
                procAttacker = PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT;
                procVictim   = PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT;
            }
            break;
    }
}

void Spell::prepareDataForTriggerSystem()
{
    //==========================================================================================
    // Now fill data for trigger system, need know:
    // can spell trigger another or not ( m_canTrigger )
    // Create base triggers flags for Attacker and Victim ( m_procAttacker and  m_procVictim)
    //==========================================================================================

    // Fill flag can spell trigger or not
    if (!m_IsTriggeredSpell)
        m_canTrigger = true;          // Normal cast - can trigger
    else if (!m_triggeredByAuraSpell)
        m_canTrigger = true;          // Triggered from SPELL_EFFECT_TRIGGER_SPELL - can trigger
    else                              // Exceptions (some periodic triggers)
        m_canTrigger = false;         // Triggered spells can`t trigger another
    // Do not trigger from item cast spell
    if (m_CastItem)
       m_canTrigger = false;

    if(m_spellInfo->HasAttribute(SPELL_ATTR3_CANT_TRIGGER_PROC))
        m_canTrigger = false;

    if(m_spellInfo->HasAttribute(SPELL_ATTR2_TRIGGERED_CAN_TRIGGER_PROC))
        m_canTrigger = true;

    //some Exceptions
    switch (m_spellInfo->SpellFamilyName)
    {
        case SPELLFAMILY_MAGE:    // Arcane Missles / Blizzard / Molten Armor triggers
            if (m_spellInfo->SpellFamilyFlags & 0x0000000800240080LL) m_canTrigger = true;
        break;
        case SPELLFAMILY_WARLOCK: // For Hellfire Effect / Seed of Corruption triggers 
            if (m_spellInfo->SpellFamilyFlags & 0x0000808000000000LL) m_canTrigger = true;
        break;
        case SPELLFAMILY_HUNTER:  // Hunter Explosive Trap Effect/Immolation Trap Effect/Frost Trap Aura/Snake Trap Effect
            if (m_spellInfo->SpellFamilyFlags & 0x0000200000000014LL) m_canTrigger = true;
        break;
        case SPELLFAMILY_PALADIN: // For Holy Shock + Seal of Command & Seal of Blood
            if (m_spellInfo->SpellFamilyFlags & 0x0001040002200000LL) m_canTrigger = true;
        break;
        case SPELLFAMILY_ROGUE: // mutilate mainhand + offhand
            if (m_spellInfo->SpellFamilyFlags & 0x600000000LL) m_canTrigger = true;
        break;
        case SPELLFAMILY_SHAMAN: 
            if (m_spellInfo->SpellFamilyFlags & 0x800003LL   ) m_canTrigger = true; //Lightning bolt + Chain Lightning (needed for Lightning Overload) + Windfurry (not totem)
        break;
    }
}

void Spell::CleanupTargetList()
{
    m_UniqueTargetInfo.clear();
    m_UniqueGOTargetInfo.clear();
    m_UniqueItemInfo.clear();
    m_delayMoment = 0;
    m_delayTrajectory = 0;
}

void Spell::AddUnitTarget(Unit* target, uint32 effectMask, bool checkIfValid /*= true*/, bool implicit /*= true*/)
{
    for (uint32 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
        if (!m_spellInfo->Effects[effIndex].IsEffect() || !CheckEffectTarget(target, effIndex))
            effectMask &= ~(1 << effIndex);

    // no effects left
    if (!effectMask)
        return;

    if (checkIfValid)
    {
        SpellCastResult res = m_spellInfo->CheckTarget(m_caster, target, implicit);
        if (res != SPELL_CAST_OK)
            return;
    }

    // Check for effect immune skip if immuned
    for (uint32 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
        if (target->IsImmunedToSpellEffect(m_spellInfo, effIndex))
            effectMask &= ~(1 << effIndex);

    uint64 targetGUID = target->GetGUID();

    // Lookup target in already in list
    for (std::list<TargetInfo>::iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
    {
        if (targetGUID == ihit->targetGUID)             // Found in list
        {
            ihit->effectMask |= effectMask;             // Immune effects removed from mask
            ihit->scaleAura = false;
            if (m_auraScaleMask && ihit->effectMask == m_auraScaleMask && m_caster != target)
            {
                SpellInfo const* auraSpell = m_spellInfo->GetFirstRankSpell();
                if (uint32(target->GetLevel() + 10) >= auraSpell->SpellLevel)
                    ihit->scaleAura = true;
            }
            return;
        }
    }

    // This is new target calculate data for him

    // Get spell hit result on target
    TargetInfo targetInfo;
    targetInfo.targetGUID = targetGUID;                         // Store target GUID
    targetInfo.effectMask = effectMask;                         // Store all effects not immune
    targetInfo.processed = false;                              // Effects not apply on target
    targetInfo.alive = target->IsAlive();
    targetInfo.damage = 0;
    targetInfo.crit = false;
    targetInfo.scaleAura = false;
    if (m_auraScaleMask && targetInfo.effectMask == m_auraScaleMask && m_caster != target)
    {
        SpellInfo const* auraSpell = m_spellInfo->GetFirstRankSpell();
        if (uint32(target->GetLevel() + 10) >= auraSpell->SpellLevel)
            targetInfo.scaleAura = true;
    }

    // Calculate hit result
    if (m_originalCaster)
    {
        targetInfo.missCondition = m_originalCaster->SpellHitResult(target, m_spellInfo, m_canReflect);
        if (m_skipHitCheck && targetInfo.missCondition != SPELL_MISS_IMMUNE)
            targetInfo.missCondition = SPELL_MISS_NONE;
    }
    else
        targetInfo.missCondition = SPELL_MISS_EVADE; //SPELL_MISS_NONE;

                                                     // Spell have speed - need calculate incoming time
                                                     // Incoming time is zero for self casts. At least I think so.
    if (m_spellInfo->Speed > 0.0f && m_caster != target)
    {
        // calculate spell incoming interval
        // TODO: this is a hack
        float dist = m_caster->GetDistance(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());

        if (dist < 5.0f)
            dist = 5.0f;
        targetInfo.timeDelay = (uint64)floor(dist / m_spellInfo->Speed * 1000.0f);

        // Calculate minimum incoming time
        if (m_delayMoment == 0 || m_delayMoment > targetInfo.timeDelay)
            m_delayMoment = targetInfo.timeDelay;
    }
    else
        targetInfo.timeDelay = 0LL;

    // If target reflect spell back to caster
    if (targetInfo.missCondition == SPELL_MISS_REFLECT)
    {
        // Calculate reflected spell result on caster
        targetInfo.reflectResult = m_caster->SpellHitResult(m_caster, m_spellInfo, m_canReflect);

        if (targetInfo.reflectResult == SPELL_MISS_REFLECT)     // Impossible reflect again, so simply deflect spell
            targetInfo.reflectResult = SPELL_MISS_PARRY;

        // Increase time interval for reflected spells by 1.5
        m_caster->m_Events.AddEvent(new ReflectEvent(m_caster->GetGUID(), targetInfo.targetGUID, m_spellInfo), m_caster->m_Events.CalculateTime(targetInfo.timeDelay));
        targetInfo.timeDelay += targetInfo.timeDelay >> 1;
    }
    else
        targetInfo.reflectResult = SPELL_MISS_NONE;

    // Add target to list
    m_UniqueTargetInfo.push_back(targetInfo);
}

void Spell::AddGOTarget(GameObject* go, uint32 effectMask)
{
    for (uint32 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
    {
        if (!m_spellInfo->Effects[effIndex].IsEffect())
            effectMask &= ~(1 << effIndex);
#ifdef LICH_KING
        else
        {
            switch (m_spellInfo->Effects[effIndex].Effect)
            {
            case SPELL_EFFECT_GAMEOBJECT_DAMAGE:
            case SPELL_EFFECT_GAMEOBJECT_REPAIR:
            case SPELL_EFFECT_GAMEOBJECT_SET_DESTRUCTION_STATE:
                if (go->GetGoType() != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
                    effectMask &= ~(1 << effIndex);
                break;
            default:
                break;
            }
        }
#endif
    }

    if (!effectMask)
        return;

    uint64 targetGUID = go->GetGUID();

    // Lookup target in already in list
    for (std::list<GOTargetInfo>::iterator ihit = m_UniqueGOTargetInfo.begin(); ihit != m_UniqueGOTargetInfo.end(); ++ihit)
    {
        if (targetGUID == ihit->targetGUID)                 // Found in list
        {
            ihit->effectMask |= effectMask;                 // Add only effect mask
            return;
        }
    }

    // This is new target calculate data for him

    GOTargetInfo target;
    target.targetGUID = targetGUID;
    target.effectMask = effectMask;
    target.processed = false;                              // Effects not apply on target

                                                           // Spell have speed - need calculate incoming time
    if (m_spellInfo->Speed > 0.0f)
    {
        // calculate spell incoming interval
        float dist = m_caster->GetDistance(go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
        if (dist < 5.0f)
            dist = 5.0f;
        target.timeDelay = uint64(floor(dist / m_spellInfo->Speed * 1000.0f));
        if (m_delayMoment == 0 || m_delayMoment > target.timeDelay)
            m_delayMoment = target.timeDelay;
    }
    else
        target.timeDelay = 0LL;

    // Add target to list
    m_UniqueGOTargetInfo.push_back(target);
}

void Spell::AddItemTarget(Item* item, uint32 effectMask)
{
    for (uint32 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
        if (!m_spellInfo->Effects[effIndex].IsEffect())
            effectMask &= ~(1 << effIndex);

    // no effects left
    if (!effectMask)
        return;

    // Lookup target in already in list
    for (std::list<ItemTargetInfo>::iterator ihit = m_UniqueItemInfo.begin(); ihit != m_UniqueItemInfo.end(); ++ihit)
    {
        if (item == ihit->item)                            // Found in list
        {
            ihit->effectMask |= effectMask;                 // Add only effect mask
            return;
        }
    }

    // This is new target add data

    ItemTargetInfo target;
    target.item = item;
    target.effectMask = effectMask;

    m_UniqueItemInfo.push_back(target);
}

void Spell::AddDestTarget(SpellDestination const& dest, uint32 effIndex)
{
    m_destTargets[effIndex] = dest;
}

void Spell::DoAllEffectOnTarget(TargetInfo *target)
{
    //TC_LOG_DEBUG("FIXME","Spell %u -DoAllEffectOnTarget(...)", m_spellInfo->Id);

    if (target->processed)                                  // Check target
        return;

    target->processed = true;                               // Target checked in apply effects procedure

    // Get mask of effects for target
    uint32 mask = target->effectMask;
    if (mask == 0)                                          // No effects
        return;

    Unit* unit = m_caster->GetGUID()==target->targetGUID ? m_caster : ObjectAccessor::GetObjectInWorld(target->targetGUID,(Unit*)NULL);
    if (!unit)
    {
        uint8 farMask = 0;
        // create far target mask
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (m_spellInfo->Effects[i].IsFarUnitTargetEffect())
                if ((1 << i) & mask)
                    farMask |= (1 << i);

        if (!farMask)
            return;
        // find unit in world
        // Xinef: FindUnit Access without Map check!!! Intended
        unit = ObjectAccessor::FindUnit(target->targetGUID);
        if (!unit)
            return;

        // do far effects on the unit
        // can't use default call because of threading, do stuff as fast as possible
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (farMask & (1 << i))
                HandleEffects(unit, NULL, NULL, i, SPELL_EFFECT_HANDLE_HIT_TARGET);

        return;
    }

    // Get original caster (if exist) and calculate damage/healing from him
    Unit *caster = m_originalCasterGUID ? m_originalCaster : m_caster;

    // Skip if m_originalCaster not avaiable
    if (!caster)
        return;

    SpellMissInfo missInfo = target->missCondition;

    // Need init unitTarget by default unit (can changed in code on reflect)
    // Or on missInfo!=SPELL_MISS_NONE unitTarget undefined (but need in trigger subsystem)
    unitTarget = unit;
    bool hostileTarget = caster->IsHostileTo(unitTarget);

    // Reset damage/healing counter
    m_damage = target->damage;
    m_healing = -target->damage;

    // Fill base trigger info
    uint32 procAttacker = 0;
    uint32 procVictim = 0;
    prepareHitProcData(procAttacker,procVictim,hostileTarget);
    uint32 procEx = m_triggeredByAuraSpell? PROC_EX_INTERNAL_TRIGGERED : PROC_EX_NONE;

                            //Spells with this flag cannot trigger if effect is casted on self
                            // Slice and Dice, relentless strikes, eviscerate
    //bool canEffectTrigger = (m_spellInfo->AttributesEx4 & (SPELL_ATTR4_CANT_PROC_FROM_SELFCAST | SPELL_ATTR4_UNK4) ? m_caster!=unitTarget : true) 
    //    && m_canTrigger;

    if (missInfo==SPELL_MISS_NONE)                          // In case spell hit target, do all effect on that target
        DoSpellHitOnUnit(unit, mask);
    else if (missInfo == SPELL_MISS_REFLECT)                // In case spell reflect from target, do all effect on caster (if hit)
    {  
        if (m_removeReflect) {
            unit->RemoveAurasDueToSpell(23920);
            m_removeReflect = false;
        }

        if (target->reflectResult == SPELL_MISS_NONE)       // If reflected spell hit caster -> do all effect on him
            DoSpellHitOnUnit(m_caster, mask);
    }
    /*else //TODO: This is a hack. need fix
    {
        uint32 tempMask = 0;
        for(uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if(m_spellInfo->Effects[i].Effect == SPELL_EFFECT_DUMMY
                || m_spellInfo->Effects[i].Effect == SPELL_EFFECT_TRIGGER_SPELL)
                tempMask |= 1<<i;
        if(tempMask &= mask)
            DoSpellHitOnUnit(unit, tempMask);
    }*/

    // All calculated do it!
    // Do healing and triggers
    if (m_healing > 0)
    {
        bool crit = caster->IsSpellCrit(NULL, m_spellInfo, m_spellSchoolMask);
        uint32 addhealth = m_healing;
        if (crit)
        {
            procEx |= PROC_EX_CRITICAL_HIT;
            addhealth = caster->SpellCriticalBonus(m_spellInfo, addhealth, NULL);
        }
        else
            procEx |= PROC_EX_NORMAL_HIT;

        caster->SendHealSpellLog(unitTarget, m_spellInfo->Id, addhealth, crit);

        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (missInfo != SPELL_MISS_REFLECT)
            caster->ProcDamageAndSpell(unitTarget, procAttacker, procVictim, procEx, addhealth, m_attackType, m_spellInfo, m_canTrigger);
            
        if (unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->ToCreature()->IsAIEnabled) {
            unitTarget->ToCreature()->AI()->HealReceived(caster, addhealth);
        }
        if (caster->GetTypeId() == TYPEID_UNIT && caster->ToCreature()->IsAIEnabled)
            caster->ToCreature()->AI()->HealDone(unitTarget, addhealth);

        int32 gain = unitTarget->ModifyHealth( int32(addhealth) );

        float threat = float(gain) * 0.5f * sSpellMgr->GetSpellThreatModPercent(m_spellInfo);

        Unit* threatTarget = (GetSpellInfo()->HasAttribute(SPELL_ATTR_CU_THREAT_GOES_TO_CURRENT_CASTER) || !m_originalCaster)? m_caster : m_originalCaster;
        unitTarget->GetHostileRefManager().threatAssist(threatTarget, threat, m_spellInfo);

        if(caster->GetTypeId()==TYPEID_PLAYER)
        {
            if(Battleground *bg = (caster->ToPlayer())->GetBattleground())
            {
                bg->UpdatePlayerScore((caster->ToPlayer()), SCORE_HEALING_DONE, gain);
                if (unitTarget->GetTypeId()==TYPEID_PLAYER)
                    bg->UpdatePlayerScore(unitTarget->ToPlayer(), SCORE_HEALING_TAKEN, gain);
            }
        }
    }
    // Do damage and triggers
    else if (m_damage > 0)
    {
        // Fill base damage struct (unitTarget - is real spell target)
        SpellNonMeleeDamage damageInfo(caster, unitTarget, m_spellInfo->Id, m_spellSchoolMask);

        // Add bonuses and fill damageInfo struct
        caster->CalculateSpellDamageTaken(&damageInfo, m_damage, m_spellInfo, m_attackType,  target->crit);

        // Send log damage message to client
        caster->SendSpellNonMeleeDamageLog(&damageInfo);

        procEx = createProcExtendMask(&damageInfo, missInfo);
        procVictim |= PROC_FLAG_TAKEN_ANY_DAMAGE;

        
        caster->DealSpellDamage(&damageInfo, true);
        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (missInfo != SPELL_MISS_REFLECT)
        {
            caster->ProcDamageAndSpell(unitTarget, procAttacker, procVictim, procEx, damageInfo.damage, m_attackType, m_spellInfo, m_canTrigger);
            if(caster->GetTypeId() == TYPEID_PLAYER)
                (caster->ToPlayer())->CastItemCombatSpell(unitTarget, m_attackType, procVictim, procEx, m_spellInfo);
        }

        // Shadow Word: Death - deals damage equal to damage done to caster if victim is not killed
        if (m_spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && m_spellInfo->SpellFamilyFlags&0x0000000200000000LL &&
            caster != unitTarget && unitTarget->IsAlive())
        {
            // Redirect damage to caster if victim alive
            m_caster->CastCustomSpell(m_caster, 32409, nullptr, nullptr, nullptr, true);
            if (m_caster->ToPlayer())
                m_caster->ToPlayer()->m_swdBackfireDmg = m_damage;
            //breakcompile;   // Build damage packet directly here and fake spell damage
            //caster->DealDamage(caster, uint32(m_damage), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_NORMAL, NULL, false);
        }
        // Judgement of Blood
        else if (m_spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && m_spellInfo->SpellFamilyFlags & 0x0000000800000000LL && m_spellInfo->SpellIconID==153)
        {
            int32 damagePoint  = damageInfo.damage * 33 / 100;
            m_caster->CastCustomSpell(m_caster, 32220, &damagePoint, NULL, NULL, true);
        }
        // Bloodthirst
        else if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR && m_spellInfo->SpellFamilyFlags & 0x40000000000LL)
        {
            uint32 BTAura = 0;
            switch(m_spellInfo->Id)
            {
                case 23881: BTAura = 23885; break;
                case 23892: BTAura = 23886; break;
                case 23893: BTAura = 23887; break;
                case 23894: BTAura = 23888; break;
                case 25251: BTAura = 25252; break;
                case 30335: BTAura = 30339; break;
                default:
                    TC_LOG_ERROR("FIXME","Spell::EffectSchoolDMG: Spell %u not handled in BTAura",m_spellInfo->Id);
                    break;
            }
            if (BTAura)
                m_caster->CastSpell(m_caster,BTAura,true);
        }
    }
    // Passive spell hits/misses or active spells only misses (only triggers)
    else
    {
        // Fill base damage struct (unitTarget - is real spell target)
        SpellNonMeleeDamage damageInfo(caster, unitTarget, m_spellInfo->Id, m_spellSchoolMask);
        procEx = createProcExtendMask(&damageInfo, missInfo);
        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (missInfo != SPELL_MISS_REFLECT)
            caster->ProcDamageAndSpell(unit, procAttacker, procVictim, procEx, 0, m_attackType, m_spellInfo, m_canTrigger);
    }

    // Call scripted function for AI if this spell is casted upon a creature (except pets)
    if(IS_CREATURE_GUID(target->targetGUID))
    {
        // cast at creature (or GO) quest objectives update at successful cast finished (+channel finished)
        // ignore autorepeat/melee casts for speed (not exist quest for spells (hm... )
        if( m_caster->GetTypeId() == TYPEID_PLAYER && !IsAutoRepeat() && !IsNextMeleeSwingSpell() && !IsChannelActive() )
            (m_caster->ToPlayer())->CastedCreatureOrGO(unit->GetEntry(),unit->GetGUID(),m_spellInfo->Id);
    }
    
    //set target in combat with caster
    if( missInfo != SPELL_MISS_EVADE && !m_caster->IsFriendlyTo(unit) && !m_spellInfo->IsPositive(hostileTarget) && m_caster->GetEntry() != WORLD_TRIGGER)
    {
        if(m_spellInfo->HasInitialAggro())
        {
            m_caster->CombatStart(unit,!m_IsTriggeredSpell); //A triggered spell should not be considered as a pvp action
        }
        else if(m_spellInfo->HasAttribute(SPELL_ATTR_CU_AURA_CC))
        {
            if(!unit->IsStandState())
                unit->SetStandState(PLAYER_STATE_NONE);
        }
        else if (m_spellInfo->HasAttribute(SPELL_ATTR_CU_PUT_ONLY_CASTER_IN_COMBAT))
        {
            m_caster->SetInCombatState(true);
        }
    }
    
    // if target is flagged for pvp also flag caster
    if(unit->IsPvP() && !m_IsTriggeredSpell)
    {
        if (m_caster->GetTypeId() == TYPEID_PLAYER)
            (m_caster->ToPlayer())->UpdatePvP(true);
    }
}

bool Spell::UpdateChanneledTargetList()
{
    // Not need check return true
    if (m_channelTargetEffectMask == 0)
        return true;

    uint8 channelTargetEffectMask = m_channelTargetEffectMask;
    uint8 channelAuraMask = 0;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (m_spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA)
            channelAuraMask |= 1 << i;

    channelAuraMask &= channelTargetEffectMask;

    float range = 0;
    if (channelAuraMask)
    {
        range = m_spellInfo->GetMaxRange(m_spellInfo->IsPositive());
        if (range == 0)
            for (int i = EFFECT_0; i <= EFFECT_2; ++i)
                if (channelAuraMask & (1 << i) && m_spellInfo->Effects[i].RadiusEntry)
                {
                    range = m_spellInfo->Effects[i].CalcRadius(NULL, NULL);
                    break;
                }

        if (Player* modOwner = m_caster->GetSpellModOwner())
            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RANGE, range, this);

        // xinef: add little tolerance level
        range += std::min(3.0f, range*0.1f); // 10% but no more than 3yd
    }

    for (std::list<TargetInfo>::iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
    {
        if (ihit->missCondition == SPELL_MISS_NONE && (channelTargetEffectMask & ihit->effectMask))
        {
            Unit* unit = m_caster->GetGUID() == ihit->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID);

            if (!unit)
                continue;

            if (IsValidDeadOrAliveTarget(unit))
            {
                if (channelAuraMask & ihit->effectMask)
                {
                    if (auto* aurApp = unit->GetAuraApplication(m_spellInfo->Id, m_originalCasterGUID))
                    {
                        if (m_caster != unit)
                        {
                            if (!m_caster->IsWithinDistInMap(unit, range))
                            {
                                ihit->effectMask &= ~aurApp->GetEffectMask();
                                //sunwell unit->RemoveAura(aurApp);
                                unit->RemoveAura(aurApp->GetId(), aurApp->GetEffIndex());
                                continue;
                            }
                            // Xinef: Update Orientation server side (non players wont sent appropriate packets)
                            else if (m_spellInfo->HasAttribute(SPELL_ATTR1_CHANNEL_TRACK_TARGET))
                                m_caster->UpdateOrientation(m_caster->GetAngle(unit));
                        }
                    }
                    else // aura is dispelled
                        continue;
                }

                channelTargetEffectMask &= ~ihit->effectMask;   // remove from need alive mask effect that have alive target
            }
        }
    }

    // Xinef: not all effects are covered, remove applications from all targets
    if (channelTargetEffectMask != 0)
    {
        for (std::list<TargetInfo>::iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
            if (ihit->missCondition == SPELL_MISS_NONE && (channelAuraMask & ihit->effectMask))
                if (Unit* unit = m_caster->GetGUID() == ihit->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID))
                    if (IsValidDeadOrAliveTarget(unit))
                        if (auto* aurApp = unit->GetAuraApplication(m_spellInfo->Id, m_originalCasterGUID))
                        {
                            ihit->effectMask &= ~aurApp->GetEffectMask();
                            //sunwell unit->RemoveAura(aurApp);
                            unit->RemoveAura(aurApp->GetId(), aurApp->GetEffIndex());
                        }
    }

    // is all effects from m_needAliveTargetMask have alive targets
    return channelTargetEffectMask == 0;
}

void Spell::DoSpellHitOnUnit(Unit *unit, const uint32 effectMask)
{
    if(!unit || !effectMask)
        return;

    Unit *caster = m_originalCasterGUID ? m_originalCaster : m_caster;

    // Recheck immune (only for delayed spells)
    if(    m_spellInfo->Speed
        && !(m_spellInfo->Attributes & SPELL_ATTR0_UNAFFECTED_BY_INVULNERABILITY)
        && !(m_spellInfo->HasAttribute(SPELL_ATTR1_UNAFFECTED_BY_SCHOOL_IMMUNE))
        && (   unit->IsImmunedToDamage(m_spellInfo->GetSchoolMask(),true)
            || unit->IsImmunedToSpell(m_spellInfo,true) )
      )
    {
        caster->SendSpellMiss(unit, m_spellInfo->Id, SPELL_MISS_IMMUNE);
        m_damage = 0;
        return;
    }

    if( caster != unit )
    {
        if (unit->GetCharmerOrOwnerGUID() != caster->GetGUID())
        {
            // Recheck UNIT_FLAG_NON_ATTACKABLE and evade for delayed spells
            if (m_spellInfo->Speed > 0.0f)
            {
                if (unit->GetTypeId() == TYPEID_UNIT && unit->ToCreature()->IsInEvadeMode())
                {
                    caster->SendSpellMiss(unit, m_spellInfo->Id, SPELL_MISS_EVADE);
                    m_damage = 0;
                    return;
                }

                if (unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                {
                    bool nearbyEntrySpell = false;
                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; i++)
                    {
                        if (sSpellMgr->IsNearbyEntryEffect(m_spellInfo, i))
                        {
                            nearbyEntrySpell = true;
                            break;
                        }
                    }

                    if (!nearbyEntrySpell)
                    {
                        caster->SendSpellMiss(unit, m_spellInfo->Id, SPELL_MISS_EVADE);
                        m_damage = 0;
                        return;
                    }
                }
            }
        }
        if( !caster->IsFriendlyTo(unit) )
        {
            // reset damage to 0 if target has Invisibility or Vanish aura (_only_ vanish, not stealth) and isn't visible for caster
            bool isVisibleForHit = ( (unit->HasAuraType(SPELL_AURA_MOD_INVISIBILITY) || unit->HasAuraTypeWithFamilyFlags(SPELL_AURA_MOD_STEALTH, SPELLFAMILY_ROGUE ,SPELLFAMILYFLAG_ROGUE_VANISH)) && !unit->IsVisibleForOrDetect(caster, true)) ? false : true;
            
            // for delayed spells ignore not visible explicit target
            if(m_spellInfo->Speed > 0.0f && unit==m_targets.GetUnitTarget() && !isVisibleForHit)
            {
                // that was causing CombatLog errors
                //caster->SendSpellMiss(unit, m_spellInfo->Id, SPELL_MISS_EVADE);
                m_damage = 0;
                return;
            }
            unit->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_HITBYSPELL);

            if(m_spellInfo->HasAttribute(SPELL_ATTR_CU_AURA_CC))
                unit->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CAST);
        }
        else
        {
            // for delayed spells ignore negative spells (after duel end) for friendly targets
            // TODO: this cause soul transfer bugged
            if(m_spellInfo->Speed > 0.0f && unit->GetTypeId() == TYPEID_PLAYER && !m_spellInfo->IsPositive() && m_spellInfo->Id != 45034) // FIXME: Hack for Boundless Agony (Kalecgos)
            {
                caster->SendSpellMiss(unit, m_spellInfo->Id, SPELL_MISS_EVADE);
                m_damage = 0;
                return;
            }

            // assisting case, healing and resurrection
            if(unit->HasUnitState(UNIT_STATE_ATTACK_PLAYER))
            {
                caster->SetContestedPvP();
                //caster->UpdatePvP(true);
            }
            if( unit->IsInCombat() && m_spellInfo->HasInitialAggro())
            {
                //threat to current caster instead of original caster
                m_caster->SetInCombatState(unit->GetCombatTimer() > 0, unit);
                unit->GetHostileRefManager().threatAssist(m_caster, 0.0f);
            }
        }
    }

    // Get Data Needed for Diminishing Returns, some effects may have multiple auras, so this must be done on spell hit, not aura add
    if((m_diminishGroup = GetDiminishingReturnsGroupForSpell(m_spellInfo, m_triggeredByAuraSpell)))
    {
        m_diminishLevel = unit->GetDiminishing(m_diminishGroup);
        // send immunity message if target is immune
        if(m_diminishLevel == DIMINISHING_LEVEL_IMMUNE)
        {
            caster->SendSpellMiss(unitTarget, m_spellInfo->Id, SPELL_MISS_IMMUNE);
            return;
        }

        DiminishingReturnsType type = GetDiminishingReturnsGroupType(m_diminishGroup);
        // Increase Diminishing on unit, current informations for actually casts will use values above
        if((type == DRTYPE_PLAYER && (unit->GetTypeId() == TYPEID_PLAYER || (unit->ToCreature())->IsPet() || (unit->ToCreature())->IsPossessedByPlayer())) || type == DRTYPE_ALL)
            unit->IncrDiminishing(m_diminishGroup);
    }

    int8 sanct_effect = -1;
    for(uint32 effectNumber = 0;effectNumber < MAX_SPELL_EFFECTS;effectNumber++)
    {
        // handle sanctuary effects after aura apply!
        if (m_spellInfo->Effects[effectNumber].Effect == SPELL_EFFECT_SANCTUARY)
        {
            sanct_effect = effectNumber;
            continue;
        }

        if (effectMask & (1<<effectNumber))
            HandleEffects(unit,NULL,NULL,effectNumber, SPELL_EFFECT_HANDLE_HIT_TARGET);
    }

    if (sanct_effect >= 0 && (effectMask & (1 << sanct_effect)))
        HandleEffects(unit, NULL, NULL, sanct_effect, SPELL_EFFECT_HANDLE_HIT_TARGET);

    if(unit->GetTypeId() == TYPEID_UNIT && (unit->ToCreature())->IsAIEnabled) 
    {
        (unit->ToCreature())->AI()->SpellHit(caster, m_spellInfo);
    }

    if(caster->GetTypeId() == TYPEID_UNIT && (caster->ToCreature())->IsAIEnabled)
        (caster->ToCreature())->AI()->SpellHitTarget(unit, m_spellInfo);

    // trigger only for first effect targets
    if (m_ChanceTriggerSpells.size() && (effectMask & 0x1))
    {
        int _duration=0;
        for(ChanceTriggerSpells::const_iterator i = m_ChanceTriggerSpells.begin(); i != m_ChanceTriggerSpells.end(); ++i)
        {
            if(roll_chance_i(i->second))
            {
                caster->CastSpell(unit, i->first, true, NULL, NULL, 0, true);
                // SPELL_AURA_ADD_TARGET_TRIGGER auras shouldn't trigger auras without duration
                // set duration equal to triggering spell
                if (i->first->GetDuration() == -1)
                {
                    // get duration from aura-only once
                    if (!_duration)
                    {
                        Aura * aur = unit->GetAuraByCasterSpell(m_spellInfo->Id, caster->GetGUID());
                        _duration = aur ? aur->GetAuraDuration() : -1;
                    }
                    unit->SetAurasDurationByCasterSpell(i->first->Id, caster->GetGUID(), _duration);
                }
            }
        }
    }

    // Sunwell Twins hack
    if (m_spellInfo->Id == 45246) 
    { //"Burn"
        unit->RemoveAurasDueToSpell(45347); // "Dark Touched"
        unit->AddAura(45348, unit); // "Flame Touched"
    }

    if(m_spellInfo->HasAttribute(SPELL_ATTR_CU_LINK_HIT))
    {
        if(const std::vector<int32> *spell_triggered = sSpellMgr->GetSpellLinked(m_spellInfo->Id + SPELL_LINK_HIT)) 
        {
            for(std::vector<int32>::const_iterator i = spell_triggered->begin(); i != spell_triggered->end(); ++i) 
            {
                if(*i < 0)
                    unit->RemoveAurasDueToSpell(-(*i));
                else
                    unit->CastSpell(unit, *i, true, 0, 0, caster->GetGUID());
            }
        }
    }
}

void Spell::DoAllEffectOnTarget(GOTargetInfo *target)
{
    if (target->processed)                                  // Check target
        return;
    target->processed = true;                               // Target checked in apply effects procedure

    uint32 effectMask = target->effectMask;
    if(!effectMask)
        return;

    GameObject* go = ObjectAccessor::GetGameObject(*m_caster, target->targetGUID);
    if(!go)
        return;

    PrepareScriptHitHandlers();
    CallScriptBeforeHitHandlers();

    for(uint32 effectNumber = 0; effectNumber < MAX_SPELL_EFFECTS; effectNumber++)
        if (effectMask & (1 << effectNumber))
            HandleEffects(NULL,NULL,go,effectNumber, SPELL_EFFECT_HANDLE_HIT_TARGET);

    // cast at creature (or GO) quest objectives update at successful cast finished (+channel finished)
    // ignore autorepeat/melee casts for speed (not exist quest for spells (hm... )
    if( m_caster->GetTypeId() == TYPEID_PLAYER && !IsAutoRepeat() && !IsNextMeleeSwingSpell() && !IsChannelActive() )
        (m_caster->ToPlayer())->CastedCreatureOrGO(go->GetEntry(),go->GetGUID(),m_spellInfo->Id);

    CallScriptOnHitHandlers();
    CallScriptAfterHitHandlers();
}

void Spell::DoAllEffectOnTarget(ItemTargetInfo *target)
{
    uint32 effectMask = target->effectMask;
    if(!target->item || !effectMask)
        return;

    PrepareScriptHitHandlers();
    CallScriptBeforeHitHandlers();

    for (uint32 effectNumber = 0; effectNumber < MAX_SPELL_EFFECTS; ++effectNumber)
        if (effectMask & (1 << effectNumber))
            HandleEffects(NULL, target->item, NULL, effectNumber, SPELL_EFFECT_HANDLE_HIT_TARGET);

    CallScriptOnHitHandlers();
    CallScriptAfterHitHandlers();
}

/*
// Helper for Chain Healing
// Spell target first
// Raidmates then descending by injury suffered (MaxHealth - Health)
// Other players/mobs then descending by injury suffered (MaxHealth - Health)
struct ChainHealingOrder : public std::binary_function<const Unit*, const Unit*, bool>
{
    const Unit* MainTarget;
    ChainHealingOrder(Unit const* Target) : MainTarget(Target) {};
    // functor for operator ">"
    bool operator()(Unit const* _Left, Unit const* _Right) const
    {
        return (ChainHealingHash(_Left) < ChainHealingHash(_Right));
    }

    int32 ChainHealingHash(Unit const* Target) const
    {
        if (Target->GetTypeId() == TYPEID_PLAYER && MainTarget->GetTypeId() == TYPEID_PLAYER &&
            ((Player const*)Target)->IsInSameRaidWith((Player const*)MainTarget))
        {
            if (Target->GetHealth() == Target->GetMaxHealth())
                return 40000;
            else
                return 20000 - Target->GetMaxHealth() + Target->GetHealth();
        }
        else
            return 40000 - Target->GetMaxHealth() + Target->GetHealth();
    }
};

// Helper for targets nearest to the spell target
// The spell target is always first unless there is a target at _completely_ the same position (unbelievable case)
struct TargetDistanceOrder : public std::binary_function<const Unit, const Unit, bool>
{
    const Unit* MainTarget;
    TargetDistanceOrder(const Unit* Target) : MainTarget(Target) {};
    // functor for operator ">"
    bool operator()(const Unit* _Left, const Unit* _Right) const
    {
        return (MainTarget->GetDistance(_Left) < MainTarget->GetDistance(_Right));
    }
};

void Spell::SearchChainTarget(std::list<Unit*> &TagUnitMap, float max_range, uint32 num, SpellTargets TargetType)
{
    uint32 chainSpellJumpRadius = (m_spellInfo->Id == 46480) ? 45 : CHAIN_SPELL_JUMP_RADIUS;
    Unit *cur = m_targets.GetUnitTarget();
    if(!cur)
        return;

    //FIXME: This very like horrible hack and wrong for most spells
    if(m_spellInfo->DmgClass != SPELL_DAMAGE_CLASS_MELEE)
        max_range += num * chainSpellJumpRadius;

    std::list<Unit*> tempUnitMap;
    if(TargetType == SPELL_TARGETS_CHAINHEAL)
    {
        SearchAreaTarget(tempUnitMap, max_range, PUSH_CHAIN, SPELL_TARGETS_ALLY);
        tempUnitMap.sort(ChainHealingOrder(m_caster));
        //if(cur->GetHealth() == cur->GetMaxHealth() && tempUnitMap.size())
        //    cur = tempUnitMap.front();
    }
    else
        SearchAreaTarget(tempUnitMap, max_range, PUSH_CHAIN, TargetType);

    tempUnitMap.remove(cur);

    if (m_spellInfo->Id == 46285 || m_spellInfo->Id == 46289 || m_spellInfo->Id == 46008)
    {
        for(std::list<Unit*>::iterator i = tempUnitMap.begin(); i != tempUnitMap.end(); ++i)
        {
            if ((*i)->IsPet())
                tempUnitMap.remove(*i);
        }
    }

    while(num)
    {
        TagUnitMap.push_back(cur);
        --num;

        if(tempUnitMap.empty())
            break;

        std::list<Unit*>::iterator next;

        if(TargetType == SPELL_TARGETS_CHAINHEAL)
        {
            next = tempUnitMap.begin();
            while(cur->GetDistance(*next) > chainSpellJumpRadius
                || !cur->IsWithinLOSInMap(*next))
            {
                ++next;
                if(next == tempUnitMap.end())
                    return;
            }
        }
        else
        {
            tempUnitMap.sort(TargetDistanceOrder(cur));
            next = tempUnitMap.begin();

            if(cur->GetDistance(*next) > chainSpellJumpRadius)
                break;
            while((m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MELEE
                && !m_caster->isInFront(*next, max_range))
                || !m_caster->CanSeeOrDetect(*next, false)
                || !cur->IsWithinLOSInMap(*next)
                || ((m_spellInfo->HasAttribute(SPELL_ATTR6_CANT_TARGET_CROWD_CONTROLLED)) && (!(*next)->CanFreeMove() || (*next)->IsPolymorphed())))
            {
                ++next;
                if(next == tempUnitMap.end() || cur->GetDistance(*next) > chainSpellJumpRadius)
                    return;
            }
        }

        cur = *next;
        tempUnitMap.erase(next);
    }
}

void Spell::SearchAreaGOTarget(std::list<GameObject*> &TagGoMap, float radius, const uint32 type, uint32 entry)
{
    float x, y, z;
    switch (type)
    {
    case PUSH_DST_CENTER:
        CheckDst();
        x = m_targets.m_destX;
        y = m_targets.m_destY;
        z = m_targets.m_destZ;
        break;
    case PUSH_SRC_CENTER:
        CheckSrc();
        x = m_targets.m_srcX;
        y = m_targets.m_srcY;
        z = m_targets.m_srcZ;
        break;
    default:
        x = m_caster->GetPositionX();
        y = m_caster->GetPositionY();
        z = m_caster->GetPositionZ();
        break;
    }

    CellCoord pair(Trinity::ComputeCellCoord(m_caster->GetPositionX(), m_caster->GetPositionY()));
    Cell cell(pair);
    cell.data.Part.reserved = ALL_DISTRICT;
    cell.SetNoCreate();

    Trinity::AllGameObjectsInRange check(x,y,z,radius);
    Trinity::GameObjectListSearcher<Trinity::AllGameObjectsInRange> searcher(TagGoMap, check);
    TypeContainerVisitor<Trinity::GameObjectListSearcher<Trinity::AllGameObjectsInRange>, GridTypeMapContainer> visitor(searcher);

    cell.Visit(pair, visitor, *(m_caster->GetMap()));
}

void Spell::SearchAreaTarget(std::list<Unit*> &TagUnitMap, float radius, const uint32 type, SpellTargets TargetType, uint32 entry)
{
    float x, y, z;
    switch(type)
    {
        case PUSH_DST_CENTER:
            CheckDst();
            x = m_targets.m_destX;
            y = m_targets.m_destY;
            z = m_targets.m_destZ;
            break;
        case PUSH_SRC_CENTER:
            CheckSrc();
            x = m_targets.m_srcX;
            y = m_targets.m_srcY;
            z = m_targets.m_srcZ;
            break;
        case PUSH_CHAIN:
        {
            Unit *target = m_targets.GetUnitTarget();
            if(!target)
            {
                TC_LOG_ERROR("spell", "SPELL: cannot find unit target for spell ID %u\n", m_spellInfo->Id );
                return;
            }
            x = target->GetPositionX();
            y = target->GetPositionY();
            z = target->GetPositionZ();
            break;
        }
        default:
            x = m_caster->GetPositionX();
            y = m_caster->GetPositionY();
            z = m_caster->GetPositionZ();
            break;
    }

    Trinity::SpellNotifierCreatureAndPlayer notifier(*this, TagUnitMap, radius, type, TargetType, entry, x, y, z);
    if((m_spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_PLAYERS))
        || (TargetType == SPELL_TARGETS_ENTRY && !entry))
        m_caster->GetMap()->VisitWorld(x, y, radius, notifier);
    else
        m_caster->GetMap()->VisitAll(x, y, radius, notifier);
}

WorldObject* Spell::SearchNearbyTarget(float range, SpellTargets TargetType)
{
    switch(TargetType)
    {
        case SPELL_TARGETS_ENTRY:
        {
            SpellScriptTarget::const_iterator lower = sSpellMgr->GetBeginSpellScriptTarget(m_spellInfo->Id);
            SpellScriptTarget::const_iterator upper = sSpellMgr->GetEndSpellScriptTarget(m_spellInfo->Id);
            if(lower == upper)
            {
                TC_LOG_ERROR("sql.sql","Spell (ID: %u) (caster Entry: %u - DB GUID: %u) does not have record in `spell_script_target`.", m_spellInfo->Id, m_caster->GetEntry(), (m_caster->ToCreature() ? m_caster->ToCreature()->GetDBTableGUIDLow() : 0));
                if(m_targets.GetUnitTarget())
                    return m_targets.GetUnitTarget(); //keep current target
                //else search one nearby
                if(m_spellInfo->IsPositive())
                    return SearchNearbyTarget(range, SPELL_TARGETS_ALLY);
                else
                    return SearchNearbyTarget(range, SPELL_TARGETS_ENEMY);
            }

            Creature* creatureScriptTarget = NULL;
            GameObject* goScriptTarget = NULL;

            for(SpellScriptTarget::const_iterator i_spellST = lower; i_spellST != upper; ++i_spellST)
            {
                switch(i_spellST->second.type)
                {
                    case SPELL_TARGET_TYPE_GAMEOBJECT:
                    {
                        GameObject* p_GameObject = NULL;

                        if(i_spellST->second.targetEntry)
                        {
                            Trinity::NearestGameObjectEntryInObjectRangeCheck go_check(*m_caster,i_spellST->second.targetEntry,range);
                            Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> checker(m_caster, p_GameObject,go_check);
                            m_caster->VisitNearbyGridObject(range, checker);

                            if(p_GameObject)
                            {
                                // remember found target and range, next attempt will find more near target with another entry
                                creatureScriptTarget = NULL;
                                goScriptTarget = p_GameObject;
                                range = go_check.GetLastRange();
                            }
                        }
                        else if( focusObject )          //Focus Object
                        {
                            float frange = m_caster->GetDistance(focusObject);
                            if(range >= frange)
                            {
                                creatureScriptTarget = NULL;
                                goScriptTarget = focusObject;
                                range = frange;
                            }
                        }
                        break;
                    }
                    case SPELL_TARGET_TYPE_CREATURE:
                    case SPELL_TARGET_TYPE_DEAD:
                    default:
                    {
                        //keep our current target if it's already the right entry
                        if(m_targets.GetUnitTarget() && m_targets.GetUnitTarget()->GetEntry() == i_spellST->second.targetEntry)
                            return m_targets.GetUnitTarget();

                        Creature *p_Creature = NULL;

                        Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck u_check(*m_caster,i_spellST->second.targetEntry,i_spellST->second.type!=SPELL_TARGET_TYPE_DEAD,range);
                        Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(p_Creature, u_check);
                        m_caster->VisitNearbyObject(range, searcher);

                        if(p_Creature )
                        {
                            creatureScriptTarget = p_Creature;
                            goScriptTarget = NULL;
                            range = u_check.GetLastRange();
                        }
                        break;
                    }
                }
            }

            if(creatureScriptTarget)
                return creatureScriptTarget;
            else
                return goScriptTarget;
        }
        default:
        case SPELL_TARGETS_ENEMY:
        {
            Unit *target = NULL;
            Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(m_caster, m_caster, range);
            Trinity::UnitLastSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(m_caster, target, u_check);
            m_caster->VisitNearbyObject(range, searcher);
            return target;
        }
        case SPELL_TARGETS_ALLY:
        {
            Unit *target = NULL;
            Trinity::AnyFriendlyUnitInObjectRangeCheck u_check(m_caster, m_caster, range);
            Trinity::UnitLastSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(m_caster, target, u_check);
            m_caster->VisitNearbyObject(range, searcher);
            return target;
        }
    }
}

void Spell::SetTargetMap(uint32 i, uint32 cur)
{
    SpellNotifyPushType pushType = PUSH_NONE;
    Player *modOwner = NULL;
    if(m_originalCaster)
        modOwner = m_originalCaster->GetSpellModOwner();

    switch(sSpellMgr->SpellTargetType[cur])
    {
        case TARGET_TYPE_UNIT_CASTER:
        {
            switch(cur)
            {
                case TARGET_UNIT_CASTER:
                    AddUnitTarget(m_caster, i);
                    break;
                case TARGET_DEST_CASTER_FISHING:
                {
                    float min_dis = m_spellInfo->GetMinRange();
                    float max_dis = m_spellInfo->GetMaxRange(false, m_caster->GetSpellModOwner(), this);
                    float dis = m_caster->GetMap()->rand_norm() * (max_dis - min_dis) + min_dis;
                    float x, y, z;
                    m_caster->GetClosePoint(x, y, z, DEFAULT_WORLD_OBJECT_SIZE, dis);
                    m_targets.SetDestination(x, y, z);
                    break;
                }
                case TARGET_UNIT_MASTER:
                    if(Unit* owner = m_caster->GetCharmerOrOwner())
                        AddUnitTarget(owner, i);
                    break;
                case TARGET_UNIT_PET:
                    if(Pet* pet = m_caster->GetPet())
                        AddUnitTarget(pet, i);
                    break;
                case TARGET_UNIT_CASTER_AREA_PARTY:
                case TARGET_UNIT_CASTER_AREA_RAID:
                    pushType = PUSH_CASTER_CENTER;
                    break;
            }
            break;
        }

        case TARGET_TYPE_UNIT_TARGET:
        {
            Unit *target = m_targets.GetUnitTarget();
            if(!target)
            {
                TC_LOG_ERROR("FIXME","SPELL: no unit target for spell ID %u (case TARGET_TYPE_UNIT_TARGET)", m_spellInfo->Id);
                break;
            }

            switch(cur)
            {
                case TARGET_UNIT_TARGET_ENEMY:
                    if((m_spellInfo->AttributesEx & (0x8 | 0x80)) == 0)
                    {
                        // try to select magnet target first
                        if(SelectMagnetTarget() == m_targets.GetUnitTarget())
                            // if not found (target is not changed) search for SPELL_AURA_ADD_CASTER_HIT_TRIGGER
                            HandleHitTriggerAura();
                    }
                case TARGET_UNIT_TARGET_CHAINHEAL_ALLY:
                    pushType = PUSH_CHAIN;
                    break;
                case TARGET_UNIT_TARGET_ALLY:
                case TARGET_UNIT_TARGET_RAID:
                case TARGET_UNIT_TARGET_ANY: // SelectMagnetTarget()?
                case TARGET_UNIT_TARGET_PARTY:
                case TARGET_UNIT_TARGET_MINIPET:
                    AddUnitTarget(target, i);
                    break;
                case TARGET_UNIT_LASTTARGET_AREA_PARTY:
                case TARGET_UNIT_TARGET_AREA_RAID_CLASS:
                    pushType = PUSH_CASTER_CENTER; // not real
                    break;
            }
            break;
        }

        case TARGET_TYPE_UNIT_NEARBY:
        {
            float range = m_spellInfo->GetMaxRange(false, modOwner, this);

            WorldObject *target = NULL;

            switch(cur)
            {
                case TARGET_UNIT_NEARBY_ENEMY:
                    target = SearchNearbyTarget(range, SPELL_TARGETS_ENEMY);
                    break;
                case TARGET_UNIT_NEARBY_ALLY:
                case TARGET_UNIT_NEARBY_ALLY_UNK:
                case TARGET_UNIT_NEARBY_RAID:
                    target = SearchNearbyTarget(range, SPELL_TARGETS_ALLY);
                    break;
                case TARGET_UNIT_NEARBY_ENTRY:
                    target = SearchNearbyTarget(range, SPELL_TARGETS_ENTRY);
                    break;
            }

            if(!target)
                return;
            else if(target->GetTypeId() == TYPEID_UNIT || target->GetTypeId() == TYPEID_PLAYER)
            {
                pushType = PUSH_CHAIN;

                m_targets.SetUnitTarget((Unit*)target);
            }
            else if(target->GetTypeId() == TYPEID_GAMEOBJECT)
                AddGOTarget((GameObject*)target, i);

            break;
        }

        case TARGET_TYPE_AREA_SRC:
            pushType = PUSH_SRC_CENTER;
            break;

        case TARGET_TYPE_AREA_DST:
            pushType = PUSH_DST_CENTER;
            break;

        case TARGET_TYPE_AREA_CONE:
            if(m_spellInfo->HasAttribute(SPELL_ATTR_CU_CONE_BACK))
                pushType = PUSH_IN_BACK;
            else if(m_spellInfo->HasAttribute(SPELL_ATTR_CU_CONE_LINE))
                pushType = PUSH_IN_LINE;
            else if(m_spellInfo->HasAttribute(SPELL_ATTR_CU_CONE_180))
                pushType = PUSH_IN_FRONT_180;
            else
                pushType = PUSH_IN_FRONT;
            break;
           
        case TARGET_TYPE_DEST_CASTER: //4+8+2
        {
            if(cur == TARGET_SRC_CASTER)
            {
                m_targets.SetSrc(m_caster);
                break;
            }
            else if(cur == TARGET_DEST_CASTER)
            {
                m_targets.SetDestination(m_caster);
                break;
            }

            float angle, dist;

            float objSize = m_caster->GetObjectSize();
            dist = m_spellInfo->Effects[i].CalcRadius(m_caster->GetSpellModOwner());
            if(dist < objSize)
                dist = objSize;
            else if(cur == TARGET_DEST_CASTER_RANDOM)
              dist = objSize + (dist - objSize) * m_caster->GetMap()->rand_norm();

            bool calcPos = true;
            Position pos;

            switch(cur)
            {
                //special case for leap
                case TARGET_DEST_CASTER_FRONT_LEAP:
                    pos = m_caster->GetLeapPosition(dist);
                    calcPos = false;
                    break;
                case TARGET_DEST_CASTER_FRONT_LEFT: angle = -M_PI/4;    break;
                case TARGET_DEST_CASTER_BACK_LEFT:  angle = -3*M_PI/4;  break;
                case TARGET_DEST_CASTER_BACK_RIGHT: angle = 3*M_PI/4;   break;
                case TARGET_DEST_CASTER_FRONT_RIGHT:angle = M_PI/4;     break;
                case TARGET_DEST_CASTER_SUMMON:
                case TARGET_DEST_CASTER_FRONT:      angle = 0.0f;       break;
                case TARGET_DEST_CASTER_BACK:       angle = M_PI;       break;
                case TARGET_DEST_CASTER_RIGHT:      angle = M_PI/2;     break;
                case TARGET_DEST_CASTER_LEFT:       angle = -M_PI/2;    break;
                default:                            angle = rand_norm()*2*M_PI; break;
            }

            if(calcPos)
                pos = m_caster->GetFirstWalkableCollisionPosition(dist, angle);

            m_targets.SetDestination(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
            break;
        }

        case TARGET_TYPE_DEST_TARGET_ENEMY:
        {
            if(m_spellInfo->Effects[i].TargetB.GetTarget() == 0) //random target
            {
                std::list<Unit*> unitList;
                float range = m_spellInfo->GetMaxRange(false, modOwner, this);
                //todo : rewrite this, this is functionnal but far from optimized
                SearchAreaTarget(unitList, range, PUSH_NONE, SPELL_TARGETS_ENEMY);
                if(unitList.size())
                {
                    Trinity::Containers::RandomResizeList(unitList, 1);
                    m_targets.SetDestination(unitList.front());
                    break;
                }
            }
            
            Unit *target = m_targets.GetUnitTarget();
            if(!target)
            {
                TC_LOG_ERROR("FIXME","SPELL: no unit target for spell ID %u (case TARGET_DEST_TARGET_ENEMY)", m_spellInfo->Id);
                break;
            }

            m_targets.SetDestination(target);
            break;
        }

        case TARGET_TYPE_DEST_TARGET: //2+8+2
        {
            Unit *target = m_targets.GetUnitTarget();
            if(!target)
            {
                TC_LOG_ERROR("FIXME","SPELL: no unit target for spell ID %u (case TARGET_TYPE_DEST_TARGET)", m_spellInfo->Id);
                break;
            }

            if(cur == TARGET_DEST_TARGET_ANY)
            {
                m_targets.SetDestination(target);
                break;
            }

            float x, y, z, angle, dist;

            float objSize = target->GetObjectSize();
            dist = m_spellInfo->Effects[i].CalcRadius(m_caster->GetSpellModOwner(), this);
            if(dist < objSize)
                dist = objSize;
            else if(cur == TARGET_DEST_CASTER_RANDOM)
              dist = objSize + (dist - objSize) * m_caster->GetMap()->rand_norm();

            switch(cur)
            {
                case TARGET_DEST_TARGET_FRONT:      angle = 0.0f;       break;
                case TARGET_DEST_TARGET_BACK:       angle = M_PI;       break;
                case TARGET_DEST_TARGET_RIGHT:      angle = M_PI/2;     break;
                case TARGET_DEST_TARGET_LEFT:       angle = -M_PI/2;    break;
                case TARGET_DEST_TARGET_FRONT_LEFT: angle = -M_PI/4;    break;
                case TARGET_DEST_TARGET_BACK_LEFT:  angle = -3*M_PI/4;  break;
                case TARGET_DEST_TARGET_BACK_RIGHT: angle = 3*M_PI/4;   break;
                case TARGET_DEST_TARGET_FRONT_RIGHT:angle = M_PI/4;     break;
            default:                            angle = m_caster->GetMap()->rand_norm()*2*M_PI; break;
            }

            target->GetGroundPointAroundUnit(x, y, z, dist, angle);
            m_targets.SetDestination(x, y, z);
            break;
        }

        case TARGET_TYPE_DEST_DEST: //5+8+1
        {
            if(!m_targets.HasDst())
            {
                TC_LOG_ERROR("FIXME","SPELL: no destination for spell ID %u\n", m_spellInfo->Id);
                break;
            }

            float angle;
            switch(cur)
            {
                case TARGET_DEST_DYNOBJ_ENEMY:
                case TARGET_DEST_DYNOBJ_ALLY:
                case TARGET_DEST_DYNOBJ_NONE:
                case TARGET_DEST_DEST:
                case TARGET_DEST_TRAJ:
                    return;
                case TARGET_DEST_DEST_FRONT:      angle = 0.0f;       break;
                case TARGET_DEST_DEST_BACK:       angle = M_PI;       break;
                case TARGET_DEST_DEST_RIGHT:      angle = M_PI/2;     break;
                case TARGET_DEST_DEST_LEFT:       angle = -M_PI/2;    break;
                case TARGET_DEST_DEST_FRONT_LEFT: angle = -M_PI/4;    break;
                case TARGET_DEST_DEST_BACK_LEFT:  angle = -3*M_PI/4;  break;
                case TARGET_DEST_DEST_BACK_RIGHT: angle = 3*M_PI/4;   break;
                case TARGET_DEST_DEST_FRONT_RIGHT:angle = M_PI/4;     break;
            default:                          angle = m_caster->GetMap()->rand_norm()*2*M_PI; break;
            }

            float dist, x, y, z;
            dist = m_spellInfo->Effects[i].CalcRadius(m_caster->GetSpellModOwner(), this);
            if (cur == TARGET_DEST_DEST_RANDOM)
              dist *= m_caster->GetMap()->rand_norm();

            x = m_targets.m_destX;
            y = m_targets.m_destY;
            z = m_targets.m_destZ;
            m_caster->GetGroundPoint(x, y, z, dist, angle);
            m_targets.SetDestination(x, y, z);
            break;
        }

        case TARGET_TYPE_DEST_SPECIAL:
        {
            switch(cur)
            {
                case TARGET_DEST_DB:
                    if(SpellTargetPosition const* st = sSpellMgr->GetSpellTargetPosition(m_spellInfo->Id))
                    {
                        //TODO: fix this check
                        if(m_spellInfo->Effects[0].Effect == SPELL_EFFECT_TELEPORT_UNITS
                            || m_spellInfo->Effects[1].Effect == SPELL_EFFECT_TELEPORT_UNITS
                            || m_spellInfo->Effects[2].Effect == SPELL_EFFECT_TELEPORT_UNITS)
                            m_targets.SetDestination(st->target_X, st->target_Y, st->target_Z, (int32)st->target_mapId);
                        else if(st->target_mapId == m_caster->GetMapId())
                            m_targets.SetDestination(st->target_X, st->target_Y, st->target_Z);
                    }
                    else
                        TC_LOG_ERROR("spell", "SPELL: unknown target coordinates for spell ID %u\n", m_spellInfo->Id );
                    break;
                case TARGET_DEST_HOME:
                    if(m_caster->GetTypeId() == TYPEID_PLAYER)
                        m_targets.SetDestination((m_caster->ToPlayer())->m_homebindX,(m_caster->ToPlayer())->m_homebindY,(m_caster->ToPlayer())->m_homebindZ, (m_caster->ToPlayer())->m_homebindMapId);
                    break;
                case TARGET_DEST_NEARBY_ENTRY:
                {
                    float range = m_spellInfo->GetMaxRange(false, modOwner, this);

                    WorldObject *target = SearchNearbyTarget(range, SPELL_TARGETS_ENTRY);
                    if(target)
                        m_targets.SetDestination(target);
                    break;
                }
            }
            break;
        }

        case TARGET_TYPE_CHANNEL:
        {
            if(!m_originalCaster || !m_originalCaster->m_currentSpells[CURRENT_CHANNELED_SPELL])
            {
                TC_LOG_ERROR( "spell","SPELL: no current channeled spell for spell ID %u", m_spellInfo->Id );
                break;
            }

            switch(cur)
            {
                case TARGET_UNIT_CHANNEL_TARGET:
                    if(Unit* target = m_originalCaster->m_currentSpells[CURRENT_CHANNELED_SPELL]->m_targets.GetUnitTarget())
                        AddUnitTarget(target, i);
                    else
                        TC_LOG_ERROR("spell", "SPELL: cannot find channel spell target for spell ID %u", m_spellInfo->Id );
                    break;
                case TARGET_DEST_CHANNEL_TARGET:
                    if(m_originalCaster->m_currentSpells[CURRENT_CHANNELED_SPELL]->m_targets.HasDst())
                        m_targets = m_originalCaster->m_currentSpells[CURRENT_CHANNELED_SPELL]->m_targets;
                    else
                        TC_LOG_ERROR("FIXME", "SPELL: cannot find channel spell destination for spell ID %u", m_spellInfo->Id );
                    break;
            }
            break;
        }

        default:
        {
            switch(cur)
            {
                case TARGET_GAMEOBJECT_TARGET:
                case TARGET_GAMEOBJECT_NEARBY_ENTRY:
                    if(m_targets.GetGOTarget()) 
                        AddGOTarget(m_targets.GetGOTarget(), i);
                    break;
                case TARGET_GAMEOBJECT_ITEM_TARGET:
                    if(m_targets.GetGOTargetGUID())
                        AddGOTarget(m_targets.GetGOTarget(), i);
                    else if(m_targets.GetItemTarget())
                        AddItemTarget(m_targets.GetItemTarget(), i);
                    break;
                default:
                    TC_LOG_ERROR("FIXME","Unhandled spell target %u", cur);
                    break;
            }
            break;
        }
    }

    if(pushType == PUSH_CHAIN) // Chain
    {
        Unit *target = m_targets.GetUnitTarget();
        if(!target)
        {
            TC_LOG_ERROR("FIXME","SPELL: no chain unit target for spell ID %u", m_spellInfo->Id);
            return;
        }

        //Chain: 2, 6, 22, 25, 45, 77
        uint32 maxTargets = m_spellInfo->Effects[i].ChainTarget;
        if(modOwner)
            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_JUMP_TARGETS, maxTargets, this);

        if(maxTargets > 1)
        {
            //otherwise, this multiplier is used for something else
            m_damageMultipliers[i] = 1.0f;
            m_applyMultiplierMask |= 1 << i;

            float range = m_spellInfo->GetMaxRange(false, modOwner, this);

            std::list<Unit*> unitList;

            switch(cur)
            {
                case TARGET_UNIT_NEARBY_ENEMY:
                case TARGET_UNIT_TARGET_ENEMY:
                case TARGET_UNIT_NEARBY_ENTRY: // fix me
                    SearchChainTarget(unitList, range, maxTargets, SPELL_TARGETS_ENEMY);
                    break;
                case TARGET_UNIT_TARGET_CHAINHEAL_ALLY:
                case TARGET_UNIT_NEARBY_ALLY:  // fix me
                case TARGET_UNIT_NEARBY_ALLY_UNK:
                case TARGET_UNIT_NEARBY_RAID:
                    SearchChainTarget(unitList, range, maxTargets, SPELL_TARGETS_CHAINHEAL);
                    break;
            }
            
            for(std::list<Unit*>::iterator itr = unitList.begin(); itr != unitList.end(); ++itr)
                AddUnitTarget(*itr, i);
        } else {
            AddUnitTarget(target, i);
        }
    }
    else if(pushType)
    {
        // Dummy, just for client
        if(sSpellMgr->EffectTargetType[m_spellInfo->Effects[i].Effect] == SPELL_REQUIRE_DEST)
            return;

        float radius = m_spellInfo->Effects[i].CalcRadius(m_caster->GetSpellModOwner(), this) *m_spellValue->RadiusMod;

        std::list<Unit*> unitList;
        std::list<GameObject*> goList;

        switch(cur)
        {
            case TARGET_UNIT_SRC_AREA_ENEMY:
            case TARGET_UNIT_DEST_AREA_ENEMY:
            case TARGET_UNIT_CONE_ENEMY:
            case TARGET_UNIT_CONE_ENEMY_UNKNOWN:
                SearchAreaTarget(unitList, radius, pushType, SPELL_TARGETS_ENEMY);
                break;
            case TARGET_UNIT_SRC_AREA_ALLY:
            case TARGET_UNIT_DEST_AREA_ALLY:
            case TARGET_UNIT_CONE_ALLY:
                SearchAreaTarget(unitList, radius, pushType, SPELL_TARGETS_ALLY);
                break;
            case TARGET_UNIT_SRC_AREA_PARTY:
            case TARGET_UNIT_DEST_AREA_PARTY:
                m_caster->GetPartyMember(unitList, radius); //fix me
                break;
            case TARGET_GAMEOBJECT_SRC_AREA:
            case TARGET_GAMEOBJECT_DEST_AREA:
                //this is a quick and dirty impl, I'm not even sure what these targets are supposed to do. For now it will be target any gobject in range
                SearchAreaGOTarget(goList, radius, pushType);
                break;
            case TARGET_UNIT_SRC_AREA_ENTRY:
            case TARGET_UNIT_DEST_AREA_ENTRY:
            case TARGET_UNIT_CONE_ENTRY: // fix me
            {
                SpellScriptTarget::const_iterator lower = sSpellMgr->GetBeginSpellScriptTarget(m_spellInfo->Id);
                SpellScriptTarget::const_iterator upper = sSpellMgr->GetEndSpellScriptTarget(m_spellInfo->Id);
                if(lower == upper)
                {
                    TC_LOG_ERROR("FIXME","Spell (ID: %u) (caster Entry: %u - DB GUID: %u) does not have record in `spell_script_target`.", m_spellInfo->Id, m_caster->GetEntry(), (m_caster->ToCreature() ? m_caster->ToCreature()->GetDBTableGUIDLow() : 0));

                    if(m_spellInfo->IsPositiveEffect(i))
                        SearchAreaTarget(unitList, radius, pushType, SPELL_TARGETS_ALLY);
                    else
                        SearchAreaTarget(unitList, radius, pushType, SPELL_TARGETS_ENEMY);
                }
                // let it be done in one check?
                else
                {
                    for(SpellScriptTarget::const_iterator i_spellST = lower; i_spellST != upper; ++i_spellST)
                    {
                        if(i_spellST->second.type == SPELL_TARGET_TYPE_CREATURE)
                            SearchAreaTarget(unitList, radius, pushType, SPELL_TARGETS_ENTRY, i_spellST->second.targetEntry);
                    }
                }
                break;
            }
            case TARGET_UNIT_LASTTARGET_AREA_PARTY:
                m_targets.GetUnitTarget()->GetPartyMember(unitList, radius);
                break;
            case TARGET_UNIT_CASTER_AREA_PARTY:
                m_caster->GetPartyMember(unitList, radius);
                break;
            case TARGET_UNIT_CASTER_AREA_RAID:
                m_caster->GetRaidMember(unitList, radius);
                break;
            case TARGET_UNIT_TARGET_AREA_RAID_CLASS:
            {
                Player* targetPlayer = m_targets.GetUnitTarget() && m_targets.GetUnitTarget()->GetTypeId() == TYPEID_PLAYER
                    ? m_targets.GetUnitTarget()->ToPlayer() : NULL;

                Group* pGroup = targetPlayer ? targetPlayer->GetGroup() : NULL;
                if(pGroup)
                {
                    for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
                    {
                        Player* Target = itr->GetSource();

                        // IsHostileTo check duel and controlled by enemy
                        if( Target && targetPlayer->IsWithinDistInMap(Target, radius) &&
                            targetPlayer->GetClass() == Target->GetClass() &&
                            !m_caster->IsHostileTo(Target) )
                        {
                            AddUnitTarget(Target, i);
                        }
                        
                        if (targetPlayer->GetClass() == CLASS_WARRIOR) {
                            Pet* pet = Target->GetPet();
                            if (pet && targetPlayer->IsWithinDistInMap(pet, radius) && !m_caster->IsHostileTo(pet))
                                AddUnitTarget(pet, i);
                        }
                    }
                }
                else if(m_targets.GetUnitTarget())
                    AddUnitTarget(m_targets.GetUnitTarget(), i);
                break;
            }
        }

        if(!unitList.empty())
        {
            if (m_spellValue->MaxAffectedTargets)
            {
                if(m_spellInfo->Id == 5246)     //Intimidating Shout
                    unitList.remove(m_targets.GetUnitTarget());

                if(m_spellValue->MaxAffectedTargets == 1 && m_spellInfo->Attributes & SPELL_ATTR0_UNK11) //Prefer not victim ?
                {
                    if(unitList.size() > 1)
                        unitList.remove(m_caster->GetVictim());
                }
                Trinity::Containers::RandomResizeList(unitList, m_spellValue->MaxAffectedTargets);
            }
            else if (m_spellInfo->Id == 27285)  // Seed of Corruption proc spell
                unitList.remove(m_targets.GetUnitTarget());
            else if (m_spellInfo->Id == 44866)  // Kalecgos spectral blast
                unitList.remove(m_targets.GetUnitTarget());
            else if (m_spellInfo->Id == 42480 || m_spellInfo->Id == 42479)  // Protective Ward (Zul'aman)
                unitList.remove(m_targets.GetUnitTarget());
            else if (m_spellInfo->Id == 45150) {
                for(std::list<Unit*>::iterator itr = unitList.begin(); itr != unitList.end(); ++itr) {
                    if ((*itr)->IsPet())
                        unitList.remove(*itr);
                }
            } else if (m_spellInfo->Id == 32375) {
                for(std::list<Unit*>::iterator itr = unitList.begin(); itr != unitList.end(); ++itr) {
                    if ((*itr)->GetTypeId() == TYPEID_UNIT && (*itr)->ToCreature()->IsTotem())
                        unitList.remove(*itr);
                }
            }

            // Karazhan chess
            switch (m_spellInfo->Id) {
            case 37476:
            case 37474:
                for(std::list<Unit*>::iterator itr = unitList.begin(); itr != unitList.end(); ++itr) {
                    if (!m_caster->HasInArc(M_PI/2 + 0.2f, (*itr)))
                        unitList.remove(*itr);
                }
                break;
            case 37454:
            case 37461:
            case 37453:
            case 37459:
                for(std::list<Unit*>::iterator itr = unitList.begin(); itr != unitList.end(); ++itr) {
                    if (!m_caster->HasInArc(M_PI/3, (*itr)) || (*itr)->GetExactDistance2d(m_caster->GetPositionX(), m_caster->GetPositionY()) > 9.5f)
                        unitList.remove(*itr);
                }
                break;
            case 37413:
            case 37406:
                for(std::list<Unit*>::iterator itr = unitList.begin(); itr != unitList.end(); ++itr) {
                    if (!m_caster->HasInArc(M_PI/3, (*itr)) || (*itr)->GetExactDistance2d(m_caster->GetPositionX(), m_caster->GetPositionY()) > 5.2f)
                        unitList.remove(*itr);
                }
                break;
            case 46285:
            case 46008:
            case 46289:
                for(std::list<Unit*>::iterator itr = unitList.begin(); itr != unitList.end(); ++itr)
                {
                    if ((*itr)->IsPet())
                        unitList.remove(*itr);
                }
                break;
            }

            for(std::list<Unit*>::iterator itr = unitList.begin(); itr != unitList.end(); ++itr)
                AddUnitTarget(*itr, i);
            for (auto itr : goList)
                AddGOTarget(itr, i);
        }
    } // Chain or Area
}

*/
uint32 Spell::prepare(SpellCastTargets const* targets, Aura* triggeredByAura)
{
    if(m_CastItem)
        m_castItemGUID = m_CastItem->GetGUID();
    else
        m_castItemGUID = 0;

    InitExplicitTargets(*targets);

    // Fill aura scaling information
    if (m_caster->IsControlledByPlayer() && !m_spellInfo->IsPassive() && m_spellInfo->SpellLevel && !m_spellInfo->IsChanneled() /* _triggeredCastFlags NYI) /&& !(_triggeredCastFlags & TRIGGERED_IGNORE_AURA_SCALING )*/)
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (m_spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA)
            {
                // Change aura with ranks only if basepoints are taken from spellInfo and aura is positive
                if (m_spellInfo->IsPositiveEffect(i))
                {
                    m_auraScaleMask |= (1 << i);
                    if (m_spellValue->EffectBasePoints[i] != m_spellInfo->Effects[i].BasePoints)
                    {
                        m_auraScaleMask = 0;
                        break;
                    }
                }
            }
        }
    }

    m_spellState = SPELL_STATE_PREPARING;
    //TC_LOG_DEBUG("FIXME","Spell %u - State : SPELL_STATE_PREPARING",m_spellInfo->Id);
    //TC_LOG_DEBUG("FIXME","caster is %sin combat",(m_caster->IsInCombat()?"":"not "));

    m_caster->GetPosition(m_castPositionX, m_castPositionY, m_castPositionZ);
    m_castOrientation = m_caster->GetOrientation();

    if(triggeredByAura)
        m_triggeredByAuraSpell  = triggeredByAura->GetSpellInfo();

    // create and add update event for this spell
    SpellEvent* Event = new SpellEvent(this);
    m_caster->m_Events.AddEvent(Event, m_caster->m_Events.CalculateTime(1));

    //Another spell in progress ?
    if(m_caster->IsNonMeleeSpellCast(false, true) && m_cast_count)
    {
        SendCastResult(SPELL_FAILED_SPELL_IN_PROGRESS);
        finish(false,false);
        return SPELL_FAILED_SPELL_IN_PROGRESS;
    }
            
    if (m_caster->IsSpellDisabled(m_spellInfo->Id))
    {
        SendCastResult(SPELL_FAILED_SPELL_UNAVAILABLE);
        finish(false,false);
        return SPELL_FAILED_SPELL_UNAVAILABLE;
    }

    // Fill cost data
    m_powerCost = CalculatePowerCost();

    SpellCastResult result = CheckCast(true);
    //TC_LOG_DEBUG("FIXME","CheckCast for %u : %u", m_spellInfo->Id, result);
    if(result != SPELL_CAST_OK && !IsAutoRepeat()) //always cast autorepeat dummy for triggering
    {
        if(triggeredByAura)
            triggeredByAura->SetAuraDuration(0);

        SendCastResult(result);
        finish(false,false);
        return result;
    }

    // Prepare data for triggers
    prepareDataForTriggerSystem();

    // calculate cast time (calculated after first CheckCast to prevent charge counting for first CheckCast fail)
    m_casttime = m_spellInfo->CalcCastTime(this);

    // Set cast time to 0 if .cheat casttime is enabled.

    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (m_caster->ToPlayer()->GetCommandStatus(CHEAT_CASTTIME))
            m_casttime = 0;
    }

    // don't allow channeled spells / spells with cast time to be casted while moving
    // (even if they are interrupted on moving, spells with almost immediate effect get to have their effect processed before movement interrupter kicks in)
    if ((m_spellInfo->IsChanneled() || m_casttime) && m_caster->GetTypeId() == TYPEID_PLAYER && m_caster->isMoving() && m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT && !IsTriggered())
    {
        SendCastResult(SPELL_FAILED_MOVING);
        finish(false);
        return SPELL_FAILED_MOVING;
    }

    // xinef: if spell have nearby target entry only, do not allow to cast if no targets are found
    if (m_CastItem)
    {
        bool selectTargets = false;
        for (uint8 i = EFFECT_0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (!m_spellInfo->Effects[i].IsEffect())
                continue;

            if (m_spellInfo->Effects[i].TargetA.GetSelectionCategory() != TARGET_SELECT_CATEGORY_NEARBY || m_spellInfo->Effects[i].TargetA.GetCheckType() != TARGET_CHECK_ENTRY)
            {
                selectTargets = false;
                break;
            }

            // xinef: by default set it to false, and to true if any valid target is found
            selectTargets = true;
        }

        if (selectTargets)
        {
            SelectSpellTargets();
            _spellTargetsSelected = true;

            if (m_UniqueTargetInfo.empty() && m_UniqueGOTargetInfo.empty())
            {
                SendCastResult(SPELL_FAILED_CASTER_AURASTATE);
                finish(false);
                return SPELL_FAILED_CASTER_AURASTATE;
            }
        }
    }

    // set timer base at cast time
    ReSetTimer();

    if (GetCaster())
        if (Player *tmpPlayer = GetCaster()->ToPlayer())
            if (tmpPlayer->HaveSpectators())
            {
                SpectatorAddonMsg msg;
                msg.SetPlayer(tmpPlayer->GetName());
                msg.CastSpell(m_spellInfo->Id, m_casttime);
                tmpPlayer->SendSpectatorAddonMsgToBG(msg);
            }

    if(m_IsTriggeredSpell)
        cast(true);
    else
    {
        // stealth must be removed at cast starting (at show channel bar)
        // skip triggered spell (item equip spell casting and other not explicit character casts/item uses)
        if(m_spellInfo->IsBreakingStealth())
        {
            m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CAST);
            m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_SPELL_ATTACK);
        }

        m_caster->SetCurrentCastedSpell( this );
        m_selfContainer = &(m_caster->m_currentSpells[GetCurrentContainer()]);
        SendSpellStart();
        
        // set target for proper facing
        if (m_casttime && !m_IsTriggeredSpell)
            if (uint64 target = m_targets.GetUnitTargetGUID())
                if(Unit* uTarget = ObjectAccessor::GetUnit(*m_caster,target))
                    if (m_caster->GetGUID() != target && m_caster->GetTypeId() == TYPEID_UNIT)
                        m_caster->ToCreature()->FocusTarget(this, uTarget);

        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            (m_caster->ToPlayer())->AddGlobalCooldown(m_spellInfo,this);
            
        TriggerGlobalCooldown();

        if(!m_casttime && !m_spellInfo->StartRecoveryTime
            && !m_castItemGUID     //item: first cast may destroy item and second cast causes crash
            && GetCurrentContainer() == CURRENT_GENERIC_SPELL)
            cast(true);
    }
    return SPELL_CAST_OK;
}

void Spell::cancel()
{
    //TC_LOG_DEBUG("FIXME","Spell %u - cancel()", m_spellInfo->Id);
    if(m_spellState == SPELL_STATE_FINISHED)
        return;

    SetReferencedFromCurrent(false);
    if(m_selfContainer && *m_selfContainer == this)
        *m_selfContainer = NULL;

    uint32 oldState = m_spellState;
    m_spellState = SPELL_STATE_FINISHED;
    //TC_LOG_DEBUG("FIXME","Spell %u - State : SPELL_STATE_FINISHED",m_spellInfo->Id);

    m_autoRepeat = false;
    switch (oldState)
    {
        case SPELL_STATE_PREPARING:
            CancelGlobalCooldown();
            // No break
        case SPELL_STATE_DELAYED:
        {
            SendInterrupted(0);
            SendCastResult(SPELL_FAILED_INTERRUPTED);
        } break;

        case SPELL_STATE_CASTING:
        {
            for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
            {
                if( ihit->missCondition == SPELL_MISS_NONE )
                {
                    Unit* unit = m_caster->GetGUID()==(*ihit).targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID);
                    if( unit && unit->IsAlive() )
                        unit->RemoveAurasByCasterSpell(m_spellInfo->Id, m_caster->GetGUID());
                }
            }

            m_caster->RemoveAurasByCasterSpell(m_spellInfo->Id, m_caster->GetGUID());
            SendChannelUpdate(0,m_spellInfo->Id);
            SendInterrupted(0);
            SendCastResult(SPELL_FAILED_INTERRUPTED);

            if (GetCaster() && m_spellInfo)
            {
                if (Player *tmpPlayer = GetCaster()->ToPlayer())
                {
                    if (tmpPlayer->HaveSpectators())
                    {
                        SpectatorAddonMsg msg;
                        msg.SetPlayer(tmpPlayer->GetName());
                        msg.InterruptSpell(m_spellInfo->Id);
                        tmpPlayer->SendSpectatorAddonMsgToBG(msg);
                    }
                }
            }
            //ABORT();
        } break;

        default:
        {
        } break;
    }

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        (m_caster->ToPlayer())->RemoveGlobalCooldown(m_spellInfo);

    m_caster->RemoveDynObject(m_spellInfo->Id);
    m_caster->RemoveGameObject(m_spellInfo->Id,true);

    //set state back so finish will be processed
    m_spellState = oldState;
    //TC_LOG_DEBUG("FIXME","Spell %u - m_spellState = oldState = %u", m_spellInfo->Id,m_spellState);

    if (GetCaster() && m_spellInfo)
    {
        if (Player *tmpPlayer = GetCaster()->ToPlayer())
        {
            if (tmpPlayer->HaveSpectators())
            {
                SpectatorAddonMsg msg;
                msg.SetPlayer(tmpPlayer->GetName());
                msg.CancelSpell(m_spellInfo->Id);
                tmpPlayer->SendSpectatorAddonMsgToBG(msg);
            }
        }
    }

    finish(false);
}

void Spell::cast(bool skipCheck)
{
    // update pointers base at GUIDs to prevent access to non-existed already object
    if (!UpdatePointers())
    {
        // cancel the spell if UpdatePointers() returned false, something wrong happened there
        cancel();
        return;
    }

    if(Unit *pTarget = m_targets.GetUnitTarget())
    {
        if(!m_IsTriggeredSpell && pTarget->IsAlive() && (pTarget->HasAuraType(SPELL_AURA_MOD_STEALTH) || pTarget->HasAuraType(SPELL_AURA_MOD_INVISIBILITY)) && !pTarget->IsFriendlyTo(m_caster) && !pTarget->IsVisibleForOrDetect(m_caster, true))
        {
            SendCastResult(SPELL_FAILED_BAD_TARGETS);
            finish(false);
            return;
        }

    }
    SetExecutedCurrently(true);
    SpellCastResult castResult = SPELL_CAST_OK;

    // cancel at lost main target unit
    if(!m_targets.GetUnitTarget() && m_targets.GetUnitTargetGUID() && m_targets.GetUnitTargetGUID() != m_caster->GetGUID())
    {
        cancel();
        SetExecutedCurrently(false);
        return;
    }

    if(m_caster->GetTypeId() != TYPEID_PLAYER && m_targets.GetUnitTarget() && m_targets.GetUnitTarget() != m_caster)
        m_caster->SetInFront(m_targets.GetUnitTarget());

    CallScriptBeforeCastHandlers();

    if(!m_IsTriggeredSpell)
    {
        castResult = CheckPower();
        if(castResult != SPELL_CAST_OK)
        {
            SendCastResult(castResult);
            finish(false);
            SetExecutedCurrently(false);
            return;
        }
    }

    // triggered cast called from Spell::prepare where it was already checked
    if(!skipCheck)
    {
        castResult = CheckCast(false);
        //TC_LOG_DEBUG("FIXME","CheckCast for %u : %u", m_spellInfo->Id, castResult);
        if(castResult != SPELL_CAST_OK)
        {
            SendCastResult(castResult);
            finish(false);
            SetExecutedCurrently(false);
            return;
        }
    }

    if (!_spellTargetsSelected)
        SelectSpellTargets();

    // Spell may be finished after target map check
    if(m_spellState == SPELL_STATE_FINISHED)                
    {
        SendInterrupted(0);
        finish(false);
        SetExecutedCurrently(false);
        return;
    }

    CallScriptOnCastHandlers();

    // traded items have trade slot instead of guid in m_itemTargetGUID
    // set to real guid to be sent later to the client
    m_targets.UpdateTradeSlotItem();

    if(!m_IsTriggeredSpell && m_spellInfo->Effects[0].Effect != SPELL_EFFECT_TRIGGER_SPELL_2)
    {
        //TakePower();
        TakeReagents();                                         // we must remove reagents before HandleEffects to allow place crafted item in same slot
    }

    // CAST SPELL
    SendSpellCooldown();

    PrepareScriptHitHandlers();

    HandleLaunchPhase();

    //SendCastResult(castResult);
    SendSpellGo();                                          // we must send smsg_spell_go packet before m_castItem delete in TakeCastItem()...

    //handle SPELL_AURA_ADD_TARGET_TRIGGER auras
    //are there any spells need to be triggered after hit?
    Unit::AuraList const& targetTriggers = m_caster->GetAurasByType(SPELL_AURA_ADD_TARGET_TRIGGER);
    for(Unit::AuraList::const_iterator i = targetTriggers.begin(); i != targetTriggers.end(); ++i)
    {
        SpellInfo const *auraSpellInfo = (*i)->GetSpellInfo();
        uint32 auraSpellIdx = (*i)->GetEffIndex();
        if (IsAffectedBy(auraSpellInfo, auraSpellIdx))
        {
            if(SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(auraSpellInfo->Effects[auraSpellIdx].TriggerSpell))
            {
                // Calculate chance at that moment (can be depend for example from combo points)
                int32 chance = m_caster->CalculateSpellDamage(auraSpellInfo, auraSpellIdx, (*i)->GetBasePoints(), NULL);
                m_ChanceTriggerSpells.push_back(std::make_pair(spellInfo, chance * (*i)->GetStackAmount()));
            }
        }
    }

    if(m_spellInfo->HasAttribute(SPELL_ATTR_CU_CHARGE))
        EffectCharge(0);

    // Okay, everything is prepared. Now we need to distinguish between immediate and evented delayed spells
    if (m_spellInfo->Id == 2094 || m_spellInfo->Id == 14181)       // Delay Blind for 150ms to fake retail lag
    {
        m_immediateHandled = false;
        m_spellState = SPELL_STATE_DELAYED;
        m_delayMoment = uint64(150);
    }

    else if (m_spellInfo->Speed > 0.0f && !m_spellInfo->IsChanneled())
    {
        // Remove used for cast item if need (it can be already NULL after TakeReagents call
        // in case delayed spell remove item at cast delay start
        TakeCastItem();

        // Okay, maps created, now prepare flags
        m_immediateHandled = false;
        m_spellState = SPELL_STATE_DELAYED;
        //TC_LOG_DEBUG("FIXME","Spell %u - SPELL_STATE_DELAYED", m_spellInfo->Id);
        SetDelayStart(0);

        if (m_caster->HasUnitState(UNIT_STATE_CASTING) && !m_caster->IsNonMeleeSpellCast(false, false, true))
            m_caster->ClearUnitState(UNIT_STATE_CASTING);
    }
    else
    {
        // Immediate spell, no big deal
        handle_immediate();
    }

    CallScriptAfterCastHandlers();

    // combo points should not be taken before SPELL_AURA_ADD_TARGET_TRIGGER auras are handled
    if(!m_IsTriggeredSpell)
    {
        TakePower();
    }

    if(m_spellInfo->HasAttribute(SPELL_ATTR_CU_LINK_CAST))
    {
        if(const std::vector<int32> *spell_triggered = sSpellMgr->GetSpellLinked(m_spellInfo->Id))
        {
            for(std::vector<int32>::const_iterator i = spell_triggered->begin(); i != spell_triggered->end(); ++i)
            {
                if(*i < 0)
                    m_caster->RemoveAurasDueToSpell(-(*i));
                else
                {
                    if(m_targets.GetUnitTarget())
                    {
                        for(auto itr : m_UniqueTargetInfo)
                        {
                            Unit* linkCastTarget = ObjectAccessor::GetUnit(*m_caster, itr.targetGUID);
                            if(linkCastTarget)
                                m_caster->CastSpell(linkCastTarget, *i, true);
                        }
                    } else {
                        m_caster->CastSpell(m_caster, *i, true);
                    }
                }
            }
        }
    }

    if ((m_spellInfo->SpellFamilyName == 3 && m_spellInfo->SpellFamilyFlags == 0x400000) // Pyro
            || (m_spellInfo->SpellFamilyName == 3 && m_spellInfo->SpellFamilyFlags == 0x180020)) // Frostbolt
    {
        if (m_caster->ToPlayer() && m_caster->HasAuraEffect(12043))
            m_caster->RemoveAurasDueToSpell(12043);
    }
    
    // Clear spell cooldowns after every spell is cast if .cheat cooldown is enabled.
    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (m_caster->ToPlayer()->GetCommandStatus(CHEAT_COOLDOWN))
            m_caster->ToPlayer()->RemoveSpellCooldown(m_spellInfo->Id, true);
    }

    // sunwell: start combat at cast for delayed spells, only for explicit target
    if (Unit* target = m_targets.GetUnitTarget())
        if (m_caster->GetTypeId() == TYPEID_PLAYER || (m_caster->IsPet() && m_caster->IsControlledByPlayer()))
            if (GetDelayMoment() > 0 && !m_caster->IsFriendlyTo(target) && !m_spellInfo->HasAura(SPELL_AURA_BIND_SIGHT) && (!m_spellInfo->IsPositive() || m_spellInfo->HasEffect(SPELL_EFFECT_DISPEL)))
                if (!m_spellInfo->HasAttribute(SPELL_ATTR3_NO_INITIAL_AGGRO))
                    m_caster->CombatStart(target);
                 //m_caster->CombatStartOnCast(target, !m_spellInfo->HasAttribute(SPELL_ATTR3_NO_INITIAL_AGGRO), GetDelayMoment() + 500); // xinef: increase this time so we dont leave and enter combat in a moment

    SetExecutedCurrently(false);
}

void Spell::handle_immediate()
{
    //TC_LOG_DEBUG("FIXME","Spell %u - handle_immediate()",m_spellInfo->Id);
    // start channeling if applicable
    if(m_spellInfo->IsChanneled())
    {
        int32 duration = m_spellInfo->GetDuration();
        if (duration > 0)
        {
            //apply haste mods
            m_caster->ModSpellCastTime(m_spellInfo, duration, this);
            // Apply duration mod
            if (Player* modOwner = m_caster->GetSpellModOwner())
                modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_DURATION, duration);
            m_spellState = SPELL_STATE_CASTING;
            //TC_LOG_DEBUG("FIXME","Spell %u - SPELL_STATE_CASTING",m_spellInfo->Id);
            m_caster->AddInterruptMask(m_spellInfo->ChannelInterruptFlags);
            SendChannelStart(duration);
        }
        else if (duration == -1)
        {
            m_spellState = SPELL_STATE_CASTING;
            m_caster->AddInterruptMask(m_spellInfo->ChannelInterruptFlags);
            SendChannelStart(duration);
        }
    }

    PrepareTargetProcessing();

    // process immediate effects (items, ground, etc.) also initialize some variables
    _handle_immediate_phase();

    for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
        DoAllEffectOnTarget(&(*ihit));

    for(std::list<GOTargetInfo>::iterator ihit= m_UniqueGOTargetInfo.begin();ihit != m_UniqueGOTargetInfo.end();++ihit)
        DoAllEffectOnTarget(&(*ihit));

    FinishTargetProcessing();

    // spell is finished, perform some last features of the spell here
    _handle_finish_phase();

    // Remove used for cast item if need (it can be already NULL after TakeReagents call
    TakeCastItem();

    // handle ammo consumption for Hunter's volley spell
    if (m_spellInfo->IsRangedWeaponSpell() && m_spellInfo->IsChanneled())
        TakeAmmo();

    if(m_spellState != SPELL_STATE_CASTING)
        finish(true);                                       // successfully finish spell cast (not last in case autorepeat or channel spell)
}

uint64 Spell::handle_delayed(uint64 t_offset)
{
    //TC_LOG_DEBUG("FIXME","Spell %u - handle_delayed(%u)", m_spellInfo->Id,t_offset);
    
    if (!UpdatePointers())
    {
        // finish the spell if UpdatePointers() returned false, something wrong happened there
        finish(false);
        return 0;
    }

    uint64 next_time = m_delayTrajectory;

    PrepareTargetProcessing();

    if (!m_immediateHandled && m_delayTrajectory <= t_offset)
    {
        _handle_immediate_phase();
        m_immediateHandled = true;
        m_delayTrajectory = 0;
        next_time = 0;
    }

    bool single_missile = m_targets.HasDst();

    // now recheck units targeting correctness (need before any effects apply to prevent adding immunity at first effect not allow apply second spell effect and similar cases)
    for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end();++ihit)
    {
        if (ihit->processed == false)
        {
            if (single_missile || ihit->timeDelay <= t_offset)
            {
                ihit->timeDelay = t_offset;
                DoAllEffectOnTarget(&(*ihit));
            }
            else if( next_time == 0 || ihit->timeDelay < next_time )
                next_time = ihit->timeDelay;
        }
    }

    // now recheck gameobject targeting correctness
    for(std::list<GOTargetInfo>::iterator ighit= m_UniqueGOTargetInfo.begin(); ighit != m_UniqueGOTargetInfo.end();++ighit)
    {
        if (ighit->processed == false)
        {
            if ( single_missile || ighit->timeDelay <= t_offset )
                DoAllEffectOnTarget(&(*ighit));
            else if( next_time == 0 || ighit->timeDelay < next_time )
                next_time = ighit->timeDelay;
        }
    }

    FinishTargetProcessing();

    // All targets passed - need finish phase
    if (next_time == 0)
    {
        // spell is finished, perform some last features of the spell here
        _handle_finish_phase();

        finish(true);                                       // successfully finish spell cast

        // return zero, spell is finished now
        return 0;
    }
    else
    {
        // spell is unfinished, return next execution time
        return next_time;
    }
}

void Spell::_handle_immediate_phase()
{
    // initialize Diminishing Returns Data
    m_diminishLevel = DIMINISHING_LEVEL_1;
    m_diminishGroup = DIMINISHING_NONE;

    HandleFlatThreat();

    PrepareScriptHitHandlers();

    m_needSpellLog = IsNeedSendToClient();
    // handle effects with SPELL_EFFECT_HANDLE_HIT mode
    for(uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
    {
        // don't do anything for empty effect
        if (!m_spellInfo->Effects[j].IsEffect())
            continue;

         HandleEffects(m_originalCaster, NULL, NULL, j, SPELL_EFFECT_HANDLE_HIT);

         // Don't do spell log, if is school damage spell
         if (m_spellInfo->Effects[j].Effect == SPELL_EFFECT_SCHOOL_DAMAGE || m_spellInfo->Effects[j].Effect == 0)
             m_needSpellLog = false;
    }

    // process items
    for (std::list<ItemTargetInfo>::iterator ihit = m_UniqueItemInfo.begin(); ihit != m_UniqueItemInfo.end(); ++ihit)
        DoAllEffectOnTarget(&(*ihit));
}

void Spell::_handle_finish_phase()
{
    // spell log
    if(m_needSpellLog)
        SendLogExecute();
}

void Spell::SendSpellCooldown()
{
    if(m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    Player* _player = m_caster->ToPlayer();
    if (!_player)
    {
        // Handle pet cooldowns here if needed instead of in PetAI to avoid hidden cooldown restarts
        Creature* _creature = m_caster->ToCreature();
        if (_creature && _creature->IsPet())
            _creature->AddCreatureSpellCooldown(m_spellInfo->Id);

        return;
    }

    // Add cooldown for max (disable spell)
    // Cooldown started on SendCooldownEvent call
    if (m_spellInfo->Attributes & SPELL_ATTR0_DISABLED_WHILE_ACTIVE)
    {
        _player->AddSpellCooldown(m_spellInfo->Id, 0, time(NULL) - 1);
        return;
    }

    // init cooldown values
    uint32 cat   = 0;
    int32 rec    = -1;
    int32 catrec = -1;

    // some special item spells without correct cooldown in SpellInfo
    // cooldown information stored in item prototype
    // This used in same way in WorldSession::HandleItemQuerySingleOpcode data sending to client.

    if(m_CastItem)
    {
        ItemTemplate const* proto = m_CastItem->GetTemplate();
        if(proto)
        {
            for(int idx = 0; idx < 5; ++idx)
            {
                if(proto->Spells[idx].SpellId == m_spellInfo->Id)
                {
                    cat    = proto->Spells[idx].SpellCategory;
                    rec    = proto->Spells[idx].SpellCooldown;
                    catrec = proto->Spells[idx].SpellCategoryCooldown;
                    break;
                }
            }
        }
    }

    // if no cooldown found above then base at DBC data
    if(rec < 0 && catrec < 0)
    {
        cat = m_spellInfo->GetCategory();
        rec = m_spellInfo->RecoveryTime;
        catrec = m_spellInfo->CategoryRecoveryTime;
    }

    // shoot spells used equipped item cooldown values already assigned in GetAttackTime(RANGED_ATTACK)
    // prevent 0 cooldowns set by another way
    if (rec <= 0 && catrec <= 0 && (cat == 76 || cat == 351))
        rec = _player->GetAttackTime(RANGED_ATTACK);

    // Now we have cooldown data (if found any), time to apply mods
    if(rec > 0)
        _player->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COOLDOWN, rec, this);

    if(catrec > 0)
        _player->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COOLDOWN, catrec, this);

    // replace negative cooldowns by 0
    if (rec < 0) rec = 0;
    if (catrec < 0) catrec = 0;

    // no cooldown after applying spell mods
    if( rec == 0 && catrec == 0)
        return;

    time_t curTime = time(NULL);

    time_t catrecTime = catrec ? curTime+catrec/1000 : 0;   // in secs
    time_t recTime    = rec ? curTime+rec/1000 : catrecTime;// in secs

    // self spell cooldown
    if(recTime > 0)
        _player->AddSpellCooldown(m_spellInfo->Id, m_CastItem ? m_CastItem->GetEntry() : 0, recTime);

    // category spells
    if (catrec > 0)
    {
        SpellCategoryStore::const_iterator i_scstore = sSpellsByCategoryStore.find(cat);
        if (i_scstore != sSpellsByCategoryStore.end())
        {
            for(SpellCategorySet::const_iterator i_scset = i_scstore->second.begin(); i_scset != i_scstore->second.end(); ++i_scset)
            {
                if(*i_scset == m_spellInfo->Id)             // skip main spell, already handled above
                    continue;

                _player->AddSpellCooldown(m_spellInfo->Id, m_CastItem ? m_CastItem->GetEntry() : 0, catrecTime);
            }
        }
    }
}

void Spell::update(uint32 difftime)
{
    //TC_LOG_DEBUG("FIXME","Spell %u - update",m_spellInfo->Id);
    // update pointers based at it's GUIDs
    if (!UpdatePointers())
    {
        // cancel the spell if UpdatePointers() returned false, something wrong happened there
        cancel();
        return;
    }

    if(m_targets.GetUnitTargetGUID() && !m_targets.GetUnitTarget())
    {
        cancel();
        return;
    }
    
    // check if the player caster has moved before the spell finished
    if ((m_caster->GetTypeId() == TYPEID_PLAYER && m_timer != 0) &&
        m_caster->ToPlayer()->GetHasMovedInUpdate() &&
        (m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT) && m_spellState == SPELL_STATE_PREPARING &&
        (m_spellInfo->Effects[0].Effect != SPELL_EFFECT_STUCK || !m_caster->HasUnitMovementFlag(MOVEMENTFLAG_FALLING_FAR)))
    {
        // don't cancel for melee, autorepeat, triggered and instant spells
        if (!IsNextMeleeSwingSpell() && !IsAutoRepeat() && !m_IsTriggeredSpell)
            cancel();
    }

    switch(m_spellState)
    {
        case SPELL_STATE_PREPARING:
        {
            if(m_timer)
            {
                if(difftime >= m_timer)
                    m_timer = 0;
                else
                    m_timer -= difftime;
            }

            if(m_timer == 0 && !IsNextMeleeSwingSpell() && !IsAutoRepeat())
                cast(m_spellInfo->CalcCastTime(this) == 0);
        } break;
        case SPELL_STATE_CASTING:
        {
            if(m_timer > 0)
            {
                if( m_caster->GetTypeId() == TYPEID_PLAYER )
                {
                    // check if player has jumped before the channeling finished
                    if(m_caster->HasUnitMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING))
                        cancel();

                    // check for incapacitating player states
                    //if( m_caster->HasUnitState(UNIT_STATE_STUNNED | UNIT_STATE_CONFUSED))
                    //    cancel();
                }

                // check if there are alive targets left
                if(!UpdateChanneledTargetList())
                {
                    if (m_spellInfo->HasVisual(788) && m_spellInfo->SpellIconID == 113 && m_spellInfo->SpellFamilyName == 5) { // Drain soul exception, must remove aura on caster
                        if (m_caster->m_currentSpells[CURRENT_CHANNELED_SPELL])
                            m_caster->InterruptSpell(CURRENT_CHANNELED_SPELL, true, true);
                    }

                    SendChannelUpdate(0);
                    finish();
                }

                if(difftime >= m_timer)
                    m_timer = 0;
                else
                    m_timer -= difftime;
            }

            if(m_timer == 0)
            {
                // channeled spell processed independently for quest targeting
                // cast at creature (or GO) quest objectives update at successful cast channel finished
                // ignore autorepeat/melee casts for speed (not exist quest for spells (hm... )
                if( m_caster->GetTypeId() == TYPEID_PLAYER && !IsAutoRepeat() && !IsNextMeleeSwingSpell() )
                {
                    for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
                    {
                        TargetInfo* target = &*ihit;
                        if(!IS_CREATURE_GUID(target->targetGUID))
                            continue;

                        Unit* unit = m_caster->GetGUID()==target->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster,target->targetGUID);
                        if (unit==NULL)
                            continue;

                        (m_caster->ToPlayer())->CastedCreatureOrGO(unit->GetEntry(),unit->GetGUID(),m_spellInfo->Id);
                    }

                    for(std::list<GOTargetInfo>::iterator ihit= m_UniqueGOTargetInfo.begin();ihit != m_UniqueGOTargetInfo.end();++ihit)
                    {
                        GOTargetInfo* target = &*ihit;

                        GameObject* go = ObjectAccessor::GetGameObject(*m_caster, target->targetGUID);
                        if(!go)
                            continue;

                        (m_caster->ToPlayer())->CastedCreatureOrGO(go->GetEntry(),go->GetGUID(),m_spellInfo->Id);
                    }
                }

                finish();
            }
        } break;
        default:
        {
        }break;
    }
}

void Spell::finish(bool ok, bool cancelChannel)
{
    //TC_LOG_DEBUG("FIXME","Spell %u - finish(%s)", m_spellInfo->Id, (ok?"true":"false"));
    
    if(!m_caster)
        return;

    if(m_spellState == SPELL_STATE_FINISHED)
        return;

    m_spellState = SPELL_STATE_FINISHED;
    //TC_LOG_DEBUG("FIXME","Spell %u - State : SPELL_STATE_FINISHED",m_spellInfo->Id);

    if(m_spellInfo->IsChanneled() && cancelChannel)
    {
        SendChannelUpdate(0,m_spellInfo->Id);
        m_caster->UpdateInterruptMask();
    }
    
    if(!m_caster->IsNonMeleeSpellCast(false, false, true))
        m_caster->ClearUnitState(UNIT_STATE_CASTING);
    
    if (m_caster->GetTypeId() == TYPEID_UNIT && (m_caster->ToCreature())->IsAIEnabled)
        (m_caster->ToCreature())->AI()->OnSpellFinish(m_caster, m_spellInfo->Id, m_targets.GetUnitTarget(), ok);
        
    if (m_caster->GetTypeId() == TYPEID_UNIT)
    {
        if (m_caster->ToCreature())
        {
            m_caster->ToCreature()->ReleaseFocus(this);
        }
    }

    if(!ok)
    {
        //restore spell mods
        if (m_caster->GetTypeId() == TYPEID_PLAYER)
            (m_caster->ToPlayer())->RestoreSpellMods(this);
        return;
    }
    // other code related only to successfully finished spells

    //remove spell mods
    if (m_caster->GetTypeId() == TYPEID_PLAYER)
        (m_caster->ToPlayer())->RemoveSpellMods(this);

    // Okay to remove extra attacks
    if(IsSpellHaveEffect(m_spellInfo, SPELL_EFFECT_ADD_EXTRA_ATTACKS))
        m_caster->m_extraAttacks = 0;

    // Heal caster for all health leech from all targets
    if (m_healthLeech)
    {
        m_caster->ModifyHealth(m_healthLeech);
        m_caster->SendHealSpellLog(m_caster, m_spellInfo->Id, uint32(m_healthLeech));
    }

    if (IsAutoActionResetSpell())
    {
        if (!(m_spellInfo->HasAttribute(SPELL_ATTR2_NOT_RESET_AUTO_ACTIONS)))
        {
            m_caster->ResetAttackTimer(BASE_ATTACK);
            if (m_caster->HaveOffhandWeapon())
                m_caster->ResetAttackTimer(OFF_ATTACK);
            m_caster->ResetAttackTimer(RANGED_ATTACK);
        }
    }

    // call triggered spell only at successful cast (after clear combo points -> for add some if need)
    // I assume what he means is that some triggered spells may add combo points
    if(!m_TriggerSpells.empty())
        TriggerSpell();

    // Stop Attack for some spells
    if( m_spellInfo->Attributes & SPELL_ATTR0_STOP_ATTACK_TARGET )
        m_caster->AttackStop();
        
    //if (ok && !m_IsTriggeredSpell && m_caster->GetTypeId() == TYPEID_PLAYER && m_caster->HasAuraEffect(43983) && m_spellInfo->PowerType == POWER_MANA)
    //    m_caster->CastSpell(m_caster, 43137, true);
}

void Spell::SendCastResult(SpellCastResult result)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if((m_caster->ToPlayer())->GetSession()->PlayerLoading())  // don't send cast results at loading time
        return;

/* Sunwell
    // Xinef: override every possible result, except for gm fail result... breaks many things and goes unnoticed because of this and makes me rage when i find this out
    if ((_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR) && result != SPELL_FAILED_BM_OR_INVISGOD)
        result = SPELL_FAILED_DONT_REPORT;
    */

    if(result != SPELL_CAST_OK)
    {
        WorldPacket data(SMSG_CAST_FAILED, (4+1+1));
        data << uint32(m_spellInfo->Id);
        data << uint8(result);                              // problem
        data << uint8(m_cast_count);                        // single cast or multi 2.3 (0/1)
        switch (result)
        {
            case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
                data << uint32(m_spellInfo->RequiresSpellFocus);
                break;
            case SPELL_FAILED_REQUIRES_AREA:
                // hardcode areas limitation case
                switch(m_spellInfo->Id)
                {
                    case 41617:                             // Cenarion Mana Salve
                    case 41619:                             // Cenarion Healing Salve
                        data << uint32(3905);
                        break;
                    case 41618:                             // Bottled Nethergon Energy
                    case 41620:                             // Bottled Nethergon Vapor
                        data << uint32(3842);
                        break;
                    case 45373:                             // Bloodberry Elixir
                        data << uint32(4075);
                        break;
                    default:                                // default case
                        data << uint32(m_spellInfo->AreaId);
                        break;
                }
                break;
            case SPELL_FAILED_TOTEMS:
                if(m_spellInfo->Totem[0])
                    data << uint32(m_spellInfo->Totem[0]);
                if(m_spellInfo->Totem[1])
                    data << uint32(m_spellInfo->Totem[1]);
                break;
            case SPELL_FAILED_TOTEM_CATEGORY:
                if(m_spellInfo->TotemCategory[0])
                    data << uint32(m_spellInfo->TotemCategory[0]);
                if(m_spellInfo->TotemCategory[1])
                    data << uint32(m_spellInfo->TotemCategory[1]);
                break;
            case SPELL_FAILED_EQUIPPED_ITEM_CLASS:
                data << uint32(m_spellInfo->EquippedItemClass);
                data << uint32(m_spellInfo->EquippedItemSubClassMask);
                data << uint32(m_spellInfo->EquippedItemInventoryTypeMask);
                break;
        }
        (m_caster->ToPlayer())->SendDirectMessage(&data);
    }
    else
    {
        WorldPacket data(SMSG_CLEAR_EXTRA_AURA_INFO, (8+4));
        data << m_caster->GetPackGUID();
        data << uint32(m_spellInfo->Id);
        (m_caster->ToPlayer())->SendDirectMessage(&data);
    }
}

void Spell::SendSpellStart()
{
    if(!IsNeedSendToClient())
        return;

    uint32 castFlags = CAST_FLAG_UNKNOWN_2;

    if (((m_IsTriggeredSpell && !m_spellInfo->IsAutoRepeatRangedSpell()) || m_triggeredByAuraSpell) && !m_spellInfo->IsChanneled())
        castFlags |= CAST_FLAG_PENDING;

    if (m_spellInfo->HasAttribute(SPELL_ATTR0_RANGED))
        castFlags |= CAST_FLAG_AMMO;

    WorldPacket data(SMSG_SPELL_START, (8+8+4+4+2));
    if(m_CastItem)
        data << m_CastItem->GetPackGUID();
    else
        data << m_caster->GetPackGUID();

    data << m_caster->GetPackGUID();
    data << uint32(m_spellInfo->Id);
    data << uint8(m_cast_count);                            // single cast or multi 2.3 (0/1)
    data << uint16(castFlags);
    data << uint32(m_timer);

    m_targets.Write(data);

#ifdef LICH_KING
    if (castFlags & CAST_FLAG_POWER_LEFT_SELF)
        data << uint32(m_caster->GetPower((Powers)m_spellInfo->PowerType));
#endif

    if( castFlags & CAST_FLAG_AMMO )
        WriteAmmoToPacket(&data);

#ifdef LICH_KING
    if (castFlags & CAST_FLAG_UNKNOWN_23)
    {
        data << uint32(0);
        data << uint32(0);
    }
#endif

    m_caster->SendMessageToSet(&data, true);
}

void Spell::SendSpellGo()
{
    // not send invisible spell casting
    if(!IsNeedSendToClient())
        return;

    uint32 castFlags = CAST_FLAG_UNKNOWN_9;

    // triggered spells with spell visual != 0 and not auto shot
    if( (m_IsTriggeredSpell && !m_spellInfo->IsAutoRepeatRangedSpell()) || m_triggeredByAuraSpell)
        castFlags |= CAST_FLAG_PENDING; 

    if(m_spellInfo->IsRangedWeaponSpell())
        castFlags |= CAST_FLAG_AMMO;                        // arrows/bullets visual

#ifdef LICH_KING
    if ((m_caster->GetTypeId() == TYPEID_PLAYER ||
        (m_caster->GetTypeId() == TYPEID_UNIT && m_caster->IsPet()))
        && m_spellInfo->PowerType != POWER_HEALTH)
        castFlags |= CAST_FLAG_POWER_LEFT_SELF; // should only be sent to self, but the current messaging doesn't make that possible


    if ((m_caster->GetTypeId() == TYPEID_PLAYER)
        && (m_caster->getClass() == CLASS_DEATH_KNIGHT)
        && m_spellInfo->RuneCostID
        && m_spellInfo->PowerType == POWER_RUNE)
    {
        castFlags |= CAST_FLAG_NO_GCD;                       // not needed, but Blizzard sends it
        castFlags |= CAST_FLAG_RUNE_LIST;                    // rune cooldowns list
    }

    if (m_spellInfo->HasEffect(SPELL_EFFECT_ACTIVATE_RUNE))
        castFlags |= CAST_FLAG_RUNE_LIST;                    // rune cooldowns list

    if (m_targets.HasTraj())
        castFlags |= CAST_FLAG_ADJUST_MISSILE;

    if (!m_spellInfo->StartRecoveryTime)
        castFlags |= CAST_FLAG_NO_GCD;

#endif

    WorldPacket data(SMSG_SPELL_GO, 50);                    // guess size
    if(m_CastItem)
        data << m_CastItem->GetPackGUID();
    else
        data << m_caster->GetPackGUID();

    data << m_caster->GetPackGUID();
    data << uint32(m_spellInfo->Id);
    data << uint16(castFlags);
    data << uint32(GetMSTime());                            // timestamp

    WriteSpellGoTargets(&data);

    m_targets.Write(data);

#ifdef LICH_KING

    if (castFlags & CAST_FLAG_POWER_LEFT_SELF)
        data << uint32(m_caster->GetPower((Powers)m_spellInfo->PowerType));

    if (castFlags & CAST_FLAG_RUNE_LIST)                   // rune cooldowns list
    {
        //TODO: There is a crash caused by a spell with CAST_FLAG_RUNE_LIST casted by a creature
        //The creature is the mover of a player, so HandleCastSpellOpcode uses it as the caster
        if (Player* player = m_caster->ToPlayer())
        {
            uint8 runeMaskInitial = m_runesState;
            uint8 runeMaskAfterCast = player->GetRunesState();
            data << uint8(runeMaskInitial);                     // runes state before
            data << uint8(runeMaskAfterCast);                   // runes state after
            for (uint8 i = 0; i < MAX_RUNES; ++i)
            {
                uint8 mask = (1 << i);
                if (mask & runeMaskInitial && !(mask & runeMaskAfterCast))  // usable before andon cooldown now...
                {
                    // float casts ensure the division is performed on floats as we need float result
                    float baseCd = float(player->GetRuneBaseCooldown(i, true));
                    data << uint8((baseCd - float(player->GetRuneCooldown(i))) / baseCd * 255); // rune cooldown passed
                }
            }
        }
    }
    if (castFlags & CAST_FLAG_ADJUST_MISSILE)
    {
        data << m_targets.GetElevation();
        data << uint32(m_delayTrajectory ? m_delayTrajectory : m_delayMoment);
    }
#endif

    if( castFlags & CAST_FLAG_AMMO )
        WriteAmmoToPacket(&data);

#ifdef LICH_KING
    if (castFlags & CAST_FLAG_VISUAL_CHAIN)
    {
        data << uint32(0);
        data << uint32(0);
    }

    if (m_targets.GetTargetMask() & TARGET_FLAG_DEST_LOCATION)
    {
        data << uint8(0);
    }
#endif


    m_caster->SendMessageToSet(&data, true);
}

void Spell::WriteAmmoToPacket( WorldPacket * data )
{
    uint32 ammoInventoryType = 0;
    uint32 ammoDisplayID = 0;

    if (m_spellInfo->Id == 45248) {
        ammoDisplayID = 33069;
        ammoInventoryType = INVTYPE_THROWN;
    }
    else if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        Item *pItem = (m_caster->ToPlayer())->GetWeaponForAttack( RANGED_ATTACK );
        if(pItem)
        {
            ammoInventoryType = pItem->GetTemplate()->InventoryType;
            if( ammoInventoryType == INVTYPE_THROWN )
                ammoDisplayID = pItem->GetTemplate()->DisplayInfoID;
            else
            {
                uint32 ammoID = (m_caster->ToPlayer())->GetUInt32Value(PLAYER_AMMO_ID);
                if(ammoID)
                {
                    ItemTemplate const *pProto = sObjectMgr->GetItemTemplate( ammoID );
                    if(pProto)
                    {
                        ammoDisplayID = pProto->DisplayInfoID;
                        ammoInventoryType = pProto->InventoryType;
                    }
                }
                else if(m_caster->GetDummyAura(46699))      // Requires No Ammo
                {
                    ammoDisplayID = 5996;                   // normal arrow
                    ammoInventoryType = INVTYPE_AMMO;
                }
            }
        }
    }
    else {
        for (uint32 slot = WEAPON_SLOT_MAINHAND; slot <= WEAPON_SLOT_RANGED; ++slot)
        {
#ifdef LICH_KING
            if (uint32 item_id = m_caster->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + i))
            {
                if (ItemEntry const* itemEntry = sItemStore.LookupEntry(item_id))
                {
                    if (itemEntry->Class == ITEM_CLASS_WEAPON)
                    {
                        switch (itemEntry->SubClass)
                        {
                        case ITEM_SUBCLASS_WEAPON_THROWN:
                            ammoDisplayID = itemEntry->DisplayId;
                            ammoInventoryType = itemEntry->InventoryType;
                            break;
                        case ITEM_SUBCLASS_WEAPON_BOW:
                        case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                            ammoDisplayID = 5996;       // is this need fixing?
                            ammoInventoryType = INVTYPE_AMMO;
                            break;
                        case ITEM_SUBCLASS_WEAPON_GUN:
                            ammoDisplayID = 5998;       // is this need fixing?
                            ammoInventoryType = INVTYPE_AMMO;
                            break;
                        }

                        if (ammoDisplayID)
                            break;
                    }
                }
            }
#else
            uint32 EquipSlot = m_caster->GetUInt32Value(UNIT_VIRTUAL_ITEM_INFO + slot*2 + 1);
            switch (EquipSlot)
            {
            case INVTYPE_THROWN:
                ammoInventoryType = INVTYPE_THROWN;
                ammoDisplayID = m_caster->GetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY + slot);
                break;
            case INVTYPE_RANGED:
            case INVTYPE_RANGEDRIGHT:
                Creature* c = m_caster->ToCreature();
                if (!c)
                    break;
                switch (c->GetWeaponSubclass(WeaponSlot(slot)))
                {
                case ITEM_SUBCLASS_WEAPON_BOW:
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    ammoInventoryType = INVTYPE_AMMO;
                    ammoDisplayID = 5996;
                    break;
                case ITEM_SUBCLASS_WEAPON_GUN:
                    ammoDisplayID = 5998;
                    ammoInventoryType = INVTYPE_AMMO;
                    break;
                }
                break;
            }

            if (ammoDisplayID)
                break;
#endif
        }
    }

    *data << uint32(ammoDisplayID);
    *data << uint32(ammoInventoryType);
}

void Spell::WriteSpellGoTargets( WorldPacket * data )
{
    // This function also fill data for channeled spells:
    // m_needAliveTargetMask req for stop channelig if one target die
    for (std::list<TargetInfo>::iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
    {
        if ((*ihit).effectMask == 0)                  // No effect apply - all immuned add state
                                                      // possibly SPELL_MISS_IMMUNE2 for this??
            ihit->missCondition = SPELL_MISS_IMMUNE2;
    }

    // Hit and miss target counts are both uint8, that limits us to 255 targets for each
    // sending more than 255 targets crashes the client (since count sent would be wrong)
    // Spells like 40647 (with a huge radius) can easily reach this limit (spell might need
    // target conditions but we still need to limit the number of targets sent and keeping
    // correct count for both hit and miss).

    uint32 hit = 0;
    size_t hitPos = data->wpos();
    *data << (uint8)0; // placeholder
    for (std::list<TargetInfo>::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end() && hit < 255; ++ihit)
    {
        if ((*ihit).missCondition == SPELL_MISS_NONE)       // Add only hits
        {
            *data << uint64(ihit->targetGUID);
            // Xinef: WTF is this? No channeled spell checked, no anything
            //m_channelTargetEffectMask |=ihit->effectMask;
            ++hit;
        }
    }

    for (std::list<GOTargetInfo>::const_iterator ighit = m_UniqueGOTargetInfo.begin(); ighit != m_UniqueGOTargetInfo.end() && hit < 255; ++ighit)
    {
        *data << uint64(ighit->targetGUID);                 // Always hits
        ++hit;
    }

    uint32 miss = 0;
    size_t missPos = data->wpos();
    *data << (uint8)0; // placeholder
    for (std::list<TargetInfo>::const_iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end() && miss < 255; ++ihit)
    {
        if (ihit->missCondition != SPELL_MISS_NONE)        // Add only miss
        {
            *data << uint64(ihit->targetGUID);
            *data << uint8(ihit->missCondition);
            if (ihit->missCondition == SPELL_MISS_REFLECT)
                *data << uint8(ihit->reflectResult);
            ++miss;
        }
    }
    // Reset m_needAliveTargetMask for non channeled spell
    // Xinef: Why do we reset something that is not set??????
    //if (!m_spellInfo->IsChanneled())
    //    m_channelTargetEffectMask = 0;

    data->put<uint8>(hitPos, (uint8)hit);
    data->put<uint8>(missPos, (uint8)miss);
}

void Spell::SendLogExecute()
{
    Unit *target = m_targets.GetUnitTarget() ? m_targets.GetUnitTarget() : m_caster;

    WorldPacket data(SMSG_SPELLLOGEXECUTE, (8+4+4+4+4+8));

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        data << m_caster->GetPackGUID();
    else
        data << target->GetPackGUID();

    data << uint32(m_spellInfo->Id);
    uint32 count1 = 1;
    data << uint32(count1);                                 // count1 (effect count?)
    for(uint32 i = 0; i < count1; ++i)
    {
        data << uint32(m_spellInfo->Effects[0].Effect);             // spell effect
        uint32 count2 = 1;
        data << uint32(count2);                             // count2 (target count?)
        for(uint32 j = 0; j < count2; ++j)
        {
            switch(m_spellInfo->Effects[0].Effect)
            {
                case SPELL_EFFECT_POWER_DRAIN:
                    if(Unit *unit = m_targets.GetUnitTarget())
                        data << unit->GetPackGUID();
                    else
                        data << uint8(0);
                    data << uint32(0);
                    data << uint32(0);
                    data << float(0);
                    break;
                case SPELL_EFFECT_ADD_EXTRA_ATTACKS:
                    if(Unit *unit = m_targets.GetUnitTarget())
                        data << unit->GetPackGUID();
                    else
                        data << uint8(0);
                    data << uint32(m_caster->m_extraAttacks); 
                    break;
                case SPELL_EFFECT_INTERRUPT_CAST:
                    if(Unit *unit = m_targets.GetUnitTarget())
                        data << unit->GetPackGUID();
                    else
                        data << uint8(0);
                    data << uint32(0);                      // spellid
                    break;
                case SPELL_EFFECT_DURABILITY_DAMAGE:
                    if(Unit *unit = m_targets.GetUnitTarget())
                        data << unit->GetPackGUID();
                    else
                        data << uint8(0);
                    data << uint32(0);
                    data << uint32(0);
                    break;
                case SPELL_EFFECT_OPEN_LOCK:
                case SPELL_EFFECT_OPEN_LOCK_ITEM:
                    if(Item *item = m_targets.GetItemTarget())
                        data << item->GetPackGUID();
                    else
                        data << uint8(0);
                    break;
                case SPELL_EFFECT_CREATE_ITEM:
                    data << uint32(m_spellInfo->Effects[0].ItemType);
                    break;
                case SPELL_EFFECT_SUMMON:
                case SPELL_EFFECT_SUMMON_WILD:
                case SPELL_EFFECT_SUMMON_GUARDIAN:
                case SPELL_EFFECT_TRANS_DOOR:
                case SPELL_EFFECT_SUMMON_PET:
                case SPELL_EFFECT_SUMMON_POSSESSED:
                case SPELL_EFFECT_SUMMON_TOTEM:
                case SPELL_EFFECT_SUMMON_OBJECT_WILD:
                case SPELL_EFFECT_CREATE_HOUSE:
                case SPELL_EFFECT_DUEL:
                case SPELL_EFFECT_SUMMON_TOTEM_SLOT1:
                case SPELL_EFFECT_SUMMON_TOTEM_SLOT2:
                case SPELL_EFFECT_SUMMON_TOTEM_SLOT3:
                case SPELL_EFFECT_SUMMON_TOTEM_SLOT4:
                case SPELL_EFFECT_SUMMON_PHANTASM:
                case SPELL_EFFECT_SUMMON_CRITTER:
                case SPELL_EFFECT_SUMMON_OBJECT_SLOT1:
                case SPELL_EFFECT_SUMMON_OBJECT_SLOT2:
                case SPELL_EFFECT_SUMMON_OBJECT_SLOT3:
                case SPELL_EFFECT_SUMMON_OBJECT_SLOT4:
                case SPELL_EFFECT_SUMMON_DEMON:
                case SPELL_EFFECT_150:
                    if(Unit *unit = m_targets.GetUnitTarget())
                        data << unit->GetPackGUID();
                    else if(m_targets.GetItemTargetGUID())
                        data.appendPackGUID(m_targets.GetItemTargetGUID());
                    else if(GameObject *go = m_targets.GetGOTarget())
                        data << go->GetPackGUID();
                    else
                        data << uint8(0);                   // guid
                    break;
                case SPELL_EFFECT_FEED_PET:
                    data << uint32(m_targets.GetItemTargetEntry());
                    break;
                case SPELL_EFFECT_DISMISS_PET:
                    if(Unit *unit = m_targets.GetUnitTarget())
                        data << unit->GetPackGUID();
                    else
                        data << uint8(0);
                    break;
                default:
                    return;
            }
        }
    }

    m_caster->SendMessageToSet(&data, true);
}

void Spell::SendInterrupted(uint8 result)
{
    WorldPacket data(SMSG_SPELL_FAILURE, (8+4+1));
    data << m_caster->GetPackGUID();
    data << m_spellInfo->Id;
    data << result;
    m_caster->SendMessageToSet(&data, true);

    data.Initialize(SMSG_SPELL_FAILED_OTHER, (8+4));
    data << m_caster->GetPackGUID();
    data << m_spellInfo->Id;
    m_caster->SendMessageToSet(&data, true);
}

void Spell::SendChannelUpdate(uint32 time)
{
    if(time == 0)
    {
        m_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT,0);
        m_caster->SetUInt32Value(UNIT_CHANNEL_SPELL,0);
    }

    WorldPacket data( MSG_CHANNEL_UPDATE, 8+4 );
    data << m_caster->GetPackGUID();
    data << uint32(time);

    m_caster->SendMessageToSet(&data,true);
}

void Spell::SendChannelUpdate(uint32 time, uint32 spellId)
{
    if(m_caster->GetUInt32Value(UNIT_CHANNEL_SPELL) != spellId)
        return;

    Spell::SendChannelUpdate(time);
}

void Spell::SendChannelStart(uint32 duration)
{
    WorldObject* target = NULL;

    // select first not resisted target from target list for _0_ effect
    if(!m_UniqueTargetInfo.empty())
    {
        for(std::list<TargetInfo>::iterator itr= m_UniqueTargetInfo.begin();itr != m_UniqueTargetInfo.end();++itr)
        {
            if( (itr->effectMask & (1<<0)) && itr->reflectResult==SPELL_MISS_NONE && itr->targetGUID != m_caster->GetGUID())
            {
                target = ObjectAccessor::GetUnit(*m_caster, itr->targetGUID);
                break;
            }
        }
    }
    else if(!m_UniqueGOTargetInfo.empty())
    {
        for(std::list<GOTargetInfo>::iterator itr= m_UniqueGOTargetInfo.begin();itr != m_UniqueGOTargetInfo.end();++itr)
        {
            if(itr->effectMask & (1<<0) )
            {
                target = ObjectAccessor::GetGameObject(*m_caster, itr->targetGUID);
                break;
            }
        }
    }

    WorldPacket data( MSG_CHANNEL_START, (8+4+4) );
    data << m_caster->GetPackGUID();
    data << uint32(m_spellInfo->Id);
    data << uint32(duration);

    m_caster->SendMessageToSet(&data,true);

    m_timer = duration;
    if(target)
        m_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, target->GetGUID());
    m_caster->SetUInt32Value(UNIT_CHANNEL_SPELL, m_spellInfo->Id);
}

void Spell::SendResurrectRequest(Player* target)
{
    WorldPacket data(SMSG_RESURRECT_REQUEST, (8+4+2+4));
    data << m_caster->GetGUID();
    data << uint32(1) << uint16(0) << uint32(0);

    target->SendDirectMessage(&data);
}

void Spell::SendPlaySpellVisual(uint32 SpellID)
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 12);
    data << uint64(m_caster->GetGUID());
    data << uint32(SpellID);                                // spell visual id?
    (m_caster->ToPlayer())->SendDirectMessage(&data);
}

bool IsFreeInDuelArea(uint32 item)
{
    switch(item)
    {
    case 5140: //poudre éclipsante
    case 21991: //Bandage épais en tisse-néant
    case 32453: //Larmes de l'étoile
    case 17056: //plume légère
    case 23737: //Adamantite Grenade
    case 23827: //Super-charge de sapeur
    case 32413: //Grenade de givre
    case 24268: //Filet en tisse-néant
        return true;
    default:
        return false;
    }
}

void Spell::TakeCastItem()
{
    if(!m_CastItem || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    // not remove cast item at triggered spell (equipping, weapon damage, etc)
    if(m_IsTriggeredSpell)
        return;

    //Duel area free first aid
    if ( m_caster->ToPlayer()->IsInDuelArea() && IsFreeInDuelArea(m_CastItem->GetEntry()) )
       return;

    ItemTemplate const *proto = m_CastItem->GetTemplate();

    if(!proto)
    {
        // This code is to avoid a crash
        // I'm not sure, if this is really an error, but I guess every item needs a prototype
        TC_LOG_ERROR("FIXME","Cast item has no item prototype highId=%d, lowId=%d",m_CastItem->GetGUIDHigh(), m_CastItem->GetGUIDLow());
        return;
    }

    bool expendable = false;
    bool withoutCharges = false;

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
    {
        if (proto->Spells[i].SpellId)
        {
            // item has limited charges
            if (proto->Spells[i].SpellCharges)
            {
                if (proto->Spells[i].SpellCharges < 0)
                    expendable = true;

                int32 charges = m_CastItem->GetSpellCharges(i);

                // item has charges left
                if (charges)
                {
                    (charges > 0) ? --charges : ++charges;  // abs(charges) less at 1 after use
                    if (proto->Stackable < 2)
                        m_CastItem->SetSpellCharges(i, charges);
                    m_CastItem->SetState(ITEM_CHANGED, m_caster->ToPlayer());
                }

                // all charges used
                withoutCharges = (charges == 0);
            }
        }
    }

    if (expendable && withoutCharges)
    {
        uint32 count = 1;
        (m_caster->ToPlayer())->DestroyItemCount(m_CastItem, count, true);

        // prevent crash at access to deleted m_targets.getItemTarget
        if(m_CastItem==m_targets.GetItemTarget())
            m_targets.SetItemTarget(NULL);

        m_CastItem = NULL;
    }
}

void Spell::TakeAmmo()
{
    if (m_attackType == RANGED_ATTACK && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        Item* pItem = m_caster->ToPlayer()->GetWeaponForAttack(RANGED_ATTACK);

        // wands don't have ammo
        if (!pItem || pItem->IsBroken() || pItem->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_WAND)
            return;

        if (pItem->GetTemplate()->InventoryType == INVTYPE_THROWN)
        {
            if (pItem->GetMaxStackCount() == 1)
            {
                // decrease durability for non-stackable throw weapon
                m_caster->ToPlayer()->DurabilityPointLossForEquipSlot(EQUIPMENT_SLOT_RANGED);
            }
            else
            {
                // decrease items amount for stackable throw weapon
                uint32 count = 1;
                m_caster->ToPlayer()->DestroyItemCount(pItem, count, true);
            }
        }
        else if (uint32 ammo = m_caster->ToPlayer()->GetUInt32Value(PLAYER_AMMO_ID))
            m_caster->ToPlayer()->DestroyItemCount(ammo, 1, true);
    }
}

void Spell::TakePower()
{
    if(m_CastItem || m_triggeredByAuraSpell)
        return;

    //Don't take power if the spell is cast while .cheat power is enabled.
    if (m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (m_caster->ToPlayer()->GetCommandStatus(CHEAT_POWER))
            return;
    }

    bool hit = true;
    if(m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        if(m_spellInfo->PowerType == POWER_RAGE || m_spellInfo->PowerType == POWER_ENERGY)
            if(uint64 targetGUID = m_targets.GetUnitTargetGUID())
                for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
                    if(ihit->targetGUID == targetGUID)
                    {
                        if(ihit->missCondition != SPELL_MISS_NONE && ihit->missCondition != SPELL_MISS_MISS/* && ihit->targetGUID!=m_caster->GetGUID()*/)
                            hit = false;
                        break;
                    }
        if(hit && m_spellInfo->NeedsComboPoints())
            (m_caster->ToPlayer())->ClearComboPoints(m_spellInfo->Id);
    }

    if(!m_powerCost)
        return;

    // health as power used
    if(m_spellInfo->PowerType == POWER_HEALTH)
    {
        m_caster->ModifyHealth( -(int32)m_powerCost );
        return;
    }

    if(m_spellInfo->PowerType >= MAX_POWERS)
    {
        TC_LOG_ERROR("FIXME","Spell::TakePower: Unknown power type '%d'", m_spellInfo->PowerType);
        return;
    }

    Powers powerType = Powers(m_spellInfo->PowerType);

    if(hit)
        m_caster->ModifyPower(powerType, -m_powerCost);
    else
      m_caster->ModifyPower(powerType, -m_caster->GetMap()->irand(0, m_powerCost/4));

    // Set the five second timer
    if (powerType == POWER_MANA && m_powerCost > 0)
        m_caster->SetLastManaUse(GetMSTime());
}

void Spell::TakeReagents()
{
    if(m_IsTriggeredSpell)                                  // reagents used in triggered spell removed by original spell or don't must be removed.
        return;

    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    //Duel area no reagent system
    if (   m_spellInfo->HasAttribute(SPELL_ATTR5_NO_REAGENT_WHILE_PREP) 
        && (m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION) || m_caster->ToPlayer()->IsInDuelArea())        
        )
        return;

    Player* p_caster = m_caster->ToPlayer();

    for(uint32 x=0;x<8;x++)
    {
        if(m_spellInfo->Reagent[x] <= 0)
            continue;

        uint32 itemid = m_spellInfo->Reagent[x];
        uint32 itemcount = m_spellInfo->ReagentCount[x];

        // if CastItem is also spell reagent
        if (m_CastItem)
        {
            ItemTemplate const *proto = m_CastItem->GetTemplate();
            if( proto && proto->ItemId == itemid )
            {
                for(int s=0;s<5;s++)
                {
                    // CastItem will be used up and does not count as reagent
                    int32 charges = m_CastItem->GetSpellCharges(s);
                    if (proto->Spells[s].SpellCharges < 0 && abs(charges) < 2)
                    {
                        ++itemcount;
                        break;
                    }
                }

                m_CastItem = NULL;
            }
        }

        // if getItemTarget is also spell reagent
        if (m_targets.GetItemTargetEntry()==itemid)
            m_targets.SetItemTarget(NULL);

        p_caster->DestroyItemCount(itemid, itemcount, true);
    }
}

void Spell::HandleFlatThreat()
{
    if (m_spellInfo->HasAttribute(SPELL_ATTR1_NO_THREAT))
        return;

    int32 flatMod = sSpellMgr->GetSpellThreatModFlat(m_spellInfo);

    if(flatMod == 0)
        return;

    uint8 targetListSize = m_UniqueTargetInfo.size();
    if(targetListSize == 0)
        return;

    for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
    {
        TargetInfo &target = *ihit;
            Unit* targetUnit = ObjectAccessor::GetUnit(*m_caster, target.targetGUID);
        if (!targetUnit)
            continue;

        float threat = flatMod / targetListSize;;

        //apply threat to every negative targets
        if(!m_spellInfo->IsPositive(!m_caster->IsFriendlyTo(targetUnit)))
            targetUnit->AddThreat(m_caster, threat,(SpellSchoolMask)m_spellInfo->SchoolMask,m_spellInfo);
        else //or assist threat if friendly target
            m_caster->GetHostileRefManager().threatAssist(targetUnit, threat, m_spellInfo);
    }
}

void Spell::HandleEffects(Unit *pUnitTarget,Item *pItemTarget,GameObject *pGOTarget,uint32 i, SpellEffectHandleMode mode)
{
    effectHandleMode = mode;
    unitTarget = pUnitTarget;
    itemTarget = pItemTarget;
    gameObjTarget = pGOTarget;
    destTarget = &m_destTargets[i]._position;

    //Simply return. Do not display "immune" in red text on client
    if(unitTarget && unitTarget->IsImmunedToSpellEffect(m_spellInfo, i))
        return;

    //we do not need DamageMultiplier here.
    damage = CalculateDamage(i, NULL);

    bool preventDefault = false; // CallScriptEffectHandlers((SpellEffIndex)i, mode);

    uint8 eff = m_spellInfo->Effects[i].Effect;
    if(!preventDefault && eff<TOTAL_SPELL_EFFECTS)
    {
        //TC_LOG_DEBUG("FIXME", "WORLD: Spell FX %d < TOTAL_SPELL_EFFECTS ", eff);
        (*this.*SpellEffects[eff])(i);
    }
}

void Spell::TriggerSpell()
{
    for(TriggerSpells::iterator si=m_TriggerSpells.begin(); si!=m_TriggerSpells.end(); ++si)
    {
        Spell* spell = new Spell(m_caster, (*si), true, m_originalCasterGUID, m_selfContainer, true);
        spell->prepare(&m_targets);                         // use original spell original targets
    }
}

//Called with strict at cast start + with not strict at cast end if spell has a cast time
//strict = check for stealth aura + check IsNonMeleeSpellCast
//return : -1 = ok, everything else : see enum SpellCastResult
SpellCastResult Spell::CheckCast(bool strict)
{
    //TC_LOG_DEBUG("FIXME","Spell %u - CheckCast(%s)",m_spellInfo->Id,(strict ? "true" : "false"));

    if (m_spellInfo->Effects[0].Effect == SPELL_EFFECT_STUCK) //skip stuck spell to allow use it in falling case 
        return SPELL_CAST_OK;

    // check cooldowns to prevent cheating
    if(!m_IsTriggeredSpell && m_caster->GetTypeId()==TYPEID_PLAYER && ((m_caster->ToPlayer())->HasSpellCooldown(m_spellInfo->Id) || (strict && (m_caster->ToPlayer())->HasGlobalCooldown(m_spellInfo))))
    {
       //triggered spells shouldn't be casted (cooldown check in handleproctriggerspell)
       // if(m_triggeredByAuraSpell)
       //     return SPELL_FAILED_DONT_REPORT;
       // else
        return SPELL_FAILED_NOT_READY;
    }

     // check death state
    if (   !m_caster->IsAlive() && !(m_spellInfo->Attributes & SPELL_ATTR0_PASSIVE) 
        && !((m_spellInfo->Attributes & SPELL_ATTR0_CASTABLE_WHILE_DEAD) || (m_IsTriggeredSpell && !m_triggeredByAuraSpell)) )
        return SPELL_FAILED_CASTER_DEAD;

    Unit *target = m_targets.GetUnitTarget();
    if(!target)
        target = m_caster;
    
    if(m_caster == target)
        if (m_spellInfo->HasAttribute(SPELL_ATTR1_CANT_TARGET_SELF))
            return SPELL_FAILED_BAD_TARGETS;

    if(target != m_caster)
    {
        if (m_spellInfo->HasAttribute(SPELL_ATTR1_CANT_TARGET_IN_COMBAT))
        {
            if (target->IsInCombat())
                return SPELL_FAILED_TARGET_AFFECTING_COMBAT;
            // player with active pet counts as a player in combat
            else if (Player const* player = target->ToPlayer())
                if (Pet* pet = player->GetPet())
                    if (pet->GetVictim() && !pet->HasUnitState(UNIT_STATE_CONTROLLED))
                        return SPELL_FAILED_TARGET_AFFECTING_COMBAT;
        }

        bool ghostOnly = m_spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_GHOSTS);
        if (ghostOnly != target->HasAuraType(SPELL_AURA_GHOST))
        {
            if (ghostOnly)
                return SPELL_FAILED_TARGET_NOT_GHOST;
            else
                return SPELL_FAILED_BAD_TARGETS;
        }
        
        // Do not allow these spells to target creatures not tapped by us (Banish, Polymorph, many quest spells)
        if (m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo->HasAttribute(SPELL_ATTR2_CANT_TARGET_TAPPED))
            if (Creature const* targetCreature = target->ToCreature())
                if (targetCreature->hasLootRecipient() && !targetCreature->isTappedBy(m_caster->ToPlayer()))
                    return SPELL_FAILED_CANT_CAST_ON_TAPPED;

        // Not allow disarm unarmed player
        if (m_spellInfo->Mechanic == MECHANIC_DISARM)
        {
            if (target->GetTypeId() == TYPEID_PLAYER)
            {
                Player const* player = target->ToPlayer();
                if (!player->GetWeaponForAttack(BASE_ATTACK) || !player->IsUseEquipedWeapon(true))
                    return SPELL_FAILED_TARGET_NO_WEAPONS;
            }
            else if (!unitTarget->HasMainWeapon())
                return SPELL_FAILED_TARGET_NO_WEAPONS;
        }

        // target state requirements (apply to non-self only), to allow cast affects to self like Dirty Deeds
        if(m_spellInfo->TargetAuraState && !target->HasAuraState(AuraStateType(m_spellInfo->TargetAuraState)))
            return SPELL_FAILED_TARGET_AURASTATE;

        if( !(m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS))
            && !(m_spellInfo->HasAttribute(SPELL_ATTR3_UNK25)) //not sure about these
            && !m_caster->IsWithinLOSInMap(target) )
            return SPELL_FAILED_LINE_OF_SIGHT;

        // auto selection spell rank implemented in WorldSession::HandleCastSpellOpcode
        // this case can be triggered if rank not found (too low-level target for first rank)
        if(m_caster->GetTypeId() == TYPEID_PLAYER && !m_spellInfo->IsPassive() && !m_CastItem)
        {
            bool hostileTarget = m_caster->IsHostileTo(target);
            for(int i=0;i<3;i++)
            {
                if(m_spellInfo->IsPositiveEffect(i, hostileTarget) && m_spellInfo->Effects[i].Effect == SPELL_EFFECT_APPLY_AURA)
                    if(target->GetLevel() + 10 < m_spellInfo->SpellLevel)
                        return SPELL_FAILED_LOWLEVEL;
            }
        }

        //check creature type
        //ignore self casts (including area casts when caster selected as target)
        if(!CheckTargetCreatureType(target))
        {
            if(target->GetTypeId()==TYPEID_PLAYER)
                return SPELL_FAILED_TARGET_IS_PLAYER;
            else
                return SPELL_FAILED_BAD_TARGETS;
        }

        // TODO: this check can be applied and for player to prevent cheating when IsPositiveSpell will return always correct result.
        // check target for pet/charmed casts (not self targeted), self targeted cast used for area effects and etc
        if(m_caster->GetTypeId()==TYPEID_UNIT && m_caster->GetCharmerOrOwnerGUID())
        {
            // check correctness positive/negative cast target (pet cast real check and cheating check)
            bool hostileTarget = m_caster->IsHostileTo(target);
            if(m_spellInfo->IsPositive(hostileTarget))
            {
                if(hostileTarget)
                    return SPELL_FAILED_BAD_TARGETS;
            }
            else
            {
                if(m_caster->IsFriendlyTo(target))
                    return SPELL_FAILED_BAD_TARGETS;
            }
        }   

        //Must be behind the target.
        if( (m_spellInfo->HasAttribute(SPELL_ATTR2_BEHIND_TARGET)) && target->HasInArc(M_PI, m_caster))
        {
            SendInterrupted(2);
            return SPELL_FAILED_NOT_BEHIND;
        }

        //Target must be facing you. (TODO : Créer un attribut : ...REQ_TARGET_FACING_CASTER)
        if((m_spellInfo->Attributes == 0x150010) && !target->HasInArc(M_PI, m_caster) )
        {
            SendInterrupted(2);
            return SPELL_FAILED_NOT_INFRONT;
        }

        // check if target is alive?
        
        // Not allow casting on flying player
        if (target->IsInFlight())
            return SPELL_FAILED_BAD_TARGETS;
    } //end "if(target != m_caster)" block

    if(sWorld->getConfig(CONFIG_VMAP_INDOOR_CHECK) && m_caster->GetTypeId() == TYPEID_PLAYER && VMAP::VMapFactory::createOrGetVMapManager()->isLineOfSightCalcEnabled())
    {
        if(m_spellInfo->Attributes & SPELL_ATTR0_OUTDOORS_ONLY && !m_spellInfo->IsPassive() &&
                !m_caster->GetMap()->IsOutdoors(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
            return SPELL_FAILED_ONLY_OUTDOORS;

        if(m_spellInfo->Attributes & SPELL_ATTR0_INDOORS_ONLY && !m_spellInfo->IsPassive() &&
                m_caster->GetMap()->IsOutdoors(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ()))
            return SPELL_FAILED_ONLY_INDOORS;
    }

    if (Player *tmpPlayer = m_caster->ToPlayer())
        if (tmpPlayer->isSpectator())
            return SPELL_FAILED_DONT_REPORT;

    // only check at first call, Stealth auras are already removed at second call
    if( strict )
    {
        // for now (?), ignore triggered spells
        if(!m_IsTriggeredSpell)
        {
            // Cannot be used in this stance/form
            SpellCastResult shapeError = GetErrorAtShapeshiftedCast(m_spellInfo, m_caster->m_form);
            if(shapeError != SPELL_CAST_OK)
                return shapeError;

            if ((m_spellInfo->Attributes & SPELL_ATTR0_ONLY_STEALTHED) && !(m_caster->HasStealthAura()))
                return SPELL_FAILED_ONLY_STEALTHED;
        }
    }
    
    // caster state requirements
    if(m_spellInfo->CasterAuraState && !m_caster->HasAuraState(AuraStateType(m_spellInfo->CasterAuraState)))
        return SPELL_FAILED_CASTER_AURASTATE;
    if(m_spellInfo->CasterAuraStateNot && m_caster->HasAuraState(AuraStateType(m_spellInfo->CasterAuraStateNot)))
        return SPELL_FAILED_CASTER_AURASTATE;

    // cancel autorepeat spells if cast start when moving
    // (not wand currently autorepeat cast delayed to moving stop anyway in spell update code)
    if( m_caster->GetTypeId()==TYPEID_PLAYER && (m_caster->ToPlayer())->isMoving() )
    {
        // apply spell limitations at movement
        if( (!m_caster->HasUnitMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING)) &&
            (IsAutoRepeat() || (m_spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED) != 0) )
            return SPELL_FAILED_MOVING;
    }

    // check spell cast conditions from database
    {
        ConditionSourceInfo condInfo = ConditionSourceInfo(m_caster);
        condInfo.mConditionTargets[1] = m_targets.GetObjectTarget();
        ConditionList conditions = sConditionMgr->GetConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_SPELL, m_spellInfo->Id);
        if (!conditions.empty() && !sConditionMgr->IsObjectMeetToConditions(condInfo, conditions))
        {
            // mLastFailedCondition can be NULL if there was an error processing the condition in Condition::Meets (i.e. wrong data for ConditionTarget or others)
            if (condInfo.mLastFailedCondition && condInfo.mLastFailedCondition->ErrorType)
            {
                /* TC 
                if (condInfo.mLastFailedCondition->ErrorType == SPELL_FAILED_CUSTOM_ERROR)
                    m_customError = SpellCustomErrors(condInfo.mLastFailedCondition->ErrorTextId);
                    */
                return SpellCastResult(condInfo.mLastFailedCondition->ErrorType);
            }
            if (!condInfo.mLastFailedCondition || !condInfo.mLastFailedCondition->ConditionTarget)
                return SPELL_FAILED_CASTER_AURASTATE;
            return SPELL_FAILED_BAD_TARGETS;
        }
    }

    // prevent casting at immune friendly target
    if(m_spellInfo->IsPositive(!m_caster->IsFriendlyTo(target)) && target->IsImmunedToSpell(m_spellInfo))
        return SPELL_FAILED_TARGET_AURASTATE;

    // target state requirements (not allowed state)
    if(m_spellInfo->TargetAuraStateNot && target->HasAuraState(AuraStateType(m_spellInfo->TargetAuraStateNot)))
        return SPELL_FAILED_TARGET_AURASTATE;

    // Cant cast Ice block or Divine shield when under Cyclone
    if ((m_spellInfo->Id == 45438 || m_spellInfo->Id == 642) && m_caster->HasAuraEffect(33786))
        return SPELL_FAILED_CASTER_AURASTATE;

    // Spell casted only on battleground
    if((m_spellInfo->HasAttribute(SPELL_ATTR3_BATTLEGROUND)) &&  m_caster->GetTypeId()==TYPEID_PLAYER)
        if(!(m_caster->ToPlayer())->InBattleground())
            return SPELL_FAILED_ONLY_BATTLEGROUNDS;

    // do not allow spells to be cast in arenas
    // - with greater than 15 min CD without SPELL_ATTR4_USABLE_IN_ARENA flag
    // - with SPELL_ATTR4_NOT_USABLE_IN_ARENA flag
    if( (m_spellInfo->HasAttribute(SPELL_ATTR4_NOT_USABLE_IN_ARENA)) ||
        (m_spellInfo->GetRecoveryTime() > 15 * MINUTE * IN_MILLISECONDS && !(m_spellInfo->HasAttribute(SPELL_ATTR4_USABLE_IN_ARENA))) )
        if(MapEntry const* mapEntry = sMapStore.LookupEntry(m_caster->GetMapId()))
            if(mapEntry->IsBattleArena())
                return SPELL_FAILED_NOT_IN_ARENA;

    // zone check
    if(!IsSpellAllowedInLocation(m_spellInfo,m_caster->GetMapId(),m_caster->GetZoneId(),m_caster->GetAreaId()))
        return SPELL_FAILED_REQUIRES_AREA;

    // not let players cast spells at mount (and let do it to creatures)
    if( m_caster->IsMounted() && m_caster->GetTypeId()==TYPEID_PLAYER && !m_IsTriggeredSpell &&
        !m_spellInfo->IsPassive() && !m_spellInfo->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_MOUNTED) )
    {
        if(m_caster->IsInFlight())
            return SPELL_FAILED_NOT_FLYING;
        else
            return SPELL_FAILED_NOT_MOUNTED;
    }

    // always (except passive spells) check items (focus object can be required for any type casts)
    if(!m_spellInfo->IsPassive())
    {
        SpellCastResult castResult = CheckItems();
        if(castResult != SPELL_CAST_OK)
            return castResult;
    }

    // check combat state
    if(!m_spellInfo->CanBeUsedInCombat())
    {
        if (m_caster->IsInCombat())
            return SPELL_FAILED_AFFECTING_COMBAT;  

        // block non combat spells while we got in air projectiles
        if( !m_IsTriggeredSpell && (m_caster->HasDelayedSpell() || m_caster->m_currentSpells[CURRENT_AUTOREPEAT_SPELL]) )
            return SPELL_FAILED_DONT_REPORT;
    }

    if(!m_IsTriggeredSpell)
    {
        SpellCastResult castResult = CheckRange(strict);
        if(castResult != SPELL_CAST_OK)
            return castResult;

        castResult = CheckPower();
        if(castResult != SPELL_CAST_OK)
            return castResult;

        castResult = CheckCasterAuras();
        if(castResult != SPELL_CAST_OK)
            return castResult;

        // check creature prohibited spell school case
        /*if (m_caster->GetTypeId() == TYPEID_UNIT 
            && m_spellInfo->PreventionType == SPELL_PREVENTION_TYPE_SILENCE
            && m_caster->ToCreature()->IsSpellSchoolMaskProhibited(m_spellInfo->GetSchoolMask()))
            return SPELL_FAILED_NOT_READY;*/

        // only allow triggered spells if at an ended battleground
        if( m_caster->GetTypeId() == TYPEID_PLAYER)
            if(Battleground * bg = (m_caster->ToPlayer())->GetBattleground())
                if(bg->GetStatus() == STATUS_WAIT_LEAVE)
                    return SPELL_FAILED_DONT_REPORT;
    }
        
    // for effects of spells that have only one target
    for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        //Check pet presence
        if(m_spellInfo->Effects[i].TargetA.GetTarget() == TARGET_UNIT_PET)
        {
            Pet* pet = m_caster->GetPet();
            if(!pet)
            {
                if(m_triggeredByAuraSpell)
                    return SPELL_FAILED_DONT_REPORT;
                else
                    return SPELL_FAILED_NO_PET;
            } else if (pet->IsDead()) {
                return SPELL_FAILED_BAD_TARGETS;
            }
        }

        switch(m_spellInfo->Effects[i].Effect)
        {
            case SPELL_EFFECT_ENERGIZE:
            {
                if (m_spellInfo->Id == 32676) {
                    if (!(m_caster->RemoveAurasWithSpellFamily(SPELLFAMILY_PRIEST, 1, false)))
                        return SPELL_FAILED_CASTER_AURASTATE;
                }
                break;
            }
            case SPELL_EFFECT_DUMMY:
            {
                if(m_spellInfo->SpellIconID == 1648)        // Execute
                {
                    if(!m_targets.GetUnitTarget() || m_targets.GetUnitTarget()->GetHealth() > m_targets.GetUnitTarget()->GetMaxHealth()*0.2)
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else if (m_spellInfo->Id == 51582)          // Rocket Boots Engaged
                {
                    if(m_caster->IsInWater())
                        return SPELL_FAILED_ONLY_ABOVEWATER;
                }
                else if(m_spellInfo->SpellIconID==156)      // Holy Shock
                {
                    // spell different for friends and enemies
                    // hart version required facing
                    if(m_targets.GetUnitTarget() && !m_caster->IsFriendlyTo(m_targets.GetUnitTarget()) && !m_caster->HasInArc( M_PI, m_targets.GetUnitTarget() ))
                        return SPELL_FAILED_UNIT_NOT_INFRONT;
                }
                else if (m_spellInfo->Id == 19938)          // Awaken Peon
                {
                    Unit *unit = m_targets.GetUnitTarget();
                    if(!unit || !unit->HasAuraEffect(17743, 0))
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else if (m_spellInfo->Id == 44997) { // Converting Sentry
                    Unit* target = m_targets.GetUnitTarget();
                    if (!target || target->GetEntry() != 24972 || target->IsAlive())
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else if (m_spellInfo->Id == 35771) { // Tag Subbued Talbuk
                    Unit* target = m_targets.GetUnitTarget();
                    if (!target || !target->ToCreature() || !target->ToCreature()->IsBelowHPPercent(20))
                        return SPELL_FAILED_BAD_TARGETS;
                }
                break;
            }
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            {
                // Hammer of Wrath
                if (m_spellInfo->HasVisual(7250))
                {
                    if (!m_targets.GetUnitTarget())
                        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                    if(m_targets.GetUnitTarget()->GetHealth() > m_targets.GetUnitTarget()->GetMaxHealth()*0.2)
                        return SPELL_FAILED_BAD_TARGETS;
                }
                break;
            }
            case SPELL_EFFECT_LEARN_SPELL:
            {
                if(m_spellInfo->Effects[i].TargetA.GetTarget() != TARGET_UNIT_PET)
                    break;

                Pet* pet = m_caster->GetPet();

                if(!pet)
                    return SPELL_FAILED_NO_PET;

                SpellInfo const *learn_spellproto = sSpellMgr->GetSpellInfo(m_spellInfo->Effects[i].TriggerSpell);

                if(!learn_spellproto)
                    return SPELL_FAILED_NOT_KNOWN;

                if(!pet->CanTakeMoreActiveSpells(learn_spellproto->Id))
                    return SPELL_FAILED_TOO_MANY_SKILLS;

                if(m_spellInfo->SpellLevel > pet->GetLevel())
                    return SPELL_FAILED_LOWLEVEL;

                if(!pet->HasTPForSpell(learn_spellproto->Id))
                    return SPELL_FAILED_TRAINING_POINTS;

                break;
            }
            case SPELL_EFFECT_LEARN_PET_SPELL:
            {
                Pet* pet = m_caster->GetPet();

                if(!pet)
                    return SPELL_FAILED_NO_PET;

                SpellInfo const *learn_spellproto = sSpellMgr->GetSpellInfo(m_spellInfo->Effects[i].TriggerSpell);

                if(!learn_spellproto)
                    return SPELL_FAILED_NOT_KNOWN;

                if(!pet->CanTakeMoreActiveSpells(learn_spellproto->Id))
                    return SPELL_FAILED_TOO_MANY_SKILLS;

                if(m_spellInfo->SpellLevel > pet->GetLevel())
                    return SPELL_FAILED_LOWLEVEL;

                if(!pet->HasTPForSpell(learn_spellproto->Id))
                    return SPELL_FAILED_TRAINING_POINTS;

                break;
            }
            case SPELL_EFFECT_FEED_PET:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER || !m_targets.GetItemTarget() )
                    return SPELL_FAILED_BAD_TARGETS;

                Pet* pet = m_caster->GetPet();

                if(!pet)
                    return SPELL_FAILED_NO_PET;

                if(pet->IsDead())
                    return SPELL_FAILED_BAD_TARGETS;

                if(!pet->HaveInDiet(m_targets.GetItemTarget()->GetTemplate()))
                    return SPELL_FAILED_WRONG_PET_FOOD;

                if(!pet->GetCurrentFoodBenefitLevel(m_targets.GetItemTarget()->GetTemplate()->ItemLevel))
                    return SPELL_FAILED_FOOD_LOWLEVEL;

                if(m_caster->IsInCombat() || pet->IsInCombat())
                    return SPELL_FAILED_AFFECTING_COMBAT;

                break;
            }
            case SPELL_EFFECT_POWER_BURN:
            case SPELL_EFFECT_POWER_DRAIN:
            {
                // Can be area effect, Check only for players and not check if target - caster (spell can have multiply drain/burn effects)
                if(m_caster->GetTypeId() == TYPEID_PLAYER)
                    if(Unit* target = m_targets.GetUnitTarget())
                        if(target!=m_caster && target->GetPowerType()!=m_spellInfo->Effects[i].MiscValue)
                            return SPELL_FAILED_BAD_TARGETS;
                break;
            }
            case SPELL_EFFECT_CHARGE:
            {
                if (m_caster->HasUnitState(UNIT_STATE_ROOT))
                    return SPELL_FAILED_ROOTED;

                Unit* target = m_targets.GetUnitTarget();
                if (!target)
                    return SPELL_FAILED_DONT_REPORT;

                Position pos = target->GetFirstWalkableCollisionPosition(target->GetObjectSize(), target->GetRelativeAngle(m_caster));
                delete m_preGeneratedPath; //just in case, if logic changes elsewhere
                m_preGeneratedPath = new PathGenerator(m_caster);
                m_preGeneratedPath->SetPathLengthLimit(m_spellInfo->GetMaxRange(false, m_caster) *1.4f);
                bool result = m_preGeneratedPath->CalculatePath(pos.m_positionX, pos.m_positionY, pos.m_positionZ + target->GetObjectSize(), false, false);

                if (m_preGeneratedPath->GetPathType() & PATHFIND_SHORT) //path found is longer than limit
                    return SPELL_FAILED_OUT_OF_RANGE;

                if(m_preGeneratedPath->GetPathType() & (PATHFIND_NOPATH | PATHFIND_INCOMPLETE) )
                    return SPELL_FAILED_NOPATH;

                //re adjust final position
                if(m_preGeneratedPath->GetPath().size() > 2)
                {
                    uint16 pathSize = m_preGeneratedPath->GetPath().size();
                    G3D::Vector3 beforeLastPointV = m_preGeneratedPath->GetPath()[pathSize-2];
                    const Position beforeLastPointP { beforeLastPointV.x, beforeLastPointV.y, beforeLastPointV.z, 0.0f };
                    const Position newLastPoint = target->GetFirstWalkableCollisionPosition(CONTACT_DISTANCE*3, target->GetRelativeAngle(&beforeLastPointP));
                    //Recreate a path to this point
                    result = m_preGeneratedPath->CalculatePath(newLastPoint.m_positionX, newLastPoint.m_positionY, newLastPoint.m_positionZ + target->GetObjectSize(), false, false);
                }

                if (m_preGeneratedPath->GetPathType() & PATHFIND_SHORT)
                    return SPELL_FAILED_NOPATH; //seems to be always SPELL_FAILED_NOPATH on retail. SPELL_FAILED_OUT_OF_RANGE;
                else if (!result || m_preGeneratedPath->GetPathType() & (PATHFIND_NOPATH|PATHFIND_INCOMPLETE) )
                    return SPELL_FAILED_NOPATH;
                break;
            }
            case SPELL_EFFECT_SKINNING:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER || !m_targets.GetUnitTarget() || m_targets.GetUnitTarget()->GetTypeId() != TYPEID_UNIT)
                    return SPELL_FAILED_BAD_TARGETS;

                if( !(m_targets.GetUnitTarget()->GetUInt32Value(UNIT_FIELD_FLAGS) & UNIT_FLAG_SKINNABLE) )
                    return SPELL_FAILED_TARGET_UNSKINNABLE;

                Creature* creature = m_targets.GetUnitTarget()->ToCreature();
                if ( creature->GetCreatureType() != CREATURE_TYPE_CRITTER && ( !creature->lootForBody || !creature->loot.empty() ) )
                {
                    return SPELL_FAILED_TARGET_NOT_LOOTED;
                }

                uint32 skill = creature->GetCreatureTemplate()->GetRequiredLootSkill();

                int32 skillValue = (m_caster->ToPlayer())->GetSkillValue(skill);
                int32 TargetLevel = m_targets.GetUnitTarget()->GetLevel();
                int32 ReqValue = (skillValue < 100 ? (TargetLevel-10)*10 : TargetLevel*5);
                if (ReqValue > skillValue)
                    return SPELL_FAILED_LOW_CASTLEVEL;

                // chance for fail at orange skinning attempt
                if( (m_selfContainer && (*m_selfContainer) == this) &&
                    skillValue < sWorld->GetConfigMaxSkillValue() &&
                    (ReqValue < 0 ? 0 : ReqValue) > m_caster->GetMap()->irand(skillValue-25, skillValue+37) )
                    return SPELL_FAILED_TRY_AGAIN;

                break;
            }
            case SPELL_EFFECT_OPEN_LOCK_ITEM:
            case SPELL_EFFECT_OPEN_LOCK:
            {
                if( m_spellInfo->Effects[i].TargetA.GetTarget()!= TARGET_GAMEOBJECT_TARGET &&
                    m_spellInfo->Effects[i].TargetA.GetTarget()!= TARGET_GAMEOBJECT_ITEM_TARGET )
                    break;

                if( m_caster->GetTypeId() != TYPEID_PLAYER  // only players can open locks, gather etc.
                    // we need a go target in case of TARGET_GAMEOBJECT_TARGET
                    || (m_spellInfo->Effects[i].TargetA.GetTarget() == TARGET_GAMEOBJECT_TARGET && !m_targets.GetGOTarget())
                    // we need a go target, or an openable item target in case of TARGET_GAMEOBJECT_ITEM_TARGET
                    || (m_spellInfo->Effects[i].TargetA.GetTarget()== TARGET_GAMEOBJECT_ITEM_TARGET && !m_targets.GetGOTarget() &&
                       (!m_targets.GetItemTarget() || !m_targets.GetItemTarget()->GetTemplate()->LockID || m_targets.GetItemTarget()->GetOwner() != m_caster )) )
                    return SPELL_FAILED_BAD_TARGETS;

                // In Battleground players can use only flags and banners
                if( (m_caster->ToPlayer())->InBattleground() &&
                    !(m_caster->ToPlayer())->isAllowUseBattlegroundObject() )
                    return SPELL_FAILED_TRY_AGAIN;

                // get the lock entry
                LockEntry const *lockInfo = NULL;
                if (GameObject* go=m_targets.GetGOTarget())
                    lockInfo = sLockStore.LookupEntry(go->GetLockId());
                else if(Item* itm=m_targets.GetItemTarget())
                    lockInfo = sLockStore.LookupEntry(itm->GetTemplate()->LockID);

                // check lock compatibility
                if (lockInfo)
                {
                    // check for lock - key pair (checked by client also, just prevent cheating
                    bool ok_key = false;
                    for(int it = 0; it < 5; ++it)
                    {
                        switch(lockInfo->Type[it])
                        {
                            case LOCK_KEY_NONE:
                                break;
                            case LOCK_KEY_ITEM:
                            {
                                if(lockInfo->Index[it])
                                {
                                    if(m_CastItem && m_CastItem->GetEntry()==lockInfo->Index[it])
                                        ok_key =true;
                                    break;
                                }
                            }
                            case LOCK_KEY_SKILL:
                            {
                                if(uint32(m_spellInfo->Effects[i].MiscValue)!=lockInfo->Index[it])
                                    break;

                                switch(lockInfo->Index[it])
                                {
                                    case LOCKTYPE_HERBALISM:
                                        if((m_caster->ToPlayer())->HasSkill(SKILL_HERBALISM))
                                            ok_key =true;
                                        break;
                                    case LOCKTYPE_MINING:
                                        if((m_caster->ToPlayer())->HasSkill(SKILL_MINING))
                                            ok_key =true;
                                        break;
                                    default:
                                        ok_key =true;
                                        break;
                                }
                            }
                        }
                        if(ok_key)
                            break;
                    }

                    if(!ok_key)
                        return SPELL_FAILED_BAD_TARGETS;
                }

                // chance for fail at orange mining/herb/LockPicking gathering attempt
                if (!m_selfContainer || ((*m_selfContainer) != this))
                    break;

                // get the skill value of the player
                int32 SkillValue = 0;
                bool canFailAtMax = true;
                if (m_spellInfo->Effects[i].MiscValue == LOCKTYPE_HERBALISM)
                {
                    SkillValue = (m_caster->ToPlayer())->GetSkillValue(SKILL_HERBALISM);
                    canFailAtMax = false;
                }
                else if (m_spellInfo->Effects[i].MiscValue == LOCKTYPE_MINING)
                {
                    SkillValue = (m_caster->ToPlayer())->GetSkillValue(SKILL_MINING);
                    canFailAtMax = false;
                }
                else if (m_spellInfo->Effects[i].MiscValue == LOCKTYPE_PICKLOCK)
                    SkillValue = (m_caster->ToPlayer())->GetSkillValue(SKILL_LOCKPICKING);

                // castitem check: rogue using skeleton keys. the skill values should not be added in this case.
                if(m_CastItem)
                    SkillValue = 0;

                // add the damage modifier from the spell casted (cheat lock / skeleton key etc.) (use m_currentBasePoints, CalculateDamage returns wrong value)
                // TODO: is this a hack?
                SkillValue += m_currentBasePoints[i]+1;

                // get the required lock value
                int32 ReqValue=0;
                if (lockInfo)
                {
                    // check for lock - key pair
                    bool ok = false;
                    for(int it = 0; it < 5; ++it)
                    {
                        if(lockInfo->Type[it]==LOCK_KEY_ITEM && lockInfo->Index[it] && m_CastItem && m_CastItem->GetEntry()==lockInfo->Index[it])
                        {
                            // if so, we're good to go
                            ok = true;
                            break;
                        }
                    }
                    if(ok)
                        break;

                    if (m_spellInfo->Effects[i].MiscValue == LOCKTYPE_PICKLOCK)
                        ReqValue = lockInfo->requiredlockskill;
                    else
                        ReqValue = lockInfo->requiredminingskill;
                }

                // skill doesn't meet the required value
                if (ReqValue > SkillValue)
                    return SPELL_FAILED_LOW_CASTLEVEL;

                // chance for failure in orange gather / lockpick (gathering skill can't fail at maxskill)
                if((canFailAtMax || SkillValue < sWorld->GetConfigMaxSkillValue()) && ReqValue > m_caster->GetMap()->irand(SkillValue-25, SkillValue+37))
                    return SPELL_FAILED_TRY_AGAIN;

                break;
            }
            case SPELL_EFFECT_SUMMON_DEAD_PET:
            {
                Creature *pet = m_caster->GetPet();
                if(!pet)
                    return SPELL_FAILED_NO_PET;

                if(pet->IsAlive())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                break;
            }
            // This is generic summon effect now and don't make this check for summon types similar
            // SPELL_EFFECT_SUMMON_CRITTER, SPELL_EFFECT_SUMMON_WILD or SPELL_EFFECT_SUMMON_GUARDIAN.
            // These won't show up in m_caster->GetPetGUID()
            case SPELL_EFFECT_SUMMON:
            {
                switch(m_spellInfo->Effects[i].MiscValueB)
                {
                    case SUMMON_TYPE_POSESSED:
                    case SUMMON_TYPE_POSESSED2:
                    case SUMMON_TYPE_POSESSED3:
                    case SUMMON_TYPE_DEMON:
                    case SUMMON_TYPE_SUMMON:
                    {
                        if(m_caster->GetPetGUID())
                            return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                        if(m_caster->GetCharmGUID())
                            return SPELL_FAILED_ALREADY_HAVE_CHARM;
                        break;
                    }
                    case SUMMON_TYPE_GUARDIAN:
                        if (m_spellInfo->Id == 13166 && m_caster && m_caster->GetMapId() == 580)
                            return SPELL_FAILED_TRY_AGAIN;
                        break;
                }
                break;
            }
            // Don't make this check for SPELL_EFFECT_SUMMON_CRITTER, SPELL_EFFECT_SUMMON_WILD or SPELL_EFFECT_SUMMON_GUARDIAN.
            // These won't show up in m_caster->GetPetGUID()
            case SPELL_EFFECT_SUMMON_POSSESSED:
            case SPELL_EFFECT_SUMMON_PHANTASM:
            case SPELL_EFFECT_SUMMON_DEMON:
            {
                if(m_caster->GetPetGUID())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                if(m_caster->GetCharmGUID())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                break;
            }
            case SPELL_EFFECT_SUMMON_PET:
            {
                if(m_caster->GetPetGUID())                  //let warlock do a replacement summon
                {

                    Pet* pet = m_caster->GetPet();

                    if (m_caster->GetTypeId()==TYPEID_PLAYER && m_caster->GetClass()==CLASS_WARLOCK)
                    {
                        if (strict)                         //starting cast, trigger pet stun (cast by pet so it doesn't attack player)
                            pet->CastSpell(pet, 32752, true, NULL, NULL, pet->GetGUID());
                    }
                    else
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                }

                if(m_caster->GetCharmGUID())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                break;
            }
            case SPELL_EFFECT_SUMMON_PLAYER:
            {
                if(m_caster->GetTypeId()!=TYPEID_PLAYER)
                    return SPELL_FAILED_BAD_TARGETS;
                if(!(m_caster->ToPlayer())->GetTarget())
                    return SPELL_FAILED_BAD_TARGETS;

                Player* target = sObjectMgr->GetPlayer((m_caster->ToPlayer())->GetTarget());
                if( !target || (m_caster->ToPlayer())==target || !target->IsInSameRaidWith(m_caster->ToPlayer()) )
                    return SPELL_FAILED_BAD_TARGETS;

                // check if our map is dungeon
                if( sMapStore.LookupEntry(m_caster->GetMapId())->IsDungeon() )
                {
                    InstanceTemplate const* instance = sObjectMgr->GetInstanceTemplate(m_caster->GetMapId());
                    if(!instance)
                        return SPELL_FAILED_TARGET_NOT_IN_INSTANCE;
                    if(!target->Satisfy(sObjectMgr->GetAccessRequirement(instance->access_id), m_caster->GetMapId()))
                        return SPELL_FAILED_BAD_TARGETS;
                }
                break;
            }
            case SPELL_EFFECT_LEAP:
            case SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER:
            {
              //Do not allow to cast it before BG starts.
                if(m_caster->GetTypeId()==TYPEID_PLAYER)
                    if(Battleground const *bg = (m_caster->ToPlayer())->GetBattleground())
                        if(bg->GetStatus() != STATUS_IN_PROGRESS)
                            return SPELL_FAILED_TRY_AGAIN;
                break;
            }
            case SPELL_EFFECT_TELEPORT_UNITS:
                {
                    //Do not allow use of Trinket before BG starts
                    if(m_caster->GetTypeId()==TYPEID_PLAYER)
                        if(m_spellInfo->Id == 22563 || m_spellInfo->Id == 22564)
                            if(Battleground const *bg = (m_caster->ToPlayer())->GetBattleground())
                                if(bg->GetStatus() != STATUS_IN_PROGRESS)
                                    return SPELL_FAILED_TRY_AGAIN;
                    break;
                }
            case SPELL_EFFECT_STEAL_BENEFICIAL_BUFF:
            {
                if (m_targets.GetUnitTarget()==m_caster)
                    return SPELL_FAILED_BAD_TARGETS;
                break;
            }
            case SPELL_EFFECT_ENCHANT_ITEM:
            {
                
                if (m_spellInfo->Id == 30260 && m_targets.GetItemTarget() && m_targets.GetItemTarget()->GetTemplate()->ItemLevel < 60)
                    return SPELL_FAILED_LOWLEVEL;
                    
                break;
            }
            case SPELL_EFFECT_TRANS_DOOR:
            {
                if (m_spellInfo->Id == 39161 && m_caster) {
                    Creature* gorgrom = m_caster->FindCreatureInGrid(21514, 10.0f, false);
                    if (!gorgrom)
                        return SPELL_FAILED_TRY_AGAIN;
                }
                break;
            }
            default:break;
        }
    }

    for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        switch(m_spellInfo->Effects[i].ApplyAuraName)
        {
            case SPELL_AURA_DUMMY:
            {
                if(m_spellInfo->Id == 1515)
                {
                    if (!m_targets.GetUnitTarget() || m_targets.GetUnitTarget()->GetTypeId() == TYPEID_PLAYER)
                        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                    if (m_targets.GetUnitTarget()->GetLevel() > m_caster->GetLevel())
                        return SPELL_FAILED_HIGHLEVEL;

                    // use SMSG_PET_TAME_FAILURE?
                    if (!(m_targets.GetUnitTarget()->ToCreature())->GetCreatureTemplate()->isTameable ())
                        return SPELL_FAILED_BAD_TARGETS;

                    if(m_caster->GetPetGUID())
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                    if(m_caster->GetCharmGUID())
                        return SPELL_FAILED_ALREADY_HAVE_CHARM;
                }
            }break;
            case SPELL_AURA_MOD_POSSESS:
            case SPELL_AURA_MOD_CHARM:
            {
                if(m_caster->GetPetGUID())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                if(m_caster->GetCharmGUID())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                if(m_caster->GetCharmerGUID())
                    return SPELL_FAILED_CHARMED;

                // hack SelectSpellTargets is call after this so...
                if (m_spellInfo->Id == 34630) {
                    if (Creature* target = m_caster->FindNearestCreature(19849, 15.0f, true))
                        m_targets.SetUnitTarget(target);
                }
                else if (m_spellInfo->Id == 45839)
                {
                    if (Creature* target = m_caster->FindNearestCreature(25653, 100.0f, true))
                        m_targets.SetUnitTarget(target);
                }

                if(!m_targets.GetUnitTarget())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if(m_targets.GetUnitTarget()->GetCharmerGUID())
                    return SPELL_FAILED_CHARMED;

                if(int32(m_targets.GetUnitTarget()->GetLevel()) > CalculateDamage(i,m_targets.GetUnitTarget()))
                    return SPELL_FAILED_HIGHLEVEL;

                break;
            }
            case SPELL_AURA_MOUNTED:
            {
                if (m_caster->IsInWater())
                    return SPELL_FAILED_ONLY_ABOVEWATER;

                if (m_caster->GetTypeId()==TYPEID_PLAYER && (m_caster->ToPlayer())->GetTransport())
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;

                // Ignore map check if spell have AreaId. AreaId already checked and this prevent special mount spells
                if (m_caster->GetTypeId()==TYPEID_PLAYER && !sMapStore.LookupEntry(m_caster->GetMapId())->IsMountAllowed() && !m_IsTriggeredSpell && !m_spellInfo->AreaId)
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;

                if (m_caster->GetAreaId()==35)
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;

                if (m_caster->IsInDisallowedMountForm())
                    return SPELL_FAILED_NOT_SHAPESHIFT;

                break;
            }
            case SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS:
            {
                if(!m_targets.GetUnitTarget())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                // can be casted at non-friendly unit or own pet/charm
                if(m_caster->IsFriendlyTo(m_targets.GetUnitTarget()))
                    return SPELL_FAILED_TARGET_FRIENDLY;

                break;
            }
            case SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED:
            case SPELL_AURA_FLY:
            {
                // not allow cast fly spells at old maps by players (all spells is self target)
                if(m_caster->GetTypeId()==TYPEID_PLAYER)
                {
                    if( !(m_caster->ToPlayer())->IsGameMaster() &&
                        GetVirtualMapForMapAndZone(m_caster->GetMapId(),m_caster->GetZoneId()) != 530)
                        return SPELL_FAILED_NOT_HERE;
                }
                break;
            }
            case SPELL_AURA_PERIODIC_MANA_LEECH:
            {
                if (!m_targets.GetUnitTarget())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (m_caster->GetTypeId()!=TYPEID_PLAYER || m_CastItem)
                    break;

                if(m_targets.GetUnitTarget()->GetPowerType()!=POWER_MANA)
                    return SPELL_FAILED_BAD_TARGETS;

                break;
            }
            case SPELL_AURA_WATER_WALK:
            {
                if (!m_targets.GetUnitTarget())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (m_targets.GetUnitTarget()->GetTypeId() == TYPEID_PLAYER)
                {
                    Player const* player = m_targets.GetUnitTarget()->ToPlayer();
                    
                    // Player is not allowed to cast water walk on shapeshifted/mounted player 
                    if (player->GetShapeshiftForm() != FORM_NONE || player->IsMounted())
                        return SPELL_FAILED_BAD_TARGETS;
                }
                break;
            }
            case SPELL_AURA_PERIODIC_HEAL:
            {
                if (m_spellInfo->Id == 20631 && m_caster->GetLevel() <= 50)
                    return SPELL_FAILED_LOWLEVEL;
                    
                break;
            }
            case SPELL_AURA_MOD_SCALE:
            {
                if (m_spellInfo->Id == 36310 && m_targets.GetUnitTarget()->HasAuraEffect(36310))
                    return SPELL_FAILED_BAD_TARGETS;
                else if (m_spellInfo->Id == 33111 && !(m_targets.GetUnitTarget()->GetEntry() == 17400 || m_targets.GetUnitTarget()->GetEntry() == 18894))
                    return SPELL_FAILED_BAD_TARGETS;
                    
                break;
            }
            case SPELL_AURA_MOD_SILENCE:
            {
                if (m_spellInfo->Id == 28516) { //Torrent du puits de soleil
                    if (m_targets.GetUnitTarget()->GetTypeId() != TYPEID_UNIT || m_targets.GetUnitTarget()->GetEntry() != 16329)
                        return SPELL_FAILED_BAD_TARGETS;
                }
                break;
            }
            case SPELL_AURA_SCHOOL_IMMUNITY:
            {
                if (m_spellInfo->Id == 1022) { // Benediction of Protection: only on self if stunned (Patch 2.2)
                    if ((m_caster->HasAuraType(SPELL_AURA_MOD_STUN)/* || m_caster->HasAuraType(SPELL_AURA_MOD_CONFUSE)*/) && m_targets.GetUnitTarget()->GetGUID() != m_caster->GetGUID())
                        return SPELL_FAILED_STUNNED;
                }
                break;
            }
            default:
                break;
        }
    }
    
    // check LOS for ground targeted spells
    if (m_targets.HasDst())
        if (!(m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS)) && !m_spellInfo->HasAttribute(SPELL_ATTR5_UNK26)) //SPELL_ATTR5_UNK26 check from sunwell, not 100% sure okay but the spell with
        {
            float x, y, z;
            m_targets.GetDstPos()->GetPosition(x, y, z);

            if (!m_caster->IsWithinLOS(x,y,z))
                return SPELL_FAILED_LINE_OF_SIGHT;
        }


    // check pet presence
    for (int j = 0; j < MAX_SPELL_EFFECTS; ++j)
    {
        if (m_spellInfo->Effects[j].TargetA.GetTarget() == TARGET_UNIT_PET)
        {
            // TC if (!m_caster->GetGuardianPet() && !m_caster->GetCharm())
            if(!m_caster->GetPet())
            {
                if (m_triggeredByAuraSpell)              // not report pet not existence for triggered spells
                    return SPELL_FAILED_DONT_REPORT;
                else
                    return SPELL_FAILED_NO_PET;
            }
            break;
        }

    }

    // all ok
    return SPELL_CAST_OK;
}

SpellCastResult Spell::PetCanCast(Unit* target)
{
    if(!m_caster->IsAlive())
        return SPELL_FAILED_CASTER_DEAD;

    // dead owner (pets still alive when owners ressed?)
    if (Unit* owner = m_caster->GetCharmerOrOwner())
        if (!owner->IsAlive() 
#ifdef LICH_KING
            && m_caster->GetEntry() != 30230 // Rise Ally
#endif
            ) 
            return SPELL_FAILED_CASTER_DEAD;

    if(m_caster->IsNonMeleeSpellCast(false) && !m_IsTriggeredSpell)  //prevent spellcast interruption by another spellcast
        return SPELL_FAILED_SPELL_IN_PROGRESS;
    if(m_caster->IsInCombat() && !m_spellInfo->CanBeUsedInCombat())
        return SPELL_FAILED_AFFECTING_COMBAT;

    if (!target && m_targets.GetUnitTarget())
        target = m_targets.GetUnitTarget();

    if (m_spellInfo->NeedsExplicitUnitTarget())
    {
        if (!target)
            return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
        m_targets.SetUnitTarget(target);
    }

    // cooldown
    if (Creature const* creatureCaster = m_caster->ToCreature())
        if (creatureCaster->HasSpellCooldown(m_spellInfo->Id))
            return SPELL_FAILED_NOT_READY;

    SpellCastResult result = CheckCast(true);
    return result;
}

SpellCastResult Spell::CheckCasterAuras() const
{
    // Flag drop spells totally immuned to caster auras
    // FIXME: find more nice check for all totally immuned spells
    // AttributesEx3 & 0x10000000?
    if(m_spellInfo->Id==23336 || m_spellInfo->Id==23334 || m_spellInfo->Id==34991)
        return SPELL_CAST_OK;

    uint8 school_immune = 0;
    uint32 mechanic_immune = 0;
    uint32 dispel_immune = 0;

    //Check if the spell grants school or mechanic immunity.
    //We use bitmasks so the loop is done only once and not on every aura check below.
    if ( m_spellInfo->HasAttribute(SPELL_ATTR1_DISPEL_AURAS_ON_IMMUNITY) )
    {
        for(int i = 0;i < MAX_SPELL_EFFECTS; i ++)
        {
            if(m_spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_SCHOOL_IMMUNITY)
                school_immune |= uint32(m_spellInfo->Effects[i].MiscValue);
            else if(m_spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MECHANIC_IMMUNITY)
                mechanic_immune |= 1 << uint32(m_spellInfo->Effects[i].MiscValue);
            else if(m_spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_DISPEL_IMMUNITY)
                dispel_immune |= GetDispellMask(DispelType(m_spellInfo->Effects[i].MiscValue));
        }
        //immune movement impairment and loss of control
        if(m_spellInfo->Id==(uint32)42292)
            mechanic_immune = IMMUNE_TO_MOVEMENT_IMPAIRMENT_AND_LOSS_CONTROL_MASK;
    }

    //Check whether the cast should be prevented by any state you might have.
    SpellCastResult prevented_reason = SPELL_CAST_OK;
    // Have to check if there is a stun aura. Otherwise will have problems with ghost aura apply while logging out
    if(!(m_spellInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_STUNNED)) && m_caster->HasAuraType(SPELL_AURA_MOD_STUN))
        prevented_reason = SPELL_FAILED_STUNNED;
    else if(m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED) && !(m_spellInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_CONFUSED)))
        prevented_reason = SPELL_FAILED_CONFUSED;
    else if(m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING) && !(m_spellInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_FEARED)))
        prevented_reason = SPELL_FAILED_FLEEING;
    else if(m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED) && m_spellInfo->PreventionType==SPELL_PREVENTION_TYPE_SILENCE)
        prevented_reason = SPELL_FAILED_SILENCED;
    else if(m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED) && m_spellInfo->PreventionType==SPELL_PREVENTION_TYPE_PACIFY)
        prevented_reason = SPELL_FAILED_PACIFIED;

    // Attr must make flag drop spell totally immune from all effects
    if(prevented_reason)
    {
        if(school_immune || mechanic_immune || dispel_immune)
        {
            //Checking auras is needed now, because you are prevented by some state but the spell grants immunity.
            Unit::AuraMap const& auras = m_caster->GetAuras();
            for(Unit::AuraMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
            {
                if(itr->second)
                {
                    if( GetSpellMechanicMask(itr->second->GetSpellInfo(), itr->second->GetEffIndex()) & mechanic_immune )
                        continue;
                    if( itr->second->GetSpellInfo()->GetSchoolMask() & school_immune )
                        continue;
                    if( (1<<(itr->second->GetSpellInfo()->Dispel)) & dispel_immune)
                        continue;

                    //Make a second check for spell failed so the right SPELL_FAILED message is returned.
                    //That is needed when your casting is prevented by multiple states and you are only immune to some of them.
                    switch(itr->second->GetModifier()->m_auraname)
                    {
                        case SPELL_AURA_MOD_STUN:
                            if (!(m_spellInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_STUNNED)))
                                return SPELL_FAILED_STUNNED;
                            break;
                        case SPELL_AURA_MOD_CONFUSE:
                            if (!(m_spellInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_CONFUSED)))
                                return SPELL_FAILED_CONFUSED;
                            break;
                        case SPELL_AURA_MOD_FEAR:
                            if (!(m_spellInfo->HasAttribute(SPELL_ATTR5_USABLE_WHILE_FEARED)))
                                return SPELL_FAILED_FLEEING;
                            break;
                        case SPELL_AURA_MOD_PACIFY_SILENCE:
                            if( m_spellInfo->PreventionType==SPELL_PREVENTION_TYPE_PACIFY)
                                return SPELL_FAILED_PACIFIED;
                        case SPELL_AURA_MOD_SILENCE:
                           if ( m_spellInfo->PreventionType==SPELL_PREVENTION_TYPE_SILENCE)
                                return SPELL_FAILED_SILENCED;
                           break;
                        case SPELL_AURA_MOD_PACIFY:
                            if( m_spellInfo->PreventionType==SPELL_PREVENTION_TYPE_PACIFY)
                                return SPELL_FAILED_PACIFIED;
                            break;
                    }
                }
            }
        }
        //You are prevented from casting and the spell casted does not grant immunity. Return a failed error.
        else
            return prevented_reason;
    }
    return SPELL_CAST_OK;                                               // all ok
}

bool Spell::CanAutoCast(Unit* target)
{
    if (!target) {
        TC_LOG_ERROR("FIXME","Spell::CanAutoCast: target was NULL!");
        return false;
    }
    
    uint64 targetguid = target->GetGUID();

    for(uint32 j = 0;j<3;j++)
    {
        if(m_spellInfo->Effects[j].Effect == SPELL_EFFECT_APPLY_AURA)
        {
            if( m_spellInfo->StackAmount <= 1)
            {
                if( target->HasAuraEffect(m_spellInfo->Id, j) )
                    return false;
            }
            else
            {
                if( target->GetAuras().count(Unit::spellEffectPair(m_spellInfo->Id, j)) >= m_spellInfo->StackAmount)
                    return false;
            }
        }
        else if (m_spellInfo->Effects[j].IsAreaAuraEffect())
        {
                if( target->HasAuraEffect(m_spellInfo->Id, j) )
                    return false;
        }
    }

    SpellCastResult result = PetCanCast(target);

    if(result == SPELL_CAST_OK || result == SPELL_FAILED_UNIT_NOT_INFRONT)
    {
        SelectSpellTargets();
        //check if among target units, our WANTED target is as well (->only self cast spells return false)
        for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
            if( ihit->targetGUID == targetguid )
                return true;
    }
    return false;                                           //target invalid
}

//check range and facing
SpellCastResult Spell::CheckRange(bool strict)
{
    float max_range = m_spellInfo->GetMaxRange(false, m_caster->GetSpellModOwner(), this);
    float min_range = m_spellInfo->GetMinRange();
    uint32 range_type = 0;
    
    if (m_spellInfo->RangeEntry)
    {
        // self cast doesn't need range checking
        // these are triggered by other spells - possibly we should omit range check in that case?
        if (m_spellInfo->RangeEntry->ID == 1)
            return SPELL_CAST_OK;

        range_type = m_spellInfo->RangeEntry->type;
    }

    Unit *target = m_targets.GetUnitTarget();

    if(target && target != m_caster)
    {
        if(range_type == SPELL_RANGE_MELEE)
        {
            // Because of lag, we can not check too strictly here.
            if(!m_caster->IsWithinMeleeRange(target, max_range/* - 2*MIN_MELEE_REACH*/))
                return SPELL_FAILED_OUT_OF_RANGE;
        }
        else if(!m_caster->IsWithinCombatRange(target, max_range))
            return SPELL_FAILED_OUT_OF_RANGE;               //0x5A;

        if(range_type == SPELL_RANGE_RANGED)
        {
            if(m_caster->IsWithinMeleeRange(target))
                return SPELL_FAILED_TOO_CLOSE;
        }
        else if(min_range && m_caster->IsWithinCombatRange(target, min_range)) // skip this check if min_range = 0
            return SPELL_FAILED_TOO_CLOSE;

        if( m_caster->GetTypeId() == TYPEID_PLAYER &&
            (m_spellInfo->FacingCasterFlags & SPELL_FACING_FLAG_INFRONT) && !m_caster->HasInArc( M_PI, target ) )
            return SPELL_FAILED_UNIT_NOT_INFRONT;
    }

    if (m_targets.HasDst() && !m_targets.HasTraj())
    {
        if (!m_caster->IsWithinDist3d(m_targets.GetDstPos(), max_range))
            return SPELL_FAILED_OUT_OF_RANGE;
        if (min_range && m_caster->IsWithinDist3d(m_targets.GetDstPos(), min_range))
            return SPELL_FAILED_TOO_CLOSE;
    }

    return SPELL_CAST_OK;                                               // ok
}

int32 Spell::CalculatePowerCost()
{
    // item cast not used power
    if(m_CastItem)
        return 0;

    // Spell drain all exist power on cast (Only paladin lay of Hands)
    if (m_spellInfo->HasAttribute(SPELL_ATTR1_DRAIN_ALL_POWER))
    {
        // If power type - health drain all
        if (m_spellInfo->PowerType == POWER_HEALTH)
            return m_caster->GetHealth();
        // Else drain all power
        if (m_spellInfo->PowerType < MAX_POWERS)
            return m_caster->GetPower(Powers(m_spellInfo->PowerType));
        TC_LOG_ERROR("FIXME","Spell::CalculateManaCost: Unknown power type '%d' in spell %d", m_spellInfo->PowerType, m_spellInfo->Id);
        return 0;
    }

    // Base powerCost
    int32 powerCost = m_spellInfo->ManaCost;
    // PCT cost from total amount
    if (m_spellInfo->ManaCostPercentage)
    {
        switch (m_spellInfo->PowerType)
        {
            // health as power used
            case POWER_HEALTH:
                powerCost += m_spellInfo->ManaCostPercentage * m_caster->GetCreateHealth() / 100;
                break;
            case POWER_MANA:
                powerCost += m_spellInfo->ManaCostPercentage * m_caster->GetCreateMana() / 100;
                break;
            case POWER_RAGE:
            case POWER_FOCUS:
            case POWER_ENERGY:
            case POWER_HAPPINESS:
                //            case POWER_RUNES:
                powerCost += m_spellInfo->ManaCostPercentage * m_caster->GetMaxPower(Powers(m_spellInfo->PowerType)) / 100;
                break;
            default:
                TC_LOG_ERROR("FIXME","Spell::CalculateManaCost: Unknown power type '%d' in spell %d", m_spellInfo->PowerType, m_spellInfo->Id);
                return 0;
        }
    }
    SpellSchools school = GetFirstSchoolInMask(m_spellSchoolMask);
    // Flat mod from caster auras by spell school
    powerCost += m_caster->GetInt32Value(UNIT_FIELD_POWER_COST_MODIFIER + school);
    // Shiv - costs 20 + weaponSpeed*10 energy (apply only to non-triggered spell with energy cost)
    if ( m_spellInfo->HasAttribute(SPELL_ATTR4_SPELL_VS_EXTEND_COST) )
        powerCost += m_caster->GetAttackTime(OFF_ATTACK)/100;
    // Apply cost mod by spell
    if(Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_COST, powerCost, this);

    if(m_spellInfo->Attributes & SPELL_ATTR0_LEVEL_DAMAGE_CALCULATION)
        powerCost = int32(powerCost/ (1.117f* m_spellInfo->SpellLevel / m_caster->GetLevel() -0.1327f));

    // PCT mod from user auras by school
    powerCost = int32(powerCost * (1.0f+m_caster->GetFloatValue(UNIT_FIELD_POWER_COST_MULTIPLIER+school)));
    if (powerCost < 0)
        powerCost = 0;
    return powerCost;
}

SpellCastResult Spell::CheckPower()
{
    // item cast not used power
    if(m_CastItem)
        return SPELL_CAST_OK;

    // health as power used - need check health amount
    if(m_spellInfo->PowerType == POWER_HEALTH)
    {
        if(m_caster->GetHealth() <= m_powerCost)
            return SPELL_FAILED_CASTER_AURASTATE;
        return SPELL_CAST_OK;
    }
    // Check valid power type
    if( m_spellInfo->PowerType >= MAX_POWERS )
    {
        TC_LOG_ERROR("FIXME","Spell::CheckMana: Unknown power type '%d'", m_spellInfo->PowerType);
        return SPELL_FAILED_UNKNOWN;
    }
    // Check power amount
    Powers powerType = Powers(m_spellInfo->PowerType);
    if(m_caster->GetPower(powerType) < m_powerCost)
        return SPELL_FAILED_NO_POWER;
    

    return SPELL_CAST_OK;
}

SpellCastResult Spell::CheckItems()
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return SPELL_CAST_OK;

    uint32 itemid, itemcount;
    Player* p_caster = m_caster->ToPlayer();

    if(!m_CastItem)
    {
        if(m_castItemGUID)
            return SPELL_FAILED_ITEM_NOT_READY;
    }
    else
    {
        itemid = m_CastItem->GetEntry();
        if( !p_caster->HasItemCount(itemid,1) )
            return SPELL_FAILED_ITEM_NOT_READY;
        else
        {
            ItemTemplate const *proto = m_CastItem->GetTemplate();
            if(!proto)
                return SPELL_FAILED_ITEM_NOT_READY;

            for (int i = 0; i<5; i++)
            {
                if (proto->Spells[i].SpellCharges)
                {
                    if(m_CastItem->GetSpellCharges(i)==0)
                        return SPELL_FAILED_NO_CHARGES_REMAIN;
                }
            }

            uint32 ItemClass = proto->Class;
            if (ItemClass == ITEM_CLASS_CONSUMABLE && m_targets.GetUnitTarget())
            {
                // such items should only fail if there is no suitable effect at all - see Rejuvenation Potions for example
                SpellCastResult failReason = SPELL_CAST_OK;
                for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
                {
                    // skip check, pet not required like checks, and for TARGET_UNIT_PET m_targets.GetUnitTarget() is not the real target but the caster
                    if (m_spellInfo->Effects[i].TargetA.GetTarget()== TARGET_UNIT_PET)
                        continue;

                    if (m_spellInfo->Effects[i].Effect == SPELL_EFFECT_HEAL)
                    {
                        if (m_targets.GetUnitTarget()->GetHealth() == m_targets.GetUnitTarget()->GetMaxHealth())
                        {
                            failReason = SPELL_FAILED_ALREADY_AT_FULL_HEALTH;
                            continue;
                        }
                        else
                        {
                            failReason = SPELL_CAST_OK;
                            break;
                        }
                    }

                    // Mana Potion, Rage Potion, Thistle Tea(Rogue), ...
                    if (m_spellInfo->Effects[i].Effect == SPELL_EFFECT_ENERGIZE)
                    {
                        if(m_spellInfo->Effects[i].MiscValue < 0 || m_spellInfo->Effects[i].MiscValue >= MAX_POWERS)
                        {
                            failReason = SPELL_FAILED_ALREADY_AT_FULL_POWER;
                            continue;
                        }

                        Powers power = Powers(m_spellInfo->Effects[i].MiscValue);
                        if (m_targets.GetUnitTarget()->GetPower(power) == m_targets.GetUnitTarget()->GetMaxPower(power))
                        {
                            failReason = SPELL_FAILED_ALREADY_AT_FULL_POWER;
                            continue;
                        }
                        else
                        {
                            failReason = SPELL_CAST_OK;
                            break;
                        }
                    }
                }
                if (failReason != SPELL_CAST_OK)
                    return failReason;
            }
        }
    }

    if(m_targets.GetItemTargetGUID())
    {
        if(m_caster->GetTypeId() != TYPEID_PLAYER)
            return SPELL_FAILED_BAD_TARGETS;

        if(!m_targets.GetItemTarget())
            return SPELL_FAILED_ITEM_GONE;

        if(!m_targets.GetItemTarget()->IsFitToSpellRequirements(m_spellInfo)) {
            if (m_spellInfo->Effects[0].Effect == SPELL_EFFECT_ENCHANT_ITEM || m_spellInfo->Effects[0].Effect == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY) {
                m_caster->ToPlayer()->GetSession()->SendNotification("Cet objet n'est pas une cible autorisée.");
                return SPELL_FAILED_DONT_REPORT;
            }
            else
                return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
        }
    }
    // if not item target then required item must be equipped
    else
    {
        if(m_caster->GetTypeId() == TYPEID_PLAYER && !(m_caster->ToPlayer())->HasItemFitToSpellRequirements(m_spellInfo))
            return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
    }

    if(m_spellInfo->RequiresSpellFocus)
    {
        CellCoord p(Trinity::ComputeCellCoord(m_caster->GetPositionX(), m_caster->GetPositionY()));
        Cell cell(p);
        cell.data.Part.reserved = ALL_DISTRICT;

        GameObject* ok = NULL;
        Trinity::GameObjectFocusCheck go_check(m_caster,m_spellInfo->RequiresSpellFocus);
        Trinity::GameObjectSearcher<Trinity::GameObjectFocusCheck> checker(ok,go_check);

        TypeContainerVisitor<Trinity::GameObjectSearcher<Trinity::GameObjectFocusCheck>, GridTypeMapContainer > object_checker(checker);
        Map& map = *m_caster->GetMap();
        cell.Visit(p, object_checker, map, *m_caster, map.GetVisibilityRange());

        if(!ok)
            return SPELL_FAILED_REQUIRES_SPELL_FOCUS;

        focusObject = ok;                                   // game object found in range
    }

    if (!(m_spellInfo->HasAttribute(SPELL_ATTR5_NO_REAGENT_WHILE_PREP) &&
        m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PREPARATION)))
    {
        for(uint32 i=0;i<8;i++)
        {
            if(m_spellInfo->Reagent[i] <= 0)
                continue;

            itemid    = m_spellInfo->Reagent[i];
            itemcount = m_spellInfo->ReagentCount[i];

            // if CastItem is also spell reagent
            if( m_CastItem && m_CastItem->GetEntry() == itemid )
            {
                ItemTemplate const *proto = m_CastItem->GetTemplate();
                if(!proto)
                    return SPELL_FAILED_ITEM_NOT_READY;
                for(int s=0;s<5;s++)
                {
                    // CastItem will be used up and does not count as reagent
                    int32 charges = m_CastItem->GetSpellCharges(s);
                    if (proto->Spells[s].SpellCharges < 0 && abs(charges) < 2)
                    {
                        ++itemcount;
                        break;
                    }
                }
            }
            if( !p_caster->HasItemCount(itemid,itemcount) )
                return SPELL_FAILED_ITEM_NOT_READY;      //0x54
        }
    }

    uint32 totems = 2;
    for(int i=0;i<2;++i)
    {
        if(m_spellInfo->Totem[i] != 0)
        {
            if( p_caster->HasItemCount(m_spellInfo->Totem[i],1) )
            {
                totems -= 1;
                continue;
            }
        }else
        totems -= 1;
    }
    if(totems != 0)
        return SPELL_FAILED_TOTEMS;                  //0x7C

    //Check items for TotemCategory
    uint32 TotemCategory = 2;
    for(int i=0;i<2;++i)
    {
        if(m_spellInfo->TotemCategory[i] != 0)
        {
            if( p_caster->HasItemTotemCategory(m_spellInfo->TotemCategory[i]) )
            {
                TotemCategory -= 1;
                continue;
            }
        }
        else
            TotemCategory -= 1;
    }
    if(TotemCategory != 0)
        return SPELL_FAILED_TOTEM_CATEGORY;          //0x7B

    for(int i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        switch (m_spellInfo->Effects[i].Effect)
        {
            case SPELL_EFFECT_CREATE_ITEM:
            {
                if (!m_IsTriggeredSpell && m_spellInfo->Effects[i].ItemType)
                {
                    ItemPosCountVec dest;
                    uint8 msg = p_caster->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, m_spellInfo->Effects[i].ItemType, 1 );
                    if (msg != EQUIP_ERR_OK )
                    {
                        p_caster->SendEquipError( msg, NULL, NULL );
                        return SPELL_FAILED_DONT_REPORT;
                    }
                }
                break;
            }
            case SPELL_EFFECT_ENCHANT_ITEM:
            {
                Item* targetItem = m_targets.GetItemTarget();
                if(!targetItem)
                    return SPELL_FAILED_ITEM_NOT_FOUND;

                if( targetItem->GetTemplate()->ItemLevel < m_spellInfo->BaseLevel )
                    return SPELL_FAILED_LOWLEVEL;
                // Not allow enchant in trade slot for some enchant type
                if( targetItem->GetOwner() != m_caster )
                {
                    uint32 enchant_id = m_spellInfo->Effects[i].MiscValue;
                    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                    if(!pEnchant)
                        return SPELL_FAILED_ERROR;
                    if (pEnchant->slot & ENCHANTMENT_CAN_SOULBOUND)
                        return SPELL_FAILED_NOT_TRADEABLE;
                }
                break;
            }
            case SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY:
            {
                Item *item = m_targets.GetItemTarget();
                if(!item)
                    return SPELL_FAILED_ITEM_NOT_FOUND;
                // Not allow enchant in trade slot for some enchant type
                if( item->GetOwner() != m_caster )
                {
                    uint32 enchant_id = m_spellInfo->Effects[i].MiscValue;
                    SpellItemEnchantmentEntry const *pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
                    if(!pEnchant)
                        return SPELL_FAILED_ERROR;
                    if (pEnchant->slot & ENCHANTMENT_CAN_SOULBOUND)
                        return SPELL_FAILED_NOT_TRADEABLE;
                }
                break;
            }
            case SPELL_EFFECT_ENCHANT_HELD_ITEM:
                // check item existence in effect code (not output errors at offhand hold item effect to main hand for example
                break;
            case SPELL_EFFECT_DISENCHANT:
            {
                if(!m_targets.GetItemTarget())
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;

                // prevent disenchanting in trade slot
                if( m_targets.GetItemTarget()->GetOwnerGUID() != m_caster->GetGUID() )
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;

                ItemTemplate const* itemProto = m_targets.GetItemTarget()->GetTemplate();
                if(!itemProto)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;

                uint32 item_quality = itemProto->Quality;
                // 2.0.x addon: Check player enchanting level against the item disenchanting requirements
                uint32 item_disenchantskilllevel = itemProto->RequiredDisenchantSkill;
                if (item_disenchantskilllevel == uint32(-1))
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                if (item_disenchantskilllevel > p_caster->GetSkillValue(SKILL_ENCHANTING))
                    return SPELL_FAILED_LOW_CASTLEVEL;
                if(item_quality > 4 || item_quality < 2)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                if(itemProto->Class != ITEM_CLASS_WEAPON && itemProto->Class != ITEM_CLASS_ARMOR)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                if (!itemProto->DisenchantID)
                    return SPELL_FAILED_CANT_BE_DISENCHANTED;
                break;
            }
            case SPELL_EFFECT_PROSPECTING:
            {
                if(!m_targets.GetItemTarget())
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                //ensure item is a prospectable ore
                if(!(m_targets.GetItemTarget()->GetTemplate()->BagFamily & BAG_FAMILY_MASK_MINING_SUPP) || m_targets.GetItemTarget()->GetTemplate()->Class != ITEM_CLASS_TRADE_GOODS)
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                //prevent prospecting in trade slot
                if( m_targets.GetItemTarget()->GetOwnerGUID() != m_caster->GetGUID() )
                    return SPELL_FAILED_CANT_BE_PROSPECTED;
                //Check for enough skill in jewelcrafting
                uint32 item_prospectingskilllevel = m_targets.GetItemTarget()->GetTemplate()->RequiredSkillRank;
                if(item_prospectingskilllevel >p_caster->GetSkillValue(SKILL_JEWELCRAFTING))
                    return SPELL_FAILED_LOW_CASTLEVEL;
                //make sure the player has the required ores in inventory
                if(m_targets.GetItemTarget()->GetCount() < 5)
                    return SPELL_FAILED_PROSPECT_NEED_MORE;

                if(!LootTemplates_Prospecting.HaveLootFor(m_targets.GetItemTargetEntry()))
                    return SPELL_FAILED_CANT_BE_PROSPECTED;

                break;
            }
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            {
                if(m_caster->GetTypeId() != TYPEID_PLAYER) return SPELL_FAILED_TARGET_NOT_PLAYER;
                if( m_attackType != RANGED_ATTACK )
                    break;
                Item *pItem = (m_caster->ToPlayer())->GetWeaponForAttack(m_attackType);
                if(!pItem || pItem->IsBroken())
                    return SPELL_FAILED_EQUIPPED_ITEM;

                switch(pItem->GetTemplate()->SubClass)
                {
                    case ITEM_SUBCLASS_WEAPON_THROWN:
                    {
                        uint32 ammo = pItem->GetEntry();
                        if( !(m_caster->ToPlayer())->HasItemCount( ammo, 1 ) )
                            return SPELL_FAILED_NO_AMMO;
                    };  break;
                    case ITEM_SUBCLASS_WEAPON_GUN:
                    case ITEM_SUBCLASS_WEAPON_BOW:
                    case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    {
                        uint32 ammo = (m_caster->ToPlayer())->GetUInt32Value(PLAYER_AMMO_ID);
                        if(!ammo)
                        {
                            // Requires No Ammo
                            if(m_caster->GetDummyAura(46699))
                                break;                      // skip other checks

                            return SPELL_FAILED_NO_AMMO;
                        }

                        ItemTemplate const *ammoProto = sObjectMgr->GetItemTemplate( ammo );
                        if(!ammoProto)
                            return SPELL_FAILED_NO_AMMO;

                        if(ammoProto->Class != ITEM_CLASS_PROJECTILE)
                            return SPELL_FAILED_NO_AMMO;

                        // check ammo ws. weapon compatibility
                        switch(pItem->GetTemplate()->SubClass)
                        {
                            case ITEM_SUBCLASS_WEAPON_BOW:
                            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                                if(ammoProto->SubClass!=ITEM_SUBCLASS_ARROW)
                                    return SPELL_FAILED_NO_AMMO;
                                break;
                            case ITEM_SUBCLASS_WEAPON_GUN:
                                if(ammoProto->SubClass!=ITEM_SUBCLASS_BULLET)
                                    return SPELL_FAILED_NO_AMMO;
                                break;
                            default:
                                return SPELL_FAILED_NO_AMMO;
                        }

                        if( !(m_caster->ToPlayer())->HasItemCount( ammo, 1 ) )
                            return SPELL_FAILED_NO_AMMO;
                    };  break;
                    case ITEM_SUBCLASS_WEAPON_WAND:
                    default:
                        break;
                }
                break;
            }
            default:break;
        }
    }

    return SPELL_CAST_OK;
}

void Spell::Delayed() // only called in DealDamage()
{
    if(!m_caster)// || m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    //if (m_spellState == SPELL_STATE_DELAYED)
    //    return;                                             // spell is active and can't be time-backed

    // spells not loosing casting time ( slam, dynamites, bombs.. )
    //if(!(m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_DAMAGE))
    //    return;

    //check resist chance
    int32 resistChance = 100;                               //must be initialized to 100 for percent modifiers
    (m_caster->ToPlayer())->ApplySpellMod(m_spellInfo->Id,SPELLMOD_NOT_LOSE_CASTING_TIME,resistChance, this);
    resistChance += m_caster->GetTotalAuraModifier(SPELL_AURA_RESIST_PUSHBACK) - 100;
    if (roll_chance_i(resistChance))
        return;

    int32 delaytime = GetNextDelayAtDamageMsTime();

    if(int32(m_timer) + delaytime > m_casttime)
    {
        delaytime = m_casttime - m_timer;
        m_timer = m_casttime;
    }
    else
        m_timer += delaytime;

    TC_LOG_DEBUG("FIXME","Spell %u partially interrupted for (%d) ms at damage",m_spellInfo->Id,delaytime);

    WorldPacket data(SMSG_SPELL_DELAYED, 8+4);
    data << m_caster->GetPackGUID();
    data << uint32(delaytime);

    m_caster->SendMessageToSet(&data,true);
}

void Spell::DelayedChannel()
{
    if(!m_caster || m_caster->GetTypeId() != TYPEID_PLAYER || getState() != SPELL_STATE_CASTING)
        return;

    //check resist chance
    int32 resistChance = 100;                               //must be initialized to 100 for percent modifiers
    (m_caster->ToPlayer())->ApplySpellMod(m_spellInfo->Id,SPELLMOD_NOT_LOSE_CASTING_TIME,resistChance, this);
    resistChance += m_caster->GetTotalAuraModifier(SPELL_AURA_RESIST_PUSHBACK) - 100;
    if (roll_chance_i(resistChance))
        return;

    int32 delaytime = GetNextDelayAtDamageMsTime();

    if(int32(m_timer) < delaytime)
    {
        delaytime = m_timer;
        m_timer = 0;
    }
    else
        m_timer -= delaytime;

    for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
    {
        if ((*ihit).missCondition == SPELL_MISS_NONE)
        {
            Unit* unit = m_caster->GetGUID()==ihit->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit->targetGUID);
            if (unit)
            {
                for (int j=0;j<3;j++)
                    if( ihit->effectMask & (1<<j) )
                        unit->DelayAura(m_spellInfo->Id, j, delaytime);
            }

        }
    }

    for(int j = 0; j < 3; j++)
    {
        // partially interrupt persistent area auras
        DynamicObject* dynObj = m_caster->GetDynObject(m_spellInfo->Id, j);
        if(dynObj)
            dynObj->Delay(delaytime);
    }

    SendChannelUpdate(m_timer);
}

bool Spell::UpdatePointers()
{
    if(m_originalCasterGUID==m_caster->GetGUID())
        m_originalCaster = m_caster;
    else
    {
        m_originalCaster = ObjectAccessor::GetUnit(*m_caster,m_originalCasterGUID);
        if(m_originalCaster && !m_originalCaster->IsInWorld()) m_originalCaster = NULL;
    }

    if(m_castItemGUID && m_caster->GetTypeId() == TYPEID_PLAYER)
        m_CastItem = (m_caster->ToPlayer())->GetItemByGuid(m_castItemGUID);

    m_targets.Update(m_caster);

#ifdef LICH_KING
    // further actions done only for dest targets
    if (!m_targets.HasDst())
        return true;

    // cache last transport
    WorldObject* transport = NULL;

    // update effect destinations (in case of moved transport dest target)
    for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS; ++effIndex)
    {
        SpellDestination& dest = m_destTargets[effIndex];
        if (!dest._transportGUID)
            continue;

        if (!transport || transport->GetGUID() != dest._transportGUID)
            transport = ObjectAccessor::GetWorldObject(*m_caster, dest._transportGUID);

        if (transport)
        {
            dest._position.Relocate(transport);
            dest._position.RelocateOffset(dest._transportOffset);
        }
    }

#endif
    return true;
}

bool Spell::IsAffectedBy(SpellInfo const *spellInfo, uint32 effectId)
{
    return sSpellMgr->IsAffectedBySpell(m_spellInfo,spellInfo->Id,effectId,spellInfo->Effects[effectId].ItemType);
}

bool Spell::CheckTargetCreatureType(Unit* target) const
{
    uint32 spellCreatureTargetMask = m_spellInfo->TargetCreatureType;

    // Curse of Doom : not find another way to fix spell target check :/
    if(m_spellInfo->SpellFamilyName==SPELLFAMILY_WARLOCK && m_spellInfo->SpellFamilyFlags == 0x0200000000LL)
    {
        // not allow cast at player
        if(target->GetTypeId()==TYPEID_PLAYER)
            return false;

        spellCreatureTargetMask = 0x7FF;
    }

    // Dismiss Pet and Taming Lesson skipped
    if(m_spellInfo->Id == 2641 || m_spellInfo->Id == 23356)
        spellCreatureTargetMask =  0;
    
    // Grounding Totem and Polymorph spells
    if (target->GetEntry() == 5925 && m_spellInfo->SpellFamilyName == SPELLFAMILY_MAGE && m_spellInfo->SpellFamilyFlags == 0x1000000)
        return true;

    if (spellCreatureTargetMask)
    {
        uint32 TargetCreatureType = target->GetCreatureTypeMask();

        return !TargetCreatureType || (spellCreatureTargetMask & TargetCreatureType);
    }
    return true;
}

CurrentSpellTypes Spell::GetCurrentContainer()
{
    if (IsNextMeleeSwingSpell())
        return(CURRENT_MELEE_SPELL);
    else if (IsAutoRepeat())
        return(CURRENT_AUTOREPEAT_SPELL);
    else if (m_spellInfo->IsChanneled())
        return(CURRENT_CHANNELED_SPELL);
    else
        return(CURRENT_GENERIC_SPELL);
}

bool Spell::CheckEffectTarget(Unit const* target, uint32 eff) const
{
    switch (m_spellInfo->Effects[eff].ApplyAuraName)
    {
    case SPELL_AURA_MOD_POSSESS:
    case SPELL_AURA_MOD_CHARM:
    case SPELL_AURA_MOD_POSSESS_PET:
    case SPELL_AURA_AOE_CHARM:
#ifdef LICH_KING
        if (target->GetTypeId() == TYPEID_UNIT && target->IsVehicle())
            return false;
#endif
        if (target->IsMounted())
            return false;
        if (target->GetCharmerGUID())
            return false;

        if (int32 damage = m_caster->CalculateSpellDamage(m_spellInfo, eff, m_spellInfo->Effects[eff].BasePoints, target))
            if ((int32)target->GetLevel() > damage)
                return false;
        break;
    default:
        break;
    }

    // xinef: skip los checking if spell has appropriate attribute, or target requires specific entry
    // this is only for target addition and target has to have unselectable flag, this is valid for FLAG_EXTRA_TRIGGER and quest triggers however there are some without this flag, used not_selectable
    if (m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS) || (target->GetTypeId() == TYPEID_UNIT && target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE) && (m_spellInfo->Effects[eff].TargetA.GetCheckType() == TARGET_CHECK_ENTRY || m_spellInfo->Effects[eff].TargetB.GetCheckType() == TARGET_CHECK_ENTRY)))
        return true;

    if (IsTriggered())
    {
        if (!m_caster->IsInMap(target)) // pussywizard: crashfix, avoid IsWithinLOS on another map! >_>
            return true;

        float x = m_caster->GetPositionX(), y = m_caster->GetPositionY(), z = m_caster->GetPositionZ();
        if (m_targets.HasDst())
        {
            x = m_targets.GetDstPos()->GetPositionX();
            y = m_targets.GetDstPos()->GetPositionY();
            z = m_targets.GetDstPos()->GetPositionZ();
        }

        // Ignore LoS for totems with positive effects, check it in all other cases
        bool casterIsTotem = (m_caster->GetTypeId() == TYPEID_UNIT && m_caster->ToCreature()->IsTotem());
        if ((!casterIsTotem || !m_spellInfo->IsPositive()) && !target->IsWithinLOS(x, y, z))
            return false;

        return true;
    }

    // todo: shit below shouldn't be here, but it's temporary
    //Check targets for LOS visibility (except spells without range limitations)
    switch (m_spellInfo->Effects[eff].Effect)
    {
    case SPELL_EFFECT_RESURRECT_NEW:
        // player far away, maybe his corpse near?
        if (target != m_caster && !target->IsWithinLOSInMap(m_caster))
        {
            if (!m_targets.GetCorpseTargetGUID())
                return false;

            Corpse* corpse = ObjectAccessor::GetCorpse(*m_caster, m_targets.GetCorpseTargetGUID());
            if (!corpse)
                return false;

            if (target->GetGUID() != corpse->GetOwnerGUID())
                return false;

            if (!corpse->IsWithinLOSInMap(m_caster))
                return false;
        }
        break;
    case SPELL_EFFECT_SKIN_PLAYER_CORPSE:
    {
        if (!m_targets.GetCorpseTargetGUID())
        {
            if (target->IsWithinLOSInMap(m_caster) && target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
                return true;

            return false;
        }

        Corpse* corpse = ObjectAccessor::GetCorpse(*m_caster, m_targets.GetCorpseTargetGUID());
        if (!corpse)
            return false;

        if (target->GetGUID() != corpse->GetOwnerGUID())
            return false;

        if (!corpse->HasFlag(CORPSE_FIELD_FLAGS, CORPSE_FLAG_LOOTABLE))
            return false;

        if (!corpse->IsWithinLOSInMap(m_caster))
            return false;
    }
    break;
    /*case SPELL_EFFECT_CHARGE:
    if (MMAP::MMapFactory::IsPathfindingEnabled(m_caster->FindMap()))
    break;*/
    // else no break intended
    default:                                            // normal case
                                                        // Get GO cast coordinates if original caster -> GO
        WorldObject* caster = NULL;
        if (IS_GAMEOBJECT_GUID(m_originalCasterGUID))
            caster = m_caster->GetMap()->GetGameObject(m_originalCasterGUID);
        if (!caster)
            caster = m_caster;
        if (target != caster)
        {
            float x = caster->GetPositionX(), y = caster->GetPositionY(), z = caster->GetPositionZ();
            if (m_targets.HasDst())
            {
                x = m_targets.GetDstPos()->GetPositionX();
                y = m_targets.GetDstPos()->GetPositionY();
                z = m_targets.GetDstPos()->GetPositionZ();
            }

            if (!target->IsInMap(caster) || !target->IsWithinLOS(x, y, z))
                return false;
        }
        break;
    }

    return true;
}

bool Spell::CheckTarget(Unit* target, uint32 eff)
{
    // Check targets for creature type mask and remove not appropriate (skip explicit self target case, maybe need other explicit targets)
    if(m_spellInfo->Effects[eff].TargetA.GetTarget() != TARGET_UNIT_CASTER)
    {
        if (!CheckTargetCreatureType(target))
            return false;
    }

    // Check targets for not_selectable unit flag and remove
    // A player can cast spells on his pet (or other controlled unit) though in any state
    if (target != m_caster && target->GetCharmerOrOwnerGUID() != m_caster->GetGUID())
    {
        // any unattackable target skipped
        if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE)
            && !sSpellMgr->IsNearbyEntryEffect(m_spellInfo,eff))
            return false;

        // unselectable targets skipped in all cases except TARGET_UNIT_NEARBY_ENTRY targeting
        // in case TARGET_UNIT_NEARBY_ENTRY target selected by server always and can't be cheated
        /*if( target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE) &&
            m_spellInfo->Effects[eff].TargetA.GetTarget()!= TARGET_UNIT_NEARBY_ENTRY &&
            m_spellInfo->Effects[eff].TargetB->GetTarget() != TARGET_UNIT_NEARBY_ENTRY )
            return false;*/
    }

    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        if (target->ToPlayer()->isSpectator())
        {
            if (m_caster->GetTypeId() == TYPEID_PLAYER && m_caster->ToPlayer()->IsGameMaster())
                return true;
            else
                return false;
        }
    }

    //Check player targets and remove if in GM mode or GM invisibility (for not self casting case)
    if( target != m_caster && target->GetTypeId()==TYPEID_PLAYER)
    {
        if((target->ToPlayer())->GetVisibility()==VISIBILITY_OFF)
            return false;

        if((target->ToPlayer())->IsGameMaster() && !m_spellInfo->IsPositive())
            return false;
    }

    //Do not check LOS for triggered spells
    if( (m_IsTriggeredSpell)
      || (m_spellInfo->HasAttribute(SPELL_ATTR3_UNK25)) //not sure about these
      || (m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS)) )
        return true;

    //Check targets for LOS visibility (except spells without range limitations )
    switch(m_spellInfo->Effects[eff].Effect)
    {
        case SPELL_EFFECT_SUMMON_PLAYER:                    // from anywhere
            break;
        case SPELL_EFFECT_DUMMY:
            if(m_spellInfo->Id!=20577)                      // Cannibalize
                break;
            //fall through
        case SPELL_EFFECT_RESURRECT_NEW:
            // player far away, maybe his corpse near?
            if(target!=m_caster && !target->IsWithinLOSInMap(m_caster))
            {
                if(!m_targets.GetCorpseTargetGUID())
                    return false;

                Corpse *corpse = ObjectAccessor::GetCorpse(*m_caster,m_targets.GetCorpseTargetGUID());
                if(!corpse)
                    return false;

                if(target->GetGUID()!=corpse->GetOwnerGUID())
                    return false;

                if(!corpse->IsWithinLOSInMap(m_caster))
                    return false;
            }

            // all ok by some way or another, skip normal check
            break;
        default:                                            // normal case
            if(target!=m_caster && !target->IsWithinLOSInMap(m_caster))
                return false;
            break;
    }

    return true;
}

Unit* Spell::SelectMagnetTarget()
{
    Unit* target = m_targets.GetUnitTarget();

    if(target && m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MAGIC && target->HasAuraType(SPELL_AURA_SPELL_MAGNET))
    {
        Unit::AuraList const& magnetAuras = target->GetAurasByType(SPELL_AURA_SPELL_MAGNET);
        for(Unit::AuraList::const_iterator itr = magnetAuras.begin(); itr != magnetAuras.end(); ++itr)
        {
            if(Unit* magnet = (*itr)->GetCaster())
            {
                if((*itr)->m_procCharges>0)
                {
                    (*itr)->SetCharges((*itr)->m_procCharges-1);
                    target = magnet;
                    m_targets.SetUnitTarget(target);
                    AddUnitTarget(target, 0);
                    uint64 targetGUID = target->GetGUID();
                    for(std::list<TargetInfo>::iterator ihit= m_UniqueTargetInfo.begin();ihit != m_UniqueTargetInfo.end();++ihit)
                    {
                        if (targetGUID == ihit->targetGUID)                 // Found in list
                        {
                            (*ihit).damage = target->GetHealth();
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    return target;
}

void Spell::HandleHitTriggerAura()
{
    Unit* target = m_targets.GetUnitTarget();

    if(target && m_spellInfo->DmgClass != SPELL_DAMAGE_CLASS_MAGIC && target->HasAuraType(SPELL_AURA_ADD_CASTER_HIT_TRIGGER))
    {
        Unit::AuraList const& hitTriggerAuras = target->GetAurasByType(SPELL_AURA_ADD_CASTER_HIT_TRIGGER);
        for(Unit::AuraList::const_iterator itr = hitTriggerAuras.begin(); itr != hitTriggerAuras.end(); ++itr)
        {
            if(Unit* hitTarget = (*itr)->GetCaster())
            {
                if((*itr)->m_procCharges>0)
                {
                    (*itr)->SetCharges((*itr)->m_procCharges-1);
                    target = hitTarget;
                    m_targets.SetUnitTarget(target);
                    AddUnitTarget(target, 0);
                    return;
                }
            }
        }
    }
}

bool Spell::IsNeedSendToClient() const
{
    return m_spellInfo->SpellVisual!=0 || m_spellInfo->IsChanneled() ||
        m_spellInfo->Speed > 0.0f || (!m_triggeredByAuraSpell && !m_IsTriggeredSpell);
}

bool Spell::HaveTargetsForEffect( uint8 effect ) const
{
    for(std::list<TargetInfo>::const_iterator itr= m_UniqueTargetInfo.begin();itr != m_UniqueTargetInfo.end();++itr)
        if(itr->effectMask & (1<<effect))
            return true;

    for(std::list<GOTargetInfo>::const_iterator itr= m_UniqueGOTargetInfo.begin();itr != m_UniqueGOTargetInfo.end();++itr)
        if(itr->effectMask & (1<<effect))
            return true;

    for(std::list<ItemTargetInfo>::const_iterator itr= m_UniqueItemInfo.begin();itr != m_UniqueItemInfo.end();++itr)
        if(itr->effectMask & (1<<effect))
            return true;

    return false;
}

SpellEvent::SpellEvent(Spell* spell) : BasicEvent()
{
    m_Spell = spell;
}

SpellEvent::~SpellEvent()
{
    if (m_Spell->getState() != SPELL_STATE_FINISHED)
        m_Spell->cancel();

    if (m_Spell->IsDeletable())
    {
        delete m_Spell;
    }
    else
    {
        TC_LOG_ERROR("spells", "~SpellEvent: %s %u tried to delete non-deletable spell %u. Was not deleted, causes memory leak.",
            (m_Spell->GetCaster()->GetTypeId()==TYPEID_PLAYER?"Player":"Creature"), m_Spell->GetCaster()->GetGUIDLow(), m_Spell->m_spellInfo->Id);
    }
}

bool SpellEvent::Execute(uint64 e_time, uint32 p_time)
{
    // update spell if it is not finished
    if (m_Spell->getState() != SPELL_STATE_FINISHED)
        m_Spell->update(p_time);

    // check spell state to process
    switch (m_Spell->getState())
    {
        case SPELL_STATE_FINISHED:
        {
            // spell was finished, check deletable state
            if (m_Spell->IsDeletable())
            {
                // check, if we do have unfinished triggered spells

                return(true);                               // spell is deletable, finish event
            }
            // event will be re-added automatically at the end of routine)
        } break;

        case SPELL_STATE_DELAYED:
        {
            // first, check, if we have just started
            if (m_Spell->GetDelayStart() != 0)
            {
                // no, we aren't, do the typical update
                // check, if we have channeled spell on our hands
                if (m_Spell->m_spellInfo->IsChanneled())
                {
                    // evented channeled spell is processed separately, casted once after delay, and not destroyed till finish
                    // check, if we have casting anything else except this channeled spell and autorepeat
                    if (m_Spell->GetCaster()->IsNonMeleeSpellCast(false, true, true))
                    {
                        // another non-melee non-delayed spell is casted now, abort
                        m_Spell->cancel();
                    }
                    // Check if target of channeled spell still in range
                    else if (m_Spell->CheckRange(false) != SPELL_CAST_OK)
                        m_Spell->cancel();
                    else
                    {
                        // do the action (pass spell to channeling state)
                        m_Spell->handle_immediate();
                    }
                    // event will be re-added automatically at the end of routine)
                }
                else
                {
                    // run the spell handler and think about what we can do next
                    uint64 t_offset = e_time - m_Spell->GetDelayStart();
                    uint64 n_offset = m_Spell->handle_delayed(t_offset);
                    if (n_offset)
                    {
                        // re-add us to the queue
                        m_Spell->GetCaster()->m_Events.AddEvent(this, m_Spell->GetDelayStart() + n_offset, false);
                        return(false);                      // event not complete
                    }
                    // event complete
                    // finish update event will be re-added automatically at the end of routine)
                }
            }
            else
            {
                // delaying had just started, record the moment
                m_Spell->SetDelayStart(e_time);
                // re-plan the event for the delay moment
                m_Spell->GetCaster()->m_Events.AddEvent(this, e_time + m_Spell->GetDelayMoment(), false);
                return(false);                              // event not complete
            }
        } break;

        default:
        {
            // all other states
            // event will be re-added automatically at the end of routine)
        } break;
    }

    // spell processing not complete, plan event on the next update interval
    m_Spell->GetCaster()->m_Events.AddEvent(this, e_time + 1, false);
    return(false);                                          // event not complete
}

void SpellEvent::Abort(uint64 /*e_time*/)
{
    // oops, the spell we try to do is aborted
    if (m_Spell->getState() != SPELL_STATE_FINISHED)
        m_Spell->cancel();
}

bool SpellEvent::IsDeletable() const
{
    return m_Spell->IsDeletable();
}


bool ReflectEvent::Execute(uint64 e_time, uint32 p_time)
{
    Unit* caster = ObjectAccessor::FindUnit(_casterGUID);
    Unit* target = ObjectAccessor::FindUnit(_targetGUID);
    if (caster && target && caster->IsInMap(target))
    {
        // Start triggers for remove charges if need (trigger only for victim, and mark as active spell)
        caster->ProcDamageAndSpell(target, PROC_FLAG_NONE, PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT, PROC_EX_REFLECT, 1, BASE_ATTACK, _spellInfo);
        // FIXME: Add a flag on unit itself, not to setRemoveReflect if unit is already flagged for it (prevent infinite delay on reflect lolz)
        if (Spell* sp = caster->m_currentSpells[CURRENT_CHANNELED_SPELL])
            sp->setRemoveReflect();
        else if (Spell* sp = caster->m_currentSpells[CURRENT_GENERIC_SPELL])
            sp->setRemoveReflect();
    }
    return true;
}

bool Spell::IsValidDeadOrAliveTarget(Unit const* target) const
{
    return m_spellInfo->IsValidDeadOrAliveTarget(target);
}


void Spell::HandleLaunchPhase()
{
    // handle effects with SPELL_EFFECT_HANDLE_LAUNCH mode
    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        // don't do anything for empty effect
        if (!m_spellInfo->Effects[i].IsEffect())
            continue;

        HandleEffects(NULL, NULL, NULL, i, SPELL_EFFECT_HANDLE_LAUNCH);
    }

    float multiplier[MAX_SPELL_EFFECTS];
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (m_applyMultiplierMask & (1 << i))
            multiplier[i] = m_spellInfo->Effects[i].CalcDamageMultiplier(m_originalCaster, this);

    bool usesAmmo = m_spellInfo->HasAttribute(SPELL_ATTR_CU_DIRECT_DAMAGE);
#ifdef LICH_KING
    Unit::AuraEffectList const& Auras = m_caster->GetAuraEffectsByType(SPELL_AURA_ABILITY_CONSUME_NO_AMMO);
    for (Unit::AuraEffectList::const_iterator j = Auras.begin(); j != Auras.end(); ++j)
    {
        if ((*j)->IsAffectedOnSpell(m_spellInfo))
            usesAmmo = false;
    }
#endif

    bool firstTarget = true;
    for (std::list<TargetInfo>::iterator ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end(); ++ihit)
    {
        TargetInfo& target = *ihit;

        uint32 mask = target.effectMask;
        if (!mask)
            continue;

        // do not consume ammo anymore for Hunter's volley spell
        if (IsTriggered() && m_spellInfo->SpellFamilyName == SPELLFAMILY_HUNTER && m_spellInfo->IsTargetingArea())
            usesAmmo = false;

        if (usesAmmo)
        {
            bool ammoTaken = false;
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; i++)
            {
                if (!(mask & 1 << i))
                    continue;
                switch (m_spellInfo->Effects[i].Effect)
                {
                case SPELL_EFFECT_SCHOOL_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE:
                case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
                case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                    ammoTaken = true;
                    TakeAmmo();
                }
                if (ammoTaken)
                    break;
            }
        }
        DoAllEffectOnLaunchTarget(target, multiplier, firstTarget);
        firstTarget = false;
    }
}

void Spell::DoAllEffectOnLaunchTarget(TargetInfo& targetInfo, float* multiplier, bool firstTarget)
{
    Unit* unit = NULL;
    // In case spell hit target, do all effect on that target
    if (targetInfo.missCondition == SPELL_MISS_NONE)
        unit = m_caster->GetGUID() == targetInfo.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, targetInfo.targetGUID);
    // In case spell reflect from target, do all effect on caster (if hit)
    else if (targetInfo.missCondition == SPELL_MISS_REFLECT && targetInfo.reflectResult == SPELL_MISS_NONE)
        unit = m_caster;
    if (!unit)
        return;

    uint8 ssEffect = MAX_SPELL_EFFECTS;
    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (targetInfo.effectMask & (1 << i))
        {
            m_damage = 0;
            m_healing = 0;

            HandleEffects(unit, NULL, NULL, i, SPELL_EFFECT_HANDLE_LAUNCH_TARGET);

            if (m_damage > 0)
            {
                // Xinef: Area Auras, AoE Targetting spells AND Chain Target spells (cleave etc.)
                if (m_spellInfo->Effects[i].IsAreaAuraEffect() || m_spellInfo->Effects[i].IsTargetingArea() || (m_spellInfo->Effects[i].ChainTarget > 1 && m_spellInfo->DmgClass != SPELL_DAMAGE_CLASS_MAGIC))
                {
                    m_damage = unit->CalculateAOEDamageReduction(m_damage, m_spellInfo->SchoolMask, m_caster);
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        uint32 targetAmount = m_UniqueTargetInfo.size();
                        if (targetAmount > 10)
                            m_damage = m_damage * 10 / targetAmount;
                    }
                }
            }

            if (m_applyMultiplierMask & (1 << i))
            {
                m_damage = int32(m_damage * m_damageMultipliers[i]);
                m_damageMultipliers[i] *= multiplier[i];
            }
            targetInfo.damage += m_damage;

            // Sweeping Strikes
            switch (m_spellInfo->Effects[i].Effect)
            {
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE:
            case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
            case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                ssEffect = i;
                break;
            }
        }
    }

    // xinef: totem's inherit owner crit chance and dancing rune weapon
    Unit* caster = m_caster;
    if ((m_caster->GetTypeId() == TYPEID_UNIT && m_caster->ToCreature()->IsTotem()) || m_caster->GetEntry() == 27893)
    {
        if (Unit* owner = m_caster->GetOwner())
            caster = owner;
    }
    else if (m_originalCaster)
        caster = m_originalCaster;

    /* TC spells
    float critChance = caster->SpellDoneCritChance(unit, m_spellInfo, m_spellSchoolMask, m_attackType, false);
    critChance = unit->SpellTakenCritChance(caster, m_spellInfo, m_spellSchoolMask, critChance, m_attackType, false);
    targetInfo.crit = roll_chance_f(std::max(0.0f, critChance));
    */
    targetInfo.crit = m_caster->IsSpellCrit(unit, m_spellInfo, m_spellSchoolMask, m_attackType);

    /* sunwell
    // Sweeping strikes wtf shit ;d
    if (m_caster->GetClass() == CLASS_WARRIOR && ssEffect < MAX_SPELL_EFFECTS && m_spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR &&
        ((m_spellInfo->Id != 50622 && m_spellInfo->Id != 44949) || firstTarget))
    {
        if (Aura* aur = m_caster->GetAura(12328))
        {
            Unit* target = m_caster->SelectNearbyNoTotemTarget(unit);
            if (!target || target == unit)
                return; // finish

            int32 mdmg = m_damage;
            int32 basepoints = 0;
            m_damage = 0;

            HandleEffects(target, NULL, NULL, ssEffect, SPELL_EFFECT_HANDLE_LAUNCH_TARGET);

            basepoints = (targetInfo.crit ? Unit::SpellCriticalDamageBonus(m_caster, m_spellInfo, m_damage, target) : m_damage);
            m_damage = mdmg;
            m_caster->CastCustomSpell(target, 26654, &basepoints, NULL, NULL, true);

            if (m_spellInfo->Id != 44949)
                aur->DropCharge();
        }
    }
    */
}

void Spell::SetSpellValue(SpellValueMod mod, int32 value)
{
    switch(mod)
    {
        case SPELLVALUE_BASE_POINT0:
            m_spellValue->EffectBasePoints[0] = value - int32(m_spellInfo->Effects[0].BaseDice);
            m_currentBasePoints[0] = m_spellValue->EffectBasePoints[0]; //this should be removed in the future
            break;
        case SPELLVALUE_BASE_POINT1:
            m_spellValue->EffectBasePoints[1] = value - int32(m_spellInfo->Effects[1].BaseDice);
            m_currentBasePoints[1] = m_spellValue->EffectBasePoints[1];
            break;
        case SPELLVALUE_BASE_POINT2:
            m_spellValue->EffectBasePoints[2] = value - int32(m_spellInfo->Effects[2].BaseDice);
            m_currentBasePoints[2] = m_spellValue->EffectBasePoints[2];
            break;
        case SPELLVALUE_MAX_TARGETS:
            m_spellValue->MaxAffectedTargets = (uint32)value;
            break;
        case SPELLVALUE_MOD_RADIUS:
            m_spellValue->RadiusMod = (float)value / 10000;
            break;
    }
}

void Spell::PrepareTargetProcessing()
{
  //TC spells  CheckEffectExecuteData();
}

void Spell::FinishTargetProcessing()
{
   //TC spells  SendLogExecute();
}

bool Spell::DoesApplyAuraName(uint32 name)
{
    return (m_spellInfo->Effects[0].ApplyAuraName == name || m_spellInfo->Effects[1].ApplyAuraName == name || m_spellInfo->Effects[2].ApplyAuraName == name);
}

bool Spell::HasGlobalCooldown()
{
    // global cooldown have only player or controlled units
    if (m_caster->GetCharmInfo())
        return m_caster->GetCharmInfo()->GetGlobalCooldownMgr().HasGlobalCooldown(m_spellInfo);
    /*else if (m_caster->GetTypeId() == TYPEID_PLAYER)
        return ((Player*)m_caster)->GetGlobalCooldownMgr().HasGlobalCooldown(m_spellInfo);*/
    else
        return false;
}

void Spell::TriggerGlobalCooldown()
{
    int32 gcd = m_spellInfo->StartRecoveryTime;
    if (!gcd) {
        if (m_caster->GetCharmInfo())
            gcd = 1000;
        else
            return;
    }

    // global cooldown can't leave range 1..1.5 secs (if it it)
    // exist some spells (mostly not player directly casted) that have < 1 sec and > 1.5 sec global cooldowns
    // but its as test show not affected any spell mods.
    if (m_spellInfo->StartRecoveryTime >= 1000 && m_spellInfo->StartRecoveryTime <= 1500)
    {
        // gcd modifier auras applied only to self spells and only player have mods for this
        /*if (m_caster->GetTypeId() == TYPEID_PLAYER)
            ((Player*)m_caster)->ApplySpellMod(m_spellInfo->Id, SPELLMOD_GLOBAL_COOLDOWN, gcd, this);*/

        // apply haste rating
        gcd = int32(float(gcd) * m_caster->GetFloatValue(UNIT_MOD_CAST_SPEED));

        if (gcd < 1000)
            gcd = 1000;
        else if (gcd > 1500)
            gcd = 1500;
    }

    // global cooldown have only player or controlled units
    if (m_caster->GetCharmInfo())
        m_caster->GetCharmInfo()->GetGlobalCooldownMgr().AddGlobalCooldown(m_spellInfo, gcd);
    /*else if (m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_caster)->GetGlobalCooldownMgr().AddGlobalCooldown(m_spellInfo, gcd);*/
}

void Spell::CancelGlobalCooldown()
{
    if (!m_spellInfo->StartRecoveryTime)
        return;

    // cancel global cooldown when interrupting current cast
    if (m_caster->m_currentSpells[CURRENT_GENERIC_SPELL] != this)
        return;

    // global cooldown have only player or controlled units
    if (m_caster->GetCharmInfo())
        m_caster->GetCharmInfo()->GetGlobalCooldownMgr().CancelGlobalCooldown(m_spellInfo);
    /*else if (m_caster->GetTypeId() == TYPEID_PLAYER)
        ((Player*)m_caster)->GetGlobalCooldownMgr().CancelGlobalCooldown(m_spellInfo);*/
}


namespace Trinity
{

    WorldObjectSpellTargetCheck::WorldObjectSpellTargetCheck(Unit* caster, Unit* referer, SpellInfo const* spellInfo,
        SpellTargetCheckTypes selectionType, ConditionList* condList) : _caster(caster), _referer(referer), _spellInfo(spellInfo),
        _targetSelectionType(selectionType), _condList(condList)
    {
        if (condList)
            _condSrcInfo = new ConditionSourceInfo(NULL, caster);
        else
            _condSrcInfo = NULL;
    }

    WorldObjectSpellTargetCheck::~WorldObjectSpellTargetCheck()
    {
        if (_condSrcInfo)
            delete _condSrcInfo;
    }

    bool WorldObjectSpellTargetCheck::operator()(WorldObject* target)
    {
        if (_spellInfo->CheckTarget(_caster, target, true) != SPELL_CAST_OK)
            return false;
        Unit* unitTarget = target->ToUnit();
        if (Corpse* corpseTarget = target->ToCorpse())
        {
            // use ofter for party/assistance checks
            if (Player* owner = ObjectAccessor::FindPlayer(corpseTarget->GetOwnerGUID()))
                unitTarget = owner;
            else
                return false;
        }
        if (unitTarget)
        {
            bool targetIsTotem = unitTarget->GetTypeId() == TYPEID_UNIT && unitTarget->ToCreature()->IsTotem();
            switch (_targetSelectionType)
            {
            case TARGET_CHECK_ENEMY:
                if (targetIsTotem)
                    return false;
                /* sunwell if (!_caster->_IsValidAttackTarget(unitTarget, _spellInfo)) */
                if(_caster->CanAttack(unitTarget) != CAN_ATTACK_RESULT_OK)
                    return false;
                break;
            case TARGET_CHECK_ALLY:
                if (targetIsTotem)
                    return false;
                if (!_caster->_IsValidAssistTarget(unitTarget, _spellInfo))
                    return false;
                break;
            case TARGET_CHECK_PARTY:
                if (targetIsTotem)
                    return false;
                if (!_caster->_IsValidAssistTarget(unitTarget, _spellInfo))
                    return false;
                if (!_referer->IsInPartyWith(unitTarget))
                    return false;
                break;
            case TARGET_CHECK_RAID_CLASS:
                if (_referer->GetClass() != unitTarget->GetClass())
                    return false;
                // nobreak;
            case TARGET_CHECK_RAID:
                if (targetIsTotem)
                    return false;
                if (!_caster->_IsValidAssistTarget(unitTarget, _spellInfo))
                    return false;
                if (!_referer->IsInRaidWith(unitTarget))
                    return false;
                break;
            case TARGET_CHECK_CORPSE:
                if (_caster->IsFriendlyTo(unitTarget))
                    return false;
                break;
            default:
                break;
            }
        }
        if (!_condSrcInfo)
            return true;
        _condSrcInfo->mConditionTargets[0] = target;
        return sConditionMgr->IsObjectMeetToConditions(*_condSrcInfo, *_condList);
    }

    WorldObjectSpellNearbyTargetCheck::WorldObjectSpellNearbyTargetCheck(float range, Unit* caster, SpellInfo const* spellInfo,
        SpellTargetCheckTypes selectionType, ConditionList* condList)
        : WorldObjectSpellTargetCheck(caster, caster, spellInfo, selectionType, condList), _range(range), _position(caster)
    {
    }

    bool WorldObjectSpellNearbyTargetCheck::operator()(WorldObject* target)
    {
        float dist = target->GetDistance(*_position);
        if (dist < _range && WorldObjectSpellTargetCheck::operator ()(target))
        {
            _range = dist;
            return true;
        }
        return false;
    }

    WorldObjectSpellAreaTargetCheck::WorldObjectSpellAreaTargetCheck(float range, Position const* position, Unit* caster,
        Unit* referer, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList)
        : WorldObjectSpellTargetCheck(caster, referer, spellInfo, selectionType, condList), _range(range), _position(position)
    {
    }

    bool WorldObjectSpellAreaTargetCheck::operator()(WorldObject* target)
    {
        if (target->GetTypeId() == TYPEID_GAMEOBJECT)
        {
            if (!target->ToGameObject()->IsInRange(_position->GetPositionX(), _position->GetPositionY(), _position->GetPositionZ(), _range))
                return false;
        }
        else if (!target->IsWithinDist3d(_position, _range))
            return false;
        /* sunwell else if (target->GetTypeId() == TYPEID_UNIT && target->ToCreature()->IsAvoidingAOE()) // pussywizard
            return false; */
        return WorldObjectSpellTargetCheck::operator ()(target);
    }

    WorldObjectSpellConeTargetCheck::WorldObjectSpellConeTargetCheck(float coneAngle, float range, Unit* caster,
        SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList)
        : WorldObjectSpellAreaTargetCheck(range, caster, caster, caster, spellInfo, selectionType, condList), _coneAngle(coneAngle)
    {
    }

    bool WorldObjectSpellConeTargetCheck::operator()(WorldObject* target)
    {
        if (_spellInfo->HasAttribute(SPELL_ATTR_CU_CONE_BACK))
        {
            if (!_caster->isInBack(target, _coneAngle))
                return false;
        }
        else if (_spellInfo->HasAttribute(SPELL_ATTR_CU_CONE_LINE))
        {
            if (!_caster->HasInLine(target, _caster->GetObjectSize()))
                return false;
        }
        else if (_spellInfo->HasAttribute(SPELL_ATTR_CU_CONE_180))
        {
            if (!_caster->isInFront(target, M_PI))
                return false;
        }
        else
        {
            if (!_caster->isInFront(target, _coneAngle))
                return false;
        }
        return WorldObjectSpellAreaTargetCheck::operator ()(target);
    }

    WorldObjectSpellTrajTargetCheck::WorldObjectSpellTrajTargetCheck(float range, Position const* position, Unit* caster,
        SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionList* condList)
        : WorldObjectSpellAreaTargetCheck(range, position, caster, caster, spellInfo, selectionType, condList)
    {
    }

    bool WorldObjectSpellTrajTargetCheck::operator()(WorldObject* target)
    {
        // return all targets on missile trajectory (0 - size of a missile)
        if (!_caster->HasInLine(target, 0))
            return false;
        return WorldObjectSpellAreaTargetCheck::operator ()(target);
    }

} //namespace Trinity

void Spell::CallScriptDestinationTargetSelectHandlers(SpellDestination& target, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    /* TC spellscript
    for (std::list<SpellScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(SPELL_SCRIPT_HOOK_DESTINATION_TARGET_SELECT);
        std::list<SpellScript::DestinationTargetSelectHandler>::iterator hookItrEnd = (*scritr)->OnDestinationTargetSelect.end(), hookItr = (*scritr)->OnDestinationTargetSelect.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if (hookItr->IsEffectAffected(m_spellInfo, effIndex) && targetType.GetTarget() == hookItr->GetTarget())
                hookItr->Call(*scritr, target);

        (*scritr)->_FinishScriptCall();
    }
    */
}

void Spell::CallScriptObjectAreaTargetSelectHandlers(std::list<WorldObject*>& targets, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
  /* todo spellscript
  for (std::list<SpellScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(SPELL_SCRIPT_HOOK_OBJECT_AREA_TARGET_SELECT);
        std::list<SpellScript::ObjectAreaTargetSelectHandler>::iterator hookItrEnd = (*scritr)->OnObjectAreaTargetSelect.end(), hookItr = (*scritr)->OnObjectAreaTargetSelect.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if (hookItr->IsEffectAffected(m_spellInfo, effIndex) && targetType.GetTarget() == hookItr->GetTarget())
                hookItr->Call(*scritr, targets);

        (*scritr)->_FinishScriptCall();
    }
    */
}

void Spell::CallScriptObjectTargetSelectHandlers(WorldObject*& target, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    /* tc spellscript
    for (std::list<SpellScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(SPELL_SCRIPT_HOOK_OBJECT_TARGET_SELECT);
        std::list<SpellScript::ObjectTargetSelectHandler>::iterator hookItrEnd = (*scritr)->OnObjectTargetSelect.end(), hookItr = (*scritr)->OnObjectTargetSelect.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if (hookItr->IsEffectAffected(m_spellInfo, effIndex) && targetType.GetTarget() == hookItr->GetTarget())
                hookItr->Call(*scritr, target);

        (*scritr)->_FinishScriptCall();
    }
    */
}

void Spell::PrepareScriptHitHandlers()
{
    /* todo spellscript
    for (std::list<SpellScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
        (*scritr)->_InitHit();
        */
}


bool Spell::CallScriptEffectHandlers(SpellEffIndex effIndex, SpellEffectHandleMode mode)
{
    // execute script effect handler hooks and check if effects was prevented
    bool preventDefault = false;
    /* todo spellscript
    for (std::list<SpellScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        std::list<SpellScript::EffectHandler>::iterator effItr, effEndItr;
        SpellScriptHookType hookType;
        switch (mode)
        {
        case SPELL_EFFECT_HANDLE_LAUNCH:
            effItr = (*scritr)->OnEffectLaunch.begin();
            effEndItr = (*scritr)->OnEffectLaunch.end();
            hookType = SPELL_SCRIPT_HOOK_EFFECT_LAUNCH;
            break;
        case SPELL_EFFECT_HANDLE_LAUNCH_TARGET:
            effItr = (*scritr)->OnEffectLaunchTarget.begin();
            effEndItr = (*scritr)->OnEffectLaunchTarget.end();
            hookType = SPELL_SCRIPT_HOOK_EFFECT_LAUNCH_TARGET;
            break;
        case SPELL_EFFECT_HANDLE_HIT:
            effItr = (*scritr)->OnEffectHit.begin();
            effEndItr = (*scritr)->OnEffectHit.end();
            hookType = SPELL_SCRIPT_HOOK_EFFECT_HIT;
            break;
        case SPELL_EFFECT_HANDLE_HIT_TARGET:
            effItr = (*scritr)->OnEffectHitTarget.begin();
            effEndItr = (*scritr)->OnEffectHitTarget.end();
            hookType = SPELL_SCRIPT_HOOK_EFFECT_HIT_TARGET;
            break;
        default:
            ASSERT(false);
            return false;
        }
        (*scritr)->_PrepareScriptCall(hookType);
        for (; effItr != effEndItr; ++effItr)
            // effect execution can be prevented
            if (!(*scritr)->_IsEffectPrevented(effIndex) && (*effItr).IsEffectAffected(m_spellInfo, effIndex))
                (*effItr).Call(*scritr, effIndex);

        if (!preventDefault)
            preventDefault = (*scritr)->_IsDefaultEffectPrevented(effIndex);

        (*scritr)->_FinishScriptCall();
    }
    */
    return preventDefault;
}

void Spell::CallScriptAfterCastHandlers()
{
    /* tc spellscript
    for (std::list<SpellScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(SPELL_SCRIPT_HOOK_AFTER_CAST);
        std::list<SpellScript::CastHandler>::iterator hookItrEnd = (*scritr)->AfterCast.end(), hookItr = (*scritr)->AfterCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(*scritr);

        (*scritr)->_FinishScriptCall();
    }
    */
}


void Spell::CallScriptAfterHitHandlers()
{
/* todo spellscript
    for (std::list<SpellScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(SPELL_SCRIPT_HOOK_AFTER_HIT);
        std::list<SpellScript::HitHandler>::iterator hookItrEnd = (*scritr)->AfterHit.end(), hookItr = (*scritr)->AfterHit.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(*scritr);

        (*scritr)->_FinishScriptCall();
    }
    */
}

void Spell::CallScriptBeforeCastHandlers()
{
    /* todo spellscript
    for (std::list<SpellScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(SPELL_SCRIPT_HOOK_BEFORE_CAST);
        std::list<SpellScript::CastHandler>::iterator hookItrEnd = (*scritr)->BeforeCast.end(), hookItr = (*scritr)->BeforeCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(*scritr);

        (*scritr)->_FinishScriptCall();
    }
    */
}

void Spell::CallScriptBeforeHitHandlers()
{
    /* todo spellscript
    for (std::list<SpellScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(SPELL_SCRIPT_HOOK_BEFORE_HIT);
        std::list<SpellScript::HitHandler>::iterator hookItrEnd = (*scritr)->BeforeHit.end(), hookItr = (*scritr)->BeforeHit.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(*scritr);

        (*scritr)->_FinishScriptCall();
    }
    */
}

void Spell::CallScriptOnCastHandlers()
{
    /* todo spellscript
    for (std::list<SpellScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(SPELL_SCRIPT_HOOK_ON_CAST);
        std::list<SpellScript::CastHandler>::iterator hookItrEnd = (*scritr)->OnCast.end(), hookItr = (*scritr)->OnCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(*scritr);

        (*scritr)->_FinishScriptCall();
    }
    */
}

void Spell::CallScriptOnHitHandlers()
{
    /* todo spellscript
    for (std::list<SpellScript*>::iterator scritr = m_loadedScripts.begin(); scritr != m_loadedScripts.end(); ++scritr)
    {
        (*scritr)->_PrepareScriptCall(SPELL_SCRIPT_HOOK_HIT);
        std::list<SpellScript::HitHandler>::iterator hookItrEnd = (*scritr)->OnHit.end(), hookItr = (*scritr)->OnHit.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(*scritr);

        (*scritr)->_FinishScriptCall();
    }
    */
}