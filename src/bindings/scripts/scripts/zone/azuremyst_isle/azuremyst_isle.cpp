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
SDName: Azuremyst_Isle
SD%Complete: 75
SDComment: Quest support: 9283, 9537, 9582, 9554, 9531, 9303(special flight path, proper model for mount missing). Injured Draenei cosmetic only, 9582.
SDCategory: Azuremyst Isle
EndScriptData */

/* ContentData
npc_draenei_survivor
npc_engineer_spark_overgrind
npc_injured_draenei
npc_magwin
npc_susurrus
npc_geezle
mob_nestlewood_owlkin
go_ravager_cage
npc_death_ravager
EndContentData */

#include "precompiled.h"
#include "../../npc/npc_escortAI.h"
#include <cmath>

/*######
## npc_draenei_survivor
######*/

enum eDraeneiSurvivor
{
HEAL1       = -1000248,
HEAL2       = -1000249,
HEAL3       = -1000250,
HEAL4       = -1000251,

HELP1       = -1000252,
HELP2       = -1000253,
HELP3       = -1000254,
HELP4       = -1000255
};

struct npc_draenei_survivorAI : public ScriptedAI
{
    npc_draenei_survivorAI(Creature *c) : ScriptedAI(c) {}

    uint32 UnSpawnTimer;
    uint32 ResetlifeTimer;
    uint32 SayingTimer;
    uint32 HealSayTimer;
    bool UnSpawn;
    bool say;
    bool HealSay;
    bool isRun;
    bool isMove;

    void Reset()
    {
        UnSpawnTimer = 2500;
        ResetlifeTimer= 60000;
        SayingTimer    = 5000;
        HealSayTimer = 6000;
        say = false;
        isRun = false;
        isMove = false;
        UnSpawn    = false;
        HealSay = false;
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
        //cast red shining
        m_creature->CastSpell(m_creature, 29152, false, NULL);
        //set creature health
        m_creature->SetHealth(int(m_creature->GetMaxHealth()*.1));
        m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 3);
    }

    void Aggro(Unit *who) {}

    void MoveInLineOfSight(Unit *who)                       //MoveInLineOfSight is called if creature could see you, updated all 100 ms
    {
        if (!who)
            return;

        if(who->GetTypeId() == TYPEID_PLAYER && m_creature->IsFriendlyTo(who) && m_creature->IsWithinDistInMap(who, 15) && say && !isRun)
        {
            switch (rand()%4)                               //Random switch between 4 texts
            {
                case 0:
                    DoScriptText(HELP1, m_creature);
                    SayingTimer = 15000;
                    say = false;
                    break;
                case 1:
                    DoScriptText(HELP2, m_creature);
                    SayingTimer = 15000;
                    say = false;
                    break;
                case 2:
                    DoScriptText(HELP3, m_creature);
                    SayingTimer = 15000;
                    say = false;
                    break;
                case 3:
                    DoScriptText(HELP4, m_creature);
                    SayingTimer = 15000;
                    say = false;
                    break;
            }
        }
        else
        {
            isRun = false;
        }
    }

    void UpdateAI(const uint32 diff)                        //Is also called each ms for Creature AI Updates...
    {
        if (m_creature->GetHealth() > 50)
        {
            if(ResetlifeTimer < diff)
            {
                ResetlifeTimer = 60000;
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
                //set creature health
                m_creature->SetHealth(int(m_creature->GetMaxHealth()*.1));
                // ley down
                m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1,3);
            }
            else ResetlifeTimer -= diff;
        }

        if(HealSay)
        {
            if (HealSayTimer < diff)
            {
                UnSpawn = true;
                isRun = true;
                isMove = true;
            }else HealSayTimer -= diff;
        }

        if(UnSpawn)
        {
            if(isMove)
            {
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MovePoint(0, -4115.053711f, -13754.831055f, 73.508949f);
                isMove = false;
            }

            if (UnSpawnTimer < diff)
            {
                m_creature->StopMoving();
                EnterEvadeMode();
                //set creature health
                m_creature->SetHealth(int(m_creature->GetMaxHealth()*.1));
                return;
            }else UnSpawnTimer -= diff;
        }

        if(SayingTimer < diff)
        {
            say = true;
        }else SayingTimer -= diff;
    }

    void SpellHit(Unit *Hitter, const SpellEntry *Spellkind)//Called if you cast a spell and do some things if Specified spell is true!
    {
        if (Hitter && Spellkind->Id == 28880)
        {
            m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
            m_creature->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
            m_creature->HandleEmoteCommand(ANIM_RISE);
            switch (rand()%4)                               //This switch doesn't work at all, creature say nothing!
            {
                case 0: DoScriptText(HEAL1, m_creature, Hitter); break;
                case 1: DoScriptText(HEAL2, m_creature, Hitter); break;
                case 2: DoScriptText(HEAL3, m_creature, Hitter); break;
                case 3: DoScriptText(HEAL4, m_creature, Hitter); break;
            }
            HealSay    = true;
        }
    }
};

