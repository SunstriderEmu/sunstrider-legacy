#include "DBCStores.h"
#include "Log.h"
#include "TransportMgr.h"
#include "Item.h"
#include "DBCfmt.h"
#include "DBCFileLoader.h"
#include "IteratorPair.h"

#include <map>
#include <fstream>
#include <iostream>
#include <iomanip>

typedef std::map<uint16,uint32> AreaFlagByAreaID;
typedef std::map<uint32,uint32> AreaFlagByMapID;

struct WMOAreaTableTripple
{
    WMOAreaTableTripple(int32 r, int32 a, int32 g) : rootId(r), adtId(a), groupId(g)
    {
    }

    bool operator <(const WMOAreaTableTripple& b) const
    {
        return memcmp(this, &b, sizeof(WMOAreaTableTripple))<0;
    }

    // ordered by entropy; that way memcmp will have a minimal medium runtime
    int32 groupId;
    int32 rootId;
    int32 adtId;
};

typedef std::map<WMOAreaTableTripple, WMOAreaTableEntry const *> WMOAreaInfoByTripple;

//zones are actually area so you can also search those with this
DBCStorage <AreaTableEntry> sAreaTableStore(AreaTableEntryfmt);

static WMOAreaInfoByTripple sWMOAreaInfoByTripple;

DBCStorage <AreaTriggerEntry> sAreaTriggerStore(AreaTriggerEntryfmt);
DBCStorage <AuctionHouseEntry> sAuctionHouseStore(AuctionHouseEntryfmt);
DBCStorage <BankBagSlotPricesEntry> sBankBagSlotPricesStore(BankBagSlotPricesEntryfmt);
DBCStorage <BattlemasterListEntry> sBattlemasterListStore(BattlemasterListEntryfmt);
DBCStorage <CharacterFacialHairStylesEntry> sCharacterFacialHairStylesStore(CharacterFacialHairStylesfmt);
std::unordered_map<uint32, CharacterFacialHairStylesEntry const*> sCharFacialHairMap;
DBCStorage <CharSectionsEntry> sCharSectionsStore(CharSectionsEntryfmt);
std::unordered_multimap<uint32, CharSectionsEntry const*> sCharSectionMap;
DBCStorage <CharStartOutfitEntry> sCharStartOutfitStore(CharStartOutfitEntryfmt);
std::map<uint32, CharStartOutfitEntry const*> sCharStartOutfitMap;
DBCStorage <CharTitlesEntry> sCharTitlesStore(CharTitlesEntryfmt);
DBCStorage <ChatChannelsEntry> sChatChannelsStore(ChatChannelsEntryfmt);
DBCStorage <ChrClassesEntry> sChrClassesStore(ChrClassesEntryfmt);
DBCStorage <ChrRacesEntry> sChrRacesStore(ChrRacesEntryfmt);
DBCStorage <CinematicCameraEntry> sCinematicCameraStore(CinematicCameraEntryfmt);
DBCStorage <CinematicSequencesEntry> sCinematicSequencesStore(CinematicSequencesEntryfmt);
DBCStorage <CreatureDisplayInfoEntry> sCreatureDisplayInfoStore(CreatureDisplayInfofmt);
DBCStorage <CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore(CreatureDisplayInfoExtrafmt);
DBCStorage <CreatureFamilyEntry> sCreatureFamilyStore(CreatureFamilyfmt);
DBCStorage <CreatureModelDataEntry> sCreatureModelDataStore(CreatureModelDatafmt);
DBCStorage <CreatureSpellDataEntry> sCreatureSpellDataStore(CreatureSpellDatafmt);

DBCStorage <DurabilityQualityEntry> sDurabilityQualityStore(DurabilityQualityfmt);
DBCStorage <DurabilityCostsEntry> sDurabilityCostsStore(DurabilityCostsfmt);

DBCStorage <EmotesEntry> sEmotesStore(Emotesfmt);
DBCStorage <EmotesTextEntry> sEmotesTextStore(EmoteEntryfmt);

typedef std::map<uint32,SimpleFactionsList> FactionTeamMap;
static FactionTeamMap sFactionTeamMap;
DBCStorage <FactionEntry> sFactionStore(FactionEntryfmt);
DBCStorage <FactionTemplateEntry> sFactionTemplateStore(FactionTemplateEntryfmt);

DBCStorage <GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore(GameObjectDisplayInfofmt);
DBCStorage <GemPropertiesEntry> sGemPropertiesStore(GemPropertiesEntryfmt);

DBCStorage <GtCombatRatingsEntry>         sGtCombatRatingsStore(GtCombatRatingsfmt);
DBCStorage <GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore(GtChanceToMeleeCritBasefmt);
DBCStorage <GtChanceToMeleeCritEntry>     sGtChanceToMeleeCritStore(GtChanceToMeleeCritfmt);
DBCStorage <GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore(GtChanceToSpellCritBasefmt);
DBCStorage <GtChanceToSpellCritEntry>     sGtChanceToSpellCritStore(GtChanceToSpellCritfmt);
DBCStorage <GtNPCManaCostScalerEntry>     sGtNPCManaCostScalerStore(GtNPCManaCostScalerfmt);
DBCStorage <GtOCTRegenHPEntry>            sGtOCTRegenHPStore(GtOCTRegenHPfmt);
//DBCStorage <GtOCTRegenMPEntry>            sGtOCTRegenMPStore(GtOCTRegenMPfmt);  -- not used currently
DBCStorage <GtRegenHPPerSptEntry>         sGtRegenHPPerSptStore(GtRegenHPPerSptfmt);
DBCStorage <GtRegenMPPerSptEntry>         sGtRegenMPPerSptStore(GtRegenMPPerSptfmt);
DBCStorage <ItemEntry>                    sItemStore(Itemfmt);
//DBCStorage <ItemCondExtCostsEntry> sItemCondExtCostsStore(ItemCondExtCostsEntryfmt);
//DBCStorage <ItemDisplayInfoEntry> sItemDisplayInfoStore(ItemDisplayTemplateEntryfmt); -- not used currently
//moved to db DBCStorage <ItemExtendedCostEntry> sItemExtendedCostStore(ItemExtendedCostEntryfmt);
DBCStorage <ItemRandomPropertiesEntry> sItemRandomPropertiesStore(ItemRandomPropertiesfmt);
DBCStorage <ItemRandomSuffixEntry> sItemRandomSuffixStore(ItemRandomSuffixfmt);
DBCStorage <ItemSetEntry> sItemSetStore(ItemSetEntryfmt);

