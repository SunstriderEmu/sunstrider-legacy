#include "ScriptedGossip.h"
#include "Player.h"
#include "Creature.h"

void ClearGossipMenuFor(Player* player) { player->PlayerTalkClass->ClearMenus(); }