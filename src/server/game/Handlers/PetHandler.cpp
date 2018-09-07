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
#include "PetAI.h"
#include "SpellHistory.h"

void WorldSession::HandlePetActionHelper(Unit* pet, ObjectGuid guid1, uint32 spellid, uint16 flag, ObjectGuid guid2)
{
    CharmInfo* charmInfo = pet->GetCharmInfo();
    if (!charmInfo)
    {
        TC_LOG_DEBUG("entities.pet", "WorldSession::HandlePetAction(petGuid: %s, tagGuid: %s, spellId: %u, flag: %u): object (GUID: %u Entry: %u TypeId: %u) is considered pet-like but doesn't have a charminfo!",
            ObjectGuid(guid1).ToString().c_str(), ObjectGuid(guid2).ToString().c_str(), spellid, flag, ObjectGuid(pet->GetGUID()).GetCounter(), pet->GetEntry(), pet->GetTypeId());
        return;
    }

    switch(flag)
    {
        case ACT_COMMAND:                                   //0x07
            // Possessed or shared vision pets are only able to attack
            if ((pet->IsPossessed() || pet->HasAuraType(SPELL_AURA_BIND_SIGHT)) && spellid != COMMAND_ATTACK && spellid != COMMAND_ABANDON)
                return;

            switch(spellid)
            {
                case COMMAND_STAY:                          //flat=1792  //STAY
                    pet->StopMoving();
                    pet->GetMotionMaster()->Clear();
                    pet->GetMotionMaster()->MoveIdle();
                    charmInfo->SetCommandState( COMMAND_STAY );

                    charmInfo->SetIsCommandAttack(false);
                    charmInfo->SetIsAtStay(true);
                    charmInfo->SetIsCommandFollow(false);
                    charmInfo->SetIsFollowing(false);
                    charmInfo->SetIsReturning(false);
                    charmInfo->SaveStayPosition(); //this saves at current destination
                    break;
                case COMMAND_FOLLOW:                        //spellid=1792  //FOLLOW
                    pet->AttackStop();
                    pet->InterruptNonMeleeSpells(false);
                    pet->GetMotionMaster()->MoveFollow(_player, PET_FOLLOW_DIST, pet->GetFollowAngle());
                    charmInfo->SetCommandState( COMMAND_FOLLOW );

                    charmInfo->SetIsCommandAttack(false);
                    charmInfo->SetIsAtStay(false);
                    charmInfo->SetIsReturning(true);
                    charmInfo->SetIsCommandFollow(true);
                    charmInfo->SetIsFollowing(false);
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
                    Unit* TargetUnit = ObjectAccessor::GetUnit(*_player, guid2);
                    if(!TargetUnit)
                        return;

                    if (Unit* owner = pet->GetOwner())
                        if (!owner->IsValidAttackTarget(TargetUnit))
                            return;

                    pet->ClearUnitState(UNIT_STATE_FOLLOW);

                    //sunstrider custom
                    if (Pet* _pet = pet->ToPet())
                        if (_pet->getPetType() == HUNTER_PET)
                            if (TargetUnit->HasBreakableByDamageCrowdControlAura(pet))
                                ((PetAI*)_pet->AI())->ForceAttackBreakable(TargetUnit);

                    // This is true if pet has no target or has target but targets differs.
                    if (pet->GetVictim() != TargetUnit || (pet->GetVictim() == TargetUnit && !pet->GetCharmInfo()->IsCommandAttack()))
                    {
                        if (pet->GetVictim())
                            pet->AttackStop();

                        if (pet->GetTypeId() != TYPEID_PLAYER && (pet->ToCreature())->IsAIEnabled())
                        {
                            charmInfo->SetIsCommandAttack(true);
                            charmInfo->SetIsAtStay(false);
                            charmInfo->SetIsFollowing(false);
                            charmInfo->SetIsCommandFollow(false);
                            charmInfo->SetIsReturning(false);

                            CreatureAI* AI = pet->ToCreature()->AI();
                            if (PetAI* petAI = dynamic_cast<PetAI*>(AI))
                                petAI->_AttackStart(TargetUnit); // force target switch
                            else
                                AI->AttackStart(TargetUnit);

                            //10% chance to play special pet attack talk, else growl
                            if ((pet->ToCreature())->IsPet() && ((Pet*)pet)->getPetType() == SUMMON_PET && pet != TargetUnit && urand(0, 99) < 10)
                                pet->SendPetTalk((uint32)PET_TALK_ATTACK);
                            else
                            {
                                // 90% chance for pet and 100% chance for charmed creature
                                pet->SendPetAIReaction();
                            }
                        }
                        else                                    // charmed player
                        {
                            charmInfo->SetIsCommandAttack(true);
                            charmInfo->SetIsAtStay(false);
                            charmInfo->SetIsFollowing(false);
                            charmInfo->SetIsCommandFollow(false);
                            charmInfo->SetIsReturning(false);

                            pet->Attack(TargetUnit, true);
                            pet->SendPetAIReaction();
                        }
                    }
                    break;
                }
                case COMMAND_ABANDON:                       // abandon (hunter pet) or dismiss (summoned pet)
                    if (pet->GetCharmerGUID() == GetPlayer()->GetGUID())
                        _player->StopCastingCharm();
                    else if (pet->GetOwnerGUID() == GetPlayer()->GetGUID())
                    {
                        ASSERT(pet->GetTypeId() == TYPEID_UNIT);
                        if (pet->IsPet())
                        {
                            if (((Pet*)pet)->getPetType() == HUNTER_PET)
                                _player->RemovePet((Pet*)pet, PET_SAVE_AS_DELETED);
                            else
                                //dismissing a summoned pet is like killing them (this prevents returning a soulshard...)
                                pet->SetDeathState(CORPSE);
                        }
                        else if (pet->HasUnitTypeMask(UNIT_MASK_MINION))
                        {
                            ((Minion*)pet)->UnSummon();
                        }
                    }
                    else                                    // charmed or possessed
                        _player->StopCastingCharm();
                    break;
                default:
                    TC_LOG_ERROR("network","WORLD: unknown PET flag Action %i and spellid %i.\n", flag, spellid);
            }
            break;
        case ACT_REACTION:                                  // 0x6
            switch(spellid)
            {
                case REACT_PASSIVE:                         //passive
                    /* sun: TC disabled, we don't want pets to stop attacking on setting passive. Passive is not a non-combat mode but a fine pet control mode.
                    pet->AttackStop();
                    */
                    [[fallthrough]];
                case REACT_DEFENSIVE:                       //recovery
                case REACT_AGGRESSIVE:                      //activete
                    if(pet->GetTypeId() == TYPEID_UNIT)
                        (pet->ToCreature())->SetReactState( ReactStates(spellid) );
                    break;
            }
            break;
        case ACT_DISABLED:                                  //0x81    spell (disabled), ignore
        case ACT_PASSIVE:                                   //0x01
        case ACT_ENABLED:                                   //0xc1    spell
        {
            Unit* unit_target = nullptr;
            if(guid2)
                unit_target = ObjectAccessor::GetUnit(*_player,guid2);

            // do not cast unknown spells
            SpellInfo const *spellInfo = sSpellMgr->GetSpellInfo(spellid );
            if(!spellInfo)
            {
                TC_LOG_ERROR("spells.pet","WORLD: unknown PET spell id %i\n", spellid);
                return;
            }

            for(const auto & Effect : spellInfo->Effects)
            {
                if(Effect.TargetA.GetTarget() == TARGET_UNIT_SRC_AREA_ENEMY || Effect.TargetA.GetTarget() == TARGET_UNIT_DEST_AREA_ENEMY || Effect.TargetA.GetTarget() == TARGET_DEST_DYNOBJ_ENEMY)
                    return;
            }

            // do not cast not learned spells
            if(!pet->HasSpell(spellid) || spellInfo->IsPassive())
                return;

            //  Clear the flags as if owner clicked 'attack'. AI will reset them
            //  after AttackStart, even if spell failed
            if (pet->GetCharmInfo())
            {
                pet->GetCharmInfo()->SetIsAtStay(false);
                pet->GetCharmInfo()->SetIsCommandAttack(true);
                pet->GetCharmInfo()->SetIsReturning(false);
                pet->GetCharmInfo()->SetIsFollowing(false);
            }

            auto spell = new Spell(pet, spellInfo, TRIGGERED_NONE);
            int16 result = spell->CheckPetCast(unit_target);

            //auto turn to target unless possessed
            if(unit_target && result == SPELL_FAILED_UNIT_NOT_INFRONT && !pet->IsPossessed())
            {
                if (unit_target)
                {
                    if (!pet->IsFocusing())
                        pet->SetInFront(unit_target);
                    if (Player* player = unit_target->ToPlayer())
                        pet->SendUpdateToPlayer(player);
                }
                else if (Unit* unit_target2 = spell->m_targets.GetUnitTarget())
                {
                    if (!pet->IsFocusing())
                        pet->SetInFront(unit_target2);
                    if (Player* player = unit_target2->ToPlayer())
                        pet->SendUpdateToPlayer(player);
                }

                if (Unit* powner = pet->GetCharmerOrOwner())
                    if (Player* player = powner->ToPlayer())
                        pet->SendUpdateToPlayer(player);

                result = SPELL_CAST_OK;
            }

            if(result == SPELL_CAST_OK)
            {
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
                    if (CreatureAI* AI = pet->ToCreature()->AI())
                    {
                        if (PetAI* petAI = dynamic_cast<PetAI*>(AI))
                            petAI->_AttackStart(unit_target); // force victim switch
                        else
                            AI->AttackStart(unit_target);
                    }
                }

                spell->prepare((spell->m_targets));
            }
            else
            {
                if(pet->IsPossessed())
                {
                    WorldPacket data(SMSG_CAST_FAILED, (4+1+1));
                    //structure seems wrong here
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

                if (!pet->GetSpellHistory()->HasCooldown(spellid))
                    pet->GetSpellHistory()->ResetCooldown(spellid, true);

                spell->finish(false);
                delete spell;


                // reset specific flags in case of spell fail. AI will reset other flags
                if (pet->GetCharmInfo())
                    pet->GetCharmInfo()->SetIsCommandAttack(false);
            }
            break;
        }
        default:
            TC_LOG_ERROR("entities.pet","WORLD: unknown PET flag Action %i and spellid %i.\n", flag, spellid);
    }
}