DBCStorage <LightEntry> sLightStore(LightEntryfmt);
DBCStorage <LockEntry> sLockStore(LockEntryfmt);
DBCStorage <LiquidTypeEntry> sLiquidTypeStore(LiquidTypefmt);

DBCStorage <MailTemplateEntry> sMailTemplateStore(MailTemplateEntryfmt);
DBCStorage <MapEntry> sMapStore(MapEntryfmt);

// DBC used only for initialization sMapDifficultyMap at startup.
#ifdef LICH_KING //this one does not exist on BC
DBCStorage <MapDifficultyEntry> sMapDifficultyStore(MapDifficultyEntryfmt); // only for loading
DBCStorage <PvPDifficultyEntry> sPvPDifficultyStore(PvPDifficultyfmt);
#endif
MapDifficultyMap sMapDifficultyMap;


DBCStorage <QuestSortEntry> sQuestSortStore(QuestSortEntryfmt);

DBCStorage <RandomPropertiesPointsEntry> sRandomPropertiesPointsStore(RandomPropertiesPointsfmt);

DBCStorage <SkillLineEntry> sSkillLineStore(SkillLinefmt);
DBCStorage <SkillLineAbilityEntry> sSkillLineAbilityStore(SkillLineAbilityfmt);
DBCStorage <SkillRaceClassInfoEntry> sSkillRaceClassInfoStore(SkillRaceClassInfofmt);
SkillRaceClassInfoMap SkillRaceClassInfoBySkill;
DBCStorage <SkillTiersEntry> sSkillTiersStore(SkillTiersfmt);

DBCStorage <SoundEntriesEntry> sSoundEntriesStore(SoundEntriesfmt);

DBCStorage <SpellItemEnchantmentEntry> sSpellItemEnchantmentStore(SpellItemEnchantmentfmt);
DBCStorage <SpellItemEnchantmentConditionEntry> sSpellItemEnchantmentConditionStore(SpellItemEnchantmentConditionfmt);
DBCStorage <SpellEntry> sSpellStore(SpellEntryfmt);
PetFamilySpellsStore sPetFamilySpellsStore;
SpellCategoryStore sSpellsByCategoryStore;

DBCStorage <SpellCastTimesEntry> sSpellCastTimesStore(SpellCastTimefmt);
DBCStorage <SpellCategoryEntry> sSpellCategoryStore(SpellCategoryfmt);
DBCStorage <SpellDurationEntry> sSpellDurationStore(SpellDurationfmt);
DBCStorage <SpellFocusObjectEntry> sSpellFocusObjectStore(SpellFocusObjectfmt);
DBCStorage <SpellRadiusEntry> sSpellRadiusStore(SpellRadiusfmt);
DBCStorage <SpellRangeEntry> sSpellRangeStore(SpellRangefmt);
DBCStorage <SpellShapeshiftEntry> sSpellShapeshiftStore(SpellShapeshiftfmt);
DBCStorage <StableSlotPricesEntry> sStableSlotPricesStore(StableSlotPricesfmt);
DBCStorage <SummonPropertiesEntry> sSummonPropertiesStore(SummonPropertiesfmt);
DBCStorage <TalentEntry> sTalentStore(TalentEntryfmt);
TalentSpellPosMap sTalentSpellPosMap;
DBCStorage <TalentTabEntry> sTalentTabStore(TalentTabEntryfmt);

// store absolute bit position for first rank for talent inspect
typedef std::map<uint32,uint32> TalentInspectMap;
static TalentInspectMap sTalentPosInInspect;
static TalentInspectMap sTalentTabSizeInInspect;
static uint32 sTalentTabPages[12/*MAX_CLASSES*/][3];

DBCStorage <TaxiNodesEntry> sTaxiNodesStore(TaxiNodesEntryfmt);
TaxiMask sTaxiNodesMask;

// DBC used only for initialization sTaxiPathSetBySource at startup.
TaxiPathSetBySource sTaxiPathSetBySource;
DBCStorage <TaxiPathEntry> sTaxiPathStore(TaxiPathEntryfmt);

// DBC used only for initialization sTaxiPathSetBySource at startup.
TaxiPathNodesByPath sTaxiPathNodesByPath;

static DBCStorage <TaxiPathNodeEntry> sTaxiPathNodeStore(TaxiPathNodeEntryfmt);
DBCStorage <TotemCategoryEntry> sTotemCategoryStore(TotemCategoryEntryfmt);
DBCStorage <TransportAnimationEntry> sTransportAnimationStore(TransportAnimationfmt);
DBCStorage <WMOAreaTableEntry> sWMOAreaTableStore(WMOAreaTableEntryfmt);
DBCStorage <WorldMapAreaEntry>  sWorldMapAreaStore(WorldMapAreaEntryfmt); //the index is set on area_id instead of the first column dbc id
DBCStorage <WorldSafeLocsEntry> sWorldSafeLocsStore(WorldSafeLocsEntryfmt);

typedef std::list<std::string> StoreProblemList;

uint32 DBCFileCount = 0;

static bool LoadDBC_assert_print(uint32 fsize,uint32 rsize, const std::string& filename)
{
    TC_LOG_ERROR("FIXME","ERROR: Size of '%s' setted by format string (%u) not equal size of C++ structure (%u).",filename.c_str(),fsize,rsize);

    // assert must fail after function call
    return false;
}


