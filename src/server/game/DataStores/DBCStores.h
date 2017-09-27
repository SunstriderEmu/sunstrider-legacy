
#ifndef DBCSTORE_H
#define DBCSTORE_H

#include "DBCFileLoader.h"
#include "Field.h"
#include "DBCStructure.h"
#include "Item.h"
#include "Database/DatabaseEnv.h"

// Structures for M4 file. Source: https://wowdev.wiki
template<typename T>
struct M2SplineKey
{
    T p0;
    T p1;
    T p2;
};

//This is from TC, BC adaptation are done with https://wowdev.wiki/M2#Global_sequences (tested and okay)
struct M2Header
{
    char   Magic[4];               // "MD20"
    uint32 Version;                // The version of the format.
    uint32 lName;                  // Length of the model's name including the trailing \0
    uint32 ofsName;                // Offset to the name, it seems like models can get reloaded by this name.should be unique, i guess.
    uint32 GlobalModelFlags;       // 0x0001: tilt x, 0x0002: tilt y, 0x0008: add 2 fields in header, 0x0020: load .phys data (MoP+), 0x0080: has _lod .skin files (MoP?+), 0x0100: is camera related.
    uint32 nGlobalSequences;
    uint32 ofsGlobalSequences;     // A list of timestamps.
    uint32 nAnimations;
    uint32 ofsAnimations;          // Information about the animations in the model.
    uint32 nAnimationLookup;
    uint32 ofsAnimationLookup;     // Mapping of global IDs to the entries in the Animation sequences block.
#ifndef LICH_KING
    uint32 nPlayableAnimationlookup;
    uint32 ofsPlayableAnimationLookup;
#endif
    uint32 nBones;                 // MAX_BONES = 0x100
    uint32 ofsBones;               // Information about the bones in this model.
    uint32 nKeyBoneLookup;
    uint32 ofsKeyBoneLookup;       // Lookup table for key skeletal bones.
    uint32 nVertices;
    uint32 ofsVertices;            // Vertices of the model.
#ifdef LICH_KING
    uint32 nViews;                 // Views (LOD) are now in .skins.
#else
    uint32 nSkinProfiles;
    uint32 ofsSkinProfiles;
#endif
    uint32 nSubmeshAnimations;
    uint32 ofsSubmeshAnimations;   // Submesh color and alpha animations definitions.
    uint32 nTextures;
    uint32 ofsTextures;            // Textures of this model.
    uint32 nTransparency;
    uint32 ofsTransparency;        // Transparency of textures.
#ifndef LICH_KING
    uint32 nUnk;
    uint32 ofsUnk;
#endif
    uint32 nUVAnimation;
    uint32 ofsUVAnimation;
    uint32 nTexReplace;
    uint32 ofsTexReplace;          // Replaceable Textures.
    uint32 nRenderFlags;
    uint32 ofsRenderFlags;         // Blending modes / render flags.
    uint32 nBoneLookupTable;
    uint32 ofsBoneLookupTable;     // A bone lookup table.
    uint32 nTexLookup;
    uint32 ofsTexLookup;           // The same for textures.
    uint32 nTexUnits;              // possibly removed with cata?!
    uint32 ofsTexUnits;            // And texture units. Somewhere they have to be too.
    uint32 nTransLookup;
    uint32 ofsTransLookup;         // Everything needs its lookup. Here are the transparencies.
    uint32 nUVAnimLookup;
    uint32 ofsUVAnimLookup;
    G3D::AABox BoundingBox;            // min/max( [1].z, 2.0277779f ) - 0.16f seems to be the maximum camera height
    float  BoundingSphereRadius;
    G3D::AABox CollisionBox;
    float  CollisionSphereRadius;
    uint32 nBoundingTriangles;
    uint32 ofsBoundingTriangles;   // Our bounding volumes. Similar structure like in the old ofsViews.
    uint32 nBoundingVertices;
    uint32 ofsBoundingVertices;
    uint32 nBoundingNormals;
    uint32 ofsBoundingNormals;
    uint32 nAttachments;
    uint32 ofsAttachments;         // Attachments are for weapons etc.
    uint32 nAttachLookup;
    uint32 ofsAttachLookup;        // Of course with a lookup.
    uint32 nEvents;
    uint32 ofsEvents;              // Used for playing sounds when dying and a lot else.
    uint32 nLights;
    uint32 ofsLights;              // Lights are mainly used in loginscreens but in wands and some doodads too.
    uint32 nCameras;               // Format of Cameras changed with version 271!
    uint32 ofsCameras;             // The cameras are present in most models for having a model in the Character-Tab.
    uint32 nCameraLookup;
    uint32 ofsCameraLookup;        // And lookup-time again.
    uint32 nRibbonEmitters;
    uint32 ofsRibbonEmitters;      // Things swirling around. See the CoT-entrance for light-trails.
    uint32 nParticleEmitters;
    uint32 ofsParticleEmitters;    // Spells and weapons, doodads and loginscreens use them. Blood dripping of a blade? Particles.
#ifdef LICH_KING
    uint32 nBlendMaps;             // This has to deal with blending. Exists IFF (flags & 0x8) != 0. When set, textures blending is overriden by the associated array. See M2/WotLK#Blend_mode_overrides
    uint32 ofsBlendMaps;           // Same as above. Points to an array of uint16 of nBlendMaps entries -- From WoD information.};
#endif
};


