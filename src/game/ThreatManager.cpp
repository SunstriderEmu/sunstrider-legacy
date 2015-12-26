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

#include "ThreatManager.h"
#include "Unit.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "CreatureAINew.h"
#include "Map.h"
#include "MapManager.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "UnitEvents.h"

//==============================================================
//================= ThreatCalcHelper ===========================
//==============================================================

// Apply modifiers from spells mods (SPELLMOD_THREAT) and global threat modifiers (SPELL_AURA_MOD_THREAT). Also brought to you with some dirty hacks.
float ThreatCalcHelper::calcThreat(Unit* pHatedUnit, Unit* pHatingUnit, float pThreat, SpellSchoolMask schoolMask, SpellInfo const *pThreatSpell)
{
    if(pThreatSpell)
    {
        if(pThreatSpell->HasAttribute(SPELL_ATTR1_NO_THREAT))
            return 0.0f;

        if( Player* modOwner = pHatedUnit->GetSpellModOwner() )
            modOwner->ApplySpellMod(pThreatSpell->Id, SPELLMOD_THREAT, pThreat);
        //TC_LOG_INFO("FIXME","calcThreat: pThreat after spell mod = %f",pThreat);

    }

    pHatedUnit->ApplyTotalThreatModifier(pThreat, schoolMask);
    //TC_LOG_INFO("FIXME","calcThreat: pThreat after total threat modifier = %f",pThreat);

    return pThreat;
}

bool ThreatCalcHelper::isValidProcess(Unit* hatedUnit, Unit* hatingUnit, SpellInfo const* threatSpell)
{
    //function deals with adding threat and adding players and pets into ThreatList
    //mobs, NPCs, guards have ThreatList and HateOfflineList
    //players and pets have only InHateListOf
    //HateOfflineList is used co contain unattackable victims (in-flight, in-water, GM etc.)

    if (!hatedUnit || !hatingUnit)
        return false;

    // not to self
    if (hatedUnit == hatingUnit)
        return false;

    // not to GM
    if (hatedUnit->GetTypeId() == TYPEID_PLAYER && 
            (  hatedUnit->ToPlayer()->IsGameMaster()
            || hatedUnit->ToPlayer()->isSpectator()
            || hatedUnit->ToPlayer()->GetTransForm() == FORM_SPIRITOFREDEMPTION
            )
        )
        return false;

    // not to dead and not for dead
    if (!hatedUnit->IsAlive() || !hatingUnit->IsAlive())
        return false;

    // not in same map or phase
    if (!hatedUnit->IsInMap(hatingUnit) || !hatedUnit->InSamePhase(hatingUnit))
        return false;

    // spell not causing threat
    if (threatSpell && threatSpell->HasAttribute(SPELL_ATTR1_NO_THREAT))
        return false;

    if(hatedUnit->isTotem())
        return false;

    ASSERT(hatingUnit->GetTypeId() == TYPEID_UNIT);

    return true;
}

//============================================================
//================= HostileReference ==========================
//============================================================

HostileReference::HostileReference(Unit* pUnit, ThreatManager *pThreatManager, float pThreat)
{
    iThreat = pThreat;
    iTempThreatModifyer = 0.0f;
    link(pUnit, pThreatManager);
    iUnitGuid = pUnit->GetGUID();
    iOnline = true;
    iAccessible = true;
}

//============================================================
// Tell our refTo (target) object that we have a link
void HostileReference::targetObjectBuildLink()
{
    getTarget()->addHatedBy(this);
}

//============================================================
// Tell our refTo (taget) object, that the link is cut
void HostileReference::targetObjectDestroyLink()
{
    getTarget()->removeHatedBy(this);
}

//============================================================
// Tell our refFrom (source) object, that the link is cut (Target destroyed)

void HostileReference::sourceObjectDestroyLink()
{
    setOnlineOfflineState(false);
}

//============================================================
// Inform the source, that the status of the reference changed

void HostileReference::fireStatusChanged(const ThreatRefStatusChangeEvent& pThreatRefStatusChangeEvent)
{
    if(GetSource())
        GetSource()->processThreatEvent(&pThreatRefStatusChangeEvent);
}

//============================================================

void HostileReference::addThreat(float pMod)
{
    iThreat += pMod;
    // the threat is changed. Source and target unit have to be availabe
    // if the link was cut before relink it again
    if(!isOnline())
        updateOnlineStatus();
    if(pMod != 0.0f)
        fireStatusChanged(ThreatRefStatusChangeEvent(UEV_THREAT_REF_THREAT_CHANGE, this, pMod));
    if(isValid() && pMod >= 0)
    {
        Unit* victim_owner = getTarget()->GetCharmerOrOwner();
        if(victim_owner && victim_owner->IsAlive())
            GetSource()->addThreat(victim_owner, 0.0f);     // create a threat to the owner of a pet, if the pet attacks
    }
}

