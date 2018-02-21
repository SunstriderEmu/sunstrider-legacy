
#include "Common.h"
#include "WorldPacket.h"
#include "Log.h"
#include "Corpse.h"
#include "GameObject.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "WorldSession.h"
#include "LootMgr.h"
#include "Object.h"
#include "Group.h"
#include "World.h"
#include "Util.h"
#include "LootItemStorage.h"

void WorldSession::HandleAutostoreLootItemOpcode( WorldPacket & recvData )
{
    Player  *player =   GetPlayer();
    ObjectGuid   lguid =    player->GetLootGUID();
    Loot    *loot;
    uint8    lootSlot;

    recvData >> lootSlot;

    if (lguid.IsGameObject())
    {
        GameObject *go =
            ObjectAccessor::GetGameObject(*player, lguid);

        if(!go)
            return;
        
        // not check distance for GO in case owned GO (fishing bobber case, for example) or Fishing hole GO
        if (   go->GetOwnerGUID() != _player->GetGUID() 
            && go->GetGoType() != GAMEOBJECT_TYPE_FISHINGHOLE
            && !go->IsInRange(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), INTERACTION_DISTANCE)
           )
        {
            player->SendLootRelease(lguid);
            return;
        }

        loot = &go->loot;
    }
    else if (lguid.IsItem())
    {
        Item *pItem = player->GetItemByGuid( lguid );

        if (!pItem)
        {
            player->SendLootRelease(lguid);
            return;
        }

        loot = &pItem->loot;
    }
    else if (lguid.IsCorpse())
    {
        Corpse *bones = ObjectAccessor::GetCorpse(*player, lguid);
        if (!bones)
        {
            player->SendLootRelease(lguid);
            return;
        }
        loot = &bones->loot;
    }
    else
    {
        Creature* creature = ObjectAccessor::GetCreature(*player, lguid);

        bool lootAllowed = creature && creature->IsAlive() == (player->GetClass() == CLASS_ROGUE && creature->loot.loot_type == LOOT_PICKPOCKETING);
        if (!lootAllowed)
        {
            player->SendLootError(lguid, LOOT_ERROR_DIDNT_KILL);
            return;
        }

        if(!creature->IsWithinDistInMap(_player,INTERACTION_DISTANCE))
        {
            player->SendLootError(lguid, LOOT_ERROR_TOO_FAR);
            return;
        }

        loot = &creature->loot;
    }

    player->StoreLootItem(lootSlot, loot);
   
    // If player is removing the last LootItem, delete the empty container.
    if (loot->isLooted() && lguid.IsItem())
        player->GetSession()->DoLootRelease(lguid);
}

void WorldSession::HandleLootMoneyOpcode( WorldPacket & /*recvData*/ )
{
    Player *player = GetPlayer();
    ObjectGuid guid = player->GetLootGUID();
    if(!guid)
        return;

    Loot* loot = nullptr;
    bool shareMoney = true;

    switch(guid.GetHigh())
    {
        case HighGuid::GameObject:
        {
            GameObject* pGameObject = ObjectAccessor::GetGameObject(*GetPlayer(), guid);

            // not check distance for GO in case owned GO (fishing bobber case, for example)
            if( pGameObject && (pGameObject->GetOwnerGUID()==_player->GetGUID() || pGameObject->IsWithinDistInMap(_player,INTERACTION_DISTANCE)) )
                loot = &pGameObject->loot;

            break;
        }
        case HighGuid::Corpse:                               // remove insignia ONLY in BG
        {
            Corpse *bones = ObjectAccessor::GetCorpse(*GetPlayer(), guid);

            if (bones && bones->IsWithinDistInMap(_player, INTERACTION_DISTANCE))
            {
                loot = &bones->loot;
                shareMoney = false;
            }

            break;
        }
        case HighGuid::Item:
        {
            if(Item *item = GetPlayer()->GetItemByGuid(guid))
            {
                loot = &item->loot;
                shareMoney = false;
            }
            break;
        }
        case HighGuid::Unit:
#ifdef LICH_KING
        case HighGuid::Vehicle:
#endif
        {
            Creature* creature = ObjectAccessor::GetCreature(*GetPlayer(), guid);

            bool lootAllowed = creature && creature->IsAlive() == (player->GetClass() == CLASS_ROGUE && creature->loot.loot_type == LOOT_PICKPOCKETING);
            if (!lootAllowed)
            {
                player->SendLootError(guid, LOOT_ERROR_DIDNT_KILL);
                break;
            }
            if (!creature->IsWithinDistInMap(_player, INTERACTION_DISTANCE))
            {
                player->SendLootError(guid, LOOT_ERROR_TOO_FAR);
                break;
            }
             
            loot = &creature->loot;
            if (creature->IsAlive())
                shareMoney = false;

            break;
        }
        default:
            return;                                         // unlootable type
    }

    if (loot)
    {
        loot->NotifyMoneyRemoved();
        //sun: no use getting further if there is no gold left
        if (loot->gold == 0)
            return;

        if (shareMoney && player->GetGroup())      //item can be looted only single player
        {
            Group *group = player->GetGroup();

            std::vector<Player*> playersNear;
            for(GroupReference *itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* playerGroup = itr->GetSource();
                if(!playerGroup)
                    continue;
                if (player->IsAtGroupRewardDistance(playerGroup))
                    playersNear.push_back(playerGroup);
            }

            uint32 goldPerPlayer = uint32((loot->gold) / (playersNear.size()));
            for (auto& i : playersNear)
            {
                i->ModifyMoney(goldPerPlayer);
#ifdef LICH_KING
                i->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY, goldPerPlayer);
#endif
                WorldPacket data(SMSG_LOOT_MONEY_NOTIFY, 4);
                data << uint32(goldPerPlayer);
#ifdef LICH_KING
                data << uint8(playersNear.size() <= 1); // Controls the text displayed in chat. 0 is "Your share is..." and 1 is "You loot..."
#endif
                i->SendDirectMessage( &data );
            }
        }
        else
        {
            player->ModifyMoney(loot->gold);
#ifdef LICH_KING
            player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY, pLoot->gold);
#endif
            WorldPacket data(SMSG_LOOT_MONEY_NOTIFY, 4);
            data << uint32(loot->gold);
#ifdef LICH_KING
            data << uint8(1);   // "You loot..."
#endif
            SendPacket(&data);
        }

        loot->gold = 0;

        // Delete the money loot record from the DB
        if (loot->containerID > 0)
            sLootItemStorage->RemoveStoredMoneyForContainer(loot->containerID);

        // Delete container if empty
        if (loot->isLooted() && guid.IsItem())
            player->GetSession()->DoLootRelease(guid);
    }
}

