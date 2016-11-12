/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

/* ScriptData
SDName: Boss_Prince_Malchezzar
SD%Complete: 100
SDComment:
SDCategory: Karazhan
EndScriptData */


#include "def_karazhan.h"

#define SAY_AGGRO           -1532091
#define SAY_AXE_TOSS1       -1532092
#define SAY_AXE_TOSS2       -1532093
#define SAY_SPECIAL1        -1532094
#define SAY_SPECIAL2        -1532095
#define SAY_SPECIAL3        -1532096
#define SAY_SLAY1           -1532097
#define SAY_SLAY2           -1532098
#define SAY_SLAY3           -1532099
#define SAY_SUMMON1         -1532100
#define SAY_SUMMON2         -1532101
#define SAY_DEATH           -1532102

// 19 Coordinates for Infernal spawns
struct InfernalPoint
{
    float x,y;
};

#define INFERNAL_Z  275.5

static InfernalPoint InfernalPoints[] =
{
    {-10922.8, -1985.2},
    {-10916.2, -1996.2},
    {-10932.2, -2008.1},
    {-10948.8, -2022.1},
    {-10958.7, -1997.7},
    {-10971.5, -1997.5},
    {-10990.8, -1995.1},
    {-10989.8, -1976.5},
    {-10971.6, -1973.0},
    {-10955.5, -1974.0},
    {-10939.6, -1969.8},
    {-10958.0, -1952.2},
    {-10941.7, -1954.8},
    {-10943.1, -1988.5},
    {-10948.8, -2005.1},
    {-10984.0, -2019.3},
    {-10932.8, -1979.6},
    {-10932.8, -1979.6},
    {-10935.7, -1996.0}
};

#define TOTAL_INFERNAL_POINTS 19

//Enfeeble is supposed to reduce hp to 1 and then heal player back to full when it ends
//Along with reducing healing and regen while enfeebled to 0%
//This spell effect will only reduce healing

#define SPELL_ENFEEBLE          30843                       //Enfeeble during phase 1 and 2
#define SPELL_ENFEEBLE_EFFECT   41624

#define SPELL_SHADOWNOVA        30852                       //Shadownova used during all phases
#define SPELL_SW_PAIN           30854                       //Shadow word pain during phase 1 and 3 (different targeting rules though)
#define SPELL_THRASH_PASSIVE    12787                       //Extra attack chance during phase 2
#define SPELL_SUNDER_ARMOR      30901                       //Sunder armor during phase 2
#define SPELL_THRASH_AURA       3417                        //Passive proc chance for thrash
#define SPELL_EQUIP_AXES        30857                       //Visual for axe equiping
#define SPELL_AMPLIFY_DAMAGE    12738                       //Amplifiy during phase 3
#define SPELL_HELLFIRE          30859                       //Infenals' hellfire aura
#define NETHERSPITE_INFERNAL    17646                       //The netherspite infernal creature
#define MALCHEZARS_AXE          17650                       //Malchezar's axes (creatures), summoned during phase 3

#define INFERNAL_MODEL_INVISIBLE 11686                      //Infernal Effects
#define SPELL_INFERNAL_RELAY          30834                 //visual effect + dummy effect on target 17646
#define SPELL_INFERNAL_RELAY_SUMMON   30836                 //summon creature (not used here)
#define SPELL_INFERNAL_RELAY2         30835                 //also visual + script effect on nearby entry (not used here)
#define SPELL_INFERNAL_RELAY_TRIGGER  53108                 //custom spell, target max 1 CREATURE_INFERNAL_RELAY and do script effect
#define SPELL_INFERAL_LAND_VISUAL     24207                 // Not right id but same visual
#define CREATURE_INFERNAL_RELAY  17645                      //rain down inferno from this one
#define CREATURE_INFERNAL        17646

#define AXE_EQUIP_MODEL          40066                      //Axes model

enum Messages
{
    MESSAGE_GET_RANDOM_POSITION,
    MESSAGE_SET_POINT,
    MESSAGE_FREE_POINT,
    MESSAGE_ADD_INFERNAL,
    MESSAGE_REMOVE_INFERNAL,
};

