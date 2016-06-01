
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
        for (uint32 i = 0; i< sqlTableName.size(); ++i)
        {
            if (isalpha(sqlTableName[i]))
                sqlTableName[i] = char(tolower(sqlTableName[i]));
            else if (sqlTableName[i] == '.')
                sqlTableName[i] = '_';
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
class DBCStorage
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

                        fields = NULL;
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

SimpleFactionsList const* GetFactionTeamList(uint32 faction);
std::string GetPetName(uint32 petfamily, LocaleConstant dbclang);
uint32 GetTalentSpellCost(uint32 spellId);
TalentSpellPos const* GetTalentSpellPos(uint32 spellId);

WMOAreaTableEntry const* GetWMOAreaTableEntryByTripple(int32 rootid, int32 adtid, int32 groupid);

uint32 GetVirtualMapForMapAndZone(uint32 mapid, uint32 zoneId);

ChatChannelsEntry const* GetChannelEntryFor(uint32 channel_id);

bool IsTotemCategoryCompatibleWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId, Item* pItem);

void Zone2MapCoordinates(float& x,float& y,uint32 zone);
void Map2ZoneCoordinates(float& x,float& y,uint32 zone);

typedef std::map<uint32/*pair32(map, diff)*/, MapDifficulty> MapDifficultyMap;
MapDifficulty const* GetMapDifficultyData(uint32 mapId, Difficulty difficulty);
MapDifficulty const* GetDownscaledMapDifficultyData(uint32 mapId, Difficulty &difficulty);

typedef std::unordered_multimap<uint32, SkillRaceClassInfoEntry const*> SkillRaceClassInfoMap;
typedef std::pair<SkillRaceClassInfoMap::iterator, SkillRaceClassInfoMap::iterator> SkillRaceClassInfoBounds;
SkillRaceClassInfoEntry const* GetSkillRaceClassInfo(uint32 skill, uint8 race, uint8 class_);

uint32 GetTalentInspectBitPosInTab(uint32 talentId);
uint32 GetTalentTabInspectBitSize(uint32 talentTabId);
uint32 const* /*[3]*/ GetTalentTabPages(uint32 cls);

uint32 GetDefaultMapLight(uint32 mapId);

enum ContentLevels
{
    CONTENT_1_60 = 0,
    CONTENT_61_70,
    CONTENT_71_80
};
ContentLevels GetContentLevelsForMapAndZone(uint32 mapid, uint32 zoneId);

extern DBCStorage <AreaTableEntry>               sAreaTableStore;
extern DBCStorage <AreaTriggerEntry>             sAreaTriggerStore;
extern DBCStorage <AuctionHouseEntry>            sAuctionHouseStore;
extern DBCStorage <BankBagSlotPricesEntry>       sBankBagSlotPricesStore;
extern DBCStorage <BattlemasterListEntry>        sBattlemasterListStore;
extern DBCStorage <CinematicCameraEntry>         sCinematicCameraStore;
//extern DBCStorage <ChatChannelsEntry>           sChatChannelsStore; -- accessed using function, no usable index
extern DBCStorage <CharStartOutfitEntry>         sCharStartOutfitStore;
extern DBCStorage <CharTitlesEntry>              sCharTitlesStore;
extern DBCStorage <ChrClassesEntry>              sChrClassesStore;
extern DBCStorage <ChrRacesEntry>                sChrRacesStore;
extern DBCStorage <CreatureDisplayInfoEntry>     sCreatureDisplayInfoStore;
extern DBCStorage <CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore;
extern DBCStorage <CreatureFamilyEntry>          sCreatureFamilyStore;
extern DBCStorage <CreatureModelDataEntry>       sCreatureModelDataStore;
extern DBCStorage <CreatureSpellDataEntry>       sCreatureSpellDataStore;
extern DBCStorage <DurabilityCostsEntry>         sDurabilityCostsStore;
extern DBCStorage <DurabilityQualityEntry>       sDurabilityQualityStore;
extern DBCStorage <EmotesTextEntry>              sEmotesTextStore;
extern DBCStorage <FactionEntry>                 sFactionStore;
extern DBCStorage <FactionTemplateEntry>         sFactionTemplateStore;
extern DBCStorage <GameObjectDisplayInfoEntry>   sGameObjectDisplayInfoStore;
extern DBCStorage <GemPropertiesEntry>           sGemPropertiesStore;

