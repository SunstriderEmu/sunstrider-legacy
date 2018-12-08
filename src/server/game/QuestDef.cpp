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
#include "QuestPackets.h"
#include "ObjectMgr.h"
#include "Formulas.h"

Quest::Quest(Field* questRecord)
{
    uint32 i = 0;
    _id = questRecord[i++].GetUInt32();
    _method = questRecord[i++].GetUInt8();
    _zoneOrSort = questRecord[i++].GetInt16();
    MinLevel = questRecord[i++].GetUInt8();
    _level = questRecord[i++].GetUInt8();
    Type = (QuestTypes)(questRecord[i++].GetUInt16());
    _suggestedPlayers = questRecord[i++].GetUInt8();
    _timeAllowed = questRecord[i++].GetUInt32();
    _allowableRaces = questRecord[i++].GetUInt16();
    _requiredFactionId1 = questRecord[i++].GetUInt16();
    _requiredFactionValue1 = questRecord[i++].GetInt32();
    _flags = questRecord[i++].GetUInt16();
    _rewardTitleId = questRecord[i++].GetUInt8();
    _rewardNextQuest = questRecord[i++].GetUInt32();
    _startItem = questRecord[i++].GetUInt32();
    Title = questRecord[i++].GetString();
    Details = questRecord[i++].GetString();
    Objectives = questRecord[i++].GetString();
    AreaDescription = questRecord[i++].GetString();

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; ++j)
        ObjectiveText[j] = questRecord[i++].GetString();

    for (int j = 0; j < QUEST_ITEM_OBJECTIVES_COUNT; ++j)
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
        RewardFactionId[j] = questRecord[i++].GetUInt16();

    for (int j = 0; j < QUEST_REPUTATIONS_COUNT; ++j)
        RewardRepValue[j] = questRecord[i++].GetInt32();

    RewardHonorableKills = questRecord[i++].GetUInt32();
    RewardOrReqMoney = questRecord[i++].GetInt32();
    RewardMoneyMaxLevel = questRecord[i++].GetUInt32();
    _rewardDisplaySpell = questRecord[i++].GetUInt32();
    _rewardSpell = questRecord[i++].GetUInt32();
    PointMapId = questRecord[i++].GetUInt16();
    PointX = questRecord[i++].GetFloat();
    PointY = questRecord[i++].GetFloat();
    PointOpt = questRecord[i++].GetUInt32();

    QuestStartScript = questRecord[i++].GetUInt32();
    QuestCompleteScript = questRecord[i++].GetUInt32();

    _reqItemsCount = 0;
    _reqCreatureOrGOcount = 0;
    _rewItemsCount = 0;
    _rewChoiceItemsCount = 0;

    for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
        if ( RequiredNpcOrGo[j] )
            ++_reqCreatureOrGOcount;

    for (int j = 0; j < QUEST_ITEM_OBJECTIVES_COUNT; j++)
        if (RequiredItemId[j])
            ++_reqItemsCount;

    for (uint32 j : RewardItemId)
        if (j)
            ++_rewItemsCount;

    for (uint32 j : RewardChoiceItemId)
        if (j)
            ++_rewChoiceItemsCount;
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

