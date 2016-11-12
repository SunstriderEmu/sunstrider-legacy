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


#include "def_molten_core.h"

/* Missing feature
 *   - Lava Splash Visual. Spells SPELL_LAVA_BURST_INVOC_A to SPELL_LAVA_BURST_INVOC_I correctly spawns gobjects at random location but we need to find the correct visual before adding damages.
 */

enum
{
    EMERGED_TIME =              180000,
    SUBMERGING_TIME =             1000,
    SUBMERGED_TIME =             90000,
    EMERGING_TIME =               3000,

    MELTWEAPON_CHANCE =             10, //%
    MANABURN_RANGE =                 8,
    ADDS_NUMBER =                    8,

    SAY_SUMMON_DOMO =         -1409008,
    SAY_ARRIVAL1_RAG =        -1409009,
    SAY_ARRIVAL2_DOMO =       -1409010,
    SAY_ARRIVAL3_RAG =        -1409011,
    SAY_ARRIVAL4_RAG =        -1409012,
 
    SAY_REINFORCEMENTS1 =     -1409013,
    SAY_REINFORCEMENTS2 =     -1409014,
    SAY_HAMMER =              -1409015,
    SAY_WRATH =               -1409016,
    SAY_KILL =                -1409017,
    SAY_MAGMA_BLAST =         -1409018,
 
    FACTION_FRIENDLY =              35,
 
    CREATURE_SON_OF_FLAME =      12143,
    CREATURE_FLAME_OF_RAGNAROS = 13148,
 
    SPELL_WRATH_OF_RAGNAROS =    20566,
   
    SPELL_MAGMA_BLAST =          20565,
    SPELL_MELT_WEAPON =          21388,
    SPELL_ELEMENTAL_FIRE =       20564,
 
    SPELL_SUBMERGE =             21107, //Stealth aura
    SPELL_EMERGE =               20568,
    SPELL_ELEMENTAL_FIRE_KILL =  19773, //instakill, used in intro only
    SPELL_INTENSE_HEAT         = 21155,
    SPELL_MIGHT_OF_RAGNAROS    = 21154,
 
    SPELL_SUMMON_RAGNAROS =      19774, //for Domo, only visual
    SPELL_MANABURN =             19665, //for adds
 
    SPELL_LAVA_BURST_INVOC_A =   21886,
    SPELL_LAVA_BURST_INVOC_B =   21900,
    /* ... */
    SPELL_LAVA_BURST_INVOC_I =   21907,
 
    /* These used for something?
    SPELL_LAVABURST =            21158,
    SPELL_ERUPTION =             17731,
    SPELL_SONSOFFLAME_DUMMY =    21108,
    SPELL_HAMMER_OF_RAGNAROS =   19780,
    */
};

struct Locations { float x, y, z, o; };
static Locations DomoNewLocation   = { 851.106262, -814.688660, -229.283966, 4.641055 };
static Locations DomoInvocLocation = { 829.947754, -814.614807, -228.950043, 5.6      };

enum
{
    INTRO,
    EMERGED,
    SUBMERGING,
    SUBMERGED,
    EMERGING,
};

class Boss_Ragnaros : public CreatureScript
{
    public:
        Boss_Ragnaros() : CreatureScript("Boss_Ragnaros") {}

    class Boss_RagnarosAI : public CreatureAI
    {
        public:
            uint8 Intro_Phase;
            uint32 Intro_Timer;
            Creature* Domo;

            uint32 WrathOfRagnaros_Timer;
            uint32 HammerOfRagnaros_Timer;
            uint32 MagmaBlast_Wait_Timer;
            uint32 LavaSplash_Timer;
            bool Said_MagmaBlast;
            bool Emerging_Waited;

            uint32 RagnaFaction;
            uint8 AddCount;

            uint8 phase;

            uint32 Phase_Timer;

            EventMap events;

            Boss_RagnarosAI(Creature* creature) : CreatureAI(creature), Summons(me)
            {
                _instance = ((InstanceScript*)creature->GetInstanceScript());
                RagnaFaction = me->GetFaction();
                Domo = nullptr;
                SetCombatMovementAllowed(false);
            }

            void Reset()
            override {
                Summons.DespawnAll();
                AddCount = 0;

                events.SetPhase(EMERGED);

                if (_instance)
                {
                    _instance->SetData(DATA_RAGNAROS, NOT_STARTED);
                    if (Intro_Phase == 0)
                        setPhase(INTRO);
                }
            }

            void ResetAttacksTimers()
            {
                WrathOfRagnaros_Timer = 25000;
                HammerOfRagnaros_Timer = urand(20000, 30000);
                MagmaBlast_Wait_Timer = 2000 ;
                LavaSplash_Timer = urand(500, 15000);
                Said_MagmaBlast = false;
            }