//============================================================
// check, if source can reach target and set the status

void HostileReference::updateOnlineStatus()
{
    bool online = false;
    bool accessible = false;

    if(!isValid())
    {
        Unit* target = ObjectAccessor::GetUnit(*getSourceUnit(), getUnitGuid());
        if(target)
            link(target, GetSource());
    }
    // only check for online status if
    // ref is valid
    // target is no player or not gamemaster
    // target is not in flight
    if(isValid() &&
        ((getTarget()->GetTypeId() != TYPEID_PLAYER || !(getTarget()->ToPlayer())->IsGameMaster() || !(getTarget()->ToPlayer())->isSpectator()) ||
        !getTarget()->HasUnitState(UNIT_STATE_IN_FLIGHT)))
    {
        Creature* creature = (Creature* ) getSourceUnit();
        online = getTarget()->isInAccessiblePlaceFor(creature);
        if(!online)
        {
            if(creature->AI()->canReachByRangeAttack(getTarget()))
                online = true;                              // not accessible but stays online
        }
        else
            accessible = true;

    }
    setAccessibleState(accessible);
    setOnlineOfflineState(online);
}

//============================================================
// set the status and fire the event on status change

void HostileReference::setOnlineOfflineState(bool pIsOnline)
{
    if(iOnline != pIsOnline)
    {
        iOnline = pIsOnline;
        if(!iOnline)
            setAccessibleState(false);                      // if not online that not accessible as well
        fireStatusChanged(ThreatRefStatusChangeEvent(UEV_THREAT_REF_ONLINE_STATUS, this));
    }
}

//============================================================

void HostileReference::setAccessibleState(bool pIsAccessible)
{
    if(iAccessible != pIsAccessible)
    {
        iAccessible = pIsAccessible;
        fireStatusChanged(ThreatRefStatusChangeEvent(UEV_THREAT_REF_ASSECCIBLE_STATUS, this));
    }
}

//============================================================
// prepare the reference for deleting
// this is called be the target

void HostileReference::removeReference()
{
    invalidate();
    fireStatusChanged(ThreatRefStatusChangeEvent(UEV_THREAT_REF_REMOVE_FROM_LIST, this));
}

//============================================================

Unit* HostileReference::getSourceUnit()
{
    return (GetSource()->GetOwner());
}

//============================================================
//================ ThreatContainer ===========================
//============================================================

void ThreatContainer::clearReferences()
{
    for(std::list<HostileReference*>::iterator i = iThreatList.begin(); i != iThreatList.end(); ++i)
    {
        (*i)->unlink();
        delete (*i);
    }
    iThreatList.clear();
}

//============================================================
// Return the HostileReference of NULL, if not found
HostileReference* ThreatContainer::getReferenceByTarget(Unit* pVictim)
{
    HostileReference* result = NULL;
    uint64 guid = pVictim->GetGUID();
    for(std::list<HostileReference*>::iterator i = iThreatList.begin(); i != iThreatList.end(); ++i)
    {
        if((*i)->getUnitGuid() == guid)
        {
            result = (*i);
            break;
        }
    }

    return result;
}

//============================================================
// Add the threat, if we find the reference

HostileReference* ThreatContainer::addThreat(Unit* pVictim, float pThreat)
{
    HostileReference* ref = getReferenceByTarget(pVictim);
    if(ref)
        ref->addThreat(pThreat);
    return ref;
}

//============================================================

void ThreatContainer::modifyThreatPercent(Unit *pVictim, int32 pPercent)
{
    if(HostileReference* ref = getReferenceByTarget(pVictim))
    {
        if(pPercent < -100.0f)
        {
            ref->setThreat(0.0f);
        }
        else
            ref->addThreatPercent(pPercent);
    }
}

//============================================================

bool HostileReferenceSortPredicate(const HostileReference* lhs, const HostileReference* rhs)
{
    // std::list::sort ordering predicate must be: (Pred(x,y)&&Pred(y,x))==false
    return lhs->getThreat() > rhs->getThreat();             // reverse sorting
}

//============================================================
// Check if the list is dirty and sort if necessary

void ThreatContainer::update()
{
    if(iDirty && iThreatList.size() >1)
    {
        iThreatList.sort(HostileReferenceSortPredicate);
    }
    iDirty = false;
}

//============================================================
// return the next best victim
// could be the current victim