extern DBCStorage <GtCombatRatingsEntry>         sGtCombatRatingsStore;
extern DBCStorage <GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore;
extern DBCStorage <GtChanceToMeleeCritEntry>     sGtChanceToMeleeCritStore;
extern DBCStorage <GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore;
extern DBCStorage <GtChanceToSpellCritEntry>     sGtChanceToSpellCritStore;
extern DBCStorage <GtOCTRegenHPEntry>            sGtOCTRegenHPStore;
//extern DBCStorage <GtOCTRegenMPEntry>            sGtOCTRegenMPStore; -- not used currently
extern DBCStorage <GtRegenHPPerSptEntry>         sGtRegenHPPerSptStore;
extern DBCStorage <GtRegenMPPerSptEntry>         sGtRegenMPPerSptStore;
extern DBCStorage <ItemEntry>                    sItemStore;
//extern DBCStorage <ItemDisplayInfoEntry>      sItemDisplayInfoStore; -- not used currently
//moved to db extern DBCStorage <ItemExtendedCostEntry>        sItemExtendedCostStore;
extern DBCStorage <ItemRandomPropertiesEntry>    sItemRandomPropertiesStore;
extern DBCStorage <ItemRandomSuffixEntry>        sItemRandomSuffixStore;
extern DBCStorage <ItemSetEntry>                 sItemSetStore;
extern DBCStorage <LiquidTypeEntry>              sLiquidTypeStore;
extern DBCStorage <LockEntry>                    sLockStore;
extern DBCStorage <MailTemplateEntry>            sMailTemplateStore;
extern DBCStorage <MapEntry>                     sMapStore;
//extern DBCStorage <MapDifficultyEntry>           sMapDifficultyStore; -- use GetMapDifficultyData insteed
extern MapDifficultyMap                          sMapDifficultyMap;
extern DBCStorage <QuestSortEntry>               sQuestSortStore;
extern DBCStorage <RandomPropertiesPointsEntry>  sRandomPropertiesPointsStore;
extern DBCStorage <SkillLineEntry>               sSkillLineStore;
extern DBCStorage <SkillLineAbilityEntry>        sSkillLineAbilityStore;
extern DBCStorage <SkillTiersEntry>              sSkillTiersStore;
extern DBCStorage <SkillRaceClassInfoEntry>      sSkillRaceClassInfoStore;
extern DBCStorage <SoundEntriesEntry>            sSoundEntriesStore;
extern DBCStorage <SpellCastTimesEntry>          sSpellCastTimesStore;
extern DBCStorage <SpellDurationEntry>           sSpellDurationStore;
extern DBCStorage <SpellFocusObjectEntry>        sSpellFocusObjectStore;
extern DBCStorage <SpellItemEnchantmentEntry>    sSpellItemEnchantmentStore;
extern DBCStorage <SpellItemEnchantmentConditionEntry> sSpellItemEnchantmentConditionStore;
extern DBCStorage <SpellCategoryEntry>           sSpellCategoryStore;
extern PetFamilySpellsStore                      sPetFamilySpellsStore;
extern SpellCategoryStore                        sSpellsByCategoryStore;
extern DBCStorage <SpellRadiusEntry>             sSpellRadiusStore;
extern DBCStorage <SpellRangeEntry>              sSpellRangeStore;
extern DBCStorage <SpellShapeshiftEntry>         sSpellShapeshiftStore;
//extern DBCStorage <SpellEntry>                   sSpellStore;
extern DBCStorage <StableSlotPricesEntry>        sStableSlotPricesStore;
extern DBCStorage <SummonPropertiesEntry>        sSummonPropertiesStore;
extern DBCStorage <TalentEntry>                  sTalentStore;
extern DBCStorage <TalentTabEntry>               sTalentTabStore;
extern DBCStorage <TaxiNodesEntry>               sTaxiNodesStore;
extern DBCStorage <TaxiPathEntry>                sTaxiPathStore;
extern TaxiMask                                  sTaxiNodesMask;
extern TaxiPathSetBySource                       sTaxiPathSetBySource;
extern TaxiPathNodesByPath                       sTaxiPathNodesByPath;
extern DBCStorage <TotemCategoryEntry>           sTotemCategoryStore;
extern DBCStorage <WMOAreaTableEntry>            sWMOAreaTableStore;
//extern DBCStorage <WorldMapAreaEntry>           sWorldMapAreaStore; -- use Zone2MapCoordinates and Map2ZoneCoordinates
extern DBCStorage <WorldSafeLocsEntry>           sWorldSafeLocsStore;
extern std::unordered_map<uint32, FlyByCameraCollection> sFlyByCameraStore;

void LoadDBCStores(const std::string& dataPath);
//Disabled for now, need research
void LoadM2Cameras(const std::string& dataPath);

// script support functions
DBCStorage <SoundEntriesEntry>  const* GetSoundEntriesStore();
//DBCStorage <SpellInfo>         const* GetSpellStore();
DBCStorage <SpellRangeEntry>    const* GetSpellRangeStore();

#endif
