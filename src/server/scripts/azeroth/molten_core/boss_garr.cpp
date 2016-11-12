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
SDName: Boss_Garr
SD%Complete: 50
SDComment: Adds NYI
SDCategory: Molten Core
EndScriptData */


#include "def_molten_core.h"

enum
{
    // Garr spells
    SPELL_ANTIMAGICPULSE     = 19492,
    SPELL_MAGMASHACKLES      = 19496,
    SPELL_FRENZY             = 19516,

    //Add spells
    SPELL_ERUPTION           = 19497,
    SPELL_IMMOLATE           = 20294,

    NPC_FIRE_WORN            = 12099
};

class Boss_Garr : public CreatureScript
{
    public:
        Boss_Garr() : CreatureScript("Boss_Garr") {}
    
    class Boss_GarrAI : public CreatureAI
    {
        public:
            enum event
            {
                EV_ANTIMAGICPULSE   = 1,
                EV_MAGMASHACKLES    = 2
            };

            Boss_GarrAI(Creature* creature) : CreatureAI(creature)
            {
                _instance = ((InstanceScript*)creature->GetInstanceScript());
            }

            EventMap events;

            void Reset()
            override {
                events.RescheduleEvent(EV_ANTIMAGICPULSE, 25000);
                events.RescheduleEvent(EV_MAGMASHACKLES, 15000);
            
                if (_instance)
                    _instance->SetData(DATA_GARR, NOT_STARTED);
            
                // Respawn the adds if needed
                std::list<Creature*> adds;
                me->GetCreatureListWithEntryInGrid(adds, NPC_FIRE_WORN, 100.0f);
                for (auto & add : adds)
                {
                    if (add->IsDead())
                    {
                        add->DisappearAndDie();
                        add->Respawn();
                    }
                }
            }

            void EnterCombat(Unit* /*victim*/)
            override {
                if (_instance)
                    _instance->SetData(DATA_GARR, IN_PROGRESS);
            }
        
            void JustDied(Unit* /*killer*/)
            override {
                if (_instance)
                    _instance->SetData(DATA_GARR, DONE);
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
                    case EV_ANTIMAGICPULSE:
                        me->CastSpell(me, SPELL_ANTIMAGICPULSE);
                        events.RescheduleEvent(EV_ANTIMAGICPULSE, urand(10000, 15000));
                        break;
                    case EV_MAGMASHACKLES:
                        me->CastSpell(me, SPELL_MAGMASHACKLES);
                        events.RescheduleEvent(EV_MAGMASHACKLES, urand(8000, 12000));
                        break;
                }
            
                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Boss_GarrAI(creature);
    }
};

class Mob_FiresWorn : public CreatureScript
{
    public:
        Mob_FiresWorn() : CreatureScript("Mob_FiresWorn") {}
    
    class Mob_FiresWornAI : public CreatureAI
    {
        public:
            enum event
            {
                EV_ERUPTION         = 1,
                EV_IMMOLATE         = 2
            };

            Mob_FiresWornAI(Creature* creature) : CreatureAI(creature)
            {
                _instance = ((InstanceScript*)creature->GetInstanceScript());
            }

            EventMap events;

            void Reset()
            override {
                events.RescheduleEvent(EV_ERUPTION, 200);
                events.RescheduleEvent(EV_IMMOLATE, 4000);
            }

            void EnterCombat(Unit* victim)
            override {
                if (Creature* garr = _instance->instance->GetCreature(_instance->GetData64(DATA_GARR)))
                    if (!garr->GetVictim())
                        garr->AI()->AttackStart(victim);
            }

            void JustDied(Unit* /*killer*/)
            override {
                if (Creature* garr = _instance->instance->GetCreature(_instance->GetData64(DATA_GARR)))
                    if (garr->IsAlive())
                        garr->CastSpell(garr, SPELL_FRENZY, false);
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
                    case EV_ERUPTION:
                        me->CastSpell(me->GetVictim(), SPELL_ERUPTION);
                        events.CancelEvent(EV_ERUPTION);
                        me->DisappearAndDie();
                        break;
                    case EV_IMMOLATE:
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true), SPELL_IMMOLATE);
                        events.RescheduleEvent(EV_IMMOLATE, urand(5000, 10000));
                        break;
                }

                if (me->GetHealth() <= me->GetMaxHealth() * 0.10)
                {
                    events.RescheduleEvent(EV_ERUPTION, 200);
                }

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Mob_FiresWornAI(creature);
    }
};

void AddSC_boss_garr()
{
    new Boss_Garr();
    new Mob_FiresWorn();
}

