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
SDName: instance_stratholme
SD%Complete: 50
SDComment: In progress. Undead side 75% implemented. Save/load not implemented.
SDCategory: Stratholme
EndScriptData */


#include "def_stratholme.h"

enum eStratholme {
    GO_SERVICE_ENTRANCE     = 175368,
    GO_GAUNTLET_GATE1       = 175357,
    GO_ZIGGURAT1            = 175380,                      //baroness
    GO_ZIGGURAT2            = 175379,                      //nerub'enkan
    GO_ZIGGURAT3            = 175381,                      //maleki
    GO_ZIGGURAT4            = 175405,                      //rammstein
    GO_ZIGGURAT5            = 175796,                      //baron
    GO_PORT_GAUNTLET        = 175374,                      //port from gauntlet to slaugther
    GO_PORT_SLAUGTHER       = 175373,                      //port at slaugther
    GO_PORT_ELDERS          = 175377,                      //port at elders square
    GO_CANNONBALL_STACK     = 176215,

    C_CRYSTAL               = 10415,                       //three ziggurat crystals
    C_BARON                 = 10440,
    C_YSIDA_TRIGGER         = 16100,

    C_RAMSTEIN              = 10439,
    C_ABOM_BILE             = 10416,
    C_ABOM_VENOM            = 10417,
    C_BLACK_GUARD           = 10394,
    C_YSIDA                 = 16031
};

#define ENCOUNTERS              6

