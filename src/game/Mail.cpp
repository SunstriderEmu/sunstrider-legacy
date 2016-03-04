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

#include "Mail.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "UpdateMask.h"
#include "Unit.h"
#include "Language.h"
#include "DBCStores.h"
#include "Chat.h"
#include "LogsDatabaseAccessor.h"

void MailItem::deleteItem( bool inDB )
{
    if(item)
    {
        if(inDB)
            CharacterDatabase.PExecute("DELETE FROM item_instance WHERE guid='%u'", item->GetGUIDLow());

        delete item;
        item=NULL;
    }
}

void WorldSession::HandleSendMail(WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,8+1+1+1+4+4+1+4+4+8+1);

    uint64 mailbox, unk3;
    std::string receiver, subject, body;
    uint32 StationeryID, PackageID, money, COD;
    uint8 unk4;
    recvData >> mailbox;
    recvData >> receiver;

    // recheck
    CHECK_PACKET_SIZE(recvData, 8+(receiver.size()+1)+1+1+4+4+1+4+4+8+1);

    recvData >> subject;

    // recheck
    CHECK_PACKET_SIZE(recvData, 8+(receiver.size()+1)+(subject.size()+1)+1+4+4+1+4+4+8+1);

    recvData >> body;

    // recheck
    CHECK_PACKET_SIZE(recvData, 8+(receiver.size()+1)+(subject.size()+1)+(body.size()+1)+4+4+1+4+4+8+1);

    recvData >> StationeryID;
    recvData >> PackageID;

    MailItemsInfo mi;

    uint8 items_count;
    recvData >> items_count;                               // attached items count

    if(items_count > 12)                                    // client limit
        return;

    // recheck
    CHECK_PACKET_SIZE(recvData, 8+(receiver.size()+1)+(subject.size()+1)+(body.size()+1)+4+4+1+items_count*(1+8)+4+4+8+1);

    if(items_count)
    {
        for(uint8 i = 0; i < items_count; ++i)
        {
            uint8  item_slot;
            uint64 item_guid;
            recvData >> item_slot;
            recvData >> item_guid;
            mi.AddItem(GUID_LOPART(item_guid), item_slot);
        }
    }

    recvData >> money >> COD;                              // money and cod
    recvData >> unk3;                                      // const 0, removed in later version
    recvData >> unk4;                                      // const 0, removed in later version

    items_count = mi.size();                                // this is the real size after the duplicates have been removed

    if (receiver.empty())
        return;

    Player* pl = _player;

    uint64 rc = 0;
    if(normalizePlayerName(receiver))
        rc = sObjectMgr->GetPlayerGUIDByName(receiver);

    if (!rc)
    {
        TC_LOG_DEBUG("network","Player %u is sending mail to %s (GUID: not existed!) with subject %s and body %s includes %u items, %u copper and %u COD copper with StationeryID = %u, PackageID = %u",
            pl->GetGUIDLow(), receiver.c_str(), subject.c_str(), body.c_str(), items_count, money, COD, StationeryID, PackageID);
        pl->SendMailResult(0, 0, MAIL_ERR_RECIPIENT_NOT_FOUND);
        return;
    }

    TC_LOG_DEBUG("network", "Player %u is sending mail to %s (GUID: %u) with subject %s and body %s includes %u items, %u copper and %u COD copper with StationeryID = %u, PackageID = %u", pl->GetGUIDLow(), receiver.c_str(), GUID_LOPART(rc), subject.c_str(), body.c_str(), items_count, money, COD, StationeryID, PackageID);

    if(pl->GetGUID() == rc)
    {
        pl->SendMailResult(0, 0, MAIL_ERR_CANNOT_SEND_TO_SELF);
        return;
    }

    uint32 reqmoney = money + 30;
    if (items_count)
        reqmoney = money + (30 * items_count);

    if (pl->GetMoney() < reqmoney)
    {
        pl->SendMailResult(0, 0, MAIL_ERR_NOT_ENOUGH_MONEY);
        return;
    }

    Player *receive = sObjectMgr->GetPlayer(rc);

    uint32 rc_team = 0;
    uint8 mails_count = 0;                                  //do not allow to send to one player more than 100 mails

    if(receive)
    {
        rc_team = receive->GetTeam();
        mails_count = receive->GetMailSize();
    }
    else
    {
        rc_team = sObjectMgr->GetPlayerTeamByGUID(rc);
        QueryResult result = CharacterDatabase.PQuery("SELECT COUNT(*) FROM mail WHERE receiver = '%u'", GUID_LOPART(rc));
        if(result)
        {
            Field *fields = result->Fetch();
            mails_count = fields[0].GetUInt64();
        }
    }
    //do not allow to have more than 100 mails in mailbox.. mails count is in opcode uint8!!! - so max can be 255..
    if (mails_count > 100)
    {
        pl->SendMailResult(0, 0, MAIL_ERR_INTERNAL_ERROR);
        return;
    }
    // test the receiver's Faction...
    if (!sWorld->getConfig(CONFIG_ALLOW_TWO_SIDE_INTERACTION_MAIL) && pl->GetTeam() != rc_team && GetSecurity() == SEC_PLAYER)
    {
        pl->SendMailResult(0, 0, MAIL_ERR_NOT_YOUR_TEAM);
        return;
    }

    if (items_count)
    {
        for(MailItemMap::iterator mailItemIter = mi.begin(); mailItemIter != mi.end(); ++mailItemIter)
        {
            MailItem& mailItem = mailItemIter->second;

            if(!mailItem.item_guidlow)
            {
                pl->SendMailResult(0, 0, MAIL_ERR_INTERNAL_ERROR);
                return;
            }

            mailItem.item = pl->GetItemByGuid(MAKE_NEW_GUID(mailItem.item_guidlow, 0, HIGHGUID_ITEM));
            // prevent sending bag with items (cheat: can be placed in bag after adding equipped empty bag to mail)
            if(!mailItem.item || !mailItem.item->CanBeTraded())
            {
                pl->SendMailResult(0, 0, MAIL_ERR_INTERNAL_ERROR);
                return;
            }
            if (mailItem.item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_CONJURED) || mailItem.item->GetUInt32Value(ITEM_FIELD_DURATION))
            {
                pl->SendMailResult(0, 0, MAIL_ERR_INTERNAL_ERROR);
                return;
            }

            if(COD && mailItem.item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAPPED))
            {
                pl->SendMailResult(0, 0, MAIL_ERR_CANT_SEND_WRAPPED_COD);
                return;
            }
        }
    }
    pl->SendMailResult(0, 0, MAIL_OK);

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    uint32 itemTextId = 0;
    if (!body.empty())
    {
        itemTextId = sObjectMgr->CreateItemText(trans, body);
    }

    pl->ModifyMoney( -int32(reqmoney) );

    bool needItemDelay = false;

    if(items_count > 0 || money > 0)
    {
        uint32 rc_account = 0;
        if(receive)
            rc_account = receive->GetSession()->GetAccountId();
        else
            rc_account = sObjectMgr->GetPlayerAccountIdByGUID(rc);

        if (items_count > 0)
        {
            for(MailItemMap::iterator mailItemIter = mi.begin(); mailItemIter != mi.end(); ++mailItemIter)
            {
                MailItem& mailItem = mailItemIter->second;
                if(!mailItem.item)
                    continue;

                mailItem.item_template = mailItem.item ? mailItem.item->GetEntry() : 0;

                pl->MoveItemFromInventory(mailItem.item->GetBagSlot(), mailItem.item->GetSlot(), true);
                mailItem.item->DeleteFromInventoryDB(trans);     //deletes item from character's inventory
                mailItem.item->SaveToDB(trans);                  // recursive and not have transaction guard into self, item not in inventory and can be save standalone
                // owner in data will set at mail receive and item extracting
                trans->PAppend("UPDATE item_instance SET owner_guid = '%u' WHERE guid='%u'", GUID_LOPART(rc), mailItem.item->GetGUIDLow());
            }

            // if item send to character at another account, then apply item delivery delay
            needItemDelay = pl->GetSession()->GetAccountId() != rc_account;
        }
    }

    // If theres is an item, there is a one hour delivery delay if sent to another account's character.
    uint32 deliver_delay = needItemDelay ? sWorld->getConfig(CONFIG_MAIL_DELIVERY_DELAY) : 0;

    // will delete item or place to receiver mail list
    uint8 stationery = pl->IsGameMaster() ? MAIL_STATIONERY_GM : MAIL_STATIONERY_NORMAL;
    WorldSession::SendMailTo(trans, receive, MAIL_NORMAL, stationery, pl->GetGUIDLow(), GUID_LOPART(rc), subject, itemTextId, &mi, money, COD, MAIL_CHECK_MASK_NONE, deliver_delay);

    pl->SaveInventoryAndGoldToDB(trans);
    CharacterDatabase.CommitTransaction(trans);
}

