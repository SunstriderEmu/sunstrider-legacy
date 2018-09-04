
#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "GossipDef.h"
#include "QuestDef.h"
#include "ObjectAccessor.h"
#include "Group.h"
#include "BattleGround.h"
#include "BattleGroundAV.h"
#include "Chat.h"
#include "ScriptMgr.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "GameObjectAI.h"
#include "Language.h"
#include "ScriptedGossip.h"
#include "QuestPackets.h"

void WorldSession::HandleQuestgiverStatusQueryOpcode( WorldPacket & recvData )
{
    ObjectGuid guid;
    recvData >> guid;
    uint8 questStatus = DIALOG_STATUS_NONE;

    Object* questgiver = ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT);
    if (!questgiver) {
        TC_LOG_ERROR("network.opcode","Error in CMSG_QUESTGIVER_STATUS_QUERY, called for not found questgiver (Typeid: %u GUID: %u)", GuidHigh2TypeId(guid.GetHigh()), guid.GetCounter());
        return;
    }

    switch (questgiver->GetTypeId()) {
    case TYPEID_UNIT:
    {
        /*
        Creature* cr_questgiver = questgiver->ToCreature();
        if (!cr_questgiver->IsHostileTo(_player)) // not show quest status to enemies
        {
            questStatus = sScriptMgr->GetDialogStatus(_player, cr_questgiver);
            if (questStatus > 6)
                questStatus = getquestdialogstatus(_player, cr_questgiver, defstatus);
        }
        */
        if (!questgiver->ToCreature()->IsHostileTo(_player)) // do not show quest status to enemies
            questStatus = _player->GetQuestDialogStatus(questgiver);
        break;
    }
    case TYPEID_GAMEOBJECT:
    {
        _player->GetQuestDialogStatus(questgiver);
        break;
    }
    default:
        TC_LOG_ERROR("network.opcode","QuestGiver called for unexpected type %u", questgiver->GetTypeId());
        break;
    }

    //inform client about status of quest
    _player->PlayerTalkClass->SendQuestGiverStatus(questStatus, guid);
}

void WorldSession::HandleQuestgiverHelloOpcode( WorldPacket & recvData )
{
    ObjectGuid guid;
    recvData >> guid;

    Creature* pCreature = GetPlayer()->GetNPCIfCanInteractWith(guid);
    if (!pCreature)
    {
        TC_LOG_ERROR ("network","WORLD: HandleQuestgiverHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.",
            guid.GetCounter());
        return;
    }

    // remove fake death
    if(GetPlayer()->HasUnitState(UNIT_STATE_DIED))
        GetPlayer()->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);

    // Stop the npc if moving
    pCreature->PauseMovement(sWorld->getIntConfig(CONFIG_CREATURE_STOP_FOR_PLAYER));
    pCreature->SetHomePosition(pCreature->GetPosition());

    if (pCreature->AI()->GossipHello(_player))
        return;

    _player->PrepareGossipMenu(pCreature, _player->GetDefaultGossipMenuForSource(pCreature), true);
    _player->SendPreparedGossip(pCreature);
}

void WorldSession::HandleQuestgiverAcceptQuestOpcode( WorldPacket & recvData )
{
    ObjectGuid guid;
    uint32 questId;
    recvData >> guid >> questId;
#ifdef LICH_KING
    uint32 startCheat;
    recvData >> startCheat;
#endif

    //TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUESTGIVER_ACCEPT_QUEST %s, quest = %u, startCheat = %u", guid.ToString().c_str(), questId);

    Object* object;
    if (!guid.IsPlayer())
        object = ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT | TYPEMASK_ITEM);
    else
        object = ObjectAccessor::FindPlayer(guid);

