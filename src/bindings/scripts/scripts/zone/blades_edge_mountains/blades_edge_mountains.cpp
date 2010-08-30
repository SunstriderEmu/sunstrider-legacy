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
SDName: Blades_Edge_Mountains
SD%Complete: 90
SDComment: Quest support: 10503, 10504, 10556, 10609, 10682, 10821, 10980. Ogri'la->Skettis Flight. (npc_daranelle needs bit more work before consider complete)
SDCategory: Blade's Edge Mountains
EndScriptData */

/* ContentData
mobs_bladespire_ogre
mobs_nether_drake
npc_daranelle
npc_overseer_nuaar
npc_saikkal_the_elder
npc_skyguard_handler_irena
go_legion_obelisk
npc_prophecy_questcredit
npc_grishna_falconwing
EndContentData */

#include "precompiled.h"

/*######
## mobs_bladespire_ogre
######*/

//TODO: add support for quest 10512 + creature abilities
struct mobs_bladespire_ogreAI : public ScriptedAI
{
    mobs_bladespire_ogreAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
    }

    void Aggro(Unit* pWho)
    {
    }

    void JustDied(Unit* pKiller)
    {
        if (pKiller->GetTypeId() == TYPEID_PLAYER)
            CAST_PLR(pKiller)->KilledMonster(19995, m_creature->GetGUID());
    }
};
CreatureAI* GetAI_mobs_bladespire_ogre(Creature *pCreature)
{
    return new mobs_bladespire_ogreAI (pCreature);
}

/*######
## mobs_nether_drake
######*/

enum eNetherDrake
{
SAY_NIHIL_1                 = -1000396,
SAY_NIHIL_2                 = -1000397,
SAY_NIHIL_3                 = -1000398,
SAY_NIHIL_4                 = -1000399,
SAY_NIHIL_INTERRUPT         = -1000400,

ENTRY_WHELP                 = 20021,
ENTRY_PROTO                 = 21821,
ENTRY_ADOLE                 = 21817,
ENTRY_MATUR                 = 21820,
ENTRY_NIHIL                 = 21823,

SPELL_T_PHASE_MODULATOR     = 37573,
SPELL_ARCANE_BLAST          = 38881,
SPELL_MANA_BURN             = 38884,
SPELL_INTANGIBLE_PRESENCE   = 36513
};

struct mobs_nether_drakeAI : public ScriptedAI
{
    mobs_nether_drakeAI(Creature *c) : ScriptedAI(c) {}

    bool IsNihil;
    uint32 NihilSpeech_Timer;
    uint32 NihilSpeech_Phase;

    uint32 ArcaneBlast_Timer;
    uint32 ManaBurn_Timer;
    uint32 IntangiblePresence_Timer;

    void Reset()
    {
        NihilSpeech_Timer = 2000;
        IsNihil = false;
        if( m_creature->GetEntry() == ENTRY_NIHIL )
        {
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            IsNihil = true;
        }
        NihilSpeech_Phase = 1;

        ArcaneBlast_Timer = 7500;
        ManaBurn_Timer = 10000;
        IntangiblePresence_Timer = 15000;
    }

    void Aggro(Unit* pWho) { }