struct M2Array
{
    uint32_t number;
    uint32 offset_elements;
};

#ifndef LICH_KING
struct InterpolationPair
{
    uint32 min;
    uint32 max;
};
#endif

//https://wowdev.wiki/M2#Standard_animation_block
struct M2Track
{
    uint16_t interpolation_type;
    uint16_t global_sequence;
#ifdef LICH_KING
    M2Array timestampsLK; // M2Array<M2Array<uint32>
    M2Array valuesLK; // M2Array<M2Array<T>>
#else
    M2Array interpolation_ranges; //M2Array<pair<uint32>> min & max of what ?
    M2Array timestampsBC; // M2Array<uint32>
    M2Array valuesBC; // M2Array<T>
#endif
};

struct M2Camera
{
    uint32_t type; // 0: portrait, 1: characterinfo; -1: else (flyby etc.); referenced backwards in the lookup table.
    float fov; // No radians, no degrees. Multiply by 35 to get degrees.
    float far_clip;
    float near_clip;
    M2Track positions; // How the camera's position moves. Should be 3*3 floats.
    G3D::Vector3 position_base;
    M2Track target_positions; // How the target moves. Should be 3*3 floats.
    G3D::Vector3 target_position_base;
    M2Track rolldata; // The camera can have some roll-effect. Its 0 to 2*Pi.
};

struct FlyByCamera
{
    uint32 timeStamp;
    G3D::Vector4 locations;
};

struct SqlDbc
{
    std::string const* formatString;
    std::string const* indexName;
    std::string sqlTableName;
    int32 indexPos;
    int32 sqlIndexPos;
    SqlDbc(std::string const* _filename, std::string const* _format, std::string const* _idname, char const* fmt)
        : formatString(_format), indexName (_idname), sqlIndexPos(0)
    {
        // Convert dbc file name to sql table name
        sqlTableName = *_filename;
        for (char & i : sqlTableName)
        {
            if (isalpha(i))
                i = char(tolower(i));
            else if (i == '.')
                i = '_';
        }

        // Get sql index position
        DBCFileLoader::GetFormatRecordSize(fmt, &indexPos);
        if (indexPos >= 0)
        {
            uint32 uindexPos = uint32(indexPos);
            for (uint32 x = 0; x < formatString->size(); ++x)
            {
                // Count only fields present in sql
                if ((*formatString)[x] == FT_SQL_PRESENT)
                {
                    if (x == uindexPos)
                        break;
                    ++sqlIndexPos;
                }
            }
        }
    }

private:
    SqlDbc(SqlDbc const& right) = delete;
    SqlDbc& operator=(SqlDbc const& right) = delete;
};

