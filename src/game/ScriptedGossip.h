/* Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
 *
 * Thanks to the original authors: ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_PLAYER_H
#define SC_PLAYER_H

#include "Player.h"
#include "GossipDef.h"
#include "QuestDef.h"

// Gossip Item Text
#define GOSSIP_TEXT_BROWSE_GOODS        "J'aimerais voir ce que vous vendez."
#define GOSSIP_TEXT_TRAIN               "Entraînez-moi !"

#define GOSSIP_TEXT_BANK                "La Banque"
#define GOSSIP_TEXT_WINDRIDER           "Maître des Chevaucheurs du Vent"
#define GOSSIP_TEXT_GRYPHON             "Maître des Gryphons"
#define GOSSIP_TEXT_BATHANDLER          "Maître des Chauve-souris"
#define GOSSIP_TEXT_HIPPOGRYPH          "Maître des Hyppogryphes"
#define GOSSIP_TEXT_FLIGHTMASTER        "Maître de vol"
#define GOSSIP_TEXT_AUCTIONHOUSE        "Hotel des ventes"
#define GOSSIP_TEXT_GUILDMASTER         "Maître de Guilde"
#define GOSSIP_TEXT_INN                 "L'Auberge"
#define GOSSIP_TEXT_MAILBOX             "Boîte aux lettres"
#define GOSSIP_TEXT_STABLEMASTER        "Maître des écuries"
#define GOSSIP_TEXT_WEAPONMASTER        "Maître d'armes"
#define GOSSIP_TEXT_BATTLEMASTER        "Maître de guerre"
#define GOSSIP_TEXT_CLASSTRAINER        "Maître de Classe"
#define GOSSIP_TEXT_PROFTRAINER         "Maître de Profession"
#define GOSSIP_TEXT_OFFICERS            "Le Quartier des Officiers"

#define GOSSIP_TEXT_ALTERACVALLEY       "Vallée d'Alterac"
#define GOSSIP_TEXT_ARATHIBASIN         "Bassin d'Arathi"
#define GOSSIP_TEXT_WARSONGULCH         "Goulet des Chanteguerres"
#define GOSSIP_TEXT_ARENA               "Arène"
#define GOSSIP_TEXT_EYEOFTHESTORM       "Oeil du Cyclone"

#define GOSSIP_TEXT_DRUID               "Druide"
#define GOSSIP_TEXT_HUNTER              "Chasseur"
#define GOSSIP_TEXT_PRIEST              "Prêtre"
#define GOSSIP_TEXT_ROGUE               "Voleur"
#define GOSSIP_TEXT_WARRIOR             "Guerrier"
#define GOSSIP_TEXT_PALADIN             "Paladin"
#define GOSSIP_TEXT_SHAMAN              "Chaman"
#define GOSSIP_TEXT_MAGE                "Mage"
#define GOSSIP_TEXT_WARLOCK             "Démoniste"

#define GOSSIP_TEXT_ALCHEMY             "Alchimie"
#define GOSSIP_TEXT_BLACKSMITHING       "Forge"
#define GOSSIP_TEXT_COOKING             "Cuisine"
#define GOSSIP_TEXT_ENCHANTING          "Enchantement"
#define GOSSIP_TEXT_ENGINEERING         "Ingénierie"
#define GOSSIP_TEXT_FIRSTAID            "Premiers Soins"
#define GOSSIP_TEXT_HERBALISM           "Herborisme"
#define GOSSIP_TEXT_LEATHERWORKING      "Travail du Cuir"
#define GOSSIP_TEXT_POISONS             "Poisons"
#define GOSSIP_TEXT_TAILORING           "Couture"
#define GOSSIP_TEXT_MINING              "Minage"
#define GOSSIP_TEXT_FISHING             "Pêche"
#define GOSSIP_TEXT_SKINNING            "Dépeçage"
#define GOSSIP_TEXT_JEWELCRAFTING       "Joaillerie"

#define GOSSIP_TEXT_IRONFORGE_BANK      "Banque de Forgefer"
#define GOSSIP_TEXT_STORMWIND_BANK      "Banque de Hurlevent"
#define GOSSIP_TEXT_DEEPRUNTRAM         "Tram des Profondeurs"
#define GOSSIP_TEXT_ZEPPLINMASTER       "Maître des Zeppelins"
#define GOSSIP_TEXT_FERRY               "Ferry de Rut'theran"

// Skill defines

#define TRADESKILL_ALCHEMY                  1
#define TRADESKILL_BLACKSMITHING            2
#define TRADESKILL_COOKING                  3
#define TRADESKILL_ENCHANTING               4
#define TRADESKILL_ENGINEERING              5
#define TRADESKILL_FIRSTAID                 6
#define TRADESKILL_HERBALISM                7
#define TRADESKILL_LEATHERWORKING           8
#define TRADESKILL_POISONS                  9
#define TRADESKILL_TAILORING                10
#define TRADESKILL_MINING                   11
#define TRADESKILL_FISHING                  12
#define TRADESKILL_SKINNING                 13
#define TRADESKILL_JEWLCRAFTING             14

#define TRADESKILL_LEVEL_NONE               0
#define TRADESKILL_LEVEL_APPRENTICE         1
#define TRADESKILL_LEVEL_JOURNEYMAN         2
#define TRADESKILL_LEVEL_EXPERT             3
#define TRADESKILL_LEVEL_ARTISAN            4
#define TRADESKILL_LEVEL_MASTER             5

// Gossip defines

#define GOSSIP_ACTION_TRADE                 1
#define GOSSIP_ACTION_TRAIN                 2
#define GOSSIP_ACTION_TAXI                  3
#define GOSSIP_ACTION_GUILD                 4
#define GOSSIP_ACTION_BATTLE                5
#define GOSSIP_ACTION_BANK                  6
#define GOSSIP_ACTION_INN                   7
#define GOSSIP_ACTION_HEAL                  8
#define GOSSIP_ACTION_TABARD                9
#define GOSSIP_ACTION_AUCTION               10
#define GOSSIP_ACTION_INN_INFO              11
#define GOSSIP_ACTION_UNLEARN               12
#define GOSSIP_ACTION_INFO_DEF              1000

#define GOSSIP_SENDER_MAIN                  1
#define GOSSIP_SENDER_INN_INFO              2
#define GOSSIP_SENDER_INFO                  3
#define GOSSIP_SENDER_SEC_PROFTRAIN         4
#define GOSSIP_SENDER_SEC_CLASSTRAIN        5
#define GOSSIP_SENDER_SEC_BATTLEINFO        6
#define GOSSIP_SENDER_SEC_BANK              7
#define GOSSIP_SENDER_SEC_INN               8
#define GOSSIP_SENDER_SEC_MAILBOX           9
#define GOSSIP_SENDER_SEC_STABLEMASTER      10

#define DEFAULT_GOSSIP_MESSAGE              0xffffff

extern uint32 GetSkillLevel(Player *player,uint32 skill);

// Defined fuctions to use with player.

// a - Icon Id
// b - Text string
// c - Sender(this is to identify the current Menu with this item)
// d - Action (identifys this Menu Item)
// e - Text to be displayed in pop up box
// f - Money value in pop up box
#define ADD_GOSSIP_ITEM(a, b, c, d)   PlayerTalkClass->GetGossipMenu().AddMenuItem(-1, a, b, c, d, "", 0)
#define ADD_GOSSIP_ITEM_EXTENDED(a, b, c, d, e, f, g)   PlayerTalkClass->GetGossipMenu().AddMenuItem(-1, a, b, c, d, e, f, g)
// a - menuId
// b - menuItemId
// c - Sender(this is to identify the current Menu with this item)
// d - Action (identifys this Menu Item)
#define ADD_GOSSIP_ITEM_DB(a, b, c, d)   PlayerTalkClass->GetGossipMenu().AddMenuItem(a, b, c, d)
//This one is for compat purpose, prefer using a gossip menu in db
// a - Icon Id
// b - TextId (from gossip_text)
// c - Sender(this is to identify the current Menu with this item)
// d - Action (identifys this Menu Item)
#define ADD_GOSSIP_ITEM_TEXTID(a, b, c, d) PlayerTalkClass->GetGossipMenu().AddMenuItemTextID(a, b, c, d)

// This fuction Sends the current menu to show to client, a - NPCTEXTID(uint32) , b - npc guid(uint64)
#define SEND_GOSSIP_MENU_TEXTID(a,b)      PlayerTalkClass->SendGossipMenuTextID(a,b)
// a : player , c : creature
// Send the default menu of the creature. (/!\  WITHOUT the previously added options, this takes the options from database)
#define SEND_DEFAULT_GOSSIP_MENU(p, c) { p->PrepareGossipMenu(c, p->GetDefaultGossipMenuForSource(c), true); p->SendPreparedGossip(c); }

// This fuction shows POI(point of interest) to client.
// a - position X
// b - position Y
// c - Icon Id
// d - Flags
// e - Data
// f - Location Name
#define SEND_POI(a,b,c,d,e,f)      PlayerTalkClass->SendPointOfInterest(a,b,c,d,e,f)

// Closes the Menu
#define CLOSE_GOSSIP_MENU()        PlayerTalkClass->SendCloseGossip()

// Fuction to tell to client the details
// a - quest object
// b - npc guid(uint64)
// c - Activate accept(bool)
#define SEND_QUEST_DETAILS(a,b,c)  PlayerTalkClass->SendQuestDetails(a,b,c)

// Fuction to tell to client the requested items to complete quest
// a - quest object
// b - npc guid(uint64)
// c - Iscompletable(bool)
// d - close at cancel(bool) - in case single incomplite ques
#define SEND_REQUESTEDITEMS(a,b,c,d) PlayerTalkClass->SendRequestedItems(a,b,c,d)

// Fuctions to send NPC lists, a - is always the npc guid(uint64)
#define SEND_VENDORLIST(a)         GetSession()->SendListInventory(a)
#define SEND_TRAINERLIST(a)        GetSession()->SendTrainerList(a)
#define SEND_BANKERLIST(a)         GetSession()->SendShowBank(a)
#define SEND_TABARDLIST(a)         GetSession()->SendTabardVendorActivate(a)
#define SEND_AUCTIONLIST(a)        GetSession()->SendAuctionHello(a)
#define SEND_TAXILIST(a)           GetSession()->SendTaxiStatus(a)

// Ressurect's the player if is dead.
#define SEND_SPRESURRECT()         GetSession()->SendSpiritResurrect()

// Get the player's honor rank.
#define GET_HONORRANK()            GetHonorRank()
// -----------------------------------

// defined fuctions to use with Creature

#define QUEST_DIALOG_STATUS(a,b,c)   GetSession()->GetQuestDialogStatus(a,b,c)
#endif

