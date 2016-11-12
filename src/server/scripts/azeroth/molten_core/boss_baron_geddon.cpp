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
SDName: Boss_Baron_Geddon
SD%Complete: 100
SDComment:
SDCategory: Molten Core
EndScriptData */


#include "def_molten_core.h"

enum
{
    EMOTE_SERVICE         = -1409000,

    SPELL_INFERNO         = 19695,
    SPELL_IGNITEMANA      = 19659,
    SPELL_LIVINGBOMB      = 20475,
    SPELL_ARMAGEDDOM      = 20479
};

class Boss_Baron_Geddon : public CreatureScript
{
    public:
        Boss_Baron_Geddon() : CreatureScript("Boss_Baron_Geddon") {}
    
    class Boss_Baron_GeddonAI : public CreatureAI
    {
        public:
            enum event
            {
                EV_INFERNO        = 1,
                EV_IGNITEMANA     ,
                EV_LIVINGBOMB     ,
                EV_ARMAGEDDOM     ,
            };

            Boss_Baron_GeddonAI(Creature* creature) : CreatureAI(creature)
            {
                _instance = ((InstanceScript*)creature->GetInstanceScript());
            }

            EventMap events;

            void Reset()
            override {
                events.RescheduleEvent(EV_INFERNO, 15000);
                events.RescheduleEvent(EV_IGNITEMANA, 5000);
                events.RescheduleEvent(EV_LIVINGBOMB, 30000);
                events.RescheduleEvent(EV_ARMAGEDDOM, 200);

                if (_instance)
                    _instance->SetData(DATA_GEDDON, NOT_STARTED);
            }

            void EnterCombat(Unit* /*victim*/)
            override {
                if (_instance)
                    _instance->SetData(DATA_GEDDON, IN_PROGRESS);
            }
        
            void JustDied(Unit* /*killer*/)
            override {
                if (_instance)
                    _instance->SetData(DATA_GEDDON, DONE);
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
                    case EV_INFERNO:
                        me->CastSpell(me, SPELL_INFERNO);
                        events.RescheduleEvent(EV_INFERNO, urand(15000, 16000));
                        break;
                    case EV_IGNITEMANA:
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true), SPELL_IGNITEMANA);
                        events.RescheduleEvent(EV_IGNITEMANA, urand(5000, 6000));
                        break;
                    case EV_LIVINGBOMB:
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true), SPELL_LIVINGBOMB);
                        events.RescheduleEvent(EV_LIVINGBOMB, 30000);
                        break;
                    case EV_ARMAGEDDOM:
                        me->InterruptNonMeleeSpells(true);
                        me->CastSpell(me, SPELL_ARMAGEDDOM);
                        DoScriptText(EMOTE_SERVICE, me);
                        events.CancelEvent(EV_ARMAGEDDOM);
                        break;
                }

                if (me->GetHealth()*100 / me->GetMaxHealth() <= 2.5f)
                {
                    events.RescheduleEvent(EV_ARMAGEDDOM, 200);
                }

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Boss_Baron_GeddonAI(creature);
    }
};

void AddSC_boss_baron_geddon()
{
    new Boss_Baron_Geddon();
}