void WorldSession::HandlePetAction( WorldPacket & recvData )
{
    ObjectGuid guid1;
    uint32 data;
    ObjectGuid targetGuid;

    recvData >> guid1;          //pet guid
    recvData >> data;
    recvData >> targetGuid;     //tag guid

    uint32 spellid = UNIT_ACTION_BUTTON_ACTION(data);
    uint8 flag = UNIT_ACTION_BUTTON_TYPE(data);

    // used also for charmed creature
    Unit* pet= ObjectAccessor::GetUnit(*_player, guid1);
    TC_LOG_DEBUG("network","HandlePetAction.Pet %u flag is %u, spellid is %u, target %u.\n", uint32(guid1.GetCounter()), flag, spellid, uint32(targetGuid.GetCounter()) );
    if(!pet)
    {
        TC_LOG_ERROR( "network","Pet %u not exist.\n", uint32(guid1.GetCounter()) );
        return;
    }

    if (pet != GetPlayer()->GetFirstControlled())
    {
        TC_LOG_ERROR("network","HandlePetAction. %u does not beling to %u %s.\n", uint32(guid1.GetCounter()), GetPlayer()->GetGUID().GetCounter(), GetPlayer()->GetName().c_str() );
        return;
    }

    if (!pet->IsAlive())
    {
        SpellInfo const* spell = (flag == ACT_ENABLED || flag == ACT_PASSIVE) ? sSpellMgr->GetSpellInfo(spellid) : NULL;
        if (!spell)
            return;
        if (!spell->HasAttribute(SPELL_ATTR0_CASTABLE_WHILE_DEAD))
            return;
    }

    /// @todo allow control charmed player?
    if(pet->GetTypeId() == TYPEID_PLAYER && !(flag == ACT_COMMAND && spellid == COMMAND_ATTACK))
        return;

    CharmInfo *charmInfo = pet->GetCharmInfo();
    if(!charmInfo)
    {
        TC_LOG_ERROR("network","WorldSession::HandlePetAction: object %u is considered pet-like but doesn't have a charminfo!", pet->GetGUID().GetCounter());
        return;
    }

    if (GetPlayer()->m_Controlled.size() == 1)
        HandlePetActionHelper(pet, guid1, spellid, flag, targetGuid);
    else
    {
        //If a pet is dismissed, m_Controlled will change
        std::vector<Unit*> controlled;
        for (Unit::ControlList::iterator itr = GetPlayer()->m_Controlled.begin(); itr != GetPlayer()->m_Controlled.end(); ++itr)
            if ((*itr)->GetEntry() == pet->GetEntry() && (*itr)->IsAlive())
                controlled.push_back(*itr);
        for (std::vector<Unit*>::iterator itr = controlled.begin(); itr != controlled.end(); ++itr)
            HandlePetActionHelper(*itr, guid1, spellid, flag, targetGuid);
    }

}

