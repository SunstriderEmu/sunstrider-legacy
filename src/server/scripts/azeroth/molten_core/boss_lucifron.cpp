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
SDName: Boss_Lucifron
SD%Complete: 100
SDComment:
SDCategory: Molten Core
EndScriptData */


#include "def_molten_core.h"

enum {
    SPELL_IMPENDINGDOOM = 19702,
    SPELL_LUCIFRONCURSE = 19703,
    SPELL_SHADOWSHOCK   = 20603,
    
    NPC_FLAMEWAKER_PROTECTOR    = 12119
};

class Boss_Lucifron : public CreatureScript
{
public:
    Boss_Lucifron() : CreatureScript("boss_lucifron") {}
    
    class Boss_LucifronAI : public CreatureAI
    {
    public:
        enum event {
            EV_IMPENDINGDOOM    = 1,
            EV_LUCIFRONCURSE    = 2,
            EV_SHADOWSHOCK      = 3
        };
        
        Boss_LucifronAI(Creature* creature) : CreatureAI(creature)
        {
            _instance = ((InstanceScript*)creature->GetInstanceScript());
        }

        EventMap events;

        void Reset()
        override {
            events.RescheduleEvent(EV_IMPENDINGDOOM, 15000);
            events.RescheduleEvent(EV_LUCIFRONCURSE, 10000);
            events.RescheduleEvent(EV_SHADOWSHOCK, 6000);

            if (_instance)
                _instance->SetData(DATA_LUCIFRON, NOT_STARTED);
            
            // Respawn the adds if needed
            std::list<Creature*> adds;
            me->GetCreatureListWithEntryInGrid(adds, NPC_FLAMEWAKER_PROTECTOR, 100.0f);
            for (auto & add : adds) {
                if (add->IsDead()) {
                    add->DisappearAndDie();
                    add->Respawn();
                }
            }
        }
        
        void EnterCombat(Unit* /*victim*/)
        override {
            if (_instance)
                _instance->SetData(DATA_LUCIFRON, IN_PROGRESS);
        }
        
        void JustDied(Unit* /*killer*/)
        override {
            if (_instance)
                _instance->SetData(DATA_LUCIFRON, DONE);
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
                case EV_IMPENDINGDOOM:
                    me->CastSpell(me->GetVictim(), SPELL_IMPENDINGDOOM);
                    events.RescheduleEvent(EV_IMPENDINGDOOM, urand(15000, 20000));
                    break;
                case EV_LUCIFRONCURSE:
                    me->CastSpell(me->GetVictim(), SPELL_LUCIFRONCURSE);
                    events.RescheduleEvent(EV_LUCIFRONCURSE, urand(15000, 20000));
                    break;
                case EV_SHADOWSHOCK:
                    me->CastSpell(me->GetVictim(), SPELL_SHADOWSHOCK);
                    events.RescheduleEvent(EV_SHADOWSHOCK, 4000);
                    break;
            }
            
            DoMeleeAttackIfReady();
        }
        
    private:
        InstanceScript* _instance;
    };
    
    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Boss_LucifronAI(creature);
    }
};

enum {
    SPELL_DOMINATE_MIND = 20604,
    SPELL_CLEAVE        = 20605
};

class Add_FlamewakerProtector : public CreatureScript
{
public:
    Add_FlamewakerProtector() : CreatureScript("add_flamewakerprotector") {}
    
    class Add_FlamewakerProtectorAI : public CreatureAI
    {
    public:
        enum event {
            EV_DOMINATE_MIND    = 1,
            EV_CLEAVE           = 2
        };
        
        Add_FlamewakerProtectorAI(Creature* creature) : CreatureAI(creature) {}
        
        EventMap events;

        void Reset()
        override {
            events.RescheduleEvent(EV_DOMINATE_MIND, 15000);
            events.RescheduleEvent(EV_CLEAVE, 6000);
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
                case EV_DOMINATE_MIND:
                    me->CastSpell(me->GetVictim(), SPELL_DOMINATE_MIND);
                    events.RescheduleEvent(EV_DOMINATE_MIND, 15000);
                    break;
                case EV_CLEAVE:
                    me->CastSpell(me->GetVictim(), SPELL_CLEAVE);
                    events.RescheduleEvent(EV_CLEAVE, 6000);
                    break;
            }
            
            DoMeleeAttackIfReady();
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Add_FlamewakerProtectorAI(creature);
    }
};

void AddSC_boss_lucifron()
{
    new Boss_Lucifron();
    new Add_FlamewakerProtector();
}

