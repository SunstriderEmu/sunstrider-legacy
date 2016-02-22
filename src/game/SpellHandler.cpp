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

#include "Common.h"
#include "DBCStores.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "BattleGround.h"
#include "MapManager.h"
#include "ScriptCalls.h"
#include "Totem.h"
#include "TemporarySummon.h"
#include "ScriptMgr.h"
#include "GameObjectAI.h"

void WorldSession::HandleUseItemOpcode(WorldPacket& recvPacket)
{
    PROFILE;
    
    // TODO: add targets.read() check
    CHECK_PACKET_SIZE(recvPacket,1+1+1+1+8);

    Player* pUser = _player;
    uint8 bagIndex, slot;
    uint8 spell_count;                                      // number of spells at item, not used
    uint8 cast_count;                                       // next cast if exists (single or not)
    uint64 item_guid;

    recvPacket >> bagIndex >> slot >> spell_count >> cast_count >> item_guid;

    // ignore for remote control state
    if (pUser->m_mover != pUser)
        return;

    Item *pItem = pUser->GetItemByPos(bagIndex, slot);
    if(!pItem)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
        return;
    }

    if(pItem->GetGUID() != item_guid)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
        return;
    }

    TC_LOG_DEBUG("network","WORLD: CMSG_USE_ITEM packet, bagIndex: %u, slot: %u, spell_count: %u , cast_count: %u, Item: %u, data length = %i", bagIndex, slot, spell_count, cast_count, pItem->GetEntry(), uint32(recvPacket.size()));

    ItemTemplate const *proto = pItem->GetTemplate();
    if(!proto)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, NULL );
        return;
    }

    // some item classes can be used only in equipped state
    if(proto->InventoryType != INVTYPE_NON_EQUIP && !pItem->IsEquipped())
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, NULL );
        return;
    }

    uint8 msg = pUser->CanUseItem(pItem);
    if( msg != EQUIP_ERR_OK )
    {
        pUser->SendEquipError( msg, pItem, NULL );
        return;
    }

    // only allow conjured consumable, bandage, poisons (all should have the 2^21 item flag set in DB)
    if( proto->Class == ITEM_CLASS_CONSUMABLE &&
        !(proto->Flags & ITEM_FLAG_USEABLE_IN_ARENA) &&
        pUser->InArena())
    {
        pUser->SendEquipError(EQUIP_ERR_NOT_DURING_ARENA_MATCH,pItem,NULL);
        return;
    }

    if (pUser->IsInCombat())
    {
        for(int i = 0; i < 5; ++i)
        {
            if (SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(proto->Spells[i].SpellId))
            {
                if (!spellInfo->CanBeUsedInCombat())
                {
                    pUser->SendEquipError(EQUIP_ERR_NOT_IN_COMBAT,pItem,NULL);
                    return;
                }
            }
        }
    }

    // check also  BIND_WHEN_PICKED_UP and BIND_QUEST_ITEM for .additem or .additemset case by GM (not binded at adding to inventory)
    if( pItem->GetTemplate()->Bonding == BIND_WHEN_USE || pItem->GetTemplate()->Bonding == BIND_WHEN_PICKED_UP || pItem->GetTemplate()->Bonding == BIND_QUEST_ITEM )
    {
        if (!pItem->IsSoulBound())
        {
            pItem->SetState(ITEM_CHANGED, pUser);
            pItem->SetBinding( true );
        }
    }

    SpellCastTargets targets;
    if(!targets.read(&recvPacket, pUser))
        return;

    //Note: If script stop casting it must send appropriate data to client to prevent stuck item in gray state.
    if(!sScriptMgr->ItemUse(pUser,pItem,targets))
    {
        // no script or script not process request by self

        // special learning case
        if(pItem->GetTemplate()->Spells[0].SpellId==SPELL_ID_GENERIC_LEARN)
        {
            uint32 learning_spell_id = pItem->GetTemplate()->Spells[1].SpellId;

            SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(SPELL_ID_GENERIC_LEARN);
            if(!spellInfo)
            {
                TC_LOG_ERROR("FIXME","Item (Entry: %u) in have wrong spell id %u, ignoring ",proto->ItemId, SPELL_ID_GENERIC_LEARN);
                pUser->SendEquipError(EQUIP_ERR_NONE,pItem,NULL);
                return;
            }

            Spell *spell = new Spell(pUser, spellInfo, false);
            spell->m_CastItem = pItem;
            spell->m_cast_count = cast_count;               //set count of casts
            spell->m_currentBasePoints[0] = learning_spell_id;
            spell->prepare(&targets);
            return;
        }

        // use triggered flag only for items with many spell casts and for not first cast
        int count = 0;

        for(int i = 0; i < 5; ++i)
        {
            _Spell const& spellData = pItem->GetTemplate()->Spells[i];

            // no spell
            if(!spellData.SpellId)
                continue;

            // wrong triggering type
            if( spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE && spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_NO_DELAY_USE)
                continue;

            SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellData.SpellId);
            if(!spellInfo)
            {
                TC_LOG_ERROR("FIXME","Item (Entry: %u) in have wrong spell id %u, ignoring ",proto->ItemId, spellData.SpellId);
                continue;
            }

            Spell *spell = new Spell(pUser, spellInfo, (count > 0));
            spell->m_CastItem = pItem;
            spell->m_cast_count = cast_count;               //set count of casts
            spell->prepare(&targets);

            ++count;
        }
    }
}

