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

#ifndef TRINITY_GRIDNOTIFIERSIMPL_H
#define TRINITY_GRIDNOTIFIERSIMPL_H

#include "GridNotifiers.h"
#include "WorldPacket.h"
#include "Corpse.h"
#include "Player.h"
#include "UpdateData.h"
#include "CreatureAI.h"
#include "SpellAuras.h"
#include "CreatureAINew.h"
#include "SpellInfo.h

inline void
Trinity::ObjectUpdater::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
        if(iter->GetSource()->IsInWorld() && !iter->GetSource()->isSpiritService())
            iter->GetSource()->Update(i_timeDiff);
}

inline void PlayerCreatureRelocationWorker(Player* pl, Creature* c)
{
    if(!pl->IsAlive() || !c->IsAlive() || pl->IsInFlight() || !c->IsAIEnabled)
        return;

    bool withinSightDist = c->IsWithinSightDist(pl);
    if(!withinSightDist) 
        return;

    // Creature AI reaction
    if(c->HasReactState(REACT_AGGRESSIVE) && !c->HasUnitState(UNIT_STATE_SIGHTLESS) && !c->IsInEvadeMode())
    {
        c->AI()->MoveInLineOfSight(pl);
        if (c->getAI())
            c->getAI()->onMoveInLoS(pl);
    }
    c->AI()->MoveInLineOfSight2(pl);
}

inline void CreatureCreatureRelocationWorker(Creature* c1, Creature* c2)
{
    bool withinSightDist = c1->IsWithinSightDist(c2);
    if(!withinSightDist) return;

    if(c1->IsAIEnabled)
    {
        if(c1->HasReactState(REACT_AGGRESSIVE) && !c1->HasUnitState(UNIT_STATE_SIGHTLESS) && !c1->IsInEvadeMode())
        {
            c1->AI()->MoveInLineOfSight(c2);
            if (c1->getAI())
                c1->getAI()->onMoveInLoS(c2);
        }
        c1->AI()->MoveInLineOfSight2(c2);
    }

    if(c2->IsAIEnabled)
    {
        if(c2->HasReactState(REACT_AGGRESSIVE) && !c2->HasUnitState(UNIT_STATE_SIGHTLESS) && !c2->IsInEvadeMode())
        {
            c2->AI()->MoveInLineOfSight(c1);
            if (c2->getAI())
                c2->getAI()->onMoveInLoS(c1);
        }
        c2->AI()->MoveInLineOfSight2(c1);
    }
}

template<class T>
inline void
Trinity::PlayerVisibilityNotifier::Visit(GridRefManager<T> &m)
{
    for(typename GridRefManager<T>::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        i_player.UpdateVisibilityOf(iter->GetSource(),i_data,i_visibleNow);
        i_clientGUIDs.erase(iter->GetSource()->GetGUID());
    }
}

template<>
inline void
Trinity::PlayerRelocationNotifier::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        i_clientGUIDs.erase(iter->GetSource()->GetGUID()); //remaining guids are marked for deletion later, so erasing here means we're going to keep these at client

        if(iter->GetSource()->m_Notified) //self is also skipped in this check
            continue;

        i_player.UpdateVisibilityOf(iter->GetSource(),i_data,i_visibleNow);
        iter->GetSource()->UpdateVisibilityOf(&i_player);

        for (auto it : i_player.GetSharedVisionList())
            if(Player* p = i_player.GetPlayer(it))
                p->UpdateVisibilityOf(iter->GetSource());

        // Cancel Trade
        if(i_player.GetTrader()==iter->GetSource())
            if(!i_player.IsWithinDistInMap(iter->GetSource(), 5)) // iteraction distance
                i_player.GetSession()->SendCancelTrade();   // will clode both side trade windows
    }
}

template<>
inline void
Trinity::PlayerRelocationNotifier::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        i_clientGUIDs.erase(iter->GetSource()->GetGUID()); //remaining guids are marked for deletion later, so erasing here means we're going to keep these at client

        if(iter->GetSource()->m_Notified)
            continue;

        i_player.UpdateVisibilityOf(iter->GetSource(),i_data,i_visibleNow);

        for (auto it : i_player.GetSharedVisionList())
            if(Player* p = i_player.GetPlayer(it))
                p->UpdateVisibilityOf(iter->GetSource());

        PlayerCreatureRelocationWorker(&i_player, iter->GetSource());
    }
}

