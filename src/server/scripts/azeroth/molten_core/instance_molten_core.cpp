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
SDName: Instance_Molten_Core
SD%Complete: 0
SDComment: Place Holder
SDCategory: Molten Core
EndScriptData */


#include "def_molten_core.h"

#define ENCOUNTERS      10

#define ID_LUCIFRON     12118
#define ID_MAGMADAR     11982
#define ID_GEHENNAS     12259
#define ID_GARR         12057
#define ID_GEDDON       12056
#define ID_SHAZZRAH     12264
#define ID_GOLEMAGG     11988
#define ID_SULFURON     12098
#define ID_DOMO         12018
#define ID_RAGNAROS     11502
#define ID_FLAMEWAKERPRIEST     11662

class instance_molten_core : public InstanceMapScript
{
public:
    instance_molten_core() : InstanceMapScript("instance_molten_core", 409) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_molten_core_script(map);
    }

    struct instance_molten_core_script : public InstanceScript
    {
        instance_molten_core_script(Map *map) : InstanceScript(map) {}

        uint64 Lucifron, Magmadar, Gehennas, Garr, Geddon, Shazzrah, Sulfuron, Golemagg, Domo, Ragnaros, FlamewakerPriest;
        uint64 RuneKoro, RuneZeth, RuneMazj, RuneTheri, RuneBlaz, RuneKress, RuneMohn;

        uint32 Encounter[ENCOUNTERS];

        void Initialize()
            override {
            Lucifron = 0;
            Magmadar = 0;
            Gehennas = 0;
            Garr = 0;
            Geddon = 0;
            Shazzrah = 0;
            Sulfuron = 0;
            Golemagg = 0;
            Domo = 0;
            Ragnaros = 0;
            FlamewakerPriest = 0;

            RuneKoro = 0;
            RuneZeth = 0;
            RuneMazj = 0;
            RuneTheri = 0;
            RuneBlaz = 0;
            RuneKress = 0;
            RuneMohn = 0;

            for (uint32 & i : Encounter)
                i = NOT_STARTED;
        }

        bool IsEncounterInProgress() const override
        {
            for (uint32 i : Encounter)
                if (i == IN_PROGRESS)
                    return true;

            return false;
        };


        void OnGameObjectCreate(GameObject *go) override
        {
            switch (go->GetEntry())
            {
            case 176951:                                    //Sulfuron
                RuneKoro = go->GetGUID();
                break;
            case 176952:                                    //Geddon
                RuneZeth = go->GetGUID();
                break;
            case 176953:                                    //Shazzrah
                RuneMazj = go->GetGUID();
                break;
            case 176954:                                    //Golemagg
                RuneTheri = go->GetGUID();
                break;
            case 176955:                                    //Garr
                RuneBlaz = go->GetGUID();
                break;
            case 176956:                                    //Magmadar
                RuneKress = go->GetGUID();
                break;
            case 176957:                                    //Gehennas
                RuneMohn = go->GetGUID();
                break;
            }
        }


        void OnCreatureCreate(Creature* creature, uint32 creature_entry) override
        {
            switch (creature_entry)
            {
            case ID_LUCIFRON:
                Lucifron = creature->GetGUID();
                break;
            case ID_MAGMADAR:
                Magmadar = creature->GetGUID();
                break;
            case ID_GEHENNAS:
                Gehennas = creature->GetGUID();
                break;
            case ID_GARR:
                Garr = creature->GetGUID();
                break;
            case ID_GEDDON:
                Geddon = creature->GetGUID();
                break;
            case ID_SHAZZRAH:
                Shazzrah = creature->GetGUID();
                break;
            case ID_SULFURON:
                Sulfuron = creature->GetGUID();
                break;
            case ID_GOLEMAGG:
                Golemagg = creature->GetGUID();
                break;
            case ID_DOMO:
                Domo = creature->GetGUID();
                break;
            case ID_RAGNAROS:
                Ragnaros = creature->GetGUID();
                break;
            case ID_FLAMEWAKERPRIEST:
                FlamewakerPriest = creature->GetGUID();
                break;
            }
        }

        uint64 GetData64(uint32 identifier) const override
        {
            switch (identifier)
            {
            case DATA_LUCIFRON:
                return Lucifron;
            case DATA_MAGMADAR:
                return Magmadar;
            case DATA_GEHENNAS:
                return Gehennas;
            case DATA_GARR:
                return Garr;
            case DATA_SHAZZRAH:
                return Shazzrah;
            case DATA_GEDDON:
                return Geddon;
            case DATA_GOLEMAGG:
                return Golemagg;
            case DATA_SULFURON:
                return Sulfuron;
            case DATA_MAJORDOMO:
                return Domo;
            case DATA_RAGNAROS:
                return Ragnaros;
            }

            return 0;
        }

        uint32 GetData(uint32 type) const override
        {
            if (type >= DATA_LUCIFRON && type <= DATA_RAGNAROS)
                return Encounter[type];

            return 0;
        }

        void SetData(uint32 type, uint32 data) override
        {
            if (type >= DATA_LUCIFRON && type <= DATA_RAGNAROS)
                Encounter[type] = data;
        }

        std::string GetSaveData() override
        {
            OUT_SAVE_INST_DATA;
            std::ostringstream saveStream;

            saveStream << Encounter[0] << " " << Encounter[1] << " " // TODO: Add "MC" in front of the saved data to check integrity
                << Encounter[2] << " " << Encounter[3] << " " << Encounter[4]
                << " " << Encounter[5] << " " << Encounter[6] << " " << Encounter[7]
                << " " << Encounter[8] << " " << Encounter[9];

            OUT_SAVE_INST_DATA_COMPLETE;
            return saveStream.str();
        }

        void Load(const char* in) override
        {
            if (!in) {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);

            std::istringstream loadStream(in);
            loadStream >> Encounter[0] >> Encounter[1] >> Encounter[2]
                >> Encounter[3] >> Encounter[4] >> Encounter[5] >> Encounter[6]
                >> Encounter[7] >> Encounter[8] >> Encounter[9];

            for (uint32 & i : Encounter)
                if (i == IN_PROGRESS)
                    i = NOT_STARTED;

            OUT_LOAD_INST_DATA_COMPLETE;
        }
    };
};

void AddSC_instance_molten_core()
{
    new instance_molten_core();
}