#define CLOSE_GOSSIP_CLEAR_SHARING_INFO() \
    do { \
        _player->PlayerTalkClass->SendCloseGossip(); \
        _player->ClearQuestSharingInfo(); \
    } while (0)

    // no or incorrect quest giver
    if (!object)
    {
        CLOSE_GOSSIP_CLEAR_SHARING_INFO();
        return;
    }


    if (Player* playerQuestObject = object->ToPlayer())
    {
        if ((_player->GetPlayerSharingQuest() && _player->GetPlayerSharingQuest() != guid) || !playerQuestObject->CanShareQuest(questId))
        {
            CLOSE_GOSSIP_CLEAR_SHARING_INFO();
            return;
        }
        if (!_player->IsInSameRaidWith(playerQuestObject))
        {
            CLOSE_GOSSIP_CLEAR_SHARING_INFO();
            return;
        }
    }
    else
    {
        if (!object->HasQuest(questId))
        {
            CLOSE_GOSSIP_CLEAR_SHARING_INFO();
            return;
        }
    }

    // some kind of WPE protection
    if (!_player->CanInteractWithQuestGiver(object))
    {
        CLOSE_GOSSIP_CLEAR_SHARING_INFO();
        return;
    }

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
    {
        // prevent cheating
        if(!GetPlayer()->CanTakeQuest(quest,true) )
        {
            CLOSE_GOSSIP_CLEAR_SHARING_INFO();
            return;
        }

        if (!_player->GetPlayerSharingQuest().IsEmpty())
        {
            Player* player = ObjectAccessor::FindPlayer(_player->GetPlayerSharingQuest());
            if (player)
            {
                player->SendPushToPartyResponse(_player, QUEST_PARTY_MSG_ACCEPT_QUEST);
                _player->ClearQuestSharingInfo();
            }
        }

        if (_player->CanAddQuest(quest, true))
        {
            _player->AddQuestAndCheckCompletion(quest, object);

            if (quest->HasFlag(QUEST_FLAGS_PARTY_ACCEPT))
            {
                if (Group* pGroup = _player->GetGroup())
                {
                    for (GroupReference *itr = pGroup->GetFirstMember(); itr != nullptr; itr = itr->next())
                    {
                        Player* player = itr->GetSource();

                        if (!player || player == _player)     // not self
                            continue;

                        if (player->IsAtGroupRewardDistance(_player) && player->CanTakeQuest(quest, true))
                        {
                            player->SetQuestSharingInfo(_player->GetGUID(), questId);

                            //need confirmation that any gossip window will close
                            player->PlayerTalkClass->SendCloseGossip();

                            _player->SendQuestConfirmAccept(quest, player);
                        }
                    }
                }
            }

            _player->PlayerTalkClass->SendCloseGossip();

            if (quest->GetSrcSpell() > 0)
                _player->CastSpell( _player, quest->GetSrcSpell(), true);

            return;
        }
    }

    CLOSE_GOSSIP_CLEAR_SHARING_INFO();

#undef CLOSE_GOSSIP_CLEAR_SHARING_INFO
}

void WorldSession::HandleQuestgiverQueryQuestOpcode( WorldPacket & recvData )
{
    ObjectGuid guid;
    uint32 quest;
    recvData >> guid >> quest;

    // Verify that the guid is valid and is a questgiver or involved in the requested quest
    Object* pObject = ObjectAccessor::GetObjectByTypeMask(*_player, guid,TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT|TYPEMASK_ITEM);
    if(!pObject || (!pObject->HasQuest(quest) && !pObject->HasInvolvedQuest(quest)))
    {
        _player->PlayerTalkClass->SendCloseGossip();
        return;
    }

    Quest const* pQuest = sObjectMgr->GetQuestTemplate(quest);
    if ( pQuest )
    {
        _player->PlayerTalkClass->SendQuestGiverQuestDetails(pQuest, pObject->GetGUID(), true);
    }
}

void WorldSession::HandleQuestQueryOpcode(WorldPackets::Quest::QueryQuestInfo& query)
{
    //TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUEST_QUERY quest = %u", query.QuestID);

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(query.QuestID))
        _player->PlayerTalkClass->SendQuestQueryResponse(quest);
}

