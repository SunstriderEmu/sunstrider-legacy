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

#include "LootMgr.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "World.h"
#include "Util.h"
#include "SharedDefines.h"
#include "ItemEnchantmentMgr.h"

static Rates const qualityToRate[MAX_ITEM_QUALITY] = {
    RATE_DROP_ITEM_POOR,                                    // ITEM_QUALITY_POOR
    RATE_DROP_ITEM_NORMAL,                                  // ITEM_QUALITY_NORMAL
    RATE_DROP_ITEM_UNCOMMON,                                // ITEM_QUALITY_UNCOMMON
    RATE_DROP_ITEM_RARE,                                    // ITEM_QUALITY_RARE
    RATE_DROP_ITEM_EPIC,                                    // ITEM_QUALITY_EPIC
    RATE_DROP_ITEM_LEGENDARY,                               // ITEM_QUALITY_LEGENDARY
    RATE_DROP_ITEM_ARTIFACT,                                // ITEM_QUALITY_ARTIFACT
};

LootStore LootTemplates_Creature(     "creature_loot_template",     "creature entry");
LootStore LootTemplates_Disenchant(   "disenchant_loot_template",   "item disenchant id");
LootStore LootTemplates_Fishing(      "fishing_loot_template",      "area id");
LootStore LootTemplates_Gameobject(   "gameobject_loot_template",   "gameobject entry");
LootStore LootTemplates_Item(         "item_loot_template",         "item entry");
LootStore LootTemplates_Pickpocketing("pickpocketing_loot_template","creature pickpocket lootid");
LootStore LootTemplates_Prospecting(  "prospecting_loot_template",  "item entry");
LootStore LootTemplates_QuestMail(    "quest_mail_loot_template",   "quest id");
LootStore LootTemplates_Reference(    "reference_loot_template",    "reference id");
LootStore LootTemplates_Skinning(     "skinning_loot_template",     "creature skinning id");

class LootTemplate::LootGroup                               // A set of loot definitions for items (refs are not allowed)
{
    public:
        void AddEntry(LootStoreItem* item);                 // Adds an entry to the group (at loading stage)
        bool HasQuestDrop() const;                          // True if group includes at least 1 quest drop entry
        bool HasQuestDropForPlayer(Player const * player) const;
                                                            // The same for active quests of the player
        void Process(Loot& loot) const;                     // Rolls an item from the group (if any) and adds the item to the loot
        float RawTotalChance() const;                       // Overall chance for the group (without equal chanced items)
        float TotalChance() const;                          // Overall chance for the group

        void Verify(LootStore const& lootstore, uint32 id, uint32 group_id) const;
        void CollectLootIds(LootIdSet& set) const;
        void CheckLootRefs(LootTemplateMap const& store, LootIdSet* ref_set) const;

        LootStoreItemList* GetExplicitlyChancedItemList() { return &ExplicitlyChanced; }
        LootStoreItemList* GetEqualChancedItemList() { return &EqualChanced; }
        void CopyConditions(ConditionContainer conditions);
    private:
        LootStoreItemList ExplicitlyChanced;                // Entries with chances defined in DB
        LootStoreItemList EqualChanced;                     // Zero chances - every entry takes the same chance

        LootStoreItem const * Roll() const;                 // Rolls an item from the group, returns NULL if all miss their chances
};

//Remove all data and free all memory
void LootStore::Clear()
{
    for (LootTemplateMap::const_iterator itr=m_LootTemplates.begin(); itr != m_LootTemplates.end(); ++itr)
        delete itr->second;

    m_LootTemplates.clear();
}

// Checks validity of the loot store
// Actual checks are done within LootTemplate::Verify() which is called for every template
void LootStore::Verify() const
{
    for (const auto & m_LootTemplate : m_LootTemplates)
        m_LootTemplate.second->Verify(*this, m_LootTemplate.first);
}

// Loads a *_loot_template DB table into loot store
// All checks of the loaded template are called from here, no error reports at loot generation required
void LootStore::LoadLootTable()
{
    LootTemplateMap::const_iterator tab;
    uint32 count = 0;

    // Clearing store (for reloading case)
    Clear();

    TC_LOG_INFO("server.loading", "%s :", GetName());

    //                                                 0      1     2                    3        4              5         6              7                 8
    QueryResult result = WorldDatabase.PQuery("SELECT entry, item, ChanceOrQuestChance, groupid, mincountOrRef, maxcount, lootcondition, condition_value1, condition_value2 FROM %s",GetName());

    if(!result)
    {
        TC_LOG_ERROR( "server.loading",">> Loaded 0 loot definitions. DB table `%s` is empty.", GetName());
        return;
    }

    do
    {
        Field *fields = result->Fetch();

        uint32 entry               = fields[0].GetUInt32();
        uint32 item                = fields[1].GetUInt32();
        float  chanceOrQuestChance = fields[2].GetFloat();
        uint8  group               = fields[3].GetUInt8();
        int32  mincountOrRef       = fields[4].GetInt32();
        uint8  maxcount            = fields[5].GetUInt8();
        OldConditionType condition    = (OldConditionType)fields[6].GetUInt8();
        uint32 cond_value1         = fields[7].GetUInt32();
        uint32 cond_value2         = fields[8].GetUInt32();

        if(!PlayerCondition::IsValid(condition,cond_value1, cond_value2))
        {
            TC_LOG_ERROR("sql.sql","... in table '%s' entry %u item %u", GetName(), entry, item);
            continue;                                   // error already printed to log/console.
        }

        // (condition + cond_value1/2) are converted into single conditionId
        uint16 conditionId = sObjectMgr->GetConditionId(condition, cond_value1, cond_value2);

        auto  storeitem = new LootStoreItem(item, chanceOrQuestChance, group, conditionId, mincountOrRef, maxcount);

        if (!storeitem->IsValid(*this, entry))            // Validity checks
        {
            delete storeitem;
            storeitem = nullptr;
            continue;
        }

        // Looking for the template of the entry
                                                        // often entries are put together
        if (m_LootTemplates.empty() || tab->first != entry)
        {
            // Searching the template (in case template Id changed)
            tab = m_LootTemplates.find(entry);
            if ( tab == m_LootTemplates.end() )
            {
                std::pair< LootTemplateMap::iterator, bool > pr = m_LootTemplates.insert(LootTemplateMap::value_type(entry, new LootTemplate));
                tab = pr.first;
            }
        }
        // else is empty - template Id and iter are the same
        // finally iter refers to already existed or just created <entry, LootTemplate>

        // Adds current row to the template
        tab->second->AddEntry(storeitem);
        ++count;

    } while (result->NextRow());

    Verify();                                           // Checks validity of the loot store

    TC_LOG_INFO( "server.loading",">> Loaded %u loot definitions (%u templates)", count, uint32(m_LootTemplates.size()));
}