struct infernal_relayAI : public ScriptedAI
{
    infernal_relayAI(Creature *c) : ScriptedAI(c) 
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
        me->SetReactState(REACT_PASSIVE);
    }

    InstanceScript* pInstance;

    void SummonInfernal()
    {
        Creature* malch;
        float posX,posY,posZ;
        InfernalPoint* point = nullptr;
        if(me->GetMapId() != 532)
        {
            me->GetRandomPoint(me, 60, posX, posY, posZ);
        }
        else
        {
            if(!pInstance) return;
            malch = ObjectAccessor::GetCreature(*me,pInstance->GetData64(DATA_MALCHEZAAR));
            if(!malch)
            {
                TC_LOG_ERROR("scripts","infernal_relayAI : could not find malchezaar");
                return;
            }
            point = (InfernalPoint*)malch->AI()->message(MESSAGE_GET_RANDOM_POSITION,0);
            if(!point) 
            {
                TC_LOG_ERROR("scripts","infernal_relayAI : Malchezaar did not return any point");
                return;
            }

            posX = point->x;
            posY = point->y;
            posZ = INFERNAL_Z;
        }

        //creature is faction 35 at spawn
        if (Creature *Infernal = me->SummonCreature(NETHERSPITE_INFERNAL, posX, posY, posZ, 0, TEMPSUMMON_TIMED_DESPAWN, 180000))
        {
            //creature is invisible until SPELL_INFERNAL_RELAY touch it
            Infernal->SetUInt32Value(UNIT_FIELD_DISPLAYID, INFERNAL_MODEL_INVISIBLE);
            Infernal->AI()->message(MESSAGE_SET_POINT,(uint64)point);
            DoCast(Infernal, SPELL_INFERNAL_RELAY);
            if(malch)
                malch->AI()->message(MESSAGE_ADD_INFERNAL,Infernal->GetGUID());
        }
    }

    void SpellHit(Unit *who, const SpellInfo *spell)
    override {
        //this spell is cast by Malchezaar whenever he wants a new inferno
        if(spell->Id == SPELL_INFERNAL_RELAY_TRIGGER)
        {
            SummonInfernal();
            me->SetUInt32Value(UNIT_FIELD_DISPLAYID, me->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
            me->CastSpell(me,SPELL_INFERAL_LAND_VISUAL,true);
        }
    }
};

//---------Infernal code first
struct netherspite_infernalAI : public ScriptedAI
{
    netherspite_infernalAI(Creature *c) : ScriptedAI(c) ,
        malchezaar(0), HellfireTimer(0), CleanupTimer(0), point(nullptr) 
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    uint32 HellfireTimer;
    uint32 CleanupTimer;
    uint64 malchezaar;
    InfernalPoint* point;
    InstanceScript* pInstance;

    uint64 message(uint32 id, uint64 data) 
    override { 
        if(id == MESSAGE_SET_POINT)
            point = (InfernalPoint*)data;
        return 0;
    }

    void Reset()
    override {
        me->SetStunned(true);
        me->AI()->SetCombatMovementAllowed(false);
    }

    void Cleanup()
    {
        if(!pInstance) return;
        if(Creature* malch = ObjectAccessor::GetCreature(*me,pInstance->GetData64(DATA_MALCHEZAAR)))
        {
            malch->AI()->message(MESSAGE_FREE_POINT,(uint64)point);
            malch->AI()->message(MESSAGE_REMOVE_INFERNAL,me->GetGUID());
        }
    }

    void UpdateAI(const uint32 diff)
    override {
        if(HellfireTimer)
        {
            if(HellfireTimer <= diff)
            {
                me->SetFaction(14);
                DoCast(me, SPELL_HELLFIRE);
                HellfireTimer = 0;
            }
            else 
            {
                HellfireTimer -= diff;
            }
        }

        if(CleanupTimer)
        {
            if(CleanupTimer <= diff)
            {
                Cleanup();
                CleanupTimer = 0;
            } else { CleanupTimer -= diff; }
        }
    }

