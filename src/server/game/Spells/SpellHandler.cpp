
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
#include "Totem.h"
#include "TemporarySummon.h"
#include "ScriptMgr.h"
#include "GameObjectAI.h"
#include "SpellAuraEffects.h"

void WorldSession::HandleUseItemOpcode(WorldPacket& recvPacket)
{
    uint8 bagIndex, slot;
    uint8 spell_count;                                      // number of spells at item, not used
    uint8 cast_count;                                       // next cast if exists (single or not)
    ObjectGuid item_guid;

#ifdef LICH_KING
    recvPacket >> bagIndex >> slot >> castCount >> spellId >> itemGUID >> glyphIndex >> castFlags;
#else
    recvPacket >> bagIndex >> slot >> spell_count >> cast_count >> item_guid;
#endif

    SpellCastTargets targets;
    targets.Read(recvPacket, _player);

    _HandleUseItemOpcode(bagIndex, slot, spell_count, cast_count, item_guid, targets);
    TC_LOG_DEBUG("network", "WORLD: CMSG_USE_ITEM packet, bagIndex: %u, slot: %u, spell_count: %u , cast_count: %u, Item: %u, data length = %i", bagIndex, slot, spell_count, cast_count, item_guid.GetCounter(), uint32(recvPacket.size()));
}

bool WorldSession::_HandleUseItemOpcode(uint8 bagIndex, uint8 slot, uint8 spell_count, uint8 cast_count, ObjectGuid item_guid, SpellCastTargets targets)
{
    Player* pUser = _player;

    // ignore for remote control state
    if (_activeMover != pUser)
        return false;

    Item* pItem = pUser->GetUseableItemByPos(bagIndex, slot);
    if (!pItem)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr);
        return false;
    }

    if (pItem->GetGUID() != item_guid)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr);
        return false;
    }

    ItemTemplate const *proto = pItem->GetTemplate();
    if (!proto)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, nullptr);
        return false;
    }

    // some item classes can be used only in equipped state
    if (proto->InventoryType != INVTYPE_NON_EQUIP && !pItem->IsEquipped())
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, nullptr);
        return false;
    }

    uint8 msg = pUser->CanUseItem(pItem);
    if (msg != EQUIP_ERR_OK)
    {
        pUser->SendEquipError(msg, pItem, nullptr);
        return false;
    }

    // only allow conjured consumable, bandage, poisons (all should have the 2^21 item flag set in DB)
    if (proto->Class == ITEM_CLASS_CONSUMABLE &&
        !(proto->Flags & ITEM_FLAG_USEABLE_IN_ARENA) &&
        pUser->InArena())
    {
        pUser->SendEquipError(EQUIP_ERR_NOT_DURING_ARENA_MATCH, pItem, nullptr);
        return false;
    }

    // don't allow items banned in arena
    if ((proto->Flags & ITEM_FLAG_NOT_USEABLE_IN_ARENA) && pUser->InArena())
    {
        pUser->SendEquipError(EQUIP_ERR_NOT_DURING_ARENA_MATCH, pItem, nullptr);
        return false;
    }

    if (pUser->IsInCombat())
    {
        for (const auto & Spell : proto->Spells)
        {
            if (SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(Spell.SpellId))
            {
                if (!spellInfo->CanBeUsedInCombat())
                {
                    pUser->SendEquipError(EQUIP_ERR_NOT_IN_COMBAT, pItem, nullptr);
                    return false;
                }
            }
        }
    }

    // check also  BIND_WHEN_PICKED_UP and BIND_QUEST_ITEM for .additem or .additemset case by GM (not binded at adding to inventory)
    if (pItem->GetTemplate()->Bonding == BIND_WHEN_USE || pItem->GetTemplate()->Bonding == BIND_WHEN_PICKED_UP || pItem->GetTemplate()->Bonding == BIND_QUEST_ITEM)
    {
        if (!pItem->IsSoulBound())
        {
            pItem->SetState(ITEM_CHANGED, pUser);
            pItem->SetBinding(true);
        }
    }

    //Note: If script stop casting it must send appropriate data to client to prevent stuck item in gray state.
    if (!sScriptMgr->OnItemUse(pUser, pItem, targets))
    {
        // no script or script not process request by self
        return pUser->CastItemUseSpell(pItem, targets, cast_count);
    }
    return true;
}

#define OPEN_CHEST 11437
#define OPEN_SAFE 11535
#define OPEN_CAGE 11792
#define OPEN_BOOTY_CHEST 5107
#define OPEN_STRONGBOX 8517

