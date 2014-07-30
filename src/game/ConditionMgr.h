/*
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*
* Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
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

#ifndef TRINITY_CONDITIONMGR_H
#define TRINITY_CONDITIONMGR_H

#include "LootMgr.h"
#include "ObjectMgr.h"

class Player;
class Unit;
class LootTemplate;

enum ConditionSourceType
{
    CONDITION_SOURCE_TYPE_NONE = 0,//DONE
    CONDITION_SOURCE_TYPE_CREATURE_LOOT_TEMPLATE = 1,//DONE
    CONDITION_SOURCE_TYPE_DISENCHANT_LOOT_TEMPLATE = 2,//DONE
    CONDITION_SOURCE_TYPE_FISHING_LOOT_TEMPLATE = 3,//DONE
    CONDITION_SOURCE_TYPE_GAMEOBJECT_LOOT_TEMPLATE = 4,//DONE
    CONDITION_SOURCE_TYPE_ITEM_LOOT_TEMPLATE = 5,//DONE
    CONDITION_SOURCE_TYPE_MAIL_LOOT_TEMPLATE = 6,//DONE
    CONDITION_SOURCE_TYPE_MILLING_LOOT_TEMPLATE = 7,//DONE
    CONDITION_SOURCE_TYPE_PICKPOCKETING_LOOT_TEMPLATE = 8,//DONE
    CONDITION_SOURCE_TYPE_PROSPECTING_LOOT_TEMPLATE = 9,//DONE
    CONDITION_SOURCE_TYPE_REFERENCE_LOOT_TEMPLATE = 10,//DONE
    CONDITION_SOURCE_TYPE_SKINNING_LOOT_TEMPLATE = 11,//DONE
    CONDITION_SOURCE_TYPE_SPELL_LOOT_TEMPLATE = 12,//DONE
    CONDITION_SOURCE_TYPE_SPELL_SCRIPT_TARGET = 13,//DONE
    CONDITION_SOURCE_TYPE_GOSSIP_MENU = 14,//DONE
    CONDITION_SOURCE_TYPE_GOSSIP_MENU_OPTION = 15,//DONE
//    CONDITION_SOURCE_TYPE_CREATURE_TEMPLATE_VEHICLE = 16,//DONE
    CONDITION_SOURCE_TYPE_SPELL = 17,//DONE
    CONDITION_SOURCE_TYPE_ITEM_REQUIRED_TARGET = 18,//DONE
};

#define MAX_CONDITIONSOURCETYPE 19

struct Condition
{
    ConditionSourceType mSourceType; //SourceTypeOrReferenceId
    uint32 mSourceGroup;
    uint32 mSourceEntry;
    uint32 mElseGroup;
    ConditionType mConditionType; //ConditionTypeOrReference
    uint32 mConditionValue1;
    uint32 mConditionValue2;
    uint32 mConditionValue3;
    uint32 ErrorTextd;
    uint32 mReferenceId;

    Condition()
    {
        mSourceType = CONDITION_SOURCE_TYPE_NONE;
        mSourceGroup = 0;
        mSourceEntry = 0;
        mElseGroup = 0;
        mConditionType = CONDITION_NONE;
        mConditionValue1 = 0;
        mConditionValue2 = 0;
        mConditionValue3 = 0;
        mReferenceId = 0;
        ErrorTextd = 0;
    }
    bool Meets(Player * player, Unit* targetOverride = NULL);
    bool isLoaded() { return mConditionType > CONDITION_SOURCE_TYPE_NONE || mReferenceId; }
};

typedef std::list<Condition*> ConditionList;
typedef std::map<uint32, ConditionList > ConditionTypeMap;
typedef std::map<ConditionSourceType, ConditionTypeMap > ConditionMap;//used for all conditions, except references

typedef std::map<uint32, ConditionList > ConditionReferenceMap;//only used for references

class ConditionMgr
{
    public:
        ConditionMgr();
        ~ConditionMgr();

        void LoadConditions(bool isReload = false);
        bool isConditionTypeValid(Condition* cond);
        ConditionList GetConditionReferences(uint32 refId);
        
        bool IsPlayerMeetToConditions(Player* player, ConditionList conditions, Unit* targetOverride = NULL);
        ConditionList GetConditionsForNotGroupedEntry(ConditionSourceType sType, uint32 uEntry);

    protected:
        ConditionMap m_ConditionMap;
        ConditionReferenceMap m_ConditionReferenceMap;

    private:
        bool isSourceTypeValid(Condition* cond);
        bool addToLootTemplate(Condition* cond, LootTemplate* loot);
        bool addToGossipMenus(Condition* cond);
        bool addToGossipMenuItems(Condition* cond);
        bool IsPlayerMeetToConditionList(Player* player,const ConditionList& conditions, Unit* targetOverride = NULL);

        bool isGroupable(ConditionSourceType sourceType)
        {
            return (sourceType == CONDITION_SOURCE_TYPE_CREATURE_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_DISENCHANT_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_FISHING_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_GAMEOBJECT_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_ITEM_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_MAIL_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_MILLING_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_PICKPOCKETING_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_PROSPECTING_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_REFERENCE_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_SKINNING_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_SPELL_LOOT_TEMPLATE ||
                    sourceType == CONDITION_SOURCE_TYPE_GOSSIP_MENU ||
                    sourceType == CONDITION_SOURCE_TYPE_GOSSIP_MENU_OPTION);
        }
        
        void Clean(); // free up resources
        std::list<Condition*> m_AllocatedMemory; // some garbage collection :)
};

#define sConditionMgr Trinity::Singleton<ConditionMgr>::Instance()

#endif
