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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Log.h"
#include "Opcodes.h"
#include "Spell.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "CreatureAI.h"
#include "Util.h"
#include "Pet.h"
#include "Language.h"

void WorldSession::HandlePetAction( WorldPacket & recvData )
{
    CHECK_PACKET_SIZE(recvData, 8+2+2+8);

    uint64 guid1;
    uint16 spellid;
    uint16 flag;
    uint64 guid2;
    recvData >> guid1;                                     //pet guid
    recvData >> spellid;
    recvData >> flag;                                      //delete = 0x0700 CastSpell = C100
    recvData >> guid2;                                     //tag guid

    // used also for charmed creature
    Unit* pet= ObjectAccessor::GetUnit(*_player, guid1);
    TC_LOG_DEBUG("network","HandlePetAction.Pet %u flag is %u, spellid is %u, target %u.\n", uint32(GUID_LOPART(guid1)), flag, spellid, uint32(GUID_LOPART(guid2)) );
    if(!pet)
    {
        TC_LOG_ERROR( "network","Pet %u not exist.\n", uint32(GUID_LOPART(guid1)) );
        return;
    }

    if(pet != GetPlayer()->GetPet() && pet != GetPlayer()->GetCharm())
    {
        TC_LOG_ERROR("network","HandlePetAction.Pet %u isn't pet of player %s.\n", uint32(GUID_LOPART(guid1)), GetPlayer()->GetName().c_str() );
        return;
    }

    if(!pet->IsAlive())
        return;

    if(pet->GetTypeId() == TYPEID_PLAYER && !(flag == ACT_COMMAND && spellid == COMMAND_ATTACK))
        return;

    CharmInfo *charmInfo = pet->GetCharmInfo();
    if(!charmInfo)
    {
        TC_LOG_ERROR("network","WorldSession::HandlePetAction: object " UI64FMTD " is considered pet-like but doesn't have a charminfo!", pet->GetGUID());
        return;
    }

    switch(flag)
    {
        case ACT_COMMAND:                                   //0x0700
            // Possessed or shared vision pets are only able to attack
            if ((pet->IsPossessed() || pet->HasAuraType(SPELL_AURA_BIND_SIGHT)) && spellid != COMMAND_ATTACK)
                return;

            switch(spellid)
            {
                case COMMAND_STAY:                          //flat=1792  //STAY
                    pet->AttackStop();
                    pet->InterruptNonMeleeSpells(false);
                    pet->GetMotionMaster()->MoveIdle();
                    charmInfo->SetCommandState( COMMAND_STAY );
                    break;
                case COMMAND_FOLLOW:                        //spellid=1792  //FOLLOW
                    pet->AttackStop();
                    pet->InterruptNonMeleeSpells(false);
                    pet->GetMotionMaster()->MoveFollow(_player,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE);
                    charmInfo->SetCommandState( COMMAND_FOLLOW );
                    break;
                case COMMAND_ATTACK:                        //spellid=1792  //ATTACK
                {
                    // Can't attack if owner is pacified
                    if (_player->HasAuraType(SPELL_AURA_MOD_PACIFY))
                    {
                        //pet->SendPetCastFail(spellid, SPELL_FAILED_PACIFIED);
                        //TODO: Send proper error message to client
                        return;
                    }

                    // only place where pet can be player
                    Unit *TargetUnit = ObjectAccessor::GetUnit(*_player, guid2);
                    if(!TargetUnit)
                        return;

                    if(pet->CanAttack(TargetUnit) != CAN_ATTACK_RESULT_OK)
                        return;

                    // Not let attack through obstructions
                    if(sWorld->getConfig(CONFIG_PET_LOS))
                    {

                      if(pet->GetOwner() && !pet->GetOwner()->IsWithinLOSInMap(TargetUnit))
                        return;

                    }

                    pet->ClearUnitState(UNIT_STATE_FOLLOW);

                    if(pet->GetTypeId() != TYPEID_PLAYER && (pet->ToCreature())->IsAIEnabled)
                    {
                        (pet->ToCreature())->AI()->AttackStart(TargetUnit);

                        //10% chance to play special pet attack talk, else growl
                        if((pet->ToCreature())->IsPet() && ((Pet*)pet)->getPetType() == SUMMON_PET && pet != TargetUnit && urand(0, 100) < 10)
                            pet->SendPetTalk((uint32)PET_TALK_ATTACK);
                        else
                        {
                            // 90% chance for pet and 100% chance for charmed creature
                            pet->SendPetAIReaction();
                        }
                    }
                    else                                    // charmed player
                    {
                        if(pet->GetVictim() && pet->GetVictim() != TargetUnit)
                            pet->AttackStop();

                        pet->Attack(TargetUnit,true);
                        pet->SendPetAIReaction();
                    }

                    break;
                }
                case COMMAND_ABANDON:                       // abandon (hunter pet) or dismiss (summoned pet)
                    if((pet->ToCreature())->IsPet())
                    {
                        Pet* p = (Pet*)pet;
                        if(p->getPetType() == HUNTER_PET)
                            _player->RemovePet(p,PET_SAVE_AS_DELETED);
                        else
                            //dismissing a summoned pet is like killing them (this prevents returning a soulshard...)
                            p->SetDeathState(CORPSE);
                    }
                    else                                    // charmed or possessed
                        _player->Uncharm();
                    break;
                default:
                    TC_LOG_ERROR("network","WORLD: unknown PET flag Action %i and spellid %i.\n", flag, spellid);
            }
            break;
        case ACT_REACTION:                                  // 0x600
            switch(spellid)
            {
                case REACT_PASSIVE:                         //passive
                case REACT_DEFENSIVE:                       //recovery
                case REACT_AGGRESSIVE:                      //activete
                    if(pet->GetTypeId() == TYPEID_UNIT)
                        (pet->ToCreature())->SetReactState( ReactStates(spellid) );
                    break;
            }
            break;
        case ACT_DISABLED:                                  //0x8100    spell (disabled), ignore
        case ACT_CAST:                                      //0x0100
        case ACT_ENABLED:                                   //0xc100    spell
        {
            Unit* unit_target;
            if(guid2)
                unit_target = ObjectAccessor::GetUnit(*_player,guid2);
            else
                unit_target = NULL;

            if ((pet->ToCreature())->GetGlobalCooldown() > 0)
                return;

            // do not cast unknown spells
            SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellid );
            if(!spellInfo)
            {
                TC_LOG_ERROR("network","WORLD: unknown PET spell id %i\n", spellid);
                return;
            }

            for(uint32 i = 0; i < MAX_SPELL_EFFECTS;i++)
            {
                if(spellInfo->Effects[i].TargetA.GetTarget() == TARGET_UNIT_SRC_AREA_ENEMY || spellInfo->Effects[i].TargetA.GetTarget() == TARGET_UNIT_DEST_AREA_ENEMY || spellInfo->Effects[i].TargetA.GetTarget() == TARGET_DEST_DYNOBJ_ENEMY)
                    return;
            }

            // do not cast not learned spells
            if(!pet->HasSpell(spellid) || spellInfo->IsPassive())
                return;

            pet->ClearUnitState(UNIT_STATE_FOLLOW);

            Spell *spell = new Spell(pet, spellInfo, false);
            int16 result = spell->PetCanCast(unit_target);

                                                            //auto turn to target unless possessed
            if(unit_target && result == SPELL_FAILED_UNIT_NOT_INFRONT && !pet->IsPossessed())
            {
                pet->SetInFront(unit_target);
                if( unit_target->GetTypeId() == TYPEID_PLAYER )
                    pet->SendUpdateToPlayer( unit_target->ToPlayer() );
                if(Unit* powner = pet->GetCharmerOrOwner())
                    if(powner->GetTypeId() == TYPEID_PLAYER)
                        pet->SendUpdateToPlayer(powner->ToPlayer());
                result = -1;
            }

            if(result == SPELL_CAST_OK)
            {
                (pet->ToCreature())->AddCreatureSpellCooldown(spellid);
                if ((pet->ToCreature())->IsPet())
                    ((Pet*)pet)->CheckLearning(spellid);

                unit_target = spell->m_targets.GetUnitTarget();

                //10% chance to play special pet attack talk, else growl
                //actually this only seems to happen on special spells, fire shield for imp, torment for voidwalker, but it's stupid to check every spell
                if((pet->ToCreature())->IsPet() && (((Pet*)pet)->getPetType() == SUMMON_PET) && (pet != unit_target) && (urand(0, 100) < 10))
                    pet->SendPetTalk((uint32)PET_TALK_SPECIAL_SPELL);
                else
                {
                    pet->SendPetAIReaction();
                }

                if( unit_target && !GetPlayer()->IsFriendlyTo(unit_target) && !pet->IsPossessed())
                {
                    pet->ClearUnitState(UNIT_STATE_FOLLOW);
                    if(pet->GetVictim())
                        pet->AttackStop();
                    pet->GetMotionMaster()->Clear();
                    if ((pet->ToCreature())->IsAIEnabled) {
                        (pet->ToCreature())->AI()->AttackStart(unit_target);
                    }
                }

                spell->prepare(&(spell->m_targets));
            }
            else
            {
                if(pet->IsPossessed())
                {
                    WorldPacket data(SMSG_CAST_FAILED, (4+1+1));
                    data << uint32(spellid) << uint8(2) << uint8(result);
                    switch (result)
                    {
                        case SPELL_FAILED_REQUIRES_SPELL_FOCUS:
                            data << uint32(spellInfo->RequiresSpellFocus);
                            break;
                        case SPELL_FAILED_REQUIRES_AREA:
                            data << uint32(spellInfo->AreaId);
                            break;
                    }
                    SendPacket(&data);
                }
                else
                    pet->SendPetCastFail(spellid, result);

                if(!(pet->ToCreature())->HasSpellCooldown(spellid))
                    GetPlayer()->SendClearCooldown(spellid, pet);

                spell->finish(false);
                delete spell;
            }
            break;
        }
        default:
            TC_LOG_ERROR("network","WORLD: unknown PET flag Action %i and spellid %i.\n", flag, spellid);
    }
}