#define OPEN_CHEST 11437
#define OPEN_SAFE 11535
#define OPEN_CAGE 11792
#define OPEN_BOOTY_CHEST 5107
#define OPEN_STRONGBOX 8517

void WorldSession::HandleOpenItemOpcode(WorldPacket& recvPacket)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvPacket,1+1);

    TC_LOG_DEBUG("network","WORLD: CMSG_OPEN_ITEM packet, data length = %u", (uint32)recvPacket.size());

    Player* pUser = _player;
    uint8 bagIndex, slot;

    // ignore for remote control state
    if (pUser->m_mover != pUser)
        return;

    recvPacket >> bagIndex >> slot;

    TC_LOG_DEBUG("network","bagIndex: %u, slot: %u",bagIndex,slot);

    Item *pItem = pUser->GetItemByPos(bagIndex, slot);
    if(!pItem)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, NULL, NULL );
        return;
    }

    ItemTemplate const *proto = pItem->GetTemplate();
    if(!proto)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, NULL );
        return;
    }

    if (pItem->IsBag())
    {
        pUser->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, pItem, NULL);
        return;
    }

    // locked item
    uint32 lockId = proto->LockID;
    if(lockId)
    {
        LockEntry const *lockInfo = sLockStore.LookupEntry(lockId);

        if (!lockInfo)
        {
            pUser->SendEquipError(EQUIP_ERR_ITEM_LOCKED, pItem, NULL );
            TC_LOG_ERROR("network", "WORLD::OpenItem: item [guid = %u] has an unknown lockId: %u!", pItem->GetGUIDLow() , lockId);
            return;
        }

        // required picklocking
        if (lockInfo->requiredminingskill)
        {
            pUser->SendEquipError(EQUIP_ERR_ITEM_LOCKED, pItem, NULL );
            return;
        }
        
        if (lockInfo->requiredlockskill) 
        {
            // check key
            for (int i = 0; i < 5; ++i) 
            {
                // type==1 This means lockInfo->key[i] is an item
                if (lockInfo->Type[i]==LOCK_KEY_ITEM && lockInfo->Index[i]) {
                    if (pUser->HasItemCount(lockInfo->Index[i], 1, false)) {
                        break; //found
                    }
                }
                
                //not found
                pUser->SendEquipError(EQUIP_ERR_ITEM_LOCKED, pItem, NULL );
                return;
            }
        }
    }

    if(pItem->HasFlag(ITEM_FIELD_FLAGS, ITEM_FLAG_WRAPPED))// wrapped?
    {
        QueryResult result = CharacterDatabase.PQuery("SELECT entry, flags FROM character_gifts WHERE item_guid = '%u'", pItem->GetGUIDLow());
        if (result)
        {
            Field *fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            uint32 flags = fields[1].GetUInt32();

            pItem->SetUInt64Value(ITEM_FIELD_GIFTCREATOR, 0);
            pItem->SetEntry(entry);
            pItem->SetUInt32Value(ITEM_FIELD_FLAGS, flags);
            pItem->SetState(ITEM_CHANGED, pUser);
        }
        else
        {
            TC_LOG_ERROR("FIXME","Wrapped item %u don't have record in character_gifts table and will deleted", pItem->GetGUIDLow());
            pUser->DestroyItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
            return;
        }
        CharacterDatabase.PExecute("DELETE FROM character_gifts WHERE item_guid = '%u'", pItem->GetGUIDLow());
    }
    else
        pUser->SendLoot(pItem->GetGUID(),LOOT_CORPSE);
}

