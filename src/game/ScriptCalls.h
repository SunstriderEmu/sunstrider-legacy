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

#ifndef __SCRIPT_CALLS_H
#define __SCRIPT_CALLS_H

#include "Common.h"
#include "ObjectMgr.h"

class Creature;
class CreatureAI;
class GameObject;
class Item;
class Player;
class Quest;
class SpellCastTargets;
class Map;
class InstanceData;

bool LoadScriptingModule();

//On Event Handlers
typedef void( * scriptCallOnLogin) (Player *pPlayer);
typedef void( * scriptCallOnLogout) (Player *pPlayer);
typedef void( * scriptCallOnPVPKill) (Player *killer, Player *killed);

typedef void( * scriptCallScriptsInit) (char const*);
typedef void( * scriptCallScriptsFree) ();
typedef char const* ( * scriptCallScriptsVersion) ();

typedef bool( * scriptCallGossipHello) (Player *player, Creature *_Creature );
typedef bool( * scriptCallQuestAccept) (Player *player, Creature *_Creature, Quest const *);
typedef bool( * scriptCallGossipSelect)(Player *player, Creature *_Creature, uint32 sender, uint32 action);
typedef bool( * scriptCallGossipSelectWithCode)( Player *player, Creature *_Creature, uint32 sender, uint32 action, const char* sCode );
typedef bool( * scriptCallGOSelect)(Player *player, GameObject *_GO, uint32 sender, uint32 action);
typedef bool( * scriptCallGOSelectWithCode)( Player *player, GameObject *_GO, uint32 sender, uint32 action, const char* sCode );
typedef bool( * scriptCallQuestSelect)( Player *player, Creature *_Creature, Quest const* );
typedef bool( * scriptCallQuestComplete)(Player *player, Creature *_Creature, Quest const*);
typedef uint32( * scriptCallNPCDialogStatus)( Player *player, Creature *_Creature);
typedef uint32( * scriptCallGODialogStatus)( Player *player, GameObject * _GO);
typedef bool( * scriptCallChooseReward)( Player *player, Creature *_Creature, Quest const*, uint32 opt );
typedef bool( * scriptCallItemHello)( Player *player, Item *, Quest const*);
typedef bool( * scriptCallGOHello)( Player *player, GameObject * );
typedef bool( * scriptCallAreaTrigger)( Player *player, AreaTriggerEntry const* );
typedef bool( * scriptCallItemQuestAccept)(Player *player, Item *, Quest const*);
typedef bool( * scriptCallGOQuestAccept)(Player *player, GameObject *, Quest const*);
typedef bool( * scriptCallGOChooseReward)(Player *player, GameObject *, Quest const*, uint32 opt );
typedef bool( * scriptCallReceiveEmote) ( Player *player, Creature *_Creature, uint32 emote );
typedef bool( * scriptCallItemUse) (Player *player, Item *_Item, SpellCastTargets const& targets);
typedef CreatureAI* ( * scriptCallGetAI) ( Creature *_Creature );
typedef InstanceData* ( * scriptCallCreateInstanceData) (Map *map);

typedef struct
{
    scriptCallScriptsInit ScriptsInit;
    scriptCallScriptsFree ScriptsFree;
    scriptCallScriptsVersion ScriptsVersion;
    
    scriptCallOnLogin OnLogin;
    scriptCallOnLogout OnLogout;
    scriptCallOnPVPKill OnPVPKill;

    scriptCallGossipHello GossipHello;
    scriptCallGOChooseReward GOChooseReward;
    scriptCallQuestAccept QuestAccept;
    scriptCallGossipSelect GossipSelect;
    scriptCallGossipSelectWithCode GossipSelectWithCode;
    scriptCallGOSelect GOSelect;
    scriptCallGOSelectWithCode GOSelectWithCode;
    scriptCallQuestSelect QuestSelect;
    scriptCallQuestComplete QuestComplete;
    scriptCallNPCDialogStatus NPCDialogStatus;
    scriptCallGODialogStatus GODialogStatus;
    scriptCallChooseReward ChooseReward;
    scriptCallItemHello ItemHello;
    scriptCallGOHello GOHello;
    scriptCallAreaTrigger scriptAreaTrigger;
    scriptCallItemQuestAccept ItemQuestAccept;
    scriptCallGOQuestAccept GOQuestAccept;
    scriptCallReceiveEmote ReceiveEmote;
    scriptCallItemUse ItemUse;
    scriptCallGetAI GetAI;
    scriptCallCreateInstanceData CreateInstanceData;

    TRINITY_LIBRARY_HANDLE hScriptsLib;
}_ScriptSet,*ScriptsSet;

//extern ScriptsSet Script;
#endif

