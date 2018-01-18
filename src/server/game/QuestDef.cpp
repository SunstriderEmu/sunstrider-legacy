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

#include "QuestDef.h"
#include "Player.h"
#include "World.h"

Quest::Quest(Field * questRecord)
{
    uint32 i = 0;
    QuestId = questRecord[i++].GetUInt32();
    QuestMethod = questRecord[i++].GetUInt8();
    ZoneOrSort = questRecord[i++].GetInt16();
    SkillOrClass = questRecord[i++].GetInt16();
    MinLevel = questRecord[i++].GetUInt8();
    QuestLevel = questRecord[i++].GetUInt8();
    Type = (QuestTypes)(questRecord[i++].GetUInt16());
    RequiredRaces = questRecord[i++].GetUInt16();
    RequiredSkillValue = questRecord[i++].GetUInt16();
    RepObjectiveFaction = questRecord[i++].GetUInt16();
    RepObjectiveValue = questRecord[i++].GetInt32();
    RequiredMinRepFaction = questRecord[i++].GetUInt16();
    RequiredMinRepValue = questRecord[i++].GetInt32();
    RequiredMaxRepFaction = questRecord[i++].GetUInt16();
    RequiredMaxRepValue = questRecord[i++].GetInt32();
    SuggestedPlayers = questRecord[i++].GetUInt8();
    LimitTime = questRecord[i++].GetUInt32();
    QuestFlags = questRecord[i++].GetUInt16();
    uint8 SpecialFlags = questRecord[i++].GetUInt8();
    CharTitleId = questRecord[i++].GetUInt8();
    PrevQuestId = questRecord[i++].GetInt32();
    NextQuestId = questRecord[i++].GetInt32();
    ExclusiveGroup = questRecord[i++].GetInt32();
    _rewardNextQuest = questRecord[i++].GetUInt32();
    SrcItemId = questRecord[i++].GetUInt32();
    SrcItemCount = questRecord[i++].GetUInt8();
    SrcSpell = questRecord[i++].GetUInt32();
    Title = questRecord[i++].GetString();
    Details = questRecord[i++].GetString();
    Objectives = questRecord[i++].GetString();
    EndText = questRecord[i++].GetString();

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
        ObjectiveText[j] = questRecord[i++].GetString();

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
        RequiredItemId[j] = questRecord[i++].GetUInt32();

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
        RequiredItemCount[j] = questRecord[i++].GetUInt16();

    for (int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; ++j)
        RequiredSourceItemId[j] = questRecord[i++].GetUInt32();

    for (int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; ++j)
        RequiredSourceItemCount[j] = questRecord[i++].GetUInt16();

    for (int j = 0; j < QUEST_SOURCE_ITEM_IDS_COUNT; ++j)
        ReqSourceRef[j] = questRecord[i++].GetUInt8();

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
        RequiredNpcOrGo[j] = questRecord[i++].GetInt32();

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
        RequiredNpcOrGoCount[j] = questRecord[i++].GetUInt16();

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
        ReqSpell[j] = questRecord[i++].GetUInt32();

    for (int j = 0; j < QUEST_REWARD_CHOICES_COUNT; ++j)
        RewardChoiceItemId[j] = questRecord[i++].GetUInt32();

    for (int j = 0; j < QUEST_REWARD_CHOICES_COUNT; ++j)
        RewardChoiceItemCount[j] = questRecord[i++].GetUInt16();

    for (int j = 0; j < QUEST_REWARDS_COUNT; ++j)
        RewardItemId[j] = questRecord[i++].GetUInt32();

    for (int j = 0; j < QUEST_REWARDS_COUNT; ++j)
        RewardItemIdCount[j] = questRecord[i++].GetUInt16();

    for (int j = 0; j < QUEST_REPUTATIONS_COUNT; ++j)
        RewardRepFaction[j] = questRecord[i++].GetUInt16();

    for (int j = 0; j < QUEST_REPUTATIONS_COUNT; ++j)
        RewardRepValue[j] = questRecord[i++].GetInt32();

    RewardHonorableKills = questRecord[i++].GetUInt32();
    RewardOrReqMoney = questRecord[i++].GetInt32();
    RewardMoneyMaxLevel = questRecord[i++].GetUInt32();
    RewardSpell = questRecord[i++].GetUInt32();
    RewardSpellCast = questRecord[i++].GetUInt32();
    RewardMailTemplateId = questRecord[i++].GetUInt32();
    RewardMailDelaySecs = questRecord[i++].GetUInt32();
    PointMapId = questRecord[i++].GetUInt16();
    PointX = questRecord[i++].GetFloat();
    PointY = questRecord[i++].GetFloat();
    PointOpt = questRecord[i++].GetUInt32();

    QuestStartScript = questRecord[i++].GetUInt32();
    QuestCompleteScript = questRecord[i++].GetUInt32();
    m_markedAsBugged = questRecord[i++].GetBool();

    QuestFlags |= SpecialFlags << 16;

    m_reqitemscount = 0;
    m_reqCreatureOrGOcount = 0;
    m_rewitemscount = 0;
    m_rewchoiceitemscount = 0;

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
    {
        if ( RequiredItemId[i] )
            ++m_reqitemscount;
        if ( RequiredNpcOrGo[i] )
            ++m_reqCreatureOrGOcount;
    }

    for (uint32 j : RewardItemId)
    {
        if (j)
            ++m_rewitemscount;
    }

    for (uint32 j : RewardChoiceItemId)
    {
        if (j)
            ++m_rewchoiceitemscount;
    }
}