void WorldSession::HandleGameObjectUseOpcode( WorldPacket & recvData )
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8);

    uint64 guid;

    recvData >> guid;

    GameObject *obj = _player->GetGameObjectIfCanInteractWith(guid);

    if(!obj)
        return;

    // ignore for remote control state
    if (GetPlayer()->m_mover != GetPlayer())
        return;

    if (sScriptMgr->OnGossipHello(_player, obj))
        return;
        
    obj->AI()->OnGossipHello(_player);

    obj->Use(_player);

    //TC LK _player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT, go->GetEntry());
}

void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvPacket,4+1+2);

    // ignore for remote control state (for player case)
    Unit* mover = _player->m_mover;
    if (mover != _player && mover->GetTypeId() == TYPEID_PLAYER)
    {
        recvPacket.rfinish(); // prevent spam at ignore packet
        return;
    }

    uint32 spellId;
    uint8  cast_count;
    recvPacket >> spellId;
    recvPacket >> cast_count;

    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId );

    if(!spellInfo)
    {
        TC_LOG_ERROR("network","WORLD: unknown spell id %u", spellId);
        return;
    }

    // not have spell or spell passive and not casted by client
    if ( !_player->HasSpell(spellId) || spellInfo->IsPassive() )
    {
        //cheater? kick? ban?
        return;
    }

    // can't use our own spells when we're in possession of another unit,
    if(_player->IsPossessing())
        return;

    // client provided targets
    SpellCastTargets targets;
    if(!targets.read(&recvPacket,_player))
        return;

    // auto-selection buff level base at target level (in spellInfo)
    if(targets.GetUnitTarget())
    {
        SpellInfo const *actualSpellInfo = sSpellMgr->SelectAuraRankForPlayerLevel(spellInfo,targets.GetUnitTarget()->GetLevel(),_player->IsHostileTo(targets.GetUnitTarget()));

        // if rank not found then function return NULL but in explicit cast case original spell can be casted and later failed with appropriate error message
        if(actualSpellInfo)
            spellInfo = actualSpellInfo;
    } else if (spellInfo->Effects[0].ApplyAuraName == SPELL_AURA_BIND_SIGHT) {
        //client doesn't send target if it's not in range, so we have to pick it from UNIT_FIELD_TARGET
        if(uint64 targetGUID = _player->GetUInt64Value(UNIT_FIELD_TARGET))
            if(Unit* target = ObjectAccessor::GetUnit(*_player, targetGUID))
                targets.SetUnitTarget(target);
    }
    
    if (spellInfo->HasAttribute(SPELL_ATTR2_AUTOREPEAT_FLAG))
    {
        if (_player->m_currentSpells[CURRENT_AUTOREPEAT_SPELL] && _player->m_currentSpells[CURRENT_AUTOREPEAT_SPELL]->m_spellInfo->Id == spellInfo->Id)
            return;
    }

    Spell *spell = new Spell(_player, spellInfo, false);
    spell->m_cast_count = cast_count;                       // set count of casts
    spell->prepare(&targets);
}

