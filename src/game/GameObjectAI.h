/*
* Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRINITY_GAMEOBJECTAI_H
#define TRINITY_GAMEOBJECTAI_H

#include "Define.h"
#include <list>
#include "CreatureAI.h"
#include "Object.h"
#include "GameObject.h"

//class GameObject;
enum GOState: uint32;
enum LootState: uint32;

class GameObjectAI
{
    protected:
        GameObject * const go;
    public:
        explicit GameObjectAI(GameObject *g) : go(g) {}
        virtual ~GameObjectAI() {}

        virtual void UpdateAI(const uint32 diff) {}

        virtual void InitializeAI() { Reset(); }

        virtual void Reset() {};
        
        static int Permissible(const GameObject* go);
        
        virtual bool OnGossipHello(Player* player) {return false;}
        virtual bool OnGossipSelect(Player* player, uint32 sender, uint32 action) {return false;}
        virtual bool OnGossipSelectCode(Player* /*player*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/) {return false;}
        virtual bool OnQuestAccept(Player* player, Quest const* quest) {return false;}
        virtual bool QuestReward(Player* player, Quest const* quest, uint32 opt) {return false;}
        uint32 GetDialogStatus(Player* /*player*/);
        virtual void Destroyed(Player* player, uint32 eventId) {}
        virtual void SetData(uint32 id, uint32 value) {}
        virtual void GetData(uint32 id) const {}
        virtual void EventInform(uint32 /*eventId*/) {}

        virtual void OnStateChanged(GOState /*state*/, Unit* /*unit*/) { }
        virtual void OnLootStateChanged(LootState /*state*/, Unit* /*unit*/) { }
};

class NullGameObjectAI : public GameObjectAI
{
    public:
        explicit NullGameObjectAI(GameObject *g);

        void UpdateAI(const uint32) {}

        static int Permissible(const GameObject* go) { return PERMIT_BASE_IDLE; }
};
#endif