void Quest::LoadQuestTemplateAddon(Field* fields)
{
    _maxLevel = fields[1].GetUInt8();
    _requiredClasses = fields[2].GetUInt32();
    _sourceSpellid = fields[3].GetUInt32();
    _prevQuestId = fields[4].GetInt32();
    _nextQuestId = fields[5].GetUInt32();
    _exclusiveGroup = fields[6].GetInt32();
    _rewardMailTemplateId = fields[7].GetUInt32();
    _rewardMailDelay = fields[8].GetUInt32();
    _requiredSkillId = fields[9].GetUInt16();
    _requiredSkillPoints = fields[10].GetUInt16();
    _requiredMinRepFaction = fields[11].GetUInt16();
    _requiredMaxRepFaction = fields[12].GetUInt16();
    _requiredMinRepValue = fields[13].GetInt32();
    _requiredMaxRepValue = fields[14].GetInt32();
    _startItemCount = fields[15].GetUInt8();
    _specialFlags = fields[16].GetUInt8();

#ifdef LICH_KING
    if (_specialFlags & QUEST_SPECIAL_FLAGS_AUTO_ACCEPT)
        _flags |= QUEST_FLAGS_AUTO_ACCEPT;
#endif
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

void Quest::LoadQuestMailSender(Field* fields)
{
    _rewardMailSenderEntry = fields[1].GetUInt32();
}

uint32 Quest::XPValue(Player* pPlayer) const
{
    if (pPlayer)
    {
        if (RewardMoneyMaxLevel > 0)
        {
            uint32 pLevel = pPlayer->GetLevel();
            uint32 qLevel = _level;
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
            else if (qLevel > 0 /*&& qLevel <= 60*/)
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

int32 Quest::GetRewOrReqMoney() const
{
    if(RewardOrReqMoney <=0)
        return RewardOrReqMoney;

    return int32(RewardOrReqMoney * sWorld->GetRate(RATE_DROP_MONEY));
}

bool Quest::IsAutoComplete() const 
{ 
    return _method == 0 
#ifdef LICH_KING
        || HasFlag(QUEST_FLAGS_AUTOCOMPLETE)
#endif
        ;
}

bool Quest::IsRaidQuest(Difficulty difficulty) const
{
    switch (Type)
    {
    case QUEST_TYPE_RAID:
        return true;
#ifdef LICH_KING
    case QUEST_TYPE_RAID_10:
        return !(difficulty & RAID_DIFFICULTY_MASK_25MAN);
    case QUEST_TYPE_RAID_25:
        return difficulty & RAID_DIFFICULTY_MASK_25MAN;
    default:
        break;
#endif
    }

    if ((_flags & QUEST_FLAGS_RAID) != 0)
        return true;

    return false;
}

bool Quest::IsAllowedInRaid(Difficulty difficulty) const
{
    if (IsRaidQuest(difficulty))
        return true;

    //TC return sWorld->getBoolConfig(CONFIG_QUEST_IGNORE_RAID);
    return false;
}

uint32 Quest::CalculateHonorGain(uint8 level) const
{
    uint32 honor = 0;

#ifdef LICH_KING
    if (level > GT_MAX_LEVEL)
        level = GT_MAX_LEVEL;

    if (GetRewHonorAddition() > 0 || GetRewHonorMultiplier() > 0.0f)
    {
        // values stored from 0.. for 1...
        TeamContributionPointsEntry const* tc = sTeamContributionPointsStore.LookupEntry(level);
        if (!tc)
            return 0;

        honor = uint32(tc->value * GetRewHonorMultiplier() * 0.1f);
        honor += GetRewHonorAddition();
    }
#else
    //quest as simply a kill count, we calculate honor given from that
    honor = Trinity::Honor::hk_honor_at_level(level, GetRewHonorableKills());
#endif

    return honor;
}

bool Quest::CanIncreaseRewardedQuestCounters() const
{
    // Dungeon Finder/Daily/Repeatable (if not weekly, monthly or seasonal) quests are never considered rewarded serverside.
    // This affects counters and client requests for completed quests.
    return (!IsDFQuest() && !IsDaily() && (!IsRepeatable() || IsWeekly() || IsMonthly() || IsSeasonal()));
}

void Quest::InitializeQueryData()
{
    for (uint8 loc = LOCALE_enUS; loc < TOTAL_LOCALES; ++loc)
        QueryData[loc] = BuildQueryData(static_cast<LocaleConstant>(loc));
}

WorldPacket Quest::BuildQueryData(LocaleConstant loc) const
{
    WorldPackets::Quest::QueryQuestInfoResponse response;

    std::string locQuestTitle = GetTitle();
    std::string locQuestDetails = GetDetails();
    std::string locQuestObjectives = GetObjectives();
    std::string locQuestAreaDescription = GetAreaDescription();
#ifdef LICH_KING
    std::string locQuestCompletedText = GetCompletedText();
#endif

    std::string locQuestObjectiveText[QUEST_OBJECTIVES_COUNT];
    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        locQuestObjectiveText[i] = ObjectiveText[i];

    if (QuestLocale const* localeData = sObjectMgr->GetQuestLocale(GetQuestId()))
    {
        ObjectMgr::GetLocaleString(localeData->Title, loc, locQuestTitle);
        ObjectMgr::GetLocaleString(localeData->Details, loc, locQuestDetails);
        ObjectMgr::GetLocaleString(localeData->Objectives, loc, locQuestObjectives);
        ObjectMgr::GetLocaleString(localeData->AreaDescription, loc, locQuestAreaDescription);
#ifdef LICH_KING
        ObjectMgr::GetLocaleString(localeData->CompletedText, loc, locQuestCompletedText);
#endif

        for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
            ObjectMgr::GetLocaleString(localeData->ObjectiveText[i], loc, locQuestObjectiveText[i]);
    }

    response.Info.QuestID = GetQuestId();
    response.Info.QuestMethod = GetQuestMethod();
    response.Info.QuestLevel = GetQuestLevel();
#ifdef LICH_KING
    response.Info.QuestMinLevel = GetMinLevel();
#endif
    response.Info.QuestSortID = GetZoneOrSort();

    response.Info.QuestType = GetType();
    response.Info.SuggestedGroupNum = GetSuggestedPlayers();

    response.Info.RequiredFactionId[0] = GetRepObjectiveFaction();
    response.Info.RequiredFactionValue[0] = GetRepObjectiveValue();

#ifdef LICH_KING
    response.Info.RequiredFactionId[1] = GetRepObjectiveFaction2();
    response.Info.RequiredFactionValue[1] = GetRepObjectiveValue2();
#else
    response.Info.RequiredFactionId[1] = 0;  //always 0 on BC
    response.Info.RequiredFactionValue[1] = 0;  //always 0 on BC
#endif

    response.Info.RewardNextQuest = GetNextQuestInChain();
#ifdef LICH_KING
    response.Info.RewardXPDifficulty = GetXPId();
#endif

    response.Info.RewardMoney = GetRewOrReqMoney();
    response.Info.RewardBonusMoney = GetRewMoneyMaxLevel();
    response.Info.RewardDisplaySpell = GetRewSpell();
    response.Info.RewardSpell = GetRewSpellCast();

#ifdef LICH_KING
    response.Info.RewardHonor = GetRewHonorAddition();
    response.Info.RewardKillHonor = GetRewHonorMultiplier();
#else
    response.Info.RewardHonorForPlayer = GetRewHonorableKills();
#endif

    response.Info.StartItem = GetSrcItemId();
    response.Info.Flags = GetFlags();
    response.Info.RewardTitleId = GetCharTitleId();
#ifdef LICH_KING
    response.Info.RequiredPlayerKills = GetPlayersSlain();
    response.Info.RewardTalents = GetBonusTalents();
    response.Info.RewardArenaPoints = GetRewArenaPoints();
#endif

    for (uint8 i = 0; i < QUEST_REWARDS_COUNT; ++i)
    {
        response.Info.RewardItems[i] = RewardItemId[i];
        response.Info.RewardAmount[i] = RewardItemIdCount[i];
    }

    for (uint8 i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
    {
        response.Info.UnfilteredChoiceItems[i].ItemID = RewardChoiceItemId[i];
        response.Info.UnfilteredChoiceItems[i].Quantity = RewardChoiceItemCount[i];
    }

#ifdef LICH_KING
    for (uint8 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)             // reward factions ids
        response.Info.RewardFactionID[i] = RewardFactionId[i];

    for (uint8 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)             // columnid+1 QuestFactionReward.dbc?
        response.Info.RewardFactionValue[i] = RewardFactionValueId[i];

    for (uint8 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)             // unk (0)
        response.Info.RewardFactionValueOverride[i] = RewardFactionValueIdOverride[i];
#endif

    response.Info.POIContinent = GetPOIContinent();
    response.Info.POIx = GetPOIx();
    response.Info.POIy = GetPOIy();
    response.Info.POIPriority = GetPointOpt();

    response.Info.Title = locQuestTitle;
    response.Info.Objectives = locQuestObjectives;
    response.Info.Details = locQuestDetails;
    response.Info.AreaDescription = locQuestAreaDescription;
#ifdef LICH_KING
    response.Info.CompletedText = locQuestCompletedText;
#endif

    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        response.Info.RequiredNpcOrGo[i] = RequiredNpcOrGo[i];
        response.Info.RequiredNpcOrGoCount[i] = RequiredNpcOrGoCount[i];
#ifdef LICH_KING
        response.Info.ItemDrop[i] = ItemDrop[i];
#endif
    }

    for (uint8 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
    {
        response.Info.RequiredItemId[i] = RequiredItemId[i];
        response.Info.RequiredItemCount[i] = RequiredItemCount[i];
    }

    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        response.Info.ObjectiveText[i] = locQuestObjectiveText[i];

    response.Write();
    response.ShrinkToFit();
    return response.Move();
}