CreatureAI* GetAI_npc_draenei_survivor(Creature *pCreature)
{
    return new npc_draenei_survivorAI (pCreature);
}

/*######
## npc_engineer_spark_overgrind
######*/

enum eSparkOvergrind
{
SAY_TEXT        = -1000256,
SAY_EMOTE       = -1000257,
ATTACK_YELL     = -1000258,

SPELL_DYNAMITE  = 7978
};

#define GOSSIP_FIGHT    "Traitor! You will be brought to justice!"

struct npc_engineer_spark_overgrindAI : public ScriptedAI
{
    npc_engineer_spark_overgrindAI(Creature *c) : ScriptedAI(c) {}

    uint32 Dynamite_Timer;
    uint32 Emote_Timer;

    void Reset()
    {
        Dynamite_Timer = 8000;
        Emote_Timer = 120000 + rand()%30000;
        m_creature->setFaction(875);
    }

    void Aggro(Unit *who) { }

    void UpdateAI(const uint32 diff)
    {
        if( !InCombat )
        {
            if (Emote_Timer < diff)
            {
                DoScriptText(SAY_TEXT, m_creature);
                DoScriptText(SAY_EMOTE, m_creature);
                Emote_Timer = 120000 + rand()%30000;
            }else Emote_Timer -= diff;
        }

        if(!UpdateVictim())
            return;

        if (Dynamite_Timer < diff)
        {
            DoCast(m_creature->getVictim(), SPELL_DYNAMITE);
            Dynamite_Timer = 8000;
        } else Dynamite_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_engineer_spark_overgrind(Creature *pCreature)
{
    return new npc_engineer_spark_overgrindAI (pCreature);
}

bool GossipHello_npc_engineer_spark_overgrind(Player *pPlayer, Creature *pCreature)
{
    if( pPlayer->GetQuestStatus(9537) == QUEST_STATUS_INCOMPLETE )
        pPlayer->ADD_GOSSIP_ITEM(0, GOSSIP_FIGHT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());
    return true;
}

bool GossipSelect_npc_engineer_spark_overgrind(Player *pPlayer, Creature *pCreature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF )
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->setFaction(14);
        DoScriptText(ATTACK_YELL, pCreature, pPlayer);
        ((npc_engineer_spark_overgrindAI*)pCreature->AI())->AttackStart(pPlayer);
    }
    return true;
}

/*######
## npc_injured_draenei
######*/

struct npc_injured_draeneiAI : public ScriptedAI
{
    npc_injured_draeneiAI(Creature *c) : ScriptedAI(c) {}

    void Reset()
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
        m_creature->SetHealth(int(m_creature->GetMaxHealth()*.15));
        switch (rand()%2)
        {
            case 0: m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 1); break;
            case 1: m_creature->SetUInt32Value(UNIT_FIELD_BYTES_1, 3); break;
        }
    }

    void Aggro(Unit *who) {}

    void MoveInLineOfSight(Unit *who)
    {
        return;                                             //ignore everyone around them (won't aggro anything)
    }

    void UpdateAI(const uint32 diff)
    {
        return;
    }

};
CreatureAI* GetAI_npc_injured_draenei(Creature *pCreature)
{
    return new npc_injured_draeneiAI (pCreature);
}