bool LootStore::HaveQuestLootFor(uint32 loot_id) const
{
    auto itr = m_LootTemplates.find(loot_id);
    if(itr == m_LootTemplates.end())
        return false;

    // scan loot for quest items
    return itr->second->HasQuestDrop(m_LootTemplates);
}

bool LootStore::HaveQuestLootForPlayer(uint32 loot_id,Player* player) const
{
    auto tab = m_LootTemplates.find(loot_id);
    if (tab != m_LootTemplates.end())
        if (tab->second->HasQuestDropForPlayer(m_LootTemplates, player))
            return true;

    return false;
}

LootTemplate const* LootStore::GetLootFor(uint32 loot_id) const
{
    auto tab = m_LootTemplates.find(loot_id);

    if (tab == m_LootTemplates.end())
        return nullptr;

    return tab->second;
}

void LootStore::ResetConditions()
{
    for (auto & m_LootTemplate : m_LootTemplates)
    {
        ConditionContainer empty;
        m_LootTemplate.second->CopyConditions(empty);
    }
}

LootTemplate* LootStore::GetLootForConditionFill(uint32 loot_id)
{
    auto tab = m_LootTemplates.find(loot_id);

    if (tab == m_LootTemplates.end())
        return nullptr;

    return tab->second;
}

void LootStore::LoadAndCollectLootIds(LootIdSet& ids_set)
{
    LoadLootTable();

    for(LootTemplateMap::const_iterator tab = m_LootTemplates.begin(); tab != m_LootTemplates.end(); ++tab)
        ids_set.insert(tab->first);
}

void LootStore::CheckLootRefs(LootIdSet* ref_set) const
{
    for(const auto & m_LootTemplate : m_LootTemplates)
        m_LootTemplate.second->CheckLootRefs(m_LootTemplates,ref_set);
}

void LootStore::ReportUnusedIds(LootIdSet const& ids_set) const
{
    // all still listed ids isn't referenced
    for(uint32 itr : ids_set)
    {
        TC_LOG_ERROR("sql.sql", "Table '%s' entry %d isn't %s and not referenced from loot, and then useless.", GetName(), itr, GetEntryName());
    }
}

void LootStore::ReportNotExistedId(uint32 id) const
{
    TC_LOG_ERROR("sql.sql","Table '%s' entry %d (%s) not exist but used as loot id in DB.", GetName(), id, GetEntryName());
}

//
// --------- LootStoreItem ---------
//

// Checks if the entry (quest, non-quest, reference) takes it's chance (at loot generation)
// RATE_DROP_ITEMS is no longer used for all types of entries
bool LootStoreItem::Roll() const
{
    if(chance>=100.f)
        return true;

    if(mincountOrRef < 0)                                   // reference case
        return roll_chance_f(chance*sWorld->GetRate(RATE_DROP_ITEM_REFERENCED));

    ItemTemplate const *pProto = sObjectMgr->GetItemTemplate(itemid);

    float qualityModifier = pProto ? sWorld->GetRate(qualityToRate[pProto->Quality]) : 1.0f;

    return roll_chance_f(chance*qualityModifier);
}

// Checks correctness of values
bool LootStoreItem::IsValid(LootStore const& store, uint32 entry) const
{
    if (mincountOrRef == 0)
    {
        TC_LOG_ERROR("sql.sql","Table '%s' entry %d item %d: wrong mincountOrRef (%d) - skipped", store.GetName(), entry, itemid, mincountOrRef);
        return false;
    }

    if( mincountOrRef > 0 )                                 // item (quest or non-quest) entry, maybe grouped
    {
        ItemTemplate const *proto = sObjectMgr->GetItemTemplate(itemid);
        if(!proto)
        {
            TC_LOG_ERROR("sql.sql","Table '%s' entry %d item %d: item entry not listed in `item_template` - skipped", store.GetName(), entry, itemid);
            return false;
        }

        if( chance == 0 && group == 0)                      // Zero chance is allowed for grouped entries only
        {
            TC_LOG_ERROR("sql.sql","Table '%s' entry %d item %d: equal-chanced grouped entry, but group not defined - skipped", store.GetName(), entry, itemid);
            return false;
        }

        if( chance != 0 && chance < 0.000001f )             // loot with low chance
        {
            TC_LOG_ERROR("sql.sql","Table '%s' entry %d item %d: low chance (%f) - skipped",
                store.GetName(), entry, itemid, chance);
            return false;
        }
    }
    else                                                    // mincountOrRef < 0
    {
        if (needs_quest)
            TC_LOG_ERROR("sql.sql","Table '%s' entry %d item %d: quest chance will be treated as non-quest chance", store.GetName(), entry, itemid);
        else if( chance == 0 )                              // no chance for the reference
        {
            TC_LOG_ERROR("sql.sql","Table '%s' entry %d item %d: zero chance is specified for a reference, skipped", store.GetName(), entry, itemid);
            return false;
        }
    }
    return true;                                            // Referenced template existence is checked at whole store level
}