            void NextPhase()
            {
                switch(phase)
                {
                    case INTRO:        setPhase(EMERGED); break;
                    case EMERGED :     setPhase(SUBMERGING); break;
                    case SUBMERGING :  setPhase(SUBMERGED); break;
                    case SUBMERGED :   setPhase(EMERGING); break;
                    case EMERGING :    setPhase(EMERGED); break;
                }
            }

            void setPhase(uint32 newPhase)
            {
                phase = newPhase;
                events.SetPhase(newPhase);
                switch(newPhase)
                {
                    case INTRO:
                        Intro_Timer = 0;
                        Intro_Phase = 0;
                        me->SetVisibility(VISIBILITY_OFF);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->SetFaction(FACTION_FRIENDLY);
                        me->AddAura(SPELL_SUBMERGE, me);
                        if (Domo)
                        {
                            Domo->DisappearAndDie();
                            Domo = nullptr;
                        }
                        break;
                    case EMERGED:
                        Phase_Timer = EMERGED_TIME;
                        ResetAttacksTimers();
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->SetFaction(RagnaFaction);
                        break;
                    case SUBMERGING:
                        Phase_Timer = SUBMERGING_TIME;
                        me->CastSpell(me,SPELL_SUBMERGE);
                        DoScriptText(rand()%2 ? SAY_REINFORCEMENTS1 : SAY_REINFORCEMENTS2, me);
                        break;
                    case SUBMERGED:
                        Phase_Timer = SUBMERGED_TIME;
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->SetFaction(FACTION_FRIENDLY);
                        SummonAdds();
                        break;
                    case EMERGING:
                        //submerge spell set this off, emerge set it back on but the animation is then skipped,
                        //so we have to set on back ourselves and wait a world update before continuing
                        me->SetVisibility(VISIBILITY_ON);
                        Phase_Timer = EMERGING_TIME;
                        Emerging_Waited = false;
                        break;
                }
            }

            void JustSummoned(Creature* summoned)
            override {
                Summons.Summon(summoned);

                if (summoned->GetEntry() == CREATURE_SON_OF_FLAME)
                {
                    if (summoned->AI())
                    {
                        Unit* target = nullptr;
                        if (Unit* pTarget = summoned->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 80, true))
                            target = pTarget;
                        else
                            target = me->GetVictim();

                        if(target)
                        {
                            summoned->AI()->AttackStart(target);
                            summoned->AddThreat(target, 4000.0);
                        }
                    }
                    summoned->SetOwnerGUID(me->GetGUID());
                    AddCount++;
                }

                if (summoned->AI())
                    summoned->AI()->DoZoneInCombat();
            }
    
            void SummonedCreatureDespawn(Creature* unit)
            override {
                Summons.Despawn(unit);
            }

            void KilledUnit(Unit* /*victim*/)
            override {
                if (rand()%4)
                    return;

                DoScriptText(SAY_KILL, me);
            }

            void JustDied(Unit* /*killer*/)
            override {
                Summons.DespawnAll();

                if(_instance)
                    _instance->SetData(DATA_RAGNAROS, DONE);
            }

            void EnterCombat(Unit* /*victim*/)
            override {
                if(_instance)
                    _instance->SetData(DATA_RAGNAROS, IN_PROGRESS);
            }

            void SummonAdds()
            {
                float fX, fY, fZ;
                for(uint8 i = 0; i < ADDS_NUMBER; ++i)
                {
                    me->GetRandomPoint(me, 30.0f, fX, fY, fZ);
                    me->SummonCreature(CREATURE_SON_OF_FLAME, fX, fY, fZ, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1000);
                }
            }

            bool UpdateMeleeVictim()
            {
                if (me->IsWithinMeleeRange(me->GetVictim()))
                    return true;
                else
                {
                    Unit* target = nullptr;
                    float MaxThreat = 0;
                    std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
                    for (auto & i : m_threatlist)
                    {
                        Unit* pUnit = ObjectAccessor::GetUnit((*me), i->getUnitGuid());
                        if(pUnit && me->IsWithinMeleeRange(pUnit))
                        {
                            if (i->getThreat() > MaxThreat)
                            {
                                target = pUnit;
                                MaxThreat = i->getThreat();
                            }
                        }
                    }
                    if (target)
                    {
                        AttackStart(target);
                        return true;
                    }
                }
                return false;
            }

            void DamageTaken(Unit* who, uint32& /* Damage */)
            override {  
                if (who->HasUnitState(UNIT_STATE_MELEE_ATTACKING) && rand()%100 < MELTWEAPON_CHANCE)
                    me->CastSpell(who, SPELL_MELT_WEAPON, true);
            }
 
