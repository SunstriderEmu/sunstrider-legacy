
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
#include "SpellScript.h"
#include "GameObjectAI.h"

extern pEffect SpellEffects[TOTAL_SPELL_EFFECTS];

bool IsQuestTameSpell(uint32 spellId)
{
    SpellInfo const *spellproto = sSpellMgr->GetSpellInfo(spellId);
    if (!spellproto) 
        return false;

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
#endif //LICH_KING
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
    m_objectTarget = nullptr;
    m_itemTarget = nullptr;

    m_objectTargetGUID = ObjectGuid::Empty;
    m_itemTargetGUID = ObjectGuid::Empty;
    m_itemTargetEntry = 0;

    m_targetMask = 0;

    // sunwell: Channel data
    m_objectTargetGUIDChannel = ObjectGuid::Empty;
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
        data >> m_objectTargetGUID.ReadAsPacked();

    if (m_targetMask & (TARGET_FLAG_ITEM | TARGET_FLAG_TRADE_ITEM))
        data >> m_itemTargetGUID.ReadAsPacked();

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
        data >> m_objectTargetGUID.ReadAsPacked();


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

ObjectGuid SpellCastTargets::GetUnitTargetGUID() const
{
    if (m_objectTargetGUID.IsUnit())
        return m_objectTargetGUID;

    return ObjectGuid::Empty;
}

Unit* SpellCastTargets::GetUnitTarget() const
{
    if (m_objectTarget)
        return m_objectTarget->ToUnit();

    return nullptr;
}

void SpellCastTargets::SetUnitTarget(Unit* target)
{
    if (!target)
        return;

    m_objectTarget = target;
    m_objectTargetGUID = target->GetGUID();
    m_targetMask |= TARGET_FLAG_UNIT;
}

ObjectGuid SpellCastTargets::GetGOTargetGUID() const
{
    if (m_objectTargetGUID.IsAnyTypeGameObject())
        return m_objectTargetGUID;

    return ObjectGuid::Empty;
}

GameObject* SpellCastTargets::GetGOTarget() const
{
    if (m_objectTarget)
        return m_objectTarget->ToGameObject();
    return nullptr;
}


void SpellCastTargets::SetGOTarget(GameObject* target)
{
    if (!target)
        return;

    m_objectTarget = target;
    m_objectTargetGUID = target->GetGUID();
    m_targetMask |= TARGET_FLAG_GAMEOBJECT;
}

ObjectGuid SpellCastTargets::GetCorpseTargetGUID() const
{
    if (m_objectTargetGUID.IsCorpse())
        return m_objectTargetGUID;

    return ObjectGuid::Empty;
}

Corpse* SpellCastTargets::GetCorpseTarget() const
{
    if (m_objectTarget)
        return m_objectTarget->ToCorpse();
    return nullptr;
}

WorldObject* SpellCastTargets::GetObjectTarget() const
{
    return m_objectTarget;
}

ObjectGuid SpellCastTargets::GetObjectTargetGUID() const
{
    return m_objectTargetGUID;
}

void SpellCastTargets::RemoveObjectTarget()
{
    m_objectTarget = nullptr;
    m_objectTargetGUID.Clear();
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
    m_itemTargetGUID.Set(uint64(TRADE_SLOT_NONTRADED));
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

// sunwell: Channel Data
void SpellCastTargets::SetObjectTargetChannel(ObjectGuid targetGUID)
{
    m_objectTargetGUIDChannel = targetGUID;
}

void SpellCastTargets::SetDstChannel(SpellDestination const& spellDest)
{
    m_dstChannel = spellDest;
}

WorldObject* SpellCastTargets::GetObjectTargetChannel(Unit* caster) const
{
    return m_objectTargetGUIDChannel ? ((m_objectTargetGUIDChannel == caster->GetGUID()) ? caster : ObjectAccessor::GetWorldObject(*caster, m_objectTargetGUIDChannel)) : nullptr;
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
    m_objectTarget = m_objectTargetGUID ? ((m_objectTargetGUID == caster->GetGUID()) ? caster : ObjectAccessor::GetWorldObject(*caster, m_objectTargetGUID)) : nullptr;

    m_itemTarget = nullptr;
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

Spell::Spell(Unit* Caster, SpellInfo const *info, TriggerCastFlags triggerFlags, ObjectGuid originalCasterGUID, Spell** triggeringContainer, bool skipCheck) :
    m_spellInfo(info), 
    m_spellValue(new SpellValue(m_spellInfo)),
    m_caster(Caster),
    m_preGeneratedPath(nullptr),
    _scriptsLoaded(false),
    _spellEvent(nullptr)
{
    m_skipCheck = skipCheck;
    m_selfContainer = nullptr;
    m_triggeringContainer = triggeringContainer;
    m_referencedFromCurrentSpell = false;
    m_executedCurrently = false;
    m_needComboPoints = m_spellInfo->NeedsComboPoints();
    m_delayStart = 0;
    m_immediateHandled = false;
    m_delayAtDamageCount = 0;

    m_applyMultiplierMask = 0;
    m_auraScaleMask = 0;

    // Get data for type of attack
    m_attackType = info->GetAttackType();

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

    if(originalCasterGUID)
        m_originalCasterGUID = originalCasterGUID;
    else
        m_originalCasterGUID = m_caster->GetGUID();

    if(m_originalCasterGUID==m_caster->GetGUID())
        m_originalCaster = m_caster;
    else
    {
        m_originalCaster = ObjectAccessor::GetUnit(*m_caster,m_originalCasterGUID);
        if(m_originalCaster && !m_originalCaster->IsInWorld()) m_originalCaster = nullptr;
    }

    for(int i=0; i <3; ++i)
        m_currentBasePoints[i] = m_spellValue->EffectBasePoints[i];

    m_spellState = SPELL_STATE_NULL;

    m_castPositionX = m_castPositionY = m_castPositionZ = 0;
    m_TriggerSpells.clear();
    _triggeredCastFlags = triggerFlags;
    if (info->HasAttribute(SPELL_ATTR4_CAN_CAST_WHILE_CASTING))
        _triggeredCastFlags = TriggerCastFlags(uint32(_triggeredCastFlags) | TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_CAST_DIRECTLY);
    //m_AreaAura = false;
    m_CastItem = nullptr;

    unitTarget = nullptr;
    itemTarget = nullptr;
    gameObjTarget = nullptr;
    focusObject = nullptr;
    m_cast_count = 0;
    m_triggeredByAuraSpell  = nullptr;

    effectHandleMode = SPELL_EFFECT_HANDLE_LAUNCH;

    //Auto Shot & Shoot
    m_autoRepeat = !IsTriggered() && m_spellInfo->IsAutoRepeatRangedSpell();

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
       && (!IsPositive() || m_spellInfo->HasEffect(SPELL_EFFECT_DISPEL))
      )
        m_canReflect = true;

    CleanupTargetList();
    //TC memset(m_effectExecuteData, 0, MAX_SPELL_EFFECTS * sizeof(ByteBuffer*));

    for (auto & m_destTarget : m_destTargets)
        m_destTarget = SpellDestination(*m_caster);

    // sunwell:
    _spellTargetsSelected = false;

    if (uint64 dstDelay = CalculateDelayMomentForDst())
        m_delayMoment = dstDelay;
}

uint64 Spell::CalculateDelayMomentForDst() const
{
    if (m_targets.HasDst())
    {
        if (m_targets.HasTraj())
        {
            float speed = m_targets.GetSpeedXY();
            if (speed > 0.0f)
                return (uint64)floor(m_targets.GetDist2d() / speed * 1000.0f);
        }
        else if (m_spellInfo->Speed > 0.0f)
        {
            // We should not subtract caster size from dist calculation (fixes execution time desync with animation on client, eg. Malleable Goo cast by PP)
            float dist = m_caster->GetExactDist(*m_targets.GetDstPos());
            return (uint64)std::floor(dist / m_spellInfo->Speed * 1000.0f);
        }
    }

    return 0;
}

void Spell::RecalculateDelayMomentForDst()
{
    m_delayMoment = CalculateDelayMomentForDst();
    m_caster->m_Events.ModifyEventTime(_spellEvent, GetDelayStart() + m_delayMoment);
}

Spell::~Spell()
{
    // unload scripts
    while (!m_loadedScripts.empty())
    {
        auto itr = m_loadedScripts.begin();
        (*itr)->_Unload();
        delete (*itr);
        m_loadedScripts.erase(itr);
    }

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
            Unit* unit = nullptr;
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
                && (!IsPositive() || (!m_caster->IsFriendlyTo(target) && m_spellInfo->HasEffect(SPELL_EFFECT_DISPEL)))))
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
                redirect = nullptr;
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
                WorldObject* result = nullptr;

                Trinity::CannibalizeObjectCheck u_check(m_caster, max_range);
                Trinity::WorldObjectSearcher<Trinity::CannibalizeObjectCheck > searcher(m_caster, result, u_check);
                Cell::VisitGridObjects(m_caster, searcher, max_range);
                if (!result)
                    Cell::VisitWorldObjects(m_caster, searcher, max_range);

                if (result)
                {
                    switch (result->GetTypeId())
                    {
                    case TYPEID_UNIT:
                    case TYPEID_PLAYER:
                        AddUnitTarget((Unit*)result, 1 << i);
                        break;
                    case TYPEID_CORPSE:
                        /* todo spelltargets m_targets.SetCorpseTarget((Corpse*)result); */
                        if (Player* owner = ObjectAccessor::FindPlayer(((Corpse*)result)->GetOwnerGUID()))
                            AddUnitTarget(owner, 1 << i);
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
                GameObject* go = m_caster->FindNearestGameObject(185861, 100.0f);

                if (go && go->GetDistance2d(m_targets.GetDstPos()->GetPositionX(), m_targets.GetDstPos()->GetPositionY()) <= 4.0f) {
                    go->SetLootState(GO_JUST_DEACTIVATED);
                    m_caster->ToPlayer()->KilledMonsterCredit(23118, go->GetGUID());
                }

                break;
            }
            case 33655: // Dropping the Nether Modulator
            {
                for(uint32 entry : { 183350, 183351 })
                {
                    GameObject* go = m_caster->FindNearestGameObject(entry, 100.0f);
                    if (go && go->GetDistance2d(m_targets.GetDstPos()->GetPositionX(), m_targets.GetDstPos()->GetPositionY()) <= 17.0f) {
                        go->SetLootState(GO_JUST_DEACTIVATED);
                        m_caster->ToPlayer()->KilledMonsterCredit(19291, go->GetGUID());
                        break;
                    }
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
            // maybe do this for all spells?
            if (!focusObject && m_UniqueTargetInfo.empty() && m_UniqueGOTargetInfo.empty() && m_UniqueItemInfo.empty() && !m_targets.HasDst())
            {
                SendCastResult(SPELL_FAILED_BAD_IMPLICIT_TARGETS);
                finish(false);
                return;
            }

            uint8 mask = (1 << i);
            for (auto & ihit : m_UniqueTargetInfo)
            {
                if (ihit.effectMask & mask)
                {
                    m_channelTargetEffectMask |= mask;
                    break;
                }
            }
        }
        else if (m_auraScaleMask)
        {
            bool checkLvl = !m_UniqueTargetInfo.empty();
            for (auto ihit = m_UniqueTargetInfo.begin(); ihit != m_UniqueTargetInfo.end();)
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

    uint8 effectMask = 1 << effIndex;
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
        for (uint8 j = effIndex + 1; j < MAX_SPELL_EFFECTS; ++j)
        {
            SpellEffectInfo const* effects = GetSpellInfo()->Effects;
            if (effects[effIndex].TargetA.GetTarget() == effects[j].TargetA.GetTarget() &&
                effects[effIndex].TargetB.GetTarget() == effects[j].TargetB.GetTarget() &&
                effects[effIndex].ImplicitTargetConditions == effects[j].ImplicitTargetConditions &&
                effects[effIndex].CalcRadius(m_caster) == effects[j].CalcRadius(m_caster) &&
                CheckScriptEffectImplicitTargets(effIndex, (SpellEffIndex) j) 
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
        // just in case there is no dest, explanation in SelectImplicitDestDestTargets
        CheckDst();

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
        // sunwell: All channel selectors have needed data passed in m_targets structure
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
        range = m_spellInfo->GetMaxRange(IsPositive(), m_caster, this);
        break;
    default:
        ASSERT(false && "Spell::SelectImplicitNearbyTargets: received not implemented selection check type");
        break;
    }

    ConditionContainer* condList = m_spellInfo->Effects[effIndex].ImplicitTargetConditions;

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
                else
                {
                    SendCastResult(SPELL_FAILED_BAD_IMPLICIT_TARGETS);
                    finish(false);
                }
                return;
            }
            break;
        case TARGET_OBJECT_TYPE_DEST:
            if (m_spellInfo->RequiresSpellFocus)
            {
                if (focusObject)
                {
                    SpellDestination dest(*focusObject);
                    CallScriptDestinationTargetSelectHandlers(dest, effIndex, targetType);
                    m_targets.SetDst(dest);
                }
                else
                {
                    SendCastResult(SPELL_FAILED_BAD_IMPLICIT_TARGETS);
                    finish(false);
                }
                return;
            }
            else
            {
                SendCastResult(SPELL_FAILED_BAD_IMPLICIT_TARGETS);
                finish(false);
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
        SendCastResult(SPELL_FAILED_BAD_IMPLICIT_TARGETS);
        finish(false);
        return;
    }

    CallScriptObjectTargetSelectHandlers(target, effIndex, targetType);
    if (!target)
    {
        //TC_LOG_DEBUG("spells", "Spell::SelectImplicitNearbyTargets: OnObjectTargetSelect script hook for spell Id %u set NULL target, effect %u", m_spellInfo->Id, effIndex);
        SendCastResult(SPELL_FAILED_BAD_IMPLICIT_TARGETS);
        finish(false);
        return;
    }

    switch (targetType.GetObjectType())
    {
    case TARGET_OBJECT_TYPE_UNIT:
    {
        if (Unit* _unitTarget = target->ToUnit())
        {
            AddUnitTarget(_unitTarget, effMask, true, false);
            // sunwell: important! if channeling spell have nearby entry, it has no unitTarget by default
            // and if channeled spell has target 77, it requires unitTarget, set it here!
            // sunwell: if we have NO unit target
            if (!m_targets.GetUnitTarget())
                m_targets.SetUnitTarget(_unitTarget);
        }
        else
        {
            //TC_LOG_DEBUG("spells", "Spell::SelectImplicitNearbyTargets: OnObjectTargetSelect script hook for spell Id %u set object of wrong type, expected unit, got %s, effect %u", m_spellInfo->Id, GetLogNameForGuid(target->GetGUID()), effMask);
            SendCastResult(SPELL_FAILED_BAD_IMPLICIT_TARGETS);
            finish(false);
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
            SendCastResult(SPELL_FAILED_BAD_IMPLICIT_TARGETS);
            finish(false);
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
    ConditionContainer* condList = m_spellInfo->Effects[effIndex].ImplicitTargetConditions;
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
                Trinity::Containers::RandomResize(targets, maxTargets);
            }

            for (auto & target : targets)
            {
                if (Unit* newTarget = target->ToUnit())
                    AddUnitTarget(newTarget, effMask, false);
                else if (GameObject* gObjTarget = target->ToGameObject())
                    AddGOTarget(gObjTarget, effMask);
            }
        }
    }
}

void Spell::SelectImplicitAreaTargets(SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType, uint32 effMask)
{
    Unit* referer = nullptr;
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
        for (auto ihit = m_UniqueTargetInfo.rbegin(); ihit != m_UniqueTargetInfo.rend(); ++ihit)
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

    Position const* center = nullptr;
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

    // sunwell: the distance should be increased by caster size, it is neglected in latter calculations
    std::list<WorldObject*> targets;
    float radius = m_spellInfo->Effects[effIndex].CalcRadius(m_caster);
    // Workaround for some spells that don't have RadiusEntry set in dbc (but SpellRange instead)
    if (G3D::fuzzyEq(radius, 0.f))
        radius = m_spellInfo->GetMaxRange(m_spellInfo->IsPositiveEffect(effIndex), m_caster, this);

    radius *= m_spellValue->RadiusMod;

    // if this is a proximity based aoe (Frost Nova, Psychic Scream, ...), include the caster's own combat reach
    if (targetType.IsProximityBasedAoe())
        radius += GetCaster()->GetCombatReach();

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
            Trinity::Containers::RandomResize(targets, maxTargets);
        }

        for (auto & target : targets)
        {
            if (Unit* newTarget = target->ToUnit())
                AddUnitTarget(newTarget, effMask, false);
            else if (GameObject* gObjTarget = target->ToGameObject())
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
        m_caster->GetClosePoint(x, y, z, DEFAULT_PLAYER_BOUNDING_RADIUS, dis, angle);

        float ground = m_caster->GetMapHeight(x, y, z);
        float liquidLevel = VMAP_INVALID_HEIGHT_VALUE;
        LiquidData liquidData;
        if (m_caster->GetMap()->GetLiquidStatus(x, y, z, MAP_ALL_LIQUIDS, &liquidData, m_caster->GetCollisionHeight()))
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
        float dist = m_spellInfo->Effects[effIndex].CalcRadius(m_caster);
        float angle = targetType.CalcDirectionAngle();
        float objSize = m_caster->GetCombatReach();

        switch (targetType.GetTarget())
        {
        case TARGET_DEST_CASTER_SUMMON:
            dist = PET_FOLLOW_DIST;
            break;
        case TARGET_DEST_CASTER_RANDOM:
            if (dist > objSize)
                dist = objSize + (dist - objSize) * float(rand_norm());
            break;
        case TARGET_DEST_CASTER_FRONT_LEFT:
        case TARGET_DEST_CASTER_BACK_LEFT:
        case TARGET_DEST_CASTER_FRONT_RIGHT:
        case TARGET_DEST_CASTER_BACK_RIGHT:
        {
            static float const DefaultTotemDistance = 3.0f;
            if (!m_spellInfo->Effects[effIndex].HasRadius())
                dist = DefaultTotemDistance;
            break;
        }
        default:
            break;
        }

        if (dist < objSize)
        {
            dist = objSize;
            // sunwell: give the summon some space (eg. totems)
            if (m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo->Effects[effIndex].IsEffect(SPELL_EFFECT_SUMMON))
                dist += objSize;
        }

        Position pos;
        SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(m_spellInfo->Effects[effIndex].MiscValueB);
        if (   (m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_SUMMON && properties && (properties->Type == SUMMON_TYPE_TOTEM || properties->Type == SUMMON_TYPE_LIGHTWELL))
            || (m_spellInfo->Effects[effIndex].Effect >= SPELL_EFFECT_SUMMON_OBJECT_SLOT1 && m_spellInfo->Effects[effIndex].Effect <= SPELL_EFFECT_SUMMON_OBJECT_SLOT4)
            )
        {
            pos = m_caster->GetFirstWalkableCollisionPosition(dist, angle, false);
        } else {
            if (m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_LEAP
                || m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_TELEPORT_UNITS
#ifdef LICH_KING
                || m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_JUMP_DEST
#endif
                || (m_caster->GetTypeId() == TYPEID_PLAYER && m_spellInfo->Effects[effIndex].Effect == SPELL_EFFECT_SUMMON)
                )
                pos = m_caster->GetLeapPosition(dist);
            else
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
        float dist = m_spellInfo->Effects[effIndex].CalcRadius(m_caster);
        if (targetType.GetTarget() == TARGET_DEST_TARGET_RANDOM)
            dist *= float(rand_norm());

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
    WorldObject* target = nullptr;
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
        target = m_caster->GetGuardianPet();
        if (!target)
            target = m_caster->GetCharm();
        break;
    case TARGET_UNIT_SUMMONER:
        if (m_caster->IsSummon())
            target = m_caster->ToTempSummon()->GetSummoner(); 
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

        for (auto & _target : targets)
            if (Unit* _unitTarget = _target->ToUnit())
                AddUnitTarget(_unitTarget, effMask, false);
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

    Position srcPos = *m_targets.GetSrcPos();
    srcPos.SetOrientation(m_caster->GetOrientation());
    float srcToDestDelta = m_targets.GetDstPos()->m_positionZ - srcPos.m_positionZ;


    std::list<WorldObject*> targets;
    Trinity::WorldObjectSpellTrajTargetCheck check(dist2d, &srcPos, m_caster, m_spellInfo, targetType.GetCheckType(), m_spellInfo->Effects[effIndex].ImplicitTargetConditions);
    Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellTrajTargetCheck> searcher(m_caster, targets, check, GRID_MAP_TYPE_MASK_ALL);
    SearchTargets<Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellTrajTargetCheck> >(searcher, GRID_MAP_TYPE_MASK_ALL, m_caster, &srcPos, dist2d);
    if (targets.empty())
        return;

    targets.sort(Trinity::ObjectDistanceOrderPred(m_caster));

    float b = tangent(m_targets.GetElevation());
    float a = (srcToDestDelta - dist2d * b) / (dist2d * dist2d);
    if (a > -0.0001f)
        a = 0;

    // We should check if triggered spell has greater range (which is true in many cases, and initial spell has too short max range)
    // limit max range to 300 yards, sometimes triggered spells can have 50000yds
    float bestDist = m_spellInfo->GetMaxRange(false);
    if (SpellInfo const* triggerSpellInfo = sSpellMgr->GetSpellInfo(m_spellInfo->Effects[effIndex].TriggerSpell))
        bestDist = std::min(std::max(bestDist, triggerSpellInfo->GetMaxRange(false)), std::min(dist2d, 300.0f));

    std::list<WorldObject*>::const_iterator itr = targets.begin();
    for (; itr != targets.end(); ++itr)
    {
        if (m_spellInfo->CheckTarget(m_caster, *itr, true) != SPELL_CAST_OK)
            continue;

        if (Unit* unit = (*itr)->ToUnit())
        {
            if (m_caster == *itr 
#ifdef LICH_KING
                || m_caster->IsOnVehicle(unit) || unit->GetVehicle()
#endif
                )
                continue;

            /*TC
            if (Creature* creatureTarget = unit->ToCreature())
            {
                if (!(creatureTarget->GetCreatureTemplate()->type_flags & CREATURE_TYPE_FLAG_CAN_COLLIDE_WITH_MISSILES))
                    continue;
            }
            */
        }

        const float size = std::max((*itr)->GetCombatReach(), 1.0f);
        const float objDist2d = srcPos.GetExactDist2d(*itr);
        const float dz = (*itr)->GetPositionZ() - srcPos.m_positionZ;

        const float horizontalDistToTraj = std::fabs(objDist2d * std::sin(srcPos.GetRelativeAngle(*itr)));
        const float sizeFactor = std::cos((horizontalDistToTraj / size) * (M_PI / 2.0f));
        const float distToHitPoint = std::max(objDist2d * std::cos(srcPos.GetRelativeAngle(*itr)) - size * sizeFactor, 0.0f);
        const float height = distToHitPoint * (a * distToHitPoint + b);

        if (fabs(dz - height) > size + b / 2.0f + TRAJECTORY_MISSILE_SIZE)
            continue;

        if (distToHitPoint < bestDist)
        {
            bestDist = distToHitPoint;
            break;
        }
    }

    if (dist2d > bestDist)
    {
        float x = m_targets.GetSrcPos()->m_positionX + std::cos(m_caster->GetOrientation()) * bestDist;
        float y = m_targets.GetSrcPos()->m_positionY + std::sin(m_caster->GetOrientation()) * bestDist;
        float z = m_targets.GetSrcPos()->m_positionZ + bestDist * (a * bestDist + b);

        SpellDestination dest(x, y, z, m_caster->GetOrientation());
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

            CallScriptObjectTargetSelectHandlers(target, SpellEffIndex(effIndex), SpellImplicitTargetInfo());

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

    WorldObject* target = nullptr;

    switch (m_spellInfo->Effects[effIndex].GetImplicitTargetType())
    {
        // add explicit object target or self to the target map
    case EFFECT_IMPLICIT_TARGET_EXPLICIT:
        // player which not released his spirit is Unit, but target flag for it is TARGET_FLAG_CORPSE_MASK
        if (targetMask & (TARGET_FLAG_UNIT_MASK | TARGET_FLAG_CORPSE_MASK))
        {
            if (Unit* newTarget = m_targets.GetUnitTarget())
                target = newTarget;
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
            if (Item* pitemTarget = m_targets.GetItemTarget())
                AddItemTarget(pitemTarget, 1 << effIndex);
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

    CallScriptObjectTargetSelectHandlers(target, SpellEffIndex(effIndex), SpellImplicitTargetInfo());

    if (target)
    {
        if (target->ToUnit())
            AddUnitTarget(target->ToUnit(), 1 << effIndex, false);
        else if (target->ToGameObject())
            AddGOTarget(target->ToGameObject(), 1 << effIndex);
    }
}

uint32 Spell::GetSearcherTypeMask(SpellTargetObjectTypes objType, ConditionContainer* condList)
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

        Map* map = referer->GetMap();

        if (searchInWorld)
            Cell::VisitWorldObjects(x, y, map, searcher, radius + SPELL_SEARCHER_COMPENSATION);

        if (searchInGrid)
            Cell::VisitGridObjects(x, y, map, searcher, radius + SPELL_SEARCHER_COMPENSATION);
    }
}

WorldObject* Spell::SearchNearbyTarget(float range, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionContainer* condList)
{
    WorldObject* target = nullptr;
    uint32 containerTypeMask = GetSearcherTypeMask(objectType, condList);
    if (!containerTypeMask)
        return nullptr;
    Trinity::WorldObjectSpellNearbyTargetCheck check(range, m_caster, m_spellInfo, selectionType, condList);
    Trinity::WorldObjectLastSearcher<Trinity::WorldObjectSpellNearbyTargetCheck> searcher(m_caster, target, check, containerTypeMask);
    SearchTargets<Trinity::WorldObjectLastSearcher<Trinity::WorldObjectSpellNearbyTargetCheck> >(searcher, containerTypeMask, m_caster, m_caster, range);
    return target;
}

void Spell::SearchAreaTargets(std::list<WorldObject*>& targets, float range, Position const* position, Unit* referer, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectionType, ConditionContainer* condList)
{
    uint32 containerTypeMask = GetSearcherTypeMask(objectType, condList);
    if (!containerTypeMask)
        return;
    Trinity::WorldObjectSpellAreaTargetCheck check(range, position, m_caster, referer, m_spellInfo, selectionType, condList);
    Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellAreaTargetCheck> searcher(m_caster, targets, check, containerTypeMask);
    SearchTargets<Trinity::WorldObjectListSearcher<Trinity::WorldObjectSpellAreaTargetCheck> >(searcher, containerTypeMask, m_caster, position, range);
}

void Spell::SearchChainTargets(std::list<WorldObject*>& targets, uint32 chainTargets, WorldObject* target, SpellTargetObjectTypes objectType, SpellTargetCheckTypes selectType, SpellTargetSelectionCategories selectCategory, ConditionContainer* condList, bool isChainHeal)
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

    // sunwell: the distance should be increased by caster size, it is neglected in latter calculations
    std::list<WorldObject*> tempTargets;
    SearchAreaTargets(tempTargets, searchRadius, (m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MELEE ? m_caster : target), m_caster, objectType, selectType, condList);
    tempTargets.remove(target);

    // sunwell: if we have select category nearby and checktype entry, select random of what we have, not by distance
    if (selectCategory == TARGET_SELECT_CATEGORY_NEARBY && selectType == TARGET_CHECK_ENTRY)
    {
        Trinity::Containers::RandomResize(tempTargets, chainTargets);
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

        for (auto itr = tempTargets.begin(); itr != tempTargets.end();)
        {
            auto checkItr = itr++;
            if (!m_caster->HasInArc(static_cast<float>(M_PI), *checkItr))
                tempTargets.erase(checkItr);
            else if (allowedArc > 0.0f && !m_caster->HasInArc(allowedArc, *checkItr, (*checkItr)->GetCombatReach()))
                tempTargets.erase(checkItr);
        }
    }

    while (chainTargets)
    {
        // try to get unit for next chain jump
        auto foundItr = tempTargets.end();
        // get unit with highest hp deficit in dist
        if (isChainHeal)
        {
            uint32 maxHPDeficit = 0;
            for (auto itr = tempTargets.begin(); itr != tempTargets.end(); ++itr)
            {
                if (Unit* itrTarget = (*itr)->ToUnit())
                {
                    uint32 deficit = itrTarget->GetMaxHealth() - itrTarget->GetHealth();
                    // sunwell: chain should not heal targets with max health
                    if (deficit == 0)
                        continue;

                    if ((deficit > maxHPDeficit || foundItr == tempTargets.end()) && target->IsWithinDist(itrTarget, jumpRadius) && target->IsWithinLOSInMap(itrTarget, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
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
            for (auto itr = tempTargets.begin(); itr != tempTargets.end(); ++itr)
            {
                if (foundItr == tempTargets.end())
                {
                    if ((!isBouncingFar || target->IsWithinDist(*itr, jumpRadius)) && target->IsWithinLOSInMap(*itr, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
                        foundItr = itr;
                }
                else if (target->GetDistanceOrder(*itr, *foundItr) && target->IsWithinLOSInMap(*itr, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
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
            if(IsPositive(hostileTarget))          // Check for positive spell
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
    if (!IsTriggered())
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
        if (target->IsImmunedToSpellEffect(m_spellInfo, effIndex, m_caster))
            effectMask &= ~(1 << effIndex);

    ObjectGuid targetGUID = target->GetGUID();

    // Lookup target in already in list
    for (auto & ihit : m_UniqueTargetInfo)
    {
        if (targetGUID == ihit.targetGUID)             // Found in list
        {
            ihit.effectMask |= effectMask;             // Immune effects removed from mask
            ihit.scaleAura = false;
            if (m_auraScaleMask && ihit.effectMask == m_auraScaleMask && m_caster != target)
            {
                SpellInfo const* auraSpell = m_spellInfo->GetFirstRankSpell();
                if (uint32(target->GetLevel() + 10) >= auraSpell->SpellLevel)
                    ihit.scaleAura = true;
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
        if (m_skipCheck && targetInfo.missCondition != SPELL_MISS_IMMUNE)
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
        m_caster->m_Events.AddEvent(new ReflectEvent(m_caster, targetInfo.targetGUID, m_spellInfo), m_caster->m_Events.CalculateTime(targetInfo.timeDelay));
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

    ObjectGuid targetGUID = go->GetGUID();

    // Lookup target in already in list
    for (auto & ihit : m_UniqueGOTargetInfo)
    {
        if (targetGUID == ihit.targetGUID)                 // Found in list
        {
            ihit.effectMask |= effectMask;                 // Add only effect mask
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
    for (auto & ihit : m_UniqueItemInfo)
    {
        if (item == ihit.item)                            // Found in list
        {
            ihit.effectMask |= effectMask;                 // Add only effect mask
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

    Unit* unit = m_caster->GetGUID() == target->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, target->targetGUID);
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
        // sunwell: FindUnit Access without Map check!!! Intended
        unit = ObjectAccessor::FindPlayer(target->targetGUID);
        if (!unit)
            return;

        // do far effects on the unit
        // can't use default call because of threading, do stuff as fast as possible
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (farMask & (1 << i))
                HandleEffects(unit, nullptr, nullptr, i, SPELL_EFFECT_HANDLE_HIT_TARGET);

        return;
    }

    // Get original caster (if exist) and calculate damage/healing from him
    Unit* caster = m_originalCasterGUID ? m_originalCaster : m_caster;

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
    Unit* spellHitTarget = nullptr;

    if (missInfo==SPELL_MISS_NONE)                          // In case spell hit target, do all effect on that target
        spellHitTarget = unit;
    else if (missInfo == SPELL_MISS_REFLECT)                // In case spell reflect from target, do all effect on caster (if hit)
    {  
        if (m_removeReflect) {
            unit->RemoveAurasDueToSpell(23920);
            m_removeReflect = false;
        }

        if (target->reflectResult == SPELL_MISS_NONE)       // If reflected spell hit caster -> do all effect on him
            spellHitTarget = m_caster;
    }

    bool enablePvP = false; // need to check PvP state before spell effects, but act on it afterwards

    if(spellHitTarget)
    {   // if target is flagged for pvp also flag caster if a player
        if (!IsTriggered() && unit->IsPvP() && m_caster->GetTypeId() == TYPEID_PLAYER)
            enablePvP = true; // Decide on PvP flagging now, but act on it later.

        SpellMissInfo missInfo2 = DoSpellHitOnUnit(spellHitTarget, mask);
        if (missInfo2 != SPELL_MISS_NONE)
        {
            if (missInfo2 != SPELL_MISS_MISS)
                m_caster->SendSpellMiss(unit, m_spellInfo->Id, missInfo2);
            m_damage = 0;
            spellHitTarget = nullptr;
        }
    }

    // Do not take combo points on dodge and miss
    if (missInfo != SPELL_MISS_NONE && m_needComboPoints && m_targets.GetUnitTargetGUID() == target->targetGUID)
        m_needComboPoints = false;

    CallScriptOnHitHandlers();

    // All calculated do it!
    // Do healing and triggers
    if (m_healing > 0)
    {
        bool crit = target->crit;
        uint32 addhealth = m_healing;
        if (crit)
        {
            procEx |= PROC_EX_CRITICAL_HIT;
            addhealth = caster->SpellCriticalDamageBonus(m_spellInfo, addhealth, nullptr);
        }
        else
            procEx |= PROC_EX_NORMAL_HIT;

        HealInfo healInfo(caster, unitTarget, addhealth, m_spellInfo, m_spellInfo->GetSchoolMask());
        caster->HealBySpell(healInfo, crit, missInfo);

        float threat = healInfo.GetEffectiveHeal() * 0.5f * sSpellMgr->GetSpellThreatModPercent(m_spellInfo);
        Unit* threatTarget = (GetSpellInfo()->HasAttribute(SPELL_ATTR_CU_THREAT_GOES_TO_CURRENT_CASTER) || !m_originalCaster) ? m_caster : m_originalCaster;
        unitTarget->GetThreatManager().ForwardThreatForAssistingMe(threatTarget, threat, m_spellInfo);

        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (missInfo != SPELL_MISS_REFLECT)
            caster->ProcDamageAndSpell(unitTarget, procAttacker, procVictim, procEx, addhealth, m_attackType, m_spellInfo, m_canTrigger);
    }
    // Do damage and triggers
    else if (m_damage > 0)
    {
        // Fill base damage struct (unitTarget - is real spell target)
        SpellNonMeleeDamage damageInfo(caster, unitTarget, m_spellInfo->Id, m_spellSchoolMask);

        // Add bonuses and fill damageInfo struct
        caster->CalculateSpellDamageTaken(&damageInfo, m_damage, m_spellInfo, m_attackType, target->crit);

        // Send log damage message to client
        caster->SendSpellNonMeleeDamageLog(&damageInfo);

        procEx = createProcExtendMask(&damageInfo, missInfo);
        procVictim |= PROC_FLAG_TAKEN_ANY_DAMAGE;

        
        caster->DealSpellDamage(&damageInfo, true);
        // Do triggers for unit (reflect triggers passed on hit phase for correct drop charge)
        if (missInfo != SPELL_MISS_REFLECT)
        {
            caster->ProcDamageAndSpell(unitTarget, procAttacker, procVictim, procEx, damageInfo.damage, m_attackType, m_spellInfo, m_canTrigger);
            if(caster->GetTypeId() == TYPEID_PLAYER 
                && !m_spellInfo->HasAttribute(SPELL_ATTR0_STOP_ATTACK_TARGET) 
                && !m_spellInfo->HasAttribute(SPELL_ATTR4_CANT_TRIGGER_ITEM_SPELLS) 
                && (m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_MELEE || m_spellInfo->DmgClass == SPELL_DAMAGE_CLASS_RANGED))
                (caster->ToPlayer())->CastItemCombatSpell(unitTarget, m_attackType, procVictim, procEx, m_spellInfo);
        }

        // Shadow Word: Death - deals damage equal to damage done to caster if victim is not killed
        if (m_spellInfo->SpellFamilyName == SPELLFAMILY_PRIEST && m_spellInfo->SpellFamilyFlags & 0x0000000200000000LL &&
            caster != unitTarget && unitTarget->IsAlive())
        {
            // Redirect damage to caster if victim alive
            m_caster->CastSpell(m_caster, 32409, TRIGGERED_FULL_MASK);
            if (m_caster->ToPlayer())
                m_caster->ToPlayer()->m_swdBackfireDmg = m_damage;
            //breakcompile;   // Build damage packet directly here and fake spell damage
            //caster->DealDamage(caster, uint32(m_damage), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_NORMAL, NULL, false);
        }
        // Judgement of Blood
        else if (m_spellInfo->SpellFamilyName == SPELLFAMILY_PALADIN && m_spellInfo->SpellFamilyFlags & 0x0000000800000000LL && m_spellInfo->SpellIconID==153)
        {
            CastSpellExtraArgs args;
            args.TriggerFlags = TRIGGERED_FULL_MASK;
            args.AddSpellBP0(damageInfo.damage * 33 / 100);
            m_caster->CastSpell(m_caster, 32220, args);
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
                    TC_LOG_ERROR("spells","Spell::EffectSchoolDMG: Spell %u not handled in blood thirst Aura",m_spellInfo->Id);
                    break;
            }
            if (BTAura)
                m_caster->CastSpell(m_caster, BTAura, TRIGGERED_FULL_MASK);
        }

#ifdef TESTS
        if(Player* p = m_caster->GetCharmerOrOwnerPlayerOrPlayerItself())
            if (p->GetPlayerbotAI())
                p->GetPlayerbotAI()->CastedDamageSpell(unitTarget, damageInfo, missInfo, target->crit);
#endif
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

#ifdef TESTS
        if (Player* p = m_caster->GetCharmerOrOwnerPlayerOrPlayerItself())
            if (p->GetPlayerbotAI())
                p->GetPlayerbotAI()->CastedDamageSpell(unitTarget, damageInfo, missInfo, target->crit);
#endif
    }

    // Call scripted function for AI if this spell is casted upon a creature (except pets)
    if(target->targetGUID.IsCreature())
    {
        // cast at creature (or GO) quest objectives update at successful cast finished (+channel finished)
        // ignore autorepeat/melee casts for speed (not exist quest for spells (hm... )
        if( m_caster->GetTypeId() == TYPEID_PLAYER && !IsAutoRepeat() && !IsNextMeleeSwingSpell() && !IsChannelActive() )
            (m_caster->ToPlayer())->CastedCreatureOrGO(unit->GetEntry(),unit->GetGUID(),m_spellInfo->Id);
    }

    if( missInfo != SPELL_MISS_EVADE && !m_caster->IsFriendlyTo(unit) && !IsPositive(hostileTarget) && m_caster->GetEntry() != WORLD_TRIGGER)
    {
        if(!IsTriggered()) //sun: prevent triggered spells to trigger pvp... a frost armor proc is not an offensive action
            m_caster->AttackedTarget(unit, m_spellInfo->HasInitialAggro());

        if(m_spellInfo->HasAttribute(SPELL_ATTR_CU_AURA_CC))
        {
            if(!unit->IsStandState())
                unit->SetStandState(PLAYER_STATE_NONE);
        }
    }
    
    if (spellHitTarget)
    {
        //AI functions
        if (spellHitTarget->GetTypeId() == TYPEID_UNIT && (spellHitTarget->ToCreature())->IsAIEnabled)
            (spellHitTarget->ToCreature())->AI()->SpellHit(caster, m_spellInfo);

        if (m_caster->GetTypeId() == TYPEID_UNIT && (m_caster->ToCreature())->IsAIEnabled)
            (m_caster->ToCreature())->AI()->SpellHitTarget(spellHitTarget, m_spellInfo);

        // if target is flagged for pvp also flag caster
        if (enablePvP)
            (m_caster->ToPlayer())->UpdatePvP(true);

        CallScriptAfterHitHandlers();
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
        range = m_spellInfo->GetMaxRange(IsPositive());
        if (range == 0)
            for (int i = EFFECT_0; i <= EFFECT_2; ++i)
                if (channelAuraMask & (1 << i) && m_spellInfo->Effects[i].RadiusEntry)
                {
                    range = m_spellInfo->Effects[i].CalcRadius(nullptr, nullptr);
                    break;
                }

        if (Player* modOwner = m_caster->GetSpellModOwner())
            modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RANGE, range, this); 

        // add little tolerance level
        range += std::min(MAX_SPELL_RANGE_TOLERANCE, range*0.1f); // 10% but no more than MAX_SPELL_RANGE_TOLERANCE
    }

    for (auto & ihit : m_UniqueTargetInfo)
    {
        if (ihit.missCondition == SPELL_MISS_NONE && (channelTargetEffectMask & ihit.effectMask))
        {
            Unit* unit = m_caster->GetGUID() == ihit.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit.targetGUID);

            if (!unit)
                continue;

            if (IsValidDeadOrAliveTarget(unit))
            {
                //sunstrider: exclude omnidirectional spells for range check... seems pretty okay in all cases I can see If you remove this, re implement mind control exclusion
                if (m_spellInfo->HasAttribute(SPELL_ATTR1_CHANNEL_TRACK_TARGET) && channelAuraMask & ihit.effectMask) 
                {
                    if (auto* aurApp = unit->GetAuraApplication(m_spellInfo->Id, m_originalCasterGUID))
                    {
                        if (m_caster != unit)
                        {
                            if (!m_caster->IsWithinDistInMap(unit, range))
                            {
                                ihit.effectMask &= ~aurApp->GetEffectMask();
                                //sunwell unit->RemoveAura(aurApp);
                                unit->RemoveAura(aurApp->GetId(), aurApp->GetEffIndex());
                                continue;
                            }
                            // sunwell: Update Orientation server side (non players wont sent appropriate packets)
                            else if (m_spellInfo->HasAttribute(SPELL_ATTR1_CHANNEL_TRACK_TARGET))
                                m_caster->UpdateOrientation(m_caster->GetAngle(unit));
                        }
                    }
                    else // aura is dispelled
                        continue;
                }

                channelTargetEffectMask &= ~ihit.effectMask;   // remove from need alive mask effect that have alive target
            }
        }
    }

    // sunwell: not all effects are covered, remove applications from all targets
    if (channelTargetEffectMask != 0)
    {
        for (auto & ihit : m_UniqueTargetInfo)
            if (ihit.missCondition == SPELL_MISS_NONE && (channelAuraMask & ihit.effectMask))
                if (Unit* unit = m_caster->GetGUID() == ihit.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit.targetGUID))
                    if (IsValidDeadOrAliveTarget(unit))
                        if (auto* aurApp = unit->GetAuraApplication(m_spellInfo->Id, m_originalCasterGUID))
                        {
                            ihit.effectMask &= ~aurApp->GetEffectMask();
                            //sunwell unit->RemoveAura(aurApp);
                            unit->RemoveAura(aurApp->GetId(), aurApp->GetEffIndex());
                        }
    }

    // is all effects from m_needAliveTargetMask have alive targets
    return channelTargetEffectMask == 0;
}

SpellMissInfo Spell::DoSpellHitOnUnit(Unit* unit, uint32 effectMask)
{
    if(!unit || !effectMask)
        return SPELL_MISS_EVADE;

    // Target may have begun evading between launch and hit phases - re-check now
    if (Creature* creatureTarget = unit->ToCreature())
        if (creatureTarget->IsEvadingAttacks())
            return SPELL_MISS_EVADE;

    Unit* caster = m_originalCasterGUID ? m_originalCaster : m_caster;

    // For delayed spells immunity may be applied between missile launch and hit - check immunity for that case
    if (m_spellInfo->Speed && unit->IsImmunedToSpell(m_spellInfo, caster))
        return SPELL_MISS_IMMUNE;

    // disable effects to which unit is immune
    SpellMissInfo returnVal = SPELL_MISS_IMMUNE;
    for (uint32 effectNumber = 0; effectNumber < MAX_SPELL_EFFECTS; ++effectNumber)
    {
        if (effectMask & (1 << effectNumber))
        {
            if (unit->IsImmunedToSpellEffect(m_spellInfo, effectNumber, m_caster))
                effectMask &= ~(1 << effectNumber);
        }
    }

    if (!effectMask)
        return returnVal;

    PrepareScriptHitHandlers();
    CallScriptBeforeHitHandlers();

#ifdef LICH_KING
    if (Player* player = unit->ToPlayer())
    {
        player->StartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_SPELL_TARGET, m_spellInfo->Id);
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, m_spellInfo->Id, 0, m_caster);
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2, m_spellInfo->Id);
    }

    if (Player* player = m_caster->ToPlayer())
    {
        player->StartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_SPELL_CASTER, m_spellInfo->Id);
        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2, m_spellInfo->Id, 0, unit);
    }
#endif

    if( caster != unit )
    {
        if (unit->GetCharmerOrOwnerGUID() != caster->GetGUID())
        {
            // Recheck UNIT_FLAG_NON_ATTACKABLE and evade for delayed spells
            if (m_spellInfo->Speed > 0.0f)
            {
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
                        return SPELL_MISS_EVADE;
                }
            }
        }
        
        if (m_caster->_IsValidAttackTarget(unit, m_spellInfo))
            unit->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_HITBYSPELL);
        else if( !caster->IsFriendlyTo(unit) )
        {
            // reset damage to 0 if target has Invisibility or Vanish aura (_only_ vanish, not stealth) and isn't visible for caster
            bool isVisibleForHit = ( (unit->HasAuraType(SPELL_AURA_MOD_INVISIBILITY) || unit->HasAuraTypeWithFamilyFlags(SPELL_AURA_MOD_STEALTH, SPELLFAMILY_ROGUE ,SPELLFAMILYFLAG_ROGUE_VANISH)) && !unit->CanSeeOrDetect(caster, true)) ? false : true;
            
            // for delayed spells ignore not visible explicit target
            if(m_spellInfo->Speed > 0.0f && unit == m_targets.GetUnitTarget() && !isVisibleForHit)
            {
                // that was causing CombatLog errors
                //caster->SendSpellMiss(unit, m_spellInfo->Id, SPELL_MISS_EVADE);
                m_damage = 0;
                return SPELL_MISS_NONE;
            }

            if(m_spellInfo->HasAttribute(SPELL_ATTR_CU_AURA_CC))
                unit->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CAST);
        }
        else
        {
            // for delayed spells ignore negative spells (after duel end) for friendly targets
            // TODO: this cause soul transfer bugged
            if(m_spellInfo->Speed > 0.0f && unit->GetTypeId() == TYPEID_PLAYER && !IsPositive() && m_spellInfo->Id != 45034) // FIXME: Hack for Boundless Agony (Kalecgos)
                return SPELL_MISS_EVADE;

            // assisting case, healing and resurrection
            if(unit->HasUnitState(UNIT_STATE_ATTACK_PLAYER))
            {
                if (Player* playerOwner = m_caster->GetCharmerOrOwnerPlayerOrPlayerItself())
                {
                    playerOwner->SetContestedPvP();
                    playerOwner->UpdatePvP(true);
                }
            }
            if( unit->IsInCombat() && m_spellInfo->HasInitialAggro())
            {
                if (m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE)) // only do explicit combat forwarding for PvP enabled units
                    m_caster->GetCombatManager().InheritCombatStatesFrom(unit);    // for creature v creature combat, the threat forward does it for us
                unit->GetThreatManager().ForwardThreatForAssistingMe(m_caster, 0.0f, nullptr, true);
            }
        }
    }

    uint8 aura_effmask = 0;
    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        if (effectMask & (1 << i) && m_spellInfo->Effects[i].IsUnitOwnedAuraEffect())
            aura_effmask |= 1 << i;

    // Get Data Needed for Diminishing Returns, some effects may have multiple auras, so this must be done on spell hit, not aura add
    bool const triggered = m_triggeredByAuraSpell != nullptr;
    DiminishingGroup const diminishGroup = m_spellInfo->GetDiminishingReturnsGroupForSpell(triggered);

    DiminishingLevels diminishLevel = DIMINISHING_LEVEL_1;
    if(diminishGroup && aura_effmask)
    {
        diminishLevel = unit->GetDiminishing(diminishGroup);
        m_diminishLevel = diminishLevel; //for later use in aura application
        // send immunity message if target is immune
        if(diminishLevel == DIMINISHING_LEVEL_IMMUNE)
            return SPELL_MISS_IMMUNE;

        DiminishingReturnsType type = m_spellInfo->GetDiminishingReturnsGroupType(triggered);
        // Increase Diminishing on unit, current informations for actually casts will use values above
        if (type == DRTYPE_ALL || (type == DRTYPE_PLAYER && unit->IsAffectedByDiminishingReturns()))
            unit->IncrDiminishing(m_spellInfo, triggered);
    }

    int8 sanct_effect = -1;
    for(uint32 effectNumber = 0;effectNumber < MAX_SPELL_EFFECTS; effectNumber++)
    {
        // handle sanctuary effects after aura apply!
        if (m_spellInfo->Effects[effectNumber].Effect == SPELL_EFFECT_SANCTUARY)
        {
            sanct_effect = effectNumber;
            continue;
        }

        if (effectMask & (1<<effectNumber))
            HandleEffects(unit, nullptr, nullptr, effectNumber, SPELL_EFFECT_HANDLE_HIT_TARGET);
    }

    if (sanct_effect >= 0 && (effectMask & (1 << sanct_effect)))
        HandleEffects(unit, nullptr, nullptr, sanct_effect, SPELL_EFFECT_HANDLE_HIT_TARGET);

    // trigger only for first effect targets
    if (m_ChanceTriggerSpells.size() && (effectMask & 0x1))
    {
        int _duration=0;
        for(ChanceTriggerSpells::const_iterator i = m_ChanceTriggerSpells.begin(); i != m_ChanceTriggerSpells.end(); ++i)
        {
            if(i->second == 100 || roll_chance_i(i->second))
            {
                CastSpellExtraArgs args;
                args.TriggerFlags = TRIGGERED_FULL_MASK;
                args.SkipHit = true;
                caster->CastSpell(unit, i->first->Id, args);
                // SPELL_AURA_ADD_TARGET_TRIGGER auras shouldn't trigger auras without duration
                // set duration equal to triggering spell
                if (i->first->GetDuration() == -1)
                {
                    // get duration from aura-only once
                    if (!_duration)
                    {
                        Aura * aur = unit->GetAuraByCasterSpell(m_spellInfo->Id, caster->GetGUID());
                        _duration = aur ? aur->GetDuration() : -1;
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
            for(int i : *spell_triggered) 
            {
                if(i < 0)
                    unit->RemoveAurasDueToSpell(-i);
                else
                {
                    CastSpellExtraArgs args;
                    args.TriggerFlags = TRIGGERED_FULL_MASK;
                    args.SetOriginalCaster(caster->GetGUID());
                    unit->CastSpell(unit, i, args);
                }
            }
        }
    }

    return SPELL_MISS_NONE;
}

void Spell::DoAllEffectOnTarget(GOTargetInfo* target)
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
            HandleEffects(nullptr,nullptr,go,effectNumber, SPELL_EFFECT_HANDLE_HIT_TARGET);

    if (go->AI())
        go->AI()->SpellHit(m_caster, m_spellInfo);

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
            HandleEffects(nullptr, target->item, nullptr, effectNumber, SPELL_EFFECT_HANDLE_HIT_TARGET);

    CallScriptOnHitHandlers();
    CallScriptAfterHitHandlers();
}

/*
void Spell::SetTargetMap(uint32 i, uint32 cur)
{
        ...

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
                Trinity::Containers::RandomResize(unitList, m_spellValue->MaxAffectedTargets);
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
uint32 Spell::prepare(SpellCastTargets const& targets, Aura const* triggeredByAura)
{
    if(m_CastItem)
        m_castItemGUID = m_CastItem->GetGUID();
    else
        m_castItemGUID = ObjectGuid::Empty;

    InitExplicitTargets(targets);

    // Fill aura scaling information
    if (m_caster->IsControlledByPlayer() && !m_spellInfo->IsPassive() && m_spellInfo->SpellLevel && !m_spellInfo->IsChanneled() && !(_triggeredCastFlags & TRIGGERED_IGNORE_AURA_SCALING))
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
    _spellEvent = new SpellEvent(this);
    m_caster->m_Events.AddEvent(_spellEvent, m_caster->m_Events.CalculateTime(1));

    //Another spell in progress ?
    if(m_caster->IsNonMeleeSpellCast(false, true) && m_cast_count && !(_triggeredCastFlags & TRIGGERED_IGNORE_CAST_IN_PROGRESS))
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

    LoadScripts();

    // Fill cost data
    m_powerCost = CalculatePowerCost(); //TC has:   m_powerCost = m_CastItem ? 0 : m_spellInfo->CalcPowerCost(m_caster, m_spellSchoolMask, this);

    // Set combo point requirement
    if ((_triggeredCastFlags & TRIGGERED_IGNORE_COMBO_POINTS) || m_CastItem)
        m_needComboPoints = false;

    SpellCastResult result = CheckCast(true);
    //TC_LOG_DEBUG("FIXME","CheckCast for %u : %u", m_spellInfo->Id, result);
    if(result != SPELL_CAST_OK && !IsAutoRepeat()) //always cast autorepeat dummy for triggering
    {
        if(triggeredByAura)
            const_cast<Aura*>(triggeredByAura)->SetDuration(0);

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
        // 1. Has casttime, 2. Or doesn't have flag to allow movement during channel
        if (m_casttime || !m_spellInfo->IsMoveAllowedChannel())
        {
            SendCastResult(SPELL_FAILED_MOVING);
            finish(false);
            return SPELL_FAILED_MOVING;
        }
    }

    // sunwell: if spell have nearby target entry only, do not allow to cast if no targets are found
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

            // sunwell: by default set it to false, and to true if any valid target is found
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

    //Containers for channeled spells have to be set
    // Why check duration? 29350: channelled triggers channelled
    if((_triggeredCastFlags & TRIGGERED_CAST_DIRECTLY) && (!m_spellInfo->IsChanneled() || !m_spellInfo->GetMaxDuration()))
        cast(true);
    else
    {
        // stealth must be removed at cast starting (at show channel bar)
        // skip triggered spell (item equip spell casting and other not explicit character casts/item uses)
        if(!(_triggeredCastFlags & TRIGGERED_IGNORE_AURA_INTERRUPT_FLAGS) && m_spellInfo->IsBreakingStealth())
        {
            m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CAST);
            for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                if (m_spellInfo->Effects[i].GetUsedTargetObjectType() == TARGET_OBJECT_TYPE_UNIT)
                {
                    m_caster->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_SPELL_ATTACK);
                    break;
                }
        }

        m_caster->SetCurrentCastedSpell( this );
        m_selfContainer = &(m_caster->m_currentSpells[GetCurrentContainer()]);
        SendSpellStart();
        
        // set target for proper facing
        if (m_casttime && !(_triggeredCastFlags & TRIGGERED_IGNORE_SET_FACING))
            if (ObjectGuid target = m_targets.GetUnitTargetGUID())
                if(Unit* uTarget = ObjectAccessor::GetUnit(*m_caster,target))
                    if (m_caster->GetGUID() != target && m_caster->GetTypeId() == TYPEID_UNIT)
                        m_caster->ToCreature()->FocusTarget(this, uTarget);

        if(m_caster->GetTypeId() == TYPEID_PLAYER)
            (m_caster->ToPlayer())->AddGlobalCooldown(m_spellInfo,this);
            
        TriggerGlobalCooldown();

        if(    !m_casttime 
            && !m_castItemGUID     //item: first cast may destroy item and second cast causes crash
            && GetCurrentContainer() == CURRENT_GENERIC_SPELL)
            cast(true);
    }
    return uint32(SPELL_CAST_OK);
}

void Spell::cancel()
{
    //TC_LOG_DEBUG("FIXME","Spell %u - cancel()", m_spellInfo->Id);
    if(m_spellState == SPELL_STATE_FINISHED)
        return;

    SetReferencedFromCurrent(false);
    if(m_selfContainer && *m_selfContainer == this)
        *m_selfContainer = nullptr;

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
            for(auto & ihit : m_UniqueTargetInfo)
                if( ihit.missCondition == SPELL_MISS_NONE )
                    if(Unit* unit = m_caster->GetGUID()==ihit.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit.targetGUID))
                        unit->RemoveAurasByCasterSpell(m_spellInfo->Id, m_caster->GetGUID());

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
        } break;

        default:
        {
        } break;
    }

    if(m_caster->GetTypeId() == TYPEID_PLAYER)
        (m_caster->ToPlayer())->RemoveGlobalCooldown(m_spellInfo);

    m_caster->RemoveDynObject(m_spellInfo->Id);
    m_caster->RemoveGameObject(m_spellInfo->Id, true);

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
        if(!IsTriggered() && pTarget->IsAlive() && (pTarget->HasAuraType(SPELL_AURA_MOD_STEALTH) || pTarget->HasAuraType(SPELL_AURA_MOD_INVISIBILITY)) && !pTarget->IsFriendlyTo(m_caster) && !pTarget->CanSeeOrDetect(m_caster, false))
        {
            SendCastResult(SPELL_FAILED_BAD_TARGETS);
            finish(false);
            return;
        }

    }

    if (Player* playerCaster = m_caster->ToPlayer())
    {
        // now that we've done the basic check, now run the scripts
        // should be done before the spell is actually executed
        // sScriptMgr->OnPlayerSpellCast(playerCaster, this, skipCheck);

        // As of 3.0.2 pets begin attacking their owner's target immediately
        // Let any pets know we've attacked something. Check DmgClass for harmful spells only
        // This prevents spells such as Hunter's Mark from triggering pet attack
        if (this->GetSpellInfo()->DmgClass != SPELL_DAMAGE_CLASS_NONE)
            if (Unit* newTarget = m_targets.GetUnitTarget())
                for (Unit* controlled : playerCaster->m_Controlled)
                    if (Creature* cControlled = controlled->ToCreature())
                        if (cControlled->IsAIEnabled)
                            cControlled->AI()->OwnerAttacked(newTarget);
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

    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_SET_FACING))
        if(m_caster->GetTypeId() != TYPEID_PLAYER && m_targets.GetUnitTarget() && m_targets.GetUnitTarget() != m_caster)
            m_caster->SetInFront(m_targets.GetUnitTarget());

    CallScriptBeforeCastHandlers();

    if(!(_triggeredCastFlags & TRIGGERED_IGNORE_POWER_AND_REAGENT_COST))
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
        auto cleanupSpell = [this](SpellCastResult res, uint32* p1 = nullptr, uint32* p2 = nullptr)
        {
            SendCastResult(res, p1, p2);
            SendInterrupted(0);
            finish(false);
            SetExecutedCurrently(false);
        };

        uint32 param1 = 0, param2 = 0;
        castResult = CheckCast(false);
        //TC_LOG_DEBUG("FIXME","CheckCast for %u : %u", m_spellInfo->Id, castResult);
        if(castResult != SPELL_CAST_OK)
        {
            cleanupSpell(castResult, &param1, &param2);
            return;
        }

        // check diminishing returns (again, only after finish cast bar, tested on retail)
        if (Unit* target = m_targets.GetUnitTarget())
        {
            uint8 aura_effmask = 0;
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                if (m_spellInfo->Effects[i].IsUnitOwnedAuraEffect())
                    aura_effmask |= 1 << i;

            if (aura_effmask)
            {
                bool const triggered = m_triggeredByAuraSpell != nullptr;
                if (DiminishingGroup diminishGroup = m_spellInfo->GetDiminishingReturnsGroupForSpell(triggered))
                {
                    DiminishingReturnsType type = m_spellInfo->GetDiminishingReturnsGroupType(triggered);
                    if (type == DRTYPE_ALL || (type == DRTYPE_PLAYER && target->IsAffectedByDiminishingReturns()))
                    {
                        Unit* caster = m_originalCaster ? m_originalCaster : m_caster;
                        if (target->HasStrongerAuraWithDR(m_spellInfo, caster, triggered))
                        {
                            cleanupSpell(SPELL_FAILED_MORE_POWERFUL_SPELL_ACTIVE);
                            return;
                        }
                    }
                }
            }
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

    if (m_spellInfo->HasAttribute(SPELL_ATTR1_DISMISS_PET))
        if (Creature* pet = ObjectAccessor::GetCreature(*m_caster, m_caster->GetMinionGUID()))
            pet->DespawnOrUnsummon();

    CallScriptOnCastHandlers();

    // traded items have trade slot instead of guid in m_itemTargetGUID
    // set to real guid to be sent later to the client
    m_targets.UpdateTradeSlotItem();

    if(!(_triggeredCastFlags & TRIGGERED_IGNORE_POWER_AND_REAGENT_COST))
    {
        TakePower();  // combo points should not be taken before SPELL_AURA_ADD_TARGET_TRIGGER auras are handled
        if(m_spellInfo->Effects[0].Effect != SPELL_EFFECT_TRIGGER_SPELL_2)
            TakeReagents();   // we must remove reagents before HandleEffects to allow place crafted item in same slot
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
    for(auto targetTrigger : targetTriggers)
    {
        SpellInfo const *auraSpellInfo = targetTrigger->GetSpellInfo();
        uint32 auraSpellIdx = targetTrigger->GetEffIndex();
        if (IsAffectedBy(auraSpellInfo, auraSpellIdx))
        {
            if(SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(auraSpellInfo->Effects[auraSpellIdx].TriggerSpell))
            {
                // Calculate chance at that moment (can be depend for example from combo points)
                int32 chance = m_caster->CalculateSpellDamage(auraSpellInfo, auraSpellIdx, targetTrigger->GetBasePoints(), nullptr);
                m_ChanceTriggerSpells.push_back(std::make_pair(spellInfo, chance * targetTrigger->GetStackAmount()));
            }
        }
    }
    
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


    if(m_spellInfo->HasAttribute(SPELL_ATTR_CU_LINK_CAST))
    {
        if(const std::vector<int32> *spell_triggered = sSpellMgr->GetSpellLinked(m_spellInfo->Id))
        {
            for(int i : *spell_triggered)
            {
                if(i < 0)
                    m_caster->RemoveAurasDueToSpell(-i);
                else
                {
                    if(m_targets.GetUnitTarget())
                    {
                        for(auto itr : m_UniqueTargetInfo)
                        {
                            Unit* linkCastTarget = ObjectAccessor::GetUnit(*m_caster, itr.targetGUID);
                            if(linkCastTarget)
                                m_caster->CastSpell(linkCastTarget, i, TRIGGERED_FULL_MASK);
                        }
                    } else {
                        m_caster->CastSpell(m_caster, i, TRIGGERED_FULL_MASK);
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
    /* Commented: Is this really a valid rule?
    if (Unit* target = m_targets.GetUnitTarget())
        if (m_caster->GetTypeId() == TYPEID_PLAYER || (m_caster->IsPet() && m_caster->IsControlledByPlayer()))
            if (GetDelayMoment() > 0 && !m_caster->IsFriendlyTo(target) && !m_spellInfo->HasAura(SPELL_AURA_BIND_SIGHT) && (!m_spellInfo->IsPositive() || m_spellInfo->HasEffect(SPELL_EFFECT_DISPEL)))
                if (!m_spellInfo->HasAttribute(SPELL_ATTR3_NO_INITIAL_AGGRO))
                    m_caster->CombatStart(target);
                 //m_caster->CombatStartOnCast(target, !m_spellInfo->HasAttribute(SPELL_ATTR3_NO_INITIAL_AGGRO), GetDelayMoment() + 500); // sunwell: increase this time so we dont leave and enter combat in a moment
                 */

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

    for(auto & ihit : m_UniqueTargetInfo)
        DoAllEffectOnTarget(&ihit);

    for(auto & ihit : m_UniqueGOTargetInfo)
        DoAllEffectOnTarget(&ihit);

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
    for(auto & ihit : m_UniqueTargetInfo)
    {
        if (ihit.processed == false)
        {
            if (single_missile || ihit.timeDelay <= t_offset)
            {
                ihit.timeDelay = t_offset;
                DoAllEffectOnTarget(&ihit);
            }
            else if( next_time == 0 || ihit.timeDelay < next_time )
                next_time = ihit.timeDelay;
        }
    }

    // now recheck gameobject targeting correctness
    for(auto & ighit : m_UniqueGOTargetInfo)
    {
        if (ighit.processed == false)
        {
            if ( single_missile || ighit.timeDelay <= t_offset )
                DoAllEffectOnTarget(&ighit);
            else if( next_time == 0 || ighit.timeDelay < next_time )
                next_time = ighit.timeDelay;
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
    HandleFlatThreat();

    PrepareScriptHitHandlers();

    m_needSpellLog = IsNeedSendToClient();
    // handle effects with SPELL_EFFECT_HANDLE_HIT mode
    for(uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
    {
        // don't do anything for empty effect
        if (!m_spellInfo->Effects[j].IsEffect())
            continue;

         HandleEffects(m_originalCaster, nullptr, nullptr, j, SPELL_EFFECT_HANDLE_HIT);

         // Don't do spell log, if is school damage spell
         if (m_spellInfo->Effects[j].Effect == SPELL_EFFECT_SCHOOL_DAMAGE || m_spellInfo->Effects[j].Effect == 0)
             m_needSpellLog = false;
    }

    // process items
    for (auto & ihit : m_UniqueItemInfo)
        DoAllEffectOnTarget(&ihit);
}

void Spell::_handle_finish_phase()
{
    // Take for real after all targets are processed
    if (m_needComboPoints && m_caster->GetTypeId() == TYPEID_PLAYER)
        m_caster->ToPlayer()->ClearComboPoints();

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
    if (m_spellInfo->IsCooldownStartedOnEvent())
    {
        _player->AddSpellCooldown(m_spellInfo->Id, 0, time(nullptr) - 1);
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
            for(const auto & Spell : proto->Spells)
            {
                if(Spell.SpellId == m_spellInfo->Id)
                {
                    cat    = Spell.SpellCategory;
                    rec    = Spell.SpellCooldown;
                    catrec = Spell.SpellCategoryCooldown;
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

    time_t curTime = time(nullptr);

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
            for(uint32 i_scset : i_scstore->second)
            {
                if(i_scset == m_spellInfo->Id)             // skip main spell, already handled above
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
        if (!IsNextMeleeSwingSpell() && !IsAutoRepeat() && !IsTriggered() && !(IsChannelActive() && m_spellInfo->IsMoveAllowedChannel()))
        {
            // if charmed by creature, trust the AI not to cheat and allow the cast to proceed
            // @todo this is a hack, "creature" movesplines don't differentiate turning/moving right now
            // however, checking what type of movement the spline is for every single spline would be really expensive
            if (!(m_caster->GetCharmerGUID().IsCreature()))
                cancel();
        }
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
                    // TC_LOG_DEBUG("spells", "Channeled spell %d is removed due to lack of targets", m_spellInfo->Id);

                    /* TC Code 
                    m_timer = 0;
                    // Also remove applied auras
                    for (TargetInfo const& target : m_UniqueTargetInfo)
                        if (Unit* unit = m_caster->GetGUID() == target.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, target.targetGUID))
                            unit->RemoveOwnedAura(m_spellInfo->Id, m_originalCasterGUID, 0, AURA_REMOVE_BY_CANCEL);
                            */
                    if (m_spellInfo->HasVisual(788) && m_spellInfo->SpellIconID == 113 && m_spellInfo->SpellFamilyName == 5) { // Drain soul hack, must remove aura on caster
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
                    for(auto & ihit : m_UniqueTargetInfo)
                    {
                        TargetInfo* target = &ihit;
                        if(!target->targetGUID.IsCreature())
                            continue;

                        Unit* unit = m_caster->GetGUID()==target->targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster,target->targetGUID);
                        if (unit==nullptr)
                            continue;

                        (m_caster->ToPlayer())->CastedCreatureOrGO(unit->GetEntry(),unit->GetGUID(),m_spellInfo->Id);
                    }

                    for(auto & ihit : m_UniqueGOTargetInfo)
                    {
                        GOTargetInfo* target = &ihit;

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

    // Unsummon summon as possessed creatures on spell cancel
    if (m_spellInfo->IsChanneled() && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (Unit* charm = m_caster->GetCharm())
            if (charm->GetTypeId() == TYPEID_UNIT
                && charm->ToCreature()->HasUnitTypeMask(UNIT_MASK_PUPPET)
                && charm->GetUInt32Value(UNIT_CREATED_BY_SPELL) == m_spellInfo->Id)
                ((Puppet*)charm)->UnSummon();
    }

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
        
    //if (ok && !IsTriggered() && m_caster->GetTypeId() == TYPEID_PLAYER && m_caster->HasAuraEffect(43983) && m_spellInfo->PowerType == POWER_MANA)
    //    m_caster->CastSpell(m_caster, 43137, TRIGGERED_FULL_MASK);
}


void Spell::WriteCastResultInfo(WorldPacket& data, Player* caster, SpellInfo const* spellInfo, uint8 castCount, SpellCastResult result, /*SpellCustomErrors customError, */ uint32* param1 /*= nullptr*/, uint32* param2 /*= nullptr*/)
{
    data << uint32(spellInfo->Id);
    data << uint8(result);                              // problem
    data << uint8(castCount);                        // single cast or multi 2.3 (0/1)
    switch (result)
    {
    case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
        if (param1)
            data << uint32(*param1);
        else
            data << uint32(spellInfo->RequiresSpellFocus);
        break;
    case SPELL_FAILED_REQUIRES_AREA:
        // hardcode areas limitation case
        if (param1)
            data << uint32(*param1);
        else
        {
            switch (spellInfo->Id)
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
                data << uint32(spellInfo->AreaId);
                break;
            }
        }
        break;
    case SPELL_FAILED_TOTEMS:
        if (param1)
        {
            data << uint32(*param1);
            if (param2)
                data << uint32(*param2);
        }
        else {
            if (spellInfo->Totem[0])
                data << uint32(spellInfo->Totem[0]);
            if (spellInfo->Totem[1])
                data << uint32(spellInfo->Totem[1]);
        }
        break;
    case SPELL_FAILED_TOTEM_CATEGORY:
        if (param1)
        {
            data << uint32(*param1);
            if (param2)
                data << uint32(*param2);
        }
        else {
            if (spellInfo->TotemCategory[0])
                data << uint32(spellInfo->TotemCategory[0]);
            if (spellInfo->TotemCategory[1])
                data << uint32(spellInfo->TotemCategory[1]);
        }
        break;
    case SPELL_FAILED_EQUIPPED_ITEM_CLASS:
        if (param1)
            data << uint32(*param1);
        else {
            data << uint32(spellInfo->EquippedItemClass);
            data << uint32(spellInfo->EquippedItemSubClassMask);
            data << uint32(spellInfo->EquippedItemInventoryTypeMask);
        }
        break;
#ifdef LICH_KING
    case SPELL_FAILED_TOO_MANY_OF_ITEM:
        if (param1)
            data << uint32(*param1);
        else
        {
            uint32 item = 0;
            for (uint8 effIndex = 0; effIndex < MAX_SPELL_EFFECTS && !item; ++effIndex)
                if (uint32 itemType = spellInfo->Effects[effIndex].ItemType)
                    item = itemType;

            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item);
            if (proto && proto->ItemLimitCategory)
                data << uint32(proto->ItemLimitCategory);
        }
        break;
    case SPELL_FAILED_CUSTOM_ERROR:
        data << uint32(customError);
        break;

    case SPELL_FAILED_NEED_MORE_ITEMS:
        if (param1 && param2)
        {
            data << uint32(*param1);
            data << uint32(*param2);
        }
        else
        {
            data << uint32(0); // Item entry
            data << uint32(0); // Count
        }
        break;
    case SPELL_FAILED_FISHING_TOO_LOW:
        if (param1)
            data << uint32(*param1);
        else
            data << uint32(0); // Skill level
        break;
    case SPELL_FAILED_NEED_EXOTIC_AMMO:
        if (param1)
            data << uint32(*param1);
        else
            data << uint32(spellInfo->EquippedItemSubClassMask);
        break;
#endif
    case SPELL_FAILED_PREVENTED_BY_MECHANIC: //BC OK?
        if (param1)
            data << uint32(*param1);
        else
            data << uint32(spellInfo->Mechanic);
        break;
    case SPELL_FAILED_MIN_SKILL: //BC OK?
        if (param1 && param2)
        {
            data << uint32(*param1);
            data << uint32(*param2);
        }
        else
        {
            data << uint32(0); // SkillLine.dbc Id
            data << uint32(0); // Amount
        }
        break;
    case SPELL_FAILED_REAGENTS: //BC OK?
    {
        if (param1)
            data << uint32(*param1);
        else
        {
            uint32 missingItem = 0;
            for (uint32 i = 0; i < MAX_SPELL_REAGENTS; i++)
            {
                if (spellInfo->Reagent[i] <= 0)
                    continue;

                uint32 itemid = spellInfo->Reagent[i];
                uint32 itemcount = spellInfo->ReagentCount[i];

                if (!caster->HasItemCount(itemid, itemcount))
                {
                    missingItem = itemid;
                    break;
                }
            }

            data << uint32(missingItem);  // first missing item
        }
        break;
    }
    default:
        break;
    }
}

void Spell::SendCastResult(Player* caster, SpellInfo const* spellInfo, uint8 castCount, SpellCastResult result, /*SpellCustomErrors customError,*/ uint32* param1 /*= nullptr*/, uint32* param2 /*= nullptr*/)
{
    if (result == SPELL_CAST_OK)
        return;

    WorldPacket data(SMSG_CAST_FAILED, 1 + 4 + 1);
    WriteCastResultInfo(data, caster, spellInfo, castCount, result, /* customError, */param1, param2);

    caster->SendDirectMessage(&data);
}

void Spell::SendCastResult(SpellCastResult result, uint32* param1 /*= nullptr*/, uint32* param2 /*= nullptr*/) const
{
    if (m_caster->GetTypeId() != TYPEID_PLAYER)
        return;

    if((m_caster->ToPlayer())->GetSession()->PlayerLoading())  // don't send cast results at loading time
        return;

    if(result != SPELL_CAST_OK)
    {
        if (_triggeredCastFlags & TRIGGERED_DONT_REPORT_CAST_ERROR)
            result = SPELL_FAILED_DONT_REPORT;

        WorldPacket data(SMSG_CAST_FAILED, (4+1+1));
        SendCastResult(m_caster->ToPlayer(), m_spellInfo, m_cast_count, result, /*m_customError, */ param1, param2);
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

#ifdef LICH_KING
    uint32 schoolImmunityMask = m_caster->GetSchoolImmunityMask();
    uint32 mechanicImmunityMask = m_caster->GetMechanicImmunityMask();
    if (schoolImmunityMask || mechanicImmunityMask)
        castFlags |= CAST_FLAG_IMMUNITY;
#endif

    if (((IsTriggered() && !m_spellInfo->IsAutoRepeatRangedSpell()) || m_triggeredByAuraSpell) && !m_spellInfo->IsChanneled() && !m_cast_count)
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
    if( ((IsTriggered() && !m_spellInfo->IsAutoRepeatRangedSpell()) || m_triggeredByAuraSpell) && !m_cast_count)
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
                    ammoInventoryType = INVTYPE_AMMO;
                    ammoDisplayID = 5998;
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
    for (auto & ihit : m_UniqueTargetInfo)
    {
        if (ihit.effectMask == 0)                  // No effect apply - all immuned add state
                                                      // possibly SPELL_MISS_IMMUNE2 for this??
            ihit.missCondition = SPELL_MISS_IMMUNE2;
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
            // sunwell: WTF is this? No channeled spell checked, no anything
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
    // sunwell: Why do we reset something that is not set??????
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
#ifdef LICH_KING
    WorldPacket data(SMSG_SPELL_FAILURE, (8+1+4+1));
#else
    WorldPacket data(SMSG_SPELL_FAILURE, (8 + 4 + 1));
#endif
    data << m_caster->GetPackGUID();
#ifdef LICH_KING
    data << uint8(m_cast_count);
#endif
    data << m_spellInfo->Id;
    data << result;
    m_caster->SendMessageToSet(&data, true);

    data.Initialize(SMSG_SPELL_FAILED_OTHER, (8+4));
    data << m_caster->GetPackGUID();
#ifdef LICH_KING
    data << uint8(m_cast_count);
#endif
    data << m_spellInfo->Id;
    m_caster->SendMessageToSet(&data, true);
}

void Spell::SendChannelUpdate(uint32 time)
{
    if(time == 0)
    {
        m_caster->SetGuidValue(UNIT_FIELD_CHANNEL_OBJECT, ObjectGuid::Empty);
        m_caster->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
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
    WorldObject* target = nullptr;

    // select first not resisted target from target list for _0_ effect
    if(!m_UniqueTargetInfo.empty())
    {
        for(auto & itr : m_UniqueTargetInfo)
        {
            if( (itr.effectMask & (1<<0)) && itr.reflectResult==SPELL_MISS_NONE && itr.targetGUID != m_caster->GetGUID())
            {
                target = ObjectAccessor::GetUnit(*m_caster, itr.targetGUID);
                break;
            }
        }
    }
    else if(!m_UniqueGOTargetInfo.empty())
    {
        for(auto & itr : m_UniqueGOTargetInfo)
        {
            if(itr.effectMask & (1<<0) )
            {
                target = ObjectAccessor::GetGameObject(*m_caster, itr.targetGUID);
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
        m_caster->SetGuidValue(UNIT_FIELD_CHANNEL_OBJECT, target->GetGUID());
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

    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 8+4);
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
    if (_triggeredCastFlags & TRIGGERED_IGNORE_CAST_ITEM)
        return;

    //Duel area free first aid
    if ( m_caster->ToPlayer()->IsInDuelArea() && IsFreeInDuelArea(m_CastItem->GetEntry()) )
       return;

    ItemTemplate const *proto = m_CastItem->GetTemplate();

    if(!proto)
    {
        // This code is to avoid a crash
        // I'm not sure, if this is really an error, but I guess every item needs a prototype
        TC_LOG_ERROR("spells","Cast item has no item prototype highId=%d, lowId=%d", int32(m_CastItem->GetGUID().GetHigh()), m_CastItem->GetGUID().GetCounter());
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
        if(m_CastItem == m_targets.GetItemTarget())
            m_targets.SetItemTarget(nullptr);

        m_CastItem = nullptr;
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
            if(ObjectGuid targetGUID = m_targets.GetUnitTargetGUID())
                for(auto & ihit : m_UniqueTargetInfo)
                    if(ihit.targetGUID == targetGUID)
                    {
                        if(ihit.missCondition != SPELL_MISS_NONE && ihit.missCondition != SPELL_MISS_MISS/* && ihit->targetGUID!=m_caster->GetGUID()*/)
                            hit = false;
                        break;
                    }
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
        TC_LOG_ERROR("spells","Spell::TakePower: Unknown power type '%d'", m_spellInfo->PowerType);
        return;
    }

    Powers powerType = Powers(m_spellInfo->PowerType);

    if(hit)
        m_caster->ModifyPower(powerType, -m_powerCost);
    else
        m_caster->ModifyPower(powerType, -irand(0, m_powerCost/4));

    // Set the five second timer
    if (powerType == POWER_MANA && m_powerCost > 0)
        m_caster->SetLastManaUse(GetMSTime());
}

void Spell::TakeReagents()
{
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

                m_CastItem = nullptr;
            }
        }

        // if getItemTarget is also spell reagent
        if (m_targets.GetItemTargetEntry()==itemid)
            m_targets.SetItemTarget(nullptr);

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

    for(auto & target : m_UniqueTargetInfo)
    {
        Unit* targetUnit = ObjectAccessor::GetUnit(*m_caster, target.targetGUID);
        if (!targetUnit)
            continue;

        float threat = flatMod / targetListSize;;

        //apply threat to every negative targets
        if(!IsPositive(!m_caster->IsFriendlyTo(targetUnit)))
            targetUnit->GetThreatManager().AddThreat(m_caster, threat, m_spellInfo);
        else //or assist threat if friendly target
            m_caster->GetThreatManager().ForwardThreatForAssistingMe(targetUnit, threat, m_spellInfo);
    }
}

void Spell::HandleEffects(Unit* pUnitTarget, Item* pItemTarget, GameObject* pGOTarget, uint32 i, SpellEffectHandleMode mode)
{
    effectHandleMode = mode;
    unitTarget = pUnitTarget;
    itemTarget = pItemTarget;
    gameObjTarget = pGOTarget;
    destTarget = &m_destTargets[i]._position;

    //Simply return. Do not display "immune" in red text on client
    if(unitTarget && unitTarget->IsImmunedToSpellEffect(m_spellInfo, i, m_caster))
        return;

    //we do not need DamageMultiplier here.
    damage = CalculateDamage(i, nullptr);

    bool preventDefault = CallScriptEffectHandlers((SpellEffIndex)i, mode);

    uint8 eff = m_spellInfo->Effects[i].Effect;
    if(!preventDefault && eff < TOTAL_SPELL_EFFECTS)
        (*this.*SpellEffects[eff])((SpellEffIndex)i);
}

void Spell::TriggerSpell()
{
    for(auto & m_TriggerSpell : m_TriggerSpells)
    {
        auto  spell = new Spell(m_caster, m_TriggerSpell, TRIGGERED_FULL_MASK, m_originalCasterGUID, m_selfContainer, true);
        spell->prepare(m_targets);                         // use original spell original targets
    }
}

//Called with strict at cast start + with not strict at cast end if spell has a cast time
//strict = check for stealth aura + check IsNonMeleeSpellCast
//return : -1 = ok, everything else : see enum SpellCastResult
SpellCastResult Spell::CheckCast(bool strict)
{
    if (m_spellInfo->Effects[0].Effect == SPELL_EFFECT_STUCK) //skip stuck spell to allow use it in falling case 
        return SPELL_CAST_OK;

    // check death state
    if (!m_caster->IsAlive() && !m_spellInfo->IsPassive() && !(m_spellInfo->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_DEAD) || (IsTriggered() && !m_triggeredByAuraSpell)))
        return SPELL_FAILED_CASTER_DEAD;

    // check cooldowns to prevent cheating
    if (!m_spellInfo->IsPassive())
    {
#ifdef LICH_KING
        if (m_caster->GetTypeId() == TYPEID_PLAYER)
        {
            //can cast triggered (by aura only?) spells while have this flag
            if (!(_triggeredCastFlags & TRIGGERED_IGNORE_CASTER_AURASTATE) && m_caster->ToPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY))
                return SPELL_FAILED_SPELL_IN_PROGRESS;

            // check if we are using a potion in combat for the 2nd+ time. Cooldown is added only after caster gets out of combat
            if (!IsIgnoringCooldowns() && m_caster->ToPlayer()->GetLastPotionId() && m_CastItem && (m_CastItem->IsPotion() || m_spellInfo->IsCooldownStartedOnEvent()))
                return SPELL_FAILED_NOT_READY;
        }
#endif

        if (!IsIgnoringCooldowns() && m_caster->GetTypeId() == TYPEID_PLAYER && ((m_caster->ToPlayer())->HasSpellCooldown(m_spellInfo->Id)))
        {
            if (m_triggeredByAuraSpell)
                return SPELL_FAILED_DONT_REPORT;
            else
                return SPELL_FAILED_NOT_READY;
        }
    }

    // Check global cooldown
    if (strict && !(_triggeredCastFlags & TRIGGERED_IGNORE_GCD) && HasGlobalCooldown())
        return !m_spellInfo->HasAttribute(SPELL_ATTR0_DISABLED_WHILE_ACTIVE) ? SPELL_FAILED_NOT_READY : SPELL_FAILED_DONT_REPORT;

    Unit *target = m_targets.GetUnitTarget();
    if(!target)
        target = m_caster;
    
    /* Disabled, this is wrong. For example with spell 39298, spell should be able to be casted at self, but self should not be an affected target. (So I guess this attributes only has sense with aoe spells)
    if(m_caster == target)
        if (m_spellInfo->HasAttribute(SPELL_ATTR1_CANT_TARGET_SELF))
            return SPELL_FAILED_BAD_TARGETS;
            */

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
            else if (!target->HasMainWeapon())
                return SPELL_FAILED_TARGET_NO_WEAPONS;
        }

        // target state requirements (apply to non-self only), to allow cast affects to self like Dirty Deeds
        if(m_spellInfo->TargetAuraState && !target->HasAuraState(AuraStateType(m_spellInfo->TargetAuraState)))
            return SPELL_FAILED_TARGET_AURASTATE;

        if(    !(m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS)) 
            && !m_spellInfo->HasAttribute(SPELL_ATTR5_SKIP_CHECKCAST_LOS_CHECK)
            && !m_caster->IsWithinLOSInMap(target, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2) )
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
        if(m_caster->GetTypeId() == TYPEID_UNIT && m_caster->GetCharmerOrOwnerGUID() && !IsTriggered())
        {
            // check correctness positive/negative cast target (pet cast real check and cheating check)
            bool hostileTarget = m_caster->IsHostileTo(target);
            if(IsPositive(hostileTarget))
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

        // Must be behind the target.
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

    if(m_caster->GetTypeId() == TYPEID_PLAYER && VMAP::VMapFactory::createOrGetVMapManager()->isLineOfSightCalcEnabled())
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
    if( strict && !(_triggeredCastFlags & TRIGGERED_IGNORE_SHAPESHIFT))
    {
#ifdef LICH_KING
        bool checkForm = true;
        // Ignore form req aura
        Unit::AuraEffectList const& ignore = m_caster->GetAuraEffectsByType(SPELL_AURA_MOD_IGNORE_SHAPESHIFT);
        for (AuraEffect const* aurEff : ignore)
        {
            if (!aurEff->IsAffectedOnSpell(m_spellInfo))
                continue;

            checkForm = false;
            break;
        }

        if(checkForm)
#endif
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
    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_CASTER_AURASTATE))
    {
        if (m_spellInfo->CasterAuraState && !m_caster->HasAuraState(AuraStateType(m_spellInfo->CasterAuraState)))
            return SPELL_FAILED_CASTER_AURASTATE;
        if (m_spellInfo->CasterAuraStateNot && m_caster->HasAuraState(AuraStateType(m_spellInfo->CasterAuraStateNot)))
            return SPELL_FAILED_CASTER_AURASTATE;

        // check combat state
        if (!m_spellInfo->CanBeUsedInCombat())
        {
            if (m_caster->IsInCombat())
                return SPELL_FAILED_AFFECTING_COMBAT;

            // sunstrider: block non combat spells while we got in air projectiles. This avoid having players casting a spell then directly stealthing. This is retail behavior.
            if (!IsTriggered() && (m_caster->HasDelayedSpell() || m_caster->m_currentSpells[CURRENT_AUTOREPEAT_SPELL]))
                return SPELL_FAILED_DONT_REPORT;
        }
    }

    // cancel autorepeat spells if cast start when moving
    // (not wand currently autorepeat cast delayed to moving stop anyway in spell update code)
    if( m_caster->GetTypeId()==TYPEID_PLAYER && (m_caster->ToPlayer())->isMoving() && (!m_caster->IsCharmed() || !m_caster->GetCharmerGUID().IsCreature()))
    {
        // apply spell limitations at movement
        if( (!m_caster->HasUnitMovementFlag(MOVEMENTFLAG_JUMPING_OR_FALLING)) &&
            (IsAutoRepeat() || (m_spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_SEATED) != 0) )
            return SPELL_FAILED_MOVING;
    }

    // check spell cast conditions from database
    {
        ConditionSourceInfo condInfo = ConditionSourceInfo(m_caster, m_targets.GetObjectTarget());
        if (!sConditionMgr->IsObjectMeetingNotGroupedConditions(CONDITION_SOURCE_TYPE_SPELL, m_spellInfo->Id, condInfo))
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

    // Don't check explicit target for passive spells (workaround) (check should be skipped only for learn case)
    // those spells may have incorrect target entries or not filled at all (for example 15332)
    // such spells when learned are not targeting anyone using targeting system, they should apply directly to caster instead
    // also, such casts shouldn't be sent to client
    // sunwell: do not check explicit casts for self cast of triggered spells (eg. reflect case)
    if (!(m_spellInfo->HasAttribute(SPELL_ATTR0_PASSIVE) && (!m_targets.GetUnitTarget() || m_targets.GetUnitTarget() == m_caster)))
    {
        // Check explicit target for m_originalCaster - todo: get rid of such workarounds
        // sunwell: do not check explicit target for triggered spell casted on self with targetflag enemy
        if (!m_triggeredByAuraSpell || m_targets.GetUnitTarget() != m_caster || !(m_spellInfo->GetExplicitTargetMask() & TARGET_FLAG_UNIT_ENEMY))
        {
            Unit* caster = (m_originalCaster && m_caster->GetEntry() != WORLD_TRIGGER) ? m_originalCaster : m_caster;
            SpellCastResult castResult = m_spellInfo->CheckExplicitTarget(caster, m_targets.GetObjectTarget(), m_targets.GetItemTarget());
            if (castResult != SPELL_CAST_OK)
                return castResult;
        }
    }

    // prevent casting at immune friendly target
    if(IsPositive(!m_caster->IsFriendlyTo(target)) && target->IsImmunedToSpell(m_spellInfo, m_caster))
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
    if( m_caster->IsMounted() && m_caster->GetTypeId() == TYPEID_PLAYER && !(_triggeredCastFlags & TRIGGERED_IGNORE_CASTER_MOUNTED_OR_ON_VEHICLE) &&
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

    SpellCastResult castResult = SPELL_CAST_OK;

    if (!IsTriggered())
    {
        castResult = CheckRange(strict);
        if (castResult != SPELL_CAST_OK)
            return castResult;
    }

    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_POWER_AND_REAGENT_COST))
    {
        castResult = CheckPower();
        if (castResult != SPELL_CAST_OK)
            return castResult;
    }

    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_CASTER_AURAS))
    {
        castResult = CheckCasterAuras();
        if (castResult != SPELL_CAST_OK)
            return castResult;
    }

    if(!IsTriggered())
    {
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

    // script hook
    castResult = CallScriptCheckCastHandlers();
    if (castResult != SPELL_CAST_OK)
        return castResult;
        
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
                    Unit* target2 = m_targets.GetUnitTarget();
                    if (!target2 || target2->GetEntry() != 24972 || target2->IsAlive())
                        return SPELL_FAILED_BAD_TARGETS;
                }
                else if (m_spellInfo->Id == 35771) { // Tag Subbued Talbuk
                    Unit* target2 = m_targets.GetUnitTarget();
                    if (!target2 || !target2->ToCreature() || !target2->ToCreature()->IsBelowHPPercent(20))
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
                    if(Unit* target2 = m_targets.GetUnitTarget())
                        if (target2 != m_caster && target2->GetPowerType() != Powers(m_spellInfo->Effects[i].MiscValue))
                            return SPELL_FAILED_BAD_TARGETS;
                break;
            }
            case SPELL_EFFECT_CHARGE:
            {
#ifdef LICH_KING
                if (m_spellInfo->SpellFamilyName == SPELLFAMILY_WARRIOR)
                {
                    // Warbringer - can't be handled in proc system - should be done before checkcast root check and charge effect process
                    if (strict && m_caster->IsScriptOverriden(m_spellInfo, 6953))
                        m_caster->RemoveMovementImpairingAuras(true);
                }
#endif
                if (m_caster->HasUnitState(UNIT_STATE_ROOT))
                    return SPELL_FAILED_ROOTED;

                Unit* _target = m_targets.GetUnitTarget();
                if (!_target)
                    return SPELL_FAILED_DONT_REPORT;

                Position pos = _target->GetFirstWalkableCollisionPosition(_target->GetCombatReach(), _target->GetRelativeAngle(m_caster));
                delete m_preGeneratedPath; //just in case, if logic changes elsewhere
                m_preGeneratedPath = new PathGenerator(m_caster);
                m_preGeneratedPath->SetPathLengthLimit(m_spellInfo->GetMaxRange(false, m_caster) *1.4f);
                bool result = m_preGeneratedPath->CalculatePath(pos.m_positionX, pos.m_positionY, pos.m_positionZ + _target->GetCombatReach(), false, false);

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
                    const Position newLastPoint = _target->GetFirstWalkableCollisionPosition(CONTACT_DISTANCE*3, _target->GetRelativeAngle(&beforeLastPointP));
                    //Recreate a path to this point
                    result = m_preGeneratedPath->CalculatePath(newLastPoint.m_positionX, newLastPoint.m_positionY, newLastPoint.m_positionZ + _target->GetCombatReach(), false, false);
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

#ifndef LICH_KING
                // chance for fail at orange skinning attempt
                if( (m_selfContainer && (*m_selfContainer) == this) &&
                    skillValue < sWorld->GetConfigMaxSkillValue() &&
                    (ReqValue < 0 ? 0 : ReqValue) > m_caster->GetMap()->irand(skillValue-25, skillValue+37) )
                    return SPELL_FAILED_TRY_AGAIN;
#endif
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
                LockEntry const *lockInfo = nullptr;
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
            case SPELL_EFFECT_RESURRECT_PET:
            {
                Creature* pet = m_caster->GetGuardianPet();
                if(!pet)
                    return SPELL_FAILED_NO_PET;

                if(pet->IsAlive())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                break;
            }
            // This is generic summon effect now and don't make this check for summon types similar
            // SPELL_EFFECT_SUMMON_CRITTER, SPELL_EFFECT_SUMMON_WILD or SPELL_EFFECT_SUMMON_GUARDIAN.
            // These won't show up in m_caster->GetMinionGUID()
            case SPELL_EFFECT_SUMMON:
            {
             SummonPropertiesEntry const* SummonProperties = sSummonPropertiesStore.LookupEntry(m_spellInfo->Effects[i].MiscValueB);
                if (!SummonProperties)
                    break;
                switch (SummonProperties->Category)
                {
                    case SUMMON_CATEGORY_PET:
                        if (!m_spellInfo->HasAttribute(SPELL_ATTR1_DISMISS_PET) && m_caster->GetPetGUID())
                            return SPELL_FAILED_ALREADY_HAVE_SUMMON;
                    // intentional missing break, check both GetPetGUID() and GetCharmGUID for SUMMON_CATEGORY_PET
                    case SUMMON_CATEGORY_PUPPET:
                        if (m_caster->GetCharmGUID())
                            return SPELL_FAILED_ALREADY_HAVE_CHARM;
                        break;
                    case SUMMON_CATEGORY_ALLY:
                        //hack for?
                        if (m_spellInfo->Id == 13166 && m_caster && m_caster->GetMapId() == 580)
                            return SPELL_FAILED_TRY_AGAIN;
                        break;
                }
                break;
            }
            // Don't make this check for SPELL_EFFECT_SUMMON_CRITTER, SPELL_EFFECT_SUMMON_WILD or SPELL_EFFECT_SUMMON_GUARDIAN.
            // These won't show up in m_caster->GetMinionGUID()
            case SPELL_EFFECT_SUMMON_POSSESSED:
            case SPELL_EFFECT_SUMMON_PHANTASM:
            case SPELL_EFFECT_SUMMON_DEMON:
            {
                if(!m_spellInfo->HasAttribute(SPELL_ATTR1_DISMISS_PET) && m_caster->GetPetGUID())
                    return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                if(m_caster->GetCharmGUID())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                break;
            }
            case SPELL_EFFECT_SUMMON_PET:
            {
                if(m_caster->GetMinionGUID())                  //let warlock do a replacement summon
                {

                    Pet* pet = m_caster->GetPet();

                    if (m_caster->GetTypeId()==TYPEID_PLAYER && m_caster->GetClass()==CLASS_WARLOCK)
                    {
                        if (strict)                         //starting cast, trigger pet stun (cast by pet so it doesn't attack player)
                        {
                            CastSpellExtraArgs args;
                            args.TriggerFlags = TRIGGERED_FULL_MASK;
                            args.SetOriginalCaster(pet->GetGUID());
                            pet->CastSpell(pet, 32752, args);
                        }
                    }
                    else if (!m_spellInfo->HasAttribute(SPELL_ATTR1_DISMISS_PET))
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

                Player* target_ = ObjectAccessor::FindPlayer((m_caster->ToPlayer())->GetTarget());
                if( !target_ || (m_caster->ToPlayer())== target_ || !target_->IsInSameRaidWith(m_caster->ToPlayer()) )
                    return SPELL_FAILED_BAD_TARGETS;

#ifdef LICH_KING
                // summon pending error
                if (target_->GetSummonExpireTimer() > time(NULL))
                    return SPELL_FAILED_SUMMON_PENDING;
#endif

                // check if our map is dungeon
                MapEntry const* map = sMapStore.LookupEntry(m_caster->GetMapId());
                if(map->IsDungeon())
                {
                    uint32 mapId = m_caster->GetMap()->GetId();
                    Difficulty difficulty = m_caster->GetMap()->GetDifficulty();
                    if (map->IsRaid())
                        if (InstancePlayerBind* targetBind = target_->GetBoundInstance(mapId, difficulty))
                            if (InstancePlayerBind* casterBind = m_caster->ToPlayer()->GetBoundInstance(mapId, difficulty))
                                if (targetBind->perm && targetBind->save != casterBind->save)
                                    return SPELL_FAILED_TARGET_LOCKED_TO_RAID_INSTANCE;

                    InstanceTemplate const* instance = sObjectMgr->GetInstanceTemplate(m_caster->GetMapId());
                    if(!instance)
                        return SPELL_FAILED_TARGET_NOT_IN_INSTANCE;
                    if(!target_->Satisfy(sObjectMgr->GetAccessRequirement(instance->access_id), m_caster->GetMapId()))
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
                if (m_targets.GetUnitTarget() == m_caster)
                    return SPELL_FAILED_BAD_TARGETS;

                /* TC
                uint32 dispelMask = SpellInfo::GetDispelMask(DispelType(m_spellInfo->Effects[i].MiscValue));
                bool hasStealableAura = false;
                Unit::VisibleAuraMap const* visibleAuras = m_targets.GetUnitTarget()->GetVisibleAuras();
                for (Unit::VisibleAuraMap::const_iterator itr = visibleAuras->begin(); itr != visibleAuras->end(); ++itr)
                {
                    if (!itr->second->IsPositive())
                        continue;

                    Aura const* aura = itr->second->GetBase();
                    if (!(aura->GetSpellInfo()->GetDispelMask() & dispelMask))
                        continue;

                    if (aura->IsPassive() || aura->GetSpellInfo()->HasAttribute(SPELL_ATTR4_NOT_STEALABLE))
                        continue;

                    hasStealableAura = true;
                    break;
                }

                if (!hasStealableAura)
                    return SPELL_FAILED_NOTHING_TO_STEAL;

                */

                break;
            }
#ifdef LICH_KING
            case SPELL_EFFECT_LEAP_BACK:
            {
                if (m_caster->HasUnitState(UNIT_STATE_ROOT))
                {
                    if (m_caster->GetTypeId() == TYPEID_PLAYER)
                        return SPELL_FAILED_ROOTED;
                    else
                        return SPELL_FAILED_DONT_REPORT;
                }
                break;
            }
            case SPELL_EFFECT_JUMP:
            case SPELL_EFFECT_JUMP_DEST:
            {
                if (m_caster->HasUnitState(UNIT_STATE_ROOT))
                    return SPELL_FAILED_ROOTED;
                break;
            }
            case SPELL_EFFECT_TALENT_SPEC_SELECT:
                // can't change during already started arena/battleground
                if (m_caster->GetTypeId() == TYPEID_PLAYER)
                    if (Battleground const* bg = m_caster->ToPlayer()->GetBattleground())
                        if (bg->GetStatus() == STATUS_IN_PROGRESS)
                            return SPELL_FAILED_NOT_IN_BATTLEGROUND;
                break;
#endif
            case SPELL_EFFECT_ENCHANT_ITEM:
            {
                
                if (m_spellInfo->Id == 30260 && m_targets.GetItemTarget() && m_targets.GetItemTarget()->GetTemplate()->ItemLevel < 60)
                    return SPELL_FAILED_LOWLEVEL;
                    
                break;
            }
            case SPELL_EFFECT_TRANS_DOOR:
            {
                if (m_spellInfo->Id == 39161 && m_caster) {
                    Creature* gorgrom = m_caster->FindNearestCreature(21514, 10.0f, false);
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
            case SPELL_AURA_MOD_POSSESS_PET:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER)
                    return SPELL_FAILED_NO_PET;

                Pet* pet = m_caster->ToPlayer()->GetPet();
                if (!pet)
                    return SPELL_FAILED_NO_PET;

                if (pet->GetCharmerGUID())
                    return SPELL_FAILED_CHARMED;
                break;
            }
            case SPELL_AURA_DUMMY:
            {
                if(m_spellInfo->Id == 1515)
                {
                    if (!m_targets.GetUnitTarget() || m_targets.GetUnitTarget()->GetTypeId() == TYPEID_PLAYER)
                        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                    if (m_targets.GetUnitTarget()->GetLevel() > m_caster->GetLevel())
                        return SPELL_FAILED_HIGHLEVEL;

                    // use SMSG_PET_TAME_FAILURE?
                    if (!(m_targets.GetUnitTarget()->ToCreature())->GetCreatureTemplate()->IsTameable ())
                        return SPELL_FAILED_BAD_TARGETS;

                    if(m_caster->GetMinionGUID())
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                    if(m_caster->GetCharmGUID())
                        return SPELL_FAILED_ALREADY_HAVE_CHARM;
                }
            }break;
            case SPELL_AURA_MOD_POSSESS:
            case SPELL_AURA_MOD_CHARM:
            case SPELL_AURA_AOE_CHARM:
            {
                if(m_caster->GetCharmGUID())
                    return SPELL_FAILED_ALREADY_HAVE_CHARM;

                if(m_caster->GetCharmerGUID())
                    return SPELL_FAILED_CHARMED;

                if (m_spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOD_CHARM
                    || m_spellInfo->Effects[i].ApplyAuraName == SPELL_AURA_MOD_POSSESS)
                {
                    if (!m_spellInfo->HasAttribute(SPELL_ATTR1_DISMISS_PET) && m_caster->GetPetGUID())
                        return SPELL_FAILED_ALREADY_HAVE_SUMMON;

                    if (m_caster->GetCharmGUID())
                        return SPELL_FAILED_ALREADY_HAVE_CHARM;
                }

                // hack SelectSpellTargets is call after this so...
                if (m_spellInfo->Id == 34630) {
                    if (Creature* target_ = m_caster->FindNearestCreature(19849, 15.0f, true))
                        m_targets.SetUnitTarget(target_);
                }
                else if (m_spellInfo->Id == 45839)
                {
                    if (Creature* target_ = m_caster->FindNearestCreature(25653, 100.0f, true))
                        m_targets.SetUnitTarget(target_);
                }

                if (Unit* target_ = m_targets.GetUnitTarget())
                {
#ifdef LICH_KING
                    if (target_->GetTypeId() == TYPEID_UNIT && target_->ToCreature()->IsVehicle())
                        return SPELL_FAILED_BAD_IMPLICIT_TARGETS;
#endif
                    if (target_->IsMounted())
                        return SPELL_FAILED_CANT_BE_CHARMED;

                    if (target_->GetCharmerGUID())
                        return SPELL_FAILED_CHARMED;

                    if (target_->GetOwnerGUID() && target_->GetOwnerGUID().IsPlayer())
                        return SPELL_FAILED_TARGET_IS_PLAYER_CONTROLLED;

                    if (target_->IsPet() && (!target->GetOwner() || target_->GetOwner()->ToPlayer()))
                        return SPELL_FAILED_CANT_BE_CHARMED;

                    int32 localDamage = CalculateDamage(i, target_);
                    if (localDamage && int32(target_->GetLevel()) > localDamage)
                        return SPELL_FAILED_HIGHLEVEL;
                }

                break;
            }
            case SPELL_AURA_MOUNTED:
            {
                if (m_caster->IsInWater()
#ifdef LICH_KING
                    //BC remove mount in water, while LK only disallow flying mounts
                    && m_spellInfo->HasAura(SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED)
#endif
                    )
                    return SPELL_FAILED_ONLY_ABOVEWATER;

                if (m_caster->GetTypeId() == TYPEID_PLAYER && (m_caster->ToPlayer())->GetTransport())
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;

                // Ignore map check if spell have AreaId. AreaId already checked and this prevent special mount spells
                bool allowMount = !m_caster->GetMap()->IsDungeon() || m_caster->GetMap()->IsBattlegroundOrArena();
                MapEntry const* mapEntry = sMapStore.LookupEntry(m_caster->GetMapId());
                if (mapEntry)
                    allowMount = mapEntry->IsMountAllowed();

                if (m_caster->GetTypeId() == TYPEID_PLAYER && !allowMount && !m_spellInfo->AreaId)
                    return SPELL_FAILED_NO_MOUNTS_ALLOWED;

                if (m_caster->GetAreaId() == 35) //hack
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
                if (m_spellInfo->Effects[i].IsTargetingArea())
                    break;

                if (!m_targets.GetUnitTarget())
                    return SPELL_FAILED_BAD_IMPLICIT_TARGETS;

                if (m_caster->GetTypeId()!=TYPEID_PLAYER || m_CastItem)
                    break;

                if(m_targets.GetUnitTarget()->GetPowerType() != POWER_MANA)
                    return SPELL_FAILED_BAD_TARGETS;

                break;
            }
            case SPELL_AURA_WATER_WALK:
            { 
                if (m_spellInfo->Effects[i].TargetA.GetTarget() == TARGET_UNIT_CASTER)
                    break;

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
        if (!(m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS)) 
            && !m_spellInfo->HasAttribute(SPELL_ATTR5_SKIP_CHECKCAST_LOS_CHECK))
        {
            float x, y, z;
            m_targets.GetDstPos()->GetPosition(x, y, z);

            if (!m_caster->IsWithinLOS(x,y,z))
                return SPELL_FAILED_LINE_OF_SIGHT;
        }


    // check pet presence
    for (const auto & Effect : m_spellInfo->Effects)
    {
        if (Effect.TargetA.GetTarget() == TARGET_UNIT_PET)
        {
            if (!m_caster->GetGuardianPet() && !m_caster->GetCharm())
            {
                if (m_triggeredByAuraSpell)              // not report pet not existence for triggered spells
                    return SPELL_FAILED_DONT_REPORT;
                else
                    return SPELL_FAILED_NO_PET;
            }
            break;
        }

    }

    // check if caster has at least 1 combo point on target for spells that require combo points
    if (m_needComboPoints && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
        if (m_spellInfo->NeedsExplicitUnitTarget())
        {
            if (!m_caster->ToPlayer()->GetComboPoints(m_targets.GetUnitTarget()))
                return SPELL_FAILED_NO_COMBO_POINTS;
        }
        else
        {
            if (!m_caster->ToPlayer()->GetComboPoints())
                return SPELL_FAILED_NO_COMBO_POINTS;
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
    if(m_caster->IsNonMeleeSpellCast(false) && !(_triggeredCastFlags & TRIGGERED_IGNORE_CAST_IN_PROGRESS) && !m_spellInfo->HasAttribute(SPELL_ATTR4_CAN_CAST_WHILE_CASTING))  //prevent spellcast interruption by another spellcast
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
        for(const auto & Effect : m_spellInfo->Effects)
        {
            if(Effect.ApplyAuraName == SPELL_AURA_SCHOOL_IMMUNITY)
                school_immune |= uint32(Effect.MiscValue);
            else if(Effect.ApplyAuraName == SPELL_AURA_MECHANIC_IMMUNITY)
                mechanic_immune |= 1 << uint32(Effect.MiscValue);
            else if(Effect.ApplyAuraName == SPELL_AURA_DISPEL_IMMUNITY)
                dispel_immune |= GetDispellMask(DispelType(Effect.MiscValue));
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
            for(const auto & aura : auras)
            {
                if(aura.second)
                {
                    if( GetSpellMechanicMask(aura.second->GetSpellInfo(), aura.second->GetEffIndex()) & mechanic_immune )
                        continue;
                    if( aura.second->GetSpellInfo()->GetSchoolMask() & school_immune )
                        continue;
                    if( (1<<(aura.second->GetSpellInfo()->Dispel)) & dispel_immune)
                        continue;

                    //Make a second check for spell failed so the right SPELL_FAILED message is returned.
                    //That is needed when your casting is prevented by multiple states and you are only immune to some of them.
                    switch(aura.second->GetModifier()->m_auraname)
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
    if (!target)
        return (PetCanCast(target) == SPELL_CAST_OK);
    
    ObjectGuid targetguid = target->GetGUID();

    for(uint8 j = 0; j < MAX_SPELL_EFFECTS; j++)
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
        for(auto & ihit : m_UniqueTargetInfo)
            if( ihit.targetGUID == targetguid )
                return true;
    }
    return false;                                           //target invalid
}

//check range and facing
SpellCastResult Spell::CheckRange(bool strict)
{
    Unit *target = m_targets.GetUnitTarget();

    float minRange, maxRange;
    std::tie(minRange, maxRange) = GetMinMaxRange(strict);

    // dont check max_range too strictly after cast
    if (m_spellInfo->RangeEntry && m_spellInfo->RangeEntry->type != SPELL_RANGE_MELEE && !strict)
        maxRange += std::min(MAX_SPELL_RANGE_TOLERANCE, maxRange*0.1f); // 10% but no more than MAX_SPELL_RANGE_TOLERANCE

    // get square values for sqr distance checks
    minRange *= minRange;
    maxRange *= maxRange;

    if (target && target != m_caster)
    {
        if (m_caster->GetExactDistSq(target) > maxRange)
            return SPELL_FAILED_OUT_OF_RANGE;

        if (minRange > 0.0f && m_caster->GetExactDistSq(target) < minRange)
            return SPELL_FAILED_OUT_OF_RANGE;

        if (m_caster->GetTypeId() == TYPEID_PLAYER &&
            (m_spellInfo->FacingCasterFlags & SPELL_FACING_FLAG_INFRONT) && !m_caster->HasInArc(static_cast<float>(M_PI), target))
            return SPELL_FAILED_UNIT_NOT_INFRONT;
    }

    if (m_targets.HasDst() && !m_targets.HasTraj())
    {
        if (m_caster->GetExactDistSq(m_targets.GetDstPos()) > maxRange)
            return SPELL_FAILED_OUT_OF_RANGE;
        if (minRange > 0.0f && m_caster->GetExactDistSq(m_targets.GetDstPos()) < minRange)
            return SPELL_FAILED_OUT_OF_RANGE;
    }

    return SPELL_CAST_OK;
}

std::pair<float, float> Spell::GetMinMaxRange(bool strict)
{
    float rangeMod = 0.0f;
    float minRange = 0.0f;
    float maxRange = 0.0f;
    if (strict && IsNextMeleeSwingSpell())
    {
        maxRange = 100.0f;
        return std::pair<float, float>(minRange, maxRange);
    }

    if (m_spellInfo->RangeEntry)
    {
        Unit* target = m_targets.GetUnitTarget();
        if (m_spellInfo->RangeEntry->type & SPELL_RANGE_MELEE)
        {
            rangeMod = m_caster->GetMeleeRange(target ? target : m_caster); // when the target is not a unit, take the caster's combat reach as the target's combat reach.
        }
        else
        {
            float meleeRange = 0.0f;
            if (m_spellInfo->RangeEntry->type & SPELL_RANGE_RANGED)
                meleeRange = m_caster->GetMeleeRange(target ? target : m_caster); // when the target is not a unit, take the caster's combat reach as the target's combat reach.

            minRange = m_caster->GetSpellMinRangeForTarget(target, m_spellInfo) + meleeRange;
            maxRange = m_caster->GetSpellMaxRangeForTarget(target, m_spellInfo);

            if (target || m_targets.GetCorpseTarget())
            {
                rangeMod = m_caster->GetCombatReach() + (target ? target->GetCombatReach() : m_caster->GetCombatReach());


                if (minRange > 0.0f && !(m_spellInfo->RangeEntry->type & SPELL_RANGE_RANGED))
                    minRange += rangeMod;
            }
        }

        if (target && m_caster->isMoving() && target->isMoving() && !m_caster->IsWalking() && !target->IsWalking() &&
            (m_spellInfo->RangeEntry->type & SPELL_RANGE_MELEE || target->GetTypeId() == TYPEID_PLAYER))
            rangeMod += 5.0f / 3.0f;
    }

    if (m_spellInfo->IsRangedWeaponSpell() && m_caster->GetTypeId() == TYPEID_PLAYER)
        if (Item* ranged = m_caster->ToPlayer()->GetWeaponForAttack(RANGED_ATTACK, true))
            maxRange *= ranged->GetTemplate()->RangedModRange * 0.01f;

    if (Player* modOwner = m_caster->GetSpellModOwner())
        modOwner->ApplySpellMod(m_spellInfo->Id, SPELLMOD_RANGE, maxRange, this);

    maxRange += rangeMod;

    return std::pair<float, float>(minRange, maxRange);
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
        TC_LOG_ERROR("spells","Spell::CalculateManaCost: Unknown power type '%d' in spell %d", m_spellInfo->PowerType, m_spellInfo->Id);
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

    if (!m_caster->IsControlledByPlayer())
    {
        if (m_spellInfo->HasAttribute(SPELL_ATTR0_LEVEL_DAMAGE_CALCULATION))
        {
            GtNPCManaCostScalerEntry const* spellScaler = sGtNPCManaCostScalerStore.LookupEntry(m_spellInfo->SpellLevel - 1);
            GtNPCManaCostScalerEntry const* casterScaler = sGtNPCManaCostScalerStore.LookupEntry(m_caster->GetLevel() - 1);
            if (spellScaler && casterScaler)
                powerCost *= casterScaler->ratio / spellScaler->ratio;
        }
        /* OLD CALC
        if (m_spellInfo->Attributes & SPELL_ATTR0_LEVEL_DAMAGE_CALCULATION)
            powerCost = int32(powerCost / (1.117f* m_spellInfo->SpellLevel / m_caster->GetLevel() - 0.1327f));
        */
    }

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
                for (const auto & Effect : m_spellInfo->Effects)
                {
                    // skip check, pet not required like checks, and for TARGET_UNIT_PET m_targets.GetUnitTarget() is not the real target but the caster
                    if (Effect.TargetA.GetTarget()== TARGET_UNIT_PET)
                        continue;

                    if (Effect.Effect == SPELL_EFFECT_HEAL)
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
                    if (Effect.Effect == SPELL_EFFECT_ENERGIZE)
                    {
                        if(Effect.MiscValue < 0 || Effect.MiscValue >= MAX_POWERS)
                        {
                            failReason = SPELL_FAILED_ALREADY_AT_FULL_POWER;
                            continue;
                        }

                        Powers power = Powers(Effect.MiscValue);
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
                m_caster->ToPlayer()->GetSession()->SendNotification("This object is not an authorized target");
                return SPELL_FAILED_DONT_REPORT;
            }
            else
                return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
        }
    }
    // if not item target then required item must be equipped
    else
    {
        if (!(_triggeredCastFlags & TRIGGERED_IGNORE_EQUIPPED_ITEM_REQUIREMENT))
            if(m_caster->GetTypeId() == TYPEID_PLAYER && !(m_caster->ToPlayer())->HasItemFitToSpellRequirements(m_spellInfo))
                return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
    }

    if(m_spellInfo->RequiresSpellFocus)
    {
        CellCoord p(Trinity::ComputeCellCoord(m_caster->GetPositionX(), m_caster->GetPositionY()));
        Cell cell(p);

        GameObject* ok = nullptr;
        Trinity::GameObjectFocusCheck go_check(m_caster, m_spellInfo->RequiresSpellFocus);
        Trinity::GameObjectSearcher<Trinity::GameObjectFocusCheck> checker(m_caster, ok, go_check);

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
    for(uint32 i : m_spellInfo->Totem)
    {
        if(i != 0)
        {
            if( p_caster->HasItemCount(i,1) )
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
    for(uint32 i : m_spellInfo->TotemCategory)
    {
        if(i != 0)
        {
            if( p_caster->HasItemTotemCategory(i) )
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

    // special checks for spell effects
    for(const auto & Effect : m_spellInfo->Effects)
    {
        switch (Effect.Effect)
        {
            case SPELL_EFFECT_CREATE_ITEM:
            {
                Unit* target = m_targets.GetUnitTarget() ? m_targets.GetUnitTarget() : m_caster;
                if (target && target->GetTypeId() == TYPEID_PLAYER && !IsTriggered() && Effect.ItemType)
                {
                    ItemPosCountVec dest;
                    uint8 msg = target->ToPlayer()->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, Effect.ItemType, 1 );
                    if (msg != EQUIP_ERR_OK )
                    {
                        target->ToPlayer()->SendEquipError( msg, nullptr, nullptr );
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
                    uint32 enchant_id = Effect.MiscValue;
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
                    uint32 enchant_id = Effect.MiscValue;
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
    
    // check weapon presence in slots for main/offhand weapons
    if (!(_triggeredCastFlags & TRIGGERED_IGNORE_EQUIPPED_ITEM_REQUIREMENT) && m_spellInfo->EquippedItemClass >= 0)
    {
        auto weaponCheck = [this](WeaponAttackType attackType) -> SpellCastResult
        {
            Item const* item = m_caster->ToPlayer()->GetWeaponForAttack(attackType);

            // skip spell if no weapon in slot or broken
            if (!item || item->IsBroken())
                return SPELL_FAILED_EQUIPPED_ITEM_CLASS;

            // skip spell if weapon not fit to triggered spell
            if (!item->IsFitToSpellRequirements(m_spellInfo))
                return SPELL_FAILED_EQUIPPED_ITEM_CLASS;

            return SPELL_CAST_OK;
        };

        if (m_spellInfo->HasAttribute(SPELL_ATTR3_MAIN_HAND))
        {
            SpellCastResult mainHandResult = weaponCheck(BASE_ATTACK);
            if (mainHandResult != SPELL_CAST_OK)
                return mainHandResult;
        }

        if (m_spellInfo->HasAttribute(SPELL_ATTR3_REQ_OFFHAND))
        {
            SpellCastResult offHandResult = weaponCheck(OFF_ATTACK);
            if (offHandResult != SPELL_CAST_OK)
                return offHandResult;
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

    for(auto & ihit : m_UniqueTargetInfo)
    {
        if (ihit.missCondition == SPELL_MISS_NONE)
        {
            Unit* unit = m_caster->GetGUID()==ihit.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, ihit.targetGUID);
            if (unit)
            {
                for (int j=0;j<3;j++)
                    if( ihit.effectMask & (1<<j) )
                        unit->DelayAura(m_spellInfo->Id, j, delaytime);
            }

        }
    }

    for(uint8 j = 0; j < MAX_SPELL_EFFECTS; j++)
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
        if(m_originalCaster && !m_originalCaster->IsInWorld()) m_originalCaster = nullptr;
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
    return m_spellInfo->IsAffectedBySpell(spellInfo->Id, effectId, spellInfo->Effects[effectId].ItemType);
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

        if (int32 _damage = m_caster->CalculateSpellDamage(m_spellInfo, eff, m_spellInfo->Effects[eff].BasePoints, target))
            if ((int32)target->GetLevel() > _damage)
                return false;
        break;
    default:
        break;
    }

    // sunwell: skip los checking if spell has appropriate attribute, or target requires specific entry
    // this is only for target addition and target has to have unselectable flag, this is valid for FLAG_EXTRA_TRIGGER and quest triggers however there are some without this flag, used not_selectable
    if (m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS) || (target->GetTypeId() == TYPEID_UNIT && target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE) && (m_spellInfo->Effects[eff].TargetA.GetCheckType() == TARGET_CHECK_ENTRY || m_spellInfo->Effects[eff].TargetB.GetCheckType() == TARGET_CHECK_ENTRY)))
        return true;

    if (IsTriggered())
    {
        if (!m_caster->IsInMap(target)) // sunwell: crashfix, avoid IsWithinLOS on another map! >_>
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
        if ((!casterIsTotem || !IsPositive()) && !target->IsWithinLOS(x, y, z))
            return false;

        return true;
    }

    // todo: shit below shouldn't be here, but it's temporary
    //Check targets for LOS visibility (except spells without range limitations)
    switch (m_spellInfo->Effects[eff].Effect)
    {
    case SPELL_EFFECT_RESURRECT_NEW:
        // player far away, maybe his corpse near?
        if (target != m_caster && !target->IsWithinLOSInMap(m_caster, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
        {
            if (!m_targets.GetCorpseTargetGUID())
                return false;

            Corpse* corpse = ObjectAccessor::GetCorpse(*m_caster, m_targets.GetCorpseTargetGUID());
            if (!corpse)
                return false;

            if (target->GetGUID() != corpse->GetOwnerGUID())
                return false;

            if (!corpse->IsWithinLOSInMap(m_caster, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
                return false;
        }
        break;
    case SPELL_EFFECT_SKIN_PLAYER_CORPSE:
    {
        if (!m_targets.GetCorpseTargetGUID())
        {
            if (target->IsWithinLOSInMap(m_caster, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2) && target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
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

        if (!corpse->IsWithinLOSInMap(m_caster, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
            return false;
    }
    break;
    /*case SPELL_EFFECT_CHARGE:
    break;*/
    // else no break intended
    default:                                            // normal case
                                                        // Get GO cast coordinates if original caster -> GO
        WorldObject* caster = nullptr;
        if (m_originalCasterGUID.IsGameObject())
            caster = m_caster->GetMap()->GetGameObject(m_originalCasterGUID);
        if (!caster)
            caster = m_caster;
        if (target != caster)
        {
            if (m_targets.HasDst())
            {
                if (!target->IsWithinLOS(m_targets.GetDstPos()->GetPositionX(), m_targets.GetDstPos()->GetPositionY(), m_targets.GetDstPos()->GetPositionZ()))
                    return false;
            }
            else 
            {
                if (target != m_caster && !target->IsWithinLOSInMap(caster))
                    return false;
            }
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
        if(!(target->ToPlayer())->IsVisible())
            return false;

        if((target->ToPlayer())->IsGameMaster() && !IsPositive())
            return false;
    }

    // check for ignore LOS on the effect itself
    if (m_spellInfo->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS) /*|| DisableMgr::IsDisabledFor(DISABLE_TYPE_SPELL, m_spellInfo->Id, nullptr, SPELL_DISABLE_LOS)*/)
        return true;

    // if spell is triggered, need to check for LOS disable on the aura triggering it and inherit that behaviour
    if (IsTriggered() && m_triggeredByAuraSpell && (m_triggeredByAuraSpell->HasAttribute(SPELL_ATTR2_CAN_TARGET_NOT_IN_LOS) /*|| DisableMgr::IsDisabledFor(DISABLE_TYPE_SPELL, m_triggeredByAuraSpell->Id, nullptr, SPELL_DISABLE_LOS)*/))
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
            if(target!=m_caster && !target->IsWithinLOSInMap(m_caster, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
            {
                if(!m_targets.GetCorpseTargetGUID())
                    return false;

                Corpse *corpse = ObjectAccessor::GetCorpse(*m_caster,m_targets.GetCorpseTargetGUID());
                if(!corpse)
                    return false;

                if(target->GetGUID()!=corpse->GetOwnerGUID())
                    return false;

                if(!corpse->IsWithinLOSInMap(m_caster, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
                    return false;
            }

            // all ok by some way or another, skip normal check
            break;
        default:                                            // normal case
            if(target!=m_caster && !target->IsWithinLOSInMap(m_caster, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
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
        for(auto magnetAura : magnetAuras)
        {
            if(Unit* magnet = magnetAura->GetCaster())
            {
                if(magnetAura->m_procCharges>0)
                {
                    magnetAura->SetCharges(magnetAura->m_procCharges-1);
                    target = magnet;
                    m_targets.SetUnitTarget(target);
                    AddUnitTarget(target, 0);
                    ObjectGuid targetGUID = target->GetGUID();
                    for(auto & ihit : m_UniqueTargetInfo)
                    {
                        if (targetGUID == ihit.targetGUID)                 // Found in list
                        {
                            ihit.damage = target->GetHealth();
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
        for(auto hitTriggerAura : hitTriggerAuras)
        {
            if(Unit* hitTarget = hitTriggerAura->GetCaster())
            {
                if(hitTriggerAura->m_procCharges>0)
                {
                    hitTriggerAura->SetCharges(hitTriggerAura->m_procCharges-1);
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
        m_spellInfo->Speed > 0.0f || (!m_triggeredByAuraSpell && !IsTriggered());
}

bool Spell::HaveTargetsForEffect( uint8 effect ) const
{
    for(const auto & itr : m_UniqueTargetInfo)
        if(itr.effectMask & (1<<effect))
            return true;

    for(const auto & itr : m_UniqueGOTargetInfo)
        if(itr.effectMask & (1<<effect))
            return true;

    for(auto itr : m_UniqueItemInfo)
        if(itr.effectMask & (1<<effect))
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
            (m_Spell->GetCaster()->GetTypeId()==TYPEID_PLAYER?"Player":"Creature"), m_Spell->GetCaster()->GetGUID().GetCounter(), m_Spell->m_spellInfo->Id);
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
    WorldObject* target = ObjectAccessor::GetWorldObject(*_caster, _targetGUID);
    if (!target)
        return true;

    Unit* unit_target = target->ToUnit();
    if (!unit_target)
        return true;

    // Start triggers for remove charges if need (trigger only for victim, and mark as active spell)
    _caster->ProcDamageAndSpell(unit_target, PROC_FLAG_NONE, PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT, PROC_EX_REFLECT, 1, BASE_ATTACK, _spellInfo);
    // FIXME: Add a flag on unit itself, not to setRemoveReflect if unit is already flagged for it (prevent infinite delay on reflect lolz)
    if (Spell* sp = _caster->m_currentSpells[CURRENT_CHANNELED_SPELL])
        sp->setRemoveReflect();
    else if (Spell* sp_ = _caster->m_currentSpells[CURRENT_GENERIC_SPELL])
        sp_->setRemoveReflect();

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

        HandleEffects(nullptr, nullptr, nullptr, i, SPELL_EFFECT_HANDLE_LAUNCH);
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

    PrepareTargetProcessing();

    bool firstTarget = true;
    for (auto & target : m_UniqueTargetInfo)
    {
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

    FinishTargetProcessing();
}

void Spell::DoAllEffectOnLaunchTarget(TargetInfo& targetInfo, float* multiplier, bool firstTarget)
{
    Unit* unit = nullptr;
    // In case spell hit target, do all effect on that target
    if (targetInfo.missCondition == SPELL_MISS_NONE)
        unit = m_caster->GetGUID() == targetInfo.targetGUID ? m_caster : ObjectAccessor::GetUnit(*m_caster, targetInfo.targetGUID);
    // In case spell reflect from target, do all effect on caster (if hit)
    else if (targetInfo.missCondition == SPELL_MISS_REFLECT && targetInfo.reflectResult == SPELL_MISS_NONE)
        unit = m_caster;
    if (!unit)
        return;

    // This will only cause combat - the target will engage once the projectile hits (in DoAllEffectOnTarget)
    if (targetInfo.missCondition != SPELL_MISS_EVADE && !m_caster->IsFriendlyTo(unit) && (!m_spellInfo->IsPositive() || m_spellInfo->HasEffect(SPELL_EFFECT_DISPEL)) && (m_spellInfo->HasInitialAggro() || unit->IsEngaged()))
        m_caster->SetInCombatWith(unit);

    uint8 ssEffect = MAX_SPELL_EFFECTS;
    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (targetInfo.effectMask & (1 << i))
        {
            m_damage = 0;
            m_healing = 0;

            HandleEffects(unit, nullptr, nullptr, i, SPELL_EFFECT_HANDLE_LAUNCH_TARGET);

            if (m_damage > 0)
            {
                // sunwell: Area Auras, AoE Targetting spells AND Chain Target spells (cleave etc.)
                if (m_spellInfo->Effects[i].IsAreaAuraEffect() || m_spellInfo->Effects[i].IsTargetingArea() || (m_spellInfo->Effects[i].ChainTarget > 1 && m_spellInfo->DmgClass != SPELL_DAMAGE_CLASS_MAGIC))
                {
                    m_damage = unit->CalculateAOEDamageReduction(m_damage, m_spellInfo->SchoolMask, m_caster);

                    // Handle area damage cap
                    //gamepedia: "Since patch 2.2.0, most area effect damage effects have an 'area damage cap' which limits the potency of the spell if used against a large number of targets simultaneously"
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

    // sunwell: totem's inherit owner crit chance and dancing rune weapon
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

            CastSpellExtraArgs args;
            args.TriggerFlags = TRIGGERED_FULL_MASK;
            args.AddSpellBP0(int32(basepoints));
            m_caster->CastSpell(target, 26654, args);

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
        /*case SPELLVALUE_AURA_STACK:
            m_spellValue->AuraStackAmount = uint8(value);
            break;*/
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
    if (m_caster->GetCurrentSpell(CURRENT_GENERIC_SPELL) != this)
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
        SpellTargetCheckTypes selectionType, ConditionContainer* condList) : _caster(caster), _referer(referer), _spellInfo(spellInfo),
        _targetSelectionType(selectionType), _condList(condList)
    {
        if (condList)
            _condSrcInfo = new ConditionSourceInfo(nullptr, caster);
        else
            _condSrcInfo = nullptr;
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
                if (!_caster->_IsValidAttackTarget(unitTarget, _spellInfo))
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
        SpellTargetCheckTypes selectionType, ConditionContainer* condList)
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
        Unit* referer, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionContainer* condList)
        : WorldObjectSpellTargetCheck(caster, referer, spellInfo, selectionType, condList), _range(range), _position(position)
    {
    }

    bool WorldObjectSpellAreaTargetCheck::operator()(WorldObject* target)
    {
        if (target->ToGameObject())
        {
            // isInRange including the dimension of the GO
            bool isInRange = target->ToGameObject()->IsInRange(_position->GetPositionX(), _position->GetPositionY(), _position->GetPositionZ(), _range);
            if (!isInRange)
                return false;
        }
        else
        {
            bool isInsideCylinder = target->IsWithinDist2d(_position, _range) && std::abs(target->GetPositionZ() - _position->GetPositionZ()) <= _range;
            if (!isInsideCylinder)
                return false;
        }

        return WorldObjectSpellTargetCheck::operator ()(target);
    }

    WorldObjectSpellConeTargetCheck::WorldObjectSpellConeTargetCheck(float coneAngle, float range, Unit* caster,
        SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionContainer* condList)
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
            if (!_caster->HasInLine(target, target->GetCombatReach(), _caster->GetCombatReach()))
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

    WorldObjectSpellTrajTargetCheck::WorldObjectSpellTrajTargetCheck(float range, Position const* position, Unit* caster, SpellInfo const* spellInfo, SpellTargetCheckTypes selectionType, ConditionContainer* condList)
        : WorldObjectSpellTargetCheck(caster, caster, spellInfo, selectionType, condList), _range(range), _position(position) { }

    bool WorldObjectSpellTrajTargetCheck::operator()(WorldObject* target)
    {
        // return all targets on missile trajectory
        if (!_caster->HasInLine(target, target->GetCombatReach(), TRAJECTORY_MISSILE_SIZE))
            return false;

        if (target->GetExactDist2d(_position) > _range)
            return false;

        return WorldObjectSpellTargetCheck::operator ()(target);
    }

} //namespace Trinity

void Spell::LoadScripts()
{
    if (_scriptsLoaded)
        return;
    _scriptsLoaded = true;
    sScriptMgr->CreateSpellScripts(m_spellInfo->Id, m_loadedScripts, this);
    for (auto itr = m_loadedScripts.begin(); itr != m_loadedScripts.end();)
    {
        if (!(*itr)->_Load(this))
        {
            auto bitr = itr;
            ++itr;
            delete (*bitr);
            m_loadedScripts.erase(bitr);
            continue;
        }
        (*itr)->Register();
        ++itr;
    }
}

void Spell::CallScriptDestinationTargetSelectHandlers(SpellDestination& target, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    for (auto & m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_DESTINATION_TARGET_SELECT);
        auto hookItrEnd = m_loadedScript->OnDestinationTargetSelect.end(), hookItr = m_loadedScript->OnDestinationTargetSelect.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if (hookItr->IsEffectAffected(m_spellInfo, effIndex) && targetType.GetTarget() == hookItr->GetTarget())
                hookItr->Call(m_loadedScript, target);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptObjectAreaTargetSelectHandlers(std::list<WorldObject*>& targets, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
  for (auto & m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_OBJECT_AREA_TARGET_SELECT);
        auto hookItrEnd = m_loadedScript->OnObjectAreaTargetSelect.end(), hookItr = m_loadedScript->OnObjectAreaTargetSelect.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if (hookItr->IsEffectAffected(m_spellInfo, effIndex) && targetType.GetTarget() == hookItr->GetTarget())
                hookItr->Call(m_loadedScript, targets);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptObjectTargetSelectHandlers(WorldObject*& target, SpellEffIndex effIndex, SpellImplicitTargetInfo const& targetType)
{
    for (auto & m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_OBJECT_TARGET_SELECT);
        auto hookItrEnd = m_loadedScript->OnObjectTargetSelect.end(), hookItr = m_loadedScript->OnObjectTargetSelect.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            if (hookItr->IsEffectAffected(m_spellInfo, effIndex) && targetType.GetTarget() == hookItr->GetTarget())
                hookItr->Call(m_loadedScript, target);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::PrepareScriptHitHandlers()
{
    for (auto & m_loadedScript : m_loadedScripts)
        m_loadedScript->_InitHit();
}


bool Spell::CallScriptEffectHandlers(SpellEffIndex effIndex, SpellEffectHandleMode mode)
{
    // execute script effect handler hooks and check if effects was prevented
    bool preventDefault = false;

    for (auto & m_loadedScript : m_loadedScripts)
    {
        HookList<SpellScript::EffectHandler>::iterator effItr, effEndItr;
        SpellScriptHookType hookType;
        switch (mode)
        {
        case SPELL_EFFECT_HANDLE_LAUNCH:
            effItr = m_loadedScript->OnEffectLaunch.begin();
            effEndItr = m_loadedScript->OnEffectLaunch.end();
            hookType = SPELL_SCRIPT_HOOK_EFFECT_LAUNCH;
            break;
        case SPELL_EFFECT_HANDLE_LAUNCH_TARGET:
            effItr = m_loadedScript->OnEffectLaunchTarget.begin();
            effEndItr = m_loadedScript->OnEffectLaunchTarget.end();
            hookType = SPELL_SCRIPT_HOOK_EFFECT_LAUNCH_TARGET;
            break;
        case SPELL_EFFECT_HANDLE_HIT:
            effItr = m_loadedScript->OnEffectHit.begin();
            effEndItr = m_loadedScript->OnEffectHit.end();
            hookType = SPELL_SCRIPT_HOOK_EFFECT_HIT;
            break;
        case SPELL_EFFECT_HANDLE_HIT_TARGET:
            effItr = m_loadedScript->OnEffectHitTarget.begin();
            effEndItr = m_loadedScript->OnEffectHitTarget.end();
            hookType = SPELL_SCRIPT_HOOK_EFFECT_HIT_TARGET;
            break;
        default:
            ASSERT(false);
            return false;
        }
        m_loadedScript->_PrepareScriptCall(hookType);
        for (; effItr != effEndItr; ++effItr)
            // effect execution can be prevented
            if (!m_loadedScript->_IsEffectPrevented(effIndex) && (*effItr).IsEffectAffected(m_spellInfo, effIndex))
                (*effItr).Call(m_loadedScript, effIndex);

        if (!preventDefault)
            preventDefault = m_loadedScript->_IsDefaultEffectPrevented(effIndex);

        m_loadedScript->_FinishScriptCall();
    }

    return preventDefault;
}

void Spell::CallScriptAfterCastHandlers()
{
    for (auto & m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_AFTER_CAST);
        auto hookItrEnd = m_loadedScript->AfterCast.end(), hookItr = m_loadedScript->AfterCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

SpellCastResult Spell::CallScriptCheckCastHandlers()
{
    SpellCastResult retVal = SPELL_CAST_OK;
    for (auto & m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_CHECK_CAST);
        auto hookItrEnd = m_loadedScript->OnCheckCast.end(), hookItr = m_loadedScript->OnCheckCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
        {
            SpellCastResult tempResult = (*hookItr).Call(m_loadedScript);
            if (retVal == SPELL_CAST_OK)
                retVal = tempResult;
        }

        m_loadedScript->_FinishScriptCall();
    }
    return retVal;
}

void Spell::CallScriptAfterHitHandlers()
{
    for (auto & m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_AFTER_HIT);
        auto hookItrEnd = m_loadedScript->AfterHit.end(), hookItr = m_loadedScript->AfterHit.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptBeforeCastHandlers()
{
    for (auto & m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_BEFORE_CAST);
        auto hookItrEnd = m_loadedScript->BeforeCast.end(), hookItr = m_loadedScript->BeforeCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptBeforeHitHandlers()
{
    for (auto & m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_BEFORE_HIT);
        auto hookItrEnd = m_loadedScript->BeforeHit.end(), hookItr = m_loadedScript->BeforeHit.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptOnCastHandlers()
{
    for (auto & m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_ON_CAST);
        auto hookItrEnd = m_loadedScript->OnCast.end(), hookItr = m_loadedScript->OnCast.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

void Spell::CallScriptOnHitHandlers()
{
    for (auto & m_loadedScript : m_loadedScripts)
    {
        m_loadedScript->_PrepareScriptCall(SPELL_SCRIPT_HOOK_HIT);
        auto hookItrEnd = m_loadedScript->OnHit.end(), hookItr = m_loadedScript->OnHit.begin();
        for (; hookItr != hookItrEnd; ++hookItr)
            (*hookItr).Call(m_loadedScript);

        m_loadedScript->_FinishScriptCall();
    }
}

bool Spell::CheckScriptEffectImplicitTargets(SpellEffIndex effIndex, SpellEffIndex effIndexToCheck)
{
    // Skip if there are not any script
    if (!m_loadedScripts.size())
        return true;

    for (auto & m_loadedScript : m_loadedScripts)
    {
        auto targetSelectHookEnd = m_loadedScript->OnObjectTargetSelect.end(), targetSelectHookItr = m_loadedScript->OnObjectTargetSelect.begin();
        for (; targetSelectHookItr != targetSelectHookEnd; ++targetSelectHookItr)
            if (((*targetSelectHookItr).IsEffectAffected(m_spellInfo, effIndex) && !(*targetSelectHookItr).IsEffectAffected(m_spellInfo, effIndexToCheck)) ||
                (!(*targetSelectHookItr).IsEffectAffected(m_spellInfo, effIndex) && (*targetSelectHookItr).IsEffectAffected(m_spellInfo, effIndexToCheck)))
                return false;

        auto areaTargetSelectHookEnd = m_loadedScript->OnObjectAreaTargetSelect.end(), areaTargetSelectHookItr = m_loadedScript->OnObjectAreaTargetSelect.begin();
        for (; areaTargetSelectHookItr != areaTargetSelectHookEnd; ++areaTargetSelectHookItr)
            if (((*areaTargetSelectHookItr).IsEffectAffected(m_spellInfo, effIndex) && !(*areaTargetSelectHookItr).IsEffectAffected(m_spellInfo, effIndexToCheck)) ||
                (!(*areaTargetSelectHookItr).IsEffectAffected(m_spellInfo, effIndex) && (*areaTargetSelectHookItr).IsEffectAffected(m_spellInfo, effIndexToCheck)))
                return false;
    }
    return true;
}

bool Spell::IsAutoActionResetSpell() const
{
    /// @todo changed SPELL_INTERRUPT_FLAG_AUTOATTACK -> SPELL_INTERRUPT_FLAG_INTERRUPT to fix compile - is this check correct at all?
    if (IsTriggered() || !(m_spellInfo->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT))
        return false;

    if (!m_casttime && m_spellInfo->HasAttribute(SPELL_ATTR6_NOT_RESET_SWING_IF_INSTANT))
        return false;

    return true;
}

bool Spell::IsPositive(bool hostileTarget) const
{
    return m_spellInfo->IsPositive(hostileTarget) && (!m_triggeredByAuraSpell || m_triggeredByAuraSpell->IsPositive(hostileTarget));
}

bool Spell::IsTriggered() const
{
    return (_triggeredCastFlags & TRIGGERED_FULL_MASK) != 0;
}

bool Spell::IsIgnoringCooldowns() const
{
    return (_triggeredCastFlags & TRIGGERED_IGNORE_SPELL_AND_CATEGORY_CD) != 0;
}

bool Spell::IsProcDisabled() const
{
    return (_triggeredCastFlags & TRIGGERED_DISALLOW_PROC_EVENTS) != 0;
}
