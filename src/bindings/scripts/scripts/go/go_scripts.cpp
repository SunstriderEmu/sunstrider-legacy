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
SDName: GO_Scripts
SD%Complete: 100
SDComment: Quest support: 4285,4287,4288(crystal pylons), 4296. Field_Repair_Bot->Teaches spell 22704. Barov_journal->Teaches spell 26089
SDCategory: Game Objects
EndScriptData */

/* ContentData
go_northern_crystal_pylon
go_eastern_crystal_pylon
go_western_crystal_pylon
go_barov_journal
go_field_repair_bot_74A
go_orb_of_command
go_tablet_of_madness
go_tablet_of_the_seven
go_teleporter
go_fel_crystalforge
go_bashir_crystalforge
go_shrine_hawk
go_southfury_moonstone
go_warmaul_prison
go_green_spot_grog_keg
go_ripe_moonshine_keg
go_fermented_seed_beer_keg
go_practice_lockbox
go_crystalline_tear
go_blacksmithing_plans
go_testing_equipment
go_forged_illidari_blade
EndContentData */

#include "precompiled.h"

/*######
## go_crystal_pylons (3x)
######*/

bool GOHello_go_northern_crystal_pylon(Player *player, GameObject* _GO)
{
    if (_GO->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
    {
        player->PrepareQuestMenu(_GO->GetGUID());
        player->SendPreparedQuest(_GO->GetGUID());
    }

    if (player->GetQuestStatus(4285) == QUEST_STATUS_INCOMPLETE)
        player->AreaExploredOrEventHappens(4285);

    return true;
}

bool GOHello_go_eastern_crystal_pylon(Player *player, GameObject* _GO)
{
    if (_GO->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
    {
        player->PrepareQuestMenu(_GO->GetGUID());
        player->SendPreparedQuest(_GO->GetGUID());
    }

    if (player->GetQuestStatus(4287) == QUEST_STATUS_INCOMPLETE)
        player->AreaExploredOrEventHappens(4287);

    return true;
}

bool GOHello_go_western_crystal_pylon(Player *player, GameObject* _GO)
{
    if (_GO->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
    {
        player->PrepareQuestMenu(_GO->GetGUID());
        player->SendPreparedQuest(_GO->GetGUID());
    }

    if (player->GetQuestStatus(4288) == QUEST_STATUS_INCOMPLETE)
        player->AreaExploredOrEventHappens(4288);

    return true;
}

/*######
## go_barov_journal
######*/

bool GOHello_go_barov_journal(Player *player, GameObject* _GO)
{
    if(player->HasSkill(SKILL_TAILORING) && player->GetBaseSkillValue(SKILL_TAILORING) >= 280 && !player->HasSpell(26086))
    {
        player->CastSpell(player,26095,false);
    }
    return true;
}

/*######
## go_field_repair_bot_74A
######*/

bool GOHello_go_field_repair_bot_74A(Player *player, GameObject* _GO)
{
    if(player->HasSkill(SKILL_ENGINERING) && player->GetBaseSkillValue(SKILL_ENGINERING) >= 300 && !player->HasSpell(22704))
    {
        player->CastSpell(player,22864,false);
    }
    return true;
}

/*######
## go_orb_of_command
######*/

bool GOHello_go_orb_of_command(Player *player, GameObject* _GO)
{
    if( player->GetQuestRewardStatus(7761) )
        player->TeleportTo(469, -7673.03, -1106.08, 396.65, 0.7);

    return true;
}

/*######
## go_tablet_of_madness
######*/

bool GOHello_go_tablet_of_madness(Player *player, GameObject* _GO)
{
    if (player->HasSkill(SKILL_ALCHEMY) && player->GetSkillValue(SKILL_ALCHEMY) >= 300 && !player->HasSpell(24266))
    {
        player->CastSpell(player,24267,false);
    }
    return true;
}

/*######
## go_tablet_of_the_seven
######*/

//TODO: use gossip option ("Transcript the Tablet") instead, if Trinity adds support.
bool GOHello_go_tablet_of_the_seven(Player *player, GameObject* _GO)
{
    if (_GO->GetGoType() != GAMEOBJECT_TYPE_QUESTGIVER)
        return true;

    if (player->GetQuestStatus(4296) == QUEST_STATUS_INCOMPLETE)
        player->CastSpell(player,15065,false);

    return true;
}

/*######
## go_teleporter
######*/

bool GOHello_go_teleporter(Player *player, GameObject* _GO)
{
    player->TeleportTo(0, 1807.07f,336.105f,70.3975f,0.0f);
    return false;
}

/*#####
## go_jump_a_tron
######*/

bool GOHello_go_jump_a_tron(Player *player, GameObject* _GO)
{
    if (player->GetQuestStatus(10111) == QUEST_STATUS_INCOMPLETE)
     player->CastSpell(player,33382,true);

    return true;
}

/*######
## go_ethereum_prison
######*/

float ethereum_NPC[2][7] =
{
 {20785,20790,20789,20784,20786,20783,20788}, // hostile npc
 {22810,22811,22812,22813,22814,22815,0}      // fiendly npc (need script in acid ? only to cast spell reputation reward)
};

bool GOHello_go_ethereum_prison(Player *player, GameObject* _GO)
{
 _GO->SetGoState(0);
 switch(rand()%2){
    case 0:
        _GO->SummonCreature(ethereum_NPC[0][rand()%6],_GO->GetPositionX(),_GO->GetPositionY(),_GO->GetPositionZ()+0.3, 0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,10000);
    break;
    case 1:
        _GO->SummonCreature(ethereum_NPC[1][rand()%5],_GO->GetPositionX(),_GO->GetPositionY(),_GO->GetPositionZ()+0.3, 0,TEMPSUMMON_TIMED_DESPAWN,10000);
    break;
}
return true;
}

/*######
## go_sacred_fire_of_life
######*/

#define NPC_ARIKARA  10882

bool GOHello_go_sacred_fire_of_life(Player* pPlayer, GameObject* pGO)
{
    if (pGO->GetGoType() == GAMEOBJECT_TYPE_GOOBER)
        pPlayer->SummonCreature(NPC_ARIKARA, -5008.338, -2118.894, 83.657, 0.874, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);

    return true;
}

/*######
## go_fel_crystalforge
######*/

#define GOSSIP_FEL_CRYSTALFORGE_TEXT 31000
#define GOSSIP_FEL_CRYSTALFORGE_ITEM_TEXT_RETURN 31001
#define GOSSIP_FEL_CRYSTALFORGE_ITEM_1 "Acheter 1 Flacon de la Bete Instable pour 10 Eclats Apogides."
#define GOSSIP_FEL_CRYSTALFORGE_ITEM_5 "Acheter 5 Flacons de la Bete Instables pour 50 Eclats Apogides."
#define GOSSIP_FEL_CRYSTALFORGE_ITEM_RETURN "Utiliser la cristalforge gangrenee pour un autre achat."

enum
{
    SPELL_CREATE_1_FLASK_OF_BEAST   = 40964,
    SPELL_CREATE_5_FLASK_OF_BEAST   = 40965,
    APEXIS_SHARD                    = 32569,
};

bool GOHello_go_fel_crystalforge(Player* pPlayer, GameObject* pGO)
{
    if ( pGO->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER ) /* != GAMEOBJECT_TYPE_QUESTGIVER) */
        pPlayer->PrepareQuestMenu(pGO->GetGUID()); /* return true*/
        
    pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_FEL_CRYSTALFORGE_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_FEL_CRYSTALFORGE_ITEM_5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    
    pPlayer->SEND_GOSSIP_MENU(GOSSIP_FEL_CRYSTALFORGE_TEXT, pGO->GetGUID());
    
    return true;
}

bool GOSelect_go_fel_crystalforge(Player* pPlayer, GameObject* pGO, uint32 uiSender, uint32 uiAction)
{
    switch(uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF:
            pPlayer->CastSpell(pPlayer,SPELL_CREATE_1_FLASK_OF_BEAST,false);
            pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_FEL_CRYSTALFORGE_ITEM_RETURN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_FEL_CRYSTALFORGE_ITEM_TEXT_RETURN, pGO->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 1:
            pPlayer->CastSpell(pPlayer,SPELL_CREATE_5_FLASK_OF_BEAST,false);
            pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_FEL_CRYSTALFORGE_ITEM_RETURN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_FEL_CRYSTALFORGE_ITEM_TEXT_RETURN, pGO->GetGUID());
            break;
    case GOSSIP_ACTION_INFO_DEF + 2:
            pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_FEL_CRYSTALFORGE_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
            pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_FEL_CRYSTALFORGE_ITEM_5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_FEL_CRYSTALFORGE_TEXT, pGO->GetGUID());
            break;
    }
    return true;
}

/*######
## go_bashir_crystalforge
######*/

enum
{
    SPELL_CREATE_1_FLASK_OF_SORCERER   = 40968,
    SPELL_CREATE_5_FLASK_OF_SORCERER   = 40970,
};

bool GOHello_go_bashir_crystalforge(Player* pPlayer, GameObject* pGO)
{
    if (!pPlayer->HasItemCount(APEXIS_SHARD, 50, false))
        pPlayer->CastSpell(pPlayer,SPELL_CREATE_1_FLASK_OF_SORCERER,false);
    else
        pPlayer->CastSpell(pPlayer,SPELL_CREATE_5_FLASK_OF_SORCERER,false);
    return false;
}

/*######
## go_shrine_hawk
######*/

#define QUEST_HAWKS_ESSENCE     10992

bool GOHello_go_shrine_hawk(Player* pPlayer, GameObject* pGo) //workaround as WDB data for this GO seem to be crazy (the GO from the two previous quests are working fine...)
{
    if (pPlayer->GetQuestStatus(QUEST_HAWKS_ESSENCE) == QUEST_STATUS_INCOMPLETE)
    {
        ItemPosCountVec dest;
        uint8 msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 32356, 1);
        if (msg == EQUIP_ERR_OK)
        {
           Item* item = pPlayer->StoreNewItem( dest, 32356, true);
           pPlayer->SendNewItem(item,1,true,false);
        }
    }
        
    return false;
}

