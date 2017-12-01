
#ifndef DBCSTORES_H
#define DBCSTORES_H

#include "DBCStore.h"
#include "DBCStructure.h"
#include "Item.h"
#include "Database/DatabaseEnv.h"

typedef std::list<uint32> SimpleFactionsList;

TC_GAME_API SimpleFactionsList const* GetFactionTeamList(uint32 faction);
TC_GAME_API std::string GetPetName(uint32 petfamily, LocaleConstant dbclang);
TC_GAME_API uint32 GetTalentSpellCost(uint32 spellId);
TC_GAME_API TalentSpellPos const* GetTalentSpellPos(uint32 spellId);

TC_GAME_API WMOAreaTableEntry const* GetWMOAreaTableEntryByTripple(int32 rootid, int32 adtid, int32 groupid);

TC_GAME_API uint32 GetVirtualMapForMapAndZone(uint32 mapid, uint32 zoneId);

TC_GAME_API ChatChannelsEntry const* GetChannelEntryFor(uint32 channel_id);

TC_GAME_API bool IsTotemCategoryCompatibleWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId, Item* pItem);

TC_GAME_API void Zone2MapCoordinates(float& x,float& y,uint32 zone);
TC_GAME_API void Map2ZoneCoordinates(float& x,float& y,uint32 zone);

typedef std::map<uint32/*pair32(map, diff)*/, MapDifficulty> MapDifficultyMap;
TC_GAME_API MapDifficulty const* GetMapDifficultyData(uint32 mapId, Difficulty difficulty);
TC_GAME_API MapDifficulty const* GetDownscaledMapDifficultyData(uint32 mapId, Difficulty &difficulty);

typedef std::unordered_multimap<uint32, SkillRaceClassInfoEntry const*> SkillRaceClassInfoMap;
typedef std::pair<SkillRaceClassInfoMap::iterator, SkillRaceClassInfoMap::iterator> SkillRaceClassInfoBounds;
TC_GAME_API SkillRaceClassInfoEntry const* GetSkillRaceClassInfo(uint32 skill, uint8 race, uint8 class_);

TC_GAME_API uint32 GetLiquidFlagsFromType(uint32 type); //convert LiquidTypeEntry.Type to MAP_LIQUID_*
TC_GAME_API uint32 GetLiquidFlags(uint32 liquidTypeRec);

//Those dbc exists only on LK... fake them for BC
TC_GAME_API PvPDifficultyEntry const* GetBattlegroundBracketByLevel(uint32 mapid, uint32 level);
TC_GAME_API PvPDifficultyEntry const* GetBattlegroundBracketById(uint32 mapid, BattlegroundBracketId id);

TC_GAME_API uint32 GetTalentInspectBitPosInTab(uint32 talentId);
TC_GAME_API uint32 GetTalentTabInspectBitSize(uint32 talentTabId);
TC_GAME_API uint32 const* /*[3]*/ GetTalentTabPages(uint32 cls);

TC_GAME_API uint32 GetDefaultMapLight(uint32 mapId);

enum ContentLevels
{
    CONTENT_1_60 = 0,
    CONTENT_61_70,
    CONTENT_71_80
};
ContentLevels TC_GAME_API GetContentLevelsForMapAndZone(uint32 mapid, uint32 zoneId);