//
// --------- LootItem ---------
//

// Constructor, copies most fields from LootStoreItem and generates random count
LootItem::LootItem(LootStoreItem const& li)
{
    itemid      = li.itemid;
    conditionId = li.conditionId;
    conditions = li.conditions;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemid);
    freeforall  = proto && (proto->Flags & ITEM_FLAG_PARTY_LOOT);

    needs_quest = li.needs_quest;

    count       = urand(li.mincountOrRef, li.maxcount);     // constructor called for mincountOrRef > 0 only
    randomSuffix = GenerateEnchSuffixFactor(itemid);
    randomPropertyId = Item::GenerateItemRandomPropertyId(itemid);
    is_looted = 0;
    is_blocked = 0;
    is_underthreshold = 0;
    is_counted = 0;
}

// Basic checks for player/item compatibility - if false no chance to see the item in the loot
bool LootItem::AllowedForPlayer(Player const * player) const
{
    // Old DB conditions check
    if ( !sObjectMgr->IsPlayerMeetToCondition(player,conditionId) )
        return false;

    if (!sConditionMgr->IsObjectMeetToConditions(const_cast<Player*>(player), conditions))
        return false;

    if ( needs_quest )
    {
        // Checking quests for quest-only drop (check only quests requirements in this case)
        if( !player->HasQuestForItem(itemid) )
            return false;
    }
    else
    {
        // Not quest only drop (check quest starting items for already accepted non-repeatable quests)
        ItemTemplate const *pProto = sObjectMgr->GetItemTemplate(itemid);
        if (pProto && pProto->StartQuest && player->GetQuestStatus(pProto->StartQuest) != QUEST_STATUS_NONE && !player->HasQuestForItem(itemid))
            return false;

        // Don't show unique items if player already has the maximum allowed quantity.
        if (pProto->MaxCount && int32(player->GetItemCount(itemid, true)) >= pProto->MaxCount)
            return false;
    }

    return true;
}

//
// --------- Loot ---------
//

// Inserts the item into the loot (called by LootTemplate processors)
void Loot::AddItem(LootStoreItem const & item)
{
    // Check if item isn't in removed_items
    for (std::vector<uint32>::const_iterator itr = removed_items.begin(); itr != removed_items.end(); itr++) {
        if ((*itr) == item.itemid)
            return;
    }
    
    if (item.needs_quest)                                   // Quest drop
    {
        if (quest_items.size() < MAX_NR_QUEST_ITEMS)
            quest_items.push_back(LootItem(item));
    }
    else if (items.size() < MAX_NR_LOOT_ITEMS)              // Non-quest drop
    {
        items.push_back(LootItem(item));

        // non-conditional one-player only items are counted here,
        // free for all items are counted in FillFFALoot(),
        // non-ffa conditionals are counted in FillNonQuestNonFFAConditionalLoot()
        if( !item.conditionId )
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item.itemid);
            if( !proto || (proto->Flags & ITEM_FLAG_PARTY_LOOT)==0 )
                ++unlootedCount;
        }
    }
}

void Loot::RemoveItem(uint32 entry)
{    
    removed_items.push_back(entry);
}

// return true if there is any FFA, quest or conditional item for the player.
bool Loot::hasItemFor(Player const* player) const
{
    QuestItemMap const& lootPlayerQuestItems = GetPlayerQuestItems();
    auto q_itr = lootPlayerQuestItems.find(player->GetGUID().GetCounter());
    if (q_itr != lootPlayerQuestItems.end())
    {
        QuestItemList* q_list = q_itr->second;
        for (QuestItemList::const_iterator qi = q_list->begin(); qi != q_list->end(); ++qi)
        {
            const LootItem &item = quest_items[qi->index];
            if (!qi->is_looted && !item.is_looted)
                return true;
        }
    }

    QuestItemMap const& lootPlayerFFAItems = GetPlayerFFAItems();
    auto ffa_itr = lootPlayerFFAItems.find(player->GetGUID().GetCounter());
    if (ffa_itr != lootPlayerFFAItems.end())
    {
        QuestItemList* ffa_list = ffa_itr->second;
        for (QuestItemList::const_iterator fi = ffa_list->begin(); fi != ffa_list->end(); ++fi)
        {
            const LootItem &item = items[fi->index];
            if (!fi->is_looted && !item.is_looted)
                return true;
        }
    }

    QuestItemMap const& lootPlayerNonQuestNonFFAConditionalItems = GetPlayerNonQuestNonFFAConditionalItems();
    auto nn_itr = lootPlayerNonQuestNonFFAConditionalItems.find(player->GetGUID().GetCounter());
    if (nn_itr != lootPlayerNonQuestNonFFAConditionalItems.end())
    {
        QuestItemList* conditional_list = nn_itr->second;
        for (QuestItemList::const_iterator ci = conditional_list->begin(); ci != conditional_list->end(); ++ci)
        {
            const LootItem &item = items[ci->index];
            if (!ci->is_looted && !item.is_looted)
                return true;
        }
    }

    return false;
}

// return true if there is any item over the group threshold (i.e. not underthreshold).
bool Loot::hasOverThresholdItem() const
{
    for (const auto & item : items)
    {
        if (!item.is_looted && !item.is_underthreshold && !item.freeforall)
            return true;
    }

    return false;
}