//called when mail is read
void WorldSession::HandleMailMarkAsRead(WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,8+4);

    uint64 mailbox;
    uint32 mailId;
    recvData >> mailbox;
    recvData >> mailId;
    Player *pl = _player;
    Mail *m = pl->GetMail(mailId);
    if (m)
    {
        if (pl->unReadMails)
            --pl->unReadMails;
        m->checked = m->checked | MAIL_CHECK_MASK_READ;
        // m->expire_time = time(NULL) + (30 * DAY);  // Expire time do not change at reading mail
        pl->m_mailsUpdated = true;
        m->state = MAIL_STATE_CHANGED;
    }
}

//called when client deletes mail
void WorldSession::HandleMailDelete(WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,8+4);

    uint64 mailbox;
    uint32 mailId;
    recvData >> mailbox;
    recvData >> mailId;
    Player* pl = _player;
    pl->m_mailsUpdated = true;
    Mail *m = pl->GetMail(mailId);
    if(m)
        m->state = MAIL_STATE_DELETED;
    pl->SendMailResult(mailId, MAIL_DELETED, 0);
}

void WorldSession::HandleMailReturnToSender(WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,8+4);

    uint64 mailbox;
    uint32 mailId;
    recvData >> mailbox;
    recvData >> mailId;
    Player *pl = _player;
    Mail *m = pl->GetMail(mailId);
    if(!m || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL))
    {
        pl->SendMailResult(mailId, MAIL_RETURNED_TO_SENDER, MAIL_ERR_INTERNAL_ERROR);
        return;
    }
    //we can return mail now
    //so firstly delete the old one
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM mail WHERE id = '%u'", mailId);
    trans->PAppend("DELETE FROM mail_items WHERE mail_id = '%u'", mailId);
    CharacterDatabase.CommitTransaction(trans);
    pl->RemoveMail(mailId);

    MailItemsInfo mi;

    if(m->HasItems())
    {
        for(std::vector<MailItemInfo>::iterator itr2 = m->items.begin(); itr2 != m->items.end(); ++itr2)
        {
            Item *item = pl->GetMItem(itr2->item_guid);
            if(item)
                mi.AddItem(item->GetGUIDLow(), item->GetEntry(), item);
            else
            {
                //WTF?
            }

            pl->RemoveMItem(itr2->item_guid);
        }
    }

    if (m->sender == 0)
    {
        SendReturnToSender(MAIL_CREATURE, GetAccountId(), m->receiver, m->sender, m->subject, m->itemTextId, &mi, m->money, m->mailTemplateId);
    }
    else
    {
        SendReturnToSender(MAIL_NORMAL, GetAccountId(), m->receiver, m->sender, m->subject, m->itemTextId, &mi, m->money, m->mailTemplateId);
    }

    delete m;                                               //we can deallocate old mail
    pl->SendMailResult(mailId, MAIL_RETURNED_TO_SENDER, 0);
}

