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

#include "GridNotifiers.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "Item.h"
#include "Map.h"
#include "MapManager.h"
#include "Transport.h"
#include "ObjectAccessor.h"

using namespace Trinity;

void
VisibleChangesNotifier::Visit(PlayerMapType &m)
{
    for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
    {
        if(iter->GetSource() == &i_object)
            continue;

        iter->GetSource()->UpdateVisibilityOf(&i_object);
    }
}

void
PlayerVisibilityNotifier::Notify()
{
    // at this moment i_clientGUIDs have guids that not iterate at grid level checks
    // but exist some case when this is possible and object not out of range: transports

    if(Transport* transport = i_player.GetTransport())
    {
        for(Transport::PassengerSet::const_iterator itr = transport->GetPassengers().begin();itr!=transport->GetPassengers().end();++itr)
        {
            if(i_clientGUIDs.find((*itr)->GetGUID())!=i_clientGUIDs.end())
            {
                i_clientGUIDs.erase((*itr)->GetGUID());

                switch ((*itr)->GetTypeId())
                {
                    case TYPEID_GAMEOBJECT:
                        i_player.UpdateVisibilityOf((*itr)->ToGameObject(), i_data, i_visibleNow);
                        break;
                    case TYPEID_PLAYER:
                        i_player.UpdateVisibilityOf((*itr)->ToPlayer(), i_data, i_visibleNow);
                        (*itr)->ToPlayer()->UpdateVisibilityOf(&i_player);
                        break;
                    case TYPEID_UNIT:
                        i_player.UpdateVisibilityOf((*itr)->ToCreature(), i_data, i_visibleNow);
                        break;
                    case TYPEID_DYNAMICOBJECT:
                        i_player.UpdateVisibilityOf((*itr)->ToDynObject(), i_data, i_visibleNow);
                        break;
                    default:
                        break;
                }                
            }
        }
    }

    //also keep far sight targets (is this needed ? maybe it's already done by PlayerRelocationNotifier atm)
    if(i_player.GetFarSight())
        i_clientGUIDs.erase(i_player.GetFarSight());

    //remaining i_clientGUIDs are out of range and should be destroyed at client
    i_data.AddOutOfRangeGUID(i_clientGUIDs);
    for(Player::ClientGUIDs::iterator itr = i_clientGUIDs.begin();itr!=i_clientGUIDs.end();++itr)
    {
        i_player.m_clientGUIDs.erase(*itr);

        TC_LOG_DEBUG("debug.grid","Object %u (Type: %u) is out of range (no in active cells set) now for player %u",GUID_LOPART(*itr),GuidHigh2TypeId(GUID_HIPART(*itr)),i_player.GetGUIDLow());
    }

    if( i_data.HasData() )
    {
        // send create/outofrange packet to player (except player create updates that already sent using SendUpdateToPlayer)
        WorldPacket packet;
        i_data.BuildPacket(&packet);
        i_player.SendDirectMessage(&packet);
        for (auto it : i_player.GetSharedVisionList())
            if(Player* p = ObjectAccessor::FindPlayer(it))
                p->SendDirectMessage(&packet);

        // send out of range to other players if need
        std::set<uint64> const& oor = i_data.GetOutOfRangeGUIDs();
        for(std::set<uint64>::const_iterator iter = oor.begin(); iter != oor.end(); ++iter)
        {
            if(!IS_PLAYER_GUID(*iter))
                continue;

            if(Player* plr = ObjectAccessor::FindPlayer(*iter))
                plr->UpdateVisibilityOf(&i_player);
        }
    }

    // Now do operations that required done at object visibility change to visible

    // target aura duration for caster show only if target exist at caster client
    // send data at target visibility change (adding to client)
    for(std::set<WorldObject*>::const_iterator vItr = i_visibleNow.begin(); vItr != i_visibleNow.end(); ++vItr)
        if((*vItr)!=&i_player && (*vItr)->isType(TYPEMASK_UNIT))
            i_player.SendInitialVisiblePackets((Unit*)(*vItr));

    if(i_visibleNow.size() >= 30)
        i_player.SetToNotify();
}

