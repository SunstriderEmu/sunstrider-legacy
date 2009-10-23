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
SDName: Hellfire_Peninsula
SD%Complete: 100
SDComment: Quest support: 9375, 9418, 10129, 10146, 10162, 10163, 10340, 10346, 10347, 10382 (Special flight paths), 10629, 10838
SDCategory: Hellfire Peninsula
EndScriptData */

/* ContentData
npc_aeranas
go_haaleshi_altar
npc_wing_commander_dabiree
npc_gryphoneer_windbellow
npc_wing_commander_brack
npc_wounded_blood_elf
npc_demoniac_scryer
npc_fel_guard_hound
EndContentData */

#include "precompiled.h"
#include "../../npc/npc_escortAI.h"

/*######
## npc_aeranas
######*/

#define SAY_SUMMON                      -1000138
#define SAY_FREE                        -1000139

#define FACTION_HOSTILE                 16
#define FACTION_FRIENDLY                35

#define SPELL_ENVELOPING_WINDS          15535
#define SPELL_SHOCK                     12553

#define C_AERANAS                       17085

struct TRINITY_DLL_DECL npc_aeranasAI : public ScriptedAI
{
    npc_aeranasAI(Creature* c) : ScriptedAI(c) {}

    uint32 Faction_Timer;
    uint32 EnvelopingWinds_Timer;
    uint32 Shock_Timer;

    void Reset()
    {
        Faction_Timer = 8000;
        EnvelopingWinds_Timer = 9000;
        Shock_Timer = 5000;

        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        m_creature->setFaction(FACTION_FRIENDLY);

        DoScriptText(SAY_SUMMON, m_creature);
    }

    void Aggro(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if (Faction_Timer)
        {
            if (Faction_Timer < diff)
            {
                m_creature->setFaction(FACTION_HOSTILE);
                Faction_Timer = 0;
            }else Faction_Timer -= diff;
        }

        if (!UpdateVictim())
            return;

        if ((m_creature->GetHealth()*100) / m_creature->GetMaxHealth() < 30)
        {
            m_creature->setFaction(FACTION_FRIENDLY);
            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            m_creature->RemoveAllAuras();
            m_creature->DeleteThreatList();
            m_creature->CombatStop();
            DoScriptText(SAY_FREE, m_creature);
            return;
        }

        if (Shock_Timer < diff)
        {
            DoCast(m_creature->getVictim(),SPELL_SHOCK);
            Shock_Timer = 10000;
        }else Shock_Timer -= diff;

        if (EnvelopingWinds_Timer < diff)
        {
            DoCast(m_creature->getVictim(),SPELL_ENVELOPING_WINDS);
            EnvelopingWinds_Timer = 25000;
        }else EnvelopingWinds_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_aeranas(Creature *_Creature)
{
    return new npc_aeranasAI (_Creature);
}

/*######
## go_haaleshi_altar
######*/

bool GOHello_go_haaleshi_altar(Player *player, GameObject* _GO)
{
    _GO->SummonCreature(C_AERANAS,-1321.79, 4043.80, 116.24, 1.25, TEMPSUMMON_TIMED_DESPAWN, 180000);
    return false;
}

/*######
## npc_wing_commander_dabiree
######*/

#define GOSSIP_ITEM1_DAB "Fly me to Murketh and Shaadraz Gateways"
#define GOSSIP_ITEM2_DAB "Fly me to Shatter Point"

bool GossipHello_npc_wing_commander_dabiree(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    //Mission: The Murketh and Shaadraz Gateways
    if (player->GetQuestStatus(10146) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM1_DAB, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    //Shatter Point
    if (!player->GetQuestRewardStatus(10340))
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM2_DAB, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_wing_commander_dabiree(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,33768,true);               //TaxiPath 585 (Gateways Murket and Shaadraz)
    }
    if (action == GOSSIP_ACTION_INFO_DEF + 2)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,35069,true);               //TaxiPath 612 (Taxi - Hellfire Peninsula - Expedition Point to Shatter Point)
    }
    return true;
}

/*######
## npc_gryphoneer_windbellow
######*/

#define GOSSIP_ITEM1_WIN "Fly me to The Abyssal Shelf"
#define GOSSIP_ITEM2_WIN "Fly me to Honor Point"

bool GossipHello_npc_gryphoneer_windbellow(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    //Mission: The Abyssal Shelf || Return to the Abyssal Shelf
    if (player->GetQuestStatus(10163) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10346) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM1_WIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    //Go to the Front
    if (player->GetQuestStatus(10382) != QUEST_STATUS_NONE && !player->GetQuestRewardStatus(10382))
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM2_WIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_gryphoneer_windbellow(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF + 1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,33899,true);               //TaxiPath 589 (Aerial Assault Flight (Alliance))
    }
    if (action == GOSSIP_ACTION_INFO_DEF + 2)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,35065,true);               //TaxiPath 607 (Taxi - Hellfire Peninsula - Shatter Point to Beach Head)
    }
    return true;
}

