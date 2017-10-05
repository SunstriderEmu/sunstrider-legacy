#include "ScriptedGossip.h"
#include "Player.h"
#include "Creature.h"
#include "GossipDef.h"

void ClearGossipMenuFor(Player* player) { player->PlayerTalkClass->ClearMenus(); }