void Loot::FillNotNormalLootFor(Player* player, bool presentAtLooting)
{
    ObjectGuid::LowType plguid = player->GetGUID().GetCounter();
    auto qmapitr = PlayerQuestItems.find(plguid);
    if (qmapitr == PlayerQuestItems.end())
    {
        FillQuestLoot(player);
    }
    qmapitr = PlayerFFAItems.find(plguid);
    if (qmapitr == PlayerFFAItems.end())
    {
        FillFFALoot(player);
    }
    qmapitr = PlayerNonQuestNonFFAConditionalItems.find(plguid);
    if (qmapitr == PlayerNonQuestNonFFAConditionalItems.end())
    {
        FillNonQuestNonFFAConditionalLoot(player);
    }
}

// Calls processor of corresponding LootTemplate (which handles everything including references)
void Loot::FillLoot(uint32 loot_id, LootStore const& store, Player* loot_owner)
{
    LootTemplate const* tab = store.GetLootFor(loot_id);

    if (!tab)
    {
        TC_LOG_ERROR("FIXME","Table '%s' loot id #%u used but it doesn't have records.",store.GetName(),loot_id);
        return;
    }

    items.reserve(MAX_NR_LOOT_ITEMS);
    quest_items.reserve(MAX_NR_QUEST_ITEMS);

    tab->Process(*this, store);                             // Processing is done there, callback via Loot::AddItem()

    // Setting access rights fow group-looting case
    if(!loot_owner)
        return;
    Group * pGroup=loot_owner->GetGroup();
    if (pGroup)
    {
        roundRobinPlayer = loot_owner->GetGUID();

        for (GroupReference *itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            //fill the quest item map for every player in the recipient's group
            Player* pl = itr->GetSource();
            if (!pl || !pl->IsInMap(loot_owner))
                continue;

            FillNotNormalLootFor(loot_owner, pl->IsAtGroupRewardDistance(loot_owner));
        }
    }
    else {
        FillNotNormalLootFor(loot_owner, true);
    }
}

QuestItemList* Loot::FillFFALoot(Player* player)
{
    auto ql = new QuestItemList();

    for(uint8 i = 0; i < items.size(); i++)
    {
        LootItem &item = items[i];
        if(!item.is_looted && item.freeforall && item.AllowedForPlayer(player) )
        {
            ql->push_back(QuestItem(i));
            ++unlootedCount;
        }
    }
    if (ql->empty())
    {
        delete ql;
        return nullptr;
    }

    PlayerFFAItems[player->GetGUID().GetCounter()] = ql;
    return ql;
}

QuestItemList* Loot::FillQuestLoot(Player* player)
{
    if (items.size() == MAX_NR_LOOT_ITEMS) return nullptr;
    auto ql = new QuestItemList();

    for(uint8 i = 0; i < quest_items.size(); i++)
    {
        LootItem &item = quest_items[i];
        if(!item.is_looted && item.AllowedForPlayer(player) )
        {
            ql->push_back(QuestItem(i));

            // questitems get blocked when they first apper in a
            // player's quest vector
            //
            // increase once if one looter only, looter-times if free for all
            if (item.freeforall || !item.is_blocked)
                ++unlootedCount;

            item.is_blocked = true;

            if (items.size() + ql->size() == MAX_NR_LOOT_ITEMS)
                break;
        }
    }
    if (ql->empty())
    {
        delete ql;
        return nullptr;
    }

    PlayerQuestItems[player->GetGUID().GetCounter()] = ql;
    return ql;
}

QuestItemList* Loot::FillNonQuestNonFFAConditionalLoot(Player* player)
{
    auto ql = new QuestItemList();

    for(uint8 i = 0; i < items.size(); ++i)
    {
        LootItem &item = items[i];
        if(!item.is_looted && !item.freeforall && item.conditionId && item.AllowedForPlayer(player))
        {
            ql->push_back(QuestItem(i));
            if(!item.is_counted)
            {
                ++unlootedCount;
                item.is_counted=true;
            }
        }
    }
    if (ql->empty())
    {
        delete ql;
        return nullptr;
    }

    PlayerNonQuestNonFFAConditionalItems[player->GetGUID().GetCounter()] = ql;
    return ql;
}

//===================================================

void Loot::NotifyItemRemoved(uint8 lootIndex)
{
    // notify all players that are looting this that the item was removed
    // convert the index to the slot the player sees
    std::set<ObjectGuid>::iterator i_next;
    for(auto i = PlayersLooting.begin(); i != PlayersLooting.end(); i = i_next)
    {
        i_next = i;
        ++i_next;
        if(Player* pl = ObjectAccessor::FindPlayer(*i))
            pl->SendNotifyLootItemRemoved(lootIndex);
        else
            PlayersLooting.erase(i);
    }
}

void Loot::NotifyMoneyRemoved()
{
    // notify all players that are looting this that the money was removed
    std::set<ObjectGuid>::iterator i_next;
    for(auto i = PlayersLooting.begin(); i != PlayersLooting.end(); i = i_next)
    {
        i_next = i;
        ++i_next;
        if(Player* pl = ObjectAccessor::FindPlayer(*i))
            pl->SendNotifyLootMoneyRemoved();
        else
            PlayersLooting.erase(i);
    }
}

void Loot::NotifyQuestItemRemoved(uint8 questIndex)
{
    // when a free for all questitem is looted
    // all players will get notified of it being removed
    // (other questitems can be looted by each group member)
    // bit inefficient but isn't called often

    std::set<ObjectGuid>::iterator i_next;
    for(auto i = PlayersLooting.begin(); i != PlayersLooting.end(); i = i_next)
    {
        i_next = i;
        ++i_next;
        if(Player* pl = ObjectAccessor::FindPlayer(*i))
        {
            auto pq = PlayerQuestItems.find(pl->GetGUID().GetCounter());
            if (pq != PlayerQuestItems.end() && pq->second)
            {
                // find where/if the player has the given item in it's vector
                QuestItemList& pql = *pq->second;

                uint8 j;
                for (j = 0; j < pql.size(); ++j)
                    if (pql[j].index == questIndex)
                        break;

                if (j < pql.size())
                    pl->SendNotifyLootItemRemoved(items.size()+j);
            }
        }
        else
            PlayersLooting.erase(i);
    }
}