            void DoIntroEvent(uint8 Event)
            {
                switch(Event)
                {
                    case 1:
                        Domo->CastSpell(Domo,SPELL_SUMMON_RAGNAROS,false); //only visual
                        Domo->GetMotionMaster()->MovePoint(0, DomoInvocLocation.x, DomoInvocLocation.y, DomoInvocLocation.z, false);
                        DoScriptText(SAY_SUMMON_DOMO, Domo);
                        Intro_Timer = 10000;
                        break;
                    case 2:
                        Domo->SetInFront(me);
                        Domo->StopMoving();
                        Intro_Timer = 4000;
                        break;
                    case 3:
                        me->SetVisibility(VISIBILITY_ON);
                        Intro_Timer = 0;
                        break;
                    case 4:
                        me->RemoveAurasDueToSpell(SPELL_SUBMERGE);
                        me->CastSpell(me,SPELL_EMERGE);
                        Intro_Timer = 3000;
                        break;
                    case 5:
                        DoScriptText(SAY_ARRIVAL1_RAG, me);
                        Intro_Timer = 14000;
                        break;
                    case 6:
                        DoScriptText(SAY_ARRIVAL2_DOMO, Domo);
                        Intro_Timer = 8000;
                        break;
                    case 7:
                        DoScriptText(SAY_ARRIVAL3_RAG, me);
                        Intro_Timer = 17000;
                        break;
                    case 8:
                        me->CastSpell(Domo,SPELL_ELEMENTAL_FIRE_KILL,false);
                        Intro_Timer = 1000;
                        break;
                    case 9:
                        Domo->SetDeathState(JUST_DIED); //to be sure
                        Intro_Timer = 3000;
                        break;        
                    case 10:
                        if(Player* faceMe = me->FindNearestPlayer(50.0f))
                        {
                            me->SetInFront(faceMe);
                            me->StopMoving();
                        }
                        DoScriptText(SAY_ARRIVAL4_RAG, me);
                        Intro_Timer = 8000;
                        break;
                    case 11:
                        Domo = nullptr;
                        NextPhase();
                        DoZoneInCombat();
                        break;
                }
            }