/*######
## go_southfury_moonstone
######*/

enum
{
    NPC_RIZZLE                  = 23002,
    SPELL_BLACKJACK             = 39865, //stuns player
    SPELL_SUMMON_RIZZLE         = 39866

};

bool GOHello_go_southfury_moonstone(Player* pPlayer, GameObject* pGo)
{
    //implicitTarget=48 not implemented as of writing this code, and manual summon may be just ok for our purpose
    //pPlayer->CastSpell(pPlayer,SPELL_SUMMON_RIZZLE,false);

    if (Creature* pCreature = pPlayer->SummonCreature(NPC_RIZZLE, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0))
        pCreature->CastSpell(pPlayer,SPELL_BLACKJACK,false);

    return false;
}

/*######
## go_warmaul_prison
######*/

#define QUEST_SURVIVORS 9948
#define MAGHAR_PRISONER 18428

bool GOHello_go_warmaul_prison(Player* pPlayer, GameObject* pGo)
{
    if (pPlayer->GetQuestStatus(QUEST_SURVIVORS) == QUEST_STATUS_INCOMPLETE)
        pPlayer->KilledMonster(MAGHAR_PRISONER, 0);
    
    return false;
}

#define QUEST_SMALLEST_CREATURES    10720

/*######
## go_green_spot_grog_keg
######*/

