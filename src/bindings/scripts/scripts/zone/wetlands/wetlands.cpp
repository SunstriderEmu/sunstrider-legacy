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
SDName: Wetlands
SD%Complete: 100
SDComment: Quest support: 1249
SDCategory: Wetlands
EndScriptData */

/* ContentData
npc_tapoke_jahn
EndContentData */

#include "precompiled.h"

/*######
## npc_tapoke_jahn
######*/

#define QUEST_MISSING_DIPLOMAT  1249

#define FACTION_FRIENDLY        123
#define FACTION_UNFRIENDLY      14

#define SPELL_PUMMEL            12555

struct TRINITY_DLL_DECL npc_tapoke_jahnAI : public ScriptedAI
{
    npc_tapoke_jahnAI(Creature* c) : ScriptedAI(c) {}
    
    Player* player;
    
    uint32 pummelTimer;
    
    void Reset()
    {
        m_creature->setFaction(FACTION_FRIENDLY);
        m_creature->SetHealth(m_creature->GetMaxHealth());
        m_creature->CombatStop();
        m_creature->DeleteThreatList();
        
        pummelTimer = 5000;
    }
    
    void Aggro(Unit* who) {}
    
    void UpdateAI(const uint32 diff)
    {
        if (m_creature->getFaction() == FACTION_FRIENDLY) //if friendly, event is not running
            return;
        
        if (m_creature->GetHealth() < (m_creature->GetMaxHealth()/5.0f)) //at 20%, he stops fighting and complete the quest
        {
            player = (m_creature->getVictim()->ToPlayer());
            
            if (player && player->GetQuestStatus(QUEST_MISSING_DIPLOMAT) == QUEST_STATUS_INCOMPLETE)
                player->KilledMonster(4962, m_creature->GetGUID());
            
            m_creature->MonsterSay("Arretez, je vais tout vous dire...", LANG_UNIVERSAL, 0);
            Reset();
            
            return;
        }
        
        if (pummelTimer <= diff)
        {
            DoCast(m_creature->getVictim(), SPELL_PUMMEL);
            pummelTimer = 20000;
        }else pummelTimer -= diff;
        
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_tapoke_jahn(Creature *pCreature)
{
    return new npc_tapoke_jahnAI(pCreature);
}

bool GossipHello_npc_tapoke_jahn(Player *pPlayer, Creature *pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_MISSING_DIPLOMAT) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(0, "A nous deux, dites-moi tout !", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        
    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());
    
    return true;
}

bool GossipSelect_npc_tapoke_jahn(Player *pPlayer, Creature *pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        pCreature->MonsterSay("Je n'ai rien a vous dire !", LANG_UNIVERSAL, 0);
        pCreature->setFaction(FACTION_UNFRIENDLY);
        pCreature->AI()->AttackStart(pPlayer);
    }
    
    return true;
}

/*######
## AddSC
######*/

void AddSC_wetlands()
{
    Script *newscript;
    
    newscript = new Script;
    newscript->Name="npc_tapoke_jahn";
    newscript->pGossipHello =  &GossipHello_npc_tapoke_jahn;
    newscript->pGossipSelect = &GossipSelect_npc_tapoke_jahn;
    newscript->GetAI = &GetAI_npc_tapoke_jahn;
    newscript->RegisterSelf();
}