void WorldSession::HandleCancelCastOpcode(WorldPacket& recvPacket)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvPacket,4);

    uint32 spellId;
    recvPacket >> spellId;

    if(_player->IsNonMeleeSpellCast(false))
        _player->InterruptNonMeleeSpells(false,spellId,false);
}

void WorldSession::HandleCancelAuraOpcode( WorldPacket& recvPacket)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvPacket,4);

    uint32 spellId;
    recvPacket >> spellId;

    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return;

    // Don't allow to remove spells with attr SPELL_ATTR0_CANT_CANCEL
    if (spellInfo->Attributes & SPELL_ATTR0_CANT_CANCEL)
        return;

    // channeled spell case (it currently casted then)
    if (spellInfo->IsChanneled()) {
        if (Spell* spell = _player->m_currentSpells[CURRENT_CHANNELED_SPELL]) {
            if (spell->m_spellInfo->Id==spellId)
                spell->cancel();
        }
        
        return;
    }

    // non channeled case
    // don't allow remove non positive spells
    // don't allow cancelling passive auras (some of them are visible)
    if (!spellInfo->IsPositive() /* || spellInfo->IsPassive()*/)
        return;

    _player->RemoveAurasDueToSpellByCancel(spellId);
}

void WorldSession::HandlePetCancelAuraOpcode( WorldPacket& recvPacket)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvPacket, 8+4);

    uint64 guid;
    uint32 spellId;

    recvPacket >> guid;
    recvPacket >> spellId;

    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId );
    if(!spellInfo)
    {
        TC_LOG_ERROR("network", "WORLD: unknown PET spell id %u", spellId);
        return;
    }

    Creature* pet=ObjectAccessor::GetCreatureOrPetOrVehicle(*_player,guid);

    if(!pet)
    {
        TC_LOG_ERROR( "network", "Pet %u not exist.", uint32(GUID_LOPART(guid)) );
        return;
    }

    if(pet != GetPlayer()->GetPet() && pet != GetPlayer()->GetCharm())
    {
        TC_LOG_ERROR( "network", "HandlePetCancelAura.Pet %u isn't pet of player %s", uint32(GUID_LOPART(guid)),GetPlayer()->GetName().c_str() );
        return;
    }

    if(!pet->IsAlive())
    {
        pet->SendPetActionFeedback(FEEDBACK_PET_DEAD);
        return;
    }

    pet->RemoveAurasDueToSpell(spellId);

    pet->AddCreatureSpellCooldown(spellId);
}

void WorldSession::HandleCancelGrowthAuraOpcode( WorldPacket& /*recvPacket*/)
{
    // nothing do
}

void WorldSession::HandleCancelAutoRepeatSpellOpcode( WorldPacket& recvPacket)
{
    CHECK_PACKET_SIZE(recvPacket, 0);

    // cancel and prepare for deleting
    _player->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
}

void WorldSession::HandleCancelChanneling( WorldPacket & recvData )
{
    CHECK_PACKET_SIZE(recvData, 4);

    // ignore for remote control state (for player case)
    Unit* mover = _player->m_mover;
    if (mover != _player && mover->GetTypeId() == TYPEID_PLAYER)
        return;

    uint32 spellId;
    recvData >> spellId;

    _player->InterruptNonMeleeSpells(false, spellId, false);
}

void WorldSession::HandleTotemDestroyed( WorldPacket& recvPacket)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvPacket, 1);

    // ignore for remote control state
    if (_player->m_mover != _player)
        return;

    uint8 slotId;

    recvPacket >> slotId;

    if (slotId >= MAX_TOTEM)
        return;

    if(!_player->m_TotemSlot[slotId])
        return;

    Creature* totem = ObjectAccessor::GetCreature(*_player,_player->m_TotemSlot[slotId]);
    // Don't unsummon sentry totem
    if(totem && totem->IsTotem() && totem->GetEntry() != SENTRY_TOTEM_ENTRY)
        ((Totem*)totem)->UnSummon();
}

