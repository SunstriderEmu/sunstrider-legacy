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

//#define EMOTE_FRENZY                -1409001

enum {
    SPELL_FRENZY                = 19451,
    SPELL_MAGMASPIT             = 19449,
    SPELL_LAVABREATH            = 19272,
    SPELL_PANIC                 = 19408,
    SPELL_LAVABOMB              = 19411,                   //This calls a dummy server side effect that isn't implemented yet
    SPELL_LAVABOMB_ALT          = 19428                   //This is the spell that the lava bomb casts
};

class Boss_Magmadar : public CreatureScript
{
public:
    Boss_Magmadar() : CreatureScript("boss_magmadar") {}
    
    class Boss_MagmadarAI : public CreatureAI
    {
    public:
        enum event {
            EV_LAVABREATH       = 1,
            EV_PANIC            ,
            EV_LAVABOMB         ,
            EV_FRENZY           ,
        };
        
        Boss_MagmadarAI(Creature* creature) : CreatureAI(creature)
        {
            _instance = ((InstanceScript*)creature->GetInstanceScript());
        }
        
        EventMap events;

        void Reset()
        override {
            events.RescheduleEvent(EV_LAVABREATH, urand(25000, 30000));
            events.RescheduleEvent(EV_PANIC, urand(25000, 30000));
            events.RescheduleEvent(EV_LAVABOMB, urand(8000, 12000));
            events.RescheduleEvent(EV_FRENZY, urand(19000, 21000));
            
            if (_instance)
                _instance->SetData(DATA_MAGMADAR, NOT_STARTED);
            
            me->CastSpell(me, SPELL_MAGMASPIT, true);
        }
        
        void EnterCombat(Unit* /*victim*/)
        override {
            if (_instance)
                _instance->SetData(DATA_MAGMADAR, IN_PROGRESS);
        }
        
        void JustDied(Unit* /*killer*/)
        override {
            if (_instance)
                _instance->SetData(DATA_MAGMADAR, DONE);
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
                case EV_LAVABREATH:
                    me->CastSpell(me->GetVictim(), SPELL_LAVABREATH);
                    events.RescheduleEvent(EV_LAVABREATH, urand(25000, 30000));
                    break;
                case EV_PANIC:
                    me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_PANIC);
                    events.RescheduleEvent(EV_PANIC, urand(25000, 30000));
                    break;
                case EV_LAVABOMB: // FIXME
                    me->CastSpell(SelectTarget(SELECT_TARGET_RANDOM, 0), SPELL_LAVABOMB);
                    events.RescheduleEvent(EV_LAVABOMB, urand(8000, 12000));
                    break;
                case EV_FRENZY:
                    me->CastSpell(me, SPELL_FRENZY);
                    events.RescheduleEvent(EV_FRENZY, urand(19000, 21000));
                    break;
            }
            
            DoMeleeAttackIfReady();
        }
        
    private:
        InstanceScript* _instance;
    };
    
    CreatureAI* GetAI(Creature* creature) const
    override {
        return new Boss_MagmadarAI(creature);
    }
};

void AddSC_boss_magmadar()
{
    new Boss_Magmadar();
}

