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
SDName: Instance_Shadowfang_Keep
SD%Complete: 100
SDComment: Complete, Doors checked
SDCategory: Shadowfang Keep
EndScriptData */


#include "def_shadowfang_keep.h"

#define ENCOUNTERS              4

class instance_shadowfang_keep : public InstanceMapScript
{
public:
    instance_shadowfang_keep() : InstanceMapScript("instance_shadowfang_keep", 33) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_shadowfang_keep_script(map);
    }

    struct instance_shadowfang_keep_script : public InstanceScript
    {
        instance_shadowfang_keep_script(Map *map) : InstanceScript(map) { Initialize(); };

        uint32 Encounters[ENCOUNTERS];
        std::string str_data;

        uint64 DoorCourtyardGUID;
        uint64 DoorSorcererGUID;
        uint64 DoorArugalGUID;

        void Initialize() override
        {
            DoorCourtyardGUID = 0;
            DoorSorcererGUID = 0;
            DoorArugalGUID = 0;

            for (uint32 & Encounter : Encounters)
                Encounter = NOT_STARTED;
        }

        void OnGameObjectCreate(GameObject *go) override
        {
            switch (go->GetEntry())
            {
            case 18895: DoorCourtyardGUID = go->GetGUID();
                if (Encounters[0] == DONE) HandleGameObject(0, true, go); break;
            case 18972: DoorSorcererGUID = go->GetGUID();
                if (Encounters[2] == DONE) HandleGameObject(0, true, go); break;
            case 18971: DoorArugalGUID = go->GetGUID();
                if (Encounters[3] == DONE) HandleGameObject(0, true, go); break;

            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
            case TYPE_FREE_NPC:
                if (data == DONE)
                    HandleGameObject(DoorCourtyardGUID, true);
                Encounters[0] = data;
                break;
            case TYPE_RETHILGORE:
                Encounters[1] = data;
                break;
            case TYPE_FENRUS:
                if (data == DONE)
                    HandleGameObject(DoorSorcererGUID, true);
                Encounters[2] = data;
                break;
            case TYPE_NANDOS:
                if (data == DONE)
                    HandleGameObject(DoorArugalGUID, true);
                Encounters[3] = data;
                break;
            }

            if (data == DONE)
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << Encounters[0] << " " << Encounters[1] << " " << Encounters[2] << " " << Encounters[3];

                str_data = saveStream.str();

                SaveToDB();
                OUT_SAVE_INST_DATA_COMPLETE;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
            case TYPE_FREE_NPC:
                return Encounters[0];
            case TYPE_RETHILGORE:
                return Encounters[1];
            case TYPE_FENRUS:
                return Encounters[2];
            case TYPE_NANDOS:
                return Encounters[3];
            }
            return 0;
        }

        std::string GetSaveData() override
        {
            OUT_SAVE_INST_DATA;
            return str_data;
            OUT_SAVE_INST_DATA_COMPLETE;
        }

        void Load(const char* in) override
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);

            std::istringstream loadStream(in);
            loadStream >> Encounters[0] >> Encounters[1] >> Encounters[2] >> Encounters[3];

            for (uint32 & Encounter : Encounters)
            {
                if (Encounter == IN_PROGRESS)
                    Encounter = NOT_STARTED;

            }

            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };
};

void AddSC_instance_shadowfang_keep()
{
    new instance_shadowfang_keep();
}