/*######
## npc_magwin
######*/

enum eMagwin
{
SAY_START               = -1000111,
SAY_AGGRO               = -1000112,
SAY_PROGRESS            = -1000113,
SAY_END1                = -1000114,
SAY_END2                = -1000115,
EMOTE_HUG               = -1000116,

QUEST_A_CRY_FOR_HELP    = 9528
};

struct npc_magwinAI : public npc_escortAI
{
    npc_magwinAI(Creature *c) : npc_escortAI(c) {}


    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        switch(i)
        {
        case 0:
            DoScriptText(SAY_START, m_creature, player);
            break;
        case 17:
            DoScriptText(SAY_PROGRESS, m_creature, player);
            break;
        case 28:
            DoScriptText(SAY_END1, m_creature, player);
            break;
        case 29:
            DoScriptText(EMOTE_HUG, m_creature, player);
            DoScriptText(SAY_END2, m_creature, player);
            player->GroupEventHappens(QUEST_A_CRY_FOR_HELP,m_creature);
            break;
        }
    }

    void Aggro(Unit* who)
    {
        DoScriptText(SAY_AGGRO, m_creature, who);
    }

    void Reset()
    {
        if (!IsBeingEscorted)
            m_creature->setFaction(80);
    }

    void JustDied(Unit* killer)
    {
        if (PlayerGUID)
        {
            Player* player = GetPlayerForEscort();
            if (player)
                player->FailQuest(QUEST_A_CRY_FOR_HELP);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);
    }
};

bool QuestAccept_npc_magwin(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_A_CRY_FOR_HELP)
    {
        pCreature->setFaction(113);
        if (npc_escortAI* pEscortAI = CAST_AI(npc_magwinAI, (pCreature->AI())))
            pEscortAI->Start(true, true, false, pPlayer->GetGUID(), pCreature->GetEntry());
    }
    return true;
}

CreatureAI* GetAI_npc_magwinAI(Creature *pCreature)
{
    return new npc_magwinAI(pCreature);
}

/*######
## npc_susurrus
######*/

bool GossipHello_npc_susurrus(Player *pPlayer, Creature *pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->HasItemCount(23843, 1, true))
        pPlayer->ADD_GOSSIP_ITEM(0, "I am ready.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(pCreature->GetNpcTextId(), pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_susurrus(Player *pPlayer, Creature *pCreature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pPlayer->CastSpell(pPlayer,32474,true);               //apparently correct spell, possible not correct place to cast, or correct caster

        std::vector<uint32> nodes;

        nodes.resize(2);
        nodes[0] = 92;                                      //from susurrus
        nodes[1] = 91;                                      //end at exodar
        pPlayer->ActivateTaxiPathTo(nodes,11686);            //TaxiPath 506. Using invisible model, possible Trinity must allow 0(from dbc) for cases like this.
    }
    return true;
}

/*######
## npc_geezle
######*/

enum eGeezle
{
GEEZLE_SAY_1    = -1000259,
SPARK_SAY_2     = -1000260,
SPARK_SAY_3     = -1000261,
GEEZLE_SAY_4    = -1000262,
SPARK_SAY_5     = -1000263,
SPARK_SAY_6     = -1000264,
GEEZLE_SAY_7    = -1000265,

EMOTE_SPARK     = -1000266,

MOB_SPARK       = 17243,
GO_NAGA_FLAG    = 181694
};

static float SparkPos[3] = {-5030.95, -11291.99, 7.97};

struct npc_geezleAI : public ScriptedAI
{
    npc_geezleAI(Creature *c) : ScriptedAI(c) {}

    std::list<GameObject*> FlagList;

    uint64 SparkGUID;

    uint32 Step;
    uint32 SayTimer;

    bool EventStarted;

    void Reset()
    {
        SparkGUID = 0;
        Step = 0;
        StartEvent();
    }

