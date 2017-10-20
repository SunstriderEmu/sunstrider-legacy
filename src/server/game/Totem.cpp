
#include "Totem.h"
#include "WorldPacket.h"
#include "MapManager.h"
#include "Log.h"
#include "Group.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"

#define SENTRY_TOTEM_SPELLID  6495

Totem::Totem(SummonPropertiesEntry const* properties, Unit* owner) : Minion(properties, owner, false)
{
    m_unitTypeMask |= UNIT_MASK_TOTEM;
    m_duration = 0;
    m_type = TOTEM_PASSIVE;
}

Totem::~Totem()
{
}

void Totem::Update( uint32 time )
{
    Unit *owner = GetOwner();
    if (!owner || !owner->IsAlive() || !this->IsAlive())
    {
        UnSummon();                                         // remove self
        return;
    }

    if (m_duration <= time)
    {
        UnSummon();                                         // remove self
        return;
    }
    else
        m_duration -= time;

    Creature::Update( time );
}

void Totem::UnSummon(uint32 msTime)
{
    if (msTime)
    {
        m_Events.AddEvent(new ForcedUnsummonDelayEvent(*this), m_Events.CalculateTime(msTime));
        return;
    }

    SendObjectDeSpawnAnim(GetGUID());

    CombatStop();
    RemoveAurasDueToSpell(GetSpell());
    Unit *owner = this->GetOwner();
    if (owner)
    {
        // clear owner's totem slot
        for (uint8 i = SUMMON_SLOT_TOTEM; i < MAX_TOTEM_SLOT; ++i)
        {
            if (GetOwner()->m_SummonSlot[i] == GetGUID())
            {
                //GetOwner()->m_SummonSlot[i].Clear();
                GetOwner()->m_SummonSlot[i] = 0;
                break;
            }
        }

        owner->RemoveAurasDueToSpell(GetSpell());

        // Remove Sentry Totem Aura
        if (GetEntry() == SENTRY_TOTEM_ENTRY)
            GetOwner()->RemoveAurasDueToSpell(SENTRY_TOTEM_SPELLID);

        //remove aura all party members too
        Group *pGroup = nullptr;
        if (owner->GetTypeId() == TYPEID_PLAYER)
        {
            // Not only the player can summon the totem (scripted AI)
            pGroup = (owner->ToPlayer())->GetGroup();
            if (pGroup)
            {
                for(GroupReference *itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
                {
                    Player* Target = itr->GetSource();
                    if(Target && pGroup->SameSubGroup(owner->ToPlayer(), Target))
                        Target->RemoveAurasDueToSpell(GetSpell());
                }
            }
        }
    }

    // any totem unsummon look like as totem kill, req. for proper animation
    /* Commented as it currently already does it without this
    if (IsAlive())
        SetDeathState(DEAD);
    */

    AddObjectToRemoveList();
}

void Totem::SetOwner(uint64 guid)
{
    SetCreatorGUID(guid);
    SetOwnerGUID(guid);
    if (Unit *owner = GetOwner())
    {
        this->SetFaction(owner->GetFaction());
        this->SetLevel(owner->GetLevel());
    }
}

/*
Unit *Totem::GetOwner()
{
    uint64 ownerid = GetOwnerGUID();
    if(!ownerid)
        return nullptr;
    return ObjectAccessor::GetUnit(*this, ownerid);
}
*/

void Totem::InitSummon()
{
    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE); //sunstrider

    if (uint32 spell_id = GetSpell())
    {
        switch (m_type)
        {
            case TOTEM_PASSIVE: CastSpell(this, GetSpell(), true); break;
            case TOTEM_STATUE:  CastSpell(GetOwner(), GetSpell(), true); break; //TC does not do this one, is there a reason?
        }
    }

    // Some totems can have both instant effect and passive spell
    if (GetSpell(1))
        CastSpell(this, GetSpell(1), true);

#ifndef LICH_KING
    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
    data << GetGUID();
    SendMessageToSet(&data, true);
#endif
}

void Totem::InitStats(uint32 duration)
{
    // client requires SMSG_TOTEM_CREATED to be sent before adding to world and before removing old totem
    if (GetOwner()->GetTypeId() == TYPEID_PLAYER
        && m_Properties->Slot >= SUMMON_SLOT_TOTEM
        && m_Properties->Slot < MAX_TOTEM_SLOT)
    {
        WorldPacket data(SMSG_TOTEM_CREATED, 1 + 8 + 4 + 4);
        data << uint8(m_Properties->Slot - 1);
        data << uint64(GetGUID());
        data << uint32(duration);
        data << uint32(GetUInt32Value(UNIT_CREATED_BY_SPELL));
        GetOwner()->ToPlayer()->SendDirectMessage(&data);

        // set display id depending on caster's race
        SetDisplayId(GetOwner()->GetModelForTotem(PlayerTotemType(m_Properties->Id)));
    }

    Minion::InitStats(duration);

    // Get spell cast by totem
    if (SpellInfo const* totemSpell = sSpellMgr->GetSpellInfo(GetSpell()))
    {
        if (totemSpell->CalcCastTime())   // If spell has cast time -> its an active totem
            m_type = TOTEM_ACTIVE;

        if (totemSpell->SpellIconID == 2056)
            m_type = TOTEM_STATUE;                              //Jewelery statue
    }

    if (GetEntry() == SENTRY_TOTEM_ENTRY)
        SetReactState(REACT_AGGRESSIVE);

    m_duration = duration;

    SetLevel(GetOwner()->GetLevel());
}

bool Totem::IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index, Unit* caster) const
{
    /// @todo possibly all negative auras immune?
    if (GetEntry() == 5925)
        return false;

    switch (spellInfo->Effects[index].ApplyAuraName)
    {
    case SPELL_AURA_PERIODIC_DAMAGE:
    case SPELL_AURA_PERIODIC_LEECH:
    case SPELL_AURA_MOD_FEAR:
    case SPELL_AURA_TRANSFORM:
        return true;
    default:
        break;
    }

    return Creature::IsImmunedToSpellEffect(spellInfo, index, caster);
}