void WorldSession::HandlePetStopAttack(WorldPacket &recvData)
{
    uint64 guid;
    recvData >> guid;

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_PET_STOP_ATTACK for GUID " UI64FMTD "", guid);

    Unit* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, guid);

    if (!pet)
    {
        TC_LOG_ERROR("network", "HandlePetStopAttack: Pet %u does not exist", uint32(GUID_LOPART(guid)));
        return;
    }

    if (pet != GetPlayer()->GetPet() && pet != GetPlayer()->GetCharm())
    {
        TC_LOG_ERROR("network", "HandlePetStopAttack: Pet GUID %u isn't a pet or charmed creature of player %s",
            uint32(GUID_LOPART(guid)), GetPlayer()->GetName().c_str());
        return;
    }

    if (!pet->IsAlive())
        return;

    pet->AttackStop();

}
void WorldSession::HandlePetNameQuery( WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData,4+8);

    TC_LOG_DEBUG("network", "HandlePetNameQuery. CMSG_PET_NAME_QUERY\n" );

    uint32 petnumber;
    uint64 petguid;

    recvData >> petnumber;
    recvData >> petguid;

    SendPetNameQuery(petguid,petnumber);
}

void WorldSession::SendPetNameQuery( uint64 petguid, uint32 petnumber)
{
    Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, petguid);
    if(!pet || !pet->GetCharmInfo() || pet->GetCharmInfo()->GetPetNumber() != petnumber)
        return;

    std::string name = pet->GetName();

    WorldPacket data(SMSG_PET_NAME_QUERY_RESPONSE, (4+4+name.size()+1));
    data << uint32(petnumber);
    data << name.c_str();
    data << uint32(pet->GetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP));

    if( pet->IsPet() && ((Pet*)pet)->GetDeclinedNames() )
    {
        data << uint8(1);
        for(int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            data << ((Pet*)pet)->GetDeclinedNames()->name[i];
    }
    else
        data << uint8(0);

    _player->SendDirectMessage(&data);
}

