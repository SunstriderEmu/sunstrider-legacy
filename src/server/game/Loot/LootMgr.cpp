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
#include "Loot.h"
#include <functional>

static Rates const qualityToRate[MAX_ITEM_QUALITY] = {
    RATE_DROP_ITEM_POOR,                                    // ITEM_QUALITY_POOR
    RATE_DROP_ITEM_NORMAL,                                  // ITEM_QUALITY_NORMAL
    RATE_DROP_ITEM_UNCOMMON,                                // ITEM_QUALITY_UNCOMMON
    RATE_DROP_ITEM_RARE,                                    // ITEM_QUALITY_RARE
    RATE_DROP_ITEM_EPIC,                                    // ITEM_QUALITY_EPIC
    RATE_DROP_ITEM_LEGENDARY,                               // ITEM_QUALITY_LEGENDARY
    RATE_DROP_ITEM_ARTIFACT,                                // ITEM_QUALITY_ARTIFACT
};

LootStore LootTemplates_Creature(     "creature_loot_template",     "creature entry",             true);
LootStore LootTemplates_Disenchant(   "disenchant_loot_template",   "item disenchant id",         true);
LootStore LootTemplates_Fishing(      "fishing_loot_template",      "area id",                    true);
LootStore LootTemplates_Gameobject(   "gameobject_loot_template",   "gameobject entry",           true);
LootStore LootTemplates_Item(         "item_loot_template",         "item entry",                 true);
LootStore LootTemplates_Pickpocketing("pickpocketing_loot_template","creature pickpocket lootid", true);
LootStore LootTemplates_Prospecting(  "prospecting_loot_template",  "item entry",                 true);
LootStore LootTemplates_QuestMail(    "quest_mail_loot_template",   "quest id",                   true);
LootStore LootTemplates_Reference(    "reference_loot_template",    "reference id",               false);
LootStore LootTemplates_Skinning(     "skinning_loot_template",     "creature skinning id",       true);

// Selects invalid loot items to be removed from group possible entries (before rolling)
struct LootGroupInvalidSelector
{
    explicit LootGroupInvalidSelector(Loot const& loot, uint16 lootMode) : _loot(loot), _lootMode(lootMode) { }

    bool operator()(LootStoreItem* item) const
    {
        if (!(item->lootmode & _lootMode))
            return true;

        uint8 foundDuplicates = 0;
        for (std::vector<LootItem>::const_iterator itr = _loot.items.begin(); itr != _loot.items.end(); ++itr)
            if (itr->itemid == item->itemid)
                if (++foundDuplicates == _loot.maxDuplicates)
                    return true;

        return false;
    }

private:
    Loot const& _loot;
    uint16 _lootMode;
};

class LootTemplate::LootGroup                               // A set of loot definitions for items (refs are not allowed)
{
    public:
        LootGroup() { }
        ~LootGroup();

        void AddEntry(LootStoreItem* item);                 // Adds an entry to the group (at loading stage)
        bool HasQuestDrop() const;                          // True if group includes at least 1 quest drop entry
        bool HasQuestDropForPlayer(Player const * player) const;
                                                            // The same for active quests of the player
        void Process(Loot& loot, uint16 lootMode) const;    // Rolls an item from the group (if any) and adds the item to the loot
        float RawTotalChance() const;                       // Overall chance for the group (without equal chanced items)
        float TotalChance() const;                          // Overall chance for the group

        void Verify(LootStore const& lootstore, uint32 id, uint32 group_id) const;
        void CheckLootRefs(LootTemplateMap const& store, LootIdSet* ref_set) const;
        LootStoreItemList* GetExplicitlyChancedItemList() { return &ExplicitlyChanced; }
        LootStoreItemList* GetEqualChancedItemList() { return &EqualChanced; }
        void CopyConditions(ConditionContainer conditions);
    private:
        LootStoreItemList ExplicitlyChanced;                // Entries with chances defined in DB
        LootStoreItemList EqualChanced;                     // Zero chances - every entry takes the same chance

        LootStoreItem const* Roll(Loot& loot, uint16 lootMode) const;   // Rolls an item from the group, returns NULL if all miss their chances

