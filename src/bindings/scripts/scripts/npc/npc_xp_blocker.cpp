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
SDName: npc_xp_blocker
SD%Complete: 100
SDComment: Block/Unblock XP for characters (to allow twinks)
SDCategory: NPCs
EndScriptData
*/

/* ContentData
npc_xp_blocker
EndContentData */

#include "precompiled.h"

#define GOSSIP_BLOCK_XP     "Je souhaite bloquer ma barre d'expérience."
#define GOSSIP_UNBLOCK_XP   "Je souhaite débloquer ma barre d'expérience."

#define BLOCK_XP_PRICE      100000       // 10 gold

bool GossipHello_npc_xp_blocker(Player* pPlayer, Creature* pCreature) {
    if (pPlayer->IsXpBlocked())
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_UNBLOCK_XP, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    else
        pPlayer->ADD_GOSSIP_ITEM_EXTENDED(0, GOSSIP_BLOCK_XP, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2, "Confirmer le bloquage d'expérience ?", BLOCK_XP_PRICE, false);
        
    pPlayer->SEND_GOSSIP_MENU(42, pCreature->GetGUID());
    
    return true;
}

bool GossipSelect_npc_xp_blocker(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action) {
    switch (action) {
    case GOSSIP_ACTION_INFO_DEF+1:      // Unblock, free
        pPlayer->SetXpBlocked(false);
        pCreature->Whisper("Expérience débloquée.", pPlayer->GetGUID());
        pPlayer->SaveToDB();
        break;
    case GOSSIP_ACTION_INFO_DEF+2:      // Block, 10 gold
        if (pPlayer->GetMoney() > BLOCK_XP_PRICE) {
            pPlayer->SetXpBlocked(true);
            pCreature->Whisper("Expérience bloquée.", pPlayer->GetGUID());
            pPlayer->ModifyMoney(-(int32)BLOCK_XP_PRICE);
            pPlayer->SaveToDB();
        }
        else
            pCreature->Whisper("Vous n'avez pas assez d'argent !", pPlayer->GetGUID());
        break;
    }
    
    pPlayer->PlayerTalkClass->CloseGossip();
    
    return true;
}

void AddSC_npc_xp_blocker()
{
    Script* newscript;
    
    newscript = new Script;
    newscript->Name = "npc_xp_blocker";
    newscript->pGossipHello = &GossipHello_npc_xp_blocker;
    newscript->pGossipSelect = &GossipSelect_npc_xp_blocker;
    newscript->RegisterSelf();
}
