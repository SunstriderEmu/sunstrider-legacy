
#include "PetAI.h"
#include "Errors.h"
#include "Pet.h"
#include "Player.h"
#include "DBCStores.h"
#include "Spell.h"
#include "ObjectAccessor.h"
#include "SpellMgr.h"
#include "Creature.h"
#include "World.h"
#include "Util.h"

int PetAI::Permissible(const Creature *creature)
{
    if (creature->HasUnitTypeMask(UNIT_MASK_CONTROLABLE_GUARDIAN))
    {
        if (reinterpret_cast<Guardian const*>(creature)->GetOwner()->GetTypeId() == TYPEID_PLAYER)
            return PERMIT_BASE_PROACTIVE;
        return PERMIT_BASE_REACTIVE;
    }

    return PERMIT_BASE_NO;
}

PetAI::PetAI(Creature *c) : CreatureAI(c), i_pet(*c), i_tracker(TIME_INTERVAL_LOOK), distanceCheckTimer(3000), _forceAttackBreakable(nullptr)
{
    m_AllySet.clear();
    UpdateAllies();
}

void PetAI::EnterEvadeMode(EvadeReason why)
{
}

void PetAI::ClearCharmInfoFlags()
{
    // Quick access to set all flags to FALSE

    CharmInfo* ci = me->GetCharmInfo();

    if (ci)
    {
        ci->SetIsAtStay(false);
        ci->SetIsCommandAttack(false);
        ci->SetIsCommandFollow(false);
        ci->SetIsFollowing(false);
        ci->SetIsReturning(false);
    }
}

bool PetAI::_needToStop() const
{
    // This is needed for charmed creatures, as once their target was reset other effects can trigger threat
    if(i_pet.IsCharmed() && i_pet.GetVictim() == i_pet.GetCharmer())
        return true;

    if (i_pet.GetOwner() && i_pet.GetOwner()->ToPlayer() && i_pet.ToPet() && i_pet.ToPet()->isControlled() && i_pet.GetVictim()->IsJustCCed() && i_pet.GetVictim()->GetEntry() != 10) // Training dummy exception
        return true;

    if (i_pet.IsNonMeleeSpellCast(false))
        return true;

    // dont allow pets to follow targets far away from owner
    if (Unit* owner = me->GetCharmerOrOwner())
        if (owner->GetExactDist(me) >= (owner->GetVisibilityRange() - 10.0f))
            return true;

    return i_pet.CanCreatureAttack(i_pet.GetVictim()) != CAN_ATTACK_RESULT_OK;
}