void
Deliverer::Visit(PlayerMapType &m)
{
    for (PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        if (!i_dist || iter->GetSource()->GetDistance(&i_source) <= i_dist)
        {
            // Send packet to all who are sharing the player's vision
            for (auto itr : iter->GetSource()->GetSharedVisionList())
                if(Player* p = ObjectAccessor::FindPlayer(itr))
                    SendPacket(p);

            VisitObject(iter->GetSource());
        }
    }
}

void
Deliverer::Visit(CreatureMapType &m)
{
    for (CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        if (!i_dist || iter->GetSource()->GetDistance(&i_source) <= i_dist)
        {
            // Send packet to all who are sharing the creature's vision
            for (auto itr : iter->GetSource()->GetSharedVisionList())
                if(Player* p = ObjectAccessor::FindPlayer(itr))
                    SendPacket(p);
        }
    }
}

void
Deliverer::Visit(DynamicObjectMapType &m)
{
    for (DynamicObjectMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        if (IS_PLAYER_GUID(iter->GetSource()->GetCasterGUID()))
        {
            // Send packet back to the caster if the caster has vision of dynamic object
            Unit* caster_unit = iter->GetSource()->GetCaster();
            Player* caster = caster_unit ? caster_unit->ToPlayer() : NULL;
            if (caster && caster->GetUInt64Value(PLAYER_FARSIGHT) == iter->GetSource()->GetGUID() &&
                (!i_dist || iter->GetSource()->GetDistance(&i_source) <= i_dist))
                SendPacket(caster);
        }
    }
}

void
Deliverer::SendPacket(Player* plr)
{
    if (!plr)
        return;

    // Don't send the packet to self if not supposed to
    if (!i_toSelf && plr == &i_source)
        return;

    // Don't send the packet to possesor if not supposed to
    if (!i_toPossessor && plr->IsPossessing() && plr->GetCharmGUID() == i_source.GetGUID())
        return;

    if (plr_list.insert(plr->GetGUID()).second) //return true if a new element was inserted
        if (WorldSession* session = plr->GetSession())
            session->SendPacket(i_message);
}

void
MessageDeliverer::VisitObject(Player* plr)
{
    SendPacket(plr);
}

void
MessageDistDeliverer::VisitObject(Player* plr)
{
    if( !i_ownTeamOnly || (i_source.GetTypeId() == TYPEID_PLAYER && plr->GetTeam() == ((Player&)i_source).GetTeam()) )
    {
        SendPacket(plr);
    }
}

template<class T> void
ObjectUpdater::Visit(GridRefManager<T> &m)
{
    for(typename GridRefManager<T>::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        if(iter->GetSource()->IsInWorld())
            iter->GetSource()->Update(i_timeDiff);
    }
}

template void ObjectUpdater::Visit<GameObject>(GameObjectMapType &);
template void ObjectUpdater::Visit<DynamicObject>(DynamicObjectMapType &);

bool CannibalizeObjectCheck::operator()(Corpse* u)
{
    // ignore bones
    if(u->GetType()==CORPSE_BONES)
        return false;

    Player* owner = ObjectAccessor::FindPlayer(u->GetOwnerGUID());

    if( !owner || i_funit->IsFriendlyTo(owner))
        return false;

    if(i_funit->IsWithinDistInMap(u, i_range) )
        return true;

    return false;
}

void RespawnDo::operator()(Creature* u) const { u->Respawn(); }
void RespawnDo::operator()(GameObject* u) const { u->Respawn(); }

void FactionDo::operator()(Creature* u) const 
{
    if (u->GetEntry() == i_entry)
        u->SetFaction(i_faction);
}

bool CannibalizeObjectCheck::operator()(Player* u)
{
    if (i_funit->IsFriendlyTo(u) || u->IsAlive() || u->IsInFlight())
        return false;

    if (i_funit->IsWithinDistInMap(u, i_range))
        return true;

    return false;
}

bool CannibalizeObjectCheck::operator()(Creature* u)
{
    if (i_funit->IsFriendlyTo(u) || u->IsAlive() || u->IsInFlight() ||
        (u->GetCreatureTypeMask() & CREATURE_TYPEMASK_HUMANOID_OR_UNDEAD) == 0)
        return false;

    if (i_funit->IsWithinDistInMap(u, i_range))
        return true;

    return false;
}