void WorldSession::HandleOpenItemOpcode(WorldPacket& recvPacket)
{
    //TC_LOG_DEBUG("network","WORLD: CMSG_OPEN_ITEM packet, data length = %u", (uint32)recvPacket.size());

    Player* pUser = _player;
    uint8 bagIndex, slot;

    // ignore for remote control state
    if (_activeMover != pUser)
        return;

    recvPacket >> bagIndex >> slot;

    //TC_LOG_DEBUG("network","bagIndex: %u, slot: %u",bagIndex,slot);

    Item *pItem = pUser->GetItemByPos(bagIndex, slot);
    if(!pItem)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, nullptr, nullptr );
        return;
    }

    ItemTemplate const *proto = pItem->GetTemplate();
    if(!proto)
    {
        pUser->SendEquipError(EQUIP_ERR_ITEM_NOT_FOUND, pItem, nullptr );
        return;
    }

    if (pItem->IsBag())
    {
        pUser->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, pItem, nullptr);
        return;
    }

    // locked item
    uint32 lockId = proto->LockID;
    if(lockId)
    {
        LockEntry const *lockInfo = sLockStore.LookupEntry(lockId);

        if (!lockInfo)
        {
            pUser->SendEquipError(EQUIP_ERR_ITEM_LOCKED, pItem, nullptr );
            TC_LOG_ERROR("network", "WORLD::OpenItem: item [guid = %u] has an unknown lockId: %u!", pItem->GetGUID().GetCounter() , lockId);
            return;
        }

        // required picklocking
        if (lockInfo->requiredminingskill)
        {
            pUser->SendEquipError(EQUIP_ERR_ITEM_LOCKED, pItem, nullptr );
            return;
        }
        
        if (lockInfo->requiredlockskill) 
        {
            // check key
            bool isFound = false;
            for (int i = 0; i < 5; ++i) 
            {
                // type==1 This means lockInfo->key[i] is an item
                if (lockInfo->Type[i]==LOCK_KEY_ITEM && lockInfo->Index[i]) {
                    if (pUser->HasItemCount(lockInfo->Index[i], 1, false)) {
                        isFound = true;
                        break; //found
                    }
                }
            }

            //not found
            if(!isFound) 
            { 
                pUser->SendEquipError(EQUIP_ERR_ITEM_LOCKED, pItem, nullptr);
                return;
            }
        }
    }

    if(pItem->HasFlag(ITEM_FIELD_FLAGS, ITEM_FIELD_FLAG_WRAPPED))// wrapped?
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_GIFT_BY_ITEM);
        stmt->setUInt32(0, pItem->GetGUID().GetCounter());
        _queryProcessor.AddQuery(CharacterDatabase.AsyncQuery(stmt)
            .WithPreparedCallback(std::bind(&WorldSession::HandleOpenWrappedItemCallback, this, pItem->GetPos(), pItem->GetGUID(), std::placeholders::_1)));
    }
    else
        pUser->SendLoot(pItem->GetGUID(), LOOT_CORPSE);
}

