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
SDName: Boss_Golemagg
SD%Complete: 90
SDComment:
SDCategory: Molten Core
EndScriptData */


#include "def_molten_core.h"

enum
{
    EMOTE_AEGIS           = -1409002,

    SPELL_MAGMASPLASH     = 13879,
    SPELL_PYROBLAST       = 20228,
    SPELL_EARTHQUAKE      = 19798,
    SPELL_ENRAGE          = 19953,
    SPELL_BUFF            = 20553,

    //-- CoreRager Spells --
    SPELL_MANGLE          = 19820,
    SPELL_AEGIS           = 20620,               //This is self casted whenever we are below 50%

    NPC_CORE_RAGER        = 11672
};

class Boss_Golemagg : public CreatureScript
{
    public:
        Boss_Golemagg() : CreatureScript("Boss_Golemagg") {}

    class Boss_GolemaggAI : public CreatureAI
    {
        public:
            enum event
            {
                EV_PYROBLAST     = 1,
                EV_EARTHQUAKE    = 2,
                EV_ENRAGE        = 3,
                EV_BUFF          = 4
            };

            Boss_GolemaggAI(Creature* creature) : CreatureAI(creature)
            {
                _instance = ((InstanceScript*)creature->GetInstanceScript());
            }

            EventMap events;

            void Reset()
            override {
                events.RescheduleEvent(EV_PYROBLAST, 4000);
                events.RescheduleEvent(EV_EARTHQUAKE, 3000);
                events.RescheduleEvent(EV_ENRAGE, 200);
                events.RescheduleEvent(EV_BUFF, 2500);
            
                if (_instance)
                    _instance->SetData(DATA_GOLEMAGG, NOT_STARTED);

                // Respawn the adds if needed
                std::list<Creature*> adds;
                me->GetCreatureListWithEntryInGrid(adds, NPC_CORE_RAGER, 100.0f);
                for (auto & add : adds)
                {
                    if (add->IsDead())
                    {
                        add->DisappearAndDie();
                        add->Respawn();
                    }
                }

                me->CastSpell(me, SPELL_MAGMASPLASH, true);
            }

            void EnterCombat(Unit* /*victim*/)
            override {
                if (_instance)
                    _instance->SetData(DATA_GOLEMAGG, IN_PROGRESS);
            }
        
            void JustDied(Unit* /*killer*/)
            override {
                std::list<Creature*> adds;
                me->GetCreatureListWithEntryInGrid(adds, NPC_CORE_RAGER, 100.0f);
                for (auto & add : adds)
                {
                    if (add->IsAlive())
                        add->DisappearAndDie();
                }

                if (_instance)
                    _instance->SetData(DATA_GOLEMAGG, DONE);
            }
        
            void UpdateAI(uint32 const diff)
            override {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                switch (events.GetEvent())
                {
                    case EV_PYROBLAST:
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true), SPELL_PYROBLAST);
                        events.RescheduleEvent(EV_PYROBLAST, urand(4000, 5000));
                        break;
                    case EV_EARTHQUAKE:
                        me->CastSpell(me->GetVictim(), SPELL_EARTHQUAKE);
                        events.RescheduleEvent(EV_EARTHQUAKE, urand(3000, 4000));
                        break;
                    case EV_ENRAGE:
                        if (me->GetHealth()*100 / me->GetMaxHealth() < 10.0f)
                            me->CastSpell(me, SPELL_ENRAGE);

                        events.RescheduleEvent(EV_ENRAGE, 62000);
                        break;
                    case EV_BUFF:
                        me->CastSpell(me, SPELL_BUFF);
                        events.RescheduleEvent(EV_BUFF, urand(2500, 3000));
                        break;
                }

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Boss_GolemaggAI(creature);
    }
};

class Mob_Core_Rager : public CreatureScript
{
    public:
        Mob_Core_Rager() : CreatureScript("Mob_Core_Rager") {}

    class Mob_Core_RagerAI : public CreatureAI
    {
        public:
            enum event
            {
                EV_MANGLE     = 1,
                EV_AEGIS      = 2
            };

            enum phases
            {
                PHASE_NORMAL           = 1,
                PHASE_BELOW_50_PERCENT = 2,
            };

            Mob_Core_RagerAI(Creature* creature) : CreatureAI(creature)
            {
                _instance = ((InstanceScript*)creature->GetInstanceScript());
            }

            EventMap events;

            void Reset()
            override {
                events.RescheduleEvent(EV_MANGLE, 7000);
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
                    case EV_MANGLE:
                        me->CastSpell(me->GetVictim(), SPELL_MANGLE);
                        events.RescheduleEvent(EV_MANGLE, urand(10000, 11000));
                        break;
                    case EV_AEGIS:
                        me->CastSpell(me, SPELL_AEGIS);
                        DoScriptText(EMOTE_AEGIS, me);
                        events.CancelEvent(EV_AEGIS);
                        break;
                }

                if (!events.IsInPhase(PHASE_BELOW_50_PERCENT) && me->IsBelowHPPercent(50.0f))
                {
                    events.SetPhase(PHASE_BELOW_50_PERCENT);
                    events.RescheduleEvent(EV_AEGIS, 200, 0, PHASE_BELOW_50_PERCENT);
                }

                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Mob_Core_RagerAI(creature);
    }
};

void AddSC_boss_golemagg()
{
    new Boss_Golemagg();
    new Mob_Core_Rager();
}