TC_GAME_API extern DBCStorage <AreaTableEntry>               sAreaTableStore;
TC_GAME_API extern DBCStorage <AreaTriggerEntry>             sAreaTriggerStore;
TC_GAME_API extern DBCStorage <AuctionHouseEntry>            sAuctionHouseStore;
TC_GAME_API extern DBCStorage <BankBagSlotPricesEntry>       sBankBagSlotPricesStore;
TC_GAME_API extern DBCStorage <BattlemasterListEntry>        sBattlemasterListStore;
TC_GAME_API extern DBCStorage <CinematicCameraEntry>         sCinematicCameraStore;
TC_GAME_API extern DBCStorage <CinematicSequencesEntry>      sCinematicSequencesStore;
//TC_GAME_API extern DBCStorage <ChatChannelsEntry>           sChatChannelsStore; -- accessed using function, no usable index
TC_GAME_API extern DBCStorage <CharStartOutfitEntry>         sCharStartOutfitStore;
TC_GAME_API extern DBCStorage <CharTitlesEntry>              sCharTitlesStore;
TC_GAME_API extern DBCStorage <ChrClassesEntry>              sChrClassesStore;
TC_GAME_API extern DBCStorage <ChrRacesEntry>                sChrRacesStore;
TC_GAME_API extern DBCStorage <CreatureDisplayInfoEntry>     sCreatureDisplayInfoStore;
TC_GAME_API extern DBCStorage <CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore;
TC_GAME_API extern DBCStorage <CreatureFamilyEntry>          sCreatureFamilyStore;
TC_GAME_API extern DBCStorage <CreatureModelDataEntry>       sCreatureModelDataStore;
TC_GAME_API extern DBCStorage <CreatureSpellDataEntry>       sCreatureSpellDataStore;
TC_GAME_API extern DBCStorage <DurabilityCostsEntry>         sDurabilityCostsStore;
TC_GAME_API extern DBCStorage <DurabilityQualityEntry>       sDurabilityQualityStore;
TC_GAME_API extern DBCStorage <EmotesEntry>                  sEmotesStore;
TC_GAME_API extern DBCStorage <EmotesTextEntry>              sEmotesTextStore;
TC_GAME_API extern DBCStorage <FactionEntry>                 sFactionStore;
TC_GAME_API extern DBCStorage <FactionTemplateEntry>         sFactionTemplateStore;
TC_GAME_API extern DBCStorage <GameObjectDisplayInfoEntry>   sGameObjectDisplayInfoStore;
TC_GAME_API extern DBCStorage <GemPropertiesEntry>           sGemPropertiesStore;