void WorldSession::HandleQuestgiverChooseRewardOpcode( WorldPacket & recvData )
{
    uint32 quest, reward;
    ObjectGuid guid;
    recvData >> guid >> quest >> reward;

    if(reward >= QUEST_REWARD_CHOICES_COUNT)
    {
        TC_LOG_ERROR("FIXME","Error in CMSG_QUESTGIVER_CHOOSE_REWARD: player %s (guid %d) tried to get invalid reward (%u) (probably packet hacking)", _player->GetName().c_str(), _player->GetGUID().GetCounter(), reward);
        return;
    }

    if(GetPlayer()->IsDying())
        return;

    Object* pObject = ObjectAccessor::GetObjectByTypeMask(*_player, guid,TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);
    if(!pObject)
        return;

    if(!pObject->HasInvolvedQuest(quest))
        return;

    Quest const *pQuest = sObjectMgr->GetQuestTemplate(quest);
    if( pQuest )
    {
        if( _player->CanRewardQuest( pQuest, reward, true ) )
        {
            _player->RewardQuest( pQuest, reward, pObject );

            switch(pObject->GetTypeId())
            {
                case TYPEID_UNIT:
                {
                    // Send next quest
                    Creature* questgiver = pObject->ToCreature();
                    if (Quest const* nextquest = _player->GetNextQuest(guid, pQuest))
                        _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextquest, guid, true);

                    ClearGossipMenuFor(_player);
                    questgiver->AI()->QuestReward(_player, pQuest, reward);
                    break;
                }
                case TYPEID_GAMEOBJECT:
                    // Send next quest
                    if (Quest const* nextQuest = _player->GetNextQuest(guid, pQuest))
                    {
                        // Only send the quest to the player if the conditions are met
                        if (_player->CanTakeQuest(nextQuest, false)) {
                            /*TC 
                             if (nextQuest->IsAutoAccept() && _player->CanAddQuest(nextQuest, true))
                                _player->AddQuestAndCheckCompletion(nextQuest, object);
                                */
                            _player->PlayerTalkClass->SendQuestGiverQuestDetails(nextQuest, guid, true);
                        }
                    }

                    ClearGossipMenuFor(_player);
                    (pObject->ToGameObject())->AI()->QuestReward(_player, pQuest, reward);
                    break;
            }
        }
        else
            _player->PlayerTalkClass->SendQuestGiverOfferReward( pQuest, guid, true );
    }
}

void WorldSession::HandleQuestgiverRequestRewardOpcode( WorldPacket & recvData )
{
    uint32 quest;
    ObjectGuid guid;
    recvData >> guid >> quest;

    Object* pObject = ObjectAccessor::GetObjectByTypeMask(*_player, guid,TYPEMASK_UNIT|TYPEMASK_GAMEOBJECT);
    if(!pObject||!pObject->HasInvolvedQuest(quest))
        return;

    if ( _player->CanCompleteQuest( quest ) )
        _player->CompleteQuest( quest );

    if( _player->GetQuestStatus( quest ) != QUEST_STATUS_COMPLETE )
        return;

    if(Quest const *pQuest = sObjectMgr->GetQuestTemplate(quest))
        _player->PlayerTalkClass->SendQuestGiverOfferReward( pQuest, guid, true );
}

void WorldSession::HandleQuestgiverCancel(WorldPacket& /*recvData*/ )
{
    _player->PlayerTalkClass->SendCloseGossip();
}

void WorldSession::HandleQuestLogSwapQuest(WorldPacket& recvData )
{
    uint8 slot1, slot2;
    recvData >> slot1 >> slot2;

    if(slot1 == slot2 || slot1 >= MAX_QUEST_LOG_SIZE || slot2 >= MAX_QUEST_LOG_SIZE)
        return;

    GetPlayer()->SwapQuestSlot(slot1,slot2);
}

void WorldSession::HandleQuestLogRemoveQuest(WorldPacket& recvData)
{
    uint8 slot;
    recvData >> slot;

    if( slot < MAX_QUEST_LOG_SIZE )
    {
        if(uint32 questId = _player->GetQuestSlotQuestId(slot))
        {
            if(!_player->TakeQuestSourceItem(questId, true ))
                return;                                     // can't un-equip some items, reject quest cancel

            /* TC
            if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
            {
                if (quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED))
                    _player->RemoveTimedQuest(questId);

                if (quest->HasFlag(QUEST_FLAGS_FLAGS_PVP))
                {
                    _player->pvpInfo.IsHostile = _player->pvpInfo.IsInHostileArea || _player->HasPvPForcingQuest();
                    _player->UpdatePvPState();
                }
            }
            */

            _player->SetQuestStatus(questId, QUEST_STATUS_NONE);
        }

        _player->SetQuestSlot(slot, 0);
    }
}

