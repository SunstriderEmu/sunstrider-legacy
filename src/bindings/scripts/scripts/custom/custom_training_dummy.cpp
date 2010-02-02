/* Copyright (C) 2006 - 2008 WoWMania Core <https://scriptdev2.svn.sourceforge.net/>
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
SDName: npc_training_dummy
SD%Complete: 100
SDComment: Custom NPC Training Dummy, like in Wotlk
SDCategory: Custom
EndScriptData */

#include "precompiled.h"

struct TRINITY_DLL_DECL npc_training_dummy : Scripted_NoMovementAI
{
    npc_training_dummy(Creature *c) : Scripted_NoMovementAI(c)
    {
        m_Entry = c->GetEntry();
    }

    uint64 m_Entry;
    uint32 ResetTimer;
    uint32 DespawnTimer;
    
    void Reset()
    {
        m_creature->SetControlled(true,UNIT_STAT_STUNNED);      //disable rotate
        m_creature->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);        //imune to knock aways like blast wave
        m_creature->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
        ResetTimer = 8000;
        DespawnTimer = 15000;
    }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        ResetTimer = 8000;
        damage = 0;
    }

    void Aggro(Unit *who)
    {
        if (m_Entry != 2674 && m_Entry != 2673)
            return;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;
        
        if (!m_creature->hasUnitState(UNIT_STAT_STUNNED))
            m_creature->SetControlled(true,UNIT_STAT_STUNNED);      //disable rotate
            
        if (!m_creature->hasUnitState(UNIT_STAT_ROOT))
            m_creature->SetControlled(true,UNIT_STAT_ROOT);
            
        m_creature->SetSpeed(MOVE_RUN, 0.0f);

        if (m_Entry != 2674 && m_Entry != 2673)
        {
            if (ResetTimer <= diff)
            {
                //EnterEvadeMode();
                m_creature->RemoveAllAuras();
                m_creature->DeleteThreatList();
                m_creature->CombatStop();
                m_creature->LoadCreaturesAddon();
                m_creature->SetLootRecipient(NULL);
                m_creature->ResetPlayerDamageReq();
                ResetTimer = 8000;
            }
            else
                ResetTimer -= diff;
            return;
        }
        else
        {
            if (DespawnTimer <= diff)
                m_creature->ForcedDespawn();
            else
                DespawnTimer -= diff;
        }
    }
    
    void MoveInLineOfSight(Unit *who){return;}
};

CreatureAI* GetAI_npc_training_dummy(Creature* pCreature)
{
    return new npc_training_dummy(pCreature);
}

void AddSC_training_dummy()
{
    Script* newscript;
    
    newscript = new Script;
    newscript->Name = "npc_training_dummy";
    newscript->GetAI = &GetAI_npc_training_dummy;
    newscript->RegisterSelf();
}