template<class T>
inline void LoadDBC(uint32& availableDbcLocales, StoreProblemList& errors, DBCStorage<T>& storage, std::string const& dbcPath, std::string const& filename, std::string const& customFormat = std::string(), std::string const& customIndexName = std::string())
{
    // compatibility format and C++ structure sizes
    ASSERT(DBCFileLoader::GetFormatRecordSize(storage.GetFormat()) == sizeof(T) || LoadDBC_assert_print(DBCFileLoader::GetFormatRecordSize(storage.GetFormat()), sizeof(T), filename));

    ++DBCFileCount;
    std::string dbcFilename = dbcPath + filename;

    if (storage.Load(dbcFilename))
    {
        for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
        {
            if (!(availableDbcLocales & (1 << i)))
                continue;

            std::string localizedName(dbcPath);
            localizedName.append(localeNames[i]);
            localizedName.push_back('/');
            localizedName.append(filename);

            if (!storage.LoadStringsFrom(localizedName))
                availableDbcLocales &= ~(1 << i);             // mark as not available for speedup next checks
        }

        if (!customFormat.empty())
            storage.LoadFromDB(filename, customFormat, customIndexName);
    }
    else
    {
        // sort problematic dbc to (1) non compatible and (2) non-existed
        if (FILE* f = fopen(dbcFilename.c_str(), "rb"))
        {
            std::ostringstream stream;
            stream << dbcFilename << " exists, and has " << storage.GetFieldCount() << " field(s) (expected " << strlen(storage.GetFormat()) << "). Extracted file might be from wrong client version or a database-update has been forgotten. Search on forum for TCE00008 for more info.";
            std::string buf = stream.str();
            errors.push_back(buf);
            fclose(f);
        }
        else
            errors.push_back(dbcFilename);
    }
}

