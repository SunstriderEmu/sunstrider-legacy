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
SDName: Boss_Shazzrah
SD%Complete: 75
SDComment: Teleport NYI
SDCategory: Molten Core
EndScriptData */


#include "def_molten_core.h"

enum
{
    SPELL_ARCANEEXPLOSION   = 19712,
    SPELL_SHAZZRAHCURSE     = 19713,
    SPELL_DEADENMAGIC       = 19714,
    SPELL_COUNTERSPELL      = 19715,
    SPELL_GATE_SHAZZRAH     = 23138
};

class Boss_Shazzrah : public CreatureScript
{
    public:
        Boss_Shazzrah() : CreatureScript("Boss_Shazzrah") {}
    
    class Boss_ShazzrahAI : public CreatureAI
    {
        public:
            enum event
            {
                EV_ARCANEEXPLOSION   = 1,
                EV_SHAZZRAHCURSE     ,
                EV_DEADENMAGIC       ,
                EV_COUNTERSPELL      ,
                EV_GATE_SHAZZRAH     ,
            };

            Boss_ShazzrahAI(Creature* creature) : CreatureAI(creature)
            {
                _instance = ((InstanceScript*)creature->GetInstanceScript());
            }

            EventMap events;

            void Reset()
            override {
                events.RescheduleEvent(EV_ARCANEEXPLOSION, 5000);
                events.RescheduleEvent(EV_SHAZZRAHCURSE, 20000);
                events.RescheduleEvent(EV_DEADENMAGIC, 24000);
                events.RescheduleEvent(EV_COUNTERSPELL, 15000);
                events.RescheduleEvent(EV_GATE_SHAZZRAH, 45000);
            
                if (_instance)
                    _instance->SetData(DATA_SHAZZRAH, NOT_STARTED);
            }

            void EnterCombat(Unit* /*victim*/)
            override {
                if (_instance)
                    _instance->SetData(DATA_SHAZZRAH, IN_PROGRESS);
            }
        
            void JustDied(Unit* /*killer*/)
            override {
                if (_instance)
                    _instance->SetData(DATA_SHAZZRAH, DONE);
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
                    case EV_ARCANEEXPLOSION:
                        me->CastSpell(me->GetVictim(), SPELL_ARCANEEXPLOSION);
                        events.RescheduleEvent(EV_ARCANEEXPLOSION, urand(5000, 6000));
                        break;
                    case EV_SHAZZRAHCURSE:
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true), SPELL_SHAZZRAHCURSE);
                        events.RescheduleEvent(EV_SHAZZRAHCURSE, urand(20000, 25000));
                        break;
                    case EV_DEADENMAGIC:
                        me->CastSpell(me, SPELL_DEADENMAGIC);
                        events.RescheduleEvent(EV_DEADENMAGIC, 35000);
                        break;
                    case EV_COUNTERSPELL:
                        me->CastSpell(me->GetVictim(), SPELL_COUNTERSPELL);
                        events.RescheduleEvent(EV_COUNTERSPELL, urand(16000, 20000));
                        break;
                    case EV_GATE_SHAZZRAH:
                        me->CastSpell(me, SPELL_GATE_SHAZZRAH);
                        events.RescheduleEvent(EV_GATE_SHAZZRAH, 45000);
                        break;
                }
            
                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Boss_ShazzrahAI(creature);
    }
};

void AddSC_boss_shazzrah()
{
    new Boss_Shazzrah();
}