void Loot::generateMoneyLoot( uint32 minAmount, uint32 maxAmount )
{
    if (maxAmount > 0)
    {
        if (maxAmount <= minAmount)
            gold = uint32(maxAmount * sWorld->GetRate(RATE_DROP_MONEY));
        else if ((maxAmount - minAmount) < 32700)
            gold = uint32(urand(minAmount, maxAmount) * sWorld->GetRate(RATE_DROP_MONEY));
        else
            gold = uint32(urand(minAmount >> 8, maxAmount >> 8) * sWorld->GetRate(RATE_DROP_MONEY)) << 8;
    }
}

LootItem* Loot::LootItemInSlot(uint32 lootSlot, Player* player, QuestItem **qitem, QuestItem **ffaitem, QuestItem **conditem)
{
    LootItem* item = nullptr;
    bool is_looted = true;
    if (lootSlot >= items.size())
    {
        uint32 questSlot = lootSlot - items.size();
        QuestItemMap::const_iterator itr = PlayerQuestItems.find(player->GetGUID().GetCounter());
        if (itr != PlayerQuestItems.end() && questSlot < itr->second->size())
        {
            QuestItem *qitem2 = &itr->second->at(questSlot);
            if(qitem)
                *qitem = qitem2;
            item = &quest_items[qitem2->index];
            is_looted = qitem2->is_looted;
        }
    }
    else
    {
        item = &items[lootSlot];
        is_looted = item->is_looted;
        if(item->freeforall)
        {
            QuestItemMap::const_iterator itr = PlayerFFAItems.find(player->GetGUID().GetCounter());
            if (itr != PlayerFFAItems.end())
            {
                for(auto & iter : *itr->second)
                    if(iter.index==lootSlot)
                    {
                        QuestItem *ffaitem2 = (QuestItem*)&iter;
                        if(ffaitem)
                            *ffaitem = ffaitem2;
                        is_looted = ffaitem2->is_looted;
                        break;
                    }
            }
        }
        else if(item->conditionId)
        {
            QuestItemMap::const_iterator itr = PlayerNonQuestNonFFAConditionalItems.find(player->GetGUID().GetCounter());
            if (itr != PlayerNonQuestNonFFAConditionalItems.end())
            {
                for(auto & iter : *itr->second)
                {
                    if(iter.index==lootSlot)
                    {
                        QuestItem *conditem2 = (QuestItem*)&iter;
                        if(conditem)
                            *conditem = conditem2;
                        is_looted = conditem2->is_looted;
                        break;
                    }
                }
            }
        }
    }

    if(is_looted)
        return nullptr;

    return item;
}

ByteBuffer& operator<<(ByteBuffer& b, LootItem const& li)
{
    b << uint32(li.itemid);
    b << uint32(li.count);                                  // nr of items of this type
    b << uint32(sObjectMgr->GetItemTemplate(li.itemid)->DisplayInfoID);
    b << uint32(li.randomSuffix);
    b << uint32(li.randomPropertyId);
    //b << uint8(0);                                        // slot type - will send after this function call
    return b;
}

ByteBuffer& operator<<(ByteBuffer& b, LootView const& lv)
{
    Loot &l = lv.loot;

    uint8 itemsShown = 0;

    //gold
    b << uint32(lv.permission!=NONE_PERMISSION ? l.gold : 0);

    size_t count_pos = b.wpos();                            // pos of item count byte
    b << uint8(0);                                          // item count placeholder

    switch (lv.permission)
    {
        case GROUP_PERMISSION:
        {
            // You are not the items proprietary, so you can only see
            // blocked rolled items and quest items, and !ffa items
            for (uint8 i = 0; i < l.items.size(); ++i)
            {
                if (!l.items[i].is_looted && !l.items[i].freeforall && !l.items[i].conditionId && l.items[i].AllowedForPlayer(lv.viewer))
                {
                    uint8 slot_type = (l.items[i].is_blocked || l.items[i].is_underthreshold) ? 0 : 1;

                    b << uint8(i) << l.items[i];            //send the index and the item if it's not looted, and blocked or under threshold, free for all items will be sent later, only one-player loots here
                    b << uint8(slot_type);                  // 0 - get 1 - look only
                    ++itemsShown;
                }
            }
            break;
        }
        case ALL_PERMISSION:
        case MASTER_PERMISSION:
        {
            uint8 slot_type = (lv.permission==MASTER_PERMISSION) ? 2 : 0;
            for (uint8 i = 0; i < l.items.size(); ++i)
            {
                if (!l.items[i].is_looted && !l.items[i].freeforall && !l.items[i].conditionId && l.items[i].AllowedForPlayer(lv.viewer))
                {
                    b << uint8(i) << l.items[i];            //only send one-player loot items now, free for all will be sent later
                    b << uint8(slot_type);                  // 0 - get 2 - master selection
                    ++itemsShown;
                }
            }
            break;
        }
        case NONE_PERMISSION:
        default:
            return b;                                       // nothing output more
    }

    if (lv.qlist)
    {
        for (auto qi = lv.qlist->begin() ; qi != lv.qlist->end(); ++qi)
        {
            LootItem &item = l.quest_items[qi->index];
            if (!qi->is_looted && !item.is_looted)
            {
                b << uint8(l.items.size() + (qi - lv.qlist->begin()));
                b << item;
                b << uint8(0);                              // allow loot
                ++itemsShown;
            }
        }
    }

    if (lv.ffalist)
    {
        for (auto & fi : *lv.ffalist)
        {
            LootItem &item = l.items[fi.index];
            if (!fi.is_looted && !item.is_looted)
            {
                b << uint8(fi.index) << item;
                b << uint8(0);                              // allow loot
                ++itemsShown;
            }
        }
    }

    if (lv.conditionallist)
    {
        for (auto & ci : *lv.conditionallist)
        {
            LootItem &item = l.items[ci.index];
            if (!ci.is_looted && !item.is_looted)
            {
                b << uint8(ci.index) << item;
                b << uint8(0);                              // allow loot
                ++itemsShown;
            }
        }
    }

    //update number of items shown
    b.put<uint8>(count_pos,itemsShown);

    return b;
}

