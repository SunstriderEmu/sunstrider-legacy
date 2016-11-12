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
SDName: Undercity
SD%Complete: 95
SDComment: Quest support: 6628, 9180(post-event).
SDCategory: Undercity
EndScriptData */

/* ContentData
npc_lady_sylvanas_windrunner
npc_highborne_lamenter
npc_parqual_fintallas
EndContentData */


#include "GameEventMgr.h"

/*######
## npc_lady_sylvanas_windrunner
######*/

#define SAY_LAMENT_END              -1000357
#define EMOTE_LAMENT_END            -1000358

#define SOUND_CREDIT                10896
#define ENTRY_HIGHBORNE_LAMENTER    21628
#define ENTRY_HIGHBORNE_BUNNY       21641

#define SPELL_HIGHBORNE_AURA        37090
#define SPELL_SYLVANAS_CAST         36568
#define SPELL_RIBBON_OF_SOULS       34432                   //the real one to use might be 37099

float HighborneLoc[4][3]=
{
    {1285.41, 312.47, 0.51},
    {1286.96, 310.40, 1.00},
    {1289.66, 309.66, 1.52},
    {1292.51, 310.50, 1.99},
};

#define HIGHBORNE_LOC_Y             -61.00
#define HIGHBORNE_LOC_Y_NEW         -55.50

enum WickermanYells {
    YELL_WICKERMAN_1    = -1000716,
    YELL_WICKERMAN_2    = -1000717,
    YELL_WICKERMAN_3    = -1000718,
    YELL_WICKERMAN_4    = -1000719,
    YELL_WICKERMAN_5    = -1000720,
    YELL_WICKERMAN_6    = -1000721
};

struct npc_lady_sylvanas_windrunnerAI : public ScriptedAI
{
    npc_lady_sylvanas_windrunnerAI(Creature *c) : ScriptedAI(c) {}

    uint32 LamentEvent_Timer;
    bool LamentEvent, WickermanEvent;
    uint64 targetGUID;
    
    uint8 TalkPhase;
    uint32 TalkTimer;

    float myX;
    float myY;
    float myZ;

    void Reset()
    override {
        myX = me->GetPositionX();
        myY = me->GetPositionY();
        myZ = me->GetPositionZ();

        LamentEvent_Timer = 5000;
        LamentEvent = false;
        targetGUID = 0;
        TalkPhase = 0;
        
        if (sGameEventMgr->IsActiveEvent(GAME_EVENT_WICKERMAN_FESTIVAL)) {
            if (GameObject* wickerman = me->FindNearestGameObject(180433, 30.0f)) {
                if (wickerman->GetGoState() == 0) // Already burning
                    return;

                WickermanEvent = true;
                TalkTimer = 1000;
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            }
        }
    }

    void EnterCombat(Unit *who) override {}

    void JustSummoned(Creature *summoned)
    override {
        if( summoned->GetEntry() == ENTRY_HIGHBORNE_BUNNY )
        {
            if( Unit* target = ObjectAccessor::GetUnit(*summoned,targetGUID) )
            {
                target->MonsterMoveWithSpeed(target->GetPositionX(), target->GetPositionY(), myZ+15.0,0);
                target->Relocate(target->GetPositionX(), target->GetPositionY(), myZ+15.0);
                summoned->CastSpell(target, SPELL_RIBBON_OF_SOULS, false);
            }

            summoned->SetDisableGravity(true);
            targetGUID = summoned->GetGUID();
        }
    }
    
    void DespawnDueToGameEventEnd(int32 eventId)
    override {
        if (eventId != 50)
            return;
            
        if (GameObject* wickerman = me->FindNearestGameObject(180433, 30.0f))
            wickerman->SetGoState(GO_STATE_READY);
    }