bool AnyUnfriendlyUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (u->IsAlive() && i_obj->IsWithinDistInMap(u, i_range) && !i_funit->IsFriendlyTo(u))
        return true;
    else
        return false;
}

bool AnyUnfriendlyAoEAttackableUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (!u->IsAttackableByAOE())
        return false;

    // From 2.1.0 Feral Charge ignored traps, from 2.3.0 Intercept and Charge started to do so too
    if (u->HasUnitState(UNIT_STATE_CHARGING))
        return false;

    if (!i_obj->IsWithinDistInMap(u, i_range))
        return false;

    if (i_funit->IsFriendlyTo(u))
        return false;

    return true;
}

bool AnyFriendlyUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (u->IsAlive() && i_obj->IsWithinDistInMap(u, i_range) && i_funit->IsFriendlyTo(u) && (!i_playerOnly || u->GetTypeId() == TYPEID_PLAYER))
        return true;
    else
        return false;
}

bool AnyFriendlyUnitInObjectRangeCheckWithRangeReturned::operator()(Unit* u, float& range)
{
    range = i_obj->GetDistance(u);
    if (u->IsAlive() && i_range > range && i_funit->IsFriendlyTo(u) && (!i_playerOnly || u->GetTypeId() == TYPEID_PLAYER))
        return true;
    else
        return false;
}

bool NearestFriendlyUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (u->IsAlive() && i_obj->IsWithinDistInMap(u, i_range) && (!i_furthest || !i_obj->IsWithinDistInMap(u, i_minRange)) && i_funit->IsFriendlyTo(u) && (!i_playerOnly || u->GetTypeId() == TYPEID_PLAYER))
    {
        if (!i_furthest)
            i_range = i_obj->GetDistance(u);
        else
            i_minRange = i_obj->GetDistance(u);
        return true;
    }

    return false;
}

bool AnyUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (u->IsAlive() && i_obj->IsWithinDistInMap(u, i_range))
        return true;

    return false;
}

bool NearestAttackableUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (i_funit->CanAttack(u) == CAN_ATTACK_RESULT_OK && i_obj->IsWithinDistInMap(u, i_range) &&
        !i_funit->IsFriendlyTo(u) && u->IsVisibleForOrDetect(i_funit, false))
    {
        i_range = i_obj->GetDistance(u);        // use found unit range as new range limit for next check
        return true;
    }

    return false;
}

AnyAoETargetUnitInObjectRangeCheck::AnyAoETargetUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range)
    : i_obj(obj), i_funit(funit), i_range(range)
{
    Unit const* check = i_funit;
    Unit const* owner = i_funit->GetOwner();
    if (owner)
        check = owner;
    i_targetForPlayer = (check->GetTypeId() == TYPEID_PLAYER);
}

bool AnyAoETargetUnitInObjectRangeCheck::operator()(Unit* u)
{
    // Check contains checks for: live, non-selectable, non-attackable flags, flight check and GM check, ignore totems
    if (i_funit->CanAttack(u) != CAN_ATTACK_RESULT_OK)
        return false;
    if (u->GetTypeId() == TYPEID_UNIT && (u->ToCreature())->IsTotem())
        return false;

    if ((i_targetForPlayer ? !i_funit->IsFriendlyTo(u) : i_funit->IsHostileTo(u)) && i_obj->IsWithinDistInMap(u, i_range))
        return true;

    return false;
}

CallOfHelpCreatureInRangeDo::CallOfHelpCreatureInRangeDo(Unit* funit, Unit* enemy, float range)
    : i_funit(funit), i_enemy(enemy), i_range(range)
{ }
void CallOfHelpCreatureInRangeDo::operator()(Creature* u)
{
    if (u == i_funit)
        return;

    if (!u->CanAssistTo(i_funit, i_enemy, false))
        return;

    // too far
    if (!u->IsWithinDistInMap(i_funit, i_range))
        return;

    // only if see assisted creature's enemy
    if (!u->IsWithinLOSInMap(i_enemy))
        return;

    if (u->AI())
        u->AI()->AttackStart(i_enemy);
}