void WorldSession::HandleOpenWrappedItemCallback(uint16 pos, ObjectGuid itemGuid, PreparedQueryResult result)
{
    if (!GetPlayer())
        return;

    Item* item = GetPlayer()->GetItemByPos(pos);
    if (!item)
        return;

    if (item->GetGUID() != itemGuid || !item->HasFlag(ITEM_FIELD_FLAGS, ITEM_FIELD_FLAG_WRAPPED)) // during getting result, gift was swapped with another item
        return;

    if (!result)
    {
        TC_LOG_ERROR("network", "Wrapped item %u don't have record in character_gifts table and will deleted", itemGuid.GetCounter());
        GetPlayer()->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
        return;
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    Field* fields = result->Fetch();
    uint32 entry = fields[0].GetUInt32();
    uint32 flags = fields[1].GetUInt32();

    item->SetGuidValue(ITEM_FIELD_GIFTCREATOR, ObjectGuid::Empty);
    item->SetEntry(entry);
    item->SetUInt32Value(ITEM_FIELD_FLAGS, flags);
    item->SetUInt32Value(ITEM_FIELD_MAXDURABILITY, item->GetTemplate()->MaxDurability);
    item->SetState(ITEM_CHANGED, GetPlayer());

    GetPlayer()->SaveInventoryAndGoldToDB(trans);

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GIFT);
    stmt->setUInt32(0, itemGuid.GetCounter());
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

void WorldSession::HandleGameObjectUseOpcode( WorldPacket & recvData )
{
    ObjectGuid guid;

    recvData >> guid;

    GameObject* gameObjTarget = _player->GetGameObjectIfCanInteractWith(guid);

    if(!gameObjTarget)
        return;

    // Players shouldn't be able to loot gameobjects that are currently despawned
    if (!gameObjTarget->isSpawned() && !_player->IsGameMaster())
    {
        TC_LOG_ERROR("entities.player.cheat", "Possible hacking attempt: Player %s [guid: %u] tried to loot a gameobject [entry: %u id: %u] which is on respawn timer without being in GM mode!",
            _player->GetName().c_str(), _player->GetGUID().GetCounter(), gameObjTarget->GetEntry(), gameObjTarget->GetGUID().GetCounter());
        return;
    }

    // ignore for remote control state
    if (_activeMover != GetPlayer())
        return;

    gameObjTarget->Use(_player);
}

void WorldSession::HandleCastSpellOpcode(WorldPacket& recvPacket)
{
    // ignore for remote control state (for player case)
    Unit* mover = _activeMover;
    if (!mover || (mover != _player && mover->GetTypeId() == TYPEID_PLAYER))
    {
        recvPacket.rfinish(); // prevent spam at ignore packet
        return;
    }

    uint32 spellId;
    uint8  cast_count;
    recvPacket >> spellId;
    recvPacket >> cast_count;
#ifdef LICH_KING
    recvPacket >> castFlags;
#endif

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
        recvPacket.rfinish(); // prevent spam at ignore packet
        return;
    }

    // can't use our own spells when we're in possession of another unit,
    if (_player->IsPossessing())
    {
        recvPacket.rfinish(); // prevent spam at ignore packet
        return;
    }

    // client provided targets
    SpellCastTargets targets;
    targets.Read(recvPacket, _player);

    // Not sure needed on BC
    // Client is resending autoshot cast opcode when other spell is cast during shoot rotation
    // Skip it to prevent "interrupt" message
    // Also check targets! target may have changed and we need to interrupt current spell
    if (spellInfo->IsAutoRepeatRangedSpell())
    {
        if (Spell* spell = _player->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL))
        {
            if (spell->m_spellInfo == spellInfo && spell->m_targets.GetUnitTargetGUID() == targets.GetUnitTargetGUID())
            {
                recvPacket.rfinish();
                return;
            }
        }
    }

    // auto-selection buff level base at target level (in spellInfo)
    if(targets.GetUnitTarget())
    {
        if (spellInfo->IsPositive())
        {
            if (Unit* target = targets.GetUnitTarget())
            {
                SpellInfo const* actualSpellInfo = spellInfo->GetAuraRankForLevel(target->GetLevel());

                // if rank not found then function return NULL but in explicit cast case original spell can be cast and later failed with appropriate error message
                if (actualSpellInfo)
                    spellInfo = actualSpellInfo;
            }
        }
    } else if (spellInfo->Effects[0].ApplyAuraName == SPELL_AURA_BIND_SIGHT) 
    {
        //client doesn't send target if it's not in range, so we have to pick it from UNIT_FIELD_TARGET
        if(ObjectGuid targetGUID = _player->GetGuidValue(UNIT_FIELD_TARGET))
            if(Unit* target = ObjectAccessor::GetUnit(*_player, targetGUID))
                targets.SetUnitTarget(target);
    }   
    
    if (spellInfo->IsAutoRepeatRangedSpell())
    {
        // Client is resending autoshot cast opcode when other spell is casted during shoot rotation
        // Skip it to prevent "interrupt" message
        if (_player->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL) && _player->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->m_spellInfo->Id == spellInfo->Id)
            return;
    }

    auto spell = new Spell(_player, spellInfo, TRIGGERED_NONE);
    spell->m_cast_count = cast_count;                       // set count of casts
    spell->prepare(targets);
}

void WorldSession::HandleCancelCastOpcode(WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;

    if (_player->IsNonMeleeSpellCast(false))
        _player->InterruptNonMeleeSpells(false, spellId, false);
}