HostileReference* ThreatContainer::selectNextVictim(Creature* pAttacker, HostileReference* pCurrentVictim)
{
    HostileReference* currentRef = NULL;
    HostileReference* fallback = NULL;
    bool found = false;

    std::list<HostileReference*>::iterator lastRef = iThreatList.end();
    lastRef--;

    for(std::list<HostileReference*>::iterator iter = iThreatList.begin(); iter != iThreatList.end() && !found; ++iter)
    {
        currentRef = (*iter);

        Unit* target = currentRef->getTarget();
        assert(target);                                     // if the ref has status online the target must be there !
        
        if (!fallback)
            fallback = currentRef;

        // some units are preferred in comparison to others
        if(iter != lastRef && (target->IsImmunedToDamage(pAttacker->GetMeleeDamageSchoolMask(), false) ||
                target->HasUnitState(UNIT_STATE_CONFUSED) || (pAttacker->IsWorldBoss() && target->HasAuraEffect(30300))
                ) )
        {
            // current victim is a second choice target, so don't compare threat with it below
            if(currentRef == pCurrentVictim)
                pCurrentVictim = NULL;
            continue;
        }

        if(!pAttacker->IsOutOfThreatArea(target))           // skip non attackable currently targets
        {
            if(pCurrentVictim)                              // select 1.3/1.1 better target in comparison current target
            {
                // Select target in range if stationary combat
                if (pAttacker->IsCombatStationary()) {
                    /*(pAttacker->ToCreature() && !pAttacker->ToCreature()->AI()->canReachByRangeAttack(target))
                        found = false;
                    else {*/
                        found = true;
                        break;
                    //}
                }
                
                // list sorted and and we check current target, then this is best case
                if(pCurrentVictim == currentRef || currentRef->getThreat() <= 1.1f * pCurrentVictim->getThreat() )
                {
                    currentRef = pCurrentVictim;            // for second case
                    found = true;
                    break;
                }

                if( currentRef->getThreat() > 1.3f * pCurrentVictim->getThreat() ||
                    (currentRef->getThreat() > 1.1f * pCurrentVictim->getThreat() && pAttacker->IsWithinMeleeRange(target) ))
                {                                           //implement 110% threat rule for targets in melee range
                    found = true;                           //and 130% rule for targets in ranged distances
                    break;                                  //for selecting alive targets
                }
            }
            else                                            // select any
            {
                found = true;
                break;
            }
        }
    }
    if(!found)
        currentRef = NULL;
        
    // Prevent evade if rooted and no one in melee range
    if (!found && pAttacker->IsCombatStationary())
        currentRef = pCurrentVictim;
    if (!currentRef && pAttacker->IsCombatStationary()) // Happens when pulling a mob with a rooting aura, no pCurrentVictim
        currentRef = fallback;

    return currentRef;
}

//============================================================
//=================== ThreatManager ==========================
//============================================================

ThreatManager::ThreatManager(Unit* owner) : iCurrentVictim(NULL), iOwner(owner)
{
}

//============================================================

void ThreatManager::clearReferences()
{
    iThreatContainer.clearReferences();
    iThreatOfflineContainer.clearReferences();
    iCurrentVictim = NULL;
}

//============================================================

void ThreatManager::addThreat(Unit* victim, float threat, SpellSchoolMask schoolMask, SpellInfo const *threatSpell)
{
    if (!ThreatCalcHelper::isValidProcess(victim, GetOwner(), threatSpell))
        return;

    doAddThreat(victim, ThreatCalcHelper::calcThreat(victim, iOwner, threat, schoolMask, threatSpell));
}

void ThreatManager::doAddThreat(Unit* victim, float threat)
{
    uint32 redirectThreadPct = victim->GetRedirectThreatPercent();

    // must check > 0.0f, otherwise dead loop
    if (threat > 0.0f && redirectThreadPct)
    {
        if (Unit* redirectTarget = victim->GetRedirectThreatTarget())
        {
            float redirectThreat = CalculatePct(threat, redirectThreadPct);
            threat -= redirectThreat;
            _addThreat(redirectTarget, redirectThreat);
        }
    }

    _addThreat(victim, threat);
}

void ThreatManager::_addThreat(Unit *pVictim, float threat)
{
    if (iOwner->ToCreature() && iOwner->ToCreature()->getAI()) {
        if (threat >= 0)
            iOwner->ToCreature()->getAI()->onThreatAdd(pVictim, threat);
        else
            iOwner->ToCreature()->getAI()->onThreatRemove(pVictim, threat);
    }

    HostileReference* ref = iThreatContainer.addThreat(pVictim, threat);
    // Ref is not in the online refs, search the offline refs next
    if(!ref)
        ref = iThreatOfflineContainer.addThreat(pVictim, threat);

    if(!ref)                                                // there was no ref => create a new one
    {
                                                            // threat has to be 0 here
        HostileReference* hostilReference = new HostileReference(pVictim, this, 0);
        iThreatContainer.addReference(hostilReference);
        hostilReference->addThreat(threat);                 // now we add the real threat
        if(pVictim->GetTypeId() == TYPEID_PLAYER && ((pVictim->ToPlayer())->IsGameMaster() || (pVictim->ToPlayer())->isSpectator()))
            hostilReference->setOnlineOfflineState(false);  // GM is always offline
    }
}

