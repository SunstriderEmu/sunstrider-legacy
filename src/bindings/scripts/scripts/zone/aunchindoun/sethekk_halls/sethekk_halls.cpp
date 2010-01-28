/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* ScriptData
SDName: SethekkHalls
SD%Complete: 100
SDComment: Lakka's gossip to complete quest 10097
SDCategory: Sethekk Halls
EndScriptData */

/* ContentData
npc_lakka
go_ravens_claw
EndContentData */

#include "precompiled.h"
#include "def_sethekk_halls.h"

/*######
## npc_lakka
######*/

#define QUEST_BRO_AGAINST_BRO   10097
#define GOSSIP_FREE_LAKKA       "You're free, Lakka!"

bool GossipHello_npc_lakka(Player *player, Creature *_Creature)
{
    if( player->GetQuestStatus(QUEST_BRO_AGAINST_BRO) == QUEST_STATUS_INCOMPLETE )
    {
        player->ADD_GOSSIP_ITEM(0, GOSSIP_FREE_LAKKA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    }

    return true;
}

bool GossipSelect_npc_lakka(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if( action == GOSSIP_ACTION_INFO_DEF+1 )
    {
        player->KilledMonster(18956,_Creature->GetGUID());
        player->CLOSE_GOSSIP_MENU();
    }

    return true;
}

/*######
## go_ravens_claw
######*/

bool hasDespawned = false;

bool GOHello_ravens_claw(Player* pPlayer, GameObject* pGo)
{
    if (ScriptedInstance* pInstance = ((ScriptedInstance*)pGo->GetInstanceData())) {
        if (pInstance->GetData64(ANZU_SUMMONER) != 0 && pPlayer->GetGUID() != pInstance->GetData64(ANZU_SUMMONER)) {      // Hehe, two different players summoning Anzu ? We aren't going to be friends...
            pPlayer->DealDamage(pPlayer, pPlayer->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            
            if (Creature* secondAnzu = pGo->FindCreatureInGrid(23035, 30.0f, true)) {
                if (!hasDespawned) {
                    secondAnzu->DisappearAndDie();
                    hasDespawned = true;
                }
            }
        }
        else
            pInstance->SetData64(ANZU_SUMMONER, pPlayer->GetGUID());
        return false;
    }
    return true;
}

/*######
## AddSC
######*/

void AddSC_sethekk_halls()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_lakka";
    newscript->pGossipHello = &GossipHello_npc_lakka;
    newscript->pGossipSelect = &GossipSelect_npc_lakka;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_ravens_claw";
    newscript->pGOHello = &GOHello_ravens_claw;
    newscript->RegisterSelf();
}