//
// --------- LootTemplate::LootGroup ---------
//

// Adds an entry to the group (at loading stage)
void LootTemplate::LootGroup::AddEntry(LootStoreItem* item)
{
    if (item->chance != 0)
        ExplicitlyChanced.push_back(item);
    else
        EqualChanced.push_back(item);
}

// Rolls an item from the group, returns NULL if all miss their chances
LootStoreItem const* LootTemplate::LootGroup::Roll() const
{
    if (!ExplicitlyChanced.empty())                         // First explicitly chanced entries are checked
    {
        float roll = rand_chance();

        for (auto item : ExplicitlyChanced)   // check each explicitly chanced entry in the template and modify its chance based on quality.
        {
            if (item->chance >= 100.0f)
                return item;

            roll -= item->chance;
            if (roll < 0)
                return item;
        }
    }
    if (!EqualChanced.empty())                              // If nothing selected yet - an item is taken from equal-chanced part
        return Trinity::Containers::SelectRandomContainerElement(EqualChanced);

    return nullptr;                                            // Empty drop from the group
}

// True if group includes at least 1 quest drop entry
bool LootTemplate::LootGroup::HasQuestDrop() const
{
    for (auto i : ExplicitlyChanced)
        if (i->needs_quest)
            return true;
    for (auto i : EqualChanced)
        if (i->needs_quest)
            return true;
    return false;
}

// True if group includes at least 1 quest drop entry for active quests of the player
bool LootTemplate::LootGroup::HasQuestDropForPlayer(Player const * player) const
{
    for (auto i : ExplicitlyChanced)
        if (player->HasQuestForItem(i->itemid))
            return true;
    for (auto i : EqualChanced)
        if (player->HasQuestForItem(i->itemid))
            return true;
    return false;
}

void LootTemplate::LootGroup::CopyConditions(ConditionContainer /*conditions*/)
{
    for (auto & i : ExplicitlyChanced)
        i->conditions.clear();

    for (auto & i : EqualChanced)
        i->conditions.clear();
}

// Rolls an item from the group (if any takes its chance) and adds the item to the loot
void LootTemplate::LootGroup::Process(Loot& loot) const
{
    LootStoreItem const * item = Roll();
    if (item != nullptr)
        loot.AddItem(*item);
}

// Overall chance for the group without equal chanced items
float LootTemplate::LootGroup::RawTotalChance() const
{
    float result = 0;

    for (auto i : ExplicitlyChanced)
        if ( !i->needs_quest )
            result += i->chance;

    return result;
}

// Overall chance for the group
float LootTemplate::LootGroup::TotalChance() const
{
    float result = RawTotalChance();

    if (!EqualChanced.empty() && result < 100.0f)
        return 100.0f;

    return result;
}

void LootTemplate::LootGroup::Verify(LootStore const& lootstore, uint32 id, uint32 group_id) const
{
    float chance = RawTotalChance();
    if (chance > 101.0f)                                    // TODO: replace with 100% when DBs will be ready
    {
        TC_LOG_ERROR("sql.sql","Table '%s' entry %u group %d has total chance > 100%% (%f)", lootstore.GetName(), id, group_id, chance);
    }

    if(chance >= 100.0f && !EqualChanced.empty())
    {
        TC_LOG_ERROR("sql.sql", "Table '%s' entry %u group %d has items with chance=0%% but group total chance >= 100%% (%f)", lootstore.GetName(), id, group_id, chance);
    }
}

void LootTemplate::LootGroup::CheckLootRefs(LootTemplateMap const& store, LootIdSet* ref_set) const
{
    for (auto ieItr : ExplicitlyChanced)
    {
        if(ieItr->mincountOrRef < 0)
        {
            if(!LootTemplates_Reference.GetLootFor(-ieItr->mincountOrRef))
                LootTemplates_Reference.ReportNotExistedId(-ieItr->mincountOrRef);
            else if(ref_set)
                ref_set->erase(-ieItr->mincountOrRef);
        }
    }

    for (auto ieItr : EqualChanced)
    {
        if(ieItr->mincountOrRef < 0)
        {
            if(!LootTemplates_Reference.GetLootFor(-ieItr->mincountOrRef))
                LootTemplates_Reference.ReportNotExistedId(-ieItr->mincountOrRef);
            else if(ref_set)
                ref_set->erase(-ieItr->mincountOrRef);
        }
    }
}

//
// --------- LootTemplate ---------
//

LootTemplate::~LootTemplate()
{
    for (auto & Entrie : Entries)
        delete Entrie;

    for (auto & Group : Groups)
        delete Group;
}

// Adds an entry to the group (at loading stage)
void LootTemplate::AddEntry(LootStoreItem* item)
{
    if (item->group > 0 && item->mincountOrRef > 0)           // Group
    {
        if (item->group >= Groups.size())
            Groups.resize(item->group, nullptr);                      // Adds new group the the loot template if needed
        if (!Groups[item->group - 1])
            Groups[item->group - 1] = new LootGroup();

        Groups[item->group-1]->AddEntry(item);                // Adds new entry to the group
    }
    else                                                    // Non-grouped entries and references are stored together
        Entries.push_back(item);
}