void WorldSession::SendReturnToSender(uint8 messageType, uint32 sender_acc, uint32 sender_guid, uint32 receiver_guid, const std::string& subject, uint32 itemTextId, MailItemsInfo *mi, uint32 money, uint16 mailTemplateId )
{
    if(messageType != MAIL_NORMAL)                          // return only to players
    {
        mi->deleteIncludedItems(true);
        return;
    }

    Player *receiver = sObjectMgr->GetPlayer(MAKE_NEW_GUID(receiver_guid, 0, HIGHGUID_PLAYER));

    uint32 rc_account = 0;
    if(!receiver)
        rc_account = sObjectMgr->GetPlayerAccountIdByGUID(MAKE_NEW_GUID(receiver_guid, 0, HIGHGUID_PLAYER));

    if(!receiver && !rc_account)                            // sender not exist
    {
        mi->deleteIncludedItems(true);
        return;
    }

    // prepare mail and send in other case
    bool needItemDelay = false;

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    if(mi && !mi->empty())
    {
        // if item send to character at another account, then apply item delivery delay
        needItemDelay = sender_acc != rc_account;

        // set owner to new receiver (to prevent delete item with sender char deleting)
        for(MailItemMap::iterator mailItemIter = mi->begin(); mailItemIter != mi->end(); ++mailItemIter)
        {
            MailItem& mailItem = mailItemIter->second;
            mailItem.item->SaveToDB(trans);                  // item not in inventory and can be save standalone
            // owner in data will set at mail receive and item extracting
            trans->PAppend("UPDATE item_instance SET owner_guid = '%u' WHERE guid='%u'", receiver_guid, mailItem.item->GetGUIDLow());
        }
    }

    // If theres is an item, there is a one hour delivery delay.
    uint32 deliver_delay = needItemDelay ? sWorld->getConfig(CONFIG_MAIL_DELIVERY_DELAY) : 0;

    // will delete item or place to receiver mail list
    WorldSession::SendMailTo(trans, receiver, MAIL_NORMAL, MAIL_STATIONERY_NORMAL, sender_guid, receiver_guid, subject, itemTextId, mi, money, 0, MAIL_CHECK_MASK_RETURNED,deliver_delay,mailTemplateId);
    CharacterDatabase.CommitTransaction(trans);
}

