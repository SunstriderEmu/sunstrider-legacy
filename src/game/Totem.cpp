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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "Totem.h"
#include "WorldPacket.h"
#include "MapManager.h"
#include "Log.h"
#include "Group.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"

Totem::Totem() : Creature()
{
    m_unitTypeMask |= UNIT_MASK_TOTEM;
    m_isTotem = true;
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

void Totem::Summon(Unit* owner)
{
    CreatureTemplate const *cinfo = GetCreatureTemplate();
    if (owner->GetTypeId()==TYPEID_PLAYER && cinfo)
    {
        uint32 modelid = 0;
        if((owner->ToPlayer())->GetTeam() == TEAM_HORDE)
        {
            if(cinfo->Modelid3)
                modelid = cinfo->Modelid3;
            else if(cinfo->Modelid4)
                modelid = cinfo->Modelid4;
        }
        else
        {
            if(cinfo->Modelid1)
                modelid = cinfo->Modelid1;
            else if(cinfo->Modelid2)
                modelid = cinfo->Modelid2;
        }
        if (modelid)
            SetDisplayId(modelid);
        else
            TC_LOG_ERROR("FIXME","Totem::Summon: Missing modelid information for entry %u, team %u, totem will use default values.",GetEntry(),(owner->ToPlayer())->GetTeam());
    }

    // Only add if a display exists.
    SetInstanceId(owner->GetInstanceId());
    owner->GetMap()->Add(this->ToCreature(), true);

    WorldPacket data(SMSG_GAMEOBJECT_SPAWN_ANIM_OBSOLETE, 8);
    data << GetGUID();
    SendMessageToSet(&data,true);

    AIM_Initialize();

    switch(m_type)
    {
        case TOTEM_PASSIVE: CastSpell(this, GetSpell(), true); break;
        case TOTEM_STATUE:  CastSpell(GetOwner(), GetSpell(), true); break;
        default: break;
    }

    if(GetEntry() == SENTRY_TOTEM_ENTRY)
        SetReactState(REACT_AGGRESSIVE);
}

void Totem::UnSummon()
{
    SendObjectDeSpawnAnim(GetGUID());

    CombatStop();
    RemoveAurasDueToSpell(GetSpell());
    Unit *owner = this->GetOwner();
    if (owner)
    {
        // clear owenr's totem slot
        for(int i = 0; i < MAX_TOTEM; ++i)
        {
            if(owner->m_TotemSlot[i]==GetGUID())
            {
                owner->m_TotemSlot[i] = 0;
                break;
            }
        }

        owner->RemoveAurasDueToSpell(GetSpell());

        //remove aura all party members too
        Group *pGroup = NULL;
        if (owner->GetTypeId() == TYPEID_PLAYER)
        {
            // Not only the player can summon the totem (scripted AI)
            pGroup = (owner->ToPlayer())->GetGroup();
            if (pGroup)
            {
                for(GroupReference *itr = pGroup->GetFirstMember(); itr != NULL; itr = itr->next())
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

Unit *Totem::GetOwner()
{
    uint64 ownerid = GetOwnerGUID();
    if(!ownerid)
        return NULL;
    return ObjectAccessor::GetUnit(*this, ownerid);
}

void Totem::SetTypeBySummonSpell(SpellInfo const * spellProto)
{
    // Get spell casted by totem
    SpellInfo const * totemSpell = sSpellMgr->GetSpellInfo(GetSpell());
    if (totemSpell)
    {
        // If spell have cast time -> so its active totem
        if (totemSpell->CalcCastTime())
            m_type = TOTEM_ACTIVE;
    }
    if(spellProto->SpellIconID==2056)
        m_type = TOTEM_STATUE;                              //Jewelery statue
}

bool Totem::IsImmunedToSpell(SpellInfo const* spellInfo, bool useCharges)
{
/*    for (int i=0;i<3;i++)
    {
        switch(spellInfo->Effects[i].ApplyAuraName)
        {
            case SPELL_AURA_PERIODIC_DAMAGE:
            case SPELL_AURA_PERIODIC_LEECH:
                return true;
            default:
                continue;
        }
    }*/
    return Creature::IsImmunedToSpell(spellInfo, useCharges);
}

