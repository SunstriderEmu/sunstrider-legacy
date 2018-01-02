
#include "GuardAI.h"
#include "Errors.h"
#include "Creature.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "World.h"

int GuardAI::Permissible(const Creature *creature)
{
    if( creature->IsGuard())
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_NO;
}

GuardAI::GuardAI(Creature *c) : CreatureAI(c), i_creature(*c), i_victimGuid(0), i_state(STATE_NORMAL), i_tracker(TIME_INTERVAL_LOOK)
{
}

bool GuardAI::CanSeeAlways(WorldObject const* obj)
{
    if (!obj->isType(TYPEMASK_UNIT))
        return false;

    ThreatContainer::StorageType threatList = me->GetThreatManager().getThreatList();
    for (ThreatContainer::StorageType::const_iterator itr = threatList.begin(); itr != threatList.end(); ++itr)
        if ((*itr)->getUnitGuid() == obj->GetGUID())
            return true;

    return false;
}

void GuardAI::MoveInLineOfSight(Unit *u)
{
    // Ignore Z for flying creatures
    if ( !i_creature.CanFly() && i_creature.GetDistanceZ(u) > CREATURE_Z_ATTACK_RANGE )
        return;

    if( !i_creature.GetVictim()
        && u->isInAccessiblePlaceFor(&i_creature) 
        && ( u->IsHostileToPlayers() || i_creature.IsHostileTo(u) ))
    {
        float attackRadius = i_creature.GetAggroRange(u);
        if(i_creature.IsWithinDistInMap(u,attackRadius))
            AttackStart(u);
    }
}

void GuardAI::EnterEvadeMode(EvadeReason why)
{
    if( !i_creature.IsAlive() )
    {
        i_creature.GetMotionMaster()->MoveIdle();

        i_state = STATE_NORMAL;

        i_victimGuid = 0;
        i_creature.CombatStop(true);
        i_creature.GetThreatManager().ClearAllThreat();
        return;
    }

    i_creature.RemoveAllAuras();
    i_creature.GetThreatManager().ClearAllThreat();
    i_victimGuid = 0;
    i_creature.CombatStop();
    i_state = STATE_NORMAL;

    // Remove TargetedMovementGenerator from MotionMaster stack list, and add HomeMovementGenerator instead
    if( i_creature.GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE )
        i_creature.GetMotionMaster()->MoveTargetedHome();

    //stealth detection! This aura also show the stealth detect icon on the npc when stealthed. This has infinite detect range, is this correct? Else there is also 37691 which has the visual but no stealth detect improvement
    i_creature.AddAura(8279, &i_creature);
}

void GuardAI::UpdateAI(const uint32 /*diff*/)
{
    if (i_creature.IsContestedGuard() && !i_creature.HasAuraEffect(18950,0)) //Invisibility and Stealth Detection
        i_creature.CastSpell(&i_creature, 18950, TRIGGERED_FULL_MASK);

    // update i_victimGuid if i_creature.GetVictim() !=0 and changed
    if(!UpdateVictim())
        return;

    i_victimGuid = i_creature.GetVictim()->GetGUID();

    if(i_creature.IsAttackReady())
    {
        if( i_creature.IsWithinMeleeRange(i_creature.GetVictim()))
        {
            i_creature.AttackerStateUpdate(i_creature.GetVictim());
            i_creature.ResetAttackTimer();
        }
    }
}

void GuardAI::JustDied(Unit *killer)
{
    if(Player* pkiller = killer->GetCharmerOrOwnerPlayerOrPlayerItself())
        i_creature.SendZoneUnderAttackMessage(pkiller);
}


//-----------------------------------------

#define GENERIC_CREATURE_COOLDOWN 5000

void guardAI::Reset()
{
    GlobalCooldown = 0;
    BuffTimer = 0;                                          //Rebuff as soon as we can
}

void guardAI::JustEngagedWith(Unit *who)
{
    if (me->GetEntry() == 15184)
    {
        switch(rand()%3)
        {
        case 0:
            me->Say("Taste blade, mongrel!", LANG_UNIVERSAL,nullptr);
            break;
        case 1:
            me->Say("Please tell me that you didn't just do what I think you just did. Please tell me that I'm not going to have to hurt you...", LANG_UNIVERSAL,nullptr);
            break;
        case 2:
            me->Say("As if we don't have enough problems, you go and create more!", LANG_UNIVERSAL,nullptr);
            break;
        }
    }

    if (SpellInfo const *spell = me->reachWithSpellAttack(who))
        DoCastSpell(who, spell);
}

void guardAI::JustDied(Unit *Killer)
{
    //Send Zone Under Attack message to the LocalDefense and WorldDefense Channels
    if (Player* pKiller = Killer->GetCharmerOrOwnerPlayerOrPlayerItself())
        me->SendZoneUnderAttackMessage(pKiller);
}