void LoadDBCStores(const std::string& dataPath)
{
    std::string dbcPath = dataPath+"dbc/";

    const uint32 DBCFilesCount = 58;

    StoreProblemList bad_dbc_files;
    uint32 availableDbcLocales = 0xFFFFFFFF;

#define LOAD_DBC(store, file) LoadDBC(availableDbcLocales, bad_dbc_files, store, dbcPath, file)

    LOAD_DBC(sAreaTableStore, "AreaTable.dbc");
    LOAD_DBC(sAreaTriggerStore, "AreaTrigger.dbc");
    LOAD_DBC(sAuctionHouseStore, "AuctionHouse.dbc");
    LOAD_DBC(sBankBagSlotPricesStore, "BankBagSlotPrices.dbc");
    LOAD_DBC(sBattlemasterListStore, "BattlemasterList.dbc");
    LOAD_DBC(sCharacterFacialHairStylesStore, "CharacterFacialHairStyles.dbc");
    LOAD_DBC(sCharSectionsStore, "CharSections.dbc");
    LOAD_DBC(sCharStartOutfitStore, "CharStartOutfit.dbc");

    LOAD_DBC(sCharTitlesStore, "CharTitles.dbc");
    LOAD_DBC(sChatChannelsStore, "ChatChannels.dbc");
    LOAD_DBC(sChrClassesStore, "ChrClasses.dbc");
    LOAD_DBC(sChrRacesStore, "ChrRaces.dbc");
    LOAD_DBC(sCinematicCameraStore, "CinematicCamera.dbc");
    LOAD_DBC(sCinematicSequencesStore, "CinematicSequences.dbc");
    LOAD_DBC(sCreatureDisplayInfoStore, "CreatureDisplayInfo.dbc");
    LOAD_DBC(sCreatureFamilyStore, "CreatureFamily.dbc");
    LOAD_DBC(sCreatureModelDataStore, "CreatureModelData.dbc");
    LOAD_DBC(sCreatureSpellDataStore, "CreatureSpellData.dbc");
    LOAD_DBC(sDurabilityCostsStore, "DurabilityCosts.dbc");
    LOAD_DBC(sDurabilityQualityStore, "DurabilityQuality.dbc");
    
    LOAD_DBC(sEmotesStore, "Emotes.dbc");
    LOAD_DBC(sEmotesTextStore, "EmotesText.dbc");
    LOAD_DBC(sFactionStore, "Faction.dbc");
    for (uint32 i=0;i<sFactionStore.GetNumRows(); ++i)
    {
        FactionEntry const * faction = sFactionStore.LookupEntry(i);
        if (faction && faction->team)
        {
            SimpleFactionsList &flist = sFactionTeamMap[faction->team];
            flist.push_back(i);
        }
    }

    LOAD_DBC(sFactionTemplateStore, "FactionTemplate.dbc");
    LoadDBC(availableDbcLocales, bad_dbc_files, sGameObjectDisplayInfoStore,  dbcPath, "GameObjectDisplayInfo.dbc");
    for (uint32 i = 0; i < sGameObjectDisplayInfoStore.GetNumRows(); ++i)
    {
        if (GameObjectDisplayInfoEntry const* info = sGameObjectDisplayInfoStore.LookupEntry(i))
        {
            if (info->maxX < info->minX)
                std::swap(*(float*)(&info->maxX), *(float*)(&info->minX));
            if (info->maxY < info->minY)
                std::swap(*(float*)(&info->maxY), *(float*)(&info->minY));
            if (info->maxZ < info->minZ)
                std::swap(*(float*)(&info->maxZ), *(float*)(&info->minZ));
        }
    }
    LOAD_DBC(sGemPropertiesStore, "GemProperties.dbc");

    LOAD_DBC(sGtCombatRatingsStore, "gtCombatRatings.dbc");

    LOAD_DBC(sGtChanceToMeleeCritBaseStore, "gtChanceToMeleeCritBase.dbc");
    LOAD_DBC(sGtChanceToMeleeCritStore, "gtChanceToMeleeCrit.dbc");

    LOAD_DBC(sGtChanceToSpellCritBaseStore, "gtChanceToSpellCritBase.dbc");
    LOAD_DBC(sGtChanceToSpellCritStore, "gtChanceToSpellCrit.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sGtNPCManaCostScalerStore, dbcPath, "gtNPCManaCostScaler.dbc");

    LOAD_DBC(sGtOCTRegenHPStore, "gtOCTRegenHP.dbc");
    //LOAD_DBC(sGtOCTRegenMPStore, "gtOCTRegenMP.dbc");       -- not used currently
    LOAD_DBC(sGtRegenHPPerSptStore, "gtRegenHPPerSpt.dbc");
    LOAD_DBC(sGtRegenMPPerSptStore, "gtRegenMPPerSpt.dbc");
    LOAD_DBC(sItemStore, "Item.dbc");
    //LOAD_DBC(sItemDisplayInfoStore, "ItemDisplayInfo.dbc");     -- not used currently
    //LOAD_DBC(sItemCondExtCostsStore, "ItemCondExtCosts.dbc");
    //moved to db LOAD_DBC(sItemExtendedCostStore, "ItemExtendedCost.dbc");
    LOAD_DBC(sItemRandomPropertiesStore, "ItemRandomProperties.dbc");
    LOAD_DBC(sItemRandomSuffixStore, "ItemRandomSuffix.dbc");
    LOAD_DBC(sItemSetStore, "ItemSet.dbc");
    LOAD_DBC(sLightStore, "Light.dbc");
    LOAD_DBC(sLiquidTypeStore, "LiquidType.dbc");
    LOAD_DBC(sLockStore, "Lock.dbc");
    LOAD_DBC(sMailTemplateStore, "MailTemplate.dbc");
    LOAD_DBC(sMapStore, "Map.dbc");
#ifdef LICH_KING
    LOAD_DBC(sMapDifficultyStore,"MapDifficulty.dbc");
    // fill data
    for (uint32 i = 1; i < sMapDifficultyStore.GetNumRows(); ++i)
        if (MapDifficultyEntry const* entry = sMapDifficultyStore.LookupEntry(i))
            sMapDifficultyMap[MAKE_PAIR32(entry->MapId, entry->Difficulty)] = MapDifficulty(entry->resetTime, entry->maxPlayers, entry->areaTriggerText[0] != '\0');
    sMapDifficultyStore.Clear();

    LOAD_DBC(sPvPDifficultyStore, "PvpDifficulty.dbc");
#else
    //fake MapDifficulty.dbc for BC 
    //also partially handled in ObjectMgr::LoadInstanceTemplate() for instances
    std::map<uint32, uint32> spawnMasks;

    for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
        if (MapEntry const* mapEntry = sMapStore.LookupEntry(i))
            if (!mapEntry->Instanceable())
                sMapDifficultyMap[MAKE_PAIR32(i, REGULAR_DIFFICULTY)] = MapDifficulty(0, 0, false);
#endif
    LOAD_DBC(sQuestSortStore, "QuestSort.dbc");
    LOAD_DBC(sRandomPropertiesPointsStore, "RandPropPoints.dbc");
    LOAD_DBC(sSkillLineStore, "SkillLine.dbc");
    LOAD_DBC(sSkillLineAbilityStore, "SkillLineAbility.dbc");
    LOAD_DBC(sSkillRaceClassInfoStore, "SkillRaceClassInfo.dbc");
    for (uint32 i = 0; i < sSkillRaceClassInfoStore.GetNumRows(); ++i)
        if (SkillRaceClassInfoEntry const* entry = sSkillRaceClassInfoStore.LookupEntry(i))
            if (sSkillLineStore.LookupEntry(entry->SkillId))
                SkillRaceClassInfoBySkill.emplace(entry->SkillId, entry);

    LOAD_DBC(sSkillTiersStore, "SkillTiers.dbc");
    LOAD_DBC(sSoundEntriesStore, "SoundEntries.dbc");
    //"Spell.dbc" is now world.spell_template table

    LOAD_DBC(sSpellCastTimesStore, "SpellCastTimes.dbc");
    LOAD_DBC(sSpellCategoryStore, "SpellCategory.dbc");
    LOAD_DBC(sSpellDurationStore, "SpellDuration.dbc");
    LOAD_DBC(sSpellFocusObjectStore, "SpellFocusObject.dbc");
    LOAD_DBC(sSpellItemEnchantmentStore, "SpellItemEnchantment.dbc");
    LOAD_DBC(sSpellItemEnchantmentConditionStore, "SpellItemEnchantmentCondition.dbc");
    LOAD_DBC(sSpellRadiusStore, "SpellRadius.dbc");
    LOAD_DBC(sSpellRangeStore, "SpellRange.dbc");
    LOAD_DBC(sSpellShapeshiftStore, "SpellShapeshiftForm.dbc");
    LOAD_DBC(sStableSlotPricesStore, "StableSlotPrices.dbc");
    LOAD_DBC(sSummonPropertiesStore, "SummonProperties.dbc");

    LOAD_DBC(sTalentStore, "Talent.dbc");

    // create talent spells set
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo) continue;
        for (int j = 0; j < 5; j++)
            if(talentInfo->RankID[j])
                sTalentSpellPosMap[talentInfo->RankID[j]] = TalentSpellPos(i,j);
    }

    LOAD_DBC(sTalentTabStore, "TalentTab.dbc");

    // prepare fast data access to bit pos of talent ranks for use at inspecting
    {
        // fill table by amount of talent ranks and fill sTalentTabBitSizeInInspect
        // store in with (row,col,talent)->size key for correct sorting by (row,col)
        typedef std::map<uint32,uint32> TalentBitSize;
        TalentBitSize sTalentBitSize;
        for(uint32 i = 1; i < sTalentStore.GetNumRows(); ++i)
        {
            TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
            if (!talentInfo) continue;

            TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentInfo->TalentTab );
            if(!talentTabInfo)
                continue;

            // find talent rank
            uint32 curtalent_maxrank = 0;
            for(uint32 k = 5; k > 0; --k)
            {
                if(talentInfo->RankID[k-1])
                {
                    curtalent_maxrank = k;
                    break;
                }
            }

            sTalentBitSize[(talentInfo->Row<<24) + (talentInfo->Col<<16)+talentInfo->TalentID] = curtalent_maxrank;
            sTalentTabSizeInInspect[talentInfo->TalentTab] += curtalent_maxrank;
        }

        // now have all max ranks (and then bit amount used for store talent ranks in inspect)
        for(uint32 talentTabId = 1; talentTabId < sTalentTabStore.GetNumRows(); ++talentTabId)
        {
            TalentTabEntry const *talentTabInfo = sTalentTabStore.LookupEntry( talentTabId );
            if(!talentTabInfo)
                continue;

            // store class talent tab pages
            uint32 cls = 1;
            for(uint32 m=1;!(m & talentTabInfo->ClassMask) && cls < 12 /*MAX_CLASSES*/;m <<=1, ++cls) {}

            sTalentTabPages[cls][talentTabInfo->tabpage]=talentTabId;

            // add total amount bits for first rank starting from talent tab first talent rank pos.
            uint32 pos = 0;
            for(TalentBitSize::iterator itr = sTalentBitSize.begin(); itr != sTalentBitSize.end(); ++itr)
            {
                uint32 talentId = itr->first & 0xFFFF;
                TalentEntry const *talentInfo = sTalentStore.LookupEntry( talentId );
                if(!talentInfo)
                    continue;

                if(talentInfo->TalentTab != talentTabId)
                    continue;

                sTalentPosInInspect[talentId] = pos;
                pos+= itr->second;
            }
        }
    }