bool WorldSession::CheckStableMaster(uint64 guid)
{
    // spell case or GM
    if (guid == GetPlayer()->GetGUID())
    {
#ifdef LICH_KING
        if (!GetPlayer()->IsGameMaster() && !GetPlayer()->HasAuraType(SPELL_AURA_OPEN_STABLE))
#else
        if (!GetPlayer()->IsGameMaster())
#endif
        {
            TC_LOG_DEBUG("network", "Player (GUID:%u) attempt open stable in cheating way.", GUID_LOPART(guid));
            return false;
        }
    }
    // stable master case
    else
    {
        if (!GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_STABLEMASTER))
        {
            TC_LOG_DEBUG("network", "Stablemaster (GUID:%u) not found or you can't interact with him.", GUID_LOPART(guid));
            return false;
        }
    }
    return true;
}

void WorldSession::HandlePetSetAction( WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData, 8+4+2+2);

    TC_LOG_DEBUG("network", "HandlePetSetAction. CMSG_PET_SET_ACTION\n" );

    uint64 petguid;
    uint32 position;
    uint16 spell_id;
    uint16 act_state;
    uint8  count;

    recvData >> petguid;

    // FIXME: charmed case
    //Pet* pet = sObjectAccessor->GetPet(petguid);
    if(ObjectAccessor::FindPlayer(petguid))
        return;

    Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, petguid);

    if(!pet || (pet != _player->GetPet() && pet != _player->GetCharm()))
    {
        TC_LOG_ERROR("network", "HandlePetSetAction: Unknown pet or pet owner.\n" );
        return;
    }

    CharmInfo *charmInfo = pet->GetCharmInfo();
    if(!charmInfo)
    {
        TC_LOG_ERROR("network","WorldSession::HandlePetSetAction: object " UI64FMTD " is considered pet-like but doesn't have a charminfo!", pet->GetGUID());
        return;
    }

    count = (recvData.size() == 24) ? 2 : 1;
    for(uint8 i = 0; i < count; i++)
    {
        recvData >> position;
        recvData >> spell_id;
        recvData >> act_state;

        TC_LOG_DEBUG("network", "Player %s has changed pet spell action. Position: %u, Spell: %u, State: 0x%X\n", _player->GetName().c_str(), position, spell_id, act_state);

                                                            //if it's act for spell (en/disable/cast) and there is a spell given (0 = remove spell) which pet doesn't know, don't add
        if(!((act_state == ACT_ENABLED || act_state == ACT_DISABLED || act_state == ACT_CAST) && spell_id && !pet->HasSpell(spell_id)))
        {
            //sign for autocast
            if(act_state == ACT_ENABLED && spell_id)
            {
                if(pet->IsCharmed())
                    charmInfo->ToggleCreatureAutocast(spell_id, true);
                else
                    ((Pet*)pet)->ToggleAutocast(spell_id, true);
            }
            //sign for no/turn off autocast
            else if(act_state == ACT_DISABLED && spell_id)
            {
                if(pet->IsCharmed())
                    charmInfo->ToggleCreatureAutocast(spell_id, false);
                else
                    ((Pet*)pet)->ToggleAutocast(spell_id, false);
            }

            charmInfo->GetActionBarEntry(position)->Type = act_state;
            charmInfo->GetActionBarEntry(position)->SpellOrAction = spell_id;
        }
    }
}