//LK OK
void WorldSession::HandlePetStopAttack(WorldPacket &recvData)
{
    ObjectGuid guid;
    recvData >> guid;

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_PET_STOP_ATTACK for GUID %u", guid.GetCounter());

    Unit* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, guid);

    if (!pet)
    {
        TC_LOG_ERROR("network", "HandlePetStopAttack: Pet %u does not exist", uint32(guid.GetCounter()));
        return;
    }

    if (pet != GetPlayer()->GetPet() && pet != GetPlayer()->GetCharmed())
    {
        TC_LOG_ERROR("network", "HandlePetStopAttack: Pet GUID %u isn't a pet or charmed creature of player %s",
            uint32(guid.GetCounter()), GetPlayer()->GetName().c_str());
        return;
    }

    if (!pet->IsAlive())
        return;

    pet->AttackStop();

}
void WorldSession::HandlePetNameQuery( WorldPacket & recvData )
{
    // TC_LOG_DEBUG("network", "HandlePetNameQuery. CMSG_PET_NAME_QUERY\n" );

    uint32 petnumber;
    ObjectGuid petguid;

    recvData >> petnumber;
    recvData >> petguid;

    SendPetNameQuery(petguid,petnumber);
}

void WorldSession::SendPetNameQuery(ObjectGuid petguid, uint32 petnumber)
{
    Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, petguid);
    if (!pet || !pet->GetCharmInfo() || pet->GetCharmInfo()->GetPetNumber() != petnumber)
    {
        WorldPacket data(SMSG_PET_NAME_QUERY_RESPONSE, (4 + 1 + 4 + 1));
        data << uint32(petnumber);
        data << uint8(0);
        data << uint32(0);
        data << uint8(0);
        _player->GetSession()->SendPacket(&data);
        return;
    }

    std::string name = pet->GetName();

    WorldPacket data(SMSG_PET_NAME_QUERY_RESPONSE, (4+4+name.size()+1));
    data << uint32(petnumber);
    data << name.c_str();
    data << uint32(pet->GetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP));

    if( pet->IsPet() && ((Pet*)pet)->GetDeclinedNames() )
    {
        data << uint8(1);
        for(const auto & i : ((Pet*)pet)->GetDeclinedNames()->name)
            data << i;
    }
    else
        data << uint8(0);

    _player->SendDirectMessage(&data);
}