void WorldSession::HandleQuestConfirmAccept(WorldPacket& recvData)
{
    uint32 questId;
    recvData >> questId;

    TC_LOG_DEBUG("network", "WORLD: Received CMSG_QUEST_CONFIRM_ACCEPT quest = %u", questId);

    if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
    {
        if (!quest->HasFlag(QUEST_FLAGS_PARTY_ACCEPT))
            return;

        Player* originalPlayer = ObjectAccessor::FindPlayer(_player->GetPlayerSharingQuest());
        if (!originalPlayer)
            return;

        if (!_player->IsInSameRaidWith(originalPlayer))
            return;

        if (!originalPlayer->IsActiveQuest(questId))
            return;

        if (!_player->CanTakeQuest(quest, true))
            return;

        if (_player->CanAddQuest(quest, true))
        {
            _player->AddQuestAndCheckCompletion(quest, nullptr); // NULL, this prevent DB script from duplicate running

            if (quest->GetSrcSpell() > 0)
                _player->CastSpell(_player, quest->GetSrcSpell(), true);
        }
    }

    _player->ClearQuestSharingInfo();
}

void WorldSession::HandleQuestgiverCompleteQuest(WorldPacket& recvData)
{
    uint32 questID;
    ObjectGuid guid;
    recvData >> guid >> questID;

    Quest const* quest = sObjectMgr->GetQuestTemplate(questID);
    if(!quest)
        return;

    Object* object = ObjectAccessor::GetObjectByTypeMask(*_player, guid, TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT);
    if (!object || !object->HasInvolvedQuest(questID))
        return;

    // some kind of WPE protection
    if (!_player->CanInteractWithQuestGiver(object))
        return;

    // TODO: need a virtual function
    if(GetPlayer()->InBattleground())
        if(Battleground* bg = GetPlayer()->GetBattleground())
            if(bg->GetTypeID() == BATTLEGROUND_AV)
                ((BattlegroundAV*)bg)->HandleQuestComplete(questID, GetPlayer());

    if( _player->GetQuestStatus( questID ) != QUEST_STATUS_COMPLETE )
    {
        if( quest->IsRepeatable() )
            _player->PlayerTalkClass->SendQuestGiverRequestItems(quest, guid, _player->CanCompleteRepeatableQuest(quest), false);
        else
            _player->PlayerTalkClass->SendQuestGiverRequestItems(quest, guid, _player->CanRewardQuest(quest,false), false);
    }
    else
        _player->PlayerTalkClass->SendQuestGiverRequestItems(quest, guid, _player->CanRewardQuest(quest,false), false);
}

void WorldSession::HandleQuestgiverQuestAutoLaunch(WorldPacket& /*recvPacket*/)
{
    
}

void WorldSession::HandlePushQuestToParty(WorldPacket& recvPacket)
{
    uint32 questId;
    recvPacket >> questId;

    Player* sender = _player;
    if (!sender->CanShareQuest(questId))
        return;

    Group* group = sender->GetGroup();
    if (!group)
    {
#ifdef LICH_KING
        sender->SendPushToPartyResponse(sender, QUEST_PARTY_MSG_NOT_IN_PARTY);
#else
        ChatHandler(sender).PSendSysMessage("You are not in a party.");
#endif
        return;
    }

    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest)
        return;

    for(GroupReference *itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* receiver = itr->GetSource();
        if (!receiver || receiver == sender)     // skip self
            continue;

        if(sender->GetDistance(receiver) > 10.0f )
        {
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_TOO_FAR );
            continue;
        }

        if( !receiver->SatisfyQuestStatus( quest, false ) )
        {
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_HAVE_QUEST );
            continue;
        }

        if(receiver->GetQuestStatus( questId ) == QUEST_STATUS_COMPLETE )
        {
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_FINISH_QUEST );
            continue;
        }

        if (!receiver->SatisfyQuestDay(quest, false))
        {
#ifdef LICH_KING
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_NOT_ELIGIBLE_TODAY);
#else
            ChatHandler(sender).PSendSysMessage("You can't take any more daily quests.");