//called when player takes item attached in mail
void WorldSession::HandleMailTakeItem(WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,8+4+4);

    uint64 mailbox;
    uint32 mailId;
    uint32 itemId;
    recvData >> mailbox;
    recvData >> mailId;
    recvData >> itemId;                                    // item guid low?
    Player* pl = _player;

    Mail* m = pl->GetMail(mailId);
    if(!m || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL))
    {
        pl->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    // prevent cheating with skip client money check
    if(pl->GetMoney() < m->COD)
    {
        pl->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_ERR_NOT_ENOUGH_MONEY);
        return;
    }

    Item *it = pl->GetMItem(itemId);

    ItemPosCountVec dest;
    uint8 msg = _player->CanStoreItem( NULL_BAG, NULL_SLOT, dest, it, false );
    if( msg == EQUIP_ERR_OK )
    {
        m->RemoveItem(itemId);
        m->removedItems.push_back(itemId);

        if (m->COD > 0)                                     //if there is COD, take COD money from player and send them to sender by mail
        {
            uint64 sender_guid = MAKE_NEW_GUID(m->sender, 0, HIGHGUID_PLAYER);
            Player *receive = sObjectMgr->GetPlayer(sender_guid);

            uint32 sender_accId = sObjectMgr->GetPlayerAccountIdByGUID(sender_guid);

            // check player existence
            if(receive || sender_accId)
            {
                WorldSession::SendMailTo(receive, MAIL_NORMAL, MAIL_STATIONERY_NORMAL, m->receiver, m->sender, m->subject, 0, NULL, m->COD, 0, MAIL_CHECK_MASK_COD_PAYMENT);
            }

            pl->ModifyMoney( -int32(m->COD) );
        }
        m->COD = 0;
        m->state = MAIL_STATE_CHANGED;
        pl->m_mailsUpdated = true;
        pl->RemoveMItem(it->GetGUIDLow());

        uint32 count = it->GetCount();                      // save counts before store and possible merge with deleting
        pl->MoveItemToInventory(dest,it,true);

        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        pl->SaveInventoryAndGoldToDB(trans);
        pl->_SaveMail(trans);
        CharacterDatabase.CommitTransaction(trans);

        pl->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_OK, 0, itemId, count);
    }
    else
        pl->SendMailResult(mailId, MAIL_ITEM_TAKEN, MAIL_ERR_BAG_FULL, msg);
}

