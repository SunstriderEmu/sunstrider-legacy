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
SDName: Dustwallow_Marsh
SD%Complete: 95
SDComment: Quest support: 558, 1173, 1324, 11126, 11180. Vendor Nat Pagle
SDCategory: Dustwallow Marsh
EndScriptData */

/* ContentData
mobs_risen_husk_spirit
npc_restless_apparition
npc_deserter_agitator
npc_lady_jaina_proudmoore
npc_nat_pagle
npc_overlord_mokmorokk
EndContentData */

#include "precompiled.h"

/*######
## mobs_risen_husk_spirit
######*/

#define SPELL_SUMMON_RESTLESS_APPARITION    42511
#define SPELL_CONSUME_FLESH                 37933           //Risen Husk
#define SPELL_INTANGIBLE_PRESENCE           43127           //Risen Spirit

struct TRINITY_DLL_DECL mobs_risen_husk_spiritAI : public ScriptedAI
{
    mobs_risen_husk_spiritAI(Creature *c) : ScriptedAI(c) {}

    uint32 ConsumeFlesh_Timer;
    uint32 IntangiblePresence_Timer;

    void Reset()
    {
        ConsumeFlesh_Timer = 10000;
        IntangiblePresence_Timer = 5000;
    }

    void Aggro(Unit* who) { }