#endif  
            continue;
        }

        if (sWorld->IsQuestInAPool(questId)) {
            if (!sWorld->IsQuestCurrentOfAPool(questId)) {
                //ChatHandler(sender).PSendSysMessage("Cette quête n'est pas disponible aujourd'hui, vous ne pouvez pas la partager.");
                ChatHandler(sender).PSendSysMessage("This quest is not available today, you can't share it.");
                break;  // Quest cannot be shared today, no point continuing
            }
        }

        if( !receiver->CanTakeQuest(quest, false ) )
        {
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_CANT_TAKE_QUEST );
            continue;
        }

        if( !receiver->SatisfyQuestLog( false ) )
        {
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_LOG_FULL );
            continue;
        }

        if (receiver->GetPlayerSharingQuest())
        {
            sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_BUSY );
            continue;
        }

        sender->SendPushToPartyResponse(receiver, QUEST_PARTY_MSG_SHARING_QUEST);
                    
        if (quest->IsAutoAccept() && receiver->CanAddQuest(quest, true) && receiver->CanTakeQuest(quest, true))
            receiver->AddQuestAndCheckCompletion(quest, sender);

        if ((quest->IsAutoComplete() && quest->IsRepeatable() && !quest->IsDailyOrWeekly()))
            receiver->PlayerTalkClass->SendQuestGiverRequestItems(quest, sender->GetGUID(), receiver->CanCompleteRepeatableQuest(quest), true);
        else
        {
            receiver->SetQuestSharingInfo(sender->GetGUID(), questId);
            receiver->PlayerTalkClass->SendQuestGiverQuestDetails(quest, receiver->GetGUID(), true);
        }

    }
}

void WorldSession::HandleQuestPushResult(WorldPacket& recvPacket)
{
    ObjectGuid guid;
    uint8 msg;
#ifdef LICH_KING
    uint32 questId;
    recvPacket >> guid >> questId >> msg;
#else
    recvPacket >> guid >> msg;
#endif

    if (!_player->GetPlayerSharingQuest())
        return;

    if (_player->GetPlayerSharingQuest() == guid)
    {
        Player* player = ObjectAccessor::FindPlayer(guid);
        if (player)
            player->SendPushToPartyResponse(_player, msg);
    }

    _player->ClearQuestSharingInfo();
}