void Quest::LoadQuestDetails(Field* fields)
{
    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
    {
        if (!sEmotesStore.LookupEntry(fields[1 + i].GetUInt16()))
        {
            TC_LOG_ERROR("sql.sql", "Table `quest_details` has non-existing Emote%i (%u) set for quest %u. Skipped.", 1 + i, fields[1 + i].GetUInt16(), fields[0].GetUInt32());
            continue;
        }

        DetailsEmote[i] = fields[1 + i].GetUInt16();
    }

    for (int i = 0; i < QUEST_EMOTE_COUNT; ++i)
        DetailsEmoteDelay[i] = fields[5 + i].GetUInt32();
}

void Quest::LoadQuestRequestItems(Field* fields)
{
    _emoteOnComplete = fields[1].GetUInt16();
    _emoteOnIncomplete = fields[2].GetUInt16();

    if (!sEmotesStore.LookupEntry(_emoteOnComplete))
        TC_LOG_ERROR("sql.sql", "Table `quest_request_items` has non-existing EmoteOnComplete (%u) set for quest %u.", _emoteOnComplete, fields[0].GetUInt32());

    if (!sEmotesStore.LookupEntry(_emoteOnIncomplete))
        TC_LOG_ERROR("sql.sql", "Table `quest_request_items` has non-existing EmoteOnIncomplete (%u) set for quest %u.", _emoteOnIncomplete, fields[0].GetUInt32());

    _requestItemsText = fields[3].GetString();
}

void Quest::LoadQuestOfferReward(Field* fields)
{
    for (uint32 i = 0; i < QUEST_EMOTE_COUNT; ++i)
    {
        if (!sEmotesStore.LookupEntry(fields[1 + i].GetUInt16()))
        {
            TC_LOG_ERROR("sql.sql", "Table `quest_offer_reward` has non-existing Emote%i (%u) set for quest %u. Skipped.", 1 + i, fields[1 + i].GetUInt16(), fields[0].GetUInt32());
            continue;
        }

        OfferRewardEmote[i] = fields[1 + i].GetUInt16();
    }

    for (uint32 i = 0; i < QUEST_EMOTE_COUNT; ++i)
        OfferRewardEmoteDelay[i] = fields[5 + i].GetUInt32();

    _offerRewardText = fields[9].GetString();
}

uint32 Quest::XPValue( Player *pPlayer ) const
{
    if( pPlayer )
    {
        if( RewardMoneyMaxLevel > 0 )
        {
            uint32 pLevel = pPlayer->GetLevel();
            uint32 qLevel = QuestLevel;
            float fullxp = 0;
            if (qLevel >= 65)
                fullxp = RewardMoneyMaxLevel / 6.0f;
            else if (qLevel == 64)
                fullxp = RewardMoneyMaxLevel / 4.8f;
            else if (qLevel == 63)
                fullxp = RewardMoneyMaxLevel / 3.6f;
            else if (qLevel == 62)
                fullxp = RewardMoneyMaxLevel / 2.4f;
            else if (qLevel == 61)
                fullxp = RewardMoneyMaxLevel / 1.2f;
            else if (qLevel > 0 && qLevel <= 60)
                fullxp = RewardMoneyMaxLevel / 0.6f;

            if( pLevel <= qLevel +  5 )
                return (uint32)fullxp;
            else if( pLevel == qLevel +  6 )
                return (uint32)(fullxp * 0.8f);
            else if( pLevel == qLevel +  7 )
                return (uint32)(fullxp * 0.6f);
            else if( pLevel == qLevel +  8 )
                return (uint32)(fullxp * 0.4f);
            else if( pLevel == qLevel +  9 )
                return (uint32)(fullxp * 0.2f);
            else
                return (uint32)(fullxp * 0.1f);
        }
    }
    return 0;
}

int32  Quest::GetRewOrReqMoney() const
{
    if(RewardOrReqMoney <=0)
        return RewardOrReqMoney;

    return int32(RewardOrReqMoney * sWorld->GetRate(RATE_DROP_MONEY));
}

bool Quest::IsAutoComplete() const 
{ 
    return QuestMethod == 0 
#ifdef LICH_KING
        || HasFlag(QUEST_FLAGS_AUTOCOMPLETE)
#endif
        ;
}