#ifdef LICH_KING
    for (PvPDifficultyEntry const* entry : sPvPDifficultyStore)
    {
        ASSERT(entry->bracketId < MAX_BATTLEGROUND_BRACKETS, "PvpDifficulty bracket (%d) exceeded max allowed value (%d)", entry->bracketId, MAX_BATTLEGROUND_BRACKETS);
    }
#endif

    LOAD_DBC(sTaxiNodesStore, "TaxiNodes.dbc");

    // Initialize global taxinodes mask
    sTaxiNodesMask.fill(0);
    for(uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
    {
        if(sTaxiNodesStore.LookupEntry(i))
        {
            uint8  field   = (uint8)((i - 1) / 32);
            uint32 submask = 1<<((i-1)%32);
            sTaxiNodesMask[field] |= submask;
        }
    }

    LOAD_DBC(sTaxiPathStore, "TaxiPath.dbc");
    for(uint32 i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        if(TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->ID,entry->price);
    uint32 pathCount = sTaxiPathStore.GetNumRows();

    //## TaxiPathNode.dbc ## Loaded only for initialization different structures
    LOAD_DBC(sTaxiPathNodeStore, "TaxiPathNode.dbc");
    // Calculate path nodes count
    std::vector<uint32> pathLength;
    pathLength.resize(pathCount);                           // 0 and some other indexes not used
    for (uint32 i = 1; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        if (TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
        {
            if (pathLength[entry->PathID] < entry->NodeIndex + 1)
                pathLength[entry->PathID] = entry->NodeIndex + 1;
        }
    // Set path length
    sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
    for (uint32 i = 1; i < sTaxiPathNodesByPath.size(); ++i)
        sTaxiPathNodesByPath[i].resize(pathLength[i]);
    // fill data
    for (uint32 i = 1; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        if (TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
            sTaxiPathNodesByPath[entry->PathID][entry->NodeIndex] = entry;

    LOAD_DBC(sTotemCategoryStore, "TotemCategory.dbc");
    LOAD_DBC(sTransportAnimationStore, "TransportAnimation.dbc");
    for (uint32 i = 0; i < sTransportAnimationStore.GetNumRows(); ++i)
    {
        TransportAnimationEntry const* anim = sTransportAnimationStore.LookupEntry(i);
        if (!anim)
            continue;

        sTransportMgr->AddPathNodeToTransport(anim->TransportEntry, anim->TimeSeg, anim);
    }

    LOAD_DBC(sWMOAreaTableStore, "WMOAreaTable.dbc");
    for(uint32 i = 0; i < sWMOAreaTableStore.GetNumRows(); ++i)
    {
        if(WMOAreaTableEntry const* entry = sWMOAreaTableStore.LookupEntry(i))
        {
            sWMOAreaInfoByTripple.insert(WMOAreaInfoByTripple::value_type(WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry));
        }
    }
    LOAD_DBC(sWorldMapAreaStore, "WorldMapArea.dbc");
    LOAD_DBC(sWorldSafeLocsStore, "WorldSafeLocs.dbc");

    for (CharacterFacialHairStylesEntry const* entry : sCharacterFacialHairStylesStore)
        if (entry->Race && ((1 << (entry->Race - 1)) & RACEMASK_ALL_PLAYABLE) != 0) // ignore nonplayable races
            sCharFacialHairMap.insert({ entry->Race | (entry->Gender << 8) | (entry->Variation << 16), entry });

    for (CharSectionsEntry const* entry : sCharSectionsStore)
        if (entry->Race && ((1 << (entry->Race - 1)) & RACEMASK_ALL_PLAYABLE) != 0) // ignore nonplayable races
            sCharSectionMap.insert({ entry->GenType | (entry->Gender << 8) | (entry->Race << 16), entry });

    for (CharStartOutfitEntry const* outfit : sCharStartOutfitStore)
        sCharStartOutfitMap[outfit->Race | (outfit->Class << 8) | (outfit->Gender << 16)] = outfit;

    // error checks
    if(bad_dbc_files.size() >= DBCFilesCount )
    {
        TC_LOG_ERROR("server.loading","\nIncorrect DataDir value in Trinityd.conf or ALL required *.dbc files (%d) not found by path: %sdbc",DBCFilesCount,dataPath.c_str());
        exit(1);
    }
    else if(!bad_dbc_files.empty() )
    {
        std::string str;
        for(std::list<std::string>::iterator i = bad_dbc_files.begin(); i != bad_dbc_files.end(); ++i)
            str += *i + "\n";

        TC_LOG_ERROR("server.loading", "\nSome required *.dbc files (" UI64FMTD " from %u) not found or not compatible : \n%s",bad_dbc_files.size(),DBCFilesCount,str.c_str());
        exit(1);
    }

    // check at up-to-date DBC files (53085 is last added spell in 2.4.3)
    // check at up-to-date DBC files (17514 is last ID in SkillLineAbilities in 2.4.3)
    // check at up-to-date DBC files (598 is last map added in 2.4.3)
    // check at up-to-date DBC files (1127 is last gem property added in 2.4.3)
    // check at up-to-date DBC files (2425 is last item extended cost added in 2.4.3)
    // check at up-to-date DBC files (71 is last char title added in 2.4.3)
    // check at up-to-date DBC files (1768 is last area added in 2.4.3)
    if( /*!sSpellStore.LookupEntry(53085)            ||*/
        !sSkillLineAbilityStore.LookupEntry(17514) ||
        !sMapStore.LookupEntry(598)                ||
        !sGemPropertiesStore.LookupEntry(1127)     ||
        !sCharTitlesStore.LookupEntry(71)          ||
        !sAreaTableStore.LookupEntry(1768)              )
    {
        TC_LOG_ERROR("server.loading","\nYou have _outdated_ DBC files. Please extract correct versions from current using client.");
        exit(1);
    }

    TC_LOG_INFO("server.loading", ">> Loaded %d data stores", DBCFilesCount );
}

SimpleFactionsList const* GetFactionTeamList(uint32 faction)
{
    FactionTeamMap::const_iterator itr = sFactionTeamMap.find(faction);
    if(itr==sFactionTeamMap.end())
        return NULL;
    return &itr->second;
}

std::string GetPetName(uint32 petfamily, LocaleConstant dbclang)
{
    if(!petfamily)
        return "";
    CreatureFamilyEntry const *pet_family = sCreatureFamilyStore.LookupEntry(petfamily);
    if(!pet_family)
        return "";
    return pet_family->Name[(uint32)dbclang]; //can be empty
}

TalentSpellPos const* GetTalentSpellPos(uint32 spellId)
{
    TalentSpellPosMap::const_iterator itr = sTalentSpellPosMap.find(spellId);
    if(itr==sTalentSpellPosMap.end())
        return NULL;

    return &itr->second;
}

uint32 GetTalentSpellCost(uint32 spellId)
{
    if(TalentSpellPos const* pos = GetTalentSpellPos(spellId))
        return pos->rank+1;

    return 0;
}

WMOAreaTableEntry const* GetWMOAreaTableEntryByTripple(int32 rootid, int32 adtid, int32 groupid)
{
    WMOAreaInfoByTripple::iterator i = sWMOAreaInfoByTripple.find(WMOAreaTableTripple(rootid, adtid, groupid));
        if(i == sWMOAreaInfoByTripple.end())
            return NULL;
        return i->second;
}

uint32 GetVirtualMapForMapAndZone(uint32 mapid, uint32 zoneId)
{
    if(mapid == 530 || mapid == 0 || mapid == 1)                                        // speed for most cases
        return mapid;

    if(WorldMapAreaEntry const* wma = sWorldMapAreaStore.LookupEntry(zoneId))
        return wma->virtual_map_id >= 0 ? wma->virtual_map_id : wma->map_id;

    return mapid;
}

ContentLevels GetContentLevelsForMapAndZone(uint32 mapid, uint32 zoneId)
{
    mapid = GetVirtualMapForMapAndZone(mapid, zoneId);
    if(mapid < 2)
        return CONTENT_1_60;

    MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
    if(!mapEntry)
        return CONTENT_1_60;

    switch(mapEntry->Expansion())
    {
        default: return CONTENT_1_60;
        case 1:  return CONTENT_61_70;
        case 2:  return CONTENT_71_80;
    }
}

ChatChannelsEntry const* GetChannelEntryFor(uint32 channel_id)
{
    // not sorted, numbering index from 0
    for(uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
    {
        ChatChannelsEntry const* ch = sChatChannelsStore.LookupEntry(i);
        if(ch && ch->ChannelID == channel_id)
            return ch;
    }
    return NULL;
}

bool IsTotemCategoryCompatibleWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId, Item* pItem)
{
    if(requiredTotemCategoryId==0)
        return true;
    if(itemTotemCategoryId==0)
        return false;

    TotemCategoryEntry const* itemEntry = sTotemCategoryStore.LookupEntry(itemTotemCategoryId);
    if(!itemEntry)
        return false;
    TotemCategoryEntry const* reqEntry = sTotemCategoryStore.LookupEntry(requiredTotemCategoryId);
    if(!reqEntry)
        return false;

    if(itemEntry->categoryType!=reqEntry->categoryType)
        return false;
        
    // Master totem MUST be equipped to be used
    if (pItem && itemTotemCategoryId == 21 && !pItem->IsEquipped())
        return false;

    return (itemEntry->categoryMask & reqEntry->categoryMask)==reqEntry->categoryMask;
}

void Zone2MapCoordinates(float& x,float& y,uint32 zoneId)
{
    //sWorldMapAreaStore has index on zoneId instead of its original id
    WorldMapAreaEntry const* maEntry = sWorldMapAreaStore.LookupEntry(zoneId);

    // if not listed then map coordinates (instance)
    if(!maEntry)
        return;

    std::swap(x,y);                                         // at client map coords swapped
    x = x*((maEntry->x2-maEntry->x1)/100)+maEntry->x1;
    y = y*((maEntry->y2-maEntry->y1)/100)+maEntry->y1;      // client y coord from top to down
}

void Map2ZoneCoordinates(float& x,float& y,uint32 zoneId)
{
    //sWorldMapAreaStore has index on zoneId instead of its original id
    WorldMapAreaEntry const* maEntry = sWorldMapAreaStore.LookupEntry(zoneId);

    // if not listed then map coordinates (instance)
    if(!maEntry)
        return;

    x = (x-maEntry->x1)/((maEntry->x2-maEntry->x1)/100);
    y = (y-maEntry->y1)/((maEntry->y2-maEntry->y1)/100);    // client y coord from top to down
    std::swap(x,y);                                         // client have map coords swapped
}

MapDifficulty const* GetMapDifficultyData(uint32 mapId, Difficulty difficulty)
{
    MapDifficultyMap::const_iterator itr = sMapDifficultyMap.find(MAKE_PAIR32(mapId, difficulty));
    return itr != sMapDifficultyMap.end() ? &itr->second : NULL;
}

MapDifficulty const* GetDownscaledMapDifficultyData(uint32 mapId, Difficulty &difficulty)
{
    uint32 tmpDiff = difficulty;
    MapDifficulty const* mapDiff = GetMapDifficultyData(mapId, Difficulty(tmpDiff));
    if (!mapDiff && tmpDiff > 0)
    {
#ifdef LICH_KING
        if (tmpDiff >= RAID_DIFFICULTY_10MAN_HEROIC) // heroic, downscale to normal (Ex: RAID_DIFFICULTY_10MAN_HEROIC - 2 = RAID_DIFFICULTY_10MAN_NORMAL)
            tmpDiff -= 2;
        else
#endif
            tmpDiff -= 1;   // any non-normal mode for raids like tbc (only one mode)

        // pull new data
        mapDiff = GetMapDifficultyData(mapId, Difficulty(tmpDiff)); // we are 10 normal or 25 normal
#ifdef LICH_KING
        if (!mapDiff && tmpDiff > 0)
        {
            tmpDiff -= 1;
            mapDiff = GetMapDifficultyData(mapId, Difficulty(tmpDiff)); // 10 normal
        }
#endif
    }

    ASSERT(tmpDiff < MAX_DIFFICULTY);
    difficulty = Difficulty(tmpDiff);
    return mapDiff;
}

SkillRaceClassInfoEntry const* GetSkillRaceClassInfo(uint32 skill, uint8 race, uint8 class_)
{
    SkillRaceClassInfoBounds bounds = SkillRaceClassInfoBySkill.equal_range(skill);
    for (SkillRaceClassInfoMap::iterator itr = bounds.first; itr != bounds.second; ++itr)
    {
        if (itr->second->RaceMask && !(itr->second->RaceMask & (1 << (race - 1))))
            continue;
        if (itr->second->ClassMask && !(itr->second->ClassMask & (1 << (class_ - 1))))
            continue;

        return itr->second;
    }

    return NULL;
}

uint32 GetTalentInspectBitPosInTab(uint32 talentId)
{
    TalentInspectMap::const_iterator itr = sTalentPosInInspect.find(talentId);
    if(itr == sTalentPosInInspect.end())
        return 0;

    return itr->second;
}

uint32 GetTalentTabInspectBitSize(uint32 talentTabId)
{
    TalentInspectMap::const_iterator itr = sTalentTabSizeInInspect.find(talentTabId);
    if(itr == sTalentTabSizeInInspect.end())
        return 0;

    return itr->second;
}

uint32 const* GetTalentTabPages(uint32 cls)
{
    return sTalentTabPages[cls];
}

uint32 GetLiquidFlagsFromType(uint32 liquidType)
{
#ifdef LICH_KING
    return 1 << liquidType;
#else
    //BC got different values in the type field, they were re ordered from LK
    switch (liquidType)
    {
        case BCLiquidTypeType_Magma: return MAP_LIQUID_TYPE_MAGMA;
        case BCLiquidTypeType_Slime: return MAP_LIQUID_TYPE_SLIME;
        case BCLiquidTypeType_Water:
        default:                     return MAP_LIQUID_TYPE_WATER;
    }
#endif
}

uint32 GetLiquidFlags(uint32 liquidTypeRec)
{
    if (LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(liquidTypeRec))
        return GetLiquidFlagsFromType(liq->GetType());

    return 0;
}

CharacterFacialHairStylesEntry const* GetCharFacialHairEntry(uint8 race, uint8 gender, uint8 facialHairID)
{
    auto itr = sCharFacialHairMap.find(uint32(race) | uint32(gender << 8) | uint32(facialHairID << 16));
    if (itr == sCharFacialHairMap.end())
        return nullptr;

    return itr->second;
}

CharSectionsEntry const* GetCharSectionEntry(uint8 race, CharSectionType genType, uint8 gender, uint8 type, uint8 color)
{
    uint32 const key = uint32(genType) | uint32(gender << 8) | uint32(race << 16);
    for (auto const& section : Trinity::Containers::MapEqualRange(sCharSectionMap, key))
    {
        if (section.second->Type == type && section.second->Color == color)
            return section.second;
    }

    return nullptr;
}

CharStartOutfitEntry const* GetCharStartOutfitEntry(uint8 race, uint8 class_, uint8 gender)
{
    std::map<uint32, CharStartOutfitEntry const*>::const_iterator itr = sCharStartOutfitMap.find(race | (class_ << 8) | (gender << 16));
    if (itr == sCharStartOutfitMap.end())
        return nullptr;

    return itr->second;
}

#ifndef LICH_KING
//only for BC, use this instead of the non existing DBC
std::map<uint32, PvPDifficultyEntry> const& GetBCDifficultyEntries()
{
    auto& BCDifficultyEntries = sObjectMgr->BCDifficultyEntries;
    if (!BCDifficultyEntries.empty())
        return BCDifficultyEntries;

    //PVPZone01 (Alterac Valley)
    BCDifficultyEntries[1] = PvPDifficultyEntry(30, 0, 51, 60, 0);
    BCDifficultyEntries[2] = PvPDifficultyEntry(30, 1, 61, 70, 1);
    //PVPZone03 (Warsong Gulch)
    BCDifficultyEntries[5] = PvPDifficultyEntry(489, 0, 10, 19, 0);
    BCDifficultyEntries[6] = PvPDifficultyEntry(489, 1, 20, 29, 0);
    BCDifficultyEntries[7] = PvPDifficultyEntry(489, 2, 30, 39, 0);
    BCDifficultyEntries[8] = PvPDifficultyEntry(489, 3, 40, 49, 0);
    BCDifficultyEntries[9] = PvPDifficultyEntry(489, 4, 50, 59, 0);
    BCDifficultyEntries[10] = PvPDifficultyEntry(489, 5, 60, 69, 0);
    BCDifficultyEntries[11] = PvPDifficultyEntry(489, 6, 70, 79, 0);
    //PVPZone04 (Arathi Basin)
    BCDifficultyEntries[13] = PvPDifficultyEntry(529, 0, 20, 29, 0);
    BCDifficultyEntries[14] = PvPDifficultyEntry(529, 1, 30, 39, 0);
    BCDifficultyEntries[15] = PvPDifficultyEntry(529, 2, 40, 49, 0);
    BCDifficultyEntries[16] = PvPDifficultyEntry(529, 3, 50, 59, 0);
    BCDifficultyEntries[17] = PvPDifficultyEntry(529, 4, 60, 69, 0);
    BCDifficultyEntries[18] = PvPDifficultyEntry(529, 5, 70, 79, 0);
    //PVPZone05 (Nagrand Arena)
    BCDifficultyEntries[20] = PvPDifficultyEntry(559, 0, 10, 14, 0);
    BCDifficultyEntries[21] = PvPDifficultyEntry(559, 1, 15, 19, 0);
    BCDifficultyEntries[22] = PvPDifficultyEntry(559, 2, 20, 24, 0);
    BCDifficultyEntries[23] = PvPDifficultyEntry(559, 3, 25, 29, 0);
    BCDifficultyEntries[24] = PvPDifficultyEntry(559, 4, 30, 34, 0);
    BCDifficultyEntries[25] = PvPDifficultyEntry(559, 5, 35, 39, 0);
    BCDifficultyEntries[26] = PvPDifficultyEntry(559, 6, 40, 44, 0);
    BCDifficultyEntries[27] = PvPDifficultyEntry(559, 7, 45, 49, 0);
    BCDifficultyEntries[28] = PvPDifficultyEntry(559, 8, 50, 54, 0);
    BCDifficultyEntries[29] = PvPDifficultyEntry(559, 9, 55, 59, 0);
    BCDifficultyEntries[30] = PvPDifficultyEntry(559, 10, 60, 64, 0);
    BCDifficultyEntries[31] = PvPDifficultyEntry(559, 11, 65, 69, 0);
    BCDifficultyEntries[32] = PvPDifficultyEntry(559, 12, 70, 74, 0);
    //bladesedgearena (Blade's Edge Arena)
    BCDifficultyEntries[36] = PvPDifficultyEntry(562, 0, 10, 14, 0);
    BCDifficultyEntries[37] = PvPDifficultyEntry(562, 1, 15, 19, 0);
    BCDifficultyEntries[38] = PvPDifficultyEntry(562, 2, 20, 24, 0);
    BCDifficultyEntries[39] = PvPDifficultyEntry(562, 3, 25, 29, 0);
    BCDifficultyEntries[40] = PvPDifficultyEntry(562, 4, 30, 34, 0);
    BCDifficultyEntries[41] = PvPDifficultyEntry(562, 5, 35, 39, 0);
    BCDifficultyEntries[42] = PvPDifficultyEntry(562, 6, 40, 44, 0);
    BCDifficultyEntries[43] = PvPDifficultyEntry(562, 7, 45, 49, 0);
    BCDifficultyEntries[44] = PvPDifficultyEntry(562, 8, 50, 54, 0);
    BCDifficultyEntries[45] = PvPDifficultyEntry(562, 9, 55, 59, 0);
    BCDifficultyEntries[46] = PvPDifficultyEntry(562, 10, 60, 64, 0);
    BCDifficultyEntries[47] = PvPDifficultyEntry(562, 11, 65, 69, 0);
    BCDifficultyEntries[48] = PvPDifficultyEntry(562, 12, 70, 74, 0);
    //NetherstormBG (Eye of the Storm)
    BCDifficultyEntries[52] = PvPDifficultyEntry(566, 0, 61, 69, 0);
    BCDifficultyEntries[53] = PvPDifficultyEntry(566, 1, 70, 79, 0);
    //PVPLordaeron (Ruins of Lordaeron)
    BCDifficultyEntries[55] = PvPDifficultyEntry(572, 0, 10, 14, 0);
    BCDifficultyEntries[56] = PvPDifficultyEntry(572, 1, 15, 19, 0);
    BCDifficultyEntries[57] = PvPDifficultyEntry(572, 2, 20, 24, 0);
    BCDifficultyEntries[58] = PvPDifficultyEntry(572, 3, 25, 29, 0);
    BCDifficultyEntries[59] = PvPDifficultyEntry(572, 4, 30, 34, 0);
    BCDifficultyEntries[60] = PvPDifficultyEntry(572, 5, 35, 39, 0);
    BCDifficultyEntries[61] = PvPDifficultyEntry(572, 6, 40, 44, 0);
    BCDifficultyEntries[62] = PvPDifficultyEntry(572, 7, 45, 49, 0);
    BCDifficultyEntries[63] = PvPDifficultyEntry(572, 8, 50, 54, 0);
    BCDifficultyEntries[64] = PvPDifficultyEntry(572, 9, 55, 59, 0);
    BCDifficultyEntries[65] = PvPDifficultyEntry(572, 10, 60, 64, 0);
    BCDifficultyEntries[66] = PvPDifficultyEntry(572, 11, 65, 69, 0);
    BCDifficultyEntries[67] = PvPDifficultyEntry(572, 12, 70, 74, 0);

    return BCDifficultyEntries;
}

#endif

PvPDifficultyEntry const* GetBattlegroundBracketByLevel(uint32 mapid, uint32 level)
{
    PvPDifficultyEntry const* maxEntry = nullptr;              // used for level > max listed level case
#ifdef LICH_KING
    for (uint32 i = 0; i < sPvPDifficultyStore.GetNumRows(); ++i)
    {
        if (PvPDifficultyEntry const* entry = sPvPDifficultyStore.LookupEntry(i))
#else
    auto const& BCDifficultyEntries = GetBCDifficultyEntries();
    for (auto const& itr : BCDifficultyEntries)
    {
        PvPDifficultyEntry const* entry = &(itr.second);
#endif
        {
            // skip unrelated and too-high brackets
            if (entry->mapId != mapid || entry->minLevel > level)
                continue;

            // exactly fit
            if (entry->maxLevel >= level)
                return entry;

            // remember for possible out-of-range case (search higher from existed)
            if (!maxEntry || maxEntry->maxLevel < entry->maxLevel)
                maxEntry = entry;
        }
    }

    return maxEntry;
}

PvPDifficultyEntry const* GetBattlegroundBracketById(uint32 mapid, BattlegroundBracketId id)
{
#ifdef LICH_KING
    for (uint32 i = 0; i < sPvPDifficultyStore.GetNumRows(); ++i)
    {
        PvPDifficultyEntry const* entry = sPvPDifficultyStore.LookupEntry(i);
        if (!entry)
            continue;

#else
    auto const& BCDifficultyEntries = GetBCDifficultyEntries();
    for (auto const& itr : BCDifficultyEntries)
    {
        PvPDifficultyEntry const* entry = &(itr.second);
#endif
        if (entry->mapId == mapid && entry->GetBracketId() == id)
            return entry;
    }

    return nullptr;
}

uint32 GetDefaultMapLight(uint32 mapId)
{
    for (int32 i = sLightStore.GetNumRows(); i >= 0; --i)
    {
        LightEntry const* light = sLightStore.LookupEntry(uint32(i));
        if (!light)
            continue;

        if (light->MapId == mapId && light->X == 0.0f && light->Y == 0.0f && light->Z == 0.0f)
            return light->Id;
    }

    return 0;
}


// script support functions
DBCStorage <SoundEntriesEntry>  const* GetSoundEntriesStore()   { return &sSoundEntriesStore;   }
//DBCStorage <SpellEntry>         const* GetSpellStore()          { return &sSpellStore;          }
DBCStorage <SpellRangeEntry>    const* GetSpellRangeStore()     { return &sSpellRangeStore;     }