template<class T>
class TC_GAME_API DBCStorage
{
    typedef std::list<char*> StringPoolList;
    public:
        explicit DBCStorage(char const* f)
            : fmt(f), nCount(0), fieldCount(0), dataTable(nullptr)
        {
            indexTable.asT = nullptr;
        }

        ~DBCStorage() { Clear(); }

        T const* LookupEntry(uint32 id) const
        {
            return (id >= nCount) ? nullptr : indexTable.asT[id];
        }

        uint32  GetNumRows() const { return nCount; }
        char const* GetFormat() const { return fmt; }
        uint32 GetFieldCount() const { return fieldCount; }

        bool Load(char const* fn, SqlDbc* sql)
        {
            DBCFileLoader dbc;
            // Check if load was sucessful, only then continue
            if (!dbc.Load(fn, fmt))
                return false;

            uint32 sqlRecordCount = 0;
            uint32 sqlHighestIndex = 0;
            Field* fields = nullptr;
            QueryResult result = QueryResult(nullptr);
            // Load data from sql
            if (sql)
            {
                std::string query = "SELECT * FROM " + sql->sqlTableName;
                if (sql->indexPos >= 0)
                    query +=" ORDER BY " + *sql->indexName + " DESC";
                query += ';';


                result = WorldDatabase.Query(query.c_str());
                if (result)
                {
                    sqlRecordCount = uint32(result->GetRowCount());
                    if (sql->indexPos >= 0)
                    {
                        fields = result->Fetch();
                        sqlHighestIndex = fields[sql->sqlIndexPos].GetUInt32();
                    }

                    // Check if sql index pos is valid
                    if (int32(result->GetFieldCount() - 1) < sql->sqlIndexPos)
                    {
                        TC_LOG_ERROR("server.loading", "Invalid index pos for dbc:'%s'", sql->sqlTableName.c_str());
                        return false;
                    }
                }
            }

            char* sqlDataTable = nullptr;
            fieldCount = dbc.GetCols();

            dataTable = reinterpret_cast<T*>(dbc.AutoProduceData(fmt, nCount, indexTable.asChar,
                sqlRecordCount, sqlHighestIndex, sqlDataTable));

            stringPoolList.push_back(dbc.AutoProduceStrings(fmt, reinterpret_cast<char*>(dataTable)));

            // Insert sql data into arrays
            if (result)
            {
                if (indexTable.asT)
                {
                    uint32 offset = 0;
                    uint32 rowIndex = dbc.GetNumRows();
                    do
                    {
                        if (!fields)
                            fields = result->Fetch();

                        if (sql->indexPos >= 0)
                        {
                            uint32 id = fields[sql->sqlIndexPos].GetUInt32();
                            if (indexTable.asT[id])
                            {
                                TC_LOG_ERROR("server.loading", "Index %d already exists in dbc:'%s'", id, sql->sqlTableName.c_str());
                                return false;
                            }

                            indexTable.asT[id] = reinterpret_cast<T*>(&sqlDataTable[offset]);
                        }
                        else
                            indexTable.asT[rowIndex]= reinterpret_cast<T*>(&sqlDataTable[offset]);

                        uint32 columnNumber = 0;
                        uint32 sqlColumnNumber = 0;

                        for (; columnNumber < sql->formatString->size(); ++columnNumber)
                        {
                            if ((*sql->formatString)[columnNumber] == FT_SQL_ABSENT)
                            {
                                switch (fmt[columnNumber])
                                {
                                    case FT_FLOAT:
                                        *reinterpret_cast<float*>(&sqlDataTable[offset]) = 0.0f;
                                        offset += 4;
                                        break;
                                    case FT_IND:
                                    case FT_INT:
                                        *reinterpret_cast<uint32*>(&sqlDataTable[offset]) = uint32(0);
                                        offset += 4;
                                        break;
                                    case FT_BYTE:
                                        *reinterpret_cast<uint8*>(&sqlDataTable[offset]) = uint8(0);
                                        offset += 1;
                                        break;
                                    case FT_STRING:
                                        // Beginning of the pool - empty string
                                        *reinterpret_cast<char**>(&sqlDataTable[offset]) = stringPoolList.back();
                                        offset += sizeof(char*);
                                        break;
                                }
                            }
                            else if ((*sql->formatString)[columnNumber] == FT_SQL_PRESENT)
                            {
                                bool validSqlColumn = true;
                                switch (fmt[columnNumber])
                                {
                                    case FT_FLOAT:
                                        *reinterpret_cast<float*>(&sqlDataTable[offset]) = fields[sqlColumnNumber].GetFloat();
                                        offset += 4;
                                        break;
                                    case FT_IND:
                                    case FT_INT:
                                        *reinterpret_cast<uint32*>(&sqlDataTable[offset]) = fields[sqlColumnNumber].GetUInt32();
                                        offset += 4;
                                        break;
                                    case FT_BYTE:
                                        *reinterpret_cast<uint8*>(&sqlDataTable[offset]) = fields[sqlColumnNumber].GetUInt8();
                                        offset += 1;
                                        break;
                                    case FT_STRING:
                                        TC_LOG_ERROR("server.loading", "Unsupported data type in table '%s' at char %d", sql->sqlTableName.c_str(), columnNumber);
                                        return false;
                                    case FT_SORT:
                                        break;
                                    default:
                                        validSqlColumn = false;
                                        break;
                                }
                                if (validSqlColumn && (columnNumber != (sql->formatString->size()-1)))
                                    sqlColumnNumber++;
                            }
                            else
                            {
                                TC_LOG_ERROR("server.loading", "Incorrect sql format string '%s' at char %d", sql->sqlTableName.c_str(), columnNumber);
                                return false;
                            }
                        }

                        if (sqlColumnNumber != (result->GetFieldCount() - 1))
                        {
                            TC_LOG_ERROR("server.loading", "SQL and DBC format strings are not matching for table: '%s'", sql->sqlTableName.c_str());
                            return false;
                        }

                        fields = nullptr;
                        ++rowIndex;
                    } while (result->NextRow());
                }
            }

            // error in dbc file at loading if NULL
            return indexTable.asT != NULL;
        }

