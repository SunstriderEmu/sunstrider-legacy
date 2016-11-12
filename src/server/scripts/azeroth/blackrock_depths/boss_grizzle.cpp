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
SDName: Boss_Grizzle
SD%Complete: 100
SDComment:
SDCategory: Blackrock Depths
EndScriptData */



enum Spells
{
    SPELL_GROUNDTREMOR                                     = 6524,
    SPELL_FRENZY                                           = 28371
};

struct boss_grizzleAI : public ScriptedAI
{
    boss_grizzleAI(Creature *c) : ScriptedAI(c) {}

    uint32 GroundTremor_Timer;
    uint32 Frenzy_Timer;

    void Reset()
    override {
        GroundTremor_Timer = 12000;
        Frenzy_Timer = 0;
    }

    void EnterCombat(Unit *who)
    override {
    }

    void UpdateAI(const uint32 diff)
    override {
        //Return since we have no target
        if (!UpdateVictim() )
            return;

        //GroundTremor_Timer
        if (GroundTremor_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_GROUNDTREMOR);
            GroundTremor_Timer = 8000;
        }else GroundTremor_Timer -= diff;

        //Frenzy_Timer
        if ( me->GetHealth()*100 / me->GetMaxHealth() < 51 )
        {
            if (Frenzy_Timer < diff)
            {
                DoCast(me,SPELL_FRENZY);
                me->TextEmote("goes into a killing frenzy!",nullptr);

                Frenzy_Timer = 15000;
            }else Frenzy_Timer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_grizzle(Creature *pCreature)
{
    return new boss_grizzleAI (pCreature);
}

void AddSC_boss_grizzle()
{
    OLDScript *newscript;
    newscript = new OLDScript;
    newscript->Name="boss_grizzle";
    newscript->GetAI = &GetAI_boss_grizzle;
    sScriptMgr->RegisterOLDScript(newscript);
}