void WorldSession::HandleSelfResOpcode( WorldPacket & /* recvData */)
{
    PROFILE;
    
//    CHECK_PACKET_SIZE(recvData, 0);

    if(_player->GetUInt32Value(PLAYER_SELF_RES_SPELL))
    {
        SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(_player->GetUInt32Value(PLAYER_SELF_RES_SPELL));
        if(spellInfo)
            _player->CastSpell(_player,spellInfo,false,0);

        _player->SetUInt32Value(PLAYER_SELF_RES_SPELL, 0);
    }
}

void WorldSession::HandleMirrorImageDataRequest(WorldPacket& recvData)
{
    PROFILE;
    
    CHECK_PACKET_SIZE(recvData, 8);

    uint64 guid;
    recvData >> guid;

    // Get unit for which data is needed by client
    Unit* unit = ObjectAccessor::GetObjectInWorld(guid, (Unit*)NULL);
    if (!unit)
        return;

    if (!unit->HasAuraType(SPELL_AURA_CLONE_CASTER))
        return;

    // Get creator of the unit (SPELL_AURA_CLONE_CASTER does not stack)
    Unit* creator = unit->GetAurasByType(SPELL_AURA_CLONE_CASTER).front()->GetCaster();
    if (!creator)
        return;

    WorldPacket data(SMSG_MIRRORIMAGE_DATA, 68);
    data << uint64(guid);
    data << uint32(creator->GetDisplayId());
    data << uint8(creator->GetRace());
    data << uint8(creator->GetGender());
    //data << uint8(creator->GetClass());                       // added in 3.x

    if (creator->GetTypeId() == TYPEID_PLAYER)
    {
        Player* player = creator->ToPlayer();
        data << uint8(player->GetByteValue(PLAYER_BYTES, 0));   // skin
        data << uint8(player->GetByteValue(PLAYER_BYTES, 1));   // face
        data << uint8(player->GetByteValue(PLAYER_BYTES, 2));   // hair
        data << uint8(player->GetByteValue(PLAYER_BYTES, 3));   // haircolor
        data << uint8(player->GetByteValue(PLAYER_BYTES_2, 0)); // facialhair
        data << uint32(player->GetGuildId());                   // guildId

        static EquipmentSlots const itemSlots[] =
        {
            EQUIPMENT_SLOT_HEAD,
            EQUIPMENT_SLOT_SHOULDERS,
            EQUIPMENT_SLOT_BODY,
            EQUIPMENT_SLOT_CHEST,
            EQUIPMENT_SLOT_WAIST,
            EQUIPMENT_SLOT_LEGS,
            EQUIPMENT_SLOT_FEET,
            EQUIPMENT_SLOT_WRISTS,
            EQUIPMENT_SLOT_HANDS,
            EQUIPMENT_SLOT_BACK,
            EQUIPMENT_SLOT_TABARD,
            EQUIPMENT_SLOT_END
        };

        // Display items in visible slots
        for (EquipmentSlots const* itr = &itemSlots[0]; *itr != EQUIPMENT_SLOT_END; ++itr)
        {
            if (*itr == EQUIPMENT_SLOT_HEAD && player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_HELM))
                data << uint32(0);
            else if (*itr == EQUIPMENT_SLOT_BACK && player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_HIDE_CLOAK))
                data << uint32(0);
            else if (Item const* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, *itr))
                data << uint32(item->GetTemplate()->DisplayInfoID);
            else
                data << uint32(0);
        }
    }
    else
    {
        // Skip player data for creatures
        data << uint8(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
        data << uint32(0);
    }

    SendPacket(&data);
}
