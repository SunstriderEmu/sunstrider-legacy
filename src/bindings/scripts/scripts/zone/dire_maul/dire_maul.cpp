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


/* ScriptData
SDName: Dire_Maul
SD%Complete: 0
SDComment: 
SDCategory: Dire_Maul
EndScriptData */

#include "precompiled.h"
#include "def_dire_maul.h"

/*######
## npc_ironbark_redeemed
######*/

struct npc_ironbark_redeemedAI : public ScriptedAI
{
    npc_ironbark_redeemedAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
    }
    
    ScriptedInstance *pInstance;
    
    void Reset() {}
    void Aggro(Unit *pWho) {}
    
    void MovementInform(uint32 type, uint32 id)
    {
        if (id == 2) {
            if (pInstance) {
                if (GameObject *pIronbarkDoor = pInstance->instance->GetGameObjectInMap(pInstance->GetData64(DATA_GUID_IRONBARKDOOR))) {
                    pIronbarkDoor->SwitchDoorOrButton(true);
                    m_creature->Kill(m_creature);
                }
            }
        }
    }
};

CreatureAI* GetAI_npc_ironbark_redeemed(Creature *pCreature)
{
    return new npc_ironbark_redeemedAI(pCreature);
}

bool GossipHello_npc_ironbark_redeemed(Player *pPlayer, Creature *pCreature)
{
    if (ScriptedInstance *pInstance = ((ScriptedInstance*)pPlayer->GetInstanceData())) {
        if (pInstance->GetData(DATA_ZEVRIM_THORNHOOF) == DONE) {
            pPlayer->ADD_GOSSIP_ITEM(0, "[PH] Ouvre la porte !", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
            pPlayer->SEND_GOSSIP_MENU(2601, pCreature->GetGUID());
            return true;
        }
    }
    return false;
}

bool GossipSelect_npc_ironbark_redeemed(Player *pPlayer, Creature *pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF) {
        pCreature->SetSpeed(MOVE_WALK, 3);
        pCreature->GetMotionMaster()->MovePath(14241, false);
        return true;
    }
    return false;
}

void AddSC_dire_maul()
{
    Script *newscript;
    
    newscript = new Script;
    newscript->Name = "npc_ironbark_redeemed";
    newscript->GetAI = &GetAI_npc_ironbark_redeemed;
    newscript->pGossipHello = &GossipHello_npc_ironbark_redeemed;
    newscript->pGossipSelect = &GossipSelect_npc_ironbark_redeemed;
    newscript->RegisterSelf();
}