//============================================================

void ThreatManager::modifyThreatPercent(Unit *pVictim, int32 pPercent)
{
    iThreatContainer.modifyThreatPercent(pVictim, pPercent);
}

//============================================================

Unit* ThreatManager::getHostilTarget()
{
    iThreatContainer.update();
    HostileReference* nextVictim = iThreatContainer.selectNextVictim(GetOwner()->ToCreature(), getCurrentVictim());
    setCurrentVictim(nextVictim);
    return getCurrentVictim() != NULL ? getCurrentVictim()->getTarget() : NULL;
}

//============================================================

float ThreatManager::getThreat(Unit *pVictim, bool pAlsoSearchOfflineList)
{
    float threat = 0.0f;
    HostileReference* ref = iThreatContainer.getReferenceByTarget(pVictim);
    if(!ref && pAlsoSearchOfflineList)
        ref = iThreatOfflineContainer.getReferenceByTarget(pVictim);
    if(ref)
        threat = ref->getThreat();
    return threat;
}

//============================================================

void ThreatManager::tauntApply(Unit* pTaunter)
{
    HostileReference* ref = iThreatContainer.getReferenceByTarget(pTaunter);
    if(getCurrentVictim() && ref && (ref->getThreat() < getCurrentVictim()->getThreat()))
    {
        if(ref->getTempThreatModifyer() == 0.0f)
                                                            // Ok, temp threat is unused
            ref->setTempThreat(getCurrentVictim()->getThreat());
    }
}

//============================================================

void ThreatManager::tauntFadeOut(Unit *pTaunter)
{
    HostileReference* ref = iThreatContainer.getReferenceByTarget(pTaunter);
    if(ref)
        ref->resetTempThreat();
}

//============================================================

void ThreatManager::detauntApply(Unit* pDetaunter)
{
    HostileReference* ref = iThreatContainer.getReferenceByTarget(pDetaunter);
    if (ref)
    {
        if (ref->getTempThreatModifyer() == 0.0f)
            ref->setTempThreat(0);
    }
}

//============================================================

void ThreatManager::detauntFadeOut(Unit *pDetaunter)
{
    HostileReference* ref = iThreatContainer.getReferenceByTarget(pDetaunter);
    if(ref)
        ref->resetTempThreat();
}

//============================================================

void ThreatManager::setCurrentVictim(HostileReference* pHostileReference)
{
#ifdef LICH_KING
    if (pHostileReference && pHostileReference != iCurrentVictim)
    {
        iOwner->SendChangeCurrentVictimOpcode(pHostileReference);
    }
#endif
    iCurrentVictim = pHostileReference;
}

//============================================================
// The hated unit is gone, dead or deleted
// return true, if the event is consumed

bool ThreatManager::processThreatEvent(const UnitBaseEvent* pUnitBaseEvent)
{
    bool consumed = false;

    ThreatRefStatusChangeEvent* threatRefStatusChangeEvent;
    HostileReference* hostilReference;

    threatRefStatusChangeEvent = (ThreatRefStatusChangeEvent*) pUnitBaseEvent;
    threatRefStatusChangeEvent->setThreatManager(this);     // now we can set the threat manager
    hostilReference = threatRefStatusChangeEvent->getReference();

    switch(pUnitBaseEvent->getType())
    {
        case UEV_THREAT_REF_THREAT_CHANGE:
            if((getCurrentVictim() == hostilReference && threatRefStatusChangeEvent->getFValue()<0.0f) ||
                (getCurrentVictim() != hostilReference && threatRefStatusChangeEvent->getFValue()>0.0f))
                setDirty(true);                             // the order in the threat list might have changed
            break;
        case UEV_THREAT_REF_ONLINE_STATUS:
            if(!hostilReference->isOnline())
            {
                if (hostilReference == getCurrentVictim())
                {
                    setCurrentVictim(NULL);
                    setDirty(true);
                }
                iThreatContainer.remove(hostilReference);
                iThreatOfflineContainer.addReference(hostilReference);
            }
            else
            {
                if(getCurrentVictim() && hostilReference->getThreat() > (1.1f * getCurrentVictim()->getThreat()))
                    setDirty(true);
                iThreatContainer.addReference(hostilReference);
                iThreatOfflineContainer.remove(hostilReference);
            }
            break;
        case UEV_THREAT_REF_REMOVE_FROM_LIST:
            if (hostilReference == getCurrentVictim())
            {
                setCurrentVictim(NULL);
                setDirty(true);
            }
            if(hostilReference->isOnline())
                iThreatContainer.remove(hostilReference);
            else
                iThreatOfflineContainer.remove(hostilReference);
            break;
    }
    return consumed;
}