#define GREEN_SPOT_GROG_KEG_CREDIT  22356

bool GOHello_go_green_spot_grog_keg(Player* pPlayer, GameObject* pGo)
{
    if (pPlayer->GetQuestStatus(QUEST_SMALLEST_CREATURES) == QUEST_STATUS_INCOMPLETE)
    {
        if (Creature* credit = pPlayer->FindCreatureInGrid(GREEN_SPOT_GROG_KEG_CREDIT, 5, true)) {
            pPlayer->DealDamage(credit, credit->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            credit->Respawn();
        }
    }
    
    return false;
}

/*######
## go_ripe_moonshine_keg
######*/

#define RIPE_MOONSHINE_KEG_CREDIT   22367

bool GOHello_go_ripe_moonshine_keg(Player* pPlayer, GameObject* pGo)
{
    if (pPlayer->GetQuestStatus(QUEST_SMALLEST_CREATURES) == QUEST_STATUS_INCOMPLETE)
    {
        if (Creature* credit = pPlayer->FindCreatureInGrid(RIPE_MOONSHINE_KEG_CREDIT, 5, true)) {
            pPlayer->DealDamage(credit, credit->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            credit->Respawn();
        }
    }
    
    return false;
}

/*######
## go_fermented_seed_beer_keg
######*/

#define FERMENTED_SEED_BEER_KEG_CREDIT  22368

bool GOHello_go_fermented_seed_beer_keg(Player* pPlayer, GameObject* pGo)
{
    if (pPlayer->GetQuestStatus(QUEST_SMALLEST_CREATURES) == QUEST_STATUS_INCOMPLETE)
    {
        if (Creature* credit = pPlayer->FindCreatureInGrid(FERMENTED_SEED_BEER_KEG_CREDIT, 5, true)) {
            pPlayer->DealDamage(credit, credit->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            credit->Respawn();
        }
    }
    
    return false;
}

/*######
## go_practice_lockbox
######*/

bool GOHello_go_practice_lockbox(Player* pPlayer, GameObject* pGo)
{
    if (25 > rand()%100)
    {
       switch(rand()%2)
       {
           case 0:
               pGo->CastSpell(pPlayer,10017);
               break;
           case 1:
               pGo->CastSpell(pPlayer,10018);
               break;
       }
    }
    
    return false;
}

/*######
## go_crystalline_tear
######*/

bool QuestAccept_go_crystalline_tear(Player* pPlayer, GameObject* pGo, Quest const* quest)
{
    if (quest->GetQuestId() == 8519)
    {
        WorldPacket data(SMSG_TRIGGER_CINEMATIC, 4);
        data << uint32(2);
        pPlayer->GetSession()->SendPacket(&data);
    }

    return true;
}

/*######
## go_blacksmithing_plans
######*/

#define ENTRY_CRIMSON   11120

bool GOHello_blacksmithing_plans(Player* pPlayer, GameObject* pGo)
{
    pPlayer->SendLoot(pGo->GetGUID(), LOOT_CORPSE);
    if (Creature* crimson = pGo->SummonCreature(ENTRY_CRIMSON, 3567.002930, -2931.949951, 125.001495, 5.163120, TEMPSUMMON_DEAD_DESPAWN, 0))
        crimson->AI()->AttackStart(pPlayer);
        
    return true;
}

/*######
## go_testing_equipment
######*/

#define QUEST_BATCH_OF_OOZE     4294
#define ITEM_TESTED_SAMPLE      15102
#define ITEM_SLIME_SAMPLE       12235

bool GOHello_go_testing_equipment(Player *pPlayer, GameObject *pGo)
{
    if (pPlayer->GetQuestStatus(QUEST_BATCH_OF_OOZE) == QUEST_STATUS_INCOMPLETE) {
        if (pPlayer->HasItemCount(ITEM_SLIME_SAMPLE, 1, false)) {
            ItemPosCountVec dest;
            uint8 msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_TESTED_SAMPLE, 1);
            if (msg == EQUIP_ERR_OK) {
                Item* item = pPlayer->StoreNewItem( dest, ITEM_TESTED_SAMPLE, true);
                pPlayer->SendNewItem(item, 1, true, false);
                pPlayer->DestroyItemCount(ITEM_SLIME_SAMPLE, 1, true, true);
            }
        }
            
        return false;
    }
    
    return true;
}

/*######
## Quest 1126: Hive in the Tower
######*/

enum eHives
{
    QUEST_HIVE_IN_THE_TOWER                       = 9544,
    NPC_HIVE_AMBUSHER                             = 13301
};

bool GOHello_go_hive_pod(Player *pPlayer, GameObject *pGO)
{
    pPlayer->SendLoot(pGO->GetGUID(), LOOT_CORPSE);
    pGO->SummonCreature(NPC_HIVE_AMBUSHER,pGO->GetPositionX()+1,pGO->GetPositionY(),pGO->GetPositionZ(),pGO->GetAngle(pPlayer),TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
    pGO->SummonCreature(NPC_HIVE_AMBUSHER,pGO->GetPositionX(),pGO->GetPositionY()+1,pGO->GetPositionZ(),pGO->GetAngle(pPlayer),TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000);
    return true;
}

/*######
## go_forged_illidari_blade
######*/

bool GOHello_go_forged_illidari_blade(Player *pPlayer, GameObject *pGo)
{
    if (pPlayer->GetQuestStatus(10679) == QUEST_STATUS_INCOMPLETE) {
        ItemPosCountVec dest;
        uint8 msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 30876, 1);
        if (msg == EQUIP_ERR_OK) {
           Item* item = pPlayer->StoreNewItem(dest, 30876, true);
           pPlayer->SendNewItem(item, 1, true, false);
        }
        
        return true;
    }
    
    return false;
}

/*######
## AddSC
######*/

void AddSC_go_scripts()
{
    Script* newscript;

    newscript = new Script;
    newscript->Name="go_northern_crystal_pylon";
    newscript->pGOHello =           &GOHello_go_northern_crystal_pylon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_eastern_crystal_pylon";
    newscript->pGOHello =           &GOHello_go_eastern_crystal_pylon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_western_crystal_pylon";
    newscript->pGOHello =           &GOHello_go_western_crystal_pylon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_barov_journal";
    newscript->pGOHello =           &GOHello_go_barov_journal;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_field_repair_bot_74A";
    newscript->pGOHello =           &GOHello_go_field_repair_bot_74A;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_orb_of_command";
    newscript->pGOHello =           &GOHello_go_orb_of_command;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_tablet_of_madness";
    newscript->pGOHello =           &GOHello_go_tablet_of_madness;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_tablet_of_the_seven";
    newscript->pGOHello =           &GOHello_go_tablet_of_the_seven;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_teleporter";
    newscript->pGOHello =           &GOHello_go_teleporter;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_jump_a_tron";
    newscript->pGOHello =           &GOHello_go_jump_a_tron;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_ethereum_prison";
    newscript->pGOHello =           &GOHello_go_ethereum_prison;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_sacred_fire_of_life";
    newscript->pGOHello =           &GOHello_go_sacred_fire_of_life;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_fel_crystalforge";
    newscript->pGOHello =           &GOHello_go_fel_crystalforge;
    newscript->pGOSelect =          &GOSelect_go_fel_crystalforge;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_bashir_crystalforge";
    newscript->pGOHello =           &GOHello_go_bashir_crystalforge;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_shrine_hawk";
    newscript->pGOHello =           &GOHello_go_shrine_hawk;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_southfury_moonstone";
    newscript->pGOHello =           &GOHello_go_southfury_moonstone;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_warmaul_prison";
    newscript->pGOHello = &GOHello_go_warmaul_prison;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_green_spot_grog_keg";
    newscript->pGOHello = &GOHello_go_green_spot_grog_keg;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_ripe_moonshine_keg";
    newscript->pGOHello = &GOHello_go_ripe_moonshine_keg;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_fermented_seed_beer_keg";
    newscript->pGOHello = &GOHello_go_fermented_seed_beer_keg;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_practice_lockbox";
    newscript->pGOHello = &GOHello_go_practice_lockbox;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_crystalline_tear";
    newscript->pGOQuestAccept = &QuestAccept_go_crystalline_tear;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_blacksmithing_plans";
    newscript->pGOHello = &GOHello_blacksmithing_plans;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_testing_equipment";
    newscript->pGOHello = &GOHello_go_testing_equipment;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_hive_pod";
    newscript->pGOHello = &GOHello_go_hive_pod;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "go_forged_illidari_blade";
    newscript->pGOHello = &GOHello_go_forged_illidari_blade;
    newscript->RegisterSelf();
}
