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
SDName: Instance_Karazhan
SD%Complete: 70
SDComment: Instance OLDScript for Karazhan to help in various encounters. TODO: GameObject visibility for Opera event.
SDCategory: Karazhan
EndScriptData */


#include "def_karazhan.h"

#define ENCOUNTERS      12

/*
0  - Attumen + Midnight (optional)
1  - Moroes
2  - Maiden of Virtue (optional)
3  - Hyakiss the Lurker /  Rokad the Ravager  / Shadikith the Glider
4  - Opera Event
5  - Curator
6  - Shade of Aran (optional)
7  - Terestian Illhoof (optional)
8  - Netherspite (optional)
9  - Chess Event
10 - Prince Malchezzar
11 - Nightbane
*/

class instance_karazhan : public InstanceMapScript
{
public:
    instance_karazhan() : InstanceMapScript("instance_karazhan", 532) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_karazhan_script(map);
    }

    struct instance_karazhan_script : public InstanceScript
    {
        instance_karazhan_script(Map* map) : InstanceScript(map) { Initialize(); }

        uint32 Encounters[ENCOUNTERS];

        uint32 OperaEvent;
        uint32 OzDeathCount;
        uint32 ChessTeam;

        uint64 CurtainGUID;
        uint64 StageDoorLeftGUID;
        uint64 StageDoorRightGUID;
        uint64 KilrekGUID;
        uint64 TerestianGUID;
        uint64 MoroesGUID;
        uint64 MalchezaarGUID;
        uint64 LibraryDoor;                                     // Door at Shade of Aran
        uint64 MassiveDoor;                                     // Door at Netherspite
        uint64 GamesmansDoor;                                   // Door before Chess
        uint64 GamesmansExitDoor;                               // Door after Chess
        uint64 NetherspaceDoor;                                // Door at Malchezaar
        uint64 SideEntranceDoor;                                // Inside top entrance door
        uint64 MastersTerraceDoor[2];
        uint64 ImageGUID;

        uint32 ChessGamePhase;

        std::vector<uint64> ChessPieces;
        std::vector<uint64> MedivhCheatFires;

        void Initialize()
            override {
            for (uint32 & Encounter : Encounters)
                Encounter = NOT_STARTED;

            OperaEvent = urand(1, 3);                   // 1 - OZ, 2 - HOOD, 3 - RAJ, this never gets altered.
            OzDeathCount = 0;

            CurtainGUID = 0;
            StageDoorLeftGUID = 0;
            StageDoorRightGUID = 0;

            KilrekGUID = 0;
            TerestianGUID = 0;
            MoroesGUID = 0;
            MalchezaarGUID = 0;

            LibraryDoor = 0;
            MassiveDoor = 0;
            GamesmansDoor = 0;
            GamesmansExitDoor = 0;
            NetherspaceDoor = 0;
            SideEntranceDoor = 0;
            MastersTerraceDoor[0] = 0;
            MastersTerraceDoor[1] = 0;
            ImageGUID = 0;
            ChessTeam = 0;

            ChessPieces.resize(0);
            MedivhCheatFires.resize(0);
            ChessGamePhase = NOTSTARTED;
        }

        bool IsEncounterInProgress() const override
        {
            for (uint32 Encounter : Encounters)
                if (Encounter == IN_PROGRESS)
                    return true;

            return false;
        }

        uint32 GetData(uint32 identifier) const override
        {
            switch (identifier)
            {
            case DATA_ATTUMEN_EVENT:          return Encounters[0];
            case DATA_MOROES_EVENT:           return Encounters[1];
            case DATA_MAIDENOFVIRTUE_EVENT:   return Encounters[2];
            case DATA_OPTIONAL_BOSS_EVENT:    return Encounters[3];
            case DATA_OPERA_EVENT:            return Encounters[4];
            case DATA_CURATOR_EVENT:          return Encounters[5];
            case DATA_SHADEOFARAN_EVENT:      return Encounters[6];
            case DATA_TERESTIAN_EVENT:        return Encounters[7];
            case DATA_NETHERSPITE_EVENT:      return Encounters[8];
            case DATA_CHESS_EVENT:            return Encounters[9];
            case DATA_MALCHEZZAR_EVENT:       return Encounters[10];
            case DATA_NIGHTBANE_EVENT:        return Encounters[11];
            case DATA_OPERA_PERFORMANCE:      return OperaEvent;
            case DATA_OPERA_OZ_DEATHCOUNT:    return OzDeathCount;
            case CHESS_EVENT_TEAM:            return ChessTeam;
            case DATA_CHESS_GAME_PHASE:       return ChessGamePhase;
            }

            return 0;
        }

        void OnCreatureCreate(Creature *creature, uint32 entry) override
        {
            switch (creature->GetEntry())
            {
            case 17229:   KilrekGUID = creature->GetGUID();      break;
            case 15688:   TerestianGUID = creature->GetGUID();   break;
            case 15687:   MoroesGUID = creature->GetGUID();      break;
            case 16816:   ImageGUID = creature->GetGUID();       break;
            case 15690:   MalchezaarGUID = creature->GetGUID();  break;
            case NPC_PAWN_H:
            case NPC_KNIGHT_H:
            case NPC_QUEEN_H:
            case NPC_BISHOP_H:
            case NPC_ROOK_H:
            case NPC_KING_H:
            case NPC_PAWN_A:
            case NPC_KNIGHT_A:
            case NPC_QUEEN_A:
            case NPC_BISHOP_A:
            case NPC_ROOK_A:
            case NPC_KING_A:
                ChessPieces.push_back(creature->GetGUID());
                creature->SetHealth(creature->GetMaxHealth());
                break;
            case 22521:
                MedivhCheatFires.push_back(creature->GetGUID());
                break;
            }
        }

        void OnPlayerEnter(Player* player) override
        {
            if (GetData(DATA_CHESS_EVENT) != IN_PROGRESS)
                player->RemoveAurasDueToSpell(39331);
            player->RemoveAurasDueToSpell(30019);
        }

        uint64 GetData64(uint32 data) const override
        {
            switch (data)
            {
            case DATA_KILREK:                      return KilrekGUID;
            case DATA_TERESTIAN:                   return TerestianGUID;
            case DATA_MOROES:                      return MoroesGUID;
            case DATA_GAMEOBJECT_STAGEDOORLEFT:    return StageDoorLeftGUID;
            case DATA_GAMEOBJECT_STAGEDOORRIGHT:   return StageDoorRightGUID;
            case DATA_GAMEOBJECT_CURTAINS:         return CurtainGUID;
            case DATA_GAMEOBJECT_LIBRARY_DOOR:     return LibraryDoor;
            case DATA_GAMEOBJECT_MASSIVE_DOOR:     return MassiveDoor;
            case DATA_GAMEOBJECT_GAME_DOOR:        return GamesmansDoor;
            case DATA_GAMEOBJECT_GAME_EXIT_DOOR:   return GamesmansExitDoor;
            case DATA_GAMEOBJECT_NETHER_DOOR:      return NetherspaceDoor;
            case DATA_MASTERS_TERRACE_DOOR_1:      return MastersTerraceDoor[0];
            case DATA_MASTERS_TERRACE_DOOR_2:      return MastersTerraceDoor[1];
            case DATA_IMAGE_OF_MEDIVH:             return ImageGUID;
            case DATA_MALCHEZAAR:                  return MalchezaarGUID;
            }

            return 0;
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
            case DATA_ATTUMEN_EVENT:           Encounters[0] = data; break;
            case DATA_MOROES_EVENT:
                if (Encounters[1] == DONE)
                    break;
                Encounters[1] = data;
                break;
            case DATA_MAIDENOFVIRTUE_EVENT:    Encounters[2] = data; break;
            case DATA_OPTIONAL_BOSS_EVENT:     Encounters[3] = data; break;
            case DATA_OPERA_EVENT:
                Encounters[4] = data;
                if (data == DONE)
                {
                    GameObject* sideDoor = instance->GetGameObject(SideEntranceDoor);
                    if (sideDoor)
                        sideDoor->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);

                    HandleGameObject(SideEntranceDoor, true);
                    HandleGameObject(StageDoorLeftGUID, true);
                    HandleGameObject(StageDoorRightGUID, true);
                }
                break;
            case DATA_CURATOR_EVENT:           Encounters[5] = data; break;
            case DATA_SHADEOFARAN_EVENT:       Encounters[6] = data; break;
            case DATA_TERESTIAN_EVENT:         Encounters[7] = data; break;
            case DATA_NETHERSPITE_EVENT:       Encounters[8] = data; break;
            case DATA_CHESS_EVENT:
                Encounters[9] = data;
                if (data == FAIL || data == DONE || data == NOT_STARTED)
                    RemoveAuraOnAllPlayers(39331);
                /*else if (data == IN_PROGRESS || data == SPECIAL)
                    CastOnAllPlayers(39331);*/
                break;
            case DATA_MALCHEZZAR_EVENT:        Encounters[10] = data; break;
            case DATA_NIGHTBANE_EVENT:
                if (Encounters[11] == DONE)
                    break;
                Encounters[11] = data;
                break;
            case DATA_OPERA_OZ_DEATHCOUNT:     ++OzDeathCount;        break;
            case CHESS_EVENT_TEAM:             ChessTeam = data;      break;
            case DATA_CHESS_REINIT_PIECES:
                for (uint64 ChessPiece : ChessPieces) {
                    if (Creature* piece = instance->GetCreature(ChessPiece)) {
                        piece->CombatStop();
                        piece->SetDeathState(JUST_RESPAWNED);
                        piece->SetHealth(piece->GetMaxHealth());
                        piece->AI()->SetData(0, 0);
                        float x, y, z, o;
                        piece->GetHomePosition(x, y, z, o);
                        piece->Relocate(x, y, z, o);
                        piece->SendMovementFlagUpdate();
                        piece->AI()->SetData(1, 0); // Reset default orientation
                        piece->RemoveAurasDueToSpell(39339);
                        piece->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    }
                }
                for (uint64 MedivhCheatFire : MedivhCheatFires) {
                    if (Creature* fire = instance->GetCreature(MedivhCheatFire))
                        fire->DisappearAndDie();
                }
                MedivhCheatFires.resize(0);
                break;
            case DATA_CHESS_CHECK_PIECES_ALIVE:
                for (uint64 ChessPiece : ChessPieces) {
                    if (Creature* piece = instance->GetCreature(ChessPiece)) {
                        if (!piece->IsAlive()) {
                            piece->SetDeathState(JUST_RESPAWNED);
                            piece->SetHealth(piece->GetMaxHealth());
                        }
                    }
                }
                break;
            case DATA_CHESS_GAME_PHASE:
                ChessGamePhase = data;
                break;
            }

            if (data == DONE && type != DATA_CHESS_GAME_PHASE)
                SaveToDB();
        }

        void SetData64(uint32 identifier, uint64 data) override
        {
            switch (identifier)
            {
            case DATA_IMAGE_OF_MEDIVH: ImageGUID = data;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
            case 183932:   CurtainGUID = go->GetGUID();         break;
            case 184278:   StageDoorLeftGUID = go->GetGUID();
                if (DATA_OPERA_EVENT == DONE)
                    HandleGameObject(StageDoorLeftGUID, true);  break;
            case 184279:   StageDoorRightGUID = go->GetGUID();
                if (DATA_OPERA_EVENT == DONE)
                    HandleGameObject(StageDoorRightGUID, true); break;
            case 184517:   LibraryDoor = go->GetGUID();         break;
            case 185521:   MassiveDoor = go->GetGUID();         break;
            case 184276:   GamesmansDoor = go->GetGUID();         break;
            case 184277:   GamesmansExitDoor = go->GetGUID();         break;
            case 185134:   NetherspaceDoor = go->GetGUID();         break;
            case 184274:   MastersTerraceDoor[0] = go->GetGUID();         break;
            case 184280:   MastersTerraceDoor[1] = go->GetGUID();         break;
            case 184275:   SideEntranceDoor = go->GetGUID();
                if (DATA_OPERA_EVENT < DONE)
                {
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                    HandleGameObject(SideEntranceDoor, true);
                }
                break;
            }

            switch (OperaEvent)
            {
                //TODO: Set Object visibilities for Opera based on performance
            case EVENT_OZ:
                break;

            case EVENT_HOOD:
                break;

            case EVENT_RAJ:
                break;
            }
        }

        std::string GetSaveData() override
        {
            OUT_SAVE_INST_DATA;
            std::ostringstream stream;
            stream << Encounters[0] << " " << Encounters[1] << " " << Encounters[2] << " " << Encounters[3] << " "
                << Encounters[4] << " " << Encounters[5] << " " << Encounters[6] << " " << Encounters[7] << " "
                << Encounters[8] << " " << Encounters[9] << " " << Encounters[10] << " " << Encounters[11];

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
            stream >> Encounters[0] >> Encounters[1] >> Encounters[2] >> Encounters[3]
                >> Encounters[4] >> Encounters[5] >> Encounters[6] >> Encounters[7]
                >> Encounters[8] >> Encounters[9] >> Encounters[10] >> Encounters[11];
            for (uint32 & Encounter : Encounters)
                if (Encounter == IN_PROGRESS)                // Do not load an encounter as "In Progress" - reset it instead.
                    Encounter = NOT_STARTED;
            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };
};

void AddSC_instance_karazhan()
{
    new instance_karazhan();
}

