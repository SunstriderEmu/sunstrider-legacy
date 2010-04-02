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
SDName: Boss_Magistrate_Barthilas
SD%Complete: 70
SDComment:
SDCategory: Stratholme
EndScriptData */

#include "precompiled.h"
#include "def_stratholme.h"

#define SPELL_DRAININGBLOW    16793
#define SPELL_CROWDPUMMEL    10887
#define SPELL_MIGHTYBLOW    14099
#define SPELL_FURIOUS_ANGER     16791

#define MODEL_NORMAL            10433
#define MODEL_HUMAN             3637

struct TRINITY_DLL_DECL boss_magistrate_barthilasAI : public ScriptedAI
{
    boss_magistrate_barthilasAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = (ScriptedInstance*)m_creature->GetInstanceData();
    }

    ScriptedInstance* pInstance;

    uint32 DrainingBlow_Timer;
    uint32 CrowdPummel_Timer;
    uint32 MightyBlow_Timer;
    uint32 FuriousAnger_Timer;
    uint32 AngerCount;
    uint32 escapeTimer;

    void Reset()
    {
        DrainingBlow_Timer = 20000;
        CrowdPummel_Timer = 15000;
        MightyBlow_Timer = 10000;
        FuriousAnger_Timer = 5000;
        AngerCount = 0;
        escapeTimer = 0;

        if (m_creature->isAlive())
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_NORMAL);
        else
            m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_HUMAN);
    }

    void MoveInLineOfSight(Unit *who)
    {
        /*if (ScriptedInstance *pInstance = ((ScriptedInstance*)m_creature->GetInstanceData())) {
            if (!pInstance->GetData(TYPE_ESCAPE_BARTH)) {
                escapeTimer = 5000;
                m_creature->SetSpeed(MOVE_WALK, 5.0f);
                m_creature->GetMotionMaster()->MovePoint(0, 3718.13, -3597.87, 142.05);
                pInstance->SetData(TYPE_ESCAPE_BARTH, IN_PROGRESS);
            }
        }*/

        ScriptedAI::MoveInLineOfSight(who);
    }

    void JustDied(Unit* Killer)
    {
        m_creature->SetUInt32Value(UNIT_FIELD_DISPLAYID, MODEL_HUMAN);
    }

    void Aggro(Unit *who)
    {
    }

    void UpdateAI(const uint32 diff)
    {
        if (escapeTimer) {
            if (escapeTimer <= diff) {
                m_creature->SummonCreature(10435, 4070.16, -3537.12, 123.10, M_PI, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 6000000);
                m_creature->DisappearAndDie();
                escapeTimer = 0;
            }
            else escapeTimer -= diff;
        }
        
        //Return since we have no target
        if (!UpdateVictim())
            return;

        if (FuriousAnger_Timer < diff)
        {
            FuriousAnger_Timer = 4000;
            if (AngerCount > 25)
                return;

            ++AngerCount;
            m_creature->CastSpell(m_creature,SPELL_FURIOUS_ANGER,false);
        }else FuriousAnger_Timer -= diff;

        //DrainingBlow
        if (DrainingBlow_Timer < diff)
        {
            DoCast(m_creature->getVictim(),SPELL_DRAININGBLOW);
            DrainingBlow_Timer = 15000;
        }else DrainingBlow_Timer -= diff;

        //CrowdPummel
        if (CrowdPummel_Timer < diff)
        {
            DoCast(m_creature->getVictim(),SPELL_CROWDPUMMEL);
            CrowdPummel_Timer = 15000;
        }else CrowdPummel_Timer -= diff;

        //MightyBlow
        if (MightyBlow_Timer < diff)
        {
            DoCast(m_creature->getVictim(),SPELL_MIGHTYBLOW);
            MightyBlow_Timer = 20000;
        }else MightyBlow_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_magistrate_barthilas(Creature *_Creature)
{
    return new boss_magistrate_barthilasAI (_Creature);
}

void AddSC_boss_magistrate_barthilas()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_magistrate_barthilas";
    newscript->GetAI = &GetAI_boss_magistrate_barthilas;
    newscript->RegisterSelf();
}

