
#ifndef TRINITY_SKILL_EXTRA_ITEMS_H
#define TRINITY_SKILL_EXTRA_ITEMS_H

// predef classes used in functions
class Player;

// returns true and sets the appropriate info if the player can create extra items with the given spellId
TC_GAME_API bool CanCreateExtraItems(Player* player, uint32 spellId, float &additionalChance, uint8 &additionalMax);
// function to load the extra item creation info from DB
TC_GAME_API void LoadSkillExtraItemTable();
#endif

