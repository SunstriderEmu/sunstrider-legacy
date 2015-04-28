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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef TRINITYCORE_GOSSIP_H
#define TRINITYCORE_GOSSIP_H

#include "Common.h"
#include "QuestDef.h"
#include "NPCHandler.h"

class WorldSession;

#define GOSSIP_MAX_MENU_ITEMS 64                            // client supported items unknown, but provided number must be enough
#define DEFAULT_GOSSIP_MESSAGE              0xffffff

//POI defines
enum Poi_Icon
{
    ICON_POI_0                  =   0,                      // Grey ?
    ICON_POI_1                  =   1,                      // Red ?
    ICON_POI_2                  =   2,                      // Blue ?
    ICON_POI_BWTOMB             =   3,                      // Blue and White Tomb Stone
    ICON_POI_HOUSE              =   4,                      // House
    ICON_POI_TOWER              =   5,                      // Tower
    ICON_POI_REDFLAG            =   6,                      // Red Flag with Yellow !
    ICON_POI_TOMB               =   7,                      // Tomb Stone
    ICON_POI_BWTOWER            =   8,                      // Blue and White Tower
    ICON_POI_REDTOWER           =   9,                      // Red Tower
    ICON_POI_BLUETOWER          =   10,                     // Blue Tower
    ICON_POI_RWTOWER            =   11,                     // Red and White Tower
    ICON_POI_REDTOMB            =   12,                     // Red Tomb Stone
    ICON_POI_RWTOMB             =   13,                     // Red and White Tomb Stone
    ICON_POI_BLUETOMB           =   14,                     // Blue Tomb Stone
    ICON_POI_NOTHING            =   15,                     // NOTHING
    ICON_POI_16                 =   16,                     // Red ?
    ICON_POI_17                 =   17,                     // Grey ?
    ICON_POI_18                 =   18,                     // Blue ?
    ICON_POI_19                 =   19,                     // Red and White ?
    ICON_POI_20                 =   20,                     // Red ?
    ICON_POI_GREYLOGS           =   21,                     // Grey Wood Logs
    ICON_POI_BWLOGS             =   22,                     // Blue and White Wood Logs
    ICON_POI_BLUELOGS           =   23,                     // Blue Wood Logs
    ICON_POI_RWLOGS             =   24,                     // Red and White Wood Logs
    ICON_POI_REDLOGS            =   25,                     // Red Wood Logs
    ICON_POI_26                 =   26,                     // Grey ?
    ICON_POI_27                 =   27,                     // Blue and White ?
    ICON_POI_28                 =   28,                     // Blue ?
    ICON_POI_29                 =   29,                     // Red and White ?
    ICON_POI_30                 =   30,                     // Red ?
    ICON_POI_GREYHOUSE          =   31,                     // Grey House
    ICON_POI_BWHOUSE            =   32,                     // Blue and White House
    ICON_POI_BLUEHOUSE          =   33,                     // Blue House
    ICON_POI_RWHOUSE            =   34,                     // Red and White House
    ICON_POI_REDHOUSE           =   35,                     // Red House
    ICON_POI_GREYHORSE          =   36,                     // Grey Horse
    ICON_POI_BWHORSE            =   37,                     // Blue and White Horse
    ICON_POI_BLUEHORSE          =   38,                     // Blue Horse
    ICON_POI_RWHORSE            =   39,                     // Red and White Horse
    ICON_POI_REDHORSE           =   40                      // Red Horse
};

struct GossipMenuItem
{
    uint8       MenuItemIcon;
    bool        IsCoded;
    std::string Message;
    uint32      Sender;
    uint32      OptionType;
    std::string BoxMessage;
    uint32      BoxMoney;
};

// need an ordered container
typedef std::map<uint32, GossipMenuItem> GossipMenuItemContainer;

struct GossipMenuItemData
{
    uint32 GossipActionMenuId;  // MenuId of the gossip triggered by this action
    uint32 GossipActionPoi;
};

// need an ordered container
typedef std::map<uint32, GossipMenuItemData> GossipMenuItemDataContainer;

struct QuestMenuItem
{
    uint32      QuestId;
    uint8       QuestIcon;
};

typedef std::vector<QuestMenuItem> QuestMenuItemList;

class GossipMenu
{
    public:
        GossipMenu();
        ~GossipMenu();

