/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
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

#ifndef TRINITY_INSTANCE_DATA_H
#define TRINITY_INSTANCE_DATA_H

class Map;
class Unit;
class Player;
class GameObject;
class Creature;


class ZoneScript
{
    public:

        explicit ZoneScript(Map *map) : instance(map) {}
        virtual ~ZoneScript() {}


        virtual uint32 GetCreatureEntry(uint32 /*guidlow*/, CreatureData const* data) { return data->id; }
        virtual uint32 GetGameObjectEntry(uint32 /*guidlow*/, uint32 entry) { return entry; }

        Map *instance;

        //On creation, NOT load.
        virtual void Initialize() {}

        //On load
        virtual void Load(const char* /*data*/) {}

        //When save is needed, this function generates the data
        virtual const char* Save() { return ""; }

        void SaveToDB();

        //Called every map update
        virtual void Update(uint32 /*diff*/) {}

        //Called when a player successfully enters the instance.
        virtual void OnPlayerEnter(Player *) {}

        //Called when a gameobject is created
        virtual void OnGameObjectCreate(GameObject *) {}

        //called on creature creation
        virtual void OnCreatureCreate(Creature * /*creature*/, uint32 /*creature_entry*/) {}
        
        // Called on creature respawn
        virtual void OnCreatureRespawn(Creature* /*creature*/, uint32 /*creature_entry*/) {}
        
        virtual void OnCreatureDeath(Creature* /*creature*/) {}

        virtual void OnCreatureRemove(Creature*) {}
        virtual void OnGameObjectRemove(GameObject*) {}

        // Called on player kill creature
        virtual void OnCreatureKill(Creature*) {}

        //All-purpose data storage 32 bit
        virtual uint32 GetData(uint32) const { return 0; }
        virtual void SetData(uint32, uint32 data) {}

        //Handle open / close objects
        //use HandleGameObject(NULL,boolen,GO); in OnGameObjectCreate in instance scripts
        //use HandleGameObject(GUID,boolen,NULL); in any other script
        void HandleGameObject(uint64 GUID, bool open, GameObject *go = NULL);
        
        //Respawns a GO having negative spawntimesecs in gameobject-table
        void DoRespawnGameObject(uint64 uiGuid, uint32 uiTimeToDespawn = MINUTE);
        
        //change active state of doors or buttons
        void DoUseDoorOrButton(uint64 uiGuid, uint32 uiWithRestoreTime = 0, bool bUseAlternativeState = false);
};
#endif