// Rolls for every item in the template and adds the rolled items the the loot
void LootTemplate::Process(Loot& loot, LootStore const& store, uint8 groupId) const
{
    if (groupId)                                            // Group reference uses own processing of the group
    {
        if (groupId > Groups.size())
            return;                                         // Error message already printed at loading stage

        Groups[groupId-1]->Process(loot);
        return;
    }

    // Rolling non-grouped items
    for (auto Entrie : Entries)
    {
        if ( !Entrie->Roll() )
            continue;                                       // Bad luck for the entry

        if (Entrie->mincountOrRef < 0)                           // References processing
        {
            LootTemplate const* Referenced = LootTemplates_Reference.GetLootFor(-Entrie->mincountOrRef);

            if(!Referenced)
                continue;                                   // Error message already printed at loading stage

            for (uint32 loop=0; loop < Entrie->maxcount; ++loop )// Ref multiplicator
                Referenced->Process(loot, store, Entrie->group); // Ref processing
        }
        else                                                // Plain entries (not a reference, not grouped)
            loot.AddItem(*Entrie);                               // Chance is already checked, just add
    }

    // Now processing groups
    for (auto group : Groups)
        if (group)
            group->Process(loot);
}

void LootTemplate::CopyConditions(const ConditionContainer& conditions)
{
    for (auto & Entrie : Entries)
        Entrie->conditions.clear();

    for (auto group : Groups)
        if (group)
            group->CopyConditions(conditions);
}

void LootTemplate::CopyConditions(LootItem* li) const
{
    // Copies the conditions list from a template item to a LootItem
    for (auto item : Entries)
    {
        if (item->itemid != li->itemid)
            continue;

        li->conditions = item->conditions;
        break;
    }
}

// True if template includes at least 1 quest drop entry
bool LootTemplate::HasQuestDrop(LootTemplateMap const& store, uint8 groupId) const
{
    if (groupId)                                            // Group reference
    {
        if (groupId > Groups.size())
            return false;                                   // Error message [should be] already printed at loading stage
        return Groups[groupId-1]->HasQuestDrop();
    }

    for (auto Entrie : Entries)
    {
        if (Entrie->mincountOrRef < 0)                           // References
        {
            auto Referenced = store.find(-Entrie->mincountOrRef);
            if( Referenced ==store.end() )
                continue;                                   // Error message [should be] already printed at loading stage
            if (Referenced->second->HasQuestDrop(store, Entrie->group) )
                return true;
        }
        else if ( Entrie->needs_quest )
            return true;                                    // quest drop found
    }

    // Now processing groups
    for (auto group : Groups)
        if (group)
            if (group->HasQuestDrop())
                return true;

    return false;
}

// True if template includes at least 1 quest drop for an active quest of the player
bool LootTemplate::HasQuestDropForPlayer(LootTemplateMap const& store, Player const* player, uint8 groupId) const
{
    if (groupId)                                            // Group reference
    {
        if (groupId > Groups.size())
            return false;                                   // Error message already printed at loading stage
        return Groups[groupId-1]->HasQuestDropForPlayer(player);
    }

    // Checking non-grouped entries
    for (auto Entrie : Entries)
    {
        if (Entrie->mincountOrRef < 0)                           // References processing
        {
            auto Referenced = store.find(-Entrie->mincountOrRef);
            if (Referenced == store.end() )
                continue;                                   // Error message already printed at loading stage
            if (Referenced->second->HasQuestDropForPlayer(store, player, Entrie->group) )
                return true;
        }
        else if ( player->HasQuestForItem(Entrie->itemid) )
            return true;                                    // active quest drop found
    }

    // Now checking groups
    for (auto group : Groups)
        if (group)
            if (group->HasQuestDropForPlayer(player))
                return true;

    return false;
}

// Checks integrity of the template
void LootTemplate::Verify(LootStore const& lootstore, uint32 id) const
{
    // Checking group chances
    for (uint32 i=0; i < Groups.size(); ++i)
        if (Groups[i])
            Groups[i]->Verify(lootstore,id,i+1);

    // TODO: References validity checks
}

void LootTemplate::CheckLootRefs(LootTemplateMap const& store, LootIdSet* ref_set) const
{
    for(auto Entrie : Entries)
    {
        if(Entrie->mincountOrRef < 0)
        {
            if(!LootTemplates_Reference.GetLootFor(-Entrie->mincountOrRef))
                LootTemplates_Reference.ReportNotExistedId(-Entrie->mincountOrRef);
            else if(ref_set)
                ref_set->erase(-Entrie->mincountOrRef);
        }
    }

    for (auto group : Groups)
        if (group)
            group->CheckLootRefs(store,ref_set);
}

