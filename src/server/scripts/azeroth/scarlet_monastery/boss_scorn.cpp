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
SDName: Boss_Scorn
SD%Complete: 100
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */



#define SPELL_LICHSLAP                  28873
#define SPELL_FROSTBOLTVOLLEY           8398
#define SPELL_MINDFLAY                  17313
#define SPELL_FROSTNOVA                 15531

struct boss_scornAI : public ScriptedAI
{
    boss_scornAI(Creature *c) : ScriptedAI(c) {}

    uint32 LichSlap_Timer;
    uint32 FrostboltVolley_Timer;
    uint32 MindFlay_Timer;
    uint32 FrostNova_Timer;

    void Reset()
    override {
        LichSlap_Timer = 45000;
        FrostboltVolley_Timer = 30000;
        MindFlay_Timer = 30000;
        FrostNova_Timer = 30000;
    }

    void EnterCombat(Unit *who)
    override {
    }

    void UpdateAI(const uint32 diff)
    override {
        if (!UpdateVictim())
            return;

        //LichSlap_Timer
        if (LichSlap_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_LICHSLAP);
            LichSlap_Timer = 45000;
        }else LichSlap_Timer -= diff;

        //FrostboltVolley_Timer
        if (FrostboltVolley_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_FROSTBOLTVOLLEY);
            FrostboltVolley_Timer = 20000;
        }else FrostboltVolley_Timer -= diff;

        //MindFlay_Timer
        if (MindFlay_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_MINDFLAY);
            MindFlay_Timer = 20000;
        }else MindFlay_Timer -= diff;

        //FrostNova_Timer
        if (FrostNova_Timer < diff)
        {
            DoCast(me->GetVictim(),SPELL_FROSTNOVA);
            FrostNova_Timer = 15000;
        }else FrostNova_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_scorn(Creature *_Creature)
{
    return new boss_scornAI (_Creature);
}

void AddSC_boss_scorn()
{
    OLDScript *newscript;
    newscript = new OLDScript;
    newscript->Name="boss_scorn";
    newscript->GetAI = &GetAI_boss_scorn;
    sScriptMgr->RegisterOLDScript(newscript);
}

