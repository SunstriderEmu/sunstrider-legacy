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
SDName: Karazhan
SD%Complete: 100
SDComment: Support for Barnes (Opera controller) and Berthold (Doorman), Support for Quest 9645.
SDCategory: Karazhan
EndScriptData */

/* ContentData
npc_barnes
npc_berthold
npc_image_of_medivh
npc_archmage_leryda
go_sealed_tome
woefulhealer
npc_arcane_anomaly
EndContentData */


#include "def_karazhan.h"
#include "EscortAI.h"
#include "Chat.h"
#include "ObjectMgr.h"

/*######
# npc_barnesAI
######*/

#define GOSSIP_READY        "I'm not an actor."

#define SAY_READY           "Splendid, I'm going to get the audience ready. Break a leg!"
#define SAY_OZ_INTRO1       "Finally, everything is in place. Are you ready for your big stage debut?"
#define OZ_GOSSIP1          "I'm not an actor."
#define SAY_OZ_INTRO2       "Don't worry, you'll be fine. You look like a natural!"
#define OZ_GOSSIP2          "Ok, I'll give it a try, then."

#define SAY_RAJ_INTRO1      "The romantic plays are really tough, but you'll do better this time. You have TALENT. Ready?"
#define RAJ_GOSSIP1         "I've never been more ready."

struct Dialogue
{
    int32 textid;
    uint32 timer;
};

static Dialogue OzDialogue[]=
{
    {-1532103, 6000},
    {-1532104, 18000},
    {-1532105, 9000},
    {-1532106, 15000}
};

static Dialogue HoodDialogue[]=
{
    {-1532107, 6000},
    {-1532108, 10000},
    {-1532109, 14000},
    {-1532110, 15000}
};

static Dialogue RAJDialogue[]=
{
    {-1532111, 5000},
    {-1532112, 7000},
    {-1532113, 14000},
    {-1532114, 14000}
};

// Entries and spawn locations for creatures in Oz event
float Spawns[6][2]=
{
    {17535, -10896},                                        // Dorothee
    {17546, -10891},                                        // Roar
    {17547, -10884},                                        // Tinhead
    {17543, -10902},                                        // Strawman
    {17603, -10892},                                        // Grandmother
    {17534, -10900},                                        // Julianne
};

float StageLocations[6][2]=
{
    {-10866.711, -1779.816},                                // Open door, begin walking (0)
    {-10894.917, -1775.467},                                // (1)
    {-10896.044, -1782.619},                                // Begin Speech after this (2)
    {-10894.917, -1775.467},                                // Resume walking (back to spawn point now) after speech (3)
    {-10866.711, -1779.816},                                // (4)
    {-10866.700, -1781.030}                                 // Summon mobs, open curtains, close door (5)
};

#define CREATURE_SPOTLIGHT  19525

#define SPELL_SPOTLIGHT     25824
#define SPELL_TUXEDO        32616

#define SPAWN_Z             90.5
#define SPAWN_Y             -1758
#define SPAWN_O             4.738