TC_GAME_API extern DBCStorage <GtCombatRatingsEntry>         sGtCombatRatingsStore;
TC_GAME_API extern DBCStorage <GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore;
TC_GAME_API extern DBCStorage <GtChanceToMeleeCritEntry>     sGtChanceToMeleeCritStore;
TC_GAME_API extern DBCStorage <GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore;
TC_GAME_API extern DBCStorage <GtChanceToSpellCritEntry>     sGtChanceToSpellCritStore;
TC_GAME_API extern DBCStorage <GtNPCManaCostScalerEntry>     sGtNPCManaCostScalerStore;
TC_GAME_API extern DBCStorage <GtOCTRegenHPEntry>            sGtOCTRegenHPStore;
//TC_GAME_API extern DBCStorage <GtOCTRegenMPEntry>            sGtOCTRegenMPStore; -- not used currently
TC_GAME_API extern DBCStorage <GtRegenHPPerSptEntry>         sGtRegenHPPerSptStore;
TC_GAME_API extern DBCStorage <GtRegenMPPerSptEntry>         sGtRegenMPPerSptStore;
TC_GAME_API extern DBCStorage <ItemEntry>                    sItemStore;
//TC_GAME_API extern DBCStorage <ItemDisplayInfoEntry>      sItemDisplayInfoStore; -- not used currently
//moved to db TC_GAME_API extern DBCStorage <ItemExtendedCostEntry>        sItemExtendedCostStore;
TC_GAME_API extern DBCStorage <ItemRandomPropertiesEntry>    sItemRandomPropertiesStore;
TC_GAME_API extern DBCStorage <ItemRandomSuffixEntry>        sItemRandomSuffixStore;
TC_GAME_API extern DBCStorage <ItemSetEntry>                 sItemSetStore;
TC_GAME_API extern DBCStorage <LiquidTypeEntry>              sLiquidTypeStore;
TC_GAME_API extern DBCStorage <LockEntry>                    sLockStore;
TC_GAME_API extern DBCStorage <MailTemplateEntry>            sMailTemplateStore;
TC_GAME_API extern DBCStorage <MapEntry>                     sMapStore;
//TC_GAME_API extern DBCStorage <MapDifficultyEntry>           sMapDifficultyStore; -- use GetMapDifficultyData insteed
TC_GAME_API extern MapDifficultyMap                          sMapDifficultyMap;
TC_GAME_API extern DBCStorage <QuestSortEntry>               sQuestSortStore;
TC_GAME_API extern DBCStorage <RandomPropertiesPointsEntry>  sRandomPropertiesPointsStore;
TC_GAME_API extern DBCStorage <SkillLineEntry>               sSkillLineStore;
TC_GAME_API extern DBCStorage <SkillLineAbilityEntry>        sSkillLineAbilityStore;
TC_GAME_API extern DBCStorage <SkillTiersEntry>              sSkillTiersStore;
TC_GAME_API extern DBCStorage <SkillRaceClassInfoEntry>      sSkillRaceClassInfoStore;
TC_GAME_API extern DBCStorage <SoundEntriesEntry>            sSoundEntriesStore;
TC_GAME_API extern DBCStorage <SpellCastTimesEntry>          sSpellCastTimesStore;
TC_GAME_API extern DBCStorage <SpellDurationEntry>           sSpellDurationStore;
TC_GAME_API extern DBCStorage <SpellFocusObjectEntry>        sSpellFocusObjectStore;
TC_GAME_API extern DBCStorage <SpellItemEnchantmentEntry>    sSpellItemEnchantmentStore;
TC_GAME_API extern DBCStorage <SpellItemEnchantmentConditionEntry> sSpellItemEnchantmentConditionStore;
TC_GAME_API extern DBCStorage <SpellCategoryEntry>           sSpellCategoryStore;
TC_GAME_API extern PetFamilySpellsStore                      sPetFamilySpellsStore;
TC_GAME_API extern SpellCategoryStore                        sSpellsByCategoryStore;
TC_GAME_API extern DBCStorage <SpellRadiusEntry>             sSpellRadiusStore;
TC_GAME_API extern DBCStorage <SpellRangeEntry>              sSpellRangeStore;
TC_GAME_API extern DBCStorage <SpellShapeshiftEntry>         sSpellShapeshiftStore;
//TC_GAME_API extern DBCStorage <SpellEntry>                   sSpellStore;
TC_GAME_API extern DBCStorage <StableSlotPricesEntry>        sStableSlotPricesStore;
TC_GAME_API extern DBCStorage <SummonPropertiesEntry>        sSummonPropertiesStore;
TC_GAME_API extern DBCStorage <TalentEntry>                  sTalentStore;
TC_GAME_API extern DBCStorage <TalentTabEntry>               sTalentTabStore;
TC_GAME_API extern DBCStorage <TaxiNodesEntry>               sTaxiNodesStore;
TC_GAME_API extern DBCStorage <TaxiPathEntry>                sTaxiPathStore;
TC_GAME_API extern TaxiMask                                  sTaxiNodesMask;
TC_GAME_API extern TaxiPathSetBySource                       sTaxiPathSetBySource;
TC_GAME_API extern TaxiPathNodesByPath                       sTaxiPathNodesByPath;
TC_GAME_API extern DBCStorage <TotemCategoryEntry>           sTotemCategoryStore;
TC_GAME_API extern DBCStorage <WMOAreaTableEntry>            sWMOAreaTableStore;
//TC_GAME_API extern DBCStorage <WorldMapAreaEntry>           sWorldMapAreaStore; -- use Zone2MapCoordinates and Map2ZoneCoordinates
TC_GAME_API extern DBCStorage <WorldSafeLocsEntry>           sWorldSafeLocsStore;

void LoadDBCStores(const std::string& dataPath);

// script support functions
TC_GAME_API DBCStorage <SoundEntriesEntry>  const* GetSoundEntriesStore();
//TC_GAME_API DBCStorage <SpellInfo>         const* GetSpellStore();
TC_GAME_API DBCStorage <SpellRangeEntry>    const* GetSpellRangeStore();

#endif
