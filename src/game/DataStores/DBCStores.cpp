#include "DBCStores.h"
#include "Log.h"
#include "TransportMgr.h"
#include "Item.h"
#include "DBCfmt.h"

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
DBCStorage <CharStartOutfitEntry> sCharStartOutfitStore(CharStartOutfitEntryfmt);
DBCStorage <CharTitlesEntry> sCharTitlesStore(CharTitlesEntryfmt);
DBCStorage <ChatChannelsEntry> sChatChannelsStore(ChatChannelsEntryfmt);
DBCStorage <ChrClassesEntry> sChrClassesStore(ChrClassesEntryfmt);
DBCStorage <ChrRacesEntry> sChrRacesStore(ChrRacesEntryfmt);
DBCStorage <CinematicCameraEntry> sCinematicCameraStore(CinematicCameraEntryfmt);
DBCStorage <CreatureDisplayInfoEntry> sCreatureDisplayInfoStore(CreatureDisplayInfofmt);
DBCStorage <CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore(CreatureDisplayInfoExtrafmt);
DBCStorage <CreatureFamilyEntry> sCreatureFamilyStore(CreatureFamilyfmt);
DBCStorage <CreatureModelDataEntry> sCreatureModelDataStore(CreatureModelDatafmt);
DBCStorage <CreatureSpellDataEntry> sCreatureSpellDataStore(CreatureSpellDatafmt);

DBCStorage <DurabilityQualityEntry> sDurabilityQualityStore(DurabilityQualityfmt);
DBCStorage <DurabilityCostsEntry> sDurabilityCostsStore(DurabilityCostsfmt);

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

//NYI for BC, you'll have to fix readCamera first to use this (some hard work already done there)
std::unordered_map<uint32, FlyByCameraCollection> sFlyByCameraStore;

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
inline void LoadDBC(uint32& availableDbcLocales, StoreProblemList& errors, DBCStorage<T>& storage, std::string const& dbcPath, std::string const& filename, std::string const* customFormat = NULL, std::string const* customIndexName = NULL)
{
    // compatibility format and C++ structure sizes
    ASSERT(DBCFileLoader::GetFormatRecordSize(storage.GetFormat()) == sizeof(T) || LoadDBC_assert_print(DBCFileLoader::GetFormatRecordSize(storage.GetFormat()), sizeof(T), filename));

    ++DBCFileCount;
    std::string dbcFilename = dbcPath + filename;
    SqlDbc * sql = NULL;
    if (customFormat)
        sql = new SqlDbc(&filename, customFormat, customIndexName, storage.GetFormat());

    if (storage.Load(dbcFilename.c_str(), sql))
    {
        for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
        {
            if (!(availableDbcLocales & (1 << i)))
                continue;

            std::string localizedName(dbcPath);
            localizedName.append(localeNames[i]);
            localizedName.push_back('/');
            localizedName.append(filename);

            if (!storage.LoadStringsFrom(localizedName.c_str()))
                availableDbcLocales &= ~(1<<i);             // mark as not available for speedup next checks
        }
    }
    else
    {
        // sort problematic dbc to (1) non compatible and (2) non-existed
        if (FILE* f = fopen(dbcFilename.c_str(), "rb"))
        {
            std::ostringstream stream;
            stream << dbcFilename << " exists, and has " << storage.GetFieldCount() << " field(s) (expected " << strlen(storage.GetFormat()) << "). Extracted file might be from wrong client version or a database-update has been forgotten.";
            std::string buf = stream.str();
            errors.push_back(buf);
            fclose(f);
        }
        else
            errors.push_back(dbcFilename);
    }

    delete sql;
}