bool WorldSession::CheckStableMaster(ObjectGuid guid)
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
            TC_LOG_DEBUG("network", "Player (GUID:%u) attempt open stable in cheating way.", guid.GetCounter());
            return false;
        }
    }
    // stable master case
    else
    {
        if (!GetPlayer()->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_STABLEMASTER))
        {
            TC_LOG_DEBUG("network", "Stablemaster (GUID:%u) not found or you can't interact with him.", guid.GetCounter());
            return false;
        }
    }
    return true;
}

void WorldSession::HandlePetSetAction( WorldPacket & recvData )
{
 //   TC_LOG_DEBUG("network", "HandlePetSetAction. CMSG_PET_SET_ACTION\n" );

    ObjectGuid petguid;
    uint8  count;

    recvData >> petguid;

    // FIXME: charmed case
    //Pet* pet = ObjectAccessor::GetPet(petguid);
    if(ObjectAccessor::FindPlayer(petguid))
        return;

    Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, petguid);

    if(!pet || pet != _player->GetFirstControlled())
    {
        TC_LOG_ERROR("network", "HandlePetSetAction: Unknown pet or pet owner.\n" );
        return;
    }

    CharmInfo *charmInfo = pet->GetCharmInfo();
    if(!charmInfo)
    {
        TC_LOG_ERROR("network","WorldSession::HandlePetSetAction: object %u is considered pet-like but doesn't have a charminfo!", pet->GetGUID().GetCounter());
        return;
    }

    count = (recvData.size() == 24) ? 2 : 1;

    uint32 position[2];
    uint32 data[2];
    bool move_command = false;

    for (uint8 i = 0; i < count; i++)
    {
        recvData >> position[i];
        recvData >> data[i];

        uint8 act_state = UNIT_ACTION_BUTTON_TYPE(data[i]);

        //ignore invalid position
        if (position[i] >= MAX_UNIT_ACTION_BAR_INDEX)
            return;

        // in the normal case, command and reaction buttons can only be moved, not removed
        // at moving count == 2, at removing count == 1
        // ignore attempt to remove command|reaction buttons (not possible at normal case)
        if (act_state == ACT_COMMAND || act_state == ACT_REACTION)
        {
            if (count == 1)
                return;

            move_command = true;
            //TC_LOG_DEBUG("network", "Player %s has changed pet spell action. Position: %u, Spell: %u, State: 0x%X\n", _player->GetName().c_str(), position, spell_id, act_state);
        }
    }

    // check swap (at command->spell swap client remove spell first in another packet, so check only command move correctness)
    if (move_command)
    {
        uint8 act_state_0 = UNIT_ACTION_BUTTON_TYPE(data[0]);
        if (act_state_0 == ACT_COMMAND || act_state_0 == ACT_REACTION)
        {
            uint32 spell_id_0 = UNIT_ACTION_BUTTON_ACTION(data[0]);
            UnitActionBarEntry const* actionEntry_1 = charmInfo->GetActionBarEntry(position[1]);
            if (!actionEntry_1 || spell_id_0 != actionEntry_1->GetAction() ||
                act_state_0 != actionEntry_1->GetType())
                return;
        }

        uint8 act_state_1 = UNIT_ACTION_BUTTON_TYPE(data[1]);
        if (act_state_1 == ACT_COMMAND || act_state_1 == ACT_REACTION)
        {
            uint32 spell_id_1 = UNIT_ACTION_BUTTON_ACTION(data[1]);
            UnitActionBarEntry const* actionEntry_0 = charmInfo->GetActionBarEntry(position[0]);
            if (!actionEntry_0 || spell_id_1 != actionEntry_0->GetAction() ||
                act_state_1 != actionEntry_0->GetType())
                return;
        }
    }

    for (uint8 i = 0; i < count; ++i)
    {
        uint32 spell_id = UNIT_ACTION_BUTTON_ACTION(data[i]);
        uint8 act_state = UNIT_ACTION_BUTTON_TYPE(data[i]);

        TC_LOG_DEBUG("entities.pet", "Player %s has changed pet spell action. Position: %u, Spell: %u, State: 0x%X",
            _player->GetName().c_str(), position[i], spell_id, uint32(act_state));

        //if it's act for spell (en/disable/cast) and there is a spell given (0 = remove spell) which pet doesn't know, don't add
        if (!((act_state == ACT_ENABLED || act_state == ACT_DISABLED || act_state == ACT_PASSIVE) && spell_id && !pet->HasSpell(spell_id)))
        {
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell_id))
            {
                //sign for autocast
                if (act_state == ACT_ENABLED)
                {
                    if (pet->GetTypeId() == TYPEID_UNIT && pet->IsPet())
                        ((Pet*)pet)->ToggleAutocast(spellInfo, true);
                    else
                        for (Unit::ControlList::iterator itr = GetPlayer()->m_Controlled.begin(); itr != GetPlayer()->m_Controlled.end(); ++itr)
                            if ((*itr)->GetEntry() == pet->GetEntry())
                                (*itr)->GetCharmInfo()->ToggleCreatureAutocast(spellInfo, true);
                }
                //sign for no/turn off autocast
                else if (act_state == ACT_DISABLED)
                {
                    if (pet->GetTypeId() == TYPEID_UNIT && pet->IsPet())
                        ((Pet*)pet)->ToggleAutocast(spellInfo, false);
                    else
                        for (Unit::ControlList::iterator itr = GetPlayer()->m_Controlled.begin(); itr != GetPlayer()->m_Controlled.end(); ++itr)
                            if ((*itr)->GetEntry() == pet->GetEntry())
                                (*itr)->GetCharmInfo()->ToggleCreatureAutocast(spellInfo, false);
                }
            }

            charmInfo->SetActionBar(position[i], spell_id, ActiveStates(act_state));
        }
    }
}