    void SpellHit(Unit *caster, const SpellEntry *spell)
    {
        if (spell->Id == SPELL_T_PHASE_MODULATOR && caster->GetTypeId() == TYPEID_PLAYER)
        {
            uint32 cEntry = 0;

            switch (m_creature->GetEntry())
            {
                case ENTRY_WHELP:
                    cEntry = RAND(ENTRY_PROTO, ENTRY_ADOLE, ENTRY_MATUR, ENTRY_NIHIL);
                    break;
                case ENTRY_PROTO:
                case ENTRY_ADOLE:
                case ENTRY_MATUR:
                    cEntry = RAND(ENTRY_ADOLE, ENTRY_MATUR, ENTRY_NIHIL);
                    break;
                case ENTRY_NIHIL:
                    if (NihilSpeech_Phase)
                    {
                        DoScriptText(SAY_NIHIL_INTERRUPT, m_creature);
                        IsNihil = false;
                        cEntry = RAND(ENTRY_ADOLE, ENTRY_MATUR, ENTRY_NIHIL);
                    }
                    break;
            }

            if (cEntry)
            {
                m_creature->UpdateEntry(cEntry);

                if (cEntry == ENTRY_NIHIL)
                {
                    m_creature->InterruptNonMeleeSpells(true);
                    m_creature->RemoveAllAuras();
                    m_creature->DeleteThreatList();
                    m_creature->CombatStop();
                    InCombat = false;
                    Reset();
                }
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (IsNihil)
        {
            if (NihilSpeech_Phase)
            {
                if (NihilSpeech_Timer <= diff)
                {
                    switch (NihilSpeech_Phase)
                    {
                        case 1:
                            DoScriptText(SAY_NIHIL_1, m_creature);
                            ++NihilSpeech_Phase;
                            break;
                        case 2:
                            DoScriptText(SAY_NIHIL_2, m_creature);
                            ++NihilSpeech_Phase;
                            break;
                        case 3:
                            DoScriptText(SAY_NIHIL_3, m_creature);
                            ++NihilSpeech_Phase;
                            break;
                        case 4:
                            DoScriptText(SAY_NIHIL_4, m_creature);
                            ++NihilSpeech_Phase;
                            break;
                        case 5:
                            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                                            // + MOVEMENTFLAG_LEVITATING
                            m_creature->AddUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT);
                            //then take off to random location. creature is initially summoned, so don't bother do anything else.
                            m_creature->GetMotionMaster()->MovePoint(0, m_creature->GetPositionX()+100, m_creature->GetPositionY(), m_creature->GetPositionZ()+100);
                            NihilSpeech_Phase = 0;
                            break;
                    }
                    NihilSpeech_Timer = 5000;
                }else NihilSpeech_Timer -=diff;
            }
            return;                                         //anything below here is not interesting for Nihil, so skip it
        }

        if (!UpdateVictim())
            return;

        if (IntangiblePresence_Timer <= diff)
        {
            DoCast(m_creature->getVictim(),SPELL_INTANGIBLE_PRESENCE);
            IntangiblePresence_Timer = 15000+rand()%15000;
        }else IntangiblePresence_Timer -= diff;

        if (ManaBurn_Timer <= diff)
        {
            Unit* target = m_creature->getVictim();
            if (target && target->getPowerType() == POWER_MANA)
                DoCast(target,SPELL_MANA_BURN);
            ManaBurn_Timer = 8000+rand()%8000;
        }else ManaBurn_Timer -= diff;

        if (ArcaneBlast_Timer <= diff)
        {
            DoCast(m_creature->getVictim(),SPELL_ARCANE_BLAST);
            ArcaneBlast_Timer = 2500+rand()%5000;
        }else ArcaneBlast_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mobs_nether_drake(Creature *pCreature)
{
    return new mobs_nether_drakeAI (pCreature);
}

/*######
## npc_daranelle
######*/

#define SAY_DARANELLE -1000401

struct npc_daranelleAI : public ScriptedAI
{
    npc_daranelleAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
    }

    void Aggro(Unit* pWho)
    {
    }

    void MoveInLineOfSight(Unit* pWho)
    {
        if (pWho->GetTypeId() == TYPEID_PLAYER)
        {
            if (pWho->HasAura(36904))
            {
                DoScriptText(SAY_DARANELLE, m_creature, pWho);
                //TODO: Move the below to updateAI and run if this statement == true
                CAST_PLR(pWho)->KilledMonster(21511, m_creature->GetGUID());
                CAST_PLR(pWho)->RemoveAurasDueToSpell(36904);
            }
        }

        ScriptedAI::MoveInLineOfSight(pWho);
    }
};

CreatureAI* GetAI_npc_daranelle(Creature *pCreature)
{
    return new npc_daranelleAI (pCreature);
}

/*######
## npc_overseer_nuaar
######*/

#define GOSSIP_HON "Overseer, I am here to negotiate on behalf of the Cenarion Expedition."

bool GossipHello_npc_overseer_nuaar(Player *pPlayer, Creature *pCreature)
{
    if (pPlayer->GetQuestStatus(10682) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_HON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(10532, pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_overseer_nuaar(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        pPlayer->SEND_GOSSIP_MENU(10533, pCreature->GetGUID());
        pPlayer->AreaExploredOrEventHappens(10682);
    }
    return true;
}

/*######
## npc_saikkal_the_elder
######*/

#define GOSSIP_HSTE "Yes... yes, it's me."
#define GOSSIP_SSTE "Yes elder. Tell me more of the book."

bool GossipHello_npc_saikkal_the_elder(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(10980) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM( 0, GOSSIP_HSTE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(10794, pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_saikkal_the_elder(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM( 0, GOSSIP_SSTE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(10795, pCreature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->TalkedToCreature(pCreature->GetEntry(), pCreature->GetGUID());
            pPlayer->SEND_GOSSIP_MENU(10796, pCreature->GetGUID());
            break;
    }
    return true;
}

/*######
## npc_skyguard_handler_irena
######*/

#define GOSSIP_SKYGUARD "Fly me to Skettis please"

bool GossipHello_npc_skyguard_handler_irena(Player* pPlayer, Creature* pCreature )
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu( pCreature->GetGUID() );

    if (pPlayer->GetReputationRank(1031) >= REP_HONORED)
        pPlayer->ADD_GOSSIP_ITEM( 2, GOSSIP_SKYGUARD, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_skyguard_handler_irena(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();

        std::vector<uint32> nodes;

        nodes.resize(2);
        nodes[0] = 172;                                     //from ogri'la
        nodes[1] = 171;                                     //end at skettis
        pPlayer->ActivateTaxiPathTo(nodes);                  //TaxiPath 706
    }
    return true;
}

/*######
## go_legion_obelisk
######*/

//Support for quest: You're Fired! (10821)
bool bObeliskOne, bObeliskTwo, bObeliskThree, bObeliskFour, bObeliskFive;	

enum eLegionObelisk
{
LEGION_OBELISK_ONE           = 185193,
LEGION_OBELISK_TWO           = 185195,
LEGION_OBELISK_THREE         = 185196,
LEGION_OBELISK_FOUR          = 185197,
LEGION_OBELISK_FIVE          = 185198
};

bool GOHello_go_legion_obelisk(Player *pPlayer, GameObject* pGo)
{	
	if (pPlayer->GetQuestStatus(10821) == QUEST_STATUS_INCOMPLETE)
	{
		switch(pGo->GetEntry())
		{
			case LEGION_OBELISK_ONE:
                bObeliskOne = true;
                break;
			case LEGION_OBELISK_TWO:
				bObeliskTwo = true;
                break;
			case LEGION_OBELISK_THREE:
                bObeliskThree = true;
                break;
			case LEGION_OBELISK_FOUR:
                bObeliskFour = true;
                break;
			case LEGION_OBELISK_FIVE:
                bObeliskFive = true;
                break;
		}
	
		if (bObeliskOne && bObeliskTwo && bObeliskThree && bObeliskFour && bObeliskFive)
		{
			pGo->SummonCreature(19963, 2943.40f, 4778.20f, 284.49f, 0.94f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 120000);
			//reset global var
			bObeliskOne = false;
			bObeliskTwo = false;
			bObeliskThree = false;
			bObeliskFour = false;
			bObeliskFive = false;
		}
	}
	
	return true;
}

/*######
## npc_prophecy_questcredit
######*/

#define QUEST_WHISPERS_RAVEN_GOD    10607

struct npc_prophecy_questcreditAI : public ScriptedAI
{
    npc_prophecy_questcreditAI(Creature *c) : ScriptedAI(c) {}
    
    void Aggro(Unit *pWho) {}
    
    void Reset()
    {
        m_creature->SetReactState(REACT_AGGRESSIVE);
    }
    
    void MoveInLineOfSight(Unit *pWho)
    {
        if (m_creature->GetDistance(pWho) >= 5.0f)
            return;
        if (pWho->GetTypeId() != TYPEID_PLAYER)
            return;
        
        Player *plr = reinterpret_cast<Player*>(pWho);
        if (plr->GetQuestStatus(QUEST_WHISPERS_RAVEN_GOD) == QUEST_STATUS_INCOMPLETE) {
            if (plr->HasAura(37642))
                plr->KilledMonster(m_creature->GetEntry(), m_creature->GetGUID());
        }
    }
};

CreatureAI* GetAI_npc_prophecy_questcredit(Creature *pCreature)
{
    return new npc_prophecy_questcreditAI(pCreature);
}

/*######
## npc_grishna_falconwing
######*/

struct npc_grishna_falconwingAI : public ScriptedAI
{
    npc_grishna_falconwingAI(Creature *c) : ScriptedAI(c) {}
    
    void Aggro(Unit *pWho) {}
    
    void JustDied(Unit *pKiller)
    {
        pKiller->AddAura(37642, pKiller);
    }
};

CreatureAI* GetAI_npc_grishna_falconwingAI(Creature *pCreature)
{
    return new npc_grishna_falconwingAI(pCreature);
}

/*######
## AddSC
######*/

void AddSC_blades_edge_mountains()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="mobs_bladespire_ogre";
    newscript->GetAI = &GetAI_mobs_bladespire_ogre;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mobs_nether_drake";
    newscript->GetAI = &GetAI_mobs_nether_drake;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_daranelle";
    newscript->GetAI = &GetAI_npc_daranelle;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_overseer_nuaar";
    newscript->pGossipHello = &GossipHello_npc_overseer_nuaar;
    newscript->pGossipSelect = &GossipSelect_npc_overseer_nuaar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_saikkal_the_elder";
    newscript->pGossipHello = &GossipHello_npc_saikkal_the_elder;
    newscript->pGossipSelect = &GossipSelect_npc_saikkal_the_elder;
    newscript->RegisterSelf();
	
    newscript = new Script;
    newscript->Name="go_legion_obelisk";
    newscript->pGOHello =           &GOHello_go_legion_obelisk;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_skyguard_handler_irena";
    newscript->pGossipHello =  &GossipHello_npc_skyguard_handler_irena;
    newscript->pGossipSelect = &GossipSelect_npc_skyguard_handler_irena;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_prophecy_questcredit";
    newscript->GetAI = &GetAI_npc_prophecy_questcredit;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name = "npc_grishna_falconwing";
    newscript->GetAI = &GetAI_npc_grishna_falconwingAI;
    newscript->RegisterSelf();
}

