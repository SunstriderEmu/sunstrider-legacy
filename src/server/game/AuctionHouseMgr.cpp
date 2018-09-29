
#include "Common.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"

#include "AccountMgr.h"
#include "AuctionHouseMgr.h"
#include "Item.h"
#include "Language.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "GameTime.h"
#include "Player.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "LogsDatabaseAccessor.h"
#include "Mail.h"
#include "Bag.h"
#include "CharacterCache.h"

AuctionHouseMgr::AuctionHouseMgr()
{
}

AuctionHouseMgr::~AuctionHouseMgr()
{
    for(auto & mAitem : mAitems)
        delete mAitem.second;
}

AuctionHouseObject * AuctionHouseMgr::GetAuctionsMap( uint32 factionTemplateId )
{
    if(sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
        return &mNeutralAuctions;

    // team have linked auction houses
    FactionTemplateEntry const* u_entry = sFactionTemplateStore.LookupEntry(factionTemplateId);
    if(!u_entry)
        return &mNeutralAuctions;
    else if(u_entry->ourMask & FACTION_MASK_ALLIANCE)
        return &mAllianceAuctions;
    else if(u_entry->ourMask & FACTION_MASK_HORDE)
        return &mHordeAuctions;
    else
        return &mNeutralAuctions;
}

uint32 AuctionHouseMgr::GetAuctionDeposit(AuctionHouseEntry const* entry, uint32 time, Item *pItem)
{
    uint32 MSV = pItem->GetTemplate()->SellPrice;
    double deposit;
    double faction_pct;
    if (MSV > 0)
    {
        if(sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
            faction_pct = (0.15 * (double)sWorld->GetRate(RATE_AUCTION_DEPOSIT));
        else
        {
            FactionTemplateEntry const* u_entry = sFactionTemplateStore.LookupEntry(entry->houseId);
            if(!u_entry)
                faction_pct = (0.75 * (double)sWorld->GetRate(RATE_AUCTION_DEPOSIT));
            else if(u_entry->ourMask & FACTION_MASK_ALLIANCE)
                faction_pct = (0.15 * (double)sWorld->GetRate(RATE_AUCTION_DEPOSIT));
            else if(u_entry->ourMask & FACTION_MASK_HORDE)
                faction_pct = (0.15 * (double)sWorld->GetRate(RATE_AUCTION_DEPOSIT));
            else
                faction_pct = (0.75 * (double)sWorld->GetRate(RATE_AUCTION_DEPOSIT));
        }
        deposit = ((double)MSV * faction_pct * (double)pItem->GetCount()) * (double)(time / MIN_AUCTION_TIME );
    }
    else
    {
        //faction_pct = 0.0f;
        deposit = 0.0f;
    }

    if (deposit > 0)
        return (uint32)deposit;
    else
        return 0;
}

//does not clear ram
void AuctionHouseMgr::SendAuctionWonMail(AuctionEntry* auction, SQLTransaction& trans)
{
    Item *pItem = GetAItem(auction->itemGUIDLow);
    if(!pItem)
        return;

    ObjectGuid bidderGuid(HighGuid::Player, auction->bidder);
    Player* bidder = ObjectAccessor::FindConnectedPlayer(bidderGuid);

    uint32 bidder_accId = 0;
    if (!bidder)
        bidder_accId = sCharacterCache->GetCharacterAccountIdByGuid(bidderGuid);
    else
        bidder_accId = bidder->GetSession()->GetAccountId();

    // data for logging
    {
        ObjectGuid ownerGuid = ObjectGuid(HighGuid::Player, auction->owner);
        uint32 owner_accid = sCharacterCache->GetCharacterAccountIdByGuid(ownerGuid);

        LogsDatabaseAccessor::WonAuction(bidder_accId, auction->bidder, owner_accid, auction->owner, auction->itemGUIDLow, auction->itemEntry, pItem->GetCount());
    }

    
    // receiver exist
    if(bidder || bidder_accId)
    {
        // set owner to bidder (to prevent delete item with sender char deleting)
        // owner in `data` will set at mail receive and item extracting
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ITEM_OWNER);
        stmt->setUInt32(0, auction->bidder);
        stmt->setUInt32(1, pItem->GetGUID().GetCounter());
        trans->Append(stmt);

        if (bidder)
        {
            bidder->GetSession()->SendAuctionBidderNotification(auction->GetHouseId(), auction->Id, bidderGuid, 0, 0, auction->itemEntry);
#ifdef LICH_KING
            // FIXME: for offline player need also
            bidder->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS, 1);
#endif
        }

        MailDraft(auction->BuildAuctionMailSubject(AUCTION_WON), AuctionEntry::BuildAuctionMailBody(auction->owner, auction->bid, auction->buyout, 0, 0))
            .AddItem(pItem)
            .SendMailTo(trans, MailReceiver(bidder, auction->bidder), auction, MAIL_CHECK_MASK_COPIED);
    }
    // receiver not exist
    else
    {
        // bidder doesn't exist, delete the item
        sAuctionMgr->RemoveAItem(auction->itemGUIDLow, true, &trans);
    }
}