void WorldSession::HandlePetRename( WorldPacket & recvData )
{
   // TC_LOG_DEBUG("network", "HandlePetRename. CMSG_PET_RENAME\n" );

    ObjectGuid petguid;
    uint8 isdeclined;

    std::string name;
    DeclinedName declinedname;

    recvData >> petguid;
    recvData >> name;
    
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
        SendPetNameInvalid(res, name, nullptr);
        return;
    }

    if(sObjectMgr->IsReservedName(name))
    {
        SendPetNameInvalid(PET_NAME_RESERVED, name, nullptr);
        return;
    }

    pet->SetName(name);

    Unit *owner = pet->GetOwner();
    if(owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
        (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_NAME);

    pet->SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_PET_FLAGS, UNIT_RENAME_NOT_ALLOWED);

    if(isdeclined)
    {
        for(auto & i : declinedname.name)
        {
            
            recvData >> i;
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
        for(auto & i : declinedname.name)
            CharacterDatabase.EscapeString(i);
        trans->PAppend("DELETE FROM character_pet_declinedname WHERE owner = '%u' AND id = '%u'", _player->GetGUID().GetCounter(), pet->GetCharmInfo()->GetPetNumber());
        trans->PAppend("INSERT INTO character_pet_declinedname (id, owner, genitive, dative, accusative, instrumental, prepositional) VALUES ('%u','%u','%s','%s','%s','%s','%s')",
            pet->GetCharmInfo()->GetPetNumber(), _player->GetGUID().GetCounter(), declinedname.name[0].c_str(), declinedname.name[1].c_str(), declinedname.name[2].c_str(), declinedname.name[3].c_str(), declinedname.name[4].c_str());
    }

    CharacterDatabase.EscapeString(name);
    trans->PAppend("UPDATE character_pet SET name = '%s', renamed = '1' WHERE owner = '%u' AND id = '%u'", name.c_str(), _player->GetGUID().GetCounter(), pet->GetCharmInfo()->GetPetNumber());
    CharacterDatabase.CommitTransaction(trans);

    pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(nullptr));
}