    void DamageTaken(Unit *done_by, uint32 &damage)
    {
        if( done_by->GetTypeId() == TYPEID_PLAYER )
            if( damage >= m_creature->GetHealth() && ((Player*)done_by)->GetQuestStatus(11180) == QUEST_STATUS_INCOMPLETE )
                m_creature->CastSpell(done_by,SPELL_SUMMON_RESTLESS_APPARITION,false);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if( ConsumeFlesh_Timer < diff )
        {
            if( m_creature->GetEntry() == 23555 )
                DoCast(m_creature->getVictim(),SPELL_CONSUME_FLESH);
            ConsumeFlesh_Timer = 15000;
        } else ConsumeFlesh_Timer -= diff;

        if( IntangiblePresence_Timer < diff )
        {
            if( m_creature->GetEntry() == 23554 )
                DoCast(m_creature->getVictim(),SPELL_INTANGIBLE_PRESENCE);
            IntangiblePresence_Timer = 20000;
        } else IntangiblePresence_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_mobs_risen_husk_spirit(Creature *_Creature)
{
    return new mobs_risen_husk_spiritAI (_Creature);
}

/*######
## npc_restless_apparition
######*/

bool GossipHello_npc_restless_apparition(Player *player, Creature *_Creature)
{
    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    player->TalkedToCreature(_Creature->GetEntry(), _Creature->GetGUID());
    _Creature->SetInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

    return true;
}

/*######
## npc_deserter_agitator
######*/

struct TRINITY_DLL_DECL npc_deserter_agitatorAI : public ScriptedAI
{
    npc_deserter_agitatorAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        m_creature->setFaction(894);
    }

    void Aggro(Unit* who) {}
};

CreatureAI* GetAI_npc_deserter_agitator(Creature *_Creature)
{
    return new npc_deserter_agitatorAI (_Creature);
}

bool GossipHello_npc_deserter_agitator(Player *player, Creature *_Creature)
{
    if (player->GetQuestStatus(11126) == QUEST_STATUS_INCOMPLETE)
    {
        _Creature->setFaction(1883);
        player->TalkedToCreature(_Creature->GetEntry(), _Creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

/*######
## npc_lady_jaina_proudmoore
######*/

#define GOSSIP_ITEM_JAINA "I know this is rather silly but i have a young ward who is a bit shy and would like your autograph."

bool GossipHello_npc_lady_jaina_proudmoore(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if( player->GetQuestStatus(558) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_JAINA, GOSSIP_SENDER_MAIN, GOSSIP_SENDER_INFO );

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_lady_jaina_proudmoore(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_SENDER_INFO )
    {
        player->SEND_GOSSIP_MENU( 7012, _Creature->GetGUID() );
        player->CastSpell( player, 23122, false);
    }
    return true;
}

/*######
## npc_nat_pagle
######*/

bool GossipHello_npc_nat_pagle(Player *player, Creature *_Creature)
{
    if(_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if(_Creature->isVendor() && player->GetQuestRewardStatus(8227))
    {
        player->ADD_GOSSIP_ITEM(1, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        player->SEND_GOSSIP_MENU( 7640, _Creature->GetGUID() );
    }
    else
        player->SEND_GOSSIP_MENU( 7638, _Creature->GetGUID() );

    return true;
}

bool GossipSelect_npc_nat_pagle(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if(action == GOSSIP_ACTION_TRADE)
        player->SEND_VENDORLIST( _Creature->GetGUID() );

    return true;
}

/*######
## npc_overlord_mokmorokk
######*/

#define QUEST_CHALLENGE_OVERLORD    1173

#define FACTION_NEUTRAL             120
#define FACTION_UNFRIENDLY          14

struct TRINITY_DLL_DECL npc_overlord_mokmorokkAI : public ScriptedAI
{
    npc_overlord_mokmorokkAI(Creature *c) : ScriptedAI(c) {}
    
    Player* player;
    
    void Reset()
    {
        m_creature->setFaction(FACTION_NEUTRAL);
        m_creature->SetHealth(m_creature->GetMaxHealth());
        m_creature->CombatStop();
        m_creature->DeleteThreatList();
    }
    
    void Aggro(Unit* who) {}
    
    void UpdateAI(const uint32 diff)
    {
        if (m_creature->getFaction() == FACTION_NEUTRAL) //if neutral, event is not running
            return;
            
        if (m_creature->GetHealth() < (m_creature->GetMaxHealth()/5.0f)) //at 20%, he stops fighting and complete the quest
        {
            player = ((Player*)m_creature->getVictim());
            
            if (player && player->GetQuestStatus(QUEST_CHALLENGE_OVERLORD) == QUEST_STATUS_INCOMPLETE)
                player->KilledMonster(4500, m_creature->GetGUID());
            
            m_creature->MonsterSay("N'en jetez plus !", LANG_UNIVERSAL, 0);
            Reset();
            
            return;
        }
        
        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_overlord_mokmorokk(Creature *pCreature)
{
    return new npc_overlord_mokmorokkAI(pCreature);
}

bool GossipHello_npc_overlord_mokmorokk(Player *pPlayer, Creature *pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_CHALLENGE_OVERLORD) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(0, "Partez maintenant !", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        
    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());
    
    return true;
}

bool GossipSelect_npc_overlord_mokmorokk(Player *pPlayer, Creature *pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        pCreature->MonsterSay("C'est ce qu'on va voir !", LANG_UNIVERSAL, 0);
        pCreature->setFaction(FACTION_UNFRIENDLY);
        pCreature->AI()->AttackStart(pPlayer);
    }
    
    return true;
}

/*######
## npc_private_hendel
######*/

enum eHendel
{
    // looks like all this text ids are wrong.
    SAY_PROGRESS_1_TER          = -1000411, // signed for 3568
    SAY_PROGRESS_2_HEN          = -1000412, // signed for 3568
    SAY_PROGRESS_3_TER          = -1000413,
    SAY_PROGRESS_4_TER          = -1000414,
    EMOTE_SURRENDER             = -1000415,

    QUEST_MISSING_DIPLO_PT16    = 1324,
    FACTION_HOSTILE             = 168,                      //guessed, may be different

    NPC_SENTRY                  = 5184,                     //helps hendel
    NPC_JAINA                   = 4968,                     //appears once hendel gives up
    NPC_TERVOSH                 = 4967
};

//TODO: develop this further, end event not created
struct TRINITY_DLL_DECL npc_private_hendelAI : public ScriptedAI
{
    npc_private_hendelAI(Creature* pCreature) : ScriptedAI(pCreature) { }

    void Reset()
    {
        me->RestoreFaction();
    }

    void AttackedBy(Unit* pAttacker)
    {
        if (m_creature->getVictim())
            return;

        if (m_creature->IsFriendlyTo(pAttacker))
            return;

        AttackStart(pAttacker);
    }
    
    void Aggro(Unit* who) {}

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (uiDamage > m_creature->GetHealth() || ((m_creature->GetHealth() - uiDamage)*100 / m_creature->GetMaxHealth() < 20))
        {
            uiDamage = 0;

            if (Player* pPlayer = pDoneBy->GetCharmerOrOwnerPlayerOrPlayerItself())
                pPlayer->GroupEventHappens(QUEST_MISSING_DIPLO_PT16, m_creature);

            DoScriptText(EMOTE_SURRENDER, m_creature);
            EnterEvadeMode();
        }
    }
};

bool QuestAccept_npc_private_hendel(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_MISSING_DIPLO_PT16)
        pCreature->setFaction(FACTION_HOSTILE);

    return true;
}

CreatureAI* GetAI_npc_private_hendel(Creature* pCreature)
{
    return new npc_private_hendelAI(pCreature);
}

/*######
## AddSC
######*/

void AddSC_dustwallow_marsh()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="mobs_risen_husk_spirit";
    newscript->GetAI = &GetAI_mobs_risen_husk_spirit;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_restless_apparition";
    newscript->pGossipHello =   &GossipHello_npc_restless_apparition;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_deserter_agitator";
    newscript->GetAI = &GetAI_npc_deserter_agitator;
    newscript->pGossipHello = &GossipHello_npc_deserter_agitator;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_lady_jaina_proudmoore";
    newscript->pGossipHello = &GossipHello_npc_lady_jaina_proudmoore;
    newscript->pGossipSelect = &GossipSelect_npc_lady_jaina_proudmoore;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_nat_pagle";
    newscript->pGossipHello = &GossipHello_npc_nat_pagle;
    newscript->pGossipSelect = &GossipSelect_npc_nat_pagle;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_overlord_mokmorokk";
    newscript->pGossipHello =  &GossipHello_npc_overlord_mokmorokk;
    newscript->pGossipSelect = &GossipSelect_npc_overlord_mokmorokk;
    newscript->GetAI = &GetAI_npc_overlord_mokmorokk;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_private_hendel";
    newscript->GetAI = &GetAI_npc_private_hendel;
    newscript->pQuestAccept = &QuestAccept_npc_private_hendel;
    newscript->RegisterSelf();
}
