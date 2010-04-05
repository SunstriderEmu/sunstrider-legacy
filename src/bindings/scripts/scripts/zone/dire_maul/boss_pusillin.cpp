/* Copyright (C) 2009 - 2010 WoWMania Core
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

#include "precompiled.h"
#include "def_dire_maul.h"

#define FACTION_FRIENDLY            35
#define FACTION_HOSTILE             90

enum ePusillinSpells {
    SPELL_SPIRIT_OF_RUNN        = 22735,
    SPELL_BLAST_WAVE            = 22424,
    SPELL_FIREBALL              = 15228,
    SPELL_FIREBLAST             = 14145
};

#define GOSSIP_TEMP     "[PH] Donne-moi cette clé !"

struct TRINITY_DLL_DECL boss_pusillinAI : public ScriptedAI
{
    boss_pusillinAI(Creature *c) : ScriptedAI(c) 
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    uint8 step;
    
    uint32 fireballTimer;
    uint32 fireblastTimer;
    uint32 blastwaveTimer;

    void Reset()
    {
        /*if (pInstance && pInstance->GetData(DATA_PUSILLIN_STEP) < 4) {
            step = 0;
            pInstance->SetData(DATA_PUSILLIN_STEP, 0);
        }
        else if (pInstance) {
            step = 4;
            pInstance->SetData(DATA_PUSILLIN_STEP, 4);
            m_creature->setFaction(FACTION_HOSTILE);
            // TODO: teleport him to last position
        }*/
        /*step = 0;
        if (pInstance) {
            step = pInstance->GetData(DATA_PUSILLIN_STEP);
            switch (step) {
            case 1:
                m_creature->SummonCreature(14354, -149.159958, -274.567322, -4.147923, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 6000000);
                m_creature->DisappearAndDie();
                break;
            case 2:
                m_creature->SummonCreature(14354, 111.177673, -352.949677, -4.101341, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 6000000);
                m_creature->DisappearAndDie();
                break;
            case 3:
                m_creature->SummonCreature(14354, 47.163609, -653.998535, -25.160894, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 6000000);
                m_creature->DisappearAndDie();
                break;
            case 4:
                if (Creature *pNewCre = m_creature->SummonCreature(14354, 17.699953, -704.177368, -12.642654, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 6000000)) {
                    pNewCre->setFaction(FACTION_HOSTILE);
                    pNewCre->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }
                m_creature->DisappearAndDie();
                break;
            }
        }*/
        
        fireballTimer = 6000;
        fireblastTimer = 8000;
        blastwaveTimer = 12000;
    }
    
    uint8 GetStep()
    {
        return step;
    }
    
    void SetStep(uint8 currStep)
    {
        step = currStep;
    }
    
    void Aggro(Unit *pWho)
    {
        DoCast(m_creature, SPELL_SPIRIT_OF_RUNN, false);
    }
    
    void UpdateAI(uint32 const diff)
    {
        if (!UpdateVictim())
            return;
            
        if (fireballTimer <= diff) {
            DoCast(m_creature->getVictim(), SPELL_FIREBALL, false);
            fireballTimer = 6000;
        }
        else
            fireballTimer -= diff;
            
        if (fireblastTimer <= diff) {
            DoCast(m_creature->getVictim(), SPELL_FIREBLAST, false);
            fireblastTimer = 8000;
        }
        else
            fireblastTimer -= diff;
            
        if (blastwaveTimer <= diff) {
            DoCast(m_creature->getVictim(), SPELL_BLAST_WAVE, false);
            blastwaveTimer = 15000;
        }
        else
            blastwaveTimer -= diff;
            
        DoMeleeAttackIfReady();
    }
};

bool GossipHello_boss_pusillin(Player *pPlayer, Creature *pCreature)
{
    uint8 step = ((boss_pusillinAI*)pCreature->AI())->GetStep();
    switch (step) {
    case 0:
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_TEMP, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        pPlayer->SEND_GOSSIP_MENU(2601, pCreature->GetGUID());
        break;
    case 1:
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_TEMP, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        pPlayer->SEND_GOSSIP_MENU(2601, pCreature->GetGUID());
        break;
    case 2:
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_TEMP, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
        pPlayer->SEND_GOSSIP_MENU(2601, pCreature->GetGUID());
        break;
    case 3:
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_TEMP, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
        pPlayer->SEND_GOSSIP_MENU(2601, pCreature->GetGUID());
        break;
    case 4:
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_TEMP, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
        pPlayer->SEND_GOSSIP_MENU(2601, pCreature->GetGUID());
        break;
    }
    
    return true;
}

bool GossipSelect_boss_pusillin(Player *pPlayer, Creature *pCreature, uint32 sender, uint32 action)
{
    switch (action) {
    case GOSSIP_ACTION_INFO_DEF + 1:
        pCreature->SetSpeed(MOVE_WALK, 8);
        pCreature->GetMotionMaster()->MovePoint(0, 21.937908, -198.946259, -4.132668);
        ((boss_pusillinAI*)pCreature->AI())->SetStep(1);
        break;
    case GOSSIP_ACTION_INFO_DEF + 2:
        pCreature->SetSpeed(MOVE_WALK, 8);
        pCreature->GetMotionMaster()->MovePoint(0, -135.910263, -349.494873, -4.070144);
        ((boss_pusillinAI*)pCreature->AI())->SetStep(2);
        break;
    case GOSSIP_ACTION_INFO_DEF + 3:
        pCreature->SetSpeed(MOVE_WALK, 8);
        pCreature->GetMotionMaster()->MovePoint(0, 112.385468, -469.288818, -2.719314);
        ((boss_pusillinAI*)pCreature->AI())->SetStep(3);
        break;
    case GOSSIP_ACTION_INFO_DEF + 4:
        pCreature->SetSpeed(MOVE_WALK, 8);
        pCreature->GetMotionMaster()->MovePoint(0, 25.040466, -696.303101, -25.160894);
        ((boss_pusillinAI*)pCreature->AI())->SetStep(4);
        break;
    case GOSSIP_ACTION_INFO_DEF + 5:
        // TODO: start fight
        break;
    }
    
    return true;
}

CreatureAI* GetAI_boss_pusillin(Creature *pCreature)
{
    return new boss_pusillinAI(pCreature);
}

void AddSC_boss_pusillin()
{
    Script *newscript;
    
    newscript = new Script;
    newscript->Name = "boss_pusillin";
    /*newscript->pGossipHello = &GossipHello_boss_pusillin;
    newscript->pGossipSelect = &GossipSelect_boss_pusillin;*/
    newscript->GetAI = &GetAI_boss_pusillin;
    newscript->RegisterSelf();
}