template<>
inline void
Trinity::CreatureRelocationNotifier::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        if(iter->GetSource()->m_Notified)
            continue;

        iter->GetSource()->UpdateVisibilityOf(&i_creature);

        for (auto it : i_creature.GetSharedVisionList())
            if(Player* p = i_creature.GetPlayer(it))
                p->UpdateVisibilityOf(iter->GetSource());
        
        PlayerCreatureRelocationWorker(iter->GetSource(), &i_creature);
    }
}

template<>
inline void
Trinity::CreatureRelocationNotifier::Visit(CreatureMapType &m)
{
    if(!i_creature.IsAlive())
        return;

    for(CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        if(iter->GetSource()->m_Notified)
            continue;
        
        if(!iter->GetSource()->IsAlive())
            continue;

        for (auto it : i_creature.GetSharedVisionList())
            if(Player* p = i_creature.GetPlayer(it))
                p->UpdateVisibilityOf(iter->GetSource());

        CreatureCreatureRelocationWorker(iter->GetSource(), &i_creature);
    }
}

inline void Trinity::DynamicObjectUpdater::VisitHelper(Unit* target)
{
    if(!target->IsAlive() || target->IsInFlight() )
        return;

    if(target->GetTypeId()==TYPEID_UNIT && (target->ToCreature())->IsTotem())
        return;

    if (!i_dynobject.IsWithinDistInMap(target, i_dynobject.GetRadius()))
        return;

    //Check targets for not_selectable unit flag and remove
    if (target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE))
        return;

    // Evade target
    if( target->GetTypeId()==TYPEID_UNIT && (target->ToCreature())->IsInEvadeMode() )
        return;

    //Check player targets and remove if in GM mode or GM invisibility (for not self casting case)
    if( target->GetTypeId()==TYPEID_PLAYER && target != i_check && (((target->ToPlayer())->IsGameMaster() || (target->ToPlayer())->isSpectator()) || (target->ToPlayer())->GetVisibility()==VISIBILITY_OFF) )
        return;

    if (i_dynobject.IsAffecting(target))
        return;

    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(i_dynobject.GetSpellId());
    uint32 eff_index  = i_dynobject.GetEffIndex();
    if(spellInfo->EffectImplicitTargetB[eff_index] == TARGET_DEST_DYNOBJ_ALLY
        || spellInfo->EffectImplicitTargetB[eff_index] == TARGET_UNIT_DEST_AREA_ALLY)
    {
        if(!i_check->IsFriendlyTo(target))
            return;
    }
    else 
    {
        if(i_check->GetTypeId()==TYPEID_PLAYER )
        {
            if (i_check->IsFriendlyTo( target ))
                return;
        } else {
            if (!i_check->IsHostileTo( target ))
                return;
        }

        if (   !(spellInfo->AttributesEx  & SPELL_ATTR1_NO_THREAT)
            && !(spellInfo->AttributesEx3 & SPELL_ATTR3_NO_INITIAL_AGGRO) )
           i_check->CombatStart(target);
    }

    // Check target immune to spell or aura
    if (target->IsImmunedToSpell(spellInfo) || target->IsImmunedToSpellEffect(spellInfo->Effect[eff_index], spellInfo->EffectMechanic[eff_index]))
        return;
    
    i_dynobject.AddAffected(target);

    // Add source to an existing aura if any, else create one
    if(Aura* aur = target->GetAuraByCasterSpell(spellInfo->Id,eff_index,i_check->GetGUID()))
    {
        PersistentAreaAura* pAur = dynamic_cast<PersistentAreaAura*>(aur);
        if(pAur)
        {
            pAur->AddSource(&i_dynobject);
            return;
        }
    } else {
        PersistentAreaAura* pAur = new PersistentAreaAura(spellInfo, eff_index, NULL, target, i_check);
        pAur->AddSource(&i_dynobject);
        pAur->SetAuraDuration(i_dynobject.GetDuration());
        target->AddAura(pAur);
    }
}

template<>
inline void
Trinity::DynamicObjectUpdater::Visit(CreatureMapType  &m)
{
    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        VisitHelper(itr->GetSource());
}

template<>
inline void
Trinity::DynamicObjectUpdater::Visit(PlayerMapType  &m)
{
    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        VisitHelper(itr->GetSource());
}

// SEARCHERS & LIST SEARCHERS & WORKERS

