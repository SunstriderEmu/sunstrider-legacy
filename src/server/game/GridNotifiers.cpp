
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "Map.h"
#include "MapManager.h"
#include "Transport.h"
#include "ObjectAccessor.h"
#include "CellImpl.h"

using namespace Trinity;

void VisibleNotifier::SendToSelf()
{
    // at this moment i_clientGUIDs have guids that not iterate at grid level checks
    // but exist one case when this possible and object not out of range: transports
    if (Transport* transport = i_player.GetTransport())
    {
        for (Transport::PassengerSet::const_iterator itr = transport->GetPassengers().begin(); itr != transport->GetPassengers().end(); ++itr)
        {
            if (vis_guids.find((*itr)->GetGUID()) != vis_guids.end())
            {
                vis_guids.erase((*itr)->GetGUID());

                switch ((*itr)->GetTypeId())
                {
                case TYPEID_GAMEOBJECT:
                    i_player.UpdateVisibilityOf((*itr)->ToGameObject(), i_data, i_visibleNow);
                    break;
                case TYPEID_PLAYER:
                    i_player.UpdateVisibilityOf((*itr)->ToPlayer(), i_data, i_visibleNow);
                    if (!(*itr)->isNeedNotify(NOTIFY_VISIBILITY_CHANGED))
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

    for (auto it = vis_guids.begin(); it != vis_guids.end(); ++it)
    {
        i_player.m_clientGUIDs.erase(*it);
        i_data.AddOutOfRangeGUID(*it);

        if (it->IsPlayer())
        {
            Player* player = ObjectAccessor::FindPlayer(*it);
            if (player && !player->isNeedNotify(NOTIFY_VISIBILITY_CHANGED))
                player->UpdateVisibilityOf(&i_player);
        }
    }

    if (!i_data.HasData())
        return;

    WorldPacket packet;
    i_data.BuildPacket(&packet, false);
    i_player.GetSession()->SendPacket(&packet);

    for (std::set<Unit*>::const_iterator it = i_visibleNow.begin(); it != i_visibleNow.end(); ++it)
        i_player.SendInitialVisiblePackets(*it);
}

void VisibleChangesNotifier::Visit(CreatureMapType &m)
{
    for (CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        if (iter->GetSource()->HasSharedVision())
            for (SharedVisionList::const_iterator i = iter->GetSource()->GetSharedVisionList().begin();
                i != iter->GetSource()->GetSharedVisionList().end(); ++i)
                if ((*i)->m_seer == iter->GetSource())
                    (*i)->UpdateVisibilityOf(&i_object);
}

void VisibleChangesNotifier::Visit(DynamicObjectMapType &m)
{
    for (DynamicObjectMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        if (Unit* caster = iter->GetSource()->GetCaster())
            if (Player* player = caster->ToPlayer())
                if (player->m_seer == iter->GetSource())
                    player->UpdateVisibilityOf(&i_object);
}

inline void CreatureUnitRelocationWorker(Creature* c, Unit* u)
{
    if (!u->IsAlive() || !c->IsAlive() || c == u || u->IsInFlight())
        return;

    if (!c->HasUnitState(UNIT_STATE_SIGHTLESS))
    {
        if (c->IsAIEnabled() && c->CanSeeOrDetect(u, false, true))
            c->AI()->MoveInLineOfSight_Safe(u);
        else
            if (u->GetTypeId() == TYPEID_PLAYER && u->HasStealthAura() && c->IsAIEnabled() && /* c->CanSeeOrDetect(u, false, true, true) already in next check */ c->CanDoStealthAlert(u))
                c->StartStealthAlert(u); //c->AI()->TriggerAlert(u);
    }
}

void PlayerRelocationNotifier::Visit(PlayerMapType &m)
{
    for (PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        Player* player = iter->GetSource();

        vis_guids.erase(player->GetGUID());

        i_player.UpdateVisibilityOf(player, i_data, i_visibleNow);

        if (player->m_seer->isNeedNotify(NOTIFY_VISIBILITY_CHANGED))
            continue;

        player->UpdateVisibilityOf(&i_player);
    }
}

void PlayerRelocationNotifier::Visit(CreatureMapType &m)
{
    bool relocated_for_ai = (&i_player == i_player.m_seer);

    for (CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        Creature* c = iter->GetSource();

        vis_guids.erase(c->GetGUID());

        i_player.UpdateVisibilityOf(c, i_data, i_visibleNow);

        if (relocated_for_ai && !c->isNeedNotify(NOTIFY_VISIBILITY_CHANGED))
            CreatureUnitRelocationWorker(c, &i_player);
    }
}

void CreatureRelocationNotifier::Visit(PlayerMapType &m)
{
    for (PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        Player* player = iter->GetSource();

        if (!player->m_seer->isNeedNotify(NOTIFY_VISIBILITY_CHANGED))
            player->UpdateVisibilityOf(&i_creature);

        CreatureUnitRelocationWorker(&i_creature, player);
    }
}

void CreatureRelocationNotifier::Visit(CreatureMapType &m)
{
    if (!i_creature.IsAlive())
        return;

    for (CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        Creature* c = iter->GetSource();
        CreatureUnitRelocationWorker(&i_creature, c);

        if (!c->isNeedNotify(NOTIFY_VISIBILITY_CHANGED))
            CreatureUnitRelocationWorker(c, &i_creature);
    }
}

void DelayedUnitRelocation::Visit(CreatureMapType &m)
{
    for (CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        Creature* unit = iter->GetSource();
        if (!unit->isNeedNotify(NOTIFY_VISIBILITY_CHANGED))
            continue;

        CreatureRelocationNotifier relocate(*unit);

        TypeContainerVisitor<CreatureRelocationNotifier, WorldTypeMapContainer > c2world_relocation(relocate);
        TypeContainerVisitor<CreatureRelocationNotifier, GridTypeMapContainer >  c2grid_relocation(relocate);

        cell.Visit(p, c2world_relocation, i_map, *unit, i_radius);
        cell.Visit(p, c2grid_relocation, i_map, *unit, i_radius);
    }
}

void DelayedUnitRelocation::Visit(PlayerMapType &m)
{
    for (PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        Player* player = iter->GetSource();
        WorldObject const* viewPoint = player->m_seer;

        if (!viewPoint->isNeedNotify(NOTIFY_VISIBILITY_CHANGED))
            continue;

        if (player != viewPoint && !viewPoint->IsPositionValid())
            continue;

        CellCoord pair2(Trinity::ComputeCellCoord(viewPoint->GetPositionX(), viewPoint->GetPositionY()));
        Cell cell2(pair2);
        //cell.SetNoCreate(); need load cells around viewPoint or player, that's why its commented

        PlayerRelocationNotifier relocate(*player);
        TypeContainerVisitor<PlayerRelocationNotifier, WorldTypeMapContainer > c2world_relocation(relocate);
        TypeContainerVisitor<PlayerRelocationNotifier, GridTypeMapContainer >  c2grid_relocation(relocate);

        cell2.Visit(pair2, c2world_relocation, i_map, *viewPoint, i_radius);
        cell2.Visit(pair2, c2grid_relocation, i_map, *viewPoint, i_radius);

        relocate.SendToSelf();
    }
}

void AIRelocationNotifier::Visit(CreatureMapType &m)
{
    for (CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        Creature* c = iter->GetSource();
        CreatureUnitRelocationWorker(c, &i_unit);
        if (isCreature)
            CreatureUnitRelocationWorker((Creature*)&i_unit, c);
    }
}


void VisibleChangesNotifier::Visit(PlayerMapType &m)
{
    for (PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        if (iter->GetSource() == &i_object)
            continue;

        iter->GetSource()->UpdateVisibilityOf(&i_object);

        if (iter->GetSource()->HasSharedVision())
        {
            for (SharedVisionList::const_iterator i = iter->GetSource()->GetSharedVisionList().begin();
                i != iter->GetSource()->GetSharedVisionList().end(); ++i)
            {
                if ((*i)->m_seer == iter->GetSource())
                    (*i)->UpdateVisibilityOf(&i_object);
            }
        }
    }
}

void MessageDistDeliverer::Visit(PlayerMapType &m)
{
    for (PlayerMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        Player* target = iter->GetSource();
        if (!target->InSamePhase(i_phaseMask))
            continue;

        if (target->GetExactDist2dSq(i_source) > i_distSq)
            continue;

        // Send packet to all who are sharing the player's vision
        for (auto p : target->GetSharedVisionList())
            if (p->m_seer == target)
                    SendPacket(p);

        //send only if player viewpoint is on himself (?except for vehicles where the viewpoint is on the vehicle?)
        if (target->m_seer != target
#ifdef LICH_KING
            && !target->GetVehicle()
#endif
            )
            continue;

        SendPacket(target);
    }
}

void MessageDistDeliverer::Visit(CreatureMapType &m)
{
    for (CreatureMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        Creature* target = iter->GetSource();
        if (!target->HasSharedVision() || !target->InSamePhase(i_phaseMask))
            continue;

        if (target->GetExactDist2dSq(i_source) > i_distSq)
            continue;

        // Send packet to all who are sharing the creature's vision
        for (auto p : target->GetSharedVisionList())
            if (p->m_seer == target)
                SendPacket(p);
    }
}

void MessageDistDeliverer::Visit(DynamicObjectMapType &m)
{
    for (DynamicObjectMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        DynamicObject* target = iter->GetSource();
        if (!target->GetCasterGUID().IsPlayer() || !target->InSamePhase(i_phaseMask))
            continue;

        // Sunwell optimization: Check whether the dynobject allows to see through it
        /*
        if (!target->IsViewpoint())
            continue;
        */

        if (target->GetExactDist2dSq(i_source) > i_distSq)
            continue;

        // Send packet back to the caster if the caster has vision of dynamic object
        Player* caster = (Player*)target->GetCaster();
        if (caster && caster->m_seer == target)
            SendPacket(caster);
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
    if(u->GetType() == CORPSE_BONES)
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
    if (i_funit->IsFriendlyTo(u) || u->IsAlive() || u->HasAuraType(SPELL_AURA_GHOST))
        return false;

    if (!i_funit->IsWithinDistInMap(u, i_range))
        return false;

    return true;
}

bool CannibalizeObjectCheck::operator()(Creature* u)
{
    if (i_funit->IsFriendlyTo(u) || u->IsAlive()||
        (u->GetCreatureTypeMask() & CREATURE_TYPEMASK_HUMANOID_OR_UNDEAD) == 0)
        return false;

    if (!i_funit->IsWithinDistInMap(u, i_range))
        return false;

    return true;
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
    if (!u->IsAlive())
        return false;

    float searchRadius = i_range;
    if (i_incOwnRadius)
        searchRadius += i_obj->GetCombatReach();
    if (i_incTargetRadius)
        searchRadius += u->GetCombatReach();

    if (!u->IsInMap(i_obj) || !u->InSamePhase(i_obj) || !u->IsWithinDoubleVerticalCylinder(i_obj, searchRadius, searchRadius))
        return false;

    if (!i_funit->IsFriendlyTo(u) || i_funit->IsTotem()) //exclude totems from aoe
        return false;

    return !i_playerOnly || u->GetTypeId() == TYPEID_PLAYER;
}

bool AnyFriendlyUnitInObjectRangeCheckWithRangeReturned::operator()(Unit* u, float& range)
{
    range = i_obj->GetDistance(u);
    if (u->IsAlive() && i_range > range && i_funit->IsFriendlyTo(u) && (!i_playerOnly || u->GetTypeId() == TYPEID_PLAYER) && !u->IsTotem())
        return true;
    else
        return false;
}

bool NearestFriendlyUnitInObjectRangeCheck::operator()(Unit* u)
{
    if (u->IsAlive() && i_obj->IsWithinDistInMap(u, i_range) && (!i_furthest || !i_obj->IsWithinDistInMap(u, i_minRange)) && i_funit->IsFriendlyTo(u) && (!i_playerOnly || u->GetTypeId() == TYPEID_PLAYER) && !u->IsTotem())
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
    if (i_funit->IsValidAttackTarget(u) == CAN_ATTACK_RESULT_OK && i_obj->IsWithinDistInMap(u, i_range) &&
        (i_funit->IsInCombatWith(u) || i_funit->IsHostileTo(u)) && i_obj->CanSeeOrDetect(u))
    {
        i_range = i_obj->GetDistance(u);        // use found unit range as new range limit for next check
        return true;
    }

    return false;
}

AnyAoETargetUnitInObjectRangeCheck::AnyAoETargetUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range, SpellInfo const* spellInfo /*= nullptr*/, bool incOwnRadius /*= true*/, bool incTargetRadius /*= true*/)
    : i_obj(obj), i_funit(funit), _spellInfo(spellInfo), i_range(range), i_incOwnRadius(incOwnRadius), i_incTargetRadius(incTargetRadius)
{
}

bool AnyAoETargetUnitInObjectRangeCheck::operator()(Unit* u)
{
    // Check contains checks for: live, non-selectable, non-attackable flags, flight check and GM check, ignore totems
    if (u->GetTypeId() == TYPEID_UNIT && u->IsTotem())
        return false;

    if (_spellInfo && _spellInfo->HasAttribute(SPELL_ATTR3_ONLY_TARGET_PLAYERS) && u->GetTypeId() != TYPEID_PLAYER)
        return false;

    if (!i_funit->IsValidAttackTarget(u, _spellInfo))
        return false;

    float searchRadius = i_range;
    if (i_incOwnRadius)
        searchRadius += i_obj->GetCombatReach();
    if (i_incTargetRadius)
        searchRadius += u->GetCombatReach();

    return u->IsInMap(i_obj) && u->InSamePhase(i_obj) && u->IsWithinDoubleVerticalCylinder(i_obj, searchRadius, searchRadius);
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
    if (!u->IsWithinLOSInMap(i_enemy, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
        return;

    u->EngageWithTarget(i_enemy);
}

bool AnyDeadUnitCheck::operator()(Unit* u) { return !u->IsAlive(); }

bool NearestHostileUnitCheck::operator()(Unit* u)
{
    if (u == me)
        return false;

    if (i_playerOnly && u->GetTypeId() != TYPEID_PLAYER)
        return false;

    if (!me->IsWithinDistInMap(u, m_range))
        return false;

    if (!me->IsValidAttackTarget(u))
        return false;

    m_range = me->GetDistance(u);   // use found unit range as new range limit for next check
    return true;
}

bool NearestHostileUnitInAttackDistanceCheck::operator()(Unit* u)
{
    if (u == me)
        return false;

    if (i_playerOnly && u->GetTypeId() != TYPEID_PLAYER)
        return false;

    if (!me->CanSeeOrDetect(u))
        return false;

    if (m_force)
    {
        if (!me->IsValidAttackTarget(u))
            return false;
    }
    else  if (!me->CanAggro(u))
        return false;

    //is in range
    if (!me->IsWithinDistInMap(u, m_range))
        return false;

    //check for furthest if set
    if (i_furthest)
    {
        if (me->IsWithinDistInMap(u, m_minRange))
            return false;
        else
            m_minRange = me->GetDistance(u);
    }
    else { //else we want the nearest, then set new max range
        m_range = me->GetDistance(u);
    }

    return true;
}

bool AllWorldObjectsInRange::operator() (WorldObject* go)
{
    return m_pObject->IsWithinDist(go, m_fRange, false) && m_pObject->InSamePhase(go);
}

bool MostHPMissingInRange::operator()(Unit* u)
{
    if (u->IsAlive() 
        && u->IsInCombat() 
        && !i_obj->IsHostileTo(u) 
        && i_obj->IsWithinDistInMap(u, i_range) 
        && u->GetMaxHealth() - u->GetHealth() > i_hp
       )
    {
        i_hp = u->GetMaxHealth() - u->GetHealth();
        return true;
    }
    return false;
}

bool FriendlyCCedInRange::operator()(Unit* u)
{
    if (u->IsAlive() 
        && u->IsInCombat() 
        && !i_obj->IsHostileTo(u) 
        && i_obj->IsWithinDistInMap(u, i_range) 
        && (u->IsFeared() || u->IsCharmed() || u->IsFrozen() || u->HasUnitState(UNIT_STATE_STUNNED) || u->HasUnitState(UNIT_STATE_CONFUSED))
       )
    {
        return true;
    }
    return false;
}

bool FriendlyMissingBuffInRange::operator()(Unit* u)
{
    if (u->IsAlive() 
        && u->IsInCombat() 
        && /*!i_obj->IsHostileTo(u)*/ i_obj->IsFriendlyTo(u) 
        && i_obj->IsWithinDistInMap(u, i_range) 
        && !(u->HasAuraEffect(i_spell, 0) || u->HasAuraEffect(i_spell, 1) || u->HasAuraEffect(i_spell, 2))
       )
    {
        return true;
    }
    return false;
}

bool FriendlyMissingBuffInRangeOutOfCombat::operator()(Unit* u)
{
    if (u->IsAlive() 
        && !u->IsInCombat() 
        && i_obj->IsFriendlyTo(u) 
        && i_obj->IsWithinDistInMap(u, i_range) 
        && !(u->HasAura(i_spell)) 
        && i_obj != u
       )
    {
        return true;
    }
    return false;
}

bool AnyPlayerInObjectRangeCheck::operator()(Player* u)
{
    if (u->IsAlive() 
        && !u->isSpectator() 
        && i_obj->IsWithinDistInMap(u, i_range))
        return true;

    return false;
}

bool NearestPlayerInObjectRangeCheck::operator()(Player* u)
{
    if (u->IsAlive() == i_alive 
        && !u->isSpectator() 
        && i_obj.IsWithinDistInMap(u, i_range))
    {
        i_range = i_obj.GetDistance(u);         // use found unit range as new range limit for next check
        return true;
    }
    return false;
}

bool AllFriendlyCreaturesInGrid::operator() (Creature* u)
{
    if (u->IsAlive() 
        && u->IsVisible() 
        && u->IsFriendlyTo(pUnit))
        return true;

    return false;
}

bool NearestCreatureEntryWithLiveStateInObjectRangeCheck::operator()(Creature* u)
{
    if (u->GetEntry() == i_entry 
        && u->IsAlive() == i_alive 
        && i_obj.IsWithinDistInMap(u, i_range))
    {
        i_range = i_obj.GetDistance(u);         // use found unit range as new range limit for next check
        return true;
    }
    return false;
}

bool AllCreaturesOfEntryInRange::operator() (Unit* unit) const
{
    if ((!m_uiEntry || unit->GetEntry() == m_uiEntry) 
        && m_pObject->IsWithinDist(unit, m_fRange, false))
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
    if (!i_funit->IsWithinLOSInMap(u, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
        return false;

    return true;
}

bool NearestAssistCreatureInCreatureRangeCheck::operator()(Creature* u)
{
    if (u->GetFaction() == i_obj->GetFaction() 
        && !u->IsInCombat() 
        && !u->GetCharmerOrOwnerGUID() 
        && u->IsHostileTo(i_enemy) 
        && u->IsAlive() 
        && i_obj->IsWithinDistInMap(u, i_range) 
        && i_obj->IsWithinLOSInMap(u, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
    {
        i_range = i_obj->GetDistance(u);         // use found unit range as new range limit for next check
        return true;
    }
    return false;
}

bool NearestGeneralizedAssistCreatureInCreatureRangeCheck::operator()(Creature* u)
{
    if (u->GetEntry() == i_entry 
        && u->GetFaction() == i_faction 
        && !u->IsInCombat() 
        && !u->GetCharmerOrOwnerGUID() 
        && u->IsAlive() 
        && i_obj->IsWithinDistInMap(u, i_range) 
        && i_obj->IsWithinLOSInMap(u, LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::M2))
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

    if (go->GetGOInfo()->spellFocus.focusId != _focusId)
        return false;

    float dist = (float)((go->GetGOInfo()->spellFocus.dist) / 2);

    return go->IsWithinDistInMap(_caster, dist);
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
    if (pGo->GetEntry() == m_uiEntry 
        && m_pObject->IsWithinDistInMap(pGo, m_fRange, false))
        return true;

    return false;
}

bool AllGameObjectsInRange::operator() (GameObject* pGo)
{
    if (pGo->IsWithinDist3d(m_X, m_Y, m_Z, m_fRange))
        return true;

    return false;
}

bool NearestGameObjectEntryInObjectRangeCheck::operator()(GameObject* go)
{
    if (go->GetEntry() == i_entry 
        && i_obj.IsWithinDistInMap(go, i_range))
    {
        i_range = i_obj.GetDistance(go);        // use found GO range as new range limit for next check
        return true;
    }
    return false;
}

bool GameObjectWithSpawnIdCheck::operator()(GameObject const* go) const
{
    return go->GetSpawnId() == i_db_guid;
}
