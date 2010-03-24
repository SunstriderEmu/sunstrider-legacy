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
SDName: Tanaris
SD%Complete: 80
SDComment: Quest support: 2882, 2954, 4005, 10277, 648, 10279(Special flight path). Noggenfogger vendor
SDCategory: Tanaris
EndScriptData */

/* ContentData
mob_aquementas
npc_custodian_of_time
npc_marin_noggenfogger
npc_steward_of_time
npc_stone_watcher_of_norgannon
npc_OOX17
go_landmark_treasure
npc_jhordy_lapforge
EndContentData */

#include "precompiled.h"
#include "../../npc/npc_escortAI.h"

/*######
## mob_aquementas
######*/

#define AGGRO_YELL_AQUE     -1000350

#define SPELL_AQUA_JET      13586
#define SPELL_FROST_SHOCK   15089

struct TRINITY_DLL_DECL mob_aquementasAI : public ScriptedAI
{
    mob_aquementasAI(Creature *c) : ScriptedAI(c) {}

    uint32 SendItem_Timer;
    uint32 SwitchFaction_Timer;
    bool isFriendly;

    uint32 FrostShock_Timer;
    uint32 AquaJet_Timer;

    void Reset()
    {
        SendItem_Timer = 0;
        SwitchFaction_Timer = 10000;
        m_creature->setFaction(35);
        isFriendly = true;

        AquaJet_Timer = 5000;
        FrostShock_Timer = 1000;
    }

    void SendItem(Unit* receiver)
    {
        if (((Player*)receiver)->HasItemCount(11169,1,false) &&
            ((Player*)receiver)->HasItemCount(11172,11,false) &&
            ((Player*)receiver)->HasItemCount(11173,1,false) &&
            !((Player*)receiver)->HasItemCount(11522,1,true))
        {
            ItemPosCountVec dest;
            uint8 msg = ((Player*)receiver)->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, 11522, 1, false);
            if( msg == EQUIP_ERR_OK )
                ((Player*)receiver)->StoreNewItem( dest, 11522, 1, true);
        }
    }

    void Aggro(Unit* who)
    {
        DoScriptText(AGGRO_YELL_AQUE, m_creature, who);
    }

    void UpdateAI(const uint32 diff)
    {
        if( isFriendly )
        {
            if( SwitchFaction_Timer < diff )
            {
                m_creature->setFaction(91);
                isFriendly = false;
            }else SwitchFaction_Timer -= diff;
        }

        if (!UpdateVictim())
            return;

        if( !isFriendly )
        {
            if( SendItem_Timer < diff )
            {
                if( m_creature->getVictim()->GetTypeId() == TYPEID_PLAYER )
                    SendItem(m_creature->getVictim());
                SendItem_Timer = 5000;
            }else SendItem_Timer -= diff;
        }

        if( FrostShock_Timer < diff )
        {
            DoCast(m_creature->getVictim(),SPELL_FROST_SHOCK);
            FrostShock_Timer = 15000;
        }else FrostShock_Timer -= diff;

        if( AquaJet_Timer < diff )
        {
            DoCast(m_creature,SPELL_AQUA_JET);
            AquaJet_Timer = 15000;
        }else AquaJet_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mob_aquementas(Creature *_Creature)
{
    return new mob_aquementasAI (_Creature);
}

/*######
## npc_custodian_of_time
######*/

#define WHISPER_CUSTODIAN_1     -1000150
#define WHISPER_CUSTODIAN_2     -1000151
#define WHISPER_CUSTODIAN_3     -1000152
#define WHISPER_CUSTODIAN_4     -1000153
#define WHISPER_CUSTODIAN_5     -1000154
#define WHISPER_CUSTODIAN_6     -1000155
#define WHISPER_CUSTODIAN_7     -1000156
#define WHISPER_CUSTODIAN_8     -1000157
#define WHISPER_CUSTODIAN_9     -1000158
#define WHISPER_CUSTODIAN_10    -1000159
#define WHISPER_CUSTODIAN_11    -1000160
#define WHISPER_CUSTODIAN_12    -1000161
#define WHISPER_CUSTODIAN_13    -1000162
#define WHISPER_CUSTODIAN_14    -1000163

struct TRINITY_DLL_DECL npc_custodian_of_timeAI : public npc_escortAI
{
    npc_custodian_of_timeAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player *pPlayer = GetPlayerForEscort();
        if( !pPlayer )
            return;

        switch( i )
        {
            case 0: DoScriptText(WHISPER_CUSTODIAN_1, m_creature, pPlayer); break;
            case 1: DoScriptText(WHISPER_CUSTODIAN_2, m_creature, pPlayer); break;
            case 2: DoScriptText(WHISPER_CUSTODIAN_3, m_creature, pPlayer); break;
            case 3: DoScriptText(WHISPER_CUSTODIAN_4, m_creature, pPlayer); break;
            case 5: DoScriptText(WHISPER_CUSTODIAN_5, m_creature, pPlayer); break;
            case 6: DoScriptText(WHISPER_CUSTODIAN_6, m_creature, pPlayer); break;
            case 7: DoScriptText(WHISPER_CUSTODIAN_7, m_creature, pPlayer); break;
            case 8: DoScriptText(WHISPER_CUSTODIAN_8, m_creature, pPlayer); break;
            case 9: DoScriptText(WHISPER_CUSTODIAN_9, m_creature, pPlayer); break;
            case 10: DoScriptText(WHISPER_CUSTODIAN_4, m_creature, pPlayer); break;
            case 13: DoScriptText(WHISPER_CUSTODIAN_10, m_creature, pPlayer); break;
            case 14: DoScriptText(WHISPER_CUSTODIAN_4, m_creature, pPlayer); break;
            case 16: DoScriptText(WHISPER_CUSTODIAN_11, m_creature, pPlayer); break;
            case 17: DoScriptText(WHISPER_CUSTODIAN_12, m_creature, pPlayer); break;
            case 18: DoScriptText(WHISPER_CUSTODIAN_4, m_creature, pPlayer); break;
            case 22: DoScriptText(WHISPER_CUSTODIAN_13, m_creature, pPlayer); break;
            case 23: DoScriptText(WHISPER_CUSTODIAN_4, m_creature, pPlayer); break;
            case 24:
                DoScriptText(WHISPER_CUSTODIAN_14, m_creature, pPlayer);
                DoCast(pPlayer,34883);
                //below here is temporary workaround, to be removed when spell works properly
                pPlayer->AreaExploredOrEventHappens(10277);
                break;
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if( IsBeingEscorted )
            return;

        if( who->GetTypeId() == TYPEID_PLAYER )
        {
            if( ((Player*)who)->HasAura(34877,1) && ((Player*)who)->GetQuestStatus(10277) == QUEST_STATUS_INCOMPLETE )
            {
                float Radius = 10.0;
                if( m_creature->IsWithinDistInMap(who, Radius) )
                {
                    ((npc_escortAI*)(m_creature->AI()))->Start(false, false, false, who->GetGUID(), m_creature->GetEntry());
                }
            }
        }
    }

    void Aggro(Unit* who) { }
    void Reset() { }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
    }
};