void WorldSession::HandlePetRename( WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData, 8+1);

    TC_LOG_DEBUG("network", "HandlePetRename. CMSG_PET_RENAME\n" );

    uint64 petguid;
    uint8 isdeclined;

    std::string name;
    DeclinedName declinedname;

    recvData >> petguid;
    recvData >> name;
    CHECK_PACKET_SIZE(recvData, recvData.rpos() + 1);
    recvData >> isdeclined;

    if(!GetPlayer())
    {
        TC_LOG_ERROR("network","HandlePetRename : GetPlayer() returned NULL");
        return;
    }
    Pet* pet = ObjectAccessor::GetPet(*GetPlayer(),petguid);
                                                            // check it!
    if( !pet || !pet->IsPet() || ((Pet*)pet)->getPetType()!= HUNTER_PET ||
        pet->GetByteValue(UNIT_FIELD_BYTES_2, 2) != UNIT_RENAME_ALLOWED ||
        pet->GetOwnerGUID() != _player->GetGUID() || !pet->GetCharmInfo() )
        return;

    PetNameInvalidReason res = ObjectMgr::CheckPetName(name);
    if (res != PET_NAME_SUCCESS)
    {
        SendPetNameInvalid(res, name, NULL);
        return;
    }

    if(sObjectMgr->IsReservedName(name))
    {
        SendPetNameInvalid(PET_NAME_RESERVED, name, NULL);
        return;
    }

    pet->SetName(name);

    Unit *owner = pet->GetOwner();
    if(owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
        (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_NAME);

    pet->SetByteValue(UNIT_FIELD_BYTES_2, 2, UNIT_RENAME_NOT_ALLOWED);

    if(isdeclined)
    {
        for(int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
        {
            CHECK_PACKET_SIZE(recvData, recvData.rpos() + 1);
            recvData >> declinedname.name[i];
        }

        std::wstring wname;
        Utf8toWStr(name, wname);
        if(!ObjectMgr::CheckDeclinedNames(GetMainPartOfName(wname,0),declinedname))
        {
            SendPetNameInvalid(PET_NAME_DECLENSION_DOESNT_MATCH_BASE_NAME, name, &declinedname);
            return;
        }
    }

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    if(isdeclined)
    {
        for(int i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            CharacterDatabase.EscapeString(declinedname.name[i]);
        trans->PAppend("DELETE FROM character_pet_declinedname WHERE owner = '%u' AND id = '%u'", _player->GetGUIDLow(), pet->GetCharmInfo()->GetPetNumber());
        trans->PAppend("INSERT INTO character_pet_declinedname (id, owner, genitive, dative, accusative, instrumental, prepositional) VALUES ('%u','%u','%s','%s','%s','%s','%s')",
            pet->GetCharmInfo()->GetPetNumber(), _player->GetGUIDLow(), declinedname.name[0].c_str(), declinedname.name[1].c_str(), declinedname.name[2].c_str(), declinedname.name[3].c_str(), declinedname.name[4].c_str());
    }

    CharacterDatabase.EscapeString(name);
    trans->PAppend("UPDATE character_pet SET name = '%s', renamed = '1' WHERE owner = '%u' AND id = '%u'", name.c_str(), _player->GetGUIDLow(), pet->GetCharmInfo()->GetPetNumber());
    CharacterDatabase.CommitTransaction(trans);

    pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(NULL));
}

void WorldSession::HandlePetAbandon( WorldPacket & recvData )
{
    
    
    CHECK_PACKET_SIZE(recvData, 8);

    uint64 guid;
    recvData >> guid;                                      //pet guid
    TC_LOG_DEBUG("network", "HandlePetAbandon. CMSG_PET_ABANDON pet guid is %u", GUID_LOPART(guid) );
    TC_LOG_INFO("entities.player", "HandlePetAbandon. CMSG_PET_ABANDON pet guid is %u", GUID_LOPART(guid));

    // pet/charmed
    Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, guid);
    if(pet)
    {
        if(pet->IsPet())
        {
            if(pet->GetGUID() == _player->GetPetGUID())
            {
                uint32 feelty = pet->GetPower(POWER_HAPPINESS);
                pet->SetPower(POWER_HAPPINESS ,(feelty-50000) > 0 ?(feelty-50000) : 0);
            }

            _player->RemovePet((Pet*)pet,PET_SAVE_AS_DELETED);
        }
        else if(pet->GetGUID() == _player->GetCharmGUID())
            _player->Uncharm();
    }
}