    void KilledUnit(Unit *who)
    override {
        Unit *pMalchezaar = ObjectAccessor::GetUnit(*me, malchezaar);
        if(pMalchezaar)
            (pMalchezaar->ToCreature())->AI()->KilledUnit(who);
    }

    void SpellHit(Unit *who, const SpellInfo *spell)
    override {
        if(spell->Id == SPELL_INFERNAL_RELAY)
        {
            me->SetUInt32Value(UNIT_FIELD_DISPLAYID, me->GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID));
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            HellfireTimer = 4000;
            CleanupTimer = 170000;
        }
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    override {
        if(done_by->GetGUID() != malchezaar)
            damage = 0;
    }
};

struct boss_malchezaarAI : public ScriptedAI
{
    boss_malchezaarAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript *pInstance;
    uint32 EnfeebleTimer;
    uint32 EnfeebleResetTimer;
    uint32 ShadowNovaTimer;
    uint32 SWPainTimer;
    uint32 SunderArmorTimer;
    uint32 AmplifyDamageTimer;
    uint32 InfernalTimer;
    uint32 AxesTargetSwitchTimer;
    uint32 InfernalCleanupTimer;

    std::vector<uint64> infernals; //spawned infernal guids
    std::vector<InfernalPoint*> positions; //free infernal positions

    uint64 axes[2];
    uint64 enfeeble_targets[5];
    uint64 enfeeble_health[5];

    uint32 phase;

    uint64 message(uint32 id, uint64 data) 
    override { 
        //return a random point and erase it from positions
        switch(id)
        {
        case MESSAGE_GET_RANDOM_POSITION:
            {
            InfernalPoint *point = nullptr;
            auto itr = positions.begin()+rand()%positions.size();
            point = *itr;
            positions.erase(itr);
            return (uint64)point;
            }
        case MESSAGE_FREE_POINT:
            positions.push_back((InfernalPoint*)data);
            break;
        case MESSAGE_ADD_INFERNAL:
            infernals.push_back(data);
            break;
        case MESSAGE_REMOVE_INFERNAL:
            for(auto itr = infernals.begin(); itr!= infernals.end(); ++itr)
            if(*itr == data)
            {
                infernals.erase(itr);
                break;
            }
            break;
        }
        return 0;
    }

    void Reset()
    override {
        AxesCleanup();
        ClearWeapons();
        InfernalCleanup();
        positions.clear();

        for(uint64 & enfeeble_target : enfeeble_targets)
            enfeeble_target = 0;

        EnfeebleTimer = 30000;
        EnfeebleResetTimer = 38000;
        ShadowNovaTimer = 35000;
        SWPainTimer = 20000;
        AmplifyDamageTimer = 10000;
        InfernalTimer = 25000;
        InfernalCleanupTimer = 47000;
        AxesTargetSwitchTimer = 7500 + rand()%12500;
        phase = 1;

        for(auto & InfernalPoint : InfernalPoints)
            positions.push_back(&InfernalPoint);

        if(pInstance)
        {
            GameObject* Door = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GAMEOBJECT_NETHER_DOOR));
            if(Door)
            {
                Door->SetGoState(GO_STATE_ACTIVE);
            }

