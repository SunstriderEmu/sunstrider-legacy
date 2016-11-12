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
SDName: Tirisfal_Glades
SD%Complete: 100
SDComment: Quest support: 590, 1819
SDCategory: Tirisfal Glades
EndScriptData */

/* ContentData
npc_calvin_montague
go_mausoleum_door
go_mausoleum_trigger
EndContentData */



/*######
## npc_calvin_montague
######*/

#define QUEST_590           590
#define FACTION_FRIENDLY    68
#define FACTION_HOSTILE     16

struct npc_calvin_montagueAI : public ScriptedAI
{
    npc_calvin_montagueAI(Creature* c) : ScriptedAI(c) {}

    void Reset()
    override {
        me->SetFaction(FACTION_FRIENDLY);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
    }

    void EnterCombat(Unit* who) override { }

    void JustDied(Unit* Killer)
    override {
        if( Killer->GetTypeId() == TYPEID_PLAYER )
            if( (Killer->ToPlayer())->GetQuestStatus(QUEST_590) == QUEST_STATUS_INCOMPLETE )
                (Killer->ToPlayer())->AreaExploredOrEventHappens(QUEST_590);
    }

    void UpdateAI(const uint32 diff)
    override {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_calvin_montague(Creature *_Creature)
{
    return new npc_calvin_montagueAI (_Creature);
}

bool QuestAccept_npc_calvin_montague(Player* player, Creature* creature, Quest const* quest)
{
    if( quest->GetQuestId() == QUEST_590 )
    {
        creature->SetFaction(FACTION_HOSTILE);
        creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        ((npc_calvin_montagueAI*)creature->AI())->AttackStart(player);
    }
    return true;
}

/*######
## go_mausoleum_door
## go_mausoleum_trigger
######*/

#define QUEST_ULAG      1819
#define C_ULAG          6390
#define GO_TRIGGER      104593
#define GO_DOOR         176594

GameObject* SearchMausoleumGo(Unit *source, uint32 entry, float range)
{
    GameObject* pGo = nullptr;

    Trinity::NearestGameObjectEntryInObjectRangeCheck go_check(*source, entry, range);
    Trinity::GameObjectLastSearcher<Trinity::NearestGameObjectEntryInObjectRangeCheck> searcher(source, pGo, go_check);

    source->VisitNearbyGridObject(range, searcher);

    return pGo;
}

class MausoleumDoor : public GameObjectScript
{
public:
    MausoleumDoor() : GameObjectScript("go_mausoleum_door")
    {}

    bool OnGossipHello(Player* player, GameObject* _GO) override
    {
        if (player->GetQuestStatus(QUEST_ULAG) != QUEST_STATUS_INCOMPLETE)
            return false;

        if (GameObject *trigger = SearchMausoleumGo(player, GO_TRIGGER, 30))
        {
            trigger->SetGoState(GO_STATE_READY);
            player->SummonCreature(C_ULAG, 2390.26, 336.47, 40.01, 2.26, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 300000);
            return false;
        }

        return false;
    }
};

class MausoleumTrigger : public GameObjectScript
{
public:
    MausoleumTrigger() : GameObjectScript("go_mausoleum_trigger")
    {}

    bool OnGossipHello(Player* player, GameObject* _GO) override
    {
        if (player->GetQuestStatus(QUEST_ULAG) != QUEST_STATUS_INCOMPLETE)
            return false;

        if (GameObject *door = SearchMausoleumGo(player, GO_DOOR, 30))
        {
            _GO->SetGoState(GO_STATE_ACTIVE);
            door->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
            return true;
        }
        return true;
    }
};

void AddSC_tirisfal_glades()
{
    OLDScript *newscript;

    newscript = new OLDScript;
    newscript->Name="npc_calvin_montague";
    newscript->GetAI = &GetAI_npc_calvin_montague;
    newscript->OnQuestAccept = &QuestAccept_npc_calvin_montague;
    sScriptMgr->RegisterOLDScript(newscript);

    new MausoleumDoor();
    new MausoleumTrigger();
}