void WorldSession::HandlePetUnlearnOpcode(WorldPacket& recvPacket)
{
    
    
    CHECK_PACKET_SIZE(recvPacket,8);

    TC_LOG_DEBUG("network","CMSG_PET_UNLEARN");
    uint64 guid;
    recvPacket >> guid;

    Pet* pet = _player->GetPet();

    if(!pet || pet->getPetType() != HUNTER_PET || pet->m_spells.size() <= 1)
        return;

    if(guid != pet->GetGUID())
    {
        TC_LOG_ERROR( "network","HandlePetUnlearnOpcode.Pet %u isn't pet of player %s .\n", uint32(GUID_LOPART(guid)),GetPlayer()->GetName().c_str() );
        return;
    }

    CharmInfo *charmInfo = pet->GetCharmInfo();
    if(!charmInfo)
    {
        TC_LOG_ERROR("network","WorldSession::HandlePetUnlearnOpcode: object " UI64FMTD " is considered pet-like but doesn't have a charminfo!", pet->GetGUID());
        return;
    }

    uint32 cost = pet->ResetTalentsCost();

    if (GetPlayer()->GetMoney() < cost)
    {
        GetPlayer()->SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
        return;
    }

    for(PetSpellMap::iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end();)
    {
        uint32 spell_id = itr->first;                       // Pet::removeSpell can invalidate iterator at erase NEW spell
        ++itr;
        pet->RemoveSpell(spell_id);
    }

    pet->SetTP(pet->GetLevel() * (pet->GetLoyaltyLevel() - 1));

    for(uint8 i = 0; i < 10; i++)
    {
        if((charmInfo->GetActionBarEntry(i)->SpellOrAction && charmInfo->GetActionBarEntry(i)->Type == ACT_ENABLED) || charmInfo->GetActionBarEntry(i)->Type == ACT_DISABLED)
            charmInfo->GetActionBarEntry(i)->SpellOrAction = 0;
    }

    // relearn pet passives
    pet->LearnPetPassives();

    pet->m_resetTalentsTime = time(NULL);
    pet->m_resetTalentsCost = cost;
    GetPlayer()->ModifyMoney(-(int32)cost);

    GetPlayer()->PetSpellInitialize();
}

