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
SDName: Boss_Sulfuron_Harbringer
SD%Complete: 80
SDComment: Adds NYI
SDCategory: Molten Core
EndScriptData */


#include "def_molten_core.h"

enum
{
    SPELL_THROW               = 19785,
    SPELL_DEMORALIZINGSHOUT   = 19778,
    SPELL_INSPIRE             = 19779,
    SPELL_KNOCKDOWN           = 19780,
    SPELL_FLAMESPEAR          = 19781,

    //Adds Spells
    SPELL_DARKSTRIKE          = 19777,
    SPELL_HEAL                = 19775,
    SPELL_SHADOWWORDPAIN      = 19776,
    SPELL_IMMOLATE            = 20294,

    NPC_FLAMEWAKER_PRIEST     = 11662
};

class Boss_Sulfuron : public CreatureScript
{
    public:
        Boss_Sulfuron() : CreatureScript("Boss_Sulfuron") {}
    
    class Boss_SulfuronAI : public ScriptedAI
    {
        public:
            enum event
            {
                EV_THROW              = 1,
                EV_DEMORALIZINGSHOUT  ,
                EV_INSPIRE            ,
                EV_KNOCKDOWN          ,
                EV_FLAMESPEAR         ,
            };

            Boss_SulfuronAI(Creature* creature) : ScriptedAI(creature)
            {
                _instance = ((InstanceScript*)creature->GetInstanceScript());
            }

            EventMap events;

            void Reset()
            override {
                events.RescheduleEvent(EV_THROW, 5000);
                events.RescheduleEvent(EV_DEMORALIZINGSHOUT, 25000);
                events.RescheduleEvent(EV_INSPIRE, 13000);
                events.RescheduleEvent(EV_KNOCKDOWN, 6000);
                events.RescheduleEvent(EV_FLAMESPEAR, 2000);

                if (_instance)
                    _instance->SetData(DATA_SULFURON, NOT_STARTED);

                // Respawn the adds if needed
                std::list<Creature*> adds;
                me->GetCreatureListWithEntryInGrid(adds, NPC_FLAMEWAKER_PRIEST, 100.0f);
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
                    _instance->SetData(DATA_SULFURON, IN_PROGRESS);
            }
        
            void JustDied(Unit* /*killer*/)
            override {
                if (_instance)
                    _instance->SetData(DATA_SULFURON, DONE);
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
                    case EV_DEMORALIZINGSHOUT:
                        me->CastSpell(me->GetVictim(), SPELL_DEMORALIZINGSHOUT);
                        events.RescheduleEvent(EV_DEMORALIZINGSHOUT, urand(25000, 26000));
                        break;
                    case EV_INSPIRE:
                    {
                        Creature* target = nullptr;
                        std::list<Creature*> pList = DoFindFriendlyMissingBuff(45.0f,SPELL_INSPIRE);
                        if (!pList.empty())
                        {
                            auto i = pList.begin();
                            advance(i, (rand()%pList.size()));
                            target = (*i);
                        }

                        if (target)
                            me->CastSpell(target, SPELL_INSPIRE);

                        me->CastSpell(me, SPELL_INSPIRE);
                        events.RescheduleEvent(EV_INSPIRE, urand(20000, 26000));
                        break;
                    }
                    case EV_KNOCKDOWN:
                        me->CastSpell(me->GetVictim(), SPELL_KNOCKDOWN);
                        events.RescheduleEvent(EV_KNOCKDOWN, 12000, 15000);
                        break;
                    case EV_FLAMESPEAR:
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true), SPELL_FLAMESPEAR);
                        events.RescheduleEvent(EV_FLAMESPEAR, urand(12000, 16000));
                        break;
                }
            
                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Boss_SulfuronAI(creature);
    }
};

class Mob_Flamewaker_Priest : public CreatureScript
{
    public:
        Mob_Flamewaker_Priest() : CreatureScript("Mob_Flamewaker_Priest") {}

    class Mob_Flamewaker_PriestAI : public ScriptedAI
    {
        public:
            enum event
            {
                EV_DARKSTRIKE         = 1,
                EV_HEAL               ,
                EV_SHADOWWORDPAIN     ,
                EV_IMMOLATE           ,
            };

            Mob_Flamewaker_PriestAI(Creature* creature) : ScriptedAI(creature)
            {
                _instance = ((InstanceScript*)creature->GetInstanceScript());
            }

            EventMap events;

            void Reset()
            override {
                events.RescheduleEvent(EV_DARKSTRIKE, 10000);
                events.RescheduleEvent(EV_HEAL, 15000);
                events.RescheduleEvent(EV_SHADOWWORDPAIN, 1000);
                events.RescheduleEvent(EV_IMMOLATE, 5000);
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
                    case EV_DARKSTRIKE:
                        me->CastSpell(me, SPELL_DARKSTRIKE);
                        events.RescheduleEvent(EV_DARKSTRIKE, urand(15000, 18000));
                        break;
                    case EV_HEAL:
                    {
                        Unit* pUnit = DoSelectLowestHpFriendly(60.0f, 1);
                        if (pUnit)
                            me->CastSpell(pUnit, SPELL_HEAL);

                        events.RescheduleEvent(EV_HEAL, urand(15000, 20000));
                        break;
                    }
                    case EV_SHADOWWORDPAIN:
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true), SPELL_SHADOWWORDPAIN);
                        events.RescheduleEvent(EV_SHADOWWORDPAIN, 5000, 6000);
                        break;
                    case EV_IMMOLATE:
                        me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true), SPELL_IMMOLATE);
                        events.RescheduleEvent(EV_IMMOLATE, urand(5000, 6000));
                        break;
                }
            
                DoMeleeAttackIfReady();
            }

        private:
            InstanceScript* _instance;
    };

    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Mob_Flamewaker_PriestAI(creature);
    }
};

void AddSC_boss_sulfuron()
{
    new Boss_Sulfuron();
    new Mob_Flamewaker_Priest();
}

