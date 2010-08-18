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
SDName: Areatrigger_Scripts
SD%Complete: 100
SDComment: Scripts for areatriggers
SDCategory: Areatrigger
EndScriptData */

/* ContentData
at_legion_teleporter    4560 Teleporter TO Invasion Point: Cataclysm
at_test                 script test only
at_coilfang_waterfall   4591
at_mechanar             4614
at_botanica             4612
EndContentData */

#include "precompiled.h"

/*#####
## at_legion_teleporter
#####*/

#define SPELL_TELE_A_TO   37387
#define SPELL_TELE_H_TO   37389

bool AreaTrigger_at_legion_teleporter(Player *player, AreaTriggerEntry *at)
{
    if (player->isAlive() && !player->isInCombat())
    {
        if (player->GetTeam()== ALLIANCE && player->GetQuestRewardStatus(10589))
        {
            player->CastSpell(player,SPELL_TELE_A_TO,false);
            return true;
        }

        if (player->GetTeam()== HORDE && player->GetQuestRewardStatus(10604))
        {
            player->CastSpell(player,SPELL_TELE_H_TO,false);
            return true;
        }

        return false;
    }
    return false;
}

bool ATtest(Player *player, AreaTriggerEntry *at)
{
    player->Say("Hi!",LANG_UNIVERSAL);
    return true;
}

/*######
## at_coilfang_waterfall
######*/

enum
{
    GO_COILFANG_WATERFALL   = 184212
};

bool AreaTrigger_at_coilfang_waterfall(Player* pPlayer, AreaTriggerEntry* pAt)
{
    if (GameObject* pGo = pPlayer->FindGOInGrid(GO_COILFANG_WATERFALL, 50.0f))
    {
        if (pGo->getLootState() == GO_READY)
            pGo->UseDoorOrButton();
    }

    return false;
}

/*######
## at_scent_larkorwi
######*/

enum eScentLarkorwi
{
    QUEST_SCENT_OF_LARKORWI                     = 4291,
    NPC_LARKORWI_MATE                           = 9683
};

bool AreaTrigger_at_scent_larkorwi(Player* pPlayer, AreaTriggerEntry* pAt)
{
    if (!pPlayer->isDead() && pPlayer->GetQuestStatus(QUEST_SCENT_OF_LARKORWI) == QUEST_STATUS_INCOMPLETE)
    {
        if (!pPlayer->FindCreatureInGrid(NPC_LARKORWI_MATE,15,true))
            pPlayer->SummonCreature(NPC_LARKORWI_MATE, pPlayer->GetPositionX()+5, pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 3.3, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 100000);
    }


    return false;
}

/*######
## at_mechanar
######*/

#define DATA_PATHALEON  5

bool AreaTrigger_at_mechanar(Player* pPlayer, AreaTriggerEntry* pAt) {
    if (ScriptedInstance* pInstance = ((ScriptedInstance*)pPlayer->GetInstanceData())) {
        if (pInstance->GetData(DATA_PATHALEON) == DONE)
            pPlayer->TeleportTo(554, -25.639709, 0.213936, -1.812820, 3.076223);
        
        return true;
    }
    return false;
}

/*######
## at_botanica
######*/

bool AreaTrigger_at_botanica(Player *pPlayer, AreaTriggerEntry* pAt) {
    if (Creature *pWarpSplinter = pPlayer->FindCreatureInGrid(17977, 100.0f, false)) {
        pPlayer->TeleportTo(553, 41.069, -29.975, -1.12, 5.49);
    }
    
    return true;
}

void AddSC_areatrigger_scripts()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name = "at_legion_teleporter";
    newscript->pAreaTrigger = &AreaTrigger_at_legion_teleporter;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="at_test";
    newscript->pAreaTrigger = &ATtest;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "at_coilfang_waterfall";
    newscript->pAreaTrigger = &AreaTrigger_at_coilfang_waterfall;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "at_scent_larkorwi";
    newscript->pAreaTrigger = &AreaTrigger_at_scent_larkorwi;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "at_mechanar";
    newscript->pAreaTrigger = &AreaTrigger_at_mechanar;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "at_botanica";
    newscript->pAreaTrigger = &AreaTrigger_at_botanica;
    newscript->RegisterSelf();
}