        // This class must never be copied - storing pointers
        LootGroup(LootGroup const&) = delete;
        LootGroup& operator=(LootGroup const&) = delete;
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
uint32 LootStore::LoadLootTable()
{
    LootTemplateMap::const_iterator tab;
    uint32 count = 0;

    // Clearing store (for reloading case)
    Clear();

    TC_LOG_INFO("server.loading", "%s :", GetName());

    //                                                 0     1     2          3       4              5         6        7         8        
    QueryResult result = WorldDatabase.PQuery("SELECT Entry, Item, Reference, Chance, QuestRequired, LootMode, GroupId, MinCount, MaxCount FROM %s", GetName());

    if(!result)
        return 0;

    do
    {
        Field *fields = result->Fetch();

        uint32 entry               = fields[0].GetUInt32();
        uint32 item                = fields[1].GetUInt32();
        uint32 reference           = fields[2].GetUInt32();
        float  chance              = fields[3].GetFloat();
        bool   needsquest          = fields[4].GetBool();
        uint16 lootmode            = fields[5].GetUInt16();
        uint8  groupid             = fields[6].GetUInt8();
        uint8  mincount            = fields[7].GetUInt8();
        uint8  maxcount            = fields[8].GetUInt8();

        LootStoreItem* storeitem = new LootStoreItem(item, reference, chance, needsquest, lootmode, groupid, mincount, maxcount);

        if (!storeitem->IsValid(*this, entry))            // Validity checks
        {
            delete storeitem;
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

    return count;
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

void LootStore::ResetConditions()
{
    for (LootTemplateMap::iterator itr = m_LootTemplates.begin(); itr != m_LootTemplates.end(); ++itr)
    {
        ConditionContainer empty;
        itr->second->CopyConditions(empty);
    }
}

LootTemplate const* LootStore::GetLootFor(uint32 loot_id) const
{
    auto tab = m_LootTemplates.find(loot_id);

    if (tab == m_LootTemplates.end())
        return nullptr;

    return tab->second;
}

LootTemplate* LootStore::GetLootForConditionFill(uint32 loot_id)
{
    auto tab = m_LootTemplates.find(loot_id);

    if (tab == m_LootTemplates.end())
        return nullptr;

    return tab->second;
}

uint32 LootStore::LoadAndCollectLootIds(LootIdSet& ids_set)
{
    uint32 count = LoadLootTable();

    for(LootTemplateMap::const_iterator tab = m_LootTemplates.begin(); tab != m_LootTemplates.end(); ++tab)
        ids_set.insert(tab->first);

    return count;
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
        TC_LOG_ERROR("sql.sql", "Table '%s' entry %d isn't %s and not referenced from loot, and then useless.", GetName(), itr, GetEntryName());
}

void LootStore::ReportNonExistingId(uint32 lootId, char const* ownerType, uint32 ownerId) const
{
    TC_LOG_ERROR("sql.sql", "Table '%s' Entry %d does not exist but it is used by %s %d", GetName(), lootId, ownerType, ownerId);
}

//
// --------- LootStoreItem ---------
//

// Checks if the entry (quest, non-quest, reference) takes it's chance (at loot generation)
// RATE_DROP_ITEMS is no longer used for all types of entries
bool LootStoreItem::Roll(bool rate) const
{
    if (chance >= 100.0f)
        return true;

    if (reference > 0)                                   // reference case
        return roll_chance_f(chance* (rate ? sWorld->GetRate(RATE_DROP_ITEM_REFERENCED) : 1.0f));

    ItemTemplate const* pProto = sObjectMgr->GetItemTemplate(itemid);

    float qualityModifier = pProto ? sWorld->GetRate(qualityToRate[pProto->Quality]) : 1.0f;

    return roll_chance_f(chance*qualityModifier);
}

// Checks correctness of values
bool LootStoreItem::IsValid(LootStore const& store, uint32 entry) const
{
    if (mincount == 0)
    {
        TC_LOG_ERROR("sql.sql","Table '%s' entry %d item %d: wrong MinCount (%d) - skipped", store.GetName(), entry, itemid, mincount);
        return false;
    }

    if (reference == 0)                                 // item (quest or non-quest) entry, maybe grouped
    {
        ItemTemplate const *proto = sObjectMgr->GetItemTemplate(itemid);
        if(!proto)
        {
            TC_LOG_ERROR("sql.sql","Table '%s' entry %d item %d: item entry not listed in `item_template` - skipped", store.GetName(), entry, itemid);
            return false;
        }

        if( chance == 0 && groupid == 0)                      // Zero chance is allowed for grouped entries only
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

        if (maxcount < mincount)                       // wrong max count
        {
            TC_LOG_ERROR("sql.sql", "Table '%s' Entry %d Item %d: MaxCount (%u) less that MinCount (%i) - skipped", store.GetName(), entry, itemid, int32(maxcount), mincount);
            return false;
        }
    }
    else                                                    // mincountOrRef < 0
    {
        if (needs_quest)
            TC_LOG_ERROR("sql.sql","Table '%s' entry %d item %d: quest chance will be treated as non-quest chance", store.GetName(), entry, itemid);
        else if (chance == 0)                              // no chance for the reference
        {
            TC_LOG_ERROR("sql.sql","Table '%s' entry %d item %d: zero chance is specified for a reference, skipped", store.GetName(), entry, itemid);
            return false;
        }
    }
    return true;                                            // Referenced template existence is checked at whole store level
}

//
// --------- LootTemplate::LootGroup ---------
//

LootTemplate::LootGroup::~LootGroup()
{
    while (!ExplicitlyChanced.empty())
    {
        delete ExplicitlyChanced.back();
        ExplicitlyChanced.pop_back();
    }

    while (!EqualChanced.empty())
    {
        delete EqualChanced.back();
        EqualChanced.pop_back();
    }
}

// Adds an entry to the group (at loading stage)
void LootTemplate::LootGroup::AddEntry(LootStoreItem* item)
{
    if (item->chance != 0)
        ExplicitlyChanced.push_back(item);
    else
        EqualChanced.push_back(item);
}

// Rolls an item from the group, returns NULL if all miss their chances
LootStoreItem const* LootTemplate::LootGroup::Roll(Loot& loot, uint16 lootMode) const
{
    LootStoreItemList possibleLoot = ExplicitlyChanced;
    possibleLoot.remove_if(LootGroupInvalidSelector(loot, lootMode));

    if (!possibleLoot.empty())                             // First explicitly chanced entries are checked
    {
        float roll = (float)rand_chance();

        for (LootStoreItemList::const_iterator itr = possibleLoot.begin(); itr != possibleLoot.end(); ++itr)   // check each explicitly chanced entry in the template and modify its chance based on quality.
        {
            LootStoreItem* item = *itr;
            if (item->chance >= 100.0f)
                return item;

            roll -= item->chance;
            if (roll < 0)
                return item;
        }
    }

    possibleLoot = EqualChanced;
    possibleLoot.remove_if(LootGroupInvalidSelector(loot, lootMode));
    if (!possibleLoot.empty())                              // If nothing selected yet - an item is taken from equal-chanced part
        return Trinity::Containers::SelectRandomContainerElement(possibleLoot);

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
void LootTemplate::LootGroup::Process(Loot& loot, uint16 lootMode) const
{
    if (LootStoreItem const * item = Roll(loot, lootMode))
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
        TC_LOG_ERROR("sql.sql","Table '%s' entry %u group %d has total chance > 100%% (%f)", lootstore.GetName(), id, group_id, chance);

    if(chance >= 100.0f && !EqualChanced.empty())
        TC_LOG_ERROR("sql.sql", "Table '%s' entry %u group %d has items with chance=0%% but group total chance >= 100%% (%f)", lootstore.GetName(), id, group_id, chance);
}

void LootTemplate::LootGroup::CheckLootRefs(LootTemplateMap const& store, LootIdSet* ref_set) const
{
    for (auto item : ExplicitlyChanced)
    {
        if (item->reference > 0)
        {
            if (!LootTemplates_Reference.GetLootFor(item->reference))
                LootTemplates_Reference.ReportNonExistingId(item->reference, "Reference", item->itemid);
            else if (ref_set)
                ref_set->erase(item->reference);
        }
    }

    for (auto item : EqualChanced)
    {
        if (item->reference > 0)
        {
            if (!LootTemplates_Reference.GetLootFor(item->reference))
                LootTemplates_Reference.ReportNonExistingId(item->reference, "Reference", item->itemid);
            else if (ref_set)
                ref_set->erase(item->reference);
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
    if (item->groupid > 0 && item->reference == 0)            // Group
    {
        if (item->groupid >= Groups.size())
            Groups.resize(item->groupid, nullptr);               // Adds new group the the loot template if needed
        if (!Groups[item->groupid - 1])
            Groups[item->groupid - 1] = new LootGroup();

        Groups[item->groupid - 1]->AddEntry(item);            // Adds new entry to the group
    }
    else                                                      // Non-grouped entries and references are stored together
        Entries.push_back(item);
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

// Rolls for every item in the template and adds the rolled items the the loot
void LootTemplate::Process(Loot& loot, bool rate, uint16 lootMode, uint8 groupId) const
{
    if (groupId)                                            // Group reference uses own processing of the group
    {
        if (groupId > Groups.size())
            return;                                         // Error message already printed at loading stage

        if (!Groups[groupId - 1])
            return;

        Groups[groupId - 1]->Process(loot, lootMode);
        return;
    }

    // Rolling non-grouped items
    for (LootStoreItemList::const_iterator i = Entries.begin(); i != Entries.end(); ++i)
    {
        LootStoreItem* item = *i;
        if (!(item->lootmode & lootMode))                       // Do not add if mode mismatch
            continue;

        if (!item->Roll(rate))
            continue;                                           // Bad luck for the entry

        if (item->reference > 0)                            // References processing
        {
            LootTemplate const* Referenced = LootTemplates_Reference.GetLootFor(item->reference);
            if (!Referenced)
                continue;                                       // Error message already printed at loading stage

            uint32 maxcount = uint32(float(item->maxcount) /** sWorld->GetRate(RATE_DROP_ITEM_REFERENCED_AMOUNT)*/);
            for (uint32 loop = 0; loop < maxcount; ++loop)      // Ref multiplicator
                Referenced->Process(loot, rate, lootMode, item->groupid);
        }
        else                                                    // Plain entries (not a reference, not grouped)
            loot.AddItem(*item);                                // Chance is already checked, just add
    }

    // Now processing groups
    for (auto group : Groups)
        if (group)
            group->Process(loot, lootMode);
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

    for (auto item : Entries)
    {
        if (item->reference > 0)                           // References
        {
            auto Referenced = store.find(item->reference);
            if (Referenced == store.end())
                continue;                                   // Error message [should be] already printed at loading stage
            if (Referenced->second->HasQuestDrop(store, item->groupid))
                return true;
        }
        else if (item->needs_quest)
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

        if (!Groups[groupId - 1])
            return false;

        return Groups[groupId-1]->HasQuestDropForPlayer(player);
    }

    // Checking non-grouped entries
    for (auto item : Entries)
    {
        if (item->reference > 0)                           // References processing
        {
            auto Referenced = store.find(item->reference);
            if (Referenced == store.end() )
                continue;                                   // Error message already printed at loading stage
            if (Referenced->second->HasQuestDropForPlayer(store, player, item->groupid) )
                return true;
        }
        else if ( player->HasQuestForItem(item->itemid) )
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
    for(auto item : Entries)
    {
        if(item->reference > 0)
        {
            if(!LootTemplates_Reference.GetLootFor(item->reference))
                LootTemplates_Reference.ReportNonExistingId(item->reference, "Reference", item->itemid);
            else if(ref_set)
                ref_set->erase(item->reference);
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
        if ((*ieItr)->itemid == id && (*ieItr)->reference > 0)
            return true;

    return false;//not found or not reference
}

void LoadLootTemplates_Creature()
{
    TC_LOG_INFO("server.loading", "Loading creature loot templates...");
    uint32 oldMSTime = GetMSTime();

    LootIdSet ids_set, ids_setUsed;
    uint32 count = LootTemplates_Creature.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot

    CreatureTemplateContainer const& ctc = sObjectMgr->GetCreatureTemplates();
    for (const auto & itr : ctc)
    {
        if(CreatureTemplate const* cInfo = &(itr.second))
        {
            if(uint32 lootid = cInfo->lootid)
            {
                if(!ids_set.count(lootid))
                    LootTemplates_Creature.ReportNonExistingId(lootid, "Creature", itr.first);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for(uint32 itr : ids_setUsed)
        ids_set.erase(itr);

    // 1 means loot for player corpse
    ids_set.erase(PLAYER_CORPSE_LOOT_ENTRY);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Creature.ReportUnusedIds(ids_set);

    if (count)
        TC_LOG_INFO("server.loading", ">> Loaded %u creature loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO("server.loading", ">> Loaded 0 creature loot templates. DB table `creature_loot_template` is empty");
}

void LoadLootTemplates_Disenchant()
{
    TC_LOG_INFO("server.loading", "Loading disenchanting loot templates...");

    uint32 oldMSTime = GetMSTime();

    LootIdSet ids_set, ids_setUsed;
    uint32 count = LootTemplates_Disenchant.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    ItemTemplateContainer const& its = sObjectMgr->GetItemTemplateStore();
    for (const auto & it : its)
    {
        if(ItemTemplate const* proto = &(it.second))
        {
            if(uint32 lootid = proto->DisenchantID)
            {
                if(!ids_set.count(lootid))
                    LootTemplates_Disenchant.ReportNonExistingId(lootid, "Item", it.first);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for(uint32 itr : ids_setUsed)
        ids_set.erase(itr);
    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Disenchant.ReportUnusedIds(ids_set);
    if (count)
        TC_LOG_INFO("server.loading", ">> Loaded %u disenchanting loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO("server.loading", ">> Loaded 0 disenchanting loot templates. DB table `disenchant_loot_template` is empty");
}

void LoadLootTemplates_Fishing()
{
    TC_LOG_INFO("server.loading", "Loading fishing loot templates...");

    uint32 oldMSTime = GetMSTime();

    LootIdSet ids_set;
    uint32 count = LootTemplates_Fishing.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    for(uint32 i = 1; i < sAreaTableStore.GetNumRows(); ++i )
    {
        if(AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(i))
            if (ids_set.find(areaEntry->ID) != ids_set.end())
                ids_set.erase(areaEntry->ID);
    }

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Fishing.ReportUnusedIds(ids_set);

    if (count)
        TC_LOG_INFO("server.loading", ">> Loaded %u fishing loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO("server.loading", ">> Loaded 0 fishing loot templates. DB table `fishing_loot_template` is empty");
}

void LoadLootTemplates_Gameobject()
{
    TC_LOG_INFO("server.loading", "Loading gameobject loot templates...");

    uint32 oldMSTime = GetMSTime();

    LootIdSet ids_set, ids_setUsed;
    uint32 count = LootTemplates_Gameobject.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    
    GameObjectTemplateContainer const* gotc = sObjectMgr->GetGameObjectTemplateStore();
    for (const auto & itr : *gotc)
    {
        if(GameObjectTemplate const* gInfo = &(itr.second))
        {
            if(uint32 lootid = gInfo->GetLootId())
            {
                if(!ids_set.count(lootid))
                    LootTemplates_Gameobject.ReportNonExistingId(lootid, "Gameobject", itr.first);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for(uint32 itr : ids_setUsed)
        ids_set.erase(itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Gameobject.ReportUnusedIds(ids_set);

    if (count)
        TC_LOG_INFO("server.loading", ">> Loaded %u gameobject loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO("server.loading", ">> Loaded 0 gameobject loot templates. DB table `gameobject_loot_template` is empty");
}

void LoadLootTemplates_Item()
{
    TC_LOG_INFO("server.loading", "Loading item loot templates...");
    uint32 oldMSTime = GetMSTime();

    LootIdSet ids_set;
    uint32 count = LootTemplates_Item.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    ItemTemplateContainer const& its = sObjectMgr->GetItemTemplateStore();
    for (const auto & it : its)
        if (ItemTemplate const* proto = &(it.second))
            if (ids_set.count(it.first) > 0 && (it.second.Flags & ITEM_FLAG_HAS_LOOT))
                ids_set.erase(proto->ItemId);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Item.ReportUnusedIds(ids_set);

    if (count)
        TC_LOG_INFO("server.loading", ">> Loaded %u item loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO("server.loading", ">> Loaded 0 item loot templates. DB table `item_loot_template` is empty");
}

void LoadLootTemplates_Pickpocketing()
{
    TC_LOG_INFO("server.loading", "Loading pickpocketing loot templates...");
    uint32 oldMSTime = GetMSTime();

    LootIdSet ids_set, ids_setUsed;
    uint32 count = LootTemplates_Pickpocketing.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    CreatureTemplateContainer const& ctc = sObjectMgr->GetCreatureTemplates();
    for (const auto & itr : ctc)
    {
        if(CreatureTemplate const* cInfo = &(itr.second))
        {
            if(uint32 lootid = cInfo->pickpocketLootId)
            {
                if(!ids_set.count(lootid))
                    LootTemplates_Pickpocketing.ReportNonExistingId(lootid, "Creature", itr.first);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for(uint32 itr : ids_setUsed)
        ids_set.erase(itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Pickpocketing.ReportUnusedIds(ids_set);

    if (count)
        TC_LOG_INFO("server.loading", ">> Loaded %u pickpocketing loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO("server.loading", ">> Loaded 0 pickpocketing loot templates. DB table `pickpocketing_loot_template` is empty");
}

void LoadLootTemplates_Prospecting()
{
    TC_LOG_INFO("server.loading", "Loading prospecting loot templates...");
    uint32 oldMSTime = GetMSTime();

    LootIdSet ids_set;
    uint32 count = LootTemplates_Prospecting.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    ItemTemplateContainer const& its = sObjectMgr->GetItemTemplateStore();
    for (auto const& itemTemplatePair : its)
    {
        if (!(itemTemplatePair.second.Flags & ITEM_FLAG_IS_PROSPECTABLE))
            continue;

        if (ids_set.count(itemTemplatePair.first) > 0)
            ids_set.erase(itemTemplatePair.first);
    };

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Prospecting.ReportUnusedIds(ids_set);

    if (count)
        TC_LOG_INFO("server.loading", ">> Loaded %u prospecting loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO("server.loading", ">> Loaded 0 prospecting loot templates. DB table `prospecting_loot_template` is empty");
}

void LoadLootTemplates_QuestMail()
{
    TC_LOG_INFO("server.loading", "Loading mail loot templates...");
    uint32 oldMSTime = GetMSTime();

    LootIdSet ids_set;
    uint32 count = LootTemplates_QuestMail.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    ObjectMgr::QuestContainer const& questMap = sObjectMgr->GetQuestTemplates();
    for(const auto & itr : questMap)
        if(ids_set.count(itr.first))
            ids_set.erase(itr.first);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_QuestMail.ReportUnusedIds(ids_set);

    if (count)
        TC_LOG_INFO("server.loading", ">> Loaded %u mail loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO("server.loading", ">> Loaded 0 mail loot templates. DB table `mail_loot_template` is empty");
}

void LoadLootTemplates_Skinning()
{
    TC_LOG_INFO("server.loading", "Loading skinning loot templates...");
    uint32 oldMSTime = GetMSTime();

    LootIdSet ids_set, ids_setUsed;
    uint32 count = LootTemplates_Skinning.LoadAndCollectLootIds(ids_set);

    // remove real entries and check existence loot
    CreatureTemplateContainer const& ctc = sObjectMgr->GetCreatureTemplates();
    for (const auto & itr : ctc)
    {
        if(CreatureTemplate const* cInfo = &(itr.second))
        {
            if(uint32 lootid = cInfo->SkinLootId)
            {
                if(!ids_set.count(lootid))
                    LootTemplates_Skinning.ReportNonExistingId(lootid, "Creature", itr.first);
                else
                    ids_setUsed.insert(lootid);
            }
        }
    }
    for(uint32 itr : ids_setUsed)
        ids_set.erase(itr);

    // output error for any still listed (not referenced from appropriate table) ids
    LootTemplates_Skinning.ReportUnusedIds(ids_set);

    if (count)
        TC_LOG_INFO("server.loading", ">> Loaded %u skinning loot templates in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
    else
        TC_LOG_INFO("server.loading", ">> Loaded 0 skinning loot templates. DB table `skinning_loot_template` is empty");
}

void LoadLootTemplates_Reference()
{
    TC_LOG_INFO("server.loading", "Loading reference loot templates...");
    uint32 oldMSTime = GetMSTime();

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
    TC_LOG_INFO("server.loading", ">> Loaded refence loot templates in %u ms", GetMSTimeDiffToNow(oldMSTime));
}

void LoadLootTables()
{
    LoadLootTemplates_Creature();
    LoadLootTemplates_Fishing();
    LoadLootTemplates_Gameobject();
    LoadLootTemplates_Item();
    LoadLootTemplates_Pickpocketing();
    LoadLootTemplates_Skinning();
    LoadLootTemplates_Disenchant();
    LoadLootTemplates_Prospecting();
    LoadLootTemplates_QuestMail();
    LoadLootTemplates_Reference();
}