/* Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * Thanks to the original authors: ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_PLAYER_H
#define SC_PLAYER_H

// Gossip Item Text
#define GOSSIP_TEXT_BROWSE_GOODS        "I'd like to browse your goods."
#define GOSSIP_TEXT_TRAIN               "Train me!"

enum eTradeskill
{
    // Skill defines
    TRADESKILL_ALCHEMY                  = 1,
    TRADESKILL_BLACKSMITHING            = 2,
    TRADESKILL_COOKING                  = 3,
    TRADESKILL_ENCHANTING               = 4,
    TRADESKILL_ENGINEERING              = 5,
    TRADESKILL_FIRSTAID                 = 6,
    TRADESKILL_HERBALISM                = 7,
    TRADESKILL_LEATHERWORKING           = 8,
    TRADESKILL_POISONS                  = 9,
    TRADESKILL_TAILORING                = 10,
    TRADESKILL_MINING                   = 11,
    TRADESKILL_FISHING                  = 12,
    TRADESKILL_SKINNING                 = 13,
    TRADESKILL_JEWLCRAFTING             = 14,
#ifdef LICH_KING
    TRADESKILL_INSCRIPTION              = 15,
#endif

    TRADESKILL_LEVEL_NONE               = 0,
    TRADESKILL_LEVEL_APPRENTICE         = 1,
    TRADESKILL_LEVEL_JOURNEYMAN         = 2,
    TRADESKILL_LEVEL_EXPERT             = 3,
    TRADESKILL_LEVEL_ARTISAN            = 4,
    TRADESKILL_LEVEL_MASTER             = 5,
#ifdef LICH_KING
    TRADESKILL_LEVEL_GRAND_MASTER       = 6,
#endif

    // Gossip defines
    GOSSIP_ACTION_TRADE                 = 1,
    GOSSIP_ACTION_TRAIN                 = 2,
    GOSSIP_ACTION_TAXI                  = 3,
    GOSSIP_ACTION_GUILD                 = 4,
    GOSSIP_ACTION_BATTLE                = 5,
    GOSSIP_ACTION_BANK                  = 6,
    GOSSIP_ACTION_INN                   = 7,
    GOSSIP_ACTION_HEAL                  = 8,
    GOSSIP_ACTION_TABARD                = 9,
    GOSSIP_ACTION_AUCTION               = 10,
    GOSSIP_ACTION_INN_INFO              = 11,
    GOSSIP_ACTION_UNLEARN               = 12,
    GOSSIP_ACTION_INFO_DEF              = 1000,

    GOSSIP_SENDER_MAIN                  = 1,
    GOSSIP_SENDER_INN_INFO              = 2,
    GOSSIP_SENDER_INFO                  = 3,
    GOSSIP_SENDER_SEC_PROFTRAIN         = 4,
    GOSSIP_SENDER_SEC_CLASSTRAIN        = 5,
    GOSSIP_SENDER_SEC_BATTLEINFO        = 6,
    GOSSIP_SENDER_SEC_BANK              = 7,
    GOSSIP_SENDER_SEC_INN               = 8,
    GOSSIP_SENDER_SEC_MAILBOX           = 9,
    GOSSIP_SENDER_SEC_STABLEMASTER      = 10
};

void TC_GAME_API ClearGossipMenuFor(Player* player);
// Using provided text, not from DB
void TC_GAME_API AddGossipItemFor(Player* player, uint32 icon, std::string const& text, uint32 sender, uint32 action);
// Using provided texts, not from DB
void TC_GAME_API AddGossipItemFor(Player* player, uint32 icon, std::string const& text, uint32 sender, uint32 action, std::string const& popupText, uint32 popupMoney, bool coded);
// Uses gossip item info from DB
void TC_GAME_API AddGossipItemFor(Player* player, uint32 gossipMenuID, uint32 gossipMenuItemID, uint32 sender, uint32 action);
void TC_GAME_API SendGossipMenuFor(Player* player, uint32 npcTextID, ObjectGuid const& guid);
void TC_GAME_API SendGossipMenuFor(Player* player, uint32 npcTextID, Creature const* creature);
void TC_GAME_API CloseGossipMenuFor(Player* player);

/* This one is for compat purpose, prefer using a gossip menu in db
 iconId - Icon Id
 textId - TextId (from gossip_text)
 sender - Sender(this is to identify the current Menu with this item)
 action - Action (identifys this Menu Item)
*/
#define ADD_GOSSIP_ITEM_TEXTID(iconId, textId, sender, action) PlayerTalkClass->GetGossipMenu().AddMenuItemTextID(iconId, textId, sender, action)

/* Send the default menu of the creature. (/!\  WITHOUT the previously added options, this takes the options from database). Use SEND_PREPARED_GOSSIP_MENU or SEND_GOSSIP_MENU_TEXTID
 player
 sourceCreature
*/
#define SEND_DEFAULT_GOSSIP_MENU(player, sourceCreature) { player->PrepareGossipMenu(sourceCreature, player->GetDefaultGossipMenuForSource(sourceCreature), true); player->SendPreparedGossip(sourceCreature); }

/* Same as SEND_GOSSIP_MENU_TEXTID but with default gossip text for this creature.
 pCreature - pointer to source creature

*/
#define SEND_PREPARED_GOSSIP_MENU(pPlayer, pCreature) pPlayer->PlayerTalkClass->SendGossipMenu(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID())

#endif