void LoadDBCStores(const std::string& dataPath)
{
    std::string dbcPath = dataPath+"dbc/";

    const uint32 DBCFilesCount = 58;

    StoreProblemList bad_dbc_files;
    uint32 availableDbcLocales = 0xFFFFFFFF;

    LoadDBC(availableDbcLocales,bad_dbc_files,sAreaTableStore,           dbcPath,"AreaTable.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sAreaTriggerStore,         dbcPath,"AreaTrigger.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sAuctionHouseStore,        dbcPath,"AuctionHouse.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sBankBagSlotPricesStore,   dbcPath,"BankBagSlotPrices.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sBattlemasterListStore,    dbcPath,"BattlemasterList.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sCharStartOutfitStore,     dbcPath,"CharStartOutfit.dbc");

    LoadDBC(availableDbcLocales,bad_dbc_files,sCharTitlesStore,          dbcPath,"CharTitles.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sChatChannelsStore,        dbcPath,"ChatChannels.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sChrClassesStore,          dbcPath,"ChrClasses.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sChrRacesStore,            dbcPath,"ChrRaces.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sCinematicCameraStore,     dbcPath,"CinematicCamera.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sCreatureDisplayInfoStore, dbcPath,"CreatureDisplayInfo.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sCreatureFamilyStore,      dbcPath,"CreatureFamily.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sCreatureSpellDataStore,   dbcPath,"CreatureSpellData.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sDurabilityCostsStore,     dbcPath,"DurabilityCosts.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sDurabilityQualityStore,   dbcPath,"DurabilityQuality.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sEmotesTextStore,          dbcPath,"EmotesText.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sFactionStore,             dbcPath,"Faction.dbc");
    for (uint32 i=0;i<sFactionStore.GetNumRows(); ++i)
    {
        FactionEntry const * faction = sFactionStore.LookupEntry(i);
        if (faction && faction->team)
        {
            SimpleFactionsList &flist = sFactionTeamMap[faction->team];
            flist.push_back(i);
        }
    }

    LoadDBC(availableDbcLocales,bad_dbc_files,sFactionTemplateStore,     dbcPath,"FactionTemplate.dbc");
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
    LoadDBC(availableDbcLocales,bad_dbc_files,sGemPropertiesStore,       dbcPath,"GemProperties.dbc");

    LoadDBC(availableDbcLocales,bad_dbc_files,sGtCombatRatingsStore,     dbcPath,"gtCombatRatings.dbc");

    LoadDBC(availableDbcLocales,bad_dbc_files,sGtChanceToMeleeCritBaseStore, dbcPath,"gtChanceToMeleeCritBase.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sGtChanceToMeleeCritStore, dbcPath,"gtChanceToMeleeCrit.dbc");

    LoadDBC(availableDbcLocales,bad_dbc_files,sGtChanceToSpellCritBaseStore, dbcPath,"gtChanceToSpellCritBase.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sGtChanceToSpellCritStore, dbcPath,"gtChanceToSpellCrit.dbc");

    LoadDBC(availableDbcLocales,bad_dbc_files,sGtOCTRegenHPStore,        dbcPath,"gtOCTRegenHP.dbc");
    //LoadDBC(availableDbcLocales,bad_dbc_files,sGtOCTRegenMPStore,        dbcPath,"gtOCTRegenMP.dbc");       -- not used currently
    LoadDBC(availableDbcLocales,bad_dbc_files,sGtRegenHPPerSptStore,     dbcPath,"gtRegenHPPerSpt.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sGtRegenMPPerSptStore,     dbcPath,"gtRegenMPPerSpt.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sItemStore,                dbcPath,"Item.dbc");
    //LoadDBC(availableDbcLocales,bad_dbc_files,sItemDisplayInfoStore,     dbcPath,"ItemDisplayInfo.dbc");     -- not used currently
    //LoadDBC(availableDbcLocales,bad_dbc_files,sItemCondExtCostsStore,    dbcPath,"ItemCondExtCosts.dbc");
    //moved to db LoadDBC(availableDbcLocales,bad_dbc_files,sItemExtendedCostStore,    dbcPath,"ItemExtendedCost.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sItemRandomPropertiesStore,dbcPath,"ItemRandomProperties.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sItemRandomSuffixStore,    dbcPath,"ItemRandomSuffix.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sItemSetStore,             dbcPath,"ItemSet.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sLightStore,               dbcPath,"Light.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sLiquidTypeStore,          dbcPath,"LiquidType.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sLockStore,                dbcPath,"Lock.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sMailTemplateStore,        dbcPath,"MailTemplate.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sMapStore,                 dbcPath,"Map.dbc");
#ifdef LICH_KING
    LoadDBC(availableDbcLocales, bad_dbc_files, sMapDifficultyStore, dbcPath, "MapDifficulty.dbc");
    // fill data
    for (uint32 i = 1; i < sMapDifficultyStore.GetNumRows(); ++i)
        if (MapDifficultyEntry const* entry = sMapDifficultyStore.LookupEntry(i))
            sMapDifficultyMap[MAKE_PAIR32(entry->MapId, entry->Difficulty)] = MapDifficulty(entry->resetTime, entry->maxPlayers, entry->areaTriggerText[0] != '\0');
    sMapDifficultyStore.Clear();
#else
    //fake MapDifficulty.dbc for BC is handled in ObjectMgr::LoadInstanceTemplate()
#endif
    LoadDBC(availableDbcLocales,bad_dbc_files,sQuestSortStore,           dbcPath,"QuestSort.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sRandomPropertiesPointsStore, dbcPath,"RandPropPoints.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sSkillLineStore,           dbcPath,"SkillLine.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sSkillLineAbilityStore,    dbcPath,"SkillLineAbility.dbc");
    LoadDBC(availableDbcLocales, bad_dbc_files, sSkillRaceClassInfoStore, dbcPath, "SkillRaceClassInfo.dbc");
    for (uint32 i = 0; i < sSkillRaceClassInfoStore.GetNumRows(); ++i)
        if (SkillRaceClassInfoEntry const* entry = sSkillRaceClassInfoStore.LookupEntry(i))
            if (sSkillLineStore.LookupEntry(entry->SkillId))
                SkillRaceClassInfoBySkill.emplace(entry->SkillId, entry);

    LoadDBC(availableDbcLocales,bad_dbc_files,sSkillTiersStore,          dbcPath, "SkillTiers.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sSoundEntriesStore,        dbcPath,"SoundEntries.dbc");
    //"Spell.dbc" is now world.spell_template table

    LoadDBC(availableDbcLocales,bad_dbc_files,sSpellCastTimesStore,      dbcPath,"SpellCastTimes.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sSpellCategoryStore,       dbcPath,"SpellCategory.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sSpellDurationStore,       dbcPath,"SpellDuration.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sSpellFocusObjectStore,    dbcPath,"SpellFocusObject.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sSpellItemEnchantmentStore,dbcPath,"SpellItemEnchantment.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sSpellItemEnchantmentConditionStore,dbcPath,"SpellItemEnchantmentCondition.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sSpellRadiusStore,         dbcPath,"SpellRadius.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sSpellRangeStore,          dbcPath,"SpellRange.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sSpellShapeshiftStore,     dbcPath,"SpellShapeshiftForm.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sStableSlotPricesStore,    dbcPath,"StableSlotPrices.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sTalentStore,              dbcPath,"Talent.dbc");

    // create talent spells set
    for (unsigned int i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const *talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo) continue;
        for (int j = 0; j < 5; j++)
            if(talentInfo->RankID[j])
                sTalentSpellPosMap[talentInfo->RankID[j]] = TalentSpellPos(i,j);
    }

    LoadDBC(availableDbcLocales,bad_dbc_files,sTalentTabStore,           dbcPath,"TalentTab.dbc");

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

    LoadDBC(availableDbcLocales,bad_dbc_files,sTaxiNodesStore,           dbcPath,"TaxiNodes.dbc");

    // Initialize global taxinodes mask
    memset(sTaxiNodesMask,0,sizeof(sTaxiNodesMask));
    for(uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
    {
        if(sTaxiNodesStore.LookupEntry(i))
        {
            uint8  field   = (uint8)((i - 1) / 32);
            uint32 submask = 1<<((i-1)%32);
            sTaxiNodesMask[field] |= submask;
        }
    }

    LoadDBC(availableDbcLocales,bad_dbc_files,sTaxiPathStore,            dbcPath,"TaxiPath.dbc");
    for(uint32 i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        if(TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
            sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->ID,entry->price);
    uint32 pathCount = sTaxiPathStore.GetNumRows();

    //## TaxiPathNode.dbc ## Loaded only for initialization different structures
    LoadDBC(availableDbcLocales, bad_dbc_files, sTaxiPathNodeStore,           dbcPath, "TaxiPathNode.dbc");
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

    LoadDBC(availableDbcLocales,bad_dbc_files,sTotemCategoryStore,       dbcPath,"TotemCategory.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sTransportAnimationStore,  dbcPath,"TransportAnimation.dbc");
    for (uint32 i = 0; i < sTransportAnimationStore.GetNumRows(); ++i)
    {
        TransportAnimationEntry const* anim = sTransportAnimationStore.LookupEntry(i);
        if (!anim)
            continue;

        sTransportMgr->AddPathNodeToTransport(anim->TransportEntry, anim->TimeSeg, anim);
    }

    LoadDBC(availableDbcLocales,bad_dbc_files,sWMOAreaTableStore,        dbcPath,"WMOAreaTable.dbc");
    for(uint32 i = 0; i < sWMOAreaTableStore.GetNumRows(); ++i)
    {
        if(WMOAreaTableEntry const* entry = sWMOAreaTableStore.LookupEntry(i))
        {
            sWMOAreaInfoByTripple.insert(WMOAreaInfoByTripple::value_type(WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry));
        }
    }
    LoadDBC(availableDbcLocales,bad_dbc_files,sWorldMapAreaStore,        dbcPath,"WorldMapArea.dbc");
    LoadDBC(availableDbcLocales,bad_dbc_files,sWorldSafeLocsStore,       dbcPath,"WorldSafeLocs.dbc");

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

    LoadM2Cameras(dataPath);

    TC_LOG_INFO("server.loading"," ");
    TC_LOG_INFO("server.loading", ">> Loaded %d data stores", DBCFilesCount );
    TC_LOG_INFO("server.loading"," ");
}


// Convert the geomoetry from a spline value, to an actual WoW XYZ
G3D::Vector3 TranslateLocation(G3D::Vector4 const* DBCPosition, G3D::Vector3 const* basePosition, G3D::Vector3 const* splineVector)
{
    G3D::Vector3 work;
    float x = basePosition->x + splineVector->x;
    float y = basePosition->y + splineVector->y;
    float z = basePosition->z + splineVector->z;
    float const distance = sqrt((x * x) + (y * y));
    float angle = std::atan2(x, y) - DBCPosition->w;

    if (angle < 0)
        angle += 2 * float(M_PI);

    work.x = DBCPosition->x + (distance * sin(angle));
    work.y = DBCPosition->y + (distance * cos(angle));
    work.z = DBCPosition->z + z;
    return work;
}

// Number of cameras not used. Multiple cameras never used in 3.3.5
bool readCamera(M2Camera const* cam, uint32 buffSize, M2Header const* header, CinematicCameraEntry const* dbcentry)
{
    char const* buffer = reinterpret_cast<char const*>(header);

    FlyByCameraCollection cameras;
    FlyByCameraCollection targetcam;

    G3D::Vector4 DBCData;
    DBCData.x = dbcentry->base_x;
    DBCData.y = dbcentry->base_y;
    DBCData.z = dbcentry->base_z;
    DBCData.w = dbcentry->base_o;

#ifndef LICH_KING
    // kelno: This is not tested yet. Note, positions seems to be wrong, maybe target_positions is swapped on BC

    uint32 targetInterpolationMin = 0;
    uint32 targetInterpolationMax = 0;
    uint32 interpolationMin = 0;
    uint32 interpolationMax = 0;

    //can this have more than 1 entry ? if so, this code is invalid
    for (uint32 k = 0; k < cam->target_positions.interpolation_ranges.number; ++k)
    {
        if (cam->target_positions.interpolation_ranges.offset_elements + (sizeof(InterpolationPair) * k) > buffSize)
            return false;

        InterpolationPair const* interpolationPair = reinterpret_cast<InterpolationPair const*>(buffer + (sizeof(InterpolationPair) * k) + cam->target_positions.interpolation_ranges.offset_elements);
        targetInterpolationMin = interpolationPair->min;
        targetInterpolationMax = interpolationPair->max;
        break;
    }

    //can this have more than 1 entry ? if so, this code is invalid
    for (uint32 k = 0; k < cam->positions.interpolation_ranges.number; ++k)
    {
        if (cam->positions.interpolation_ranges.offset_elements + (sizeof(InterpolationPair) * k) > buffSize)
            return false;

        InterpolationPair const* interpolationPair = reinterpret_cast<InterpolationPair const*>(buffer + (sizeof(InterpolationPair) * k) + cam->positions.interpolation_ranges.offset_elements);
        interpolationMin = interpolationPair->min;
        interpolationMax = interpolationPair->max;
        break;
    }

    //interpolationMax may be at 0 for some cases ?
    if (/*interpolationMax == 0 || */targetInterpolationMax == 0)
    {
        TC_LOG_ERROR("server.loading", "readCamera: Invalid max interpolation value (0)");
        return false;
    }

    // Read target locations, only so that we can calculate orientation
    for (uint32 k = 0; k < cam->target_positions.timestampsBC.number; ++k)
    {
        // Extract Target positions
        if (cam->target_positions.timestampsBC.offset_elements + (sizeof(uint32) * k) > buffSize)
            return false;

        uint32 const* timeStamp = reinterpret_cast<uint32 const*>(buffer + (sizeof(uint32) * k) + cam->target_positions.timestampsBC.offset_elements);

        if (cam->target_positions.valuesBC.offset_elements + (sizeof(M2SplineKey<G3D::Vector3>) * k) > buffSize)
            return false;

        M2SplineKey<G3D::Vector3> const* targPositions = reinterpret_cast<M2SplineKey<G3D::Vector3> const*>(buffer + (sizeof(M2SplineKey<G3D::Vector3>) * k) + cam->target_positions.valuesBC.offset_elements);

        G3D::Vector3 newPos = TranslateLocation(&DBCData, &cam->target_position_base, &targPositions->p0);

        FlyByCamera thisCam;
        thisCam.timeStamp = *timeStamp;
        thisCam.locations.x = newPos.x;
        thisCam.locations.y = newPos.y;
        thisCam.locations.z = newPos.z;
        thisCam.locations.w = 0.0f;
        targetcam.push_back(thisCam);

        if (k >= targetInterpolationMax)
            break;
    }

    // Read camera positions and timestamps (translating first position of 3 only, we don't need to translate the whole spline)
    for (uint32 k = 0; k < cam->positions.timestampsBC.number; ++k)
    {
        // Extract Target positions
        if (cam->positions.timestampsBC.offset_elements + (sizeof(uint32) * k) > buffSize)
            return false;

        uint32 const* timeStamp = reinterpret_cast<uint32 const*>(buffer + (sizeof(uint32) * k) + cam->positions.timestampsBC.offset_elements);

        if (cam->positions.valuesBC.offset_elements + (sizeof(M2SplineKey<G3D::Vector3>) * k) > buffSize)
            return false;

        M2SplineKey<G3D::Vector3> const* positions = reinterpret_cast<M2SplineKey<G3D::Vector3> const*>(buffer + (sizeof(M2SplineKey<G3D::Vector3>) * k) + cam->positions.valuesBC.offset_elements);
       
        G3D::Vector3 newPos = TranslateLocation(&DBCData, &cam->position_base, &positions->p0);

        FlyByCamera thisCam;
        thisCam.timeStamp = *timeStamp;
        thisCam.locations.x = newPos.x;
        thisCam.locations.y = newPos.y;
        thisCam.locations.z = newPos.z;
        thisCam.locations.w = 0.0f;

        if (targetcam.size() > 0)
        {
            // Find the target camera before and after this camera
            FlyByCamera lastTarget;
            FlyByCamera nextTarget;

            // Pre-load first item
            lastTarget = targetcam[0];
            nextTarget = targetcam[0];
            for (uint32 j = 0; j < targetcam.size(); ++j)
            {
                nextTarget = targetcam[j];
                if (targetcam[j].timeStamp > *timeStamp)
                    break;

                lastTarget = targetcam[j];
            }

            float x = lastTarget.locations.x;
            float y = lastTarget.locations.y;
            float z = lastTarget.locations.z;

            // Now, the timestamps for target cam and position can be different. So, if they differ we interpolate
            if (lastTarget.timeStamp != *timeStamp)
            {
                uint32 timeDiffTarget = nextTarget.timeStamp - lastTarget.timeStamp;
                uint32 timeDiffThis = *timeStamp - lastTarget.timeStamp;
                float xDiff = nextTarget.locations.x - lastTarget.locations.x;
                float yDiff = nextTarget.locations.y - lastTarget.locations.y;
                float zDiff = nextTarget.locations.z - lastTarget.locations.z;
                x = lastTarget.locations.x + (xDiff * (float(timeDiffThis) / float(timeDiffTarget)));
                y = lastTarget.locations.y + (yDiff * (float(timeDiffThis) / float(timeDiffTarget)));
                z = lastTarget.locations.z + (zDiff * (float(timeDiffThis) / float(timeDiffTarget)));
            }
            float xDiff = x - thisCam.locations.x;
            float yDiff = y - thisCam.locations.y;
            thisCam.locations.w = std::atan2(yDiff, xDiff);

            if (thisCam.locations.w < 0)
                thisCam.locations.w += 2 * float(M_PI);
        }

        cameras.push_back(thisCam);

        if (k >= interpolationMax)
            break;
    }

#else
    // Read target locations, only so that we can calculate orientation
    for (uint32 k = 0; k < cam->target_positions.timestampsLK.number; ++k)
    {
        // Extract Target positions
        if (cam->target_positions.timestampsLK.offset_elements + sizeof(M2Array) > buffSize)
            return false;
        M2Array const* targTsArray = reinterpret_cast<M2Array const*>(buffer + cam->target_positions.timestampsLK.offset_elements);
        if (targTsArray->offset_elements + sizeof(uint32) > buffSize || cam->target_positions.valuesLK.offset_elements + sizeof(M2Array) > buffSize)
            return false;
        uint32 const* targTimestamps = reinterpret_cast<uint32 const*>(buffer + targTsArray->offset_elements);
        M2Array const* targArray = reinterpret_cast<M2Array const*>(buffer + cam->target_positions.valuesLK.offset_elements);

        if (targArray->offset_elements + sizeof(M2SplineKey<G3D::Vector3>) > buffSize)
            return false;
        M2SplineKey<G3D::Vector3> const* targPositions = reinterpret_cast<M2SplineKey<G3D::Vector3> const*>(buffer + targArray->offset_elements);

        // Read the data for this set
        uint32 currPos = targArray->offset_elements;
        for (uint32 i = 0; i < targTsArray->number; ++i)
        {
            if (currPos + sizeof(M2SplineKey<G3D::Vector3>) > buffSize)
                return false;
            // Translate co-ordinates
            G3D::Vector3 newPos = TranslateLocation(&DBCData, &cam->target_position_base, &targPositions->p0);

            // Add to vector
            FlyByCamera thisCam;
            thisCam.timeStamp = targTimestamps[i];
            thisCam.locations.x = newPos.x;
            thisCam.locations.y = newPos.y;
            thisCam.locations.z = newPos.z;
            thisCam.locations.w = 0.0f;
            targetcam.push_back(thisCam);
            targPositions++;
            currPos += sizeof(M2SplineKey<G3D::Vector3>);
        }
    }

    // Read camera positions and timestamps (translating first position of 3 only, we don't need to translate the whole spline)
    for (uint32 k = 0; k < cam->positions.timestampsLK.number; ++k)
    {
        // Extract Camera positions for this set
        if (cam->positions.timestampsLK.offset_elements + sizeof(M2Array) > buffSize)
            return false;
        M2Array const* posTsArray = reinterpret_cast<M2Array const*>(buffer + cam->positions.timestampsLK.offset_elements);
        if (posTsArray->offset_elements + sizeof(uint32) > buffSize || cam->positions.valuesLK.offset_elements + sizeof(M2Array) > buffSize)
            return false;
        uint32 const* posTimestamps = reinterpret_cast<uint32 const*>(buffer + posTsArray->offset_elements);
        M2Array const* posArray = reinterpret_cast<M2Array const*>(buffer + cam->positions.valuesLK.offset_elements);
        if (posArray->offset_elements + sizeof(M2SplineKey<G3D::Vector3>) > buffSize)
            return false;
        M2SplineKey<G3D::Vector3> const* positions = reinterpret_cast<M2SplineKey<G3D::Vector3> const*>(buffer + posArray->offset_elements);

        // Read the data for this set
        uint32 currPos = posArray->offset_elements;
        for (uint32 i = 0; i < posTsArray->number; ++i)
        {
            if (currPos + sizeof(M2SplineKey<G3D::Vector3>) > buffSize)
                return false;
            // Translate co-ordinates
            G3D::Vector3 newPos = TranslateLocation(&DBCData, &cam->position_base, &positions->p0);

            // Add to vector
            FlyByCamera thisCam;
            thisCam.timeStamp = posTimestamps[i];
            thisCam.locations.x = newPos.x;
            thisCam.locations.y = newPos.y;
            thisCam.locations.z = newPos.z;

            if (targetcam.size() > 0)
            {
                // Find the target camera before and after this camera
                FlyByCamera lastTarget;
                FlyByCamera nextTarget;

                // Pre-load first item
                lastTarget = targetcam[0];
                nextTarget = targetcam[0];
                for (uint32 j = 0; j < targetcam.size(); ++j)
                {
                    nextTarget = targetcam[j];
                    if (targetcam[j].timeStamp > posTimestamps[i])
                        break;

                    lastTarget = targetcam[j];
                }

                float x = lastTarget.locations.x;
                float y = lastTarget.locations.y;
                float z = lastTarget.locations.z;

                // Now, the timestamps for target cam and position can be different. So, if they differ we interpolate
                if (lastTarget.timeStamp != posTimestamps[i])
                {
                    uint32 timeDiffTarget = nextTarget.timeStamp - lastTarget.timeStamp;
                    uint32 timeDiffThis = posTimestamps[i] - lastTarget.timeStamp;
                    float xDiff = nextTarget.locations.x - lastTarget.locations.x;
                    float yDiff = nextTarget.locations.y - lastTarget.locations.y;
                    float zDiff = nextTarget.locations.z - lastTarget.locations.z;
                    x = lastTarget.locations.x + (xDiff * (float(timeDiffThis) / float(timeDiffTarget)));
                    y = lastTarget.locations.y + (yDiff * (float(timeDiffThis) / float(timeDiffTarget)));
                    z = lastTarget.locations.z + (zDiff * (float(timeDiffThis) / float(timeDiffTarget)));
                }
                float xDiff = x - thisCam.locations.x;
                float yDiff = y - thisCam.locations.y;
                thisCam.locations.w = std::atan2(yDiff, xDiff);

                if (thisCam.locations.w < 0)
                    thisCam.locations.w += 2 * float(M_PI);
            }

            cameras.push_back(thisCam);
            positions++;
            currPos += sizeof(M2SplineKey<G3D::Vector3>);
        }
    }
#endif
    sFlyByCameraStore[dbcentry->id] = cameras;
    return true;
}

void LoadM2Cameras(const std::string& dataPath)
{
    sFlyByCameraStore.clear();
    TC_LOG_INFO("server.loading", ">> Loading Cinematic Camera files");

    uint32 oldMSTime = GetMSTime();
    for (uint32 i = 0; i < sCinematicCameraStore.GetNumRows(); ++i)
    {
        if (CinematicCameraEntry const* dbcentry = sCinematicCameraStore.LookupEntry(i))
        {
            std::string filename = dataPath.c_str();
            filename.append(dbcentry->filename);

            // Replace slashes
            size_t loc = filename.find("\\");
            while (loc != std::string::npos)
            {
                filename.replace(loc, 1, "/");
                loc = filename.find("\\");
            }

            // Replace mdx to .m2
            loc = filename.find(".mdx");
            if (loc != std::string::npos)
                filename.replace(loc, 4, ".m2");

            std::ifstream m2file(filename.c_str(), std::ios::in | std::ios::binary);
            if (!m2file.is_open())
                continue;

            // Get file size
            m2file.seekg(0, std::ios::end);
            std::streamoff const fileSize = m2file.tellg();

            // Reject if not at least the size of the header
            if (static_cast<uint32 const>(fileSize) < sizeof(M2Header))
            {
                TC_LOG_ERROR("server.loading", "Camera file %s is damaged. File is smaller than header size", filename.c_str());
                m2file.close();
                continue;
            }

            // Read 4 bytes (signature)
            m2file.seekg(0, std::ios::beg);
            char fileCheck[5];
            m2file.read(fileCheck, 4);
            fileCheck[4] = 0;

            // Check file has correct magic (MD20)
            if (strcmp(fileCheck, "MD20"))
            {
                TC_LOG_ERROR("server.loading", "Camera file %s is damaged. File identifier not found", filename.c_str());
                m2file.close();
                continue;
            }

            // Now we have a good file, read it all into a vector of char's, then close the file.
            std::vector<char> buffer(fileSize);
            m2file.seekg(0, std::ios::beg);
            if (!m2file.read(buffer.data(), fileSize))
            {
                m2file.close();
                continue;
            }
            m2file.close();

            // Read header
            M2Header const* header = reinterpret_cast<M2Header const*>(buffer.data());

            if (header->ofsCameras + sizeof(M2Camera) > static_cast<uint32 const>(fileSize))
            {
                TC_LOG_ERROR("server.loading", "Camera file %s is damaged. Camera references position beyond file end", filename.c_str());
                continue;
            }

            // Get camera(s) - Main header, then dump them.
            M2Camera const* cam = reinterpret_cast<M2Camera const*>(buffer.data() + header->ofsCameras);
            if (!readCamera(cam, fileSize, header, dbcentry))
                TC_LOG_ERROR("server.loading", "Camera file %s is damaged. Camera references position beyond file end", filename.c_str());
        }
    }
    TC_LOG_INFO("server.loading", ">> Loaded %u cinematic waypoint sets in %u ms", (uint32)sFlyByCameraStore.size(), GetMSTimeDiffToNow(oldMSTime));
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
    if (!mapDiff)
    {
        if (difficulty == 0)
            return nullptr;

#ifdef LICH_KING
        if (tmpDiff > RAID_DIFFICULTY_25MAN_NORMAL) // heroic, downscale to normal
            tmpDiff -= 2;
        else
#endif
            tmpDiff -= 1;   // any non-normal mode for raids like tbc (only one mode)

                            // pull new data
        mapDiff = GetMapDifficultyData(mapId, Difficulty(tmpDiff)); // we are 10 normal or 25 normal
        if (!mapDiff)
        {
            tmpDiff -= 1;
            mapDiff = GetMapDifficultyData(mapId, Difficulty(tmpDiff)); // 10 normal
        }
    }

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