void PetAI::ResetMovement()
{
    Unit* owner = i_pet.GetCharmerOrOwner();

    if(owner && i_pet.IsAlive() && i_pet.GetCharmInfo() && i_pet.GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
    {
        i_pet.GetMotionMaster()->MoveFollow(owner,PET_FOLLOW_DIST,owner->GetFollowAngle());
    }
    else
    {
        i_pet.ClearUnitState(UNIT_STATE_FOLLOW);
        i_pet.GetMotionMaster()->Clear();
        i_pet.GetMotionMaster()->MoveIdle();
    }
}
void PetAI::_stopAttack()
{
    if( !i_pet.IsAlive() )
        i_pet.GetHostileRefManager().deleteReferences();

    me->AttackStop();
    me->InterruptNonMeleeSpells(false);
    me->GetCharmInfo()->SetIsCommandAttack(false);
    ClearCharmInfoFlags();
    HandleReturnMovement();
}

bool PetAI::CanAttack(Unit* target)
{
    // Evaluates wether a pet can attack a specific target based on CommandState, ReactState and other flags
    // IMPORTANT: The order in which things are checked is important, be careful if you add or remove checks

    // Hmmm...
    if (!target)
        return false;

    if (!target->IsAlive())
    {
        // if target is invalid, pet should evade automaticly
        // Clear target to prevent getting stuck on dead targets
        //me->AttackStop();
        //me->InterruptNonMeleeSpells(false);
        return false;
    }

    // Passive - passive pets can attack if told to
    if (me->HasReactState(REACT_PASSIVE))
        return me->GetCharmInfo()->IsCommandAttack();

    // CC - mobs under crowd control can be attacked if owner commanded
    if (target->HasBreakableByDamageCrowdControlAura())
        return me->GetCharmInfo()->IsCommandAttack();

    // Returning - pets ignore attacks only if owner clicked follow
    if (me->GetCharmInfo()->IsReturning())
        return !me->GetCharmInfo()->IsCommandFollow();

    // Stay - can attack if target is within range or commanded to
    if (me->GetCharmInfo()->HasCommandState(COMMAND_STAY))
        return (me->IsWithinMeleeRange(target) || me->GetCharmInfo()->IsCommandAttack());

    //  Pets attacking something (or chasing) should only switch targets if owner tells them to
    if (me->GetVictim() && me->GetVictim() != target)
    {
        // Check if our owner selected this target and clicked "attack"
        Unit* ownerTarget = NULL;
        if (Player* owner = me->GetCharmerOrOwner()->ToPlayer())
            ownerTarget = owner->GetSelectedUnit();
        else
            ownerTarget = me->GetCharmerOrOwner()->GetVictim();

        if (ownerTarget && me->GetCharmInfo()->IsCommandAttack())
            return (target->GetGUID() == ownerTarget->GetGUID());
    }

    // Follow
    if (me->GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
        return !me->GetCharmInfo()->IsReturning();

    // default, though we shouldn't ever get here
    return false;
}

void PetAI::HandleReturnMovement()
{
    // Handles moving the pet back to stay or owner

    // Prevent activating movement when under control of spells
    // such as "Eyes of the Beast"
    if (me->IsCharmed())
        return;

    if (me->GetCharmInfo()->HasCommandState(COMMAND_STAY))
    {
        if (!me->GetCharmInfo()->IsAtStay() && !me->GetCharmInfo()->IsReturning())
        {
            // Return to previous position where stay was clicked
            float x, y, z;

            me->GetCharmInfo()->GetStayPosition(x, y, z);
            ClearCharmInfoFlags();
            me->GetCharmInfo()->SetIsReturning(true);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MovePoint(ObjectGuid(me->GetGUID()).GetCounter(), x, y, z);
        }
    }
    else // COMMAND_FOLLOW
    {
        if (!me->GetCharmInfo()->IsFollowing() && !me->GetCharmInfo()->IsReturning())
        {
            ClearCharmInfoFlags();
            me->GetCharmInfo()->SetIsReturning(true);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveFollow(me->GetCharmerOrOwner(), PET_FOLLOW_DIST, me->GetFollowAngle());
        }
    }

    me->ClearInPetCombat();
}


void PetAI::AttackStart(Unit* target)
{
    // Overrides Unit::AttackStart to prevent pet from switching off its assigned target
    if (!target || target == me)
        return;

    if (me->GetVictim() && me->EnsureVictim()->IsAlive())
        return;

    _AttackStart(target);
}

void PetAI::_AttackStart(Unit* target)
{
    // Check all pet states to decide if we can attack this target
    if (!CanAttack(target))
        return;

    // Only chase if not commanded to stay or if stay but commanded to attack
    DoAttack(target, (!me->GetCharmInfo()->HasCommandState(COMMAND_STAY) || me->GetCharmInfo()->IsCommandAttack()));
}

void PetAI::MovementInform(uint32 moveType, uint32 data)
{
    // Receives notification when pet reaches stay or follow owner
    switch (moveType)
    {
    case POINT_MOTION_TYPE:
    {
        // Pet is returning to where stay was clicked. data should be
        // pet's GUIDLow since we set that as the waypoint ID
        if (data == ObjectGuid(me->GetGUID()).GetCounter() && me->GetCharmInfo()->IsReturning())
        {
            ClearCharmInfoFlags();
            me->GetCharmInfo()->SetIsAtStay(true);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveIdle();
        }
        break;
    }
    case FOLLOW_MOTION_TYPE:
    {
        // If data is owner's GUIDLow then we've reached follow point,
        // otherwise we're probably chasing a creature
        if (me->GetCharmerOrOwner() && me->GetCharmInfo() && data == ObjectGuid(me->GetCharmerOrOwner()->GetGUID()).GetCounter() && me->GetCharmInfo()->IsReturning())
        {
            ClearCharmInfoFlags();
            me->GetCharmInfo()->SetIsFollowing(true);
        }
        break;
    }
    default:
        break;
    }
}


void PetAI::OwnerAttackedBy(Unit* attacker)
{
    // Called when owner takes damage. This function helps keep pets from running off
    //  simply due to owner gaining aggro.

    if (!attacker || !me->IsAlive())
        return;

    // Passive pets don't do anything
    if (me->HasReactState(REACT_PASSIVE))
        return;

    // Prevent pet from disengaging from current target
    if (me->GetVictim() && me->EnsureVictim()->IsAlive())
        return;

    // Continue to evaluate and attack if necessary
    AttackStart(attacker);
}

void PetAI::ReceiveEmote(Player* player, uint32 emote)
{
#ifdef LICH_KING
    if (me->GetOwnerGUID() && me->GetOwnerGUID() == player->GetGUID())
        switch (emote)
        {
        case TEXT_EMOTE_COWER:
            if (me->IsPet() && me->ToPet()->IsPetGhoul())
                me->HandleEmoteCommand(/*EMOTE_ONESHOT_ROAR*/EMOTE_ONESHOT_OMNICAST_GHOUL);
            break;
        case TEXT_EMOTE_ANGRY:
            if (me->IsPet() && me->ToPet()->IsPetGhoul())
                me->HandleEmoteCommand(/*EMOTE_ONESHOT_COWER*/EMOTE_STATE_STUN);
            break;
        case TEXT_EMOTE_GLARE:
            if (me->IsPet() && me->ToPet()->IsPetGhoul())
                me->HandleEmoteCommand(EMOTE_STATE_STUN);
            break;
        case TEXT_EMOTE_SOOTHE:
            if (me->IsPet() && me->ToPet()->IsPetGhoul())
                me->HandleEmoteCommand(EMOTE_ONESHOT_OMNICAST_GHOUL);
            break;
        }
#endif
}

void PetAI::OwnerAttacked(Unit* target)
{
    // Called when owner attacks something. Allows defensive pets to know
    //  that they need to assist

    // Target might be NULL if called from spell with invalid cast targets
    if (!target || !me->IsAlive())
        return;

    // Passive pets don't do anything
    if (me->HasReactState(REACT_PASSIVE))
        return;

    // Prevent pet from disengaging from current target
    if (me->GetVictim() && me->EnsureVictim()->IsAlive())
        return;

    // Continue to evaluate and attack if necessary
    AttackStart(target);
}

void PetAI::ForceAttackBreakable(Unit const* target) 
{
    _forceAttackBreakable = target;
}

void PetAI::DamageDealt(Unit* /*victim*/, uint32& /*damage*/, DamageEffectType /*damageType*/)
{
    //reset force attack
    _forceAttackBreakable = nullptr;
}

void PetAI::DoAttack(Unit* target, bool chase)
{
    // Handles attack with or without chase and also resets flags
    // for next update / creature kill

    if (me->Attack(target, true))
    {
        // properly fix fake combat after pet is sent to attack
        if (Unit* owner = me->GetOwner())
            owner->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT);

        // Play sound to let the player know the pet is attacking something it picked on its own
        if (me->HasReactState(REACT_AGGRESSIVE) && !me->GetCharmInfo()->IsCommandAttack())
            me->SendPetAIReaction(/*me->GetGUID()*/);

        if (chase)
        {
            bool oldCmdAttack = me->GetCharmInfo()->IsCommandAttack(); // This needs to be reset after other flags are cleared
            ClearCharmInfoFlags();
            me->GetCharmInfo()->SetIsCommandAttack(oldCmdAttack); // For passive pets commanded to attack so they will use spells
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveChase(target, me->GetPetChaseDistance());
        }
        else // (Stay && ((Aggressive || Defensive) && In Melee Range)))
        {
            ClearCharmInfoFlags();
            me->GetCharmInfo()->SetIsAtStay(true);
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveIdle();
        }
    }
}

void PetAI::UpdateAI(const uint32 diff)
{
    if (!i_pet.IsAlive())
        return;

    Unit* owner = i_pet.GetCharmerOrOwner();

    if(m_updateAlliesTimer <= diff)
        // UpdateAllies self set update timer
        UpdateAllies();
    else
        m_updateAlliesTimer -= diff;

    // i_pet.GetVictim() can't be used for check in case stop fighting, i_pet.GetVictim() clear at Unit death etc.
    if( Unit const* victim = i_pet.GetVictim() )
    {
        // is only necessary to stop casting, the pet must not exit combat
        if (_forceAttackBreakable != victim
            && !me->GetCurrentSpell(CURRENT_CHANNELED_SPELL) && // ignore channeled spells (Pin, Seduction)
            me->EnsureVictim()->HasBreakableByDamageCrowdControlAura(me)
            )
        {
            me->InterruptNonMeleeSpells(false);
            return;
        }

        if( _needToStop() )
        {
            //TC_LOG_DEBUG("entities.pet","Pet AI stopped attacking [guid=%u]", i_pet.GetGUIDLow());
            _stopAttack();
            return;
        }

        // Check before attacking to prevent pets from leaving stay position
        if (me->GetCharmInfo()->HasCommandState(COMMAND_STAY))
        {
            if (me->GetCharmInfo()->IsCommandAttack() || (me->GetCharmInfo()->IsAtStay() && me->IsWithinMeleeRange(me->GetVictim())))
                DoMeleeAttackIfReady();
        }
        else
            DoMeleeAttackIfReady();
    }
    else
    {
        if(!i_pet.IsNonMeleeSpellCast(false))
        {
            if (me->HasReactState(REACT_AGGRESSIVE) || me->GetCharmInfo()->IsAtStay())
            {
                // Every update we need to check targets only in certain cases
                // Aggressive - Allow auto select if owner or pet don't have a target
                // Stay - Only pick from pet or owner targets / attackers so targets won't run by
                //   while chasing our owner. Don't do auto select.
                // All other cases (ie: defensive) - Targets are assigned by DamageTaken(), OwnerAttackedBy(), OwnerAttacked(), etc.
                Unit* nextTarget = SelectNextTarget(me->HasReactState(REACT_AGGRESSIVE));

                if (nextTarget)
                    AttackStart(nextTarget);
                else
                    HandleReturnMovement();
            }
            else
                HandleReturnMovement();
        }
    }

    if (i_pet.GetGlobalCooldown() == 0 && !i_pet.HasUnitState(UNIT_STATE_CASTING))
    {
        bool inCombat = me->GetVictim();
        m_targetSpellStore.clear();

        //Autocast
        for (uint8 i = 0; i < i_pet.GetPetAutoSpellSize(); i++)
        {
            uint32 spellID = i_pet.GetPetAutoSpellOnPos(i);
            if (!spellID)
                continue;

            SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellID);
            if (!spellInfo)
                continue;

            /* TC
            if (me->GetCharmInfo() && me->GetSpellHistory()->HasGlobalCooldown(spellInfo))
                continue;
                */

            // ignore some combinations of combat state and combat/noncombat spells
            if (!inCombat)
            {
                if (!spellInfo->IsPositive())
                    continue;
            }
            else
            {
                if (!spellInfo->CanBeUsedInCombat())
                    continue;
            }

            // auto spell = new Spell(&i_pet, spellInfo, false, 0);

            if (spellInfo->IsPositive())
            {
                if (spellInfo->CanBeUsedInCombat())
                {
                    // Check if we're in combat or commanded to attack
                    if (!me->IsInCombat() && !me->GetCharmInfo()->IsCommandAttack())
                        continue;
                }

                Spell* spell = new Spell(me, spellInfo, TRIGGERED_NONE);
                bool spellUsed = false;

                // Some spells can target enemy or friendly (DK Ghoul's Leap)
                // Check for enemy first (pet then owner)
                Unit* target = me->GetAttackerForHelper();
                if (!target && owner)
                    target = owner->GetAttackerForHelper();

                if (target)
                {
                    if (CanAttack(target) && spell->CanAutoCast(target))
                    {
                        m_targetSpellStore.push_back(std::make_pair(target, spell));
                        spellUsed = true;
                    }
                }

#ifdef LICH_KING
                if (spellInfo->HasEffect(SPELL_EFFECT_JUMP_DEST))
                {
                    if (!spellUsed)
                        delete spell;
                    continue; // Pets must only jump to target
                }
#endif

                // No enemy, check friendly
                if (!spellUsed)
                {
                    for (GuidSet::const_iterator tar = m_AllySet.begin(); tar != m_AllySet.end(); ++tar)
                    {
                        Unit* ally = ObjectAccessor::GetUnit(*me, *tar);

                        //only buff targets that are in combat, unless the spell can only be cast while out of combat
                        if (!ally)
                            continue;

                        if (spell->CanAutoCast(ally))
                        {
                            m_targetSpellStore.push_back(std::make_pair(ally, spell));
                            spellUsed = true;
                            break;
                        }
                    }
                }

                // No valid targets at all
                if (!spellUsed)
                    delete spell;
            }
            else if (me->GetVictim() && CanAttack(me->GetVictim()) && spellInfo->CanBeUsedInCombat())
            {
                Spell* spell = new Spell(me, spellInfo, TRIGGERED_NONE);
                if (spell->CanAutoCast(me->GetVictim()))
                    m_targetSpellStore.push_back(std::make_pair(me->GetVictim(), spell));
                else
                    delete spell;
            }
        }

        //found units to cast on to
        if(!m_targetSpellStore.empty())
        {
            TargetSpellList::iterator it = m_targetSpellStore.begin();
            std::advance(it, urand(0, m_targetSpellStore.size() - 1));

            Spell* spell = (*it).second;
            Unit*  target = (*it).first;

            m_targetSpellStore.erase(it);

            SpellCastTargets targets;
            targets.SetUnitTarget( target );

            if( !i_pet.HasInArc(M_PI, target) )
            {
                i_pet.SetInFront(target);
                if( target->GetTypeId() == TYPEID_PLAYER )
                    i_pet.SendUpdateToPlayer( target->ToPlayer() );

                if(owner && owner->GetTypeId() == TYPEID_PLAYER)
                    i_pet.SendUpdateToPlayer( owner->ToPlayer() );
            }

            if(i_pet.IsPet())
                ((Pet*)&i_pet)->CheckLearning(spell->m_spellInfo->Id);

            spell->prepare(&targets);
        }

        for (TargetSpellList::const_iterator itr = m_targetSpellStore.begin(); itr != m_targetSpellStore.end(); ++itr)
            delete itr->second;

        if(i_pet.IsPet() && ((Pet*)&i_pet)->getPetType() == MINI_PET)
        {
            Minipet_DistanceCheck(diff);
        }
    }
}