CreatureAI* GetAI_npc_custodian_of_time(Creature *pCreature)
{
    return new npc_custodian_of_timeAI(pCreature);
}

/*######
## npc_marin_noggenfogger
######*/

bool GossipHello_npc_marin_noggenfogger(Player *player, Creature *_Creature)
{
    if( _Creature->isQuestGiver() )
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( _Creature->isVendor() && player->GetQuestRewardStatus(2662) )
        player->ADD_GOSSIP_ITEM(1, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_marin_noggenfogger(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if( action == GOSSIP_ACTION_TRADE )
        player->SEND_VENDORLIST( _Creature->GetGUID() );

    return true;
}

/*######
## npc_steward_of_time
######*/

#define GOSSIP_ITEM_FLIGHT  "Please take me to the master's lair."

bool GossipHello_npc_steward_of_time(Player *player, Creature *_Creature)
{
    if( _Creature->isQuestGiver() )
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( player->GetQuestStatus(10279) == QUEST_STATUS_INCOMPLETE || player->GetQuestRewardStatus(10279) )
    {
        player->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM_FLIGHT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(9978,_Creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU(9977,_Creature->GetGUID());

    return true;
}

bool QuestAccept_npc_steward_of_time(Player *player, Creature *creature, Quest const *quest )
{
    if( quest->GetQuestId() == 10279 )                      //Quest: To The Master's Lair
        player->CastSpell(player,34891,true);               //(Flight through Caverns)

    return false;
}

bool GossipSelect_npc_steward_of_time(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF + 1 )
        player->CastSpell(player,34891,true);               //(Flight through Caverns)

    return true;
}

/*######
## npc_stone_watcher_of_norgannon
######*/

#define GOSSIP_ITEM_NORGANNON_1     "What function do you serve?"
#define GOSSIP_ITEM_NORGANNON_2     "What are the Plates of Uldum?"
#define GOSSIP_ITEM_NORGANNON_3     "Where are the Plates of Uldum?"
#define GOSSIP_ITEM_NORGANNON_4     "Excuse me? We've been \"reschedueled for visitations\"? What does that mean?!"
#define GOSSIP_ITEM_NORGANNON_5     "So, what's inside Uldum?"
#define GOSSIP_ITEM_NORGANNON_6     "I will return when i have the Plates of Uldum."

bool GossipHello_npc_stone_watcher_of_norgannon(Player *player, Creature *_Creature)
{
    if( _Creature->isQuestGiver() )
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( player->GetQuestStatus(2954) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_NORGANNON_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(1674, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_stone_watcher_of_norgannon(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_NORGANNON_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->SEND_GOSSIP_MENU(1675, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_NORGANNON_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(1676, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_NORGANNON_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->SEND_GOSSIP_MENU(1677, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_NORGANNON_5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(1678, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_NORGANNON_6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
            player->SEND_GOSSIP_MENU(1679, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            player->CLOSE_GOSSIP_MENU();
            player->AreaExploredOrEventHappens(2954);
            break;
    }
    return true;
}

/*######
## npc_OOX17
######*/

#define Q_OOX17             648
#define SPAWN_FIRST         7803
#define SPAWN_SECOND_1      5617
#define SPAWN_SECOND_2      7805
#define SAY_SCOFF           -1060004
#define SAY_CHICKEN_ACC     -1060000
#define SAY_CHICKEN_AGGRO_1 -1060001
#define SAY_CHICKEN_AGGRO_2 -1060002
#define SAY_CHICKEN_AMB     -1060003
#define SAY_CHICKEN_COMP    -1060005

struct TRINITY_DLL_DECL npc_OOX17AI : public npc_escortAI
{
    npc_OOX17AI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        switch(i) {
            case 23:
                m_creature->SummonCreature(SPAWN_FIRST, -8350.96, -4445.79, 10.10, 6.20, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                m_creature->SummonCreature(SPAWN_FIRST, -8355.96, -4447.79, 10.10, 6.27, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                m_creature->SummonCreature(SPAWN_FIRST, -8353.96, -4442.79, 10.10, 6.08, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                DoScriptText(SAY_CHICKEN_AMB, m_creature);
                break;

            case 56:
                m_creature->SummonCreature(SPAWN_SECOND_1, -7510.07, -4795.50, 9.35, 6.06, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                m_creature->SummonCreature(SPAWN_SECOND_2, -7515.07, -4797.50, 9.35, 6.22, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                m_creature->SummonCreature(SPAWN_SECOND_2, -7518.07, -4792.50, 9.35, 6.22, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
                DoScriptText(SAY_CHICKEN_AMB, m_creature);
                {Unit* scoff = FindCreature(SPAWN_SECOND_2, 30, m_creature);
                if(scoff)
                    DoScriptText(SAY_SCOFF, scoff);}break;
                break;

            case 86:
                DoScriptText(SAY_CHICKEN_COMP, m_creature);
                player->GroupEventHappens(Q_OOX17, m_creature);
                break;
        }
    }

    void Reset(){}

    void Aggro(Unit* who)
    {
        switch (rand()%2)
        {
        case 0: DoScriptText(SAY_CHICKEN_AGGRO_1, m_creature); break;
        case 1: DoScriptText(SAY_CHICKEN_AGGRO_2, m_creature); break;
        }
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(m_creature);
    }

    void JustDied(Unit* killer)
    {
        if (PlayerGUID)
        {
            if (Player* player = GetPlayerForEscort())
                player->FailQuest(Q_OOX17);
        }
    }


    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
        if (!UpdateVictim())
            return;
    }
    };

bool QuestAccept_npc_OOX17(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == Q_OOX17)
    {
        creature->setFaction(113);
        creature->SetHealth(creature->GetMaxHealth());
        creature->SetUInt32Value(UNIT_FIELD_BYTES_1,0);
        creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
        DoScriptText(SAY_CHICKEN_ACC, creature);
        ((npc_escortAI*)(creature->AI()))->Start(true, true, false, player->GetGUID(), creature->GetEntry());

    }
    return true;
}

CreatureAI* GetAI_npc_OOX17(Creature *pCreature)
{
    return new npc_OOX17AI(pCreature);
}

/*######
## go_landmark_treasure
######*/

#define QUEST_CUERGOS_GOLD 2882

#define NPC_BUCCANEER      7902
#define NPC_PIRATE         7899
#define NPC_SWASHBUCKLER   7901

#define GO_TREASURE        142194

#define PATH_ENTRY_1       2090
#define PATH_ENTRY_2       2091
#define PATH_ENTRY_3       2092
#define PATH_ENTRY_4       2093
#define PATH_ENTRY_5       2094

bool GOHello_go_landmark_treasure(Player *player, GameObject* _GO)
{
    if (player->GetQuestStatus(QUEST_CUERGOS_GOLD) != QUEST_STATUS_INCOMPLETE)
        return false;
    
    Creature * spawn = NULL;
    
    spawn = player->SummonCreature(NPC_PIRATE, -10029.78, -4032.54, 19.41, 3.40, TEMPSUMMON_TIMED_DESPAWN, 340000);
    if(spawn)
        spawn->GetMotionMaster()->MovePath(PATH_ENTRY_1, true);
    spawn = player->SummonCreature(NPC_PIRATE, -10031.64, -4032.14, 19.11, 3.40, TEMPSUMMON_TIMED_DESPAWN, 340000);
    if(spawn)
        spawn->GetMotionMaster()->MovePath(PATH_ENTRY_3, true);
    
    spawn = player->SummonCreature(NPC_SWASHBUCKLER, -10029.86, -4030.51, 20.02, 3.40, TEMPSUMMON_TIMED_DESPAWN, 340000);
    if(spawn)
        spawn->GetMotionMaster()->MovePath(PATH_ENTRY_4, true);
    spawn = player->SummonCreature(NPC_SWASHBUCKLER, -10031.83, -4030.70, 19.52, 3.40, TEMPSUMMON_TIMED_DESPAWN, 340000);
    if(spawn)
        spawn->GetMotionMaster()->MovePath(PATH_ENTRY_5, true);
    
    spawn = player->SummonCreature(NPC_BUCCANEER, -10028.90, -4029.65, 20.53, 3.40, TEMPSUMMON_TIMED_DESPAWN, 340000);
    if(spawn)
        spawn->GetMotionMaster()->MovePath(PATH_ENTRY_2, true);
    
    player->SummonGameObject(GO_TREASURE, -10119.70, -4050.45, 5.33, 0, 0, 0, 0, 0, 240);

    return true;
};

/*######
## npc_jhordy_lapforge
######*/

#define GOSSIP_ZAP                  "[PH] J'aimerais recevoir le transporteur ultra-sécurisé."
#define SPELL_GNOME_ENGINEER        20219
#define SPELL_ULTRASAFE_TELE        23489

bool GossipHello_npc_jhordy_lapforge(Player *pPlayer, Creature *pCreature)
{
    if (pPlayer->HasSkill(SKILL_ENGINERING) && (pPlayer->GetBaseSkillValue(SKILL_ENGINERING) >= 260) && pPlayer->HasSpell(SPELL_GNOME_ENGINEER) && !pPlayer->HasSpell(SPELL_ULTRASAFE_TELE)) {
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_ZAP, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        pPlayer->SEND_GOSSIP_MENU(3377, pCreature->GetGUID());
        
        return true;
    }
    
    return false;
}

bool GossipSelect_npc_jhordy_lapforge(Player *pPlayer, Creature *pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1) {
        pPlayer->learnSpell(SPELL_ULTRASAFE_TELE);
        
        pPlayer->CLOSE_GOSSIP_MENU();
    }
}

/*######
## AddSC
######*/

void AddSC_tanaris()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="mob_aquementas";
    newscript->GetAI = &GetAI_mob_aquementas;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_custodian_of_time";
    newscript->GetAI = &GetAI_npc_custodian_of_time;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_marin_noggenfogger";
    newscript->pGossipHello =  &GossipHello_npc_marin_noggenfogger;
    newscript->pGossipSelect = &GossipSelect_npc_marin_noggenfogger;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_steward_of_time";
    newscript->pGossipHello =  &GossipHello_npc_steward_of_time;
    newscript->pGossipSelect = &GossipSelect_npc_steward_of_time;
    newscript->pQuestAccept =  &QuestAccept_npc_steward_of_time;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_stone_watcher_of_norgannon";
    newscript->pGossipHello =  &GossipHello_npc_stone_watcher_of_norgannon;
    newscript->pGossipSelect = &GossipSelect_npc_stone_watcher_of_norgannon;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_OOX17";
    newscript->GetAI = &GetAI_npc_OOX17;
    newscript->pQuestAccept = &QuestAccept_npc_OOX17;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_landmark_treasure";
    newscript->pGOHello = &GOHello_go_landmark_treasure;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_jhordy_lapforge";
    newscript->pGossipHello = &GossipHello_npc_jhordy_lapforge;
    newscript->pGossipSelect = &GossipSelect_npc_jhordy_lapforge;
    newscript->RegisterSelf();
}

