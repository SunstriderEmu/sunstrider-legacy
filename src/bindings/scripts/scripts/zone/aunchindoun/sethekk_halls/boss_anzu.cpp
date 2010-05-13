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
SDName: Boss_Anzu
SD%Complete: 80
SDComment:
SDCategory: Auchindoun, Sethekk Halls
EndScriptData */

#include "precompiled.h"
#include "def_sethekk_halls.h"

enum eAnzu
{
    SPELL_CHARGE            = 22120,
    SPELL_BANISH            = 42354,
    SPELL_PARALYSING        = 40184,
    SPELL_FEATHER_CYCLONE   = 40321,
    SPELL_SPELL_BOMB        = 40303,
    
    CREATURE_BROOD          = 23132
};


struct TRINITY_DLL_DECL boss_anzuAI : public ScriptedAI
{
    boss_anzuAI(Creature *c) : ScriptedAI(c), summons(m_creature)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
    }
    
    ScriptedInstance *pInstance;
    
    uint32 chargeTimer;
    uint32 paralysingTimer;
    uint32 featherCycloneTimer;
    uint32 spellBombTimer;
    
    bool hasSummoned66;
    bool hasSummoned33;
    
    SummonList summons;
    
    void Reset()
    {
        if (pInstance && m_creature->isAlive())
            pInstance->SetData(ANZU_EVENT, NOT_STARTED);
            
        chargeTimer = 9000;
        paralysingTimer = 14000;
        featherCycloneTimer = 5000;
        spellBombTimer = 22000;
        
        hasSummoned66 = false;
        hasSummoned33 = false;
        
        if (m_creature->HasAura(SPELL_BANISH))
            m_creature->RemoveAurasDueToSpell(SPELL_BANISH);
            
        summons.DespawnAll();
    }
    
    void JustDied(Unit *pKiller)
    {
        if (pInstance)
            pInstance->SetData(ANZU_EVENT, DONE);
    }
    
    void Aggro(Unit *pWho) {}
    
    void JustSummoned(Creature* pSummon)
    {
        summons.Summon(pSummon);
        
        if (Unit *target = SelectUnit(SELECT_TARGET_RANDOM,0))
            pSummon->AI()->AttackStart(target);
    }
    
    void SummonedCreatureDespawn(Creature* pSummon) { summons.Despawn(pSummon); }
    
    void UpdateAI(uint32 const diff)
    {
        if (!UpdateVictim())
            return;
            
        if (m_creature->HasAura(SPELL_BANISH))
            return;
            
        if (chargeTimer <= diff) {
            if (rand()%10 < 8) {
                DoCast(SelectUnit(SELECT_TARGET_RANDOM, 1), SPELL_CHARGE);
                chargeTimer = 9000;
            }
        } else chargeTimer -= diff;
        
        if (paralysingTimer <= diff) {
            DoCast(m_creature->getVictim(), SPELL_PARALYSING);
            paralysingTimer = 26000;
        } else paralysingTimer -= diff;
        
        if (featherCycloneTimer <= diff) {
            DoCast(SelectUnit(SELECT_TARGET_RANDOM, 1), SPELL_FEATHER_CYCLONE);
            featherCycloneTimer = 21000;
        } else featherCycloneTimer -= diff;
        
        if (spellBombTimer <= diff) {
            DoCast(SelectUnit(SELECT_TARGET_RANDOM, 0), SPELL_SPELL_BOMB);
            spellBombTimer = 30000;
        } else spellBombTimer -= diff;
        
        if (m_creature->GetHealth() < m_creature->GetMaxHealth()*0.66 && !hasSummoned66) {
            m_creature->InterruptNonMeleeSpells(true);
            DoCast(m_creature, SPELL_BANISH, true);
            int count;
            for (count = 0; count < 5; count++)
                m_creature->SummonCreature(CREATURE_BROOD, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 90000);
            hasSummoned66 = true;
        }
        
        if (m_creature->GetHealth() < m_creature->GetMaxHealth()*0.33 && !hasSummoned33) {
            m_creature->InterruptNonMeleeSpells(true);
            DoCast(m_creature, SPELL_BANISH, true);
            int count;
            for (count = 0; count < 5; count++)
                m_creature->SummonCreature(CREATURE_BROOD, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 90000);
            hasSummoned33 = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_anzu(Creature *pCreature)
{
    return new boss_anzuAI(pCreature);
}

void AddSC_boss_anzu()
{
    Script *newscript;
    
    newscript = new Script;
    newscript->Name = "boss_anzu";
    newscript->GetAI = &GetAI_boss_anzu;
    newscript->RegisterSelf();
}