void WorldSession::HandleLootOpcode( WorldPacket & recvData )
{
    ObjectGuid guid;
    recvData >> guid;

    // Check possible cheat
    if (!GetPlayer()->IsAlive() || !guid.IsCreatureOrVehicle())
        return;

    GetPlayer()->SendLoot(guid, LOOT_CORPSE);

    // interrupt cast
    if (GetPlayer()->IsNonMeleeSpellCast(false))
        GetPlayer()->InterruptNonMeleeSpells(false);
}

void WorldSession::HandleLootReleaseOpcode( WorldPacket & recvData )
{
    // cheaters can modify lguid to prevent correct apply loot release code and re-loot
    // use internal stored guid
    ObjectGuid   guid;
    recvData >> guid;

    ObjectGuid lguid = GetPlayer()->GetLootGUID();
    if (lguid)
    {
        if (lguid == guid)
        {
            DoLootRelease(lguid);
            return;
        }
    }
    TC_LOG_DEBUG("network.opcode", "Player %u tried to release loot %u but wasn't looting it (was looting %u)", GetPlayer()->GetGUID().GetCounter(), guid.GetCounter(), lguid.GetCounter());
}

void WorldSession::DoLootRelease(ObjectGuid lguid)
{
    Player  *player = GetPlayer();
    Loot    *loot;

    player->SetLootGUID(ObjectGuid::Empty);
    player->SendLootRelease(lguid);
    
    player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOOTING);

    if (!player->IsInWorld())
        return;

    if (lguid.IsGameObject())
    {
        GameObject *go = ObjectAccessor::GetGameObject(*player, lguid);
        if (!go)
            return;
        
        // not check distance for GO in case owned GO (fishing bobber case, for example) or Fishing hole GO
        if(    go->GetGoType() != GAMEOBJECT_TYPE_FISHINGHOLE
            && go->GetOwnerGUID() != _player->GetGUID()
            && !go->IsInRange(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(),INTERACTION_DISTANCE) )
            return;

        loot = &go->loot;

        if (go->GetGoType() == GAMEOBJECT_TYPE_DOOR)
        {
            // locked doors are opened with spelleffect openlock, prevent remove its as looted
            go->UseDoorOrButton();
        }
        else if (loot->isLooted() || go->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE)
        {
            // GO is mineral vein? so it is not removed after its looted
            if(go->GetGoType() == GAMEOBJECT_TYPE_CHEST)
            {
                uint32 go_min = go->GetGOInfo()->chest.minSuccessOpens;
                uint32 go_max = go->GetGOInfo()->chest.maxSuccessOpens;

                // only vein pass this check
                if (go_min != 0 && go_max > go_min)
                {
                    float amount_rate = sWorld->GetRate(RATE_MINING_AMOUNT);
                    float min_amount = go_min*amount_rate;
                    float max_amount = go_max*amount_rate;

                    go->AddUse();
                    float uses = float(go->GetUseCount());

                    if (uses < max_amount)
                    {
                        if (uses >= min_amount)
                        {
                            float chance_rate = sWorld->GetRate(RATE_MINING_NEXT);

                            int32 ReqValue = 175;
                            LockEntry const *lockInfo = sLockStore.LookupEntry(go->GetGOInfo()->chest.lockId);
                            if (lockInfo)
                                ReqValue = lockInfo->requiredminingskill;
                            float skill = float(player->GetSkillValue(SKILL_MINING)) / (ReqValue + 25);
                            double chance = pow(0.8*chance_rate, 4 * (1 / double(max_amount))*double(uses));
                            if (roll_chance_f(100 * chance + skill))
                            {
                                go->SetLootState(GO_READY, player);
                            }
                            else                            // not have more uses
                                go->SetLootState(GO_JUST_DEACTIVATED, player);
                        }
                        else                                // 100% chance until min uses
                            go->SetLootState(GO_READY, player);
                    }
                    else                                    // max uses already
                        go->SetLootState(GO_JUST_DEACTIVATED, player);
                }
                else                                        // not vein
                    go->SetLootState(GO_JUST_DEACTIVATED, player);
            }
            else if (go->GetGoType() == GAMEOBJECT_TYPE_FISHINGHOLE)
            {                                               // The fishing hole used once more
                go->AddUse();                               // if the max usage is reached, will be despawned in next tick
                if (go->GetUseCount()>=irand(go->GetGOInfo()->fishinghole.minSuccessOpens,go->GetGOInfo()->fishinghole.maxSuccessOpens))
                {
                    go->SetLootState(GO_JUST_DEACTIVATED, player);
                }
                else
                    go->SetLootState(GO_READY, player);
            }
            else // not chest (or vein/herb/etc)
                go->SetLootState(GO_JUST_DEACTIVATED, player);

            loot->clear();
        }
        else
        {
            // not fully looted object
            go->SetLootState(GO_ACTIVATED, player);

            // if the round robin player release, reset it.
            if (player->GetGUID() == loot->roundRobinPlayer)
                loot->roundRobinPlayer.Clear();
        }
    }
    else if (lguid.IsCorpse())        // ONLY remove insignia at BG
    {
        Corpse *corpse = ObjectAccessor::GetCorpse(*player, lguid);
        if (!corpse || !corpse->IsWithinDistInMap(_player,INTERACTION_DISTANCE) )
            return;

        loot = &corpse->loot;

        // sunwell: Buggs client? (Opening loot after closing)
        //if (loot->isLooted())
        {
            loot->clear();
            corpse->RemoveFlag(CORPSE_FIELD_DYNAMIC_FLAGS, CORPSE_DYNFLAG_LOOTABLE);
        }
    }
    else if (lguid.IsItem())
    {
        Item *pItem = player->GetItemByGuid(lguid );
        if(!pItem)
            return;

        ItemTemplate const* proto = pItem->GetTemplate();
        if (proto->Flags & (ITEM_FLAG_IS_PROSPECTABLE
#ifdef LICH_KING
            | ITEM_FLAG_IS_MILLABLE
#endif
            ))
        {
            pItem->m_lootGenerated = false;
            pItem->loot.clear();

            uint32 count = pItem->GetCount();

            // >=5 checked in spell code, but will work for cheating cases also with removing from another stacks.
            if (count > 5)
                count = 5;

            player->DestroyItemCount(pItem, count, true);
        }
        else 
        {
            // Only delete item if no loot or money (unlooted loot is saved to db) or if it isn't an openable item
            if (pItem->loot.isLooted() || !(proto->Flags & ITEM_FLAG_HAS_LOOT))
                player->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
        }
        return;                                             // item can be looted only single player
    }
    else
    {
        Creature* creature = ObjectAccessor::GetCreature(*player, lguid);

        bool lootAllowed = creature && creature->IsAlive() == (player->GetClass() == CLASS_ROGUE && creature->loot.loot_type == LOOT_PICKPOCKETING);
        if (!lootAllowed || !creature->IsWithinDistInMap(_player, INTERACTION_DISTANCE))
            return;

        loot = &creature->loot;

        // update next looter
        if(Player *recipient = creature->GetLootRecipient())
            if(Group* group = recipient->GetGroup())
                if (group->GetLooterGuid() == player->GetGUID())
                    group->UpdateLooterGuid(creature);

        if (loot->isLooted())
        {
            // skip pickpocketing loot for speed, skinning timer redunction is no-op in fact
            if(!creature->IsAlive())
                creature->AllLootRemovedFromCorpse();

            creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
            loot->clear();
        }
        else
        {
            // if the round robin player release, reset it.
            if (player->GetGUID() == loot->roundRobinPlayer)
            {
                loot->roundRobinPlayer.Clear();

                if (Group* group = player->GetGroup())
                {
                    group->SendLooter(creature, nullptr);

                    // force update of dynamic flags, otherwise other group's players still not able to loot.
                    creature->ForceValuesUpdateAtIndex(UNIT_DYNAMIC_FLAGS);
                }
            }
        }
    }

    //Player is not looking at loot list, he doesn't need to see updates on the loot list
    loot->RemoveLooter(player->GetGUID());
}