    void UpdateAI(const uint32 diff)
    override {
        if( LamentEvent )
        {
            if( LamentEvent_Timer < diff )
            {
                float raX = myX;
                float raY = myY;
                float raZ = myZ;

                Position pos = {raX, raY, raZ, 0.0f};

                me->GetRandomPoint(pos, 20.0, raX, raY, raZ);
                me->SummonCreature(ENTRY_HIGHBORNE_BUNNY, raX, raY, myZ, 0, TEMPSUMMON_TIMED_DESPAWN, 3000);

                LamentEvent_Timer = 2000;
                if( !me->HasAuraEffect(SPELL_SYLVANAS_CAST, 0))
                {
                    DoScriptText(SAY_LAMENT_END, me);
                    DoScriptText(EMOTE_LAMENT_END, me);
                    LamentEvent = false;
                }
            }else LamentEvent_Timer -= diff;
        }
        else if (WickermanEvent) {
            if (TalkTimer) {
                if (TalkTimer <= diff) {
                    uint32 talkId = 0;
                    switch (TalkPhase) {
                    case 0:
                        talkId = YELL_WICKERMAN_1;
                        TalkTimer = 8000;
                        break;
                    case 1:
                        talkId = YELL_WICKERMAN_2;
                        TalkTimer = 8000;
                        break;
                    case 2:
                        talkId = YELL_WICKERMAN_3;
                        TalkTimer = 8000;
                        break;
                    case 3:
                        talkId = YELL_WICKERMAN_4;
                        TalkTimer = 8000;
                        break;
                    case 4:
                        talkId = YELL_WICKERMAN_5;
                        TalkTimer = 8000;
                        break;
                    case 5:
                        talkId = YELL_WICKERMAN_6;
                        TalkTimer = 10000;
                        break;
                    case 6:
                        if (GameObject* wickerman = me->FindNearestGameObject(180433, 30.0f))
                            wickerman->SetGoState(GO_STATE_ACTIVE);
                        talkId = 0;
                        TalkTimer = 0;
                    default:
                        break;
                    }
                    
                    if (talkId)
                        DoScriptText(talkId, me, nullptr);
                    TalkPhase++;
                }
                else
                    TalkTimer -= diff;
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_lady_sylvanas_windrunner(Creature *_Creature)
{
    return new npc_lady_sylvanas_windrunnerAI (_Creature);
}

bool ChooseReward_npc_lady_sylvanas_windrunner(Player *player, Creature *_Creature, const Quest *_Quest, uint32 slot)
{
    if( _Quest->GetQuestId() == 9180 )
    {
        ((npc_lady_sylvanas_windrunnerAI*)_Creature->AI())->LamentEvent = true;
        ((npc_lady_sylvanas_windrunnerAI*)_Creature->AI())->DoPlaySoundToSet(_Creature,SOUND_CREDIT);
        _Creature->CastSpell(_Creature,SPELL_SYLVANAS_CAST,false);

        for(auto & i : HighborneLoc)
            _Creature->SummonCreature(ENTRY_HIGHBORNE_LAMENTER, i[0], i[1], HIGHBORNE_LOC_Y, i[2], TEMPSUMMON_TIMED_DESPAWN, 160000);
    }

    return true;
}

/*######
## npc_highborne_lamenter
######*/

struct npc_highborne_lamenterAI : public ScriptedAI
{
    npc_highborne_lamenterAI(Creature *c) : ScriptedAI(c) {}

    uint32 EventMove_Timer;
    uint32 EventCast_Timer;
    bool EventMove;
    bool EventCast;

    void Reset()
    override {
        EventMove_Timer = 10000;
        EventCast_Timer = 17500;
        EventMove = true;
        EventCast = true;
    }

    void EnterCombat(Unit *who) override {}

    void UpdateAI(const uint32 diff)
    override {
        if( EventMove )
        {
            if( EventMove_Timer < diff )
            {
                me->SetDisableGravity(true);
                me->MonsterMoveWithSpeed(me->GetPositionX(),me->GetPositionY(),HIGHBORNE_LOC_Y_NEW,5000);
                me->GetMap()->CreatureRelocation(me,me->GetPositionX(),me->GetPositionY(),HIGHBORNE_LOC_Y_NEW,me->GetOrientation());
                EventMove = false;
            }else EventMove_Timer -= diff;
        }
        if( EventCast )
        {
            if( EventCast_Timer < diff )
            {
                DoCast(me,SPELL_HIGHBORNE_AURA);
                EventCast = false;
            }else EventCast_Timer -= diff;
        }
    }
};
CreatureAI* GetAI_npc_highborne_lamenter(Creature *_Creature)
{
    return new npc_highborne_lamenterAI (_Creature);
}

/*######
## npc_parqual_fintallas
######*/

#define SPELL_MARK_OF_SHAME 6767

#define GOSSIP_HPF1 "Gul'dan"
#define GOSSIP_HPF2 "Kel'Thuzad"
#define GOSSIP_HPF3 "Ner'zhul"

bool GossipHello_npc_parqual_fintallas(Player *player, Creature *_Creature)
{
    if (_Creature->IsQuestGiver())
        player->PrepareQuestMenu( _Creature->GetGUID() );

    if (player->GetQuestStatus(6628) == QUEST_STATUS_INCOMPLETE && !player->HasAuraEffect(SPELL_MARK_OF_SHAME,0) )
    {
        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, GOSSIP_HPF1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, GOSSIP_HPF2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, GOSSIP_HPF3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        player->SEND_GOSSIP_MENU_TEXTID(5822, _Creature->GetGUID());
    }
    else
        player->SEND_GOSSIP_MENU_TEXTID(5821, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_parqual_fintallas(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->CastSpell(player,SPELL_MARK_OF_SHAME,false);
    }
    if (action == GOSSIP_ACTION_INFO_DEF+2)
    {
        player->CLOSE_GOSSIP_MENU();
        player->AreaExploredOrEventHappens(6628);
    }
    return true;
}

/*######
## AddSC
######*/

void AddSC_undercity()
{
    OLDScript *newscript;

    newscript = new OLDScript;
    newscript->Name="npc_lady_sylvanas_windrunner";
    newscript->GetAI = &GetAI_npc_lady_sylvanas_windrunner;
    newscript->OnQuestReward = &ChooseReward_npc_lady_sylvanas_windrunner;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="npc_highborne_lamenter";
    newscript->GetAI = &GetAI_npc_highborne_lamenter;
    sScriptMgr->RegisterOLDScript(newscript);

    newscript = new OLDScript;
    newscript->Name="npc_parqual_fintallas";
    newscript->OnGossipHello = &GossipHello_npc_parqual_fintallas;
    newscript->OnGossipSelect = &GossipSelect_npc_parqual_fintallas;
    sScriptMgr->RegisterOLDScript(newscript);
}