void AuctionHouseMgr::SendAuctionSalePendingMail(AuctionEntry * auction, SQLTransaction& trans)
{
    ObjectGuid owner_guid(HighGuid::Player, auction->owner);
    Player *owner = ObjectAccessor::FindConnectedPlayer(owner_guid);
    uint32 owner_accId = sCharacterCache->GetCharacterAccountIdByGuid(owner_guid);

    // owner exist (online or offline)
    if(owner || owner_accId)
    {
        MailDraft(auction->BuildAuctionMailSubject(AUCTION_SALE_PENDING), AuctionEntry::BuildAuctionMailBody(auction->bidder, auction->bid, auction->buyout, auction->deposit, auction->GetAuctionCut(), true))
            .SendMailTo(trans, MailReceiver(owner, auction->owner), auction, MAIL_CHECK_MASK_COPIED);
    }
}

//call this method to send mail to auction owner, when auction is successful, it does not clear ram
void AuctionHouseMgr::SendAuctionSuccessfulMail(AuctionEntry * auction, SQLTransaction& trans)
{
    ObjectGuid owner_guid(HighGuid::Player, auction->owner);
    Player* owner = ObjectAccessor::FindConnectedPlayer(owner_guid);
    uint32 owner_accId = sCharacterCache->GetCharacterAccountIdByGuid(owner_guid);

    // owner exist
    if(owner || owner_accId)
    {
        uint32 profit = auction->bid + auction->deposit - auction->GetAuctionCut();

        //FIXME: what do if owner offline
        if (owner)
        {
#ifdef LICH_KING
            owner->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS, profit);
            owner->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD, auction->bid);
#endif
            //send auction owner notification, bidder must be current!
            owner->GetSession()->SendAuctionOwnerNotification(auction);
        }

        MailDraft(auction->BuildAuctionMailSubject(AUCTION_SUCCESSFUL), AuctionEntry::BuildAuctionMailBody(auction->bidder, auction->bid, auction->buyout, auction->deposit, auction->GetAuctionCut()))
            .AddMoney(profit)
            .SendMailTo(trans, MailReceiver(owner, auction->owner), auction, MAIL_CHECK_MASK_COPIED, sWorld->getIntConfig(CONFIG_MAIL_DELIVERY_DELAY));
    }
    else
        TC_LOG_ERROR("auctionHouse","SendAuctionSuccessfulMail: Mail not sent for some reason to player %s (GUID %u, account %u).", owner ? owner->GetName().c_str() : "<unknown> (maybe offline)", owner ? owner->GetGUID().GetCounter() : 0, owner_accId);
}