void PetAI::KilledUnit(Unit* victim)
{
    // Called from Unit::Kill() in case where pet or owner kills something
    // if owner killed this victim, pet may still be attacking something else
    if (me->GetVictim() && me->GetVictim() != victim)
        return;

    // Clear target just in case. May help problem where health / focus / mana
    // regen gets stuck. Also resets attack command.
    // Can't use _stopAttack() because that activates movement handlers and ignores
    // next target selection
    me->AttackStop();
    me->InterruptNonMeleeSpells(false);

    // Before returning to owner, see if there are more things to attack
    if (Unit* nextTarget = SelectNextTarget(false))
        AttackStart(nextTarget);
    else
        HandleReturnMovement(); // Return
}

void PetAI::UpdateAllies()
{
    Unit* owner = i_pet.GetCharmerOrOwner();
    Group *pGroup = nullptr;

    m_updateAlliesTimer = 10000;                            //update friendly targets every 10 seconds, lesser checks increase performance

    if(!owner)
        return;
    else if(owner->GetTypeId() == TYPEID_PLAYER)
        pGroup = (owner->ToPlayer())->GetGroup();

    //only pet and owner/not in group->ok
    if(m_AllySet.size() == 2 && !pGroup)
        return;
    //owner is in group; group members filled in already (no raid -> subgroupcount = whole count)
    if(pGroup && !pGroup->isRaidGroup() && m_AllySet.size() == (pGroup->GetMembersCount() + 2))
        return;

    m_AllySet.clear();
    m_AllySet.insert(i_pet.GetGUID());
    if(pGroup)                                              //add group
    {
        for(GroupReference *itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* Target = itr->GetSource();
            if(!Target || !pGroup->SameSubGroup(owner->ToPlayer(), Target))
                continue;

            if(Target->GetGUID() == owner->GetGUID())
                continue;

            m_AllySet.insert(Target->GetGUID());
        }
    }
    else                                                    //remove group
        m_AllySet.insert(owner->GetGUID());
}