void WorldSession::HandleMailTakeMoney(WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,8+4);

    uint64 mailbox;
    uint32 mailId;
    recvData >> mailbox;
    recvData >> mailId;
    Player *pl = _player;

    Mail* m = pl->GetMail(mailId);
    if(!m || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL))
    {
        pl->SendMailResult(mailId, MAIL_MONEY_TAKEN, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    pl->SendMailResult(mailId, MAIL_MONEY_TAKEN, 0);

    pl->ModifyMoney(m->money);
    m->money = 0;
    m->state = MAIL_STATE_CHANGED;
    pl->m_mailsUpdated = true;

    // save money and mail to prevent cheating
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    pl->SaveGoldToDB(trans);                                // contains money
    pl->_SaveMail(trans);
    CharacterDatabase.CommitTransaction(trans);
}

//called when player lists his received mails
void WorldSession::HandleGetMailList(WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,8);

    uint64 mailbox;
    recvData >> mailbox;

    //GameObject* obj = ObjectAccessor::GetGameObject(_player, mailbox);
    //if(!obj || !obj->IsMailBox())
    //    return;

    Player* pl = _player;

    //load players mails, and mailed items
    if(!pl->m_mailsLoaded)
        pl ->_LoadMail();

    // client can't work with packets > max int16 value
    const uint32 maxPacketSize = 32767;

    uint32 mails_count = 0;                                 // real send to client mails amount
    
    bool partial = false; // Did we have to cut result set to prevent client crash?

    WorldPacket data(SMSG_MAIL_LIST_RESULT, (200));         // guess size
    data << uint8(0);                                       // mail's count
    time_t cur_time = time(NULL);

    for(PlayerMails::iterator itr = pl->GetmailBegin(); itr != pl->GetmailEnd(); ++itr)
    {
        // skip deleted or not delivered (deliver delay not expired) mails
        if ((*itr)->state == MAIL_STATE_DELETED || cur_time < (*itr)->deliver_time)
            continue;

        uint8 item_count = (*itr)->items.size();            // max count is MAX_MAIL_ITEMS (12)

        size_t next_mail_size = 2+4+1+8+4*8+((*itr)->subject.size()+1)+1+item_count*(1+4+4+6*3*4+4+4+1+4+4+4);

        if(data.wpos()+next_mail_size > maxPacketSize)
            break;

        data << (uint16) 0x0040;                            // unknown 2.3.0, different values
        data << (uint32) (*itr)->messageID;                 // Message ID
        data << (uint8) (*itr)->messageType;                // Message Type

        switch((*itr)->messageType)
        {
            case MAIL_NORMAL:                               // sender guid
                data << uint64(MAKE_NEW_GUID((*itr)->sender, 0, HIGHGUID_PLAYER));
                break;
            case MAIL_CREATURE:
            case MAIL_GAMEOBJECT:
            case MAIL_AUCTION:
                data << (uint32) (*itr)->sender;            // creature/gameobject entry, auction id
                break;
            case MAIL_ITEM:                                 // item entry (?) sender = "Unknown", NYI
                break;
        }

        data << (uint32) (*itr)->COD;                       // COD
        data << (uint32) (*itr)->itemTextId;                // sure about this
        data << (uint32) 0;                                 // unknown
        data << (uint32) (*itr)->stationery;                // stationery (Stationery.dbc)
        data << (uint32) (*itr)->money;                     // Gold
        data << (uint32) 0x04;                              // unknown, 0x4 - auction, 0x10 - normal
                                                            // Time
        data << (float)  ((*itr)->expire_time-time(NULL))/DAY;
        data << (uint32) (*itr)->mailTemplateId;            // mail template (MailTemplate.dbc)
        data << (*itr)->subject;                            // Subject string - once 00, when mail type = 3

        data << (uint8) item_count;

        for(uint8 i = 0; i < item_count; ++i)
        {
            Item *item = pl->GetMItem((*itr)->items[i].item_guid);
            // item index (0-6?)
            data << (uint8)  i;
            // item guid low?
            data << (uint32) (item ? item->GetGUIDLow() : 0);
            // entry
            data << (uint32) (item ? item->GetEntry() : 0);
            for(uint8 j = 0; j < 6; ++j)
            {
                data << (uint32) (item ? item->GetEnchantmentId((EnchantmentSlot)j) : 0);
                data << uint32((item ? item->GetEnchantmentDuration((EnchantmentSlot)j) : 0));
                data << uint32((item ? item->GetEnchantmentCharges((EnchantmentSlot)j) : 0));
            }
            // can be negative
            data << (uint32) (item ? item->GetItemRandomPropertyId() : 0);
            // unk
            data << (uint32) (item ? item->GetItemSuffixFactor() : 0);
            // stack count
            data << (uint8)  (item ? item->GetCount() : 0);
            // charges
            data << (uint32) (item ? item->GetSpellCharges() : 0);
            // durability
            data << (uint32) (item ? item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) : 0);
            // durability
            data << (uint32) (item ? item->GetUInt32Value(ITEM_FIELD_DURABILITY) : 0);
        }

        mails_count += 1;
        if (mails_count > 100) {
            partial = true;
            break;
        }
    }

    data.put<uint8>(0, mails_count);                        // set real send mails to client
    SendPacket(&data);
    
    if (partial)
        ChatHandler(_player).SendSysMessage(LANG_MAIL_LIST_PARTIAL);

    // recalculate m_nextMailDelivereTime and unReadMails
    _player->UpdateNextMailTimeAndUnreads();
}