void WorldSession::HandlePetAbandon( WorldPacket & recvData )
{
    ObjectGuid guid;
    recvData >> guid;                                      //pet guid
    TC_LOG_DEBUG("network", "HandlePetAbandon. CMSG_PET_ABANDON pet guid is %u", guid.GetCounter() );
    TC_LOG_INFO("entities.player", "HandlePetAbandon. CMSG_PET_ABANDON pet guid is %u", guid.GetCounter());

    // pet/charmed
    Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, guid);
    if(pet)
    {
        if(pet->IsPet())
        {
            if(pet->GetGUID() == _player->GetMinionGUID())
            {
                uint32 feelty = pet->GetPower(POWER_HAPPINESS);
                pet->SetPower(POWER_HAPPINESS ,(feelty-50000) > 0 ?(feelty-50000) : 0);
            }

            _player->RemovePet((Pet*)pet,PET_SAVE_AS_DELETED);
        }
        //this opcode is also used for some controlled creature other than pets
        else if(pet->GetGUID() == _player->GetCharmedGUID())
            _player->StopCastingCharm();
    }
}

void WorldSession::HandlePetUnlearnOpcode(WorldPacket& recvPacket)
{
    TC_LOG_DEBUG("network","CMSG_PET_UNLEARN");
    ObjectGuid guid;
    recvPacket >> guid;

    Pet* pet = _player->GetPet();

    if(!pet || pet->getPetType() != HUNTER_PET || pet->m_spells.size() <= 1)
        return;

    if(guid != pet->GetGUID())
    {
        TC_LOG_ERROR( "network","HandlePetUnlearnOpcode.Pet %u isn't pet of player %s .\n", uint32(guid.GetCounter()),GetPlayer()->GetName().c_str() );
        return;
    }

    CharmInfo *charmInfo = pet->GetCharmInfo();
    if(!charmInfo)
    {
        TC_LOG_ERROR("network","WorldSession::HandlePetUnlearnOpcode: object %u is considered pet-like but doesn't have a charminfo!", pet->GetGUID().GetCounter());
        return;
    }

    uint32 cost = pet->ResetTalentsCost();

    if (GetPlayer()->GetMoney() < cost)
    {
        GetPlayer()->SendBuyError( BUY_ERR_NOT_ENOUGHT_MONEY, nullptr, 0, 0);
        return;
    }

    for(auto itr = pet->m_spells.begin(); itr != pet->m_spells.end();)
    {
        uint32 spell_id = itr->first;                       // Pet::removeSpell can invalidate iterator at erase NEW spell
        ++itr;
        pet->RemoveSpell(spell_id, false, true);
    }

    pet->SetTP(pet->GetLevel() * (pet->GetLoyaltyLevel() - 1));

    // relearn pet passives
    pet->LearnPetPassives();

    pet->m_resetTalentsTime = time(nullptr);
    pet->m_resetTalentsCost = cost;
    GetPlayer()->ModifyMoney(-(int32)cost);

    GetPlayer()->PetSpellInitialize();
}