            void UpdateAI(uint32 const diff)
            override {
                if (phase != INTRO && UpdateVictim())
                {
                    if (Phase_Timer < diff)
                    {
                        if (!me->IsNonMeleeSpellCast(false)) //wait to be ready to cast SetPhase spells
                            NextPhase();
                        else
                            return;
                    }
                    else
                        Phase_Timer -= diff;
                }
            
                switch(phase)
                {
                    case INTRO:
                        //Setting up Domo
                        if (Intro_Phase == 0 && !Domo && _instance->GetData(DATA_MAJORDOMO) == DONE)
                        {
                            Domo = me->SummonCreature(12018, DomoNewLocation.x, DomoNewLocation.y, DomoNewLocation.z, DomoNewLocation.o, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
                            if (Domo)
                            {
                                Domo->SetOwnerGUID(me->GetGUID());
                                Domo->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            }
                            return;
                        }
 
                        if (Domo)
                        {
                            //Play Intro
                            if (Intro_Phase)
                            {
                                if (Intro_Timer < diff)
                                {
                                    DoIntroEvent(Intro_Phase);
                                    Intro_Phase++;
                                }
                                else
                                    Intro_Timer -= diff;

                                return;
                            }
                            else if (!Domo->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP))
                                Intro_Phase = 1; //start intro
                        }
                        break;
                    case EMERGED:
                        if (!me->GetVictim())
                            return;
 
                        //Wrath Of Ragnaros
                        if (WrathOfRagnaros_Timer < diff)
                        {  
                            if (!me->IsNonMeleeSpellCast(false))
                            {
                                DoScriptText(SAY_WRATH, me);
                                me->CastSpell((Unit*)nullptr, SPELL_WRATH_OF_RAGNAROS);
                                WrathOfRagnaros_Timer = 25000;
                            }
                        }
                        else
                            WrathOfRagnaros_Timer -= diff;
           
                        //Hammer of Ragnaros
                        if (HammerOfRagnaros_Timer < diff)
                        {  
                            if (!me->IsNonMeleeSpellCast(false))
                            {
                                std::vector<Unit*> ValidTargets;
                                std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
                                for (auto & i : m_threatlist)
                                {
                                    Unit* pTarget = ObjectAccessor::GetUnit(*me,i->getUnitGuid());
                                    if (pTarget && pTarget->GetPowerType() == POWER_MANA && pTarget->IsWithinLOSInMap(me)) //&& maxrange?
                                        ValidTargets.push_back(pTarget);
                                }
                                if (!ValidTargets.empty())
                                {
                                    DoScriptText(SAY_HAMMER, me);
                                    Unit* target = ValidTargets[urand(0, ValidTargets.size() -1)];
                                    me->CastSpell(target,SPELL_MIGHT_OF_RAGNAROS, false);
                                }
                                HammerOfRagnaros_Timer = urand(20000, 30000);
                            }
                        }
                        else
                            HammerOfRagnaros_Timer -= diff;

                        if (!UpdateMeleeVictim())
                        {
                            //Magma Blast
                            if (MagmaBlast_Wait_Timer < diff)
                            {
                                if(!me->IsNonMeleeSpellCast(false))
                                {
                                    Unit* target = nullptr;
                                    target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150,true);
                                    if(target && target->IsWithinLOSInMap(me))
                                    {
                                        me->CastSpell(target, SPELL_MAGMA_BLAST, false);
                                        if (!Said_MagmaBlast && Phase_Timer < EMERGED_TIME - 20000) //we dont want to say we're tired just after aggro
                                        {  
                                            DoScriptText(SAY_MAGMA_BLAST, me);
                                            Said_MagmaBlast = true;
                                        }
                                    }
                                }
                            }
                            else
                                MagmaBlast_Wait_Timer -= diff;
                        }
                        else
                        {
                            MagmaBlast_Wait_Timer = 2000;
                            //Elemental Fire
                            if (!me->GetVictim()->HasAuraEffect(SPELL_ELEMENTAL_FIRE, 1)) //index 0 = damage, 1 = aura
                                me->CastSpell(me->GetVictim(),SPELL_ELEMENTAL_FIRE, false);
                        }
                        DoMeleeAttackIfReady();
                        break;
                    case SUBMERGING:
                        break;
                    case SUBMERGED:
                        if (AddCount == 0)
                            Phase_Timer = 0;
                        break;
                    case EMERGING:
                        if (Emerging_Waited)
                        {
                            if(me->HasAuraEffect(SPELL_SUBMERGE, 0))
                            {
                                me->RemoveAurasDueToSpell(SPELL_SUBMERGE);
                                me->CastSpell(me,SPELL_EMERGE);
                            }
                        }
                        else
                            Emerging_Waited = true;

                        break;
                }
                //Lava Splash
                if (me->IsInCombat())
                {
                    if (LavaSplash_Timer < diff)
                    {
                        if (_instance)
                        {
                            uint8 SplashLoc = urand(0,8);
                            if(SplashLoc == 0)
                                me->CastSpell((Unit*)nullptr, SPELL_LAVA_BURST_INVOC_A);
                            else
                                me->CastSpell((Unit*)nullptr, (SPELL_LAVA_BURST_INVOC_B -1) + SplashLoc);
 
                            LavaSplash_Timer = urand(500, 15000);
                        }
                    }
                    else
                        LavaSplash_Timer -= diff;
                }
            }

        private:
            InstanceScript* _instance;
            SummonList Summons;
    };

    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Boss_RagnarosAI(creature);
    }
};

class Son_Of_Flame : public CreatureScript
{
    public:
        Son_Of_Flame() : CreatureScript("Son_Of_Flame") {}

    class Son_Of_FlameAI : public CreatureAI
    {
        public:
            enum event
            {
                EV_MANABURN       = 1
            };

            Son_Of_FlameAI(Creature* creature) : CreatureAI(creature)
            {
                _instance = ((InstanceScript*)creature->GetInstanceScript());
            }

            EventMap events;

            void Reset()
            override {
                events.RescheduleEvent(EV_MANABURN, 3000);
            }

            void JustDied(Unit* /*killer*/)
            override {
                if (Creature* ragnaros = _instance->instance->GetCreature(_instance->GetData64(DATA_RAGNAROS)))
                    ((Boss_Ragnaros::Boss_RagnarosAI*)ragnaros->AI())->AddCount--;
            }

            void DoAoEManaburn()
            {
                std::list<HostileReference*>& m_threatlist = me->getThreatManager().getThreatList();
                auto i = m_threatlist.begin();
                for (i = m_threatlist.begin(); i!= m_threatlist.end();++i)
                {
                    Unit* pUnit = ObjectAccessor::GetUnit((*me), (*i)->getUnitGuid());
                    if(pUnit && pUnit->IsWithinDistInMap(me,MANABURN_RANGE))
                        pUnit->CastSpell(pUnit, SPELL_MANABURN, true);
                }
            }

            void UpdateAI(uint32 const diff)
            override {
                if (!UpdateVictim())
                    return;

                events.Update(diff);
            
                switch (events.GetEvent())
                {
                    case 0:
                        break;
                    case EV_MANABURN:
                        DoAoEManaburn();
                        events.RescheduleEvent(EV_MANABURN, 3000);
                        break;
                }

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Son_Of_FlameAI(creature);
    }
};

void AddSC_boss_ragnaros()
{
    new Boss_Ragnaros();
    new Son_Of_Flame();
}