bool LootTemplate::addConditionItem(Condition* cond)
{
    if (!cond || !cond->isLoaded())//should never happen, checked at loading
    {
        TC_LOG_ERROR("loot", "LootTemplate::addConditionItem: condition is null");
        return false;
    }

    if (!Entries.empty())
    {
        for (auto & Entrie : Entries)
        {
            if (Entrie->itemid == uint32(cond->SourceEntry))
            {
                Entrie->conditions.push_back(cond);
                return true;
            }
        }
    }

    if (!Groups.empty())
    {
        for (auto group : Groups)
        {
            if (!group)
                continue;

            LootStoreItemList* itemList = group->GetExplicitlyChancedItemList();
            if (!itemList->empty())
            {
                for (auto & i : *itemList)
                {
                    if (i->itemid == uint32(cond->SourceEntry))
                    {
                        i->conditions.push_back(cond);
                        return true;
                    }
                }
            }

            itemList = group->GetEqualChancedItemList();
            if (!itemList->empty())
            {
                for (auto & i : *itemList)
                {
                    if (i->itemid == uint32(cond->SourceEntry))
                    {
                        i->conditions.push_back(cond);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool LootTemplate::isReference(uint32 id)
{
    for (LootStoreItemList::const_iterator ieItr = Entries.begin(); ieItr != Entries.end(); ++ieItr)
        if ((*ieItr)->itemid == id && (*ieItr)->mincountOrRef < 0)
            return true;

    return false;//not found or not reference
}

void LoadLootTemplates_Creature()
{
    LootIdSet ids_set, ids_setUsed;
    LootTemplates_Creature.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot

    CreatureTemplateContainer const* ctc = sObjectMgr->GetCreatureTemplates();
    for (const auto & itr : *ctc)
    {
        if(CreatureTemplate const* cInfo = &(itr.second))
        {
            if(uint32 lootid = cInfo->lootid)
            {
                if(!ids_set.count(lootid))
                    LootTemplates_Creature.ReportNotExistedId(lootid);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for(uint32 itr : ids_setUsed)
        ids_set.erase(itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Creature.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Disenchant()
{
    LootIdSet ids_set, ids_setUsed;
    LootTemplates_Disenchant.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (const auto & it : *its)
    {
        if(ItemTemplate const* proto = &(it.second))
        {
            if(uint32 lootid = proto->DisenchantID)
            {
                if(!ids_set.count(lootid))
                    LootTemplates_Disenchant.ReportNotExistedId(lootid);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for(uint32 itr : ids_setUsed)
        ids_set.erase(itr);
    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Disenchant.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Fishing()
{
    LootIdSet ids_set;
    LootTemplates_Fishing.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    for(uint32 i = 1; i < sAreaTableStore.GetNumRows(); ++i )
    {
        if(AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(i))
            if (ids_set.find(areaEntry->ID) != ids_set.end())
                ids_set.erase(areaEntry->ID);
    }

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Fishing.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Gameobject()
{
    LootIdSet ids_set, ids_setUsed;
    LootTemplates_Gameobject.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    
    GameObjectTemplateContainer const* gotc = sObjectMgr->GetGameObjectTemplateStore();
    for (const auto & itr : *gotc)
    {
        if(GameObjectTemplate const* gInfo = &(itr.second))
        {
            if(uint32 lootid = gInfo->GetLootId())
            {
                if(!ids_set.count(lootid))
                    LootTemplates_Gameobject.ReportNotExistedId(lootid);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for(uint32 itr : ids_setUsed)
        ids_set.erase(itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Gameobject.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Item()
{
    LootIdSet ids_set;
    LootTemplates_Item.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (const auto & it : *its)
        if(ItemTemplate const* proto = &(it.second))
            if(ids_set.count(proto->ItemId))
                ids_set.erase(proto->ItemId);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Item.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Pickpocketing()
{
    LootIdSet ids_set, ids_setUsed;
    LootTemplates_Pickpocketing.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    CreatureTemplateContainer const* ctc = sObjectMgr->GetCreatureTemplates();
    for (const auto & itr : *ctc)
    {
        if(CreatureTemplate const* cInfo = &(itr.second))
        {
            if(uint32 lootid = cInfo->pickpocketLootId)
            {
                if(!ids_set.count(lootid))
                    LootTemplates_Pickpocketing.ReportNotExistedId(lootid);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for(uint32 itr : ids_setUsed)
        ids_set.erase(itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Pickpocketing.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Prospecting()
{
    LootIdSet ids_set;
    LootTemplates_Prospecting.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    ItemTemplateContainer const* its = sObjectMgr->GetItemTemplateStore();
    for (const auto & it : *its)
        if(ItemTemplate const* proto = &(it.second))
            if(ids_set.count(proto->ItemId))
                ids_set.erase(proto->ItemId);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Prospecting.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_QuestMail()
{
    LootIdSet ids_set;
    LootTemplates_QuestMail.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    ObjectMgr::QuestMap const& questMap = sObjectMgr->GetQuestTemplates();
    for(const auto & itr : questMap)
        if(ids_set.count(itr.first))
            ids_set.erase(itr.first);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_QuestMail.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Skinning()
{
    LootIdSet ids_set, ids_setUsed;
    LootTemplates_Skinning.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    CreatureTemplateContainer const* ctc = sObjectMgr->GetCreatureTemplates();
    for (const auto & itr : *ctc)
    {
        if(CreatureTemplate const* cInfo = &(itr.second))
        {
            if(uint32 lootid = cInfo->SkinLootId)
            {
                if(!ids_set.count(lootid))
                    LootTemplates_Skinning.ReportNotExistedId(lootid);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for(uint32 itr : ids_setUsed)
        ids_set.erase(itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Skinning.ReportUnusedIds(ids_set);
}

void LoadLootTemplates_Reference()
{
    LootIdSet ids_set;
    LootTemplates_Reference.LoadAndCollectLootIds(ids_set);

    // check references and remove used
    LootTemplates_Creature.CheckLootRefs(&ids_set);
    LootTemplates_Fishing.CheckLootRefs(&ids_set);
    LootTemplates_Gameobject.CheckLootRefs(&ids_set);
    LootTemplates_Item.CheckLootRefs(&ids_set);
    LootTemplates_Pickpocketing.CheckLootRefs(&ids_set);
    LootTemplates_Skinning.CheckLootRefs(&ids_set);
    LootTemplates_Disenchant.CheckLootRefs(&ids_set);
    LootTemplates_Prospecting.CheckLootRefs(&ids_set);
    LootTemplates_QuestMail.CheckLootRefs(&ids_set);
    LootTemplates_Reference.CheckLootRefs(&ids_set);

    // output error for any still listed ids (not referenced from any loot table)
    LootTemplates_Reference.ReportUnusedIds(ids_set);
}