///this function is called when client needs mail message body, or when player clicks on item which has ITEM_FIELD_ITEM_TEXT_ID > 0
void WorldSession::HandleItemTextQuery(WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,4+4+4);

    uint32 itemTextId;
    uint32 mailId;                                          //this value can be item id in bag, but it is also mail id
    uint32 unk;                                             //maybe something like state - 0x70000000

    recvData >> itemTextId >> mailId >> unk;

    //some check needed, if player has item with guid mailId, or has mail with id mailId

    WorldPacket data(SMSG_ITEM_TEXT_QUERY_RESPONSE, (4+10));// guess size
    data << itemTextId;
    data << sObjectMgr->GetItemText( itemTextId );
    SendPacket(&data);
}

//used when player copies mail body to his inventory
void WorldSession::HandleMailCreateTextItem(WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,8+4);

    /* Is this supposed to be BC ?
    uint64 mailbox;
    uint32 mailId;

    recvData >> mailbox >> mailId;

    Player *pl = _player;

    Mail* m = pl->GetMail(mailId);
    if(!m || !m->itemTextId || m->state == MAIL_STATE_DELETED || m->deliver_time > time(NULL))
    {
        pl->SendMailResult(mailId, MAIL_MADE_PERMANENT, MAIL_ERR_INTERNAL_ERROR);
        return;
    }

    Item *bodyItem = new Item;                              // This is not bag and then can be used new Item.
    if(!bodyItem->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_ITEM), MAIL_BODY_ITEM_TEMPLATE, pl,sObjectMgr->GetItemTemplate(MAIL_BODY_ITEM_TEMPLATE)))
    {
        TC_LOG_ERROR("FIXME","HandleMailCreateTextItem could not create bodyItem");
        delete bodyItem;
        return;
    }

    bodyItem->SetUInt32Value( ITEM_FIELD_ITEM_TEXT_ID , m->itemTextId );
    bodyItem->SetUInt32Value( ITEM_FIELD_CREATOR, m->sender);

    TC_LOG_DEBUG("FIXME","HandleMailCreateTextItem mailid=%u",mailId);

    ItemPosCountVec dest;
    uint8 msg = _player->CanStoreItem( NULL_BAG, NULL_SLOT, dest, bodyItem, false );
    if( msg == EQUIP_ERR_OK )
    {
        m->itemTextId = 0;
        m->state = MAIL_STATE_CHANGED;
        pl->m_mailsUpdated = true;

        pl->StoreItem(dest, bodyItem, true);
        //bodyItem->SetState(ITEM_NEW, pl); is set automatically
        pl->SendMailResult(mailId, MAIL_MADE_PERMANENT, 0);
    }
    else
    {
        pl->SendMailResult(mailId, MAIL_MADE_PERMANENT, MAIL_ERR_BAG_FULL, msg);
        delete bodyItem;
    }
    */
}

