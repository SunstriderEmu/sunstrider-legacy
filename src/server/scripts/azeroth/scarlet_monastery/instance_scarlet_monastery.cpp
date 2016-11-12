/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* ScriptData
SDName: Instance_Scarlet_Monastery
SD%Complete: 50
SDComment:
SDCategory: Scarlet Monastery
EndScriptData */


#include "def_scarlet_monastery.h"

#define ENTRY_PUMPKIN_SHRINE    186267
#define ENTRY_HORSEMAN          23682
#define ENTRY_HEAD              23775
#define ENTRY_PUMPKIN           23694

#define ENCOUNTERS 1

class instance_scarlet_monastery : public InstanceMapScript
{
public:
    instance_scarlet_monastery() : InstanceMapScript("instance_scarlet_monastery", 189) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_scarlet_monastery_script(map);
    }

    struct instance_scarlet_monastery_script : public InstanceScript
    {
        instance_scarlet_monastery_script(Map *Map) : InstanceScript(Map) { Initialize(); };

        uint64 PumpkinShrineGUID;
        uint64 HorsemanGUID;
        uint64 HeadGUID;
        std::set<uint64> HorsemanAdds;

        uint64 MograineGUID;
        uint64 WhitemaneGUID;
        uint64 DoorHighInquisitorGUID;

        uint32 Encounter[ENCOUNTERS];

        void Initialize()
            override {
            PumpkinShrineGUID = 0;
            HorsemanGUID = 0;
            HeadGUID = 0;
            HorsemanAdds.clear();

            MograineGUID = 0;
            WhitemaneGUID = 0;
            DoorHighInquisitorGUID = 0;

            for (uint32 & i : Encounter)
                i = NOT_STARTED;
        }

        void OnGameObjectCreate(GameObject *go)
            override {
            switch (go->GetEntry())
            {
            case ENTRY_PUMPKIN_SHRINE: PumpkinShrineGUID = go->GetGUID(); break;
            case 104600: DoorHighInquisitorGUID = go->GetGUID(); break;
            }
        }

        void OnCreatureCreate(Creature *creature, uint32 creature_entry)
            override {
            switch (creature_entry)
            {
            case ENTRY_HORSEMAN:    HorsemanGUID = creature->GetGUID(); break;
            case ENTRY_HEAD:        HeadGUID = creature->GetGUID(); break;
            case ENTRY_PUMPKIN:     HorsemanAdds.insert(creature->GetGUID()); break;
            case 3976: MograineGUID = creature->GetGUID(); break;
            case 3977: WhitemaneGUID = creature->GetGUID(); break;
            }
        }

        void SetData(uint32 type, uint32 data)
            override {
            switch (type)
            {
            case TYPE_MOGRAINE_AND_WHITE_EVENT: Encounter[0] = data; break;
            case GAMEOBJECT_PUMPKIN_SHRINE:
            {
                GameObject *Shrine = instance->GetGameObject(PumpkinShrineGUID);
                if (Shrine)
                    Shrine->SetUInt32Value(GAMEOBJECT_STATE, 1);
            }break;
            case DATA_HORSEMAN_EVENT:
                if (data == DONE)
                {
                    for (uint64 HorsemanAdd : HorsemanAdds)
                    {
                        Creature* add = instance->GetCreature(HorsemanAdd);
                        if (add && add->IsAlive())
                            add->DealDamage(add, add->GetMaxHealth(), nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
                    }
                    HorsemanAdds.clear();
                    GameObject *Shrine = instance->GetGameObject(PumpkinShrineGUID);
                    if (Shrine)
                        Shrine->SetUInt32Value(GAMEOBJECT_STATE, 1);
                }
                break;
            }
        }

        uint64 GetData64(uint32 type) const override
        {
            switch (type)
            {
                //case GAMEOBJECT_PUMPKIN_SHRINE:   return PumpkinShrineGUID;
                //case DATA_HORSEMAN:               return HorsemanGUID;
                //case DATA_HEAD:                   return HeadGUID;
            case DATA_MOGRAINE:             return MograineGUID;
            case DATA_WHITEMANE:            return WhitemaneGUID;
            case DATA_DOOR_WHITEMANE:       return DoorHighInquisitorGUID;
            }
            return 0;
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == TYPE_MOGRAINE_AND_WHITE_EVENT)
                return Encounter[0];

            return 0;
        }
    };
};

void AddSC_instance_scarlet_monastery()
{
    new instance_scarlet_monastery();
}