        bool LoadStringsFrom(char const* fn)
        {
            // DBC must be already loaded using Load
            if (!indexTable.asT)
                return false;

            DBCFileLoader dbc;
            // Check if load was successful, only then continue
            if (!dbc.Load(fn, fmt))
                return false;

            stringPoolList.push_back(dbc.AutoProduceStrings(fmt, reinterpret_cast<char*>(dataTable)));

            return true;
        }

        void Clear()
        {
            if (!indexTable.asT)
                return;

            delete[] reinterpret_cast<char*>(indexTable.asT);
            indexTable.asT = NULL;
            delete[] reinterpret_cast<char*>(dataTable);
            dataTable = NULL;

            while (!stringPoolList.empty())
            {
                delete[] stringPoolList.front();
                stringPoolList.pop_front();
            }

            nCount = 0;
        }

    private:
        char const* fmt;
        uint32 nCount;
        uint32 fieldCount;

        union
        {
            T** asT;
            char** asChar;
        }
        indexTable;

        T* dataTable;
        StringPoolList stringPoolList;

        DBCStorage(DBCStorage const& right) = delete;
        DBCStorage& operator=(DBCStorage const& right) = delete;
};

typedef std::list<uint32> SimpleFactionsList;

typedef std::vector<FlyByCamera> FlyByCameraCollection;

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
TC_GAME_API extern std::unordered_map<uint32, FlyByCameraCollection> sFlyByCameraStore;

void LoadDBCStores(const std::string& dataPath);
//Disabled for now, need research
void LoadM2Cameras(const std::string& dataPath);

// script support functions
TC_GAME_API DBCStorage <SoundEntriesEntry>  const* GetSoundEntriesStore();
//TC_GAME_API DBCStorage <SpellInfo>         const* GetSpellStore();
TC_GAME_API DBCStorage <SpellRangeEntry>    const* GetSpellRangeStore();

#endif