// WorldObject searchers & workers

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(GameObjectMapType &m)
{
    // already found
    if(i_object)
        return;

    for(GameObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
        {
            i_object = itr->GetSource();
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(PlayerMapType &m)
{
    // already found
    if(i_object)
        return;

    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
        {
            i_object = itr->GetSource();
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(CreatureMapType &m)
{
    // already found
    if(i_object)
        return;

    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
        {
            i_object = itr->GetSource();
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(CorpseMapType &m)
{
    // already found
    if(i_object)
        return;

    for(CorpseMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
        {
            i_object = itr->GetSource();
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectSearcher<Check>::Visit(DynamicObjectMapType &m)
{
    // already found
    if(i_object)
        return;

    for(DynamicObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
        {
            i_object = itr->GetSource();
            return;
        }
    }
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->GetSource()))
            i_objects.push_back(itr->GetSource());
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->GetSource()))
            i_objects.push_back(itr->GetSource());
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(CorpseMapType &m)
{
    for(CorpseMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->GetSource()))
            i_objects.push_back(itr->GetSource());
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(GameObjectMapType &m)
{
    for(GameObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->GetSource()))
            i_objects.push_back(itr->GetSource());
}

template<class Check>
void Trinity::WorldObjectListSearcher<Check>::Visit(DynamicObjectMapType &m)
{
    for(DynamicObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->GetSource()))
            i_objects.push_back(itr->GetSource());
}

// Gameobject searchers

template<class Check>
void Trinity::GameObjectSearcher<Check>::Visit(GameObjectMapType &m)
{
    // already found
    if(i_object)
        return;

    for(GameObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
        {
            i_object = itr->GetSource();
            return;
        }
    }
}

template<class Check>
void Trinity::GameObjectLastSearcher<Check>::Visit(GameObjectMapType &m)
{
    for(GameObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
            i_object = itr->GetSource();
    }
}

template<class Check>
void Trinity::GameObjectListSearcher<Check>::Visit(GameObjectMapType &m)
{
    for(GameObjectMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->GetSource()))
            i_objects.push_back(itr->GetSource());
}

// Unit searchers

template<class Check>
void Trinity::UnitSearcher<Check>::Visit(CreatureMapType &m)
{
    // already found
    if(i_object)
        return;

    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
        {
            i_object = itr->GetSource();
            return;
        }
    }
}

template<class Check>
void Trinity::UnitSearcher<Check>::Visit(PlayerMapType &m)
{
    // already found
    if(i_object)
        return;

    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
        {
            i_object = itr->GetSource();
            return;
        }
    }
}

template<class Check>
void Trinity::UnitLastSearcher<Check>::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
            i_object = itr->GetSource();
    }
}

template<class Check>
void Trinity::UnitLastSearcher<Check>::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
            i_object = itr->GetSource();
    }
}

template<class Check>
void Trinity::UnitListSearcher<Check>::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->GetSource()))
            i_objects.push_back(itr->GetSource());
}

template<class Check>
void Trinity::UnitListSearcher<Check>::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->GetSource()))
            i_objects.push_back(itr->GetSource());
}

template<class Check>
void Trinity::PlayerListSearcher<Check>::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->GetSource()))
            i_objects.push_back(itr->GetSource());
}

// Creature searchers

template<class Check>
void Trinity::CreatureSearcher<Check>::Visit(CreatureMapType &m)
{
    // already found
    if(i_object)
        return;

    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
        {
            i_object = itr->GetSource();
            return;
        }
    }
}

template<class Check>
void Trinity::CreatureLastSearcher<Check>::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
            i_object = itr->GetSource();
    }
}

template<class Check>
void Trinity::CreatureListSearcher<Check>::Visit(CreatureMapType &m)
{
    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->GetSource()))
            i_objects.push_back(itr->GetSource());
}

template<class Check>
void Trinity::CreatureListSearcherWithRange<Check>::Visit(CreatureMapType &m)
{
    float range;
    for(CreatureMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
        if(i_check(itr->GetSource(), range))
        {
            auto pair = std::make_pair(itr->GetSource(),range);
            i_objects.push_back(pair);
        }
}

template<class Check>
void Trinity::PlayerSearcher<Check>::Visit(PlayerMapType &m)
{
    // already found
    if(i_object)
        return;

    for(PlayerMapType::iterator itr=m.begin(); itr != m.end(); ++itr)
    {
        if(i_check(itr->GetSource()))
        {
            i_object = itr->GetSource();
            return;
        }
    }
}

#endif                                                      // TRINITY_GRIDNOTIFIERSIMPL_H