//does not clear ram
void AuctionHouseMgr::SendAuctionExpiredMail(AuctionEntry * auction, SQLTransaction& trans)
{ //return an item in auction to its owner by mail
    Item *pItem = GetAItem(auction->itemGUIDLow);
    if(!pItem)
    {
        TC_LOG_ERROR("auctionHouse","Auction item (GUID: %u) not found, and lost.",auction->itemGUIDLow);
        return;
    }

    ObjectGuid owner_guid(HighGuid::Player, auction->owner);
    Player* owner = ObjectAccessor::FindConnectedPlayer(owner_guid);
    uint32 owner_accId = sCharacterCache->GetCharacterAccountIdByGuid(owner_guid);

    // owner exist
    if (owner || owner_accId)
    {
        if (owner)
            owner->GetSession()->SendAuctionOwnerNotification(auction);

        MailDraft(auction->BuildAuctionMailSubject(AUCTION_EXPIRED), AuctionEntry::BuildAuctionMailBody(0, 0, auction->buyout, auction->deposit, 0))
            .AddItem(pItem)
            .SendMailTo(trans, MailReceiver(owner, auction->owner), auction, MAIL_CHECK_MASK_COPIED, 0);    // owner not found
    }
    else
    {
        // owner doesn't exist, delete the item
        sAuctionMgr->RemoveAItem(auction->itemGUIDLow, true, &trans);
    }
}