class instance_stratholme : public InstanceMapScript
{
public:
    instance_stratholme() : InstanceMapScript("instance_stratholme", 329) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_stratholme_script(map);
    }

    struct instance_stratholme_script : public InstanceScript
    {
        instance_stratholme_script(Map *map) : InstanceScript(map)
        {
            Initialize();
        };

        uint32 Encounter[ENCOUNTERS];

        bool IsSilverHandDead[5];
        bool IsTimmySpawned;
        bool HasBarthilasEscaped;

        uint8 currentCannonStack;

        uint32 BaronRun_Timer;
        uint32 SlaugtherSquare_Timer;
        uint32 TimmySpawn_Timer;
        uint32 abominationTimer;

        uint64 serviceEntranceGUID;
        uint64 gauntletGate1GUID;
        uint64 ziggurat1GUID;
        uint64 ziggurat2GUID;
        uint64 ziggurat3GUID;
        uint64 ziggurat4GUID;
        uint64 ziggurat5GUID;
        uint64 portGauntletGUID;
        uint64 portSlaugtherGUID;
        uint64 portElderGUID;

        uint64 cannonballStacksGUIDs[5];

        uint64 baronGUID;
        uint64 ysidaTriggerGUID;
        //std::set<uint64> crystalsGUID;
        std::list<uint64> abomnationGUID;

        void Initialize() override
        {
            for (uint32 & i : Encounter)
                i = NOT_STARTED;

            for (bool & i : IsSilverHandDead)
                i = false;

            IsTimmySpawned = false;
            HasBarthilasEscaped = false;

            currentCannonStack = 0;

            BaronRun_Timer = 0;
            SlaugtherSquare_Timer = 0;
            TimmySpawn_Timer = 0;
            abominationTimer = 0;

            serviceEntranceGUID = 0;
            gauntletGate1GUID = 0;
            ziggurat1GUID = 0;
            ziggurat2GUID = 0;
            ziggurat3GUID = 0;
            ziggurat4GUID = 0;
            ziggurat5GUID = 0;
            portGauntletGUID = 0;
            portSlaugtherGUID = 0;
            portElderGUID = 0;

            baronGUID = 0;
            ysidaTriggerGUID = 0;
            //crystalsGUID.clear();
            abomnationGUID.clear();
        }

        bool StartSlaugtherSquare()
        {
            //change to DONE when crystals implemented
            if (Encounter[1] == IN_PROGRESS && Encounter[2] == IN_PROGRESS && Encounter[3] == IN_PROGRESS)
            {
                UpdateGoState(portGauntletGUID, 0, false);
                UpdateGoState(portSlaugtherGUID, 0, false);
                return true;
            }

            return false;
        }

        //if withRestoreTime true, then newState will be ignored and GO should be restored to original state after 10 seconds
        void UpdateGoState(uint64 goGuid, uint32 newState, bool withRestoreTime)
        {
            Player *player = GetPlayer();

            if (!player || !goGuid)
                return;

            if (GameObject *go = GameObject::GetGameObject(*player, goGuid))
            {
                if (withRestoreTime)
                    go->UseDoorOrButton(10);
                else
                    go->SetGoState(GOState(newState));
            }
        }

        void OnCreatureCreate(Creature *creature, uint32 creature_entry) override
        {
            switch (creature->GetEntry())
            {
            case C_BARON:           baronGUID = creature->GetGUID(); break;
            case C_YSIDA_TRIGGER:   ysidaTriggerGUID = creature->GetGUID(); break;
                //case C_CRYSTAL:         crystalsGUID.insert(creature->GetGUID()); break;
            case C_ABOM_BILE:
            case C_ABOM_VENOM:      abomnationGUID.push_back(creature->GetGUID()); break;
            }
        }

        void OnGameObjectCreate(GameObject *go) override
        {
            switch (go->GetEntry())
            {
            case GO_SERVICE_ENTRANCE:   serviceEntranceGUID = go->GetGUID(); break;
            case GO_GAUNTLET_GATE1:
                //weird, but unless flag is set, client will not respond as expected. DB bug?
                go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED);
                gauntletGate1GUID = go->GetGUID();
                break;
            case GO_ZIGGURAT1:          ziggurat1GUID = go->GetGUID(); break;
            case GO_ZIGGURAT2:          ziggurat2GUID = go->GetGUID(); break;
            case GO_ZIGGURAT3:          ziggurat3GUID = go->GetGUID(); break;
            case GO_ZIGGURAT4:          ziggurat4GUID = go->GetGUID(); break;
            case GO_ZIGGURAT5:          ziggurat5GUID = go->GetGUID(); break;
            case GO_PORT_GAUNTLET:      portGauntletGUID = go->GetGUID(); break;
            case GO_PORT_SLAUGTHER:     portSlaugtherGUID = go->GetGUID(); break;
            case GO_PORT_ELDERS:        portElderGUID = go->GetGUID(); break;
            case GO_CANNONBALL_STACK:
                if (currentCannonStack >= 5)
                    TC_LOG_ERROR("scripts", "STRATHOLME: currentCannonStack overflow: %d", currentCannonStack);
                cannonballStacksGUIDs[currentCannonStack] = go->GetGUID();
                currentCannonStack++;
                break;
            }
        }

        void ResetCannonballStacks() {
            Player *plr = GetPlayer();
            if (!plr)
                return;

            for (uint64 cannonballStacksGUID : cannonballStacksGUIDs) {
                if (GameObject *currentStack = GameObject::GetGameObject(*plr, cannonballStacksGUID))
                    currentStack->SwitchDoorOrButton(true);
            }
        }

        void ActivateCannonballStacks() {
            Player *plr = GetPlayer();
            if (!plr)
                return;

            for (uint64 cannonballStacksGUID : cannonballStacksGUIDs) {
                if (GameObject *currentStack = GameObject::GetGameObject(*plr, cannonballStacksGUID))
                    currentStack->SwitchDoorOrButton(false);
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            Player *player = GetPlayer();

            if (!player)
                return;

            switch (type)
            {
            case TYPE_ESCAPE_BARTH:
                HasBarthilasEscaped = true;
                break;
            case TYPE_BARON_RUN:
                switch (data)
                {
                case IN_PROGRESS:
                    if (Encounter[0] == IN_PROGRESS || Encounter[0] == FAIL)
                        break;
                    BaronRun_Timer = 2700000;
                    break;
                case FAIL:
                    //may add code to remove aura from players, but in theory the time should be up already and removed.
                    break;
                case DONE:
                    if (Unit *t = ObjectAccessor::GetUnit(*player, ysidaTriggerGUID))
                        t->SummonCreature(C_YSIDA, t->GetPositionX(), t->GetPositionY(), t->GetPositionZ(), t->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 1800000);
                    BaronRun_Timer = 0;
                    break;
                }
                Encounter[0] = data;
                break;
            case TYPE_BARONESS:
                Encounter[1] = data;
                if (data == IN_PROGRESS)
                    UpdateGoState(ziggurat1GUID, 0, false);
                if (data == IN_PROGRESS)                    //change to DONE when crystals implemented
                    StartSlaugtherSquare();
                break;
            case TYPE_NERUB:
                Encounter[2] = data;
                if (data == IN_PROGRESS)
                    UpdateGoState(ziggurat2GUID, 0, false);
                if (data == IN_PROGRESS)                    //change to DONE when crystals implemented
                    StartSlaugtherSquare();
                break;
            case TYPE_PALLID:
                Encounter[3] = data;
                if (data == IN_PROGRESS)
                    UpdateGoState(ziggurat3GUID, 0, false);
                if (data == IN_PROGRESS)                    //change to DONE when crystals implemented
                    StartSlaugtherSquare();
                break;
            case TYPE_RAMSTEIN:
                if (data == IN_PROGRESS)
                {
                    if (Encounter[4] != IN_PROGRESS)
                        UpdateGoState(portGauntletGUID, 1, false);

                    uint32 count = abomnationGUID.size();
                    for (uint64 & i : abomnationGUID)
                    {
                        if (Unit* abom = ObjectAccessor::GetUnit(*player, i))
                        {
                            if (!abom->IsAlive())
                                --count;
                            else
                                abominationTimer = 5000;
                        }
                    }

                    if (!count)
                    {
                        //a bit itchy, it should close the door after 10 secs, but it doesn't. skipping it for now.
                        //UpdateGoState(ziggurat4GUID,0,true);
                        player->SummonCreature(C_RAMSTEIN, 4032.84, -3390.24, 119.73, 4.71, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 1800000);
                    }
                }
                if (data == DONE)
                    SlaugtherSquare_Timer = 30000;

                Encounter[4] = data;
                break;
            case TYPE_BARON:
                if (data == IN_PROGRESS)
                {
                    if (GetData(TYPE_BARON_RUN) == IN_PROGRESS)
                    {
                        if (Group *pGroup = player->GetGroup())
                        {
                            for (GroupReference *itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
                            {
                                Player* pGroupie = itr->GetSource();
                                if (!pGroupie)
                                    continue;

                                if (pGroupie->HasAuraEffect(SPELL_BARON_ULTIMATUM, 0))
                                    pGroupie->RemoveAurasDueToSpell(SPELL_BARON_ULTIMATUM);

                                pGroupie->KilledMonsterCredit(16031, 0);
                                pGroupie->AreaExploredOrEventHappens(8945);
                            }
                        }
                        else if (player->HasAuraEffect(SPELL_BARON_ULTIMATUM, 0))
                            player->RemoveAurasDueToSpell(SPELL_BARON_ULTIMATUM);

                        if (Unit *temp = ObjectAccessor::GetUnit(*player, GetData64(DATA_BARON))) {
                            player->KilledMonsterCredit(16031, 0);
                            player->AreaExploredOrEventHappens(8945);
                        }

                        SetData(TYPE_BARON_RUN, DONE);
                    }
                }
                Encounter[5] = data;
                break;
            case TYPE_TIMMY_SPAWN:
                if (!IsTimmySpawned) {
                    TimmySpawn_Timer = 3000;
                    IsTimmySpawned = true;
                }
                break;
            case TYPE_CANNONMASTER:
                if (data == FAIL)
                    ResetCannonballStacks();
                else if (data == IN_PROGRESS)
                    ActivateCannonballStacks();
                break;
            case TYPE_SH_AELMAR:
                IsSilverHandDead[0] = (data) ? true : false;
                break;
            case TYPE_SH_CATHELA:
                IsSilverHandDead[1] = (data) ? true : false;
                break;
            case TYPE_SH_GREGOR:
                IsSilverHandDead[2] = (data) ? true : false;
                break;
            case TYPE_SH_NEMAS:
                IsSilverHandDead[3] = (data) ? true : false;
                break;
            case TYPE_SH_VICAR:
                IsSilverHandDead[4] = (data) ? true : false;
                break;
            }

            if (data == DONE)
                SaveToDB();
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
            case TYPE_ESCAPE_BARTH: return HasBarthilasEscaped ? 1 : 0;
            case TYPE_TIMMY_SPAWN:  return IsTimmySpawned ? 1 : 0;
            case TYPE_SH_QUEST:
                if (IsSilverHandDead[0] && IsSilverHandDead[1] && IsSilverHandDead[2] && IsSilverHandDead[3] && IsSilverHandDead[4])
                    return 1;
                return 0;
            case TYPE_BARON_RUN:
                return Encounter[0];
            case TYPE_BARONESS:
                return Encounter[1];
            case TYPE_NERUB:
                return Encounter[2];
            case TYPE_PALLID:
                return Encounter[3];
            case TYPE_RAMSTEIN:
                return Encounter[4];
            case TYPE_BARON:
                return Encounter[5];
            }

            return 0;
        }

        uint64 GetData64(uint32 data) const override
        {
            switch (data)
            {
            case DATA_BARON:
                return baronGUID;
            case DATA_YSIDA_TRIGGER:
                return ysidaTriggerGUID;
            }
            return 0;
        }

        std::string GetSaveData() override
        {
            OUT_SAVE_INST_DATA;
            std::ostringstream stream;
            stream << Encounter[0] << " " << Encounter[1] << " " << Encounter[2] << " " << Encounter[3] << " "
                << Encounter[4] << " " << Encounter[5];

            OUT_SAVE_INST_DATA_COMPLETE;
            return stream.str();
        }

        void Load(const char* in) override
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);
            std::istringstream stream(in);
            stream >> Encounter[0] >> Encounter[1] >> Encounter[2] >> Encounter[3]
                >> Encounter[4] >> Encounter[5];
            OUT_LOAD_INST_DATA_COMPLETE;
        }

        void Update(uint32 diff) override
        {
            if (BaronRun_Timer)
            {
                if (BaronRun_Timer <= diff)
                {
                    if (GetData(TYPE_BARON_RUN) != DONE)
                        SetData(TYPE_BARON_RUN, FAIL);
                    BaronRun_Timer = 0;
                }
                else BaronRun_Timer -= diff;
            }

            if (SlaugtherSquare_Timer)
            {
                if (SlaugtherSquare_Timer <= diff)
                {
                    if (Player *p = GetPlayer())
                    {
                        for (uint8 i = 0; i < 4; i++)
                            p->SummonCreature(C_BLACK_GUARD, 4032.84, -3390.24, 119.73, 4.71, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 1800000);

                        UpdateGoState(ziggurat4GUID, 0, false);
                        UpdateGoState(ziggurat5GUID, 0, false);
                    }
                    SlaugtherSquare_Timer = 0;
                }
                else SlaugtherSquare_Timer -= diff;
            }

            if (TimmySpawn_Timer) {
                if (TimmySpawn_Timer <= diff) {
                    if (Player *player = GetPlayer())
                        player->SummonCreature(10808, 3673.60, -3176.80, 126.31, 2.173, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 600000);
                    TimmySpawn_Timer = 0;
                }
                else TimmySpawn_Timer -= diff;
            }

            if (abominationTimer) {
                if (abominationTimer <= diff) {
                    if (Player *player = GetPlayer()) {
                        for (uint64 & i : abomnationGUID)
                        {
                            if (Unit* abom = ObjectAccessor::GetUnit(*player, i))
                            {
                                if (abom->IsAlive()) {
                                    reinterpret_cast<Creature*>(abom)->AI()->AttackStart(player);
                                    abominationTimer = 0;
                                    break;
                                }
                            }
                        }
                    }
                }
                else abominationTimer -= diff;
            }
        }
    };
};

void AddSC_instance_stratholme()
{
    new instance_stratholme();
}