            pInstance->SetData(DATA_MALCHEZZAR_EVENT, NOT_STARTED);
        }
    }

    void KilledUnit(Unit *victim)
    override {
        switch(rand()%3)
        {
        case 0: DoScriptText(SAY_SLAY1, me); break;
        case 1: DoScriptText(SAY_SLAY2, me); break;
        case 2: DoScriptText(SAY_SLAY3, me); break;
        }
    }

    void JustDied(Unit *victim)
    override {
        DoScriptText(SAY_DEATH, me);

        AxesCleanup();
        ClearWeapons();
        InfernalCleanup();
        positions.clear();

        for(auto & InfernalPoint : InfernalPoints)
            positions.push_back(&InfernalPoint);

        if(pInstance)
        {
            GameObject* Door = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GAMEOBJECT_NETHER_DOOR));
            if(Door)
            {
                Door->SetGoState(GO_STATE_ACTIVE);
            }

            pInstance->SetData(DATA_MALCHEZZAR_EVENT, DONE);
        }
    }

    void EnterCombat(Unit *who)
    override {
        DoScriptText(SAY_AGGRO, me);

        if(pInstance)
        {
            GameObject* Door = GameObject::GetGameObject((*me),pInstance->GetData64(DATA_GAMEOBJECT_NETHER_DOOR));
            if(Door)
            {
                Door->SetGoState(GO_STATE_READY);
            }

            pInstance->SetData(DATA_MALCHEZZAR_EVENT, IN_PROGRESS);
        }
    }

    void InfernalCleanup()
    {
        //Infernal Cleanup
        for(uint64 & infernal : infernals)
        {
            Unit *pInfernal = ObjectAccessor::GetUnit(*me, infernal);
            if(pInfernal && pInfernal->IsAlive())
            {
                pInfernal->SetVisibility(VISIBILITY_OFF);
                pInfernal->SetDeathState(JUST_DIED);
            }
        }
        infernals.clear();
    }

    void AxesCleanup()
    {
        for(uint64 & i : axes)
        {
            Unit *axe = ObjectAccessor::GetUnit(*me, i);
            if(axe && axe->IsAlive())
                axe->DealDamage(axe, axe->GetHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            i = 0;
        }
    }

    void ClearWeapons()
    {
        me->SetWeapon(WEAPON_SLOT_MAINHAND, 0, ITEM_SUBCLASS_WEAPON_AXE, INVTYPE_WEAPON);
        me->SetWeapon(WEAPON_SLOT_OFFHAND, 0, ITEM_SUBCLASS_WEAPON_AXE, INVTYPE_WEAPON);

        //damage
        /*FIXMEDMG
        const CreatureTemplate *cinfo = me->GetCreatureTemplate();
        me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, cinfo->mindmg);
        me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, cinfo->maxdmg);
        me->UpdateDamagePhysical(BASE_ATTACK);
        */
    }

    void EnfeebleHealthEffect()
    {
        const SpellInfo *info = sSpellMgr->GetSpellInfo(SPELL_ENFEEBLE_EFFECT);
        if(!info)
            return;

        std::list<HostileReference *> t_list = me->getThreatManager().getThreatList();
        std::vector<Unit *> targets;

        if(!t_list.size())
            return;

        //begin + 1 , so we don't target the one with the highest threat
        auto itr = t_list.begin();
        std::advance(itr, 1);
        for( ; itr!= t_list.end(); ++itr)                   //store the threat list in a different container
        {
            Unit *target = ObjectAccessor::GetUnit(*me, (*itr)->getUnitGuid());
                                                            //only on alive players
            if(target && target->IsAlive() && target->GetTypeId() == TYPEID_PLAYER && target != me->GetVictim() )
                targets.push_back( target);
        }

        //cut down to size if we have more than 5 targets
        while(targets.size() > 5)
            targets.erase(targets.begin()+rand()%targets.size());

        int i = 0;
        for(auto itr = targets.begin(); itr!= targets.end(); ++itr, ++i)
        {
            Unit *target = *itr;
            if(target)
            {
                enfeeble_targets[i] = target->GetGUID();
                enfeeble_health[i] = target->GetHealth();

                //target->CastSpell(target, SPELL_ENFEEBLE, true, 0, 0, me->GetGUID());
                target->AddAura(SPELL_ENFEEBLE, target);
                target->SetHealth(1);
            }
        }

    }

    void EnfeebleResetHealth()
    {
        for(int i = 0; i < 5; ++i)
        {
            Unit *target = ObjectAccessor::GetUnit(*me, enfeeble_targets[i]);
            if(target && target->IsAlive())
                target->SetHealth(enfeeble_health[i]);
            enfeeble_targets[i] = 0;
            enfeeble_health[i] = 0;
        }
    }

    void UpdateAI(const uint32 diff)
    override {
        if (!UpdateVictim() )
            return;

        if(EnfeebleResetTimer)
        {
            if(EnfeebleResetTimer <= diff)                  //Let's not forget to reset that
            {
                EnfeebleResetHealth();
                EnfeebleResetTimer=0;
            } else { 
                EnfeebleResetTimer -= diff; 
            }
        }
        if(me->HasUnitState(UNIT_STATE_STUNNED))     //While shifting to phase 2 malchezaar stuns himself
            return;

        if(me->GetUInt64Value(UNIT_FIELD_TARGET)!=me->GetVictim()->GetGUID())
            me->SetUInt64Value(UNIT_FIELD_TARGET, me->GetVictim()->GetGUID());

        if(phase == 1)
        {
            if( (me->GetHealth()*100) / me->GetMaxHealth() < 60)
            {
                me->InterruptNonMeleeSpells(false);

                phase = 2;

                //animation
                DoCast(me, SPELL_EQUIP_AXES);

                //text
                DoScriptText(SAY_AXE_TOSS1, me);

                //passive thrash aura
                me->CastSpell(me, SPELL_THRASH_AURA, true);

                //weapons
                me->SetWeapon(WEAPON_SLOT_MAINHAND, AXE_EQUIP_MODEL, ITEM_SUBCLASS_WEAPON_AXE, INVTYPE_WEAPON);
                me->SetWeapon(WEAPON_SLOT_OFFHAND,  AXE_EQUIP_MODEL, ITEM_SUBCLASS_WEAPON_AXE, INVTYPE_WEAPON);
                
                //damage
                /* FIXMEDMG 
                const CreatureTemplate *cinfo = me->GetCreatureTemplate();
                me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, 2*cinfo->mindmg);
                me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, 2*cinfo->maxdmg);
                me->UpdateDamagePhysical(BASE_ATTACK);

                me->SetBaseWeaponDamage(OFF_ATTACK, MINDAMAGE, cinfo->mindmg);
                me->SetBaseWeaponDamage(OFF_ATTACK, MAXDAMAGE, cinfo->maxdmg);
                //Sigh, updating only works on main attack , do it manually ....
                me->SetFloatValue(UNIT_FIELD_MINOFFHANDDAMAGE, cinfo->mindmg);
                me->SetFloatValue(UNIT_FIELD_MAXOFFHANDDAMAGE, cinfo->maxdmg); */

                me->SetAttackTime(OFF_ATTACK, (me->GetAttackTime(BASE_ATTACK)*150)/100);
            }
        }
        else if(phase == 2)
        {
            if(me->IsBelowHPPercent(30.0f))
            {
                InfernalTimer = 15000;

                phase = 3;

                ClearWeapons();

                //remove thrash
                me->RemoveAurasDueToSpell(SPELL_THRASH_AURA);

                DoScriptText(SAY_AXE_TOSS2, me);

                Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 0);
                for(uint64 & i : axes)
                {
                    Creature *axe = me->SummonCreature(MALCHEZARS_AXE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1000);
                    if(axe)
                    {
                        //no displayid, already on creaturemodel, but set correct weapon info
                        axe->SetWeapon(WEAPON_SLOT_MAINHAND, 0, ITEM_SUBCLASS_WEAPON_AXE, INVTYPE_WEAPON);
                        axe->SetWeapon(WEAPON_SLOT_OFFHAND,  0, ITEM_SUBCLASS_WEAPON_AXE, INVTYPE_WEAPON);

                        axe->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        axe->SetFaction(me->GetFaction());
                        i = axe->GetGUID();
                        if(target)
                        {
                            axe->AI()->AttackStart(target);
                            axe->AddThreat(target, 10000000.0f);
                        }
                    }
                }

                if(ShadowNovaTimer > 35000)
                    ShadowNovaTimer = EnfeebleTimer + 5000;

                return;
            }

            if(SunderArmorTimer < diff)
            {
                DoCast(me->GetVictim(), SPELL_SUNDER_ARMOR);
                SunderArmorTimer = 15000;

            }else SunderArmorTimer -= diff;
        }
        else
        {
            if(AxesTargetSwitchTimer < diff)
            {
                AxesTargetSwitchTimer = 7500 + rand()%12500 ;

                Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 0);
                if(target)
                {
                    for(uint64 i : axes)
                    {
                        Unit *axe = ObjectAccessor::GetUnit(*me, i);
                        if(axe)
                        {
                            float threat = 1000000.0f;
                            if(axe->GetVictim() && me->GetThreat(axe->GetVictim()))
                            {
                                threat = axe->getThreatManager().getThreat(axe->GetVictim());
                                axe->getThreatManager().modifyThreatPercent(axe->GetVictim(), -100);
                            }
                            if(target)
                                axe->AddThreat(target, threat);
                        }
                    }
                }
            } else AxesTargetSwitchTimer -= diff;

            if(AmplifyDamageTimer < diff)
            {
                DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_AMPLIFY_DAMAGE);
                AmplifyDamageTimer = 20000 + rand()%10000;
            }else AmplifyDamageTimer -= diff;
        }

        //Time for global and double timers
        if(InfernalTimer < diff)
        {
            DoCast(me, SPELL_INFERNAL_RELAY_TRIGGER);
            DoScriptText(rand()%2 ? SAY_SUMMON1 : SAY_SUMMON2, me);
            InfernalTimer =  phase == 3 ? 30000 : 45000;    //30 secs in phase 3, 45 otherwise
        }else InfernalTimer -= diff;

        if(ShadowNovaTimer < diff)
        {
            DoCast(me->GetVictim(), SPELL_SHADOWNOVA);
            ShadowNovaTimer = phase == 3 ? 35000 : -1;
        }else ShadowNovaTimer -= diff;

        if(phase != 2)
        {
            if(SWPainTimer < diff)
            {
                Unit* target = nullptr;
                if(phase == 1)
                    target = me->GetVictim();       // the tank
                else                                        //anyone but the tank
                    target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0, true, true);

                if (target)
                    DoCast(target, SPELL_SW_PAIN);

                SWPainTimer = 20000;
            }else SWPainTimer -= diff;
        }

        if(phase != 3)
        {
            if(EnfeebleTimer < diff)
            {
                EnfeebleHealthEffect();
                EnfeebleTimer = 30000;
                ShadowNovaTimer = 5000;
                EnfeebleResetTimer = 9000;
            }else EnfeebleTimer -= diff;
        }

        if(phase==2)
            DoMeleeAttacksIfReady();
        else
            DoMeleeAttackIfReady();
    }

    void DoMeleeAttacksIfReady()
    {
        if( me->IsWithinMeleeRange(me->GetVictim()) && !me->IsNonMeleeSpellCast(false))
        {
            //Check for base attack
            if( me->IsAttackReady() && me->GetVictim() )
            {
                me->AttackerStateUpdate(me->GetVictim());
                me->ResetAttackTimer();
            }
            //Check for offhand attack
            if( me->IsAttackReady(OFF_ATTACK) && me->GetVictim() )
            {
                me->AttackerStateUpdate(me->GetVictim(), OFF_ATTACK);
                me->ResetAttackTimer(OFF_ATTACK);
            }
        }
    }
};

CreatureAI* GetAI_netherspite_infernal(Creature *_Creature)
{
    return new netherspite_infernalAI (_Creature);
}

CreatureAI* GetAI_boss_malchezaar(Creature *_Creature)
{
    return new boss_malchezaarAI (_Creature);
}

CreatureAI* GetAI_infernal_relay(Creature *_Creature)
{
    return new infernal_relayAI (_Creature);
}

void AddSC_boss_malchezaar()
{
    OLDScript *newscript;
    newscript = new OLDScript;
    newscript->Name="boss_malchezaar";
    newscript->GetAI = &GetAI_boss_malchezaar;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="netherspite_infernal";
    newscript->GetAI = &GetAI_netherspite_infernal;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="npc_infernal_relay";
    newscript->GetAI = &GetAI_infernal_relay;
    sScriptMgr->RegisterOLDScript(newscript);
}

