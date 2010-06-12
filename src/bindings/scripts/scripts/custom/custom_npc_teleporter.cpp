/* Custom Script - WoWManiaCore
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
SDName: Custom_Example
SD%Complete: 100
SDComment: NPC teleporter to help organizing events
SDCategory: Custom Scripts
EndScriptData */

#include "precompiled.h"
#include <cstring>

/*######
## npc_teleporter
######*/

bool GossipHello_npc_teleporter(Player *pPlayer, Creature *pCreature)
{
    pPlayer->ADD_GOSSIP_ITEM(0, "Je souhaite être téléporté.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    
    if (pPlayer->isGameMaster()) //add an option to check guid of spawned arrivals
        pPlayer->ADD_GOSSIP_ITEM(0, "Quels sont les GUIDs des PNJ d'arrivée spawn ?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        
    pPlayer->PlayerTalkClass->SendGossipMenu(907,pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_teleporter(Player *pPlayer, Creature *pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF)
    {
        if (pPlayer->HasLevelInRangeForTeleport() || pPlayer->isGameMaster()) {
            uint32 destEntry = pPlayer->GetTeam() == HORDE ? 42 : 44; //depends on player's team
            
            //get coordinates of target in DB
            QueryResult* result = WorldDatabase.PQuery("SELECT map, position_x, position_y, position_z, orientation FROM creature WHERE id = %u LIMIT 1", destEntry);
            
            uint32 mapId;
            float destX;
            float destY;
            float destZ;
            float destO;
            if (result)
            {
                Field *fields = result->Fetch();
                mapId = fields[0].GetUInt32();
                destX = fields[1].GetFloat();
                destY = fields[2].GetFloat();
                destZ = fields[3].GetFloat();
                destO = fields[4].GetFloat();
            
                Trinity::NormalizeMapCoord(destX);
            
                pPlayer->TeleportTo(mapId, destX, destY, destZ, destO);
            }
            else
            {
                pCreature->Whisper("Pas de PNJ d'arrivée.", pPlayer->GetGUID());
                pPlayer->PlayerTalkClass->CloseGossip();
                return false;
            }
        }
        else {
            pCreature->Whisper("Votre niveau ne correspond pas.", pPlayer->GetGUID());
            pPlayer->PlayerTalkClass->CloseGossip();
            return false;
        }
    }
    
    if (action == GOSSIP_ACTION_INFO_DEF+1 && pPlayer->isGameMaster()) //double check
    {
        QueryResult *result = WorldDatabase.PQuery("SELECT guid FROM creature WHERE id = 42 OR id = 44");
        
        if (!result)
        {
            pCreature->Whisper("No destination NPC found.", pPlayer->GetGUID());
            pPlayer->PlayerTalkClass->CloseGossip();
            return false;
        }
        else
        {
            do
            {
                Field *fields = result->Fetch();
                std::string guid = fields[0].GetCppString();
                pCreature->Whisper(guid.c_str(), pPlayer->GetGUID());
            }while (result->NextRow());
            
            pPlayer->PlayerTalkClass->CloseGossip();
        }
    }
}

/*######
## AddSC
######*/

void AddSC_npc_teleporter()
{
    Script *newscript;
    
    newscript = new Script;
    newscript->Name = "npc_teleporter";
    newscript->pGossipHello = &GossipHello_npc_teleporter;
    newscript->pGossipSelect = &GossipSelect_npc_teleporter;
    newscript->RegisterSelf();
}
