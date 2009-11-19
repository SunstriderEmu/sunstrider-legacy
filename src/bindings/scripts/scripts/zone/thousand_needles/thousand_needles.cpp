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
SDName: Thousand Needles
SD%Complete: 100
SDComment: Support for Quest: 4770, 1950, 5151.
SDCategory: Thousand Needles
EndScriptData */

/* ContentData
npc_swiftmountain
npc_plucky
go_panther_cage
npc_enraged_panther
EndContentData */

#include "precompiled.h"
#include "../../npc/npc_escortAI.h"

/*######
## npc_swiftmountain
######*/

#define SAY_START -1000147
#define SAY_WYVERN -1000148
#define SAY_COMPLETE -1000149

#define QUEST_HOMEWARD_BOUND 4770
#define ENTRY_WYVERN 4107

struct TRINITY_DLL_DECL npc_swiftmountainAI : public npc_escortAI
{
npc_swiftmountainAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        switch (i)
        {
        case 15:
            DoScriptText(SAY_WYVERN, m_creature, player);
            m_creature->SummonCreature(ENTRY_WYVERN, -5016.45, -935.01, -5.46, 5.36,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            m_creature->SummonCreature(ENTRY_WYVERN, -5001.98, -934.96, -5.55, 3.18,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            m_creature->SummonCreature(ENTRY_WYVERN, -4999.06, -949.61, -5.42, 2.04,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 25000);
            break;
         case 26:
            DoScriptText(SAY_COMPLETE, m_creature);
            break;
         case 27:
            player->GroupEventHappens(QUEST_HOMEWARD_BOUND,m_creature);
            break;
        }
    }

    void Reset()
    {
        m_creature->setFaction(104);
    }

    void Aggro(Unit* who){}

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(m_creature);
    }

    void JustDied(Unit* killer)
    {
        if (PlayerGUID)
        {
            if (Player* player = GetPlayerForEscort())
                player->FailQuest(QUEST_HOMEWARD_BOUND);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
    }
};

bool QuestAccept_npc_swiftmountain(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_HOMEWARD_BOUND)
    {
        ((npc_escortAI*)(creature->AI()))->Start(true, true, false, player->GetGUID(), creature->GetEntry());
        DoScriptText(SAY_START, creature, player);
        creature->setFaction(113);
    }

    return true;
}

CreatureAI* GetAI_npc_swiftmountain(Creature *pCreature)
{
    return new npc_swiftmountainAI(pCreature);
}

/*######
## npc_plucky
######*/

#define GOSSIP_P    "<Learn Secret phrase>"

#define SPELL_TRANSFORM_HUMAN 9192
#define QUEST_GET_THE_SCOOP 1950

struct TRINITY_DLL_DECL npc_pluckyAI : public ScriptedAI
{
    npc_pluckyAI(Creature *c) : ScriptedAI(c) {}

    bool Transformed;
    bool Chicken;

    uint32 Timer;
    uint32 ChickenTimer;

    void Reset()   {

       Transformed = false;
       Chicken     = false;
       m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
       Timer        = 0;
       ChickenTimer = 0;
       }

    void Aggro(Unit *who){}

    void TransformHuman(uint32 emoteid)
    {
         if (!Transformed)
         {
             Transformed = true;
             DoCast(m_creature, SPELL_TRANSFORM_HUMAN);
             Timer = 120000;
             if (emoteid == TEXTEMOTE_BECKON)
                 m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
             else
             {
                 ChickenTimer = 1500;
                 Chicken = true;
             }
         }
    }

    void UpdateAI(const uint32 diff)
    {
        if (Transformed)
        {
            if (Timer < diff)
                Reset();
            else Timer-=diff;
        }

       if(Chicken)
       {
           if (ChickenTimer < diff)
           {
               m_creature->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
               Chicken = false;
           }else ChickenTimer-=diff;
       }

        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
   }
};

bool ReceiveEmote_npc_plucky( Player *player, Creature *_Creature, uint32 emote )
{
    if( (emote == TEXTEMOTE_BECKON || emote == TEXTEMOTE_CHICKEN &&
        player->GetQuestStatus(QUEST_GET_THE_SCOOP) == QUEST_STATUS_INCOMPLETE) )
    {
        _Creature->SetInFront(player);
        ((npc_pluckyAI*)((Creature*)_Creature)->AI())->TransformHuman(emote);
    }

    return true;
}

bool GossipHello_npc_plucky(Player *player, Creature *_Creature)
{
    if(player->GetQuestStatus(QUEST_GET_THE_SCOOP) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, GOSSIP_P, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    player->SEND_GOSSIP_MENU(738, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_plucky(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch( action )
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->CLOSE_GOSSIP_MENU();
            player->CompleteQuest(QUEST_GET_THE_SCOOP);
        break;
    }
    return true;
}

CreatureAI* GetAI_npc_plucky(Creature *_Creature)
{
return new npc_pluckyAI(_Creature);
}

/*######
## go_panther_cage
######*/

enum ePantherCage
{
    ENRAGED_PANTHER = 10992
};

bool go_panther_cage(Player* pPlayer, GameObject* pGo)
{

    if (pPlayer->GetQuestStatus(5151) == QUEST_STATUS_INCOMPLETE)
    {
        if(Creature* panther = pGo->FindCreatureInGrid(ENRAGED_PANTHER, 5.0f, true))
        {
            panther->RemoveFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
            panther->SetReactState(REACT_AGGRESSIVE);
            panther->AI()->AttackStart(pPlayer);
        }
    }

    return true ;
}

/*######
## npc_enraged_panther
######*/

struct TRINITY_DLL_DECL npc_enraged_pantherAI : public ScriptedAI
{
    npc_enraged_pantherAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetReactState(REACT_PASSIVE);
    }

    void Aggro(Unit* who) {}

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_enraged_panther(Creature* pCreature)
{
    return new npc_enraged_pantherAI(pCreature);
}

/*######
## AddSC
######*/

void AddSC_thousand_needles()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_swiftmountain";
    newscript->GetAI = &GetAI_npc_swiftmountain;
    newscript->pQuestAccept = &QuestAccept_npc_swiftmountain;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_plucky";
    newscript->GetAI = &GetAI_npc_plucky;
    newscript->pReceiveEmote =  &ReceiveEmote_npc_plucky;
    newscript->pGossipHello =   &GossipHello_npc_plucky;
    newscript->pGossipSelect = &GossipSelect_npc_plucky;
    newscript->RegisterSelf();
}