/*######
## npc_wing_commander_brack
######*/

#define GOSSIP_ITEM1_BRA "Fly me to Murketh and Shaadraz Gateways"
#define GOSSIP_ITEM2_BRA "Fly me to The Abyssal Shelf"
#define GOSSIP_ITEM3_BRA "Fly me to Spinebreaker Post"

bool GossipHello_npc_wing_commander_brack(Player *player, Creature *_Creature)
{
    if (_Creature->isQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    //Mission: The Murketh and Shaadraz Gateways
    if (player->GetQuestStatus(10129) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM1_BRA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    //Mission: The Abyssal Shelf || Return to the Abyssal Shelf
    if (player->GetQuestStatus(10162) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(10347) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM2_BRA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    //Spinebreaker Post
    if (player->GetQuestStatus(10242) == QUEST_STATUS_COMPLETE && !player->GetQuestRewardStatus(10242))
        player->ADD_GOSSIP_ITEM(2, GOSSIP_ITEM3_BRA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_wing_commander_brack(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch(action)
    {
    case GOSSIP_ACTION_INFO_DEF + 1:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,33659,true);               //TaxiPath 584 (Gateways Murket and Shaadraz)
        break;
    case GOSSIP_ACTION_INFO_DEF + 2:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,33825,true);               //TaxiPath 587 (Aerial Assault Flight (Horde))
        break;
    case GOSSIP_ACTION_INFO_DEF + 3:
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,34578,true);               //TaxiPath 604 (Taxi - Reaver's Fall to Spinebreaker Ridge)
        break;
    }
        return true;
}

/*######
## npc_wounded_blood_elf
######*/

#define SAY_ELF_START               -1000117
#define SAY_ELF_SUMMON1             -1000118
#define SAY_ELF_RESTING             -1000119
#define SAY_ELF_SUMMON2             -1000120
#define SAY_ELF_COMPLETE            -1000121
#define SAY_ELF_AGGRO               -1000122

#define QUEST_ROAD_TO_FALCON_WATCH  9375

struct TRINITY_DLL_DECL npc_wounded_blood_elfAI : public npc_escortAI
{
    npc_wounded_blood_elfAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* player = Unit::GetPlayer(PlayerGUID);

        if (!player)
            return;

        switch (i)
        {
        case 0:
            DoScriptText(SAY_ELF_START, m_creature, player);
            break;
        case 9:
            DoScriptText(SAY_ELF_SUMMON1, m_creature, player);
            // Spawn two Haal'eshi Talonguard
            DoSpawnCreature(16967, -15, -15, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            DoSpawnCreature(16967, -17, -17, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            break;
        case 13:
            DoScriptText(SAY_ELF_RESTING, m_creature, player);
            // make the NPC kneel
            m_creature->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);
            break;
        case 14:
            DoScriptText(SAY_ELF_SUMMON2, m_creature, player);
            // Spawn two Haal'eshi Windwalker
            DoSpawnCreature(16966, -15, -15, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            DoSpawnCreature(16966, -17, -17, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
            break;
        case 27:
            DoScriptText(SAY_ELF_COMPLETE, m_creature, player);
            // Award quest credit
            Player* player = Unit::GetPlayer(PlayerGUID);
            if (player)
                player->GroupEventHappens(QUEST_ROAD_TO_FALCON_WATCH,m_creature);
            break;
        }
    }

    void Reset()
    {
        if (!IsBeingEscorted)
            m_creature->setFaction(1604);
    }

    void Aggro(Unit* who)
    {
        if (IsBeingEscorted)
            DoScriptText(SAY_ELF_AGGRO, m_creature);
    }

    void JustSummoned(Creature* summoned)
    {
        summoned->AI()->AttackStart(m_creature);
    }

    void JustDied(Unit* killer)
    {
        if (!IsBeingEscorted)
            return;

        if (PlayerGUID)
        {
            // If NPC dies, player fails the quest
            Player* player = Unit::GetPlayer(PlayerGUID);
            if (player)
                player->FailQuest(QUEST_ROAD_TO_FALCON_WATCH);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
    }
};

CreatureAI* GetAI_npc_wounded_blood_elf(Creature *_Creature)
{
    npc_wounded_blood_elfAI* welfAI = new npc_wounded_blood_elfAI(_Creature);

    welfAI->AddWaypoint(0, -1137.72, 4272.10, 14.04, 5000);
    welfAI->AddWaypoint(1, -1141.34, 4232.42, 14.63);
    welfAI->AddWaypoint(2, -1133.47, 4220.88, 11.78);
    welfAI->AddWaypoint(3, -1126.18, 4213.26, 13.51);
    welfAI->AddWaypoint(4, -1100.12, 4204.32, 16.41);
    welfAI->AddWaypoint(5, -1063.68, 4197.92, 15.51);
    welfAI->AddWaypoint(6, -1027.28, 4194.36, 15.52);
    welfAI->AddWaypoint(7, -995.68, 4189.59, 19.84);
    welfAI->AddWaypoint(8, -970.90, 4188.60, 24.61);
    welfAI->AddWaypoint(9, -961.93, 4193.34, 26.11, 15000); // Summon 1
    welfAI->AddWaypoint(10, -935.52, 4210.99, 31.98);
    welfAI->AddWaypoint(11, -913.42, 4218.27, 37.29);
    welfAI->AddWaypoint(12, -896.53, 4207.73, 43.23);
    welfAI->AddWaypoint(13, -868.49, 4194.77, 46.75, 30000);// Kneel and Rest Here
    welfAI->AddWaypoint(14, -852.83, 4198.29, 47.28, 15000);// Summon 2
    welfAI->AddWaypoint(15, -819.85, 4200.50, 46.37);
    welfAI->AddWaypoint(16, -791.92, 4201.96, 44.19);
    welfAI->AddWaypoint(17, -774.42, 4202.46, 47.41);
    welfAI->AddWaypoint(18, -762.90, 4202.17, 48.81);
    welfAI->AddWaypoint(19, -728.25, 4195.35, 50.68);
    welfAI->AddWaypoint(20, -713.58, 4192.07, 53.98);
    welfAI->AddWaypoint(21, -703.09, 4189.74, 56.96);
    welfAI->AddWaypoint(22, -693.70, 4185.43, 57.06);
    welfAI->AddWaypoint(23, -686.38, 4159.81, 60.26);
    welfAI->AddWaypoint(24, -679.88, 4147.04, 64.20);
    welfAI->AddWaypoint(25, -656.74, 4147.72, 64.11);
    welfAI->AddWaypoint(26, -652.22, 4137.50, 64.58);
    welfAI->AddWaypoint(27, -649.99, 4136.38, 64.63, 30000);// Award Quest Credit

    return (CreatureAI*)welfAI;
}

bool QuestAccept_npc_wounded_blood_elf(Player* player, Creature* creature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_ROAD_TO_FALCON_WATCH)
    {
        ((npc_escortAI*)(creature->AI()))->Start(true, true, false, player->GetGUID());
        // Change faction so mobs attack
        creature->setFaction(775);
    }

    return true;
}

/*######
## npc_demoniac_scryer
######*/

#define HELLFIRE_WANDLING   22259
#define FEL_WARDEN          22273

#define QUEST_DEMO_SCRYER   10838

struct TRINITY_DLL_DECL npc_demoniac_scryerAI : public ScriptedAI
{
    npc_demoniac_scryerAI(Creature* c) : ScriptedAI(c) {}
    
    uint32 WandlingTimer;
    uint32 WardenTimer;
    uint8 WandlingCount;
    bool WardenSpawned;
    bool sayComeOn;
    Player* player;
    
    void Reset()
    {
        WandlingTimer = 12000;
        WardenTimer = 100000;
        WardenSpawned = false;
        WandlingCount = 0;
        sayComeOn = false;
    }
    
    void Aggro(Unit* who)
    {
    }
    
    void MoveInLineOfSight(Unit* who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
        {
            if (m_creature->GetDistance2d(who) < 15)
            {
                sLog.outString("Player spotted : %s", who->GetName());
                player = (Player*)who;
            }
        }
    }
    
    void UpdateAI(const uint32 diff)
    {
        if (WandlingCount >= 14)
        {
            //set GOSSIP flag on creature, then player has to speak to it to obtain quest item
            if (!sayComeOn)
            {
                m_creature->Say("Relevé établi. Prêt à être retiré.", LANG_UNIVERSAL, 0);
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                sayComeOn = true;
            }
            
            return;
        }
        
        if (WandlingTimer < diff)
        {
            m_creature->SummonCreature(HELLFIRE_WANDLING, m_creature->GetPositionX()+2, m_creature->GetPositionY()+2, m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(player);
            WandlingTimer = 12000;
            WandlingCount++;
        }else WandlingTimer -= diff;
        
        if (WardenTimer < diff && !WardenSpawned)
        {
            m_creature->SummonCreature(FEL_WARDEN, m_creature->GetPositionX()+2, m_creature->GetPositionY()+2, m_creature->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000)->AI()->AttackStart(player);
            WardenSpawned = true;
        }else WardenTimer -= diff;
    }
};

bool GossipHello_npc_demoniac_scryer(Player* player, Creature* _Creature)
{
    ItemPosCountVec dest;
    uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, 31607, 1);
    if (msg == EQUIP_ERR_OK)
    {
       Item* item = player->StoreNewItem( dest, 31607, true);
       player->SendNewItem(item,1,true,false);
    }
    
    return true;
}

CreatureAI* GetAI_npc_demoniac_scryer(Creature *pCreature)
{
    return new npc_demoniac_scryerAI(pCreature);
}

/*######
## npc_fel_guard_hound
######*/

#define SPELL_SUMMON_POO    37688
#define SPELL_STANKY        37695

#define DERANGED_HELBOAR    16863

struct TRINITY_DLL_DECL npc_fel_guard_houndAI : public ScriptedAI
{
    npc_fel_guard_houndAI(Creature* c) : ScriptedAI(c) {}
    
    uint32 checkTimer;
    uint64 lastHelboar; //store last helboar GUID to prevent multiple spawns of poo with the same mob
    
    void Reset()
    {
        m_creature->GetMotionMaster()->MoveFollow(m_creature->GetOwner(), PET_FOLLOW_DIST, PET_FOLLOW_ANGLE);
        checkTimer = 5000; //check for creature every 5 sec
    }
    
    void Aggro(Unit* who) {}
    
    Creature* SelectCreatureInGrid(uint32 entry, float range)
    {
        Creature* pCreature = NULL;

        CellPair pair(Trinity::ComputeCellPair(m_creature->GetPositionX(), m_creature->GetPositionY()));
        Cell cell(pair);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();

        Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*m_creature, entry, false, range); //false, as it should check only for dead creatures
        Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(pCreature, creature_check);

        TypeContainerVisitor<Trinity::CreatureLastSearcher<Trinity::NearestCreatureEntryWithLiveStateInObjectRangeCheck>, GridTypeMapContainer> creature_searcher(searcher);

        CellLock<GridReadGuard> cell_lock(cell, pair);
        cell_lock->Visit(cell_lock, creature_searcher,*(m_creature->GetMap()));
        
        return pCreature;
    }
    
    void UpdateAI(const uint32 diff)
    {
        if (checkTimer < diff)
        {
            Creature* helboar = SelectCreatureInGrid(DERANGED_HELBOAR, 10);
            if (helboar && helboar->GetGUID() != lastHelboar)
            {
                lastHelboar = helboar->GetGUID();
                DoCast(m_creature, SPELL_SUMMON_POO);
                DoCast(m_creature->GetOwner(), SPELL_STANKY);
                helboar->RemoveCorpse();
                checkTimer = 5000;
            }
        }else checkTimer -= diff;
    }
};

CreatureAI* GetAI_npc_fel_guard_hound(Creature *pCreature)
{
    return new npc_fel_guard_houndAI(pCreature);
}

/*######
## AddSC
######*/

void AddSC_hellfire_peninsula()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "npc_aeranas";
    newscript->GetAI = &GetAI_npc_aeranas;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "go_haaleshi_altar";
    newscript->pGOHello = &GOHello_go_haaleshi_altar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wing_commander_dabiree";
    newscript->pGossipHello =   &GossipHello_npc_wing_commander_dabiree;
    newscript->pGossipSelect =  &GossipSelect_npc_wing_commander_dabiree;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_gryphoneer_windbellow";
    newscript->pGossipHello =   &GossipHello_npc_gryphoneer_windbellow;
    newscript->pGossipSelect =  &GossipSelect_npc_gryphoneer_windbellow;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wing_commander_brack";
    newscript->pGossipHello =   &GossipHello_npc_wing_commander_brack;
    newscript->pGossipSelect =  &GossipSelect_npc_wing_commander_brack;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_wounded_blood_elf";
    newscript->GetAI = &GetAI_npc_wounded_blood_elf;
    newscript->pQuestAccept = &QuestAccept_npc_wounded_blood_elf;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_demoniac_scryer";
    newscript->GetAI = &GetAI_npc_demoniac_scryer;
    newscript->pGossipHello = &GossipHello_npc_demoniac_scryer;
    newscript->RegisterSelf();
    
    newscript = new Script;
    newscript->Name="npc_fel_guard_hound";
    newscript->GetAI = &GetAI_npc_fel_guard_hound;
    newscript->RegisterSelf();
}