void WorldSession::HandlePetSpellAutocastOpcode( WorldPacket& recvPacket )
{
    //TC_LOG_DEBUG("network","CMSG_PET_SPELL_AUTOCAST");
    ObjectGuid guid;
    uint16 spellid;
    uint16 spellid2;                                        //maybe second spell, automatically toggled off when first toggled on?
    uint8  state;                                           //1 for on, 0 for off

    recvPacket >> guid;
    recvPacket >> spellid;
#ifndef LICH_KING
    recvPacket >> spellid2;
#endif
    recvPacket >> state;

    if (!_player->GetGuardianPet() && !_player->GetCharmed())
        return;

    if(guid.IsPlayer())
        return;

    Creature* pet = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, guid);

    if(!pet || (pet != _player->GetGuardianPet() && pet != _player->GetCharmed()))
    {
        TC_LOG_ERROR("network", "HandlePetSpellAutocastOpcode.Pet %u isn't pet of player %s .\n", uint32(guid.GetCounter()),GetPlayer()->GetName().c_str() );
        return;
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellid);
    if(!spellInfo)
    {
        TC_LOG_ERROR("network","WORLD: unknown PET spell id %i\n", spellid);
        return;
    }

    // do not add not learned spells/ passive spells
    if (!pet->HasSpell(spellid) || !spellInfo->IsAutocastable())
        return;

    CharmInfo *charmInfo = pet->GetCharmInfo();
    if(!charmInfo)
    {
        TC_LOG_ERROR("network","WorldSession::HandlePetSpellAutocastOpcod: object %u is considered pet-like but doesn't have a charminfo!", pet->GetGUID().GetCounter());
        return;
    }

    if (pet->IsPet())
        ((Pet*)pet)->ToggleAutocast(spellInfo, state != 0);
    else
        pet->GetCharmInfo()->ToggleCreatureAutocast(spellInfo, state != 0);

    charmInfo->SetSpellAutocast(spellInfo, state != 0);
}

void WorldSession::HandlePetCastSpellOpcode( WorldPacket& recvPacket )
{
    //TC_LOG_DEBUG("network","WORLD: CMSG_PET_CAST_SPELL");
    ObjectGuid guid;
    uint32 spellid;

#ifdef LICH_KING
    recvPacket >> guid >> castCount >> spellId >> castFlags;
#else
    recvPacket >> guid >> spellid;
#endif

    // This opcode is also sent from charmed and possessed units (players and creatures)
    if(!_player->GetGuardianPet() && !_player->GetCharmed())
        return;

    Unit* caster = ObjectAccessor::GetUnit(*_player, guid);

    if(!caster || (caster != _player->GetGuardianPet() && caster != _player->GetCharmed()))
    {
        TC_LOG_ERROR("network", "HandlePetCastSpellOpcode: Pet %u isn't pet of player %s .\n", uint32(guid.GetCounter()),GetPlayer()->GetName().c_str() );
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

    SpellCastTargets targets;
    targets.Read(recvPacket, caster);
#ifdef LICH_KING
    HandleClientCastFlags(recvPacket, castFlags, targets);
#endif

    caster->ClearUnitState(UNIT_STATE_FOLLOW);
    bool triggered = spellid == 33395; //Freeze

    auto spell = new Spell(caster, spellInfo, triggered ? TRIGGERED_FULL_MASK : TRIGGERED_NONE);
    spell->m_targets = targets;

    SpellCastResult result = spell->CheckPetCast(nullptr);
    if (spellid == 33395) { //Water elemental Freeze HACKZ
        result = spell->CheckRange(true);
    }
    
    if(result == SPELL_CAST_OK)
    {
        if(caster->GetTypeId() == TYPEID_UNIT)
        {
            Creature* pet = caster->ToCreature();
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

        spell->prepare((spell->m_targets));
    }
    else
    {
        caster->SendPetCastFail(spellid, result);

        if (!caster->GetSpellHistory()->HasCooldown(spellid))
            caster->GetSpellHistory()->ResetCooldown(spellid, true);

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
        for(auto & i : declinedName->name)
            data << i;
    }
    else
        data << uint8(0);
    SendPacket(&data);
}

