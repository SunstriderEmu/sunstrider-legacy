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

        #ifdef TRINITY_DEBUG
        if((sLog->getLogFilter() & LOG_FILTER_VISIBILITY_CHANGES)==0)
            TC_LOG_DEBUG("FIXME","Object %u (Type: %u) is out of range (no in active cells set) now for player %u",GUID_LOPART(*itr),GuidHigh2TypeId(GUID_HIPART(*itr)),i_player.GetGUIDLow());
        #endif
    }

    if( i_data.HasData() )
    {
        // send create/outofrange packet to player (except player create updates that already sent using SendUpdateToPlayer)
        WorldPacket packet;
        i_data.BuildPacket(&packet);
        i_player.GetSession()->SendPacket(&packet);
        for (auto it : i_player.GetSharedVisionList())
            if(Player* p = ObjectAccessor::FindPlayer(it))
                p->GetSession()->SendPacket(&packet);

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