        void AddMenuItem(int32 menuItemId, uint8 icon, std::string const& message, uint32 sender, uint32 action, std::string const& boxMessage, uint32 boxMoney, bool coded = false);
        void AddMenuItem(uint32 menuId, uint32 menuItemId, uint32 sender, uint32 action);
        void AddMenuItemTextID(uint8 icon, uint32 textID, uint32 sender, uint32 action);

        void SetMenuId(uint32 menu_id) { _menuId = menu_id; }
        uint32 GetMenuId() const { return _menuId; }
        void SetSenderGUID(uint64 guid) { _senderGUID = guid; }
        uint64 GetSenderGUID() const { return _senderGUID; }
        void SetLocale(LocaleConstant locale) { _locale = locale; }
        LocaleConstant GetLocale() const { return _locale; }

        void AddGossipMenuItemData(uint32 menuItemId, uint32 gossipActionMenuId, uint32 gossipActionPoi);

        uint32 GetMenuItemCount() const
        {
            return _menuItems.size();
        }

        bool Empty() const
        {
            return _menuItems.empty();
        }

        GossipMenuItem const* GetItem(uint32 id) const
        {
            GossipMenuItemContainer::const_iterator itr = _menuItems.find(id);
            if (itr != _menuItems.end())
                return &itr->second;

            return NULL;
        }

        GossipMenuItemData const* GetItemData(uint32 indexId) const
        {
            GossipMenuItemDataContainer::const_iterator itr = _menuItemData.find(indexId);
            if (itr != _menuItemData.end())
                return &itr->second;

            return NULL;
        }

        uint32 GetMenuItemSender(uint32 menuItemId) const;
        uint32 GetMenuItemAction(uint32 menuItemId) const;
        bool IsMenuItemCoded(uint32 menuItemId) const;

        void ClearMenu();

        GossipMenuItemContainer const& GetMenuItems() const
        {
            return _menuItems;
        }

    private:
        GossipMenuItemContainer _menuItems;
        GossipMenuItemDataContainer _menuItemData;
        uint32 _menuId;
        uint64 _senderGUID;
        LocaleConstant _locale;
};

class QuestMenu
{
    public:
        QuestMenu();
        ~QuestMenu();

        void AddMenuItem(uint32 QuestId, uint8 Icon);
        void ClearMenu();

        uint8 GetMenuItemCount() const
        {
            return _questMenuItems.size();
        }

        bool Empty() const
        {
            return _questMenuItems.empty();
        }

        bool HasItem(uint32 questId) const;

        QuestMenuItem const& GetItem(uint16 index) const
        {
            return _questMenuItems[index];
        }

    private:
        QuestMenuItemList _questMenuItems;
};

class PlayerMenu
{
    public:
        explicit PlayerMenu(WorldSession* session);
        ~PlayerMenu();

        GossipMenu& GetGossipMenu() { return _gossipMenu; }
        QuestMenu& GetQuestMenu() { return _questMenu; }

        bool Empty() const { return _gossipMenu.Empty() && _questMenu.Empty(); }

        void ClearMenus();
        uint32 GetGossipOptionSender(uint32 selection) const { return _gossipMenu.GetMenuItemSender(selection); }
        uint32 GetGossipOptionAction(uint32 selection) const { return _gossipMenu.GetMenuItemAction(selection); }
        bool IsGossipOptionCoded(uint32 selection) const { return _gossipMenu.IsMenuItemCoded(selection); }

        //Sends a gossip menu with given text id. /!\ This is NOT a menu id
        void SendGossipMenuTextID(uint32 titleTextId, uint64 objectGUID);
        void SendCloseGossip();
        void SendPointOfInterest( float X, float Y, uint32 Icon, uint32 Flags, uint32 Data, const char * locName ) const;
        void SendPointOfInterest(uint32 poiId) const;

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/
        void SendQuestGiverStatus(uint8 questStatus, uint64 npcGUID) const;

        void SendQuestGiverQuestList(QEmote const& eEmote, const std::string& Title, uint64 npcGUID);

        void SendQuestQueryResponse(Quest const* quest) const;
        void SendQuestGiverQuestDetails(Quest const* quest, uint64 npcGUID, bool activateAccept) const;

        void SendQuestGiverOfferReward(Quest const* quest, uint64 npcGUID, bool enableNext) const;
        void SendQuestGiverRequestItems(Quest const* quest, uint64 npcGUID, bool canComplete, bool closeOnCancel) const;

        static void AddQuestLevelToTitle(std::string &title, int32 level);

    private:
        GossipMenu _gossipMenu;
        QuestMenu  _questMenu;
        WorldSession* _session;
};
#endif