struct npc_barnesAI : public npc_escortAI
{
    npc_barnesAI(Creature* c) : npc_escortAI(c)
    {
        RaidWiped = false;
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript* pInstance;

    uint64 SpotlightGUID;

    uint32 TalkCount;
    uint32 TalkTimer;
    uint32 CurtainTimer;
    uint32 WipeTimer;
    uint32 Event;

    bool PerformanceReady;
    bool RaidWiped;
    bool IsTalking;

    void Reset()
    override {
        TalkCount = 0;
        TalkTimer = 2000;
        CurtainTimer = 5000;
        WipeTimer = 5000;

        PerformanceReady = false;
        IsTalking = false;

        if(pInstance)
        {
            pInstance->SetData(DATA_OPERA_EVENT, NOT_STARTED);

            Event = pInstance->GetData(DATA_OPERA_PERFORMANCE);

             if (GameObject* Door = GameObject::GetGameObject((*me), pInstance->GetData64(DATA_GAMEOBJECT_STAGEDOORLEFT)))
             {
                if (RaidWiped)
                    Door->SetGoState(GO_STATE_ACTIVE);
                else
                    Door->SetGoState(GO_STATE_READY);
             }

             if (GameObject* Curtain = GameObject::GetGameObject((*me), pInstance->GetData64(DATA_GAMEOBJECT_CURTAINS)))
                Curtain->SetGoState(GO_STATE_READY);
        }
    }

    void EnterCombat(Unit* who) override {}

    void WaypointReached(uint32 i)
    override {
        switch(i)
        {
            case 2:
                RemoveEscortState(STATE_ESCORT_ESCORTING);
                TalkCount = 0;
                IsTalking = true;

                float x,y,z;
                me->GetPosition(x, y, z);
                if (Creature* Spotlight = me->SummonCreature(CREATURE_SPOTLIGHT, x, y, z, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 50000))
                {
                    Spotlight->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    Spotlight->CastSpell(Spotlight, SPELL_SPOTLIGHT, false);
                    SpotlightGUID = Spotlight->GetGUID();
                }
                break;

            case 5:
                if(pInstance)
                {
                    if (GameObject* Door = GameObject::GetGameObject((*me), pInstance->GetData64(DATA_GAMEOBJECT_STAGEDOORLEFT)))
                        Door->SetGoState(GO_STATE_READY);
                }
                RemoveEscortState(STATE_ESCORT_ESCORTING);
                PerformanceReady = true;
                break;
        }
    }

    void Talk(uint32 count)
    {
        int32 text = 0;

        switch(Event)
        {
            case EVENT_OZ:
                if (OzDialogue[count].textid)
                     text = OzDialogue[count].textid;
                if(OzDialogue[count].timer)
                    TalkTimer = OzDialogue[count].timer;
                break;

            case EVENT_HOOD:
                if (HoodDialogue[count].textid)
                    text = HoodDialogue[count].textid;
                if(HoodDialogue[count].timer)
                    TalkTimer = HoodDialogue[count].timer;
                break;

            case EVENT_RAJ:
                 if (RAJDialogue[count].textid)
                     text = RAJDialogue[count].textid;
                if(RAJDialogue[count].timer)
                    TalkTimer = RAJDialogue[count].timer;
                break;
        }

        if(text)
             DoScriptText(text, me);
    }

    void UpdateAI(const uint32 diff)
    override {
        npc_escortAI::UpdateAI(diff);

        if(IsTalking)
        {
            if(TalkTimer < diff)
            {
                if(TalkCount > 3)
                {
                    if (Unit* Spotlight = ObjectAccessor::GetUnit((*me), SpotlightGUID))
                    {
                        Spotlight->RemoveAllAuras();
                        Spotlight->SetVisibility(VISIBILITY_OFF);
                    }

                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                    IsTalking = false;
                    AddEscortState(STATE_ESCORT_ESCORTING);
                    return;
                }

                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_TALK);
                Talk(TalkCount);
                ++TalkCount;
            }else TalkTimer -= diff;
        }

        if(PerformanceReady)
        {
            if(CurtainTimer)
            {
                if(CurtainTimer <= diff)
                {
                    PrepareEncounter();

                    if(!pInstance)
                        return;

                    if (GameObject* Curtain = GameObject::GetGameObject((*me), pInstance->GetData64(DATA_GAMEOBJECT_CURTAINS)))
                        Curtain->SetGoState(GO_STATE_ACTIVE);

                    CurtainTimer = 0;
                }else CurtainTimer -= diff;
            }

            if(!RaidWiped)
            {
                if(WipeTimer < diff)
                {
                    Map *map = me->GetMap();
                    if(!map->IsDungeon()) return;

                    Map::PlayerList const &PlayerList = map->GetPlayers();
                    if(PlayerList.isEmpty())
                        return;

                    RaidWiped = true;
                    for(const auto & i : PlayerList)
                    {
                        if (i.GetSource()->IsAlive() && !i.GetSource()->IsGameMaster())
                        {
                            RaidWiped = false;
                            break;
                        }
                    }

                    if(RaidWiped)
                    {
                        RaidWiped = true;
                        EnterEvadeMode();
                        return;
                    }

                    WipeTimer = 15000;
                }else WipeTimer -= diff;
            }

        }

        if(!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void StartEvent()
    {
        if(!pInstance)
            return;

        pInstance->SetData(DATA_OPERA_EVENT, IN_PROGRESS);

        if (GameObject* Door = GameObject::GetGameObject((*me), pInstance->GetData64(DATA_GAMEOBJECT_STAGEDOORLEFT)))
            Door->SetGoState(GO_STATE_ACTIVE);

        me->CastSpell(me, SPELL_TUXEDO, true);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        Start(false, false, false);
    }

    void PrepareEncounter()
    {
        uint8 index = 0;
        uint8 count = 0;
        switch(Event)
        {
            case EVENT_OZ:
                index = 0;
                count = 4;
                break;

            case EVENT_HOOD:
                index = 4;
                count = index+1;
                break;

            case EVENT_RAJ:
                index = 5;
                count = index+1;
                break;
        }

        for( ; index < count; ++index)
        {
            uint32 entry = ((uint32)Spawns[index][0]);
            float PosX = Spawns[index][1];
            if (Creature* pCreature = me->SummonCreature(entry, PosX, SPAWN_Y, SPAWN_Z, SPAWN_O, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
            {
                                                            // In case database has bad flags
                pCreature->SetUInt32Value(UNIT_FIELD_FLAGS, 0);
                pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
        }

        CurtainTimer = 10000;
        PerformanceReady = true;
        RaidWiped = false;
    }
};

CreatureAI* GetAI_npc_barnesAI(Creature* _Creature)
{
    auto  Barnes_AI = new npc_barnesAI(_Creature);

    for(uint8 i = 0; i < 6; ++i)
        Barnes_AI->AddWaypoint(i, StageLocations[i][0], StageLocations[i][1], 90.465);

    return ((CreatureAI*)Barnes_AI);
}

bool GossipHello_npc_barnes(Player* player, Creature* _Creature)
{
    // Check for death of Moroes.
    InstanceScript* pInstance = ((InstanceScript*)_Creature->GetInstanceScript());
    if (pInstance && (pInstance->GetData(DATA_OPERA_EVENT) < DONE))
    {
        if(pInstance->GetData(DATA_MOROES_EVENT) >= DONE)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, OZ_GOSSIP1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            if(!((npc_barnesAI*)_Creature->AI())->RaidWiped)
                player->SEND_GOSSIP_MENU_TEXTID(8970, _Creature->GetGUID()); //try again text
            else
                player->SEND_GOSSIP_MENU_TEXTID(8975, _Creature->GetGUID());
        }else {
            player->SEND_GOSSIP_MENU_TEXTID(8978, _Creature->GetGUID()); //Someone should take care of Moroes
        }
    }

    return true;
}

bool GossipSelect_npc_barnes(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, OZ_GOSSIP2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU_TEXTID(8971, _Creature->GetGUID());
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            ((npc_barnesAI*)_Creature->AI())->StartEvent();
            break;
    }

    return true;
}

/*###
# npc_hastings
####*/

#define HASTINGS_TEXT_HELLO             554
#define HASTINGS_TEXT_MENU1             555
#define HASTINGS_TEXT_MENU2             556
#define HASTINGS_GOSSIP_ITEM_1          557
#define HASTINGS_GOSSIP_ITEM_2          19999

bool GossipHello_npc_hastings(Player* player, Creature* _Creature)
{
    player->ADD_GOSSIP_ITEM_TEXTID(0, HASTINGS_GOSSIP_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU_TEXTID(HASTINGS_TEXT_HELLO,_Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_hastings(Player* player, Creature* _Creature, uint32 sender, uint32 action)
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:     
            player->ADD_GOSSIP_ITEM_TEXTID(0, HASTINGS_GOSSIP_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU_TEXTID(HASTINGS_TEXT_MENU1,_Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:     
            player->SEND_GOSSIP_MENU_TEXTID(HASTINGS_TEXT_MENU2,_Creature->GetGUID()); 
            break;
    }

    return true;
}

/*###
# npc_calliard
####*/

#define CALLIARD_TEXT_HELLO             454
#define CALLIARD_TEXT_MENU1             455
#define CALLIARD_GOSSIP_ITEM_1          456

bool GossipHello_npc_calliard(Player* player, Creature* _Creature)
{
    player->ADD_GOSSIP_ITEM_TEXTID(0, CALLIARD_GOSSIP_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
	
    player->SEND_GOSSIP_MENU_TEXTID(CALLIARD_TEXT_HELLO,_Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_calliard(Player* player, Creature* _Creature, uint32 sender, uint32 action)
{
    if(action == GOSSIP_ACTION_INFO_DEF + 1)
        player->SEND_GOSSIP_MENU_TEXTID(CALLIARD_TEXT_MENU1,_Creature->GetGUID());

    return true;
}

/*###
# npc_image_of_medivh
####*/

//TODO translate
#define SAY_DIALOG_MEDIVH_1         "Vous avez attiré mon attention, dragon. Mais on ne m'effraie pas aussi facilement que les villageois d'en-dessous."
#define SAY_DIALOG_ARCANAGOS_2      "Votre jeu avec la force des arcanes est allé trop loin, Medivh. Vous avez attiré l'attention de puissances au-delà de votre compréhension. Vous devez quitter Karazhan sur le champ !!"
#define SAY_DIALOG_MEDIVH_3         "Vous osez me défier dans ma propre demeure ? Votre arrogance est stupéfiante, même pour un dragon !"
#define SAY_DIALOG_ARCANAGOS_4      "Une puissance obscure cherche à vous utiliser, Medivh ! Si vous restez, de sombres jours vont suivre. Il faut vous dépêcher, nous n'avons pas beaucoup de temps !"
#define SAY_DIALOG_MEDIVH_5         "Je ne vois pas de quoi vous voulez parler, dragon... mais je ne vais pas me laisser intimider par tant d'insolence. Je quitterai Karazhan quand je l'aurai décidé !"
#define SAY_DIALOG_ARCANAGOS_6      "Vous ne me laissez pas le choix. Je vous arrêterai par la force si vous ne voulez pas revenir à la raison."
#define EMOTE_DIALOG_MEDIVH_7       "commence à invoquer un sort d'une grande puissance, tissant sa propre essence avec la magie."
#define SAY_DIALOG_ARCANAGOS_8      "Qu'avez-vous fait, magicien ? Cela ne peut être... ! Je brûle de... à l'intérieur !"
#define SAY_DIALOG_MEDIVH_9         "Il n'aurait pas dû me mettre en colère. Je dois désormais aller... récupérer ma puissance..."

#define MOB_ARCANAGOS               17652
#define SPELL_FIRE_BALL             30967
#define SPELL_UBER_FIREBALL         30971
#define SPELL_CONFLAGRATION_BLAST   30977
#define SPELL_MANA_SHIELD           31635

static float MedivPos[4] = {-11161.49,-1902.24,91.48,1.94};
static float ArcanagosPos[4] = {-11169.75,-1881.48,95.39,4.83};

struct npc_image_of_medivhAI : public ScriptedAI
{
    npc_image_of_medivhAI(Creature* c) : ScriptedAI(c)
    {
        pInstance = ((InstanceScript*)c->GetInstanceScript());
    }

    InstanceScript *pInstance;

    uint64 ArcanagosGUID;

    uint32 YellTimer;
    uint32 Step;
    uint32 FireMedivhTimer;
    uint32 FireArcanagosTimer;

    bool EventStarted;

    void Reset()
    override {
        ArcanagosGUID = 0;

        if(pInstance && pInstance->GetData64(DATA_IMAGE_OF_MEDIVH_NIGHTBANE) == 0)
        {
            pInstance->SetData64(DATA_IMAGE_OF_MEDIVH_NIGHTBANE, me->GetGUID());
            (*me).GetMotionMaster()->MovePoint(1,MedivPos[0],MedivPos[1],MedivPos[2]);
            Step = 0;
        }else
        {
            me->DealDamage(me,me->GetHealth(),nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            me->RemoveCorpse();
        }
    }
    void EnterCombat(Unit* who)override {}

    void MovementInform(uint32 type, uint32 id)
    override {
        if(type != POINT_MOTION_TYPE)
            return;
        if(id == 1)
        {
            StartEvent();
            me->SetOrientation(MedivPos[3]);
            me->SetOrientation(MedivPos[3]);
        }
    }

    void StartEvent()
    {
        Step = 1;
        EventStarted = true;
        Creature* Arcanagos = me->SummonCreature(MOB_ARCANAGOS,ArcanagosPos[0],ArcanagosPos[1],ArcanagosPos[2],0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,20000);
        if(!Arcanagos)
            return;
        ArcanagosGUID = Arcanagos->GetGUID();
        Arcanagos->SetDisableGravity(true);
        (*Arcanagos).GetMotionMaster()->MovePoint(0,ArcanagosPos[0],ArcanagosPos[1],ArcanagosPos[2]);
        Arcanagos->SetOrientation(ArcanagosPos[3]);
        me->SetOrientation(MedivPos[3]);
        YellTimer = 10000;
    }


    uint32 NextStep(uint32 Step)
    {
        Unit* arca = ObjectAccessor::GetUnit((*me),ArcanagosGUID);
        Map *map = me->GetMap();
        switch(Step)
        {
        case 1:
            me->Yell(SAY_DIALOG_MEDIVH_1,LANG_UNIVERSAL,nullptr);
            return 10000;
        case 2:
            if(arca)
                (arca->ToCreature())->Yell(SAY_DIALOG_ARCANAGOS_2,LANG_UNIVERSAL,nullptr);
            return 20000;
        case 3:
            me->Yell(SAY_DIALOG_MEDIVH_3,LANG_UNIVERSAL,nullptr);
            return 10000;
        case 4:
            if(arca)
                (arca->ToCreature())->Yell(SAY_DIALOG_ARCANAGOS_4, LANG_UNIVERSAL, nullptr);
            return 20000;
        case 5:
            me->Yell(SAY_DIALOG_MEDIVH_5, LANG_UNIVERSAL, nullptr);
            return 20000;
        case 6:
            if(arca)
                (arca->ToCreature())->Yell(SAY_DIALOG_ARCANAGOS_6, LANG_UNIVERSAL, nullptr);
            return 10000;
        case 7:
            FireArcanagosTimer = 500;
            return 5000;
        case 8:
            FireMedivhTimer = 500;
            DoCast(me, SPELL_MANA_SHIELD);
            return 10000;
        case 9:
            me->TextEmote(EMOTE_DIALOG_MEDIVH_7, nullptr, false);
            return 10000;
        case 10:
            if(arca)
                me->CastSpell(arca, SPELL_CONFLAGRATION_BLAST, false);
            return 1000;
        case 11:
            if(arca)
                (arca->ToCreature())->Yell(SAY_DIALOG_ARCANAGOS_8, LANG_UNIVERSAL, nullptr);
            return 5000;
        case 12:
            arca->GetMotionMaster()->MovePoint(0, -11010.82,-1761.18, 156.47);
            arca->SetKeepActive(true);
            arca->InterruptNonMeleeSpells(true);
            arca->SetSpeedRate(MOVE_FLIGHT, 2.0f);
            return 10000;
        case 13:
            me->Yell(SAY_DIALOG_MEDIVH_9, LANG_UNIVERSAL, nullptr);
            return 10000;
        case 14:
            me->SetVisibility(VISIBILITY_OFF);
            me->ClearInCombat();

            if(map->IsDungeon())
            {
                InstanceMap::PlayerList const &PlayerList = ((InstanceMap*)map)->GetPlayers();
                for (const auto & i : PlayerList)
                {
                    if(i.GetSource()->IsAlive())
                    {
                        if(i.GetSource()->GetQuestStatus(9645) == QUEST_STATUS_INCOMPLETE)
                            i.GetSource()->CompleteQuest(9645);
                    }
                }
            }
            return 50000;
        case 15:
            arca->DealDamage(arca,arca->GetHealth(),nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
            return 5000;
        default : 
            return 9999999;
        }

    }

    void UpdateAI(const uint32 diff)
    override {

        if(YellTimer < diff)
        {
            if(EventStarted)
            {
                YellTimer = NextStep(Step++);
            }
        }else YellTimer -= diff;

        if(Step >= 7 && Step <= 12 )
        {
            Unit* arca = ObjectAccessor::GetUnit((*me),ArcanagosGUID);

            if(FireArcanagosTimer < diff)
            {
                if(arca)
                    arca->CastSpell(me, SPELL_FIRE_BALL, false);
                FireArcanagosTimer = 6000;
            }else FireArcanagosTimer -= diff;

            if(FireMedivhTimer < diff)
            {
                if(arca)
                    DoCast(arca, SPELL_FIRE_BALL);
                FireMedivhTimer = 5000;
            }else FireMedivhTimer -= diff;

        }
    }
};

CreatureAI* GetAI_npc_image_of_medivh(Creature *_Creature)
{
    return new npc_image_of_medivhAI(_Creature);
}

/*######
## npc_archmage_leryda
######*/

bool GossipHello_npc_archmage_leryda(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->IsQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());
        
    if (!pPlayer->HasItemCount(29290, 1, true) && !pPlayer->HasItemCount(29279, 1, true) && !pPlayer->HasItemCount(29283, 1, true) &&
         !pPlayer->HasItemCount(29287, 1, true) && pPlayer->GetReputationRank(967) == REP_EXALTED) {
        if (pPlayer->GetQuestStatus(11031) == QUEST_STATUS_COMPLETE || pPlayer->GetQuestStatus(11032) == QUEST_STATUS_COMPLETE
                || pPlayer->GetQuestStatus(11033) == QUEST_STATUS_COMPLETE || pPlayer->GetQuestStatus(11034) == QUEST_STATUS_COMPLETE) {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Je voudrais une chevalière de grand guérisseur.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Je voudrais une chevalière de maître assassin.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Je voudrais une chevalière de l'archimage.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Je voudrais une chevalière du grand protecteur.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
        }
    }
    
    SEND_PREPARED_GOSSIP_MENU(pPlayer, pCreature);
    
    return true;
}

bool GossipSelect_npc_archmage_leryda(Player* pPlayer, Creature* pCreature, uint32 sender, uint32 action)
{
    uint32 itemId = 0;
    switch (action) {
    case GOSSIP_ACTION_INFO_DEF+1:
        itemId = 29290;
        break;
    case GOSSIP_ACTION_INFO_DEF+2:
        itemId = 29283;
        break;
    case GOSSIP_ACTION_INFO_DEF+3:
        itemId = 29287;
        break;
    case GOSSIP_ACTION_INFO_DEF+4:
        itemId = 29279;
        break;
    }
    
    ItemPosCountVec dest;
    uint8 msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
    if (msg == EQUIP_ERR_OK)
    {
        Item* item = pPlayer->StoreNewItem( dest, itemId, true);
        pPlayer->SendNewItem(item, 1, true, false);
    }
    
    pPlayer->CLOSE_GOSSIP_MENU();
    
    return true;
}

class SealedTome : public GameObjectScript
{
public:
    SealedTome() : GameObjectScript("go_sealed_tome")
    {}

    bool OnGossipHello(Player* player, GameObject* _GO) override
    {
        if (rand() % 3 == 1)
            player->CastSpell(player, RAND(30762, 30763, 30764, 30765, 30766), true);

        return false;
    }
};

/*######
## woefulhealer
######*/

#define TIMER_GLOBALWAIT             2000 + rand() % 3000
#define MAXMANA                      10000
#define SPELL_FLASHHEAL              25235
#define VALUE_FLASHHEAL              1200   //estimation du heal
#define SPELL_PRAYEROFHEALING        25308
#define VALUE_PRAYEROFHEALING        1300

 
struct woefulhealerAI : public ScriptedAI
{
    woefulhealerAI(Creature *c) : ScriptedAI(c) {}
    
    bool flagsset;
    Unit* owner;
    Unit* healtarget;
    uint16 waittimer;
    uint8 tohealingroup;
    uint32 mostlowhp;
    
    void Reset()
    override {
        owner = me->GetCharmerOrOwner();
        me->GetMotionMaster()->MoveFollow(owner, 0.8,210);
        me->SetReactState(REACT_PASSIVE);
        waittimer = TIMER_GLOBALWAIT;
        flagsset = false;
    }
    
    void EnterCombat(Unit* who) override {}
  
    void UpdateAI(const uint32 diff)
    override {                
        if (!flagsset) 
        {        
            me->SetMaxPower(POWER_MANA, MAXMANA);
            me->SetPower(POWER_MANA, MAXMANA);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            flagsset = true;
        } //Fonctionnent pas dans le reset, une autre maniere de faire plus propre?
            
        if ( !me->IsAlive() || me->IsNonMeleeSpellCast(false))
            return;
    
        if (waittimer > diff)
        {
            waittimer -= diff;
            return;    
        }

        healtarget = nullptr;
        tohealingroup = 0;
        mostlowhp = -1;
        
        //Selection de la/les cibles du heal
        Map *map = me->GetMap();
        Map::PlayerList const& PlayerList = map->GetPlayers();
        
        for(const auto & i : PlayerList)
        {
            if (Player* i_pl = i.GetSource())
            {
                if (i_pl->IsAlive() && i_pl->GetDistance(me) < 40 
                && (i_pl->GetMaxHealth() - i_pl->GetHealth() > VALUE_PRAYEROFHEALING))
                {
                    if (mostlowhp > i_pl->GetHealth())
                    {
                        healtarget = i_pl;
                        mostlowhp = i_pl->GetHealth();
                    }
                    if (((Player*) i_pl)->IsInSameGroupWith((Player*) owner))
                        tohealingroup++;
                }
            }
        }
            
        if (!healtarget)
            return;
            
        // Cast
        if (tohealingroup >= 3)
            DoCast(me, SPELL_PRAYEROFHEALING);
        else
            DoCast(healtarget, SPELL_FLASHHEAL);        
            
        waittimer = TIMER_GLOBALWAIT;
    }
};
 
CreatureAI* GetAI_woefulhealer(Creature *_Creature)
{
    return new woefulhealerAI (_Creature);
}

#define ANOMALY_SPELL_MANA_SHIELD 29880
#define ANOMALY_SPELL_BLINK 29883
#define ANOMALY_SPELL_ARCANE_VOLLEY 29885
#define ANOMALY_SPELL_LOOSE_MANA 29882

struct npc_arcane_anomalyAI : public ScriptedAI
{
    npc_arcane_anomalyAI(Creature *c) : ScriptedAI(c) {}
    
    bool castedShield;
    uint32 blinkTimer;
    uint32 volleyTimer;

    void Reset()
    override {
        blinkTimer = 5000 + rand()%10000;
        volleyTimer = 10000 + rand()%5000;
        castedShield = false;
    }

    void EnterCombat(Unit* who) 
    override {
        DoCast(me,ANOMALY_SPELL_MANA_SHIELD,true);
        castedShield = true;
    }

    //cannot die if we havent casted our mana shield
    void DamageTaken(Unit* pKiller, uint32 &damage)
    override {
        if(!castedShield && damage >= me->GetHealth())
            damage = me->GetHealth()-1; //down to 1 hp
    }
  
    void UpdateAI(const uint32 diff)
    override {                
        if(!UpdateVictim())
            return;

        if(blinkTimer < diff)
        {
            if(DoCast(me,ANOMALY_SPELL_BLINK) == SPELL_CAST_OK)
                blinkTimer = 10000 + rand()%5000;
        } else blinkTimer -= diff;

        if(volleyTimer < diff)
        {
            if(DoCast(me,ANOMALY_SPELL_ARCANE_VOLLEY) == SPELL_CAST_OK)
                volleyTimer = 20000 + rand()%5000;
        } else volleyTimer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_arcane_anomaly(Creature *_Creature)
{
    return new npc_arcane_anomalyAI (_Creature);
}

void AddSC_karazhan()
{
    OLDScript* newscript;

    newscript = new OLDScript;
    newscript->GetAI = &GetAI_npc_barnesAI;
    newscript->Name = "npc_barnes";
    newscript->OnGossipHello = &GossipHello_npc_barnes;
    newscript->OnGossipSelect = &GossipSelect_npc_barnes;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name = "npc_image_of_medivh";
    newscript->GetAI = &GetAI_npc_image_of_medivh;
    sScriptMgr->RegisterOLDScript(newscript);
    
    newscript = new OLDScript;
    newscript->Name = "npc_archmage_leryda";
    newscript->OnGossipHello = &GossipHello_npc_archmage_leryda;
    newscript->OnGossipSelect = &GossipSelect_npc_archmage_leryda;
    sScriptMgr->RegisterOLDScript(newscript);
    
    new SealedTome();
    
    newscript = new OLDScript;
    newscript->Name="woefulhealer";
    newscript->GetAI = &GetAI_woefulhealer;
    sScriptMgr->RegisterOLDScript(newscript);
    
    newscript = new OLDScript;
    newscript->Name="npc_arcane_anomaly";
    newscript->GetAI = &GetAI_npc_arcane_anomaly;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name = "npc_hastings";
    newscript->OnGossipHello = &GossipHello_npc_hastings;
    newscript->OnGossipSelect = &GossipSelect_npc_hastings;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name = "npc_calliard";
    newscript->OnGossipHello = &GossipHello_npc_calliard;
    newscript->OnGossipSelect = &GossipSelect_npc_calliard;
    sScriptMgr->RegisterOLDScript(newscript);
}