    void Aggro(Unit* who){}

    void StartEvent()
    {
        Step = 1;
        EventStarted = true;
        Creature* Spark = m_creature->SummonCreature(MOB_SPARK, SparkPos[0], SparkPos[1], SparkPos[2], 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
        if(Spark)
        {
            SparkGUID = Spark->GetGUID();
            Spark->setActive(true);
            Spark->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            Spark->GetMotionMaster()->MovePoint(0, -5080.70, -11253.61, 0.56);
        }
        m_creature->GetMotionMaster()->MovePoint(0, -5092.26, -11252, 0.71);
        SayTimer = 23000;
    }

    uint32 NextStep(uint32 Step)
    {
        Unit* Spark = Unit::GetUnit((*m_creature), SparkGUID);

        switch(Step)
        {
        case 0: return 99999;
        case 1:
            //DespawnNagaFlag(true);
            DoScriptText(EMOTE_SPARK, Spark);
            return 1000;
        case 2:
            DoScriptText(GEEZLE_SAY_1, m_creature, Spark);
            if(Spark)
            {
                Spark->SetInFront(m_creature);
                m_creature->SetInFront(Spark);
            }
            return 5000;
        case 3: DoScriptText(SPARK_SAY_2, Spark); return 7000;
        case 4: DoScriptText(SPARK_SAY_3, Spark); return 8000;
        case 5: DoScriptText(GEEZLE_SAY_4, m_creature, Spark); return 8000;
        case 6: DoScriptText(SPARK_SAY_5, Spark); return 9000;
        case 7: DoScriptText(SPARK_SAY_6, Spark); return 8000;
        case 8: DoScriptText(GEEZLE_SAY_7, m_creature, Spark); return 2000;
        case 9:
            m_creature->GetMotionMaster()->MoveTargetedHome();
            if(Spark)
                Spark->GetMotionMaster()->MovePoint(0, -5030.95, -11291.99, 7.97);
            return 20000;
        case 10:
            if(Spark)
                Spark->DealDamage(Spark,Spark->GetHealth(),NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            //DespawnNagaFlag(false);
            m_creature->SetVisibility(VISIBILITY_OFF);
        default: return 99999999;
        }
    }

    void DespawnNagaFlag(bool despawn)
    {
        CellPair pair(Trinity::ComputeCellPair(m_creature->GetPositionX(), m_creature->GetPositionY()));
        Cell cell(pair);
        cell.data.Part.reserved = ALL_DISTRICT;
        cell.SetNoCreate();

        Trinity::AllGameObjectsWithEntryInGrid go_check(GO_NAGA_FLAG);
        Trinity::GameObjectListSearcher<Trinity::AllGameObjectsWithEntryInGrid> go_search(FlagList, go_check);
        TypeContainerVisitor
            <Trinity::GameObjectListSearcher<Trinity::AllGameObjectsWithEntryInGrid>, GridTypeMapContainer> go_visit(go_search);
        cell.Visit(pair, go_visit, *(m_creature->GetMap()));

        Player* player = NULL;
        if (!FlagList.empty())
        {
            for(std::list<GameObject*>::iterator itr = FlagList.begin(); itr != FlagList.end(); ++itr)
            {
                //TODO: Found how to despawn and respawn
                if(despawn)
                    (*itr)->RemoveFromWorld();
                else
                    (*itr)->Respawn();
            }
        } else error_log("SD2 ERROR: FlagList is empty!");
    }

    void UpdateAI(const uint32 diff)
    {
        if(SayTimer < diff)
        {
            if(EventStarted)
            {
                SayTimer = NextStep(++Step);
            }
        }else SayTimer -= diff;
    }
};

CreatureAI* GetAI_npc_geezleAI(Creature *pCreature)
{
    return new npc_geezleAI(pCreature);
}

/*######
## mob_nestlewood_owlkin
######*/

enum eNestlewoodOwlkin
{
INOCULATION_CHANNEL = 29528,
INOCULATED_OWLKIN   = 16534
};

struct mob_nestlewood_owlkinAI : public ScriptedAI
{
    mob_nestlewood_owlkinAI(Creature *c) : ScriptedAI(c) {}

    uint32 ChannelTimer;
    bool Channeled;
    bool Hitted;

    void Reset()
    {
        ChannelTimer = 0;
        Channeled = false;
        Hitted = false;
    }

    void Aggro(Unit *who){}

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if(!caster)
            return;

        if(caster->GetTypeId() == TYPEID_PLAYER && spell->Id == INOCULATION_CHANNEL)
        {
            ChannelTimer = 3000;
            Hitted = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(ChannelTimer < diff && !Channeled && Hitted)
        {
            m_creature->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            m_creature->RemoveCorpse();
            m_creature->SummonCreature(INOCULATED_OWLKIN, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 180000);
            Channeled = true;
        }else ChannelTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_nestlewood_owlkinAI(Creature *pCreature)
{
    return new mob_nestlewood_owlkinAI (pCreature);
}

/*######
## go_ravager_cage
######*/

enum eRavegerCage
{
NPC_DEATH_RAVAGER       = 17556,

SPELL_REND              = 13443,
SPELL_ENRAGING_BITE     = 30736,

QUEST_STRENGTH_ONE      = 9582
};

bool go_ravager_cage(Player* pPlayer, GameObject* pGo)
{

    if(pPlayer->GetQuestStatus(QUEST_STRENGTH_ONE) == QUEST_STATUS_INCOMPLETE)
    {
        if(Creature* ravager = pGo->FindCreatureInGrid(NPC_DEATH_RAVAGER, 5.0f, true))
        {
            ravager->RemoveFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
            ravager->SetReactState(REACT_AGGRESSIVE);
            ravager->AI()->AttackStart(pPlayer);
        }
    }
    return true ;
}

/*######
## npc_death_ravager
######*/

struct npc_death_ravagerAI : public ScriptedAI
{
    npc_death_ravagerAI(Creature *c) : ScriptedAI(c){}

    uint32 RendTimer;
    uint32 EnragingBiteTimer;

    void Reset()
    {
        RendTimer = 30000;
        EnragingBiteTimer = 20000;

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetReactState(REACT_PASSIVE);
    }
    
    void Aggro(Unit* who) {}
    
    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if(RendTimer <= diff)
        {
            DoCast(m_creature->getVictim(), SPELL_REND);
            RendTimer = 30000;
        }
        else RendTimer -= diff;

        if(EnragingBiteTimer <= diff)
        {
            DoCast(m_creature->getVictim(), SPELL_ENRAGING_BITE);
            EnragingBiteTimer = 15000;
        }
        else EnragingBiteTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_death_ravagerAI(Creature* pCreature)
{
    return new npc_death_ravagerAI(pCreature);
}

/*######
## AddSC
######*/

void AddSC_azuremyst_isle()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_draenei_survivor";
    newscript->GetAI = &GetAI_npc_draenei_survivor;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_engineer_spark_overgrind";
    newscript->GetAI = &GetAI_npc_engineer_spark_overgrind;
    newscript->pGossipHello =  &GossipHello_npc_engineer_spark_overgrind;
    newscript->pGossipSelect = &GossipSelect_npc_engineer_spark_overgrind;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_injured_draenei";
    newscript->GetAI = &GetAI_npc_injured_draenei;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_magwin";
    newscript->GetAI = &GetAI_npc_magwinAI;
    newscript->pQuestAccept = &QuestAccept_npc_magwin;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_susurrus";
    newscript->pGossipHello =  &GossipHello_npc_susurrus;
    newscript->pGossipSelect = &GossipSelect_npc_susurrus;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_geezle";
    newscript->GetAI = &GetAI_npc_geezleAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_nestlewood_owlkin";
    newscript->GetAI = &GetAI_mob_nestlewood_owlkinAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_death_ravager";
    newscript->GetAI = &GetAI_npc_death_ravagerAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="go_ravager_cage";
    newscript->pGOHello = &go_ravager_cage;
    newscript->RegisterSelf();
}