void WorldSession::HandlePetSpellAutocastOpcode( WorldPacket& recvPacket )
{
    
    
    CHECK_PACKET_SIZE(recvPacket,8+2+2+1);

    TC_LOG_DEBUG("network","CMSG_PET_SPELL_AUTOCAST");
    uint64 guid;
    uint16 spellid;
    uint16 spellid2;                                        //maybe second spell, automatically toggled off when first toggled on?
    uint8  state;                                           //1 for on, 0 for off
    recvPacket >> guid >> spellid >> spellid2 >> state;

    if(!_player->GetPet() && !_player->GetCharm())
        return;

    if(ObjectAccessor::FindPlayer(guid))
        return;

    Creature* pet=ObjectAccessor::GetCreatureOrPetOrVehicle(*_player,guid);

    if(!pet || (pet != _player->GetPet() && pet != _player->GetCharm()))
    {
        TC_LOG_ERROR("network", "HandlePetSpellAutocastOpcode.Pet %u isn't pet of player %s .\n", uint32(GUID_LOPART(guid)),GetPlayer()->GetName().c_str() );
        return;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if(!spellInfo)
    {
        TC_LOG_ERROR("network","WORLD: unknown PET spell id %i\n", spellid);
        return;
    }

    // do not add not learned spells/ passive spells
    if(!pet->HasSpell(spellid) || spellInfo->IsPassive())
        return;

    CharmInfo *charmInfo = pet->GetCharmInfo();
    if(!charmInfo)
    {
        TC_LOG_ERROR("network","WorldSession::HandlePetSpellAutocastOpcod: object " UI64FMTD " is considered pet-like but doesn't have a charminfo!", pet->GetGUID());
        return;
    }

    if(pet->IsCharmed())
                                                            //state can be used as boolean
        pet->GetCharmInfo()->ToggleCreatureAutocast(spellid, state);
    else
        ((Pet*)pet)->ToggleAutocast(spellid, state);

    for(uint8 i = 0; i < 10; ++i)
    {
        if((charmInfo->GetActionBarEntry(i)->Type == ACT_ENABLED || charmInfo->GetActionBarEntry(i)->Type == ACT_DISABLED) && spellid == charmInfo->GetActionBarEntry(i)->SpellOrAction)
            charmInfo->GetActionBarEntry(i)->Type = state ? ACT_ENABLED : ACT_DISABLED;
    }
}

void WorldSession::HandlePetCastSpellOpcode( WorldPacket& recvPacket )
{
    
    
    TC_LOG_DEBUG("network","WORLD: CMSG_PET_CAST_SPELL");

    CHECK_PACKET_SIZE(recvPacket,8+4);
    uint64 guid;
    uint32 spellid;

    recvPacket >> guid >> spellid;

    // This opcode is also sent from charmed and possessed units (players and creatures)
    if(!_player->GetPet() && !_player->GetCharm())
        return;

    Unit* caster = ObjectAccessor::GetUnit(*_player, guid);

    if(!caster || (caster != _player->GetPet() && caster != _player->GetCharm()))
    {
        TC_LOG_ERROR("network", "HandlePetCastSpellOpcode: Pet %u isn't pet of player %s .\n", uint32(GUID_LOPART(guid)),GetPlayer()->GetName().c_str() );
        return;
    }

    SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if(!spellInfo)
    {
        TC_LOG_ERROR("network","WORLD: unknown PET spell id %i\n", spellid);
        return;
    }

    // do not cast not learned spells
    if(!caster->HasSpell(spellid) || spellInfo->IsPassive())
        return;

    if (spellInfo->StartRecoveryCategory > 0) //Check if spell is affected by GCD
        if (caster->GetTypeId() == TYPEID_UNIT && (caster->ToCreature())->GetGlobalCooldown() > 0)
        {
            caster->SendPetCastFail(spellid, SPELL_FAILED_NOT_READY);
            return;
        }

    SpellCastTargets targets;
    if(!targets.read(&recvPacket,caster))
        return;

    caster->ClearUnitState(UNIT_STATE_FOLLOW);

    Spell *spell = new Spell(caster, spellInfo, spellid == 33395);
    spell->m_targets = targets;

    SpellCastResult result = spell->PetCanCast(NULL);
    if (spellid == 33395) { //Water elemental Freeze
        result = spell->CheckRange(true);
    }
    
    if(result == SPELL_CAST_OK)
    {
        if(caster->GetTypeId() == TYPEID_UNIT)
        {
            Creature* pet = caster->ToCreature();
            pet->AddCreatureSpellCooldown(spellid);
            if(pet->IsPet())
            {
                Pet* p = (Pet*)pet;
                p->CheckLearning(spellid);
                // 10% chance to play special pet attack talk, else growl
                // actually this only seems to happen on special spells, fire shield for imp, torment for voidwalker, but it's stupid to check every spell
                if(p->getPetType() == SUMMON_PET && (urand(0, 100) < 10))
                    pet->SendPetTalk((uint32)PET_TALK_SPECIAL_SPELL);
                else
                    pet->SendPetAIReaction();
            }
        }

        spell->prepare(&(spell->m_targets));
    }
    else
    {
        caster->SendPetCastFail(spellid, result);
        if(caster->GetTypeId() == TYPEID_PLAYER)
        {
            if(!(caster->ToPlayer())->HasSpellCooldown(spellid))
                GetPlayer()->SendClearCooldown(spellid, caster);
        }
        else
        {
            if(!(caster->ToCreature())->HasSpellCooldown(spellid))
                GetPlayer()->SendClearCooldown(spellid, caster);
        }

        spell->finish(false);
        delete spell;
    }
}

void WorldSession::SendPetNameInvalid(uint32 error, const std::string& name, DeclinedName *declinedName)
{
    WorldPacket data(SMSG_PET_NAME_INVALID, 4 + name.size() + 1 + 1);
    data << uint32(error);
    data << name;
    if(declinedName)
    {
        data << uint8(1);
        for(uint32 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            data << declinedName->name[i];
    }
    else
        data << uint8(0);
    SendPacket(&data);
}