bool AnyDeadUnitCheck::operator()(Unit* u) { return !u->IsAlive(); }

bool AnyStealthedCheck::operator()(Unit* u) { return u->GetVisibility() == VISIBILITY_GROUP_STEALTH; }

bool NearestHostileUnitInAttackDistanceCheck::operator()(Unit* u)
{
    //is in range
    if (!m_creature->IsWithinDistInMap(u, m_range))
        return false;

    //check for furthest if set
    if (i_furthest)
    {
        if (m_creature->IsWithinDistInMap(u, m_minRange))
            return false;
        else
            m_minRange = m_creature->GetDistance(u);
    }
    else { //else we want the nearest, then set new max range
        m_range = m_creature->GetDistance(u);
    }

    if (m_force)
    {
        if (m_creature->CanAttack(u) != CAN_ATTACK_RESULT_OK)
            return false;
    }
    else
    {
        if (!m_creature->CanAggro(u))
            return false;
    }

    if (i_playerOnly && u->GetTypeId() != TYPEID_PLAYER)
        return false;

    return true;
}

bool AllWorldObjectsInRange::operator() (WorldObject* pGo)
{
    return m_pObject->IsWithinDistInMap(pGo, m_fRange, false);
}

bool MostHPMissingInRange::operator()(Unit* u)
{
    if (u->IsAlive() && u->IsInCombat() && !i_obj->IsHostileTo(u) && i_obj->IsWithinDistInMap(u, i_range) && u->GetMaxHealth() - u->GetHealth() > i_hp)
    {
        i_hp = u->GetMaxHealth() - u->GetHealth();
        return true;
    }
    return false;
}

bool FriendlyCCedInRange::operator()(Unit* u)
{
    if (u->IsAlive() && u->IsInCombat() && !i_obj->IsHostileTo(u) && i_obj->IsWithinDistInMap(u, i_range) &&
        (u->IsFeared() || u->IsCharmed() || u->IsFrozen() || u->HasUnitState(UNIT_STATE_STUNNED) || u->HasUnitState(UNIT_STATE_CONFUSED)))
    {
        return true;
    }
    return false;
}

bool FriendlyMissingBuffInRange::operator()(Unit* u)
{
    if (u->IsAlive() && u->IsInCombat() && /*!i_obj->IsHostileTo(u)*/ i_obj->IsFriendlyTo(u) && i_obj->IsWithinDistInMap(u, i_range) &&
        !(u->HasAuraEffect(i_spell, 0) || u->HasAuraEffect(i_spell, 1) || u->HasAuraEffect(i_spell, 2)))
    {
        return true;
    }
    return false;
}

bool FriendlyMissingBuffInRangeOutOfCombat::operator()(Unit* u)
{
    if (u->IsAlive() && !u->IsInCombat() && i_obj->IsFriendlyTo(u) && i_obj->IsWithinDistInMap(u, i_range) &&
        !(u->HasAuraEffect(i_spell)) && i_obj != u)
    {
        return true;
    }
    return false;
}

bool AllPlayersInRange::operator() (Player* u)
{
    if (i_object->IsWithinDistInMap(u, i_range))
        return true;

    return false;
}

bool AnyPlayerInObjectRangeCheck::operator()(Player* u)
{
    if (u->IsAlive() && !u->isSpectator() && i_obj->IsWithinDistInMap(u, i_range))
        return true;

    return false;
}

bool NearestPlayerInObjectRangeCheck::operator()(Player* u)
{
    if (u->IsAlive() == i_alive && !u->isSpectator() && i_obj.IsWithinDistInMap(u, i_range))
    {
        i_range = i_obj.GetDistance(u);         // use found unit range as new range limit for next check
        return true;
    }
    return false;
}

bool AllFriendlyCreaturesInGrid::operator() (Creature* u)
{
    if (u->IsAlive() && u->GetVisibility() == VISIBILITY_ON && u->IsFriendlyTo(pUnit))
        return true;

    return false;
}