void WorldSession::HandleLootMasterGiveOpcode( WorldPacket & recvData )
{
    uint8 slotid;
    ObjectGuid lootguid, target_playerguid;

    recvData >> lootguid >> slotid >> target_playerguid; //BC & LK ok

    if (!_player->GetGroup() || _player->GetGroup()->GetMasterLooterGuid() != _player->GetGUID() || _player->GetGroup()->GetLootMethod() != MASTER_LOOT)
    {
        _player->SendLootError(lootguid, LOOT_ERROR_DIDNT_KILL);
        return;
    }

    // player may be on other map
    Player* target = ObjectAccessor::GetPlayer(*_player, target_playerguid);
    if(!target)
    {
        _player->SendLootError(lootguid, LOOT_ERROR_PLAYER_NOT_FOUND);
        return;
    }

    if (_player->GetLootGUID() != lootguid)
    {
        _player->SendLootError(lootguid, LOOT_ERROR_DIDNT_KILL);
        return;
    }

    if (!_player->IsInRaidWith(target) || !_player->IsInMap(target))
    {
        _player->SendLootError(lootguid, LOOT_ERROR_MASTER_OTHER);
        TC_LOG_INFO("entities.player.cheat", "MasterLootItem: Player %s tried to give an item to ineligible player %s !", GetPlayer()->GetName().c_str(), target->GetName().c_str());
        return;
    }

    if (_player->IsAtGroupRewardDistance(target))
    {
        _player->SendLootError(lootguid, LOOT_ERROR_TOO_FAR); 
        return;
    }

    if (_player->GetInstanceId() != target->GetInstanceId())
    {
        _player->SendLootError(lootguid, LOOT_ERROR_MASTER_OTHER);
        return;
    }

    Loot *pLoot = nullptr;

    if(GetPlayer()->GetLootGUID().IsCreatureOrVehicle())
    {
        if(Creature *pCreature = ObjectAccessor::GetCreature(*GetPlayer(), lootguid))
            pLoot = &pCreature->loot;
    }
    else if(GetPlayer()->GetLootGUID().IsGameObject())
    {
        if(GameObject *pGO = ObjectAccessor::GetGameObject(*GetPlayer(), lootguid))
           pLoot = &pGO->loot;
    }

    if(!pLoot)
    {
        _player->SendLootError(lootguid, LOOT_ERROR_MASTER_OTHER);
        return;
    }

    if (slotid >= pLoot->items.size() + pLoot->quest_items.size())
    {
        TC_LOG_ERROR("misc","AutoLootItem: Player %s might be using a hack! (slot %u, size %u)",
            GetPlayer()->GetName().c_str(), slotid, (uint32)pLoot->items.size());
        return;
    }

    LootItem& item = slotid >= pLoot->items.size() ? pLoot->quest_items[slotid - pLoot->items.size()] : pLoot->items[slotid];

    ItemPosCountVec dest;
    uint8 msg = target->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, item.itemid, item.count );
    if (item.follow_loot_rules && !item.AllowedForPlayer(target))
        msg = EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM;
    if (msg != EQUIP_ERR_OK)
    {
        if (msg == EQUIP_ERR_CANT_CARRY_MORE_OF_THIS)
            _player->SendLootError(lootguid, LOOT_ERROR_MASTER_UNIQUE_ITEM);
        else if (msg == EQUIP_ERR_INVENTORY_FULL)
            _player->SendLootError(lootguid, LOOT_ERROR_MASTER_INV_FULL);
        else
            _player->SendLootError(lootguid, LOOT_ERROR_MASTER_OTHER);

        target->SendEquipError( msg, nullptr, nullptr );
        return;
    }

    // not move item from loot to target inventory
    Item * newitem = target->StoreNewItem(dest, item.itemid, true, item.randomPropertyId);
    target->SendNewItem(newitem, uint32(item.count), false, false, true);
#ifdef LICH_KING
    target->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM, item.itemid, item.count);
    target->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE, loot->loot_type, item.count);
    target->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM, item.itemid, item.count)
#endif

    // mark as looted
    item.count=0;
    item.is_looted=true;

    pLoot->NotifyItemRemoved(slotid);
    --pLoot->unlootedCount;
}
