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

enum
{
    SPELL_SHADOWBOLT = 19728,
    SPELL_RAINOFFIRE = 19717,
    SPELL_GEHENNASCURSE = 19716
};

class Boss_Gehennas : public CreatureScript
{
public:

    Boss_Gehennas() : CreatureScript("boss_gehennas")
    {
    }

    class Boss_GehennasAI : public CreatureAI
    {
    public:

        enum event
        {
            EV_SHADOWBOLT = 1,
            EV_RAINOFFIRE,
            EV_CURSE
        };

        Boss_GehennasAI(Creature* creature) : CreatureAI(creature)
        {
            _instance = ((InstanceScript*) creature->GetInstanceScript());
        }

        EventMap events;

        void Reset()
        override {
            events.RescheduleEvent(EV_SHADOWBOLT, 6000);
            events.RescheduleEvent(EV_RAINOFFIRE, 8000);
            events.RescheduleEvent(EV_CURSE, 15000);

            if (_instance)
                _instance->SetData(DATA_GEHENNAS, NOT_STARTED);

            // Respawn the adds if needed
            std::list<Creature*> adds;
            me->GetCreatureListWithEntryInGrid(adds, 11661, 50.0f);
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
                _instance->SetData(DATA_GEHENNAS, IN_PROGRESS);
        }

        void JustDied(Unit* /*killer*/)
        override {
            if (_instance)
                _instance->SetData(DATA_GEHENNAS, DONE);
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
                case EV_SHADOWBOLT:
                    me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_SHADOWBOLT);
                    events.RescheduleEvent(EV_SHADOWBOLT, 6000);
                    break;
                case EV_RAINOFFIRE:
                    me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_RAINOFFIRE);
                    events.RescheduleEvent(EV_RAINOFFIRE, urand(5000, 6000));
                    break;
                case EV_CURSE:
                    me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_GEHENNASCURSE);
                    events.RescheduleEvent(EV_CURSE, urand(28000, 32000));
                    break;
            }

            DoMeleeAttackIfReady();
        }

    private:
        InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Boss_GehennasAI(creature);
    }
};

void AddSC_boss_gehennas()
{
    new Boss_Gehennas();
}