void WorldSession::HandleCancelAuraOpcode(WorldPacket& recvPacket)
{
    uint32 spellId;
    recvPacket >> spellId;

    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return;

    // Don't allow to remove spells with attr SPELL_ATTR0_CANT_CANCEL
    if (spellInfo->Attributes & SPELL_ATTR0_CANT_CANCEL)
        return;

    // channeled spell case (it currently casted then)
    if (spellInfo->IsChanneled()) 
    {
        if (Spell* spell = _player->GetCurrentSpell(CURRENT_CHANNELED_SPELL)) 
            if (spell->m_spellInfo->Id==spellId)
                spell->cancel();
        
        return;
    }

    // non channeled case
    // don't allow remove non positive spells
    // don't allow cancelling passive auras (some of them are visible)
    if (!spellInfo->IsPositive() || spellInfo->IsPassive())
        return;

    // maybe should only remove one buff when there are multiple?
    _player->RemoveOwnedAura(spellId, ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
}

void WorldSession::HandlePetCancelAuraOpcode(WorldPacket& recvPacket)
{
    ObjectGuid guid;
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
        TC_LOG_ERROR( "network", "Pet %u not exist.", uint32(guid.GetCounter()) );
        return;
    }

    if(pet != GetPlayer()->GetGuardianPet() && pet != GetPlayer()->GetCharmed())
    {
        TC_LOG_ERROR( "network", "HandlePetCancelAura.Pet %u isn't pet of player %s", uint32(guid.GetCounter()),GetPlayer()->GetName().c_str() );
        return;
    }

    if(!pet->IsAlive())
    {
        pet->SendPetActionFeedback(FEEDBACK_PET_DEAD);
        return;
    }

    pet->RemoveOwnedAura(spellId, ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);
}

void WorldSession::HandleCancelGrowthAuraOpcode(WorldPacket& /*recvPacket*/)
{
    // nothing do
}

void WorldSession::HandleCancelAutoRepeatSpellOpcode(WorldPacket& recvPacket)
{
    // cancel and prepare for deleting
    _player->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
}

void WorldSession::HandleCancelChanneling(WorldPacket & recvData)
{
    // ignore for remote control state (for player case)
    if (!_activeMover || (_activeMover != _player && _activeMover->GetTypeId() == TYPEID_PLAYER))
        return;

    uint32 spellId;
    recvData >> spellId;

    _player->InterruptNonMeleeSpells(false, spellId, false);
}

void WorldSession::HandleTotemDestroyed( WorldPacket& recvPacket)
{
    // ignore for remote control state
    if (_activeMover != _player)
        return;

    uint8 slotId;

    recvPacket >> slotId;

    if (slotId >= MAX_TOTEM_SLOT)
        return;

    if(!_player->m_SummonSlot[slotId])
        return;

    Creature* totem = ObjectAccessor::GetCreature(*_player, _player->m_SummonSlot[slotId]);

    // Don't unsummon sentry totem
    if(totem && totem->IsTotem() && totem->GetEntry() != SENTRY_TOTEM_ENTRY)
        ((Totem*)totem)->UnSummon();
}

void WorldSession::HandleSelfResOpcode( WorldPacket & /* recvData */)
{
    if(_player->GetUInt32Value(PLAYER_SELF_RES_SPELL))
    {
        SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(_player->GetUInt32Value(PLAYER_SELF_RES_SPELL));
        if(spellInfo)
            _player->CastSpell(_player, spellInfo->Id);

        _player->SetUInt32Value(PLAYER_SELF_RES_SPELL, 0);
    }
}

void WorldSession::HandleMirrorImageDataRequest(WorldPacket& recvData)
{
    ObjectGuid guid;
    recvData >> guid;

    // Get unit for which data is needed by client
    Unit* unit = _player->GetMap()->GetCreature(guid);
    if (!unit)
        return;

    if (!unit->HasAuraType(SPELL_AURA_CLONE_CASTER))
        return;

    // Get creator of the unit (SPELL_AURA_CLONE_CASTER does not stack)
    Unit* creator = unit->GetAuraEffectsByType(SPELL_AURA_CLONE_CASTER).front()->GetCaster();
    if (!creator)
        return;

    WorldPacket data(SMSG_MIRRORIMAGE_DATA, 68); //LK OK
    data << uint64(guid);
    data << uint32(creator->GetDisplayId());
    data << uint8(creator->GetRace());
    data << uint8(creator->GetGender());
#ifdef LICH_KING
    data << uint8(creator->GetClass());                       // added in 3.x
#endif

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

#ifdef LICH_KING
void WorldSession::HandleReadyForAccountDataTimes(WorldPacket& recvData)
{
    SendAccountDataTimes(GLOBAL_CACHE_MASK);
}
#endif