void AuctionHouseMgr::LoadAuctionItems()
{
    uint32 oldMSTime = GetMSTime();

    // need to clear in case we are reloading
    if (!mAitems.empty())
    {
        for (ItemMap::iterator itr = mAitems.begin(); itr != mAitems.end(); ++itr)
            delete itr->second;

        mAitems.clear();
    }

    // data needs to be at first place for Item::LoadFromDB
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_AUCTION_ITEMS);
    PreparedQueryResult result = CharacterDatabase.Query(stmt);

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 auction items. DB table `auctionhouse` or `item_instance` is empty!");
        return;
    }

    uint32 count = 0;

    Field *fields;
    do
    {
        fields = result->Fetch();

        uint32 startIndex = CHAR_SEL_ITEM_INSTANCE_FIELDS_COUNT;
        ObjectGuid::LowType item_guid = fields[startIndex++].GetUInt32();
        uint32 itemEntry = fields[startIndex++].GetUInt32();

        ItemTemplate const *proto = sObjectMgr->GetItemTemplate(itemEntry);
        if(!proto)
        {
            TC_LOG_ERROR("sql.sql", "ObjectMgr::LoadAuctionItems: Unknown item template (GUID: %u, id: #%u) in auction, skipped.", item_guid, itemEntry);
            continue;
        }

        Item *item = NewItemOrBag(proto);

        if(!item->LoadFromDB(item_guid, ObjectGuid::Empty, fields, itemEntry))
        {
            TC_LOG_ERROR("sql.sql", "ObjectMgr::LoadAuctionItems: Unknown item (GUID: %u, id: %u) in auction, skipped.", item_guid, itemEntry);
            delete item;
            continue;
        }
        AddAItem(item);

        ++count;
    }
    while( result->NextRow() );

    TC_LOG_INFO("server.loading", ">> Loaded %u auction items in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void AuctionHouseMgr::LoadAuctions()
{
    QueryResult result = CharacterDatabase.Query("SELECT COUNT(*) FROM auctionhouse");
    if( !result )
    {
        TC_LOG_INFO("server.loading",">> Loaded 0 auctions. DB table `auctionhouse` is empty.");
        return;
    }

    Field *fields = result->Fetch();
    uint32 AuctionCount=fields[0].GetUInt64();

    if(!AuctionCount)
    {
        TC_LOG_INFO("server.loading",">> Loaded 0 auctions. DB table `auctionhouse` is empty.");
        return;
    }

    result = CharacterDatabase.Query( "SELECT id,auctioneerguid,itemguid,item_template,count,itemowner,buyoutprice,time,buyguid,lastbid,startbid,deposit FROM auctionhouse ah INNER JOIN item_instance ii ON ii.guid = ah.itemguid" );
    if( !result )
    {
        TC_LOG_INFO("server.loading",">> Loaded 0 auctions. DB table `auctionhouse` is empty.");
        return;
    }

    AuctionEntry *aItem;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    do
    {
        fields = result->Fetch();

        aItem = new AuctionEntry;
        aItem->Id = fields[0].GetUInt32();
        aItem->auctioneer = fields[1].GetUInt32();
        aItem->itemGUIDLow = fields[2].GetUInt32();
        aItem->itemEntry = fields[3].GetUInt32();
        aItem->itemCount = fields[4].GetUInt32();
        aItem->owner = fields[5].GetUInt32();
        aItem->buyout = fields[6].GetUInt32();
        aItem->expire_time = fields[7].GetUInt32();
        aItem->bidder = fields[8].GetUInt32();
        aItem->bid = fields[9].GetUInt32();
        aItem->startbid = fields[10].GetUInt32();
        aItem->deposit = fields[11].GetUInt32();
        aItem->deposit_time = 0;    // No need to save this after a restart, as this is used only for a short amount of time

        CreatureData const* auctioneerData = sObjectMgr->GetCreatureData(aItem->auctioneer);
        if(!auctioneerData)
        {
            AuctionHouseMgr::SendAuctionExpiredMail(aItem, trans);
            aItem->DeleteFromDB(trans);
            TC_LOG_ERROR("misc","Auction %u has not a existing auctioneer (GUID : %u)", aItem->Id, aItem->auctioneer);
            delete aItem;
            continue;
        }

        CreatureTemplate const* auctioneerInfo = sObjectMgr->GetCreatureTemplate(auctioneerData->GetFirstSpawnEntry());
        if(!auctioneerInfo)
        {
            AuctionHouseMgr::SendAuctionExpiredMail(aItem, trans);
            aItem->DeleteFromDB(trans);
            delete aItem;
            TC_LOG_ERROR("misc","Auction %u has not a existing auctioneer (GUID : %u Entry: %u)", aItem->Id, aItem->auctioneer, auctioneerData->GetFirstSpawnEntry());
            continue;
        }

        aItem->auctionHouseEntry = AuctionHouseMgr::GetAuctionHouseEntry(auctioneerInfo->faction);
        if(!aItem->auctionHouseEntry)
        {
            AuctionHouseMgr::SendAuctionExpiredMail(aItem, trans);
            aItem->DeleteFromDB(trans);
            TC_LOG_ERROR("misc","Auction %u has auctioneer (GUID : %u Entry: %u) with wrong faction %u",
                aItem->Id, aItem->auctioneer, auctioneerData->GetFirstSpawnEntry(), auctioneerInfo->faction);
            delete aItem;
            continue;
        }

        // check if sold item exists for guid
        // and item_template in fact (GetAItem will fail if problematic in result check in ObjectMgr::LoadAuctionItems)
        if ( !GetAItem( aItem->itemGUIDLow) )
        {
            aItem->DeleteFromDB(trans);
            TC_LOG_ERROR("misc","Auction %u has not a existing item : %u", aItem->Id, aItem->itemGUIDLow);
            delete aItem;
            continue;
        }

        GetAuctionsMap( auctioneerInfo->faction )->AddAuction(aItem);

    } while (result->NextRow());

    CharacterDatabase.CommitTransaction(trans);

    TC_LOG_INFO( "server.loading",">> Loaded %u auctions", AuctionCount );
}

void AuctionHouseMgr::AddAItem(Item* it)
{
    ASSERT( it );
    ASSERT( mAitems.find(it->GetGUID().GetCounter()) == mAitems.end());
    mAitems[it->GetGUID().GetCounter()] = it;
}

bool AuctionHouseMgr::RemoveAItem(ObjectGuid::LowType id, bool deleteItem /*= false*/, SQLTransaction* trans /*= nullptr*/)
{
    auto i = mAitems.find(id);
    if (i == mAitems.end())
        return false;

    if (deleteItem)
    {
        ASSERT(trans);
        i->second->FSetState(ITEM_REMOVED);
        i->second->SaveToDB(*trans);
    }

    mAitems.erase(i);
    return true;
}

void AuctionHouseMgr::Update()
{
    mHordeAuctions.Update();
    mAllianceAuctions.Update();
    mNeutralAuctions.Update();
}

void AuctionHouseMgr::RemoveAllAuctionsOf(SQLTransaction& trans, ObjectGuid::LowType ownerGUID)
{
    mHordeAuctions.RemoveAllAuctionsOf(trans, ownerGUID);
    mAllianceAuctions.RemoveAllAuctionsOf(trans, ownerGUID);
    mNeutralAuctions.RemoveAllAuctionsOf(trans, ownerGUID);
}

AuctionHouseEntry const* AuctionHouseMgr::GetAuctionHouseEntry(uint32 factionTemplateId)
{
    uint32 houseid = 7; // goblin auction house

    if(!sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
    {
        //FIXME: found way for proper auctionhouse selection by another way
        // AuctionHouse.dbc have faction field with _player_ factions associated with auction house races.
        // but no easy way convert creature faction to player race faction for specific city
        switch(factionTemplateId)
        {
            case 12: houseid = 1; break; // human
            case 29: houseid = 6; break; // orc, and generic for horde
            case 55: houseid = 2; break; // dwarf, and generic for alliance
            case 68: houseid = 4; break; // undead
            case 80: houseid = 3; break; // n-elf
            case 104: houseid = 5; break; // trolls
            case 120: houseid = 7; break; // booty bay, neutral
            case 474: houseid = 7; break; // gadgetzan, neutral
            case 855: houseid = 7; break; // everlook, neutral
            case 1604: houseid = 6; break; // b-elfs,
            default: // for unknown case
            {
                FactionTemplateEntry const* u_entry = sFactionTemplateStore.LookupEntry(factionTemplateId);
                if(!u_entry)
                    houseid = 7; // goblin auction house
                else if(u_entry->ourMask & FACTION_MASK_ALLIANCE)
                    houseid = 1; // human auction house
                else if(u_entry->ourMask & FACTION_MASK_HORDE)
                    houseid = 6; // orc auction house
                else
                    houseid = 7; // goblin auction house
                break;
            }
        }
    }

    return sAuctionHouseStore.LookupEntry(houseid);
}

void AuctionHouseObject::Update()
{
    time_t curTime = WorldGameTime::GetGameTime();
    ///- Handle expired auctions
    AuctionEntryMap::iterator next;
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    for (auto itr = AuctionsMap.begin(); itr != AuctionsMap.end();itr = next)
    {
        next = itr;
        ++next;
        if (curTime > (itr->second->expire_time))
        {
            ///- Either cancel the auction if there was no bidder
            if (itr->second->bidder == 0)
            {
                sAuctionMgr->SendAuctionExpiredMail(itr->second, trans);
            }
            ///- Or perform the transaction
            else
            {
                //we should send an "item sold" message if the seller is online
                //we send the item to the winner
                //we send the money to the seller
                sAuctionMgr->SendAuctionSuccessfulMail(itr->second, trans);
                sAuctionMgr->SendAuctionWonMail(itr->second, trans);
            }

            ///- In any case clear the auction
            itr->second->DeleteFromDB(trans);

            sAuctionMgr->RemoveAItem(itr->second->itemGUIDLow);
            delete itr->second;
            itr->second = nullptr;
            RemoveAuction(itr->first);
        }
    }
    if(trans->GetSize()) //Sun: don't commit empty transaction
        CharacterDatabase.CommitTransaction(trans);
}

// NOT threadsafe!
void AuctionHouseObject::RemoveAllAuctionsOf(SQLTransaction& trans, ObjectGuid::LowType ownerGUID)
{
    AuctionEntryMap::iterator next;
    for (auto itr = AuctionsMap.begin(); itr != AuctionsMap.end();itr = next)
    {
        next = itr;
        ++next;
        if (itr->second->owner == ownerGUID)
        {
            ///- Either cancel the auction if there was no bidder
            if (itr->second->bidder == 0)
            {
                sAuctionMgr->SendAuctionExpiredMail(itr->second, trans);
            }
            ///- Or perform the transaction
            else
            {
                //we should send an "item sold" message if the seller is online
                //we send the item to the winner
                //we send the money to the seller
                sAuctionMgr->SendAuctionSuccessfulMail(itr->second, trans);
                sAuctionMgr->SendAuctionWonMail(itr->second, trans);
            }

            ///- In any case clear the auction
            itr->second->DeleteFromDB(trans);

            sAuctionMgr->RemoveAItem(itr->second->itemGUIDLow);
            delete itr->second;
            itr->second = nullptr;

            RemoveAuction(itr->first);
        }
    }
}

void AuctionHouseObject::BuildListBidderItems(WorldPacket& data, Player* player, uint32& count, uint32& totalcount)
{
    for (AuctionEntryMap::const_iterator itr = AuctionsMap.begin();itr != AuctionsMap.end();++itr)
    {
        AuctionEntry *Aentry = itr->second;
        if( Aentry && Aentry->bidder == player->GetGUID().GetCounter() )
        {
            if (itr->second->BuildAuctionInfo(data))
                ++count;
            ++totalcount;
        }
    }
}

void AuctionHouseObject::BuildListOwnerItems(WorldPacket& data, Player* player, uint32& count, uint32& totalcount)
{
    for (AuctionEntryMap::const_iterator itr = AuctionsMap.begin();itr != AuctionsMap.end();++itr)
    {
        AuctionEntry *Aentry = itr->second;
        if( Aentry && Aentry->owner == player->GetGUID().GetCounter() )
        {
            if(Aentry->BuildAuctionInfo(data))
                ++count;
            ++totalcount;

            if(totalcount >= MAX_AUCTIONS) //avoid client crash
                break;
        }
    }
}

uint32 AuctionHouseObject::GetAuctionsCount(Player* player)
{
    uint32 totalcount = 0;
    for (AuctionEntryMap::const_iterator itr = AuctionsMap.begin();itr != AuctionsMap.end();++itr)
    {
        AuctionEntry *Aentry = itr->second;
        if( Aentry && Aentry->owner == player->GetGUID().GetCounter() )
            ++totalcount;
    }
    return totalcount;
}

void AuctionHouseObject::BuildListAuctionItems(WorldPacket& data, Player* player,
    std::wstring const& wsearchedname, uint32 listfrom, uint32 levelmin, uint32 levelmax, uint32 usable,
    uint32 inventoryType, uint32 itemClass, uint32 itemSubClass, uint32 quality,
    uint32& count, uint32& totalcount)
{
    for (AuctionEntryMap::const_iterator itr = AuctionsMap.begin();itr != AuctionsMap.end();++itr)
    {
        AuctionEntry *Aentry = itr->second;
        Item *item = sAuctionMgr->GetAItem(Aentry->itemGUIDLow);
        if (!item)
            continue;

        ItemTemplate const *proto = item->GetTemplate();

        if (itemClass != (0xffffffff) && proto->Class != itemClass)
            continue;

        if (itemSubClass != (0xffffffff) && proto->SubClass != itemSubClass)
            continue;

        if (inventoryType != (0xffffffff) && proto->InventoryType != inventoryType)
            continue;

        if (quality != (0xffffffff) && proto->Quality != quality)
            continue;

        if(    ( levelmin && (proto->RequiredLevel < levelmin) )
            || ( levelmax && (proto->RequiredLevel > levelmax) ) 
          )
            continue;

        if( usable != (0x00) && player->CanUseItem( item ) != EQUIP_ERR_OK )
            continue;

        std::string name = player->GetSession()->GetLocalizedItemName(proto);
        if(name.empty())
            continue;

        if( !wsearchedname.empty() && !Utf8FitTo(name, wsearchedname) )
            continue;

        if ((count < 50) && (totalcount >= listfrom))
        {
            ++count;
            Aentry->BuildAuctionInfo(data);
        }

        ++totalcount;
    }
}

//this function inserts to WorldPacket auction's data
bool AuctionEntry::BuildAuctionInfo(WorldPacket & data) const
{
    Item *pItem = sAuctionMgr->GetAItem(itemGUIDLow);
    if (!pItem)
    {
        TC_LOG_ERROR("auctionHouse", "Auction to item %u, that doesn't exist !!!!", itemGUIDLow);
        return false;
    }
    data << (uint32) Id;
    data << (uint32) pItem->GetEntry();

    for (uint8 i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; i++)
    {
        data << (uint32) pItem->GetEnchantmentId(EnchantmentSlot(i));
        data << (uint32) pItem->GetEnchantmentDuration(EnchantmentSlot(i));
        data << (uint32) pItem->GetEnchantmentCharges(EnchantmentSlot(i));
    }

    data << (uint32) pItem->GetItemRandomPropertyId();      //random item property id
    data << (uint32) pItem->GetItemSuffixFactor();          //SuffixFactor
    data << (uint32) pItem->GetCount();                     //item->count
    data << (uint32) pItem->GetSpellCharges();              //item->charge FFFFFFF
    data << (uint32) 0;                                     //Unknown
    data << (uint64) owner;                                 //Auction->owner
    data << (uint32) startbid;                              //Auction->startbid (not sure if useful)
    data << (uint32) (bid ? GetAuctionOutBid() : 0);
    //minimal outbid
    data << (uint32) buyout;                                //auction->buyout
    data << (uint32) (expire_time - WorldGameTime::GetGameTime())* 1000;      //time left
    data << (uint64) bidder;                                //auction->bidder current
    data << (uint32) bid;                                   //current bid
    return true;
}

uint32 AuctionEntry::GetAuctionCut() const
{
    uint32 cutPercent;
    if (sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION))
        cutPercent = 5; //default value for horde/alliance auction house
    else
        cutPercent = auctionHouseEntry->cutPercent;

    return uint32(cutPercent * bid * sWorld->GetRate(RATE_AUCTION_CUT) / 100.f);
}

/// the sum of outbid is (1% from current bid)*5, if bid is very small, it is 1c
uint32 AuctionEntry::GetAuctionOutBid() const
{
    uint32 outbid = (bid / 100) * 5;
    if (!outbid)
        outbid = 1;
    return outbid;
}

void AuctionEntry::DeleteFromDB(SQLTransaction& trans) const
{
    //No SQL injection (Id is integer)
    trans->PAppend("DELETE FROM auctionhouse WHERE id = '%u'",Id);
}

void AuctionEntry::SaveToDB(SQLTransaction& trans) const
{
    //No SQL injection (no strings)
    trans->PAppend("INSERT INTO auctionhouse (id,auctioneerguid,itemguid,item_template,itemowner,buyoutprice,time,buyguid,lastbid,startbid,deposit) "
        "VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '" UI64FMTD "', '%u', '%u', '%u', '%u')",
        Id, auctioneer, itemGUIDLow, itemEntry, owner, buyout, (uint64)expire_time, bidder, bid, startbid, deposit);
}

std::string AuctionEntry::BuildAuctionMailSubject(MailAuctionAnswers response) const
{
    std::ostringstream strm;
    strm << itemEntry << ":0:" << response << ':' << Id << ':' << itemCount;
    return strm.str();
}

std::string AuctionEntry::BuildAuctionMailBody(ObjectGuid::LowType lowGuid, uint32 bid, uint32 buyout, uint32 deposit, uint32 cut, bool includeDeliveryTime /*= false*/)
{
    std::ostringstream strm;
    strm.width(16);
    strm << std::right << std::hex << ObjectGuid(HighGuid::Player, lowGuid).GetRawValue();   // HighGuid::Player always present, even for empty guids
    strm << std::dec << ':' << bid << ':' << buyout;
    strm << ':' << deposit << ':' << cut;
    if (includeDeliveryTime)
    { //Not sure this is present on LK
        //does not seem to be working, mail show 12:00 AM by default regardless of this
        time_t distrTime = WorldGameTime::GetGameTime() + sWorld->getConfig(CONFIG_MAIL_DELIVERY_DELAY);
        strm << ':' << secsToTimeBitFields(distrTime);
    }
    return strm.str();
}