bool NearestCreatureEntryWithLiveStateInObjectRangeCheck::operator()(Creature* u)
{
    if (u->GetEntry() == i_entry && u->IsAlive() == i_alive && i_obj.IsWithinDistInMap(u, i_range))
    {
        i_range = i_obj.GetDistance(u);         // use found unit range as new range limit for next check
        return true;
    }
    return false;
}

bool CreatureWithDbGUIDCheck::operator()(Creature* u)
{
    return u->GetDBTableGUIDLow() == i_lowguid;
}

bool AllCreaturesOfEntryInRange::operator() (Creature* u)
{
    if (u->GetEntry() == entry && pUnit->IsWithinDistInMap(u, range))
        return true;

    return false;
}

bool AllCreaturesInRange::operator() (Creature* u)
{
    if (pUnit->IsWithinDistInMap(u, range))
        return true;

    return false;
}

bool AllCreatures::operator() (Creature* u)
{
    return true;
}

bool AnyAssistCreatureInRangeCheck::operator()(Creature* u)
{
    if (u == i_funit)
        return false;

    if (!u->CanAssistTo(i_funit, i_enemy))
        return false;

    // too far
    if (!i_funit->IsWithinDistInMap(u, i_range, true))
        return false;

    // only if see assisted creature
    if (!i_funit->IsWithinLOSInMap(u))
        return false;

    return true;
}

bool NearestAssistCreatureInCreatureRangeCheck::operator()(Creature* u)
{
    if (u->GetFaction() == i_obj->GetFaction() && !u->IsInCombat() && !u->GetCharmerOrOwnerGUID() && u->IsHostileTo(i_enemy) && u->IsAlive() && i_obj->IsWithinDistInMap(u, i_range) && i_obj->IsWithinLOSInMap(u))
    {
        i_range = i_obj->GetDistance(u);         // use found unit range as new range limit for next check
        return true;
    }
    return false;
}

bool NearestGeneralizedAssistCreatureInCreatureRangeCheck::operator()(Creature* u)
{
    if (u->GetEntry() == i_entry && u->GetFaction() == i_faction && !u->IsInCombat() && !u->GetCharmerOrOwnerGUID() && u->IsAlive() && i_obj->IsWithinDistInMap(u, i_range) && i_obj->IsWithinLOSInMap(u))
    {
        i_range = i_obj->GetDistance(u);         // use found unit range as new range limit for next check
        return true;
    }
    return false;
}

bool GameObjectFocusCheck::operator()(GameObject* go) const
{
    if (go->GetGOInfo()->type != GAMEOBJECT_TYPE_SPELL_FOCUS)
        return false;

    if (go->GetGOInfo()->spellFocus.focusId != i_focusId)
        return false;

    float dist = (float)((go->GetGOInfo()->spellFocus.dist) / 2);

    return go->IsWithinDistInMap(i_unit, dist);
}

bool NearestGameObjectFishingHole::operator()(GameObject* go)
{
    if (go->GetGOInfo()->type == GAMEOBJECT_TYPE_FISHINGHOLE && go->isSpawned() && i_obj.IsWithinDistInMap(go, i_range) && i_obj.IsWithinDistInMap(go, go->GetGOInfo()->fishinghole.radius))
    {
        i_range = i_obj.GetDistance(go);
        return true;
    }
    return false;
}

bool AllGameObjectsWithEntryInGrid::operator() (GameObject* g)
{
    if (g->GetEntry() == entry)
        return true;

    return false;
}

bool AllGameObjectsWithEntryInRange::operator() (GameObject* pGo)
{
    if (pGo->GetEntry() == m_uiEntry && m_pObject->IsWithinDistInMap(pGo, m_fRange, false))
        return true;

    return false;
}

bool NearestGameObjectEntryInObjectRangeCheck::operator()(GameObject* go)
{
    if (go->GetEntry() == i_entry && i_obj.IsWithinDistInMap(go, i_range))
    {
        i_range = i_obj.GetDistance(go);        // use found GO range as new range limit for next check
        return true;
    }
    return false;
}

bool GameObjectWithDbGUIDCheck::operator()(GameObject const* go) const
{
    return go->GetDBTableGUIDLow() == i_db_guid;
}