void guardAI::UpdateAI(const uint32 diff)
{
    //Always decrease our global cooldown first
    if (GlobalCooldown > diff)
        GlobalCooldown -= diff;
    else 
        GlobalCooldown = 0;

    //Buff timer (only buff when we are alive and not in combat
    if (me->IsAlive() && !me->IsInCombat())
    {
        if (BuffTimer < diff )
        {
            //Find a spell that targets friendly and applies an aura (these are generally buffs)
            SpellInfo const *info = SelectSpell(me, SPELL_SCHOOL_MASK_NONE, MECHANIC_NONE, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_AURA); // Not sure if replacing -1 by MECHANIC_NONE is safe...

            if (info && !GlobalCooldown)
            {
                //Cast the buff spell
                DoCastSpell(me, info);

                //Set our global cooldown
                GlobalCooldown = GENERIC_CREATURE_COOLDOWN;

                //Set our timer to 10 minutes before rebuff
                BuffTimer = 600000;
            }                                                   //Try agian in 30 seconds
            else 
            { 
                BuffTimer = 30000;
            }
        } else {
            BuffTimer -= diff; 
        }
    }

    //Return since we have no target
    if (!UpdateVictim())
        return;

    // Make sure our attack is ready and we arn't currently casting
    if( me->IsAttackReady() && !me->IsNonMeleeSpellCast(false))
    {
        //If we are within range melee the target
        if( me->IsWithinMeleeRange(me->GetVictim()))
        {
            bool Healing = false;
            SpellInfo const *info = nullptr;

            //Select a healing spell if less than 30% hp
            if (me->GetHealth()*100 / me->GetMaxHealth() < 30)
                info = SelectSpell(me, SPELL_SCHOOL_MASK_NONE, MECHANIC_NONE, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_HEALING);

            //No healing spell available, select a hostile spell
            if (info) 
                Healing = true;
            else 
                info = SelectSpell(me->GetVictim(), SPELL_SCHOOL_MASK_NONE, MECHANIC_NONE, SELECT_TARGET_ANY_ENEMY, 0, 0, 0, 0, SELECT_EFFECT_DONTCARE);

            //20% chance to replace our white hit with a spell
            if (info && rand() % 5 == 0 && !GlobalCooldown)
            {
                //Cast the spell
                if (Healing)
                    DoCastSpell(me, info);
                else 
                    DoCastSpell(me->GetVictim(), info);

                //Set our global cooldown
                GlobalCooldown = GENERIC_CREATURE_COOLDOWN;
            }
            else me->AttackerStateUpdate(me->GetVictim());

            me->ResetAttackTimer();
        }
    }
    else
    {
        //Only run this code if we arn't already casting
        if (!me->IsNonMeleeSpellCast(false))
        {
            bool Healing = false;
            SpellInfo const *info = nullptr;

            //Select a healing spell if less than 30% hp ONLY 33% of the time
            if (me->GetHealth()*100 / me->GetMaxHealth() < 30 && rand() % 3 == 0)
                info = SelectSpell(me, SPELL_SCHOOL_MASK_NONE, MECHANIC_NONE, SELECT_TARGET_ANY_FRIEND, 0, 0, 0, 0, SELECT_EFFECT_HEALING);

            //No healing spell available, See if we can cast a ranged spell (Range must be greater than ATTACK_DISTANCE)
            if (info) Healing = true;
            else info = SelectSpell(me->GetVictim(), SPELL_SCHOOL_MASK_NONE, MECHANIC_NONE, SELECT_TARGET_ANY_ENEMY, 0, 0, NOMINAL_MELEE_RANGE, 0, SELECT_EFFECT_DONTCARE);

            //Found a spell, check if we arn't on cooldown
            if (info && !GlobalCooldown)
            {
                //If we are currently moving stop us and set the movement generator
                if ((*me).GetMotionMaster()->GetCurrentMovementGeneratorType()!=IDLE_MOTION_TYPE)
                {
                    (*me).GetMotionMaster()->Clear(false);
                    (*me).GetMotionMaster()->MoveIdle();
                }

                //Cast spell
                if (Healing) 
                    DoCastSpell(me,info);
                else 
                    DoCastSpell(me->GetVictim(),info);

                //Set our global cooldown
                GlobalCooldown = GENERIC_CREATURE_COOLDOWN;

            }                                               //If no spells available and we arn't moving run to target
            else if ((*me).GetMotionMaster()->GetCurrentMovementGeneratorType()!=CHASE_MOTION_TYPE)
            {
                //Cancel our current spell and then mutate new movement generator
                me->InterruptNonMeleeSpells(false);
                (*me).GetMotionMaster()->Clear(false);
                (*me).GetMotionMaster()->MoveChase(me->GetVictim());
            }
        }
    }
}