QuestGiverStatus Player::GetQuestDialogStatus(Object* questgiver)
{
    QuestGiverStatus result = DIALOG_STATUS_NONE;

    QuestRelations const* qir;
    QuestRelations const* qr;

    switch(questgiver->GetTypeId())
    {
        case TYPEID_GAMEOBJECT:
        {
            if (auto questStatus = questgiver->ToGameObject()->AI()->GetDialogStatus(this))
                return *questStatus;
            qir = &sObjectMgr->mGOQuestInvolvedRelations;
            qr  = &sObjectMgr->mGOQuestRelations;
            break;
        }
        case TYPEID_UNIT:
        {
            if (auto questStatus = questgiver->ToCreature()->AI()->GetDialogStatus(this))
                return *questStatus;

            qir = &sObjectMgr->mCreatureQuestInvolvedRelations;
            qr  = &sObjectMgr->mCreatureQuestRelations;
            break;
        }
        default:
            //its imposible, but check ^)
            TC_LOG_ERROR("entities.player.quest","Warning: GetDialogStatus called for unexpected type %u", questgiver->GetTypeId());
            return DIALOG_STATUS_NONE;
    }

    for(auto i = qir->lower_bound(questgiver->GetEntry()); i != qir->upper_bound(questgiver->GetEntry()); ++i )
    {
        QuestGiverStatus result2 = DIALOG_STATUS_NONE;
        uint32 quest_id = i->second;
        Quest const *pQuest = sObjectMgr->GetQuestTemplate(quest_id);
        if ( !pQuest ) 
            continue;

        if (!sConditionMgr->IsObjectMeetingNotGroupedConditions(CONDITION_SOURCE_TYPE_QUEST_ACCEPT, pQuest->GetQuestId(), this))
            continue;

        QuestStatus status = this->GetQuestStatus( quest_id );
        if( (status == QUEST_STATUS_COMPLETE && !this->GetQuestRewardStatus(quest_id)) ||
            (pQuest->IsAutoComplete() && this->CanTakeQuest(pQuest, false)) )
        {
            if ( pQuest->IsAutoComplete() && pQuest->IsRepeatable() )
                result2 = DIALOG_STATUS_REWARD_REP;
            else
                result2 = DIALOG_STATUS_REWARD;
        }
        else if ( status == QUEST_STATUS_INCOMPLETE )
            result2 = DIALOG_STATUS_INCOMPLETE;

        if (result2 > result)
            result = result2;
    }

    for(auto i = qr->lower_bound(questgiver->GetEntry()); i != qr->upper_bound(questgiver->GetEntry()); ++i )
    {
        QuestGiverStatus result2 = DIALOG_STATUS_NONE;
        uint32 quest_id = i->second;
        Quest const *pQuest = sObjectMgr->GetQuestTemplate(quest_id);
        if ( !pQuest )
            continue;

        if (!sConditionMgr->IsObjectMeetingNotGroupedConditions(CONDITION_SOURCE_TYPE_QUEST_ACCEPT, pQuest->GetQuestId(), this))
            continue;

        QuestStatus status = this->GetQuestStatus( quest_id );
        if ( status == QUEST_STATUS_NONE )
        {
            if ( this->CanSeeStartQuest( pQuest ) )
            {
                if (this->SatisfyQuestLevel(pQuest, false) )
                {
                    if ( pQuest->IsAutoComplete() || (pQuest->IsRepeatable() && this->getQuestStatusMap()[quest_id].Rewarded))
                        result2 = DIALOG_STATUS_REWARD_REP;
                    else if (this->GetLevel() <= pQuest->GetQuestLevel() + sWorld->getConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF) )
                    {
                        if (pQuest->HasFlag(QUEST_FLAGS_DAILY))
                            result2 = DIALOG_STATUS_AVAILABLE_REP;
                        else
                            result2 = DIALOG_STATUS_AVAILABLE;
                    }
                    else
                        result2 = DIALOG_STATUS_CHAT;
                }
                else
                    result2 = DIALOG_STATUS_UNAVAILABLE;
            }
        }

        if (result2 > result)
            result = result2;
    }

    return result;
}

void WorldSession::HandleQuestgiverStatusMultipleQuery(WorldPacket& /*recvPacket*/)
{
    uint32 count = 0;

    WorldPacket data(SMSG_QUESTGIVER_STATUS_MULTIPLE, 4);
    data << uint32(count);                                  // placeholder

    for(auto m_clientGUID : _player->m_clientGUIDs)
    {
        uint8 questStatus = DIALOG_STATUS_NONE;

        if (m_clientGUID.IsAnyTypeCreature())
        {
            Creature *questgiver = ObjectAccessor::GetCreatureOrPetOrVehicle(*_player, m_clientGUID);
            if(!questgiver || questgiver->IsHostileTo(_player))
                continue;
            if(!questgiver->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
                continue;
            questStatus = _player->GetQuestDialogStatus(questgiver);
            if( questStatus > 6 )
                questStatus = _player->GetQuestDialogStatus(questgiver);

            data << uint64(questgiver->GetGUID());
            data << uint8(questStatus);
            ++count;
        }
        else if(m_clientGUID.IsGameObject())
        {
            GameObject *questgiver = ObjectAccessor::GetGameObject(*_player, m_clientGUID);
            if(!questgiver)
                continue;
            if(questgiver->GetGoType() != GAMEOBJECT_TYPE_QUESTGIVER)
                continue;
            questStatus = _player->GetQuestDialogStatus(questgiver);
            if( questStatus > 6 )
                questStatus = _player->GetQuestDialogStatus(questgiver);

            data << uint64(questgiver->GetGUID());
            data << uint8(questStatus);
            ++count;
        }
    }

    data.put<uint32>(0, count);                             // write real count
    SendPacket(&data);
}