Unit* PetAI::SelectNextTarget(bool allowAutoSelect) const
{
    // Provides next target selection after current target death.
    // This function should only be called internally by the AI
    // Targets are not evaluated here for being valid targets, that is done in _CanAttack()
    // The parameter: allowAutoSelect lets us disable aggressive pet auto targeting for certain situations

    // Passive pets don't do next target selection
    if (me->HasReactState(REACT_PASSIVE))
        return nullptr;

    // Check pet attackers first so we don't drag a bunch of targets to the owner
    if (Unit* myAttacker = me->GetAttackerForHelper())
        if (!myAttacker->HasBreakableByDamageCrowdControlAura())
            return myAttacker;

    // Not sure why we wouldn't have an owner but just in case...
    if (!me->GetCharmerOrOwner())
        return nullptr;

    // Check owner attackers
    if (Unit* ownerAttacker = me->GetCharmerOrOwner()->GetAttackerForHelper())
        if (!ownerAttacker->HasBreakableByDamageCrowdControlAura())
            return ownerAttacker;

    // Check owner victim
    // 3.0.2 - Pets now start attacking their owners victim in defensive mode as soon as the hunter does. Sunstrider: Let's keep this for now even for BC.
    if (Unit* ownerVictim = me->GetCharmerOrOwner()->GetVictim())
        return ownerVictim;

    // Neither pet or owner had a target and aggressive pets can pick any target
    // To prevent aggressive pets from chain selecting targets and running off, we
    //  only select a random target if certain conditions are met.
    if (me->HasReactState(REACT_AGGRESSIVE) && allowAutoSelect)
    {
        if (!me->GetCharmInfo()->IsReturning() || me->GetCharmInfo()->IsFollowing() || me->GetCharmInfo()->IsAtStay())
            if (Unit* nearTarget = me->SelectNearestHostileUnitInAggroRange(true))
                return nearTarget;
    }

    // Default - no valid targets
    return NULL;
}

void PetAI::Minipet_DistanceCheck(uint32 diff)
{
    Unit* owner = me->GetOwner();
    if (!owner)
        return;
    if (distanceCheckTimer <= diff)
    {
        distanceCheckTimer = 2000;
        float masterSpeed = owner->GetSpeed(MOVE_RUN);
        float masterSpeedRate = masterSpeed / baseMoveSpeed[MOVE_RUN];
        float masterDistance = me->GetDistance(owner);
        if(masterDistance >= 20)
        {
            me->SetSpeedRate(MOVE_RUN, masterSpeedRate * (masterDistance / 15.f));
        } else if (me->GetSpeed(MOVE_RUN) > masterSpeed) {
            me->SetSpeedRate(MOVE_RUN, masterSpeedRate);
        }
    } else distanceCheckTimer -= diff;
}