//TODO Fix me! ... this void has probably bad condition, but good data are sent
void WorldSession::HandleQueryNextMailTime(WorldPacket & /*recvData*/ )
{
    
    
    WorldPacket data(MSG_QUERY_NEXT_MAIL_TIME, 8);

    if(!_player->m_mailsLoaded)
        _player->_LoadMail();

    if( _player->unReadMails > 0 )
    {
        data << float(0);                                   // float
        data << (uint32) 0;                                 // count

        uint32 count = 0;
        time_t now = time(NULL);
        std::set<uint32> sentSenders;
        for(PlayerMails::iterator itr = _player->GetmailBegin(); itr != _player->GetmailEnd(); ++itr)
        {
            Mail *m = (*itr);
            // must be not checked yet
            if (m->checked & MAIL_CHECK_MASK_READ)
                continue;

            // only send each mail sender once
            if (sentSenders.count(m->sender))
                continue;

            // and already delivered
            if (now < m->deliver_time)
                continue;

            data << uint64(m->messageType == MAIL_NORMAL ? MAKE_PAIR64(m->sender, HIGHGUID_PLAYER) :0);  // player guid
            data << uint32(m->messageType != MAIL_NORMAL ? m->sender : 0);  // non-player entries
            data << uint32(m->messageType);
            data << uint32(m->stationery);
            data << float(m->deliver_time - now);

            sentSenders.insert(m->sender);
            ++count;
            if (count == 2)
                break; // do not display more than 2 mails
        }
        data.put<uint32>(4, count);
    }
    else
    {
        data << float(-DAY);
        data << uint32(0);
    }
    SendPacket(&data);
}

void WorldSession::SendMailTo(SQLTransaction& trans, Player* receiver, MailMessageType messageType, uint8 stationery, uint32 sender_guidlow_or_entry, uint32 receiver_guidlow, std::string subject, uint32 itemTextId, MailItemsInfo* mi, uint32 money, uint32 COD, uint32 checked, uint32 deliver_delay, uint16 mailTemplateId)
{
    if (receiver_guidlow == 0)
    {
        if(messageType == MAIL_AUCTION && mi)        // auction mail with items
        {
            mi->deleteIncludedItems(true);
        }
        return;
    }
    uint32 mailId = sObjectMgr->GenerateMailID();

    time_t deliver_time = time(NULL) + deliver_delay;

    //expire time if COD 3 days, if no COD 30 days, if auction sale pending 1 hour
    uint32 expire_delay;
    if(messageType == MAIL_AUCTION && !mi && !money)        // auction mail without any items and money
        expire_delay = sWorld->getConfig(CONFIG_MAIL_DELIVERY_DELAY);
    else
        expire_delay = (COD > 0) ? 3*DAY : 30*DAY;
        
    if (stationery == MAIL_STATIONERY_GM)
        expire_delay *= 2;

    time_t expire_time = deliver_time + expire_delay;

    if(mailTemplateId && !sMailTemplateStore.LookupEntry(mailTemplateId))
    {
        TC_LOG_ERROR( "mail", "WorldSession::SendMailTo - Mail have not existed MailTemplateId (%u), remove at send", mailTemplateId);
        mailTemplateId = 0;
    }

    //log it
    LogsDatabaseAccessor::Mail(mailId, messageType, sender_guidlow_or_entry, receiver_guidlow, subject, itemTextId, mi, money > 0 ? money : -int32(COD));

    if(receiver)
    {
        receiver->AddNewMailDeliverTime(deliver_time);

        if ( receiver->IsMailsLoaded() )
        {
            Mail * m = new Mail;
            m->messageID = mailId;
            m->messageType = messageType;
            m->stationery = stationery;
            m->mailTemplateId = mailTemplateId;
            m->sender = sender_guidlow_or_entry;
            m->receiver = receiver->GetGUIDLow();
            m->subject = subject;
            m->itemTextId = itemTextId;

            if(mi)
                m->AddAllItems(*mi);

            m->expire_time = expire_time;
            m->deliver_time = deliver_time;
            m->money = money;
            m->COD = COD;
            m->checked = checked;
            m->state = MAIL_STATE_UNCHANGED;

            receiver->AddMail(m);                           //to insert new mail to beginning of maillist

            if(mi)
            {
                for(MailItemMap::iterator mailItemIter = mi->begin(); mailItemIter != mi->end(); ++mailItemIter)
                {
                    MailItem& mailItem = mailItemIter->second;
                    if(mailItem.item)
                        receiver->AddMItem(mailItem.item);
                }
            }
        }
        else if(mi)
            mi->deleteIncludedItems();
    }

    CharacterDatabase.EscapeString(subject);
    trans->PAppend("INSERT INTO mail (id,messageType,stationery,mailTemplateId,sender,receiver,subject,itemTextId,has_items,expire_time,deliver_time,money,cod,checked) "
        "VALUES ('%u', '%u', '%u', '%u', '%u', '%u', '%s', '%u', '%u', '" UI64FMTD "','" UI64FMTD "', '%u', '%u', '%d')",
        mailId, messageType, stationery, mailTemplateId, sender_guidlow_or_entry, receiver_guidlow, subject.c_str(), itemTextId, (mi && !mi->empty() ? 1 : 0), (uint64)expire_time, (uint64)deliver_time, money, COD, checked);

    if(mi)
    {
        for(MailItemMap::const_iterator mailItemIter = mi->begin(); mailItemIter != mi->end(); ++mailItemIter)
        {
            MailItem const& mailItem = mailItemIter->second;
            trans->PAppend("INSERT INTO mail_items (mail_id,item_guid,item_template,receiver) VALUES ('%u', '%u', '%u','%u')", mailId, mailItem.item_guidlow, mailItem.item_template,receiver_guidlow);
        }
    }

    //receiver is not online, delete item from memory for now
    if(mi && !receiver)
        mi->deleteIncludedItems(false);
}

void WorldSession::SendMailTo(Player* receiver, MailMessageType messageType, uint8 stationery, uint32 sender_guidlow_or_entry, uint32 received_guidlow, std::string subject, uint32 itemTextId, MailItemsInfo* mi, uint32 money, uint32 COD, uint32 checked, uint32 deliver_delay, uint16 mailTemplateId)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    SendMailTo(trans, receiver, messageType, stationery, sender_guidlow_or_entry, received_guidlow, subject, itemTextId, mi, money, COD, checked, deliver_delay, mailTemplateId);
    CharacterDatabase.CommitTransaction(trans);
}