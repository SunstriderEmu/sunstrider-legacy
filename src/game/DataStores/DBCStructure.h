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

#ifndef DBCSTRUCTURE_H
#define DBCSTRUCTURE_H

#include "DBCEnums.h"
#include "Define.h"

#include <map>
#include <set>
#include <vector>

// Structures using to access raw DBC data and required packing to portability

#pragma pack(push,1)

/* My current understanding of zones and areas :
Zone are as the usual the WoW map delimitations. General channels are per zone.
Areas are what pops on screen when you move in specific parts of a zone. An area can have a parent, on maximum one level (no parent has a parent).
A zone is actually a parent area, an area englobing all areas in a zone.

Be aware that there seems to be some confusion between "zones" and "area" in code
*/
struct AreaTableEntry
{
    uint32    ID;                                           // 0 area ID
    uint32    mapid;                                        // 1
    uint32    zone;                                         // 2 Parent zone. 0 if this is the parent zone
    uint32    exploreFlag;                                  // 3
    uint32    flags;                                        // 4, unknown value but 312 for all cities
                                                            // 5-9 unused
    int32     area_level;                                   // 10
    char*     area_name[16];                                // 11-26
                                                            // 27, string flags, unused
    uint32    team;                                         // 28
    uint32    LiquidTypeOverride[4];                        // 29-32 liquid override by type
    float     maxDepth;                                     // 33

    // helpers
    bool IsSanctuary() const
    {
        if (mapid == 609)
            return true;
        return (flags & AREA_FLAG_SANCTUARY);
    }
};

struct AreaTriggerEntry
{
    uint32    id;                                           // 0
    uint32    mapid;                                        // 1
    float     x;                                            // 2
    float     y;                                            // 3
    float     z;                                            // 4
    float     radius;                                       // 5
    float     box_x;                                        // 6 extent x edge
    float     box_y;                                        // 7 extent y edge
    float     box_z;                                        // 8 extent z edge
    float     box_orientation;                              // 9 extent rotation by about z axis
};

struct AuctionHouseEntry
{
    uint32 houseId; // 0 index
    uint32 faction; // 1 id of faction.dbc for player factions associated with city
    uint32 depositPercent; // 2 1/3 from real
    uint32 cutPercent; // 3
    //char* name[16]; // 4-19
                                                            // 20 string flag, unused
};

struct BankBagSlotPricesEntry
{
    uint32      ID;
    uint32      price;
};

struct BattlemasterListEntry
{
    uint32      id;                                         // 0
    uint32      mapid[3];                                   // 1-3 mapid
                                                            // 4-8 unused
    uint32      type;                                       // 9 (3 - BG, 4 - arena)
    uint32      minlvl;                                     // 10
    uint32      maxlvl;                                     // 11
    uint32      maxplayersperteam;                          // 12
                                                            // 13-14 unused
    char*       name[16];                                   // 15-30
                                                            // 31 string flag, unused
                                                            // 32 unused
};

#define MAX_OUTFIT_ITEMS 12
// #define MAX_OUTFIT_ITEMS 24                              // 12->24 in 3.0.x

struct CharStartOutfitEntry
{
    //uint32 Id;                                            // 0
    uint32 RaceClassGender;                                 // 1 (UNIT_FIELD_BYTES_0 & 0x00FFFFFF) comparable (0 byte = race, 1 byte = class, 2 byte = gender)
    int32 ItemId[MAX_OUTFIT_ITEMS];                         // 2-13
    //int32 ItemDisplayId[MAX_OUTFIT_ITEMS];                // 14-25 not required at server side
    //int32 ItemInventorySlot[MAX_OUTFIT_ITEMS];            // 26-37 not required at server side
    //int32 InventoryType[MAX_OUTFIT_ITEMS]                 // 38-47 not sure
    //uint32 PetDisplayID;                                  // 39
    //uint32 PetFamilyID;                                   // 40
};

struct CharTitlesEntry
{
    uint32      ID;                                         // 0, title ids, for example in Quest::GetCharTitleId()
    //uint32      ConditionID;                              // 1
    //char*       name[16];                                 // 2-17, unused
                                                            // 18 string flag, unused
    //char*       name2[16];                                // 19-34, unused
                                                            // 35 string flag, unused
    uint32      bit_index;                                  // 36 used in PLAYER_CHOSEN_TITLE and 1<<index in PLAYER__FIELD_KNOWN_TITLES
};

struct ChatChannelsEntry
{
    uint32      ChannelID;                                  // 0
    uint32      flags;                                      // 1
    char*       pattern[16];                                // 3-18
                                                            // 19 string flags, unused
    //char*       name[16];                                 // 20-35 unused
                                                            // 36 string flag, unused
};

struct ChrClassesEntry
{
    uint32      ClassID;                                    // 0
                                                            // 1-2, unused
    uint32      PowerType;                                  // 3
                                                            // 4, unused
    //char*       name[16];                                 // 5-20 unused
    char*       name[16];                                   // 5-20 unused
                                                            // 21 string flag, unused
    //char*       nameFemale[16];                           // 21-36 unused, if different from base (male) case
                                                            // 37 string flag, unused
    //char*       nameNeutralGender[16];                    // 38-53 unused, if different from base (male) case
                                                            // 54 string flag, unused
                                                            // 55, unused
    uint32      spellfamily;                                // 56
                                                            // 57, unused
};

struct ChrRacesEntry
{
    uint32      RaceID;                                     // 0
    uint32      Flags;                                      // 1
    uint32      FactionID;                                  // 2 facton template id
                                                            // 3 unused
    uint32      model_m;                                    // 4
    uint32      model_f;                                    // 5
                                                            // 6-7 unused
    uint32      TeamID;                                     // 8 (7-Alliance 1-Horde)
                                                            // 9-12 unused
    uint32      startmovie;                                 // 13 id from CinematicCamera.dbc
    char*       name[16];                                   // 14-29 used for DBC language detection/selection
                                                            // 30 string flags, unused
    //char*       nameFemale[16];                           // 31-46, if different from base (male) case
                                                            // 47 string flags, unused
    //char*       nameNeutralGender[16];                    // 48-63, if different from base (male) case
                                                            // 64 string flags, unused
                                                            // 65-67 unused
    uint32      addon;                                      // 68 (0 - original race, 1 - tbc addon, ...)
};

struct CinematicCameraEntry
{
    uint32      id;                                         // 0 index
    char*       filename;                                   // 1
    uint32      soundid;                                    // 2 in SoundEntries.dbc or 0
    float       base_x;                                     // 3
    float       base_y;                                     // 4
    float       base_z;                                     // 5
    float       base_o;                                     // 6
};

struct CinematicSequencesEntry
{
    uint32      Id;                                         // 0 index
                                                            //uint32      unk1;                                     // 1 always 0
    uint32      cinematicCamera;                            // 2 id in CinematicCamera.dbc
                                                            // 3-9 always 0
};

struct CreatureDisplayInfoEntry
{
    uint32      Displayid;                                  // 0        m_ID
    uint32      ModelId;                                    // 1        m_modelID
                                                            // 2        m_soundID
    uint32      ExtraId;                                    // 3        m_extendedDisplayInfoID
    float       scale;                                      // 4        m_creatureModelScale
                                                            // 5-11,unused
};

//from LK, not sure it's bc format too
struct CreatureDisplayInfoExtraEntry
{
    //uint32 Id;                                            // 0
    uint32 Race;                                            // 1
    uint32 Gender;                                          // 2
    //uint32 SkinColor;                                     // 3
    //uint32 FaceType;                                      // 4
    //uint32 HairType;                                      // 5
    //uint32 HairStyle;                                     // 6
    //uint32 FacialHair;                                    // 7
    //uint32 HelmDisplayId;                                 // 8
    //uint32 ShoulderDisplayId;                             // 9
    //uint32 ShirtDisplayId;                                // 10
    //uint32 ChestDisplayId;                                // 11
    //uint32 BeltDisplayId;                                 // 12
    //uint32 LegsDisplayId;                                 // 13
    //uint32 BootsDisplayId;                                // 14
    //uint32 WristDisplayId;                                // 15
    //uint32 GlovesDisplayId;                               // 16
    //uint32 TabardDisplayId;                               // 17
    //uint32 CloakDisplayId;                                // 18
    //uint32 CanEquip;                                      // 19
    //char const* Texture;                                  // 20
};

//TODOMOV confirm BC format
struct CreatureModelDataEntry
{
    uint32 Id;
    uint32 Flags;
    //char* ModelPath[16]
    //uint32 Unk1;
    float Scale;                                             // Used in calculation of unit collision data
    //int32 Unk2
    //int32 Unk3
    //uint32 Unk4
    //uint32 Unk5
    //float Unk6
    //uint32 Unk7
    //float Unk8
    //uint32 Unk9 
    //uint32 Unk10 //maybe SoundID
    //float CollisionWidth;
    float CollisionHeight;
    float MountHeight;                                       // Used in calculation of unit collision data when mounted
    //float Unks[11]
};

struct CreatureFamilyEntry
{
    uint32    ID;                                           // 0
    float     minScale;                                     // 1
    uint32    minScaleLevel;                                // 2 0/1
    float     maxScale;                                     // 3
    uint32    maxScaleLevel;                                // 4 0/60
    uint32    skillLine[2];                                 // 5-6
    uint32    petFoodMask;                                  // 7
    char*     Name[16];                                     // 8-23
                                                            // 24 string flags, unused
                                                            // 25 icon, unused
};

struct CreatureSpellDataEntry
{
    uint32    ID;                                           // 0
    //uint32    spellId[4];                                 // 1-4 hunter pet learned spell (for later use)
};

struct DurabilityCostsEntry
{
    uint32    Itemlvl;                                      // 0
    uint32    multiplier[29];                               // 1-29
};

struct DurabilityQualityEntry
{
    uint32    Id;                                           // 0
    float     quality_mod;                                  // 1
};

struct EmotesTextEntry
{
    uint32    Id;
    uint32    textid;
};

struct FactionEntry
{
    uint32      ID;                                         // 0
    int32       reputationListID;                           // 1
    uint32      BaseRepRaceMask[4];                         // 2-5 Base reputation race masks (see enum Races)
    uint32      BaseRepClassMask[4];                        // 6-9 Base reputation class masks (see enum Classes)
    int32       BaseRepValue[4];                            // 10-13 Base reputation values
    uint32      ReputationFlags[4];                         // 14-17 Default flags to apply
    uint32      team;                                       // 18 enum Team
    char*       name[16];                                   // 19-34
                                                            // 35 string flags, unused
    //char*     description[16];                            // 36-51 unused
                                                            // 52 string flags, unused

                                                            // helpers
    bool CanHaveReputation() const
    {
        return reputationListID >= 0;
    }
};

struct FactionTemplateEntry
{
    uint32      ID;                                         // 0
    uint32      faction;                                    // 1
    uint32      factionFlags;                               // 2 specific flags for that faction
    uint32      ourMask;                                    // 3 if mask set (see FactionMasks) then faction included in masked team
    uint32      friendlyMask;                               // 4 if mask set (see FactionMasks) then faction friendly to masked team
    uint32      hostileMask;                                // 5 if mask set (see FactionMasks) then faction hostile to masked team
    uint32      enemyFaction1;                              // 6
    uint32      enemyFaction2;                              // 7
    uint32      enemyFaction3;                              // 8
    uint32      enemyFaction4;                              // 9
    uint32      friendFaction1;                             // 10
    uint32      friendFaction2;                             // 11
    uint32      friendFaction3;                             // 12
    uint32      friendFaction4;                             // 13
    //-------------------------------------------------------  end structure

    // helpers
    bool IsFriendlyTo(FactionTemplateEntry const& entry) const
    {
        if(ID == entry.ID)
            return true;
        if(enemyFaction1  == entry.faction || enemyFaction2  == entry.faction || enemyFaction3 == entry.faction || enemyFaction4 == entry.faction )
            return false;
        if(friendFaction1 == entry.faction || friendFaction2 == entry.faction || friendFaction3 == entry.faction || friendFaction4 == entry.faction )
            return true;
        return (friendlyMask & entry.ourMask) || (ourMask & entry.friendlyMask);
    }
    bool IsHostileTo(FactionTemplateEntry const& entry) const
    {
        if(ID == entry.ID)
            return false;
        if(enemyFaction1  == entry.faction || enemyFaction2  == entry.faction || enemyFaction3 == entry.faction || enemyFaction4 == entry.faction )
            return true;
        if(friendFaction1 == entry.faction || friendFaction2 == entry.faction || friendFaction3 == entry.faction || friendFaction4 == entry.faction )
            return false;
        return (hostileMask & entry.ourMask) != 0;
    }
    bool IsHostileToPlayers() const { return (hostileMask & FACTION_MASK_PLAYER) !=0; }
    bool IsNeutralToAll() const { return hostileMask == 0 && friendlyMask == 0 && enemyFaction1==0 && enemyFaction2==0 && enemyFaction3==0 && enemyFaction4==0; }
    bool IsContestedGuardFaction() const { return (factionFlags & FACTION_TEMPLATE_FLAG_CONTESTED_GUARD)!=0; }
};

struct GameObjectDisplayInfoEntry
{
    uint32      Displayid;                                  // 0        m_ID
    char* filename;                                         // 1
    //uint32  unk1[10];   //2-11 //Sound ?
    float   minX;
    float   minY;
    float   minZ;
    float   maxX;
    float   maxY;
    float   maxZ;
};

struct GemPropertiesEntry
{
    uint32      ID;
    uint32      spellitemenchantement;
    uint32      color;
};

// All Gt* DBC store data for 100 levels, some by 100 per class/race
#define GT_MAX_LEVEL    100

struct GtCombatRatingsEntry
{
    float    ratio;
};

struct GtChanceToMeleeCritBaseEntry
{
    float    base;
};

struct GtChanceToMeleeCritEntry
{
    float    ratio;
};

struct GtChanceToSpellCritBaseEntry
{
    float    base;
};

struct GtChanceToSpellCritEntry
{
    float    ratio;
};

struct GtOCTRegenHPEntry
{
    float    ratio;
};

//struct GtOCTRegenMPEntry
//{
//    float    ratio;
//};

struct GtRegenHPPerSptEntry
{
    float    ratio;
};

struct GtRegenMPPerSptEntry
{
    float    ratio;
};

struct ItemEntry
{
   uint32 ID;
#ifdef LICH_KING
   uint32 Class;                                          // 1
   uint32 SubClass;                                       // 2 some items have strange subclasses
   int32 SoundOverrideSubclass;                          // 3
   int32 Material;                                       // 4
#endif
   uint32 DisplayId;
   uint32 InventoryType;
   uint32 Sheath;
};

struct ItemDisplayInfoEntry
{
    uint32      ID;
    uint32      randomPropertyChance;
};

//struct ItemCondExtCostsEntry
//{
//    uint32      ID;
//    uint32      condExtendedCost;                           // ItemTemplate::CondExtendedCost
//    uint32      itemextendedcostentry;                      // ItemTemplate::ExtendedCost
//    uint32      arenaseason;                                // arena season number(1-4)
//};

struct ItemExtendedCostEntry
{
    //uint32      ID;                                         // 0 extended-cost entry id
    uint32      reqhonorpoints;                             // 1 required honor points
    uint32      reqarenapoints;                             // 2 required arena points
    uint32      reqitem[5];                                 // 3-7 required item id
    uint32      reqitemcount[5];                            // 8-12 required count of 1st item
    uint32      reqpersonalarenarating;                     // 13 required personal arena rating
};

struct ItemRandomPropertiesEntry
{
    uint32    ID;                                           // 0
    //char*     internalName                                // 1   unused
    uint32    enchant_id[3];                                // 2-4
                                                            // 5-6 unused, 0 only values, reserved for additional enchantments?
    //char*     nameSuffix[16]                              // 7-22, unused
                                                            // 23 nameSufix flags, unused
};

struct ItemRandomSuffixEntry
{
    uint32    ID;                                           // 0
    //char*     name[16]                                    // 1-16 unused
                                                            // 17, name flags, unused
                                                            // 18  unused
    uint32    enchant_id[3];                                // 19-21
    uint32    prefix[3];                                    // 22-24
};

struct ItemSetEntry
{
    //uint32    id                                          // 0 item set ID
    char*     name[16];                                     // 1-16
                                                            // 17 string flags, unused
                                                            // 18-28 items from set, but not have all items listed, use ItemTemplate::ItemSet instead
                                                            // 29-34 unused
    uint32    spells[8];                                    // 35-42
    uint32    items_to_triggerspell[8];                     // 43-50
    uint32    required_skill_id;                            // 51
    uint32    required_skill_value;                         // 52
};

#define MAX_LOCK_CASE 8

struct LockEntry
{
    uint32      ID;                                         // 0
    uint32      Type[MAX_LOCK_CASE];                                 // 1-8 old "keytype"
    uint32      Index[MAX_LOCK_CASE];                                     // 9-16 old "key"
    uint32      requiredminingskill;                        // 17
    uint32      requiredlockskill;                          // 18
                                                            // 19-32, not used
};

struct MailTemplateEntry
{
    uint32      ID;                                         // 0
    //char*       subject[16];                              // 1-16
                                                            // 17 name flags, unused
    //char*       content[16];                              // 18-33
};

struct MapEntry
{
    uint32      MapID;                                      // 0
    //char*       internalname;                             // 1 unused
    uint32      map_type;                                   // 2
                                                            // 3 unused
    char*       name[16];                                   // 4-19
                                                            // 20 name flags, unused
                                                            // 21-23 unused (something PvPZone related - levels?)
                                                            // 24-26
    uint32      linked_zone;                                // 27 common zone for instance and continent map
    //char*     hordeIntro                                  // 28-43 text for PvP Zones
                                                            // 44 intro text flags
    //char*     allianceIntro                               // 45-60 text for PvP Zones
                                                            // 46 intro text flags
                                                            // 47-61 not used
    uint32      multimap_id;                                // 62
                                                            // 63-65 not used
    //chat*     unknownText1                                // 66-81 unknown empty text fields, possible normal Intro text.
                                                            // 82 text flags
    //chat*     heroicIntroText                             // 83-98 heroic mode requirement text
                                                            // 99 text flags
    //chat*     unknownText2                                // 100-115 unknown empty text fields
                                                            // 116 text flags
    int32       entrance_map;                               // 117 map_id of entrance map
    float       entrance_x;                                 // 118 entrance x coordinate (if exist single entry)
    float       entrance_y;                                 // 119 entrance y coordinate (if exist single entry)
    uint32 resetTimeRaid;                                   // 120
    uint32 resetTimeHeroic;                                 // 121
                                                            // 122-123
    uint32      addon;                                      // 124 (0-original maps,1-tbc addon)

    // Helpers
    uint32 Expansion() const { return addon; }

    bool IsWorldMap() const { return map_type == MAP_COMMON; }
    bool IsDungeon() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID; }
    bool IsNonRaidDungeon() const { return map_type == MAP_INSTANCE; }
    bool Instanceable() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID || map_type == MAP_BATTLEGROUND || map_type == MAP_ARENA; }
    bool IsRaid() const { return map_type == MAP_RAID; }
    bool IsBattleground() const { return map_type == MAP_BATTLEGROUND; }
    bool IsBattleArena() const { return map_type == MAP_ARENA; }
    bool IsBattlegroundOrArena() const { return map_type == MAP_BATTLEGROUND || map_type == MAP_ARENA; }
    bool HasResetTime() const { return resetTimeHeroic || resetTimeRaid; }

    bool GetEntrancePos(int32 &mapid, float &x, float &y) const
    {
        if (entrance_map < 0)
            return false;
        mapid = entrance_map;
        x = entrance_x;
        y = entrance_y;
        return true;
    }

    bool IsMountAllowed() const
    {
        return !IsDungeon() ||
            MapID==209 || MapID==269 || MapID==309 || // TanarisInstance, CavernsOfTime, Zul'gurub
            MapID==509 || MapID==534 || MapID==560 || // AhnQiraj, HyjalPast, HillsbradPast
            MapID==568 || MapID==580;                 // ZulAman, Sunwell Plateau
    }
};

struct QuestSortEntry
{
    uint32      id;                                         // 0, sort id
    //char*       name[16];                                 // 1-16, unused
                                                            // 17 name flags, unused
};

struct RandomPropertiesPointsEntry
{
    //uint32  Id;                                           // 0 hidden key
    uint32    itemLevel;                                    // 1
    uint32    EpicPropertiesPoints[5];                      // 2-6
    uint32    RarePropertiesPoints[5];                      // 7-11
    uint32    UncommonPropertiesPoints[5];                  // 12-16
};

//struct SkillLineCategoryEntry{
//    uint32    id;                                           // 0 hidden key
//    char*     name[16];                                     // 1 - 17 Category name
//                                                                  // 18 string flag
//    uint32    displayOrder;                                 // Display order in character tab
//};

//struct SkillRaceClassInfoEntry{
//    uint32    id;                                           // 0
//    uint32    skillId;                                      // 1 present some refrences to unknown skill
//    uint32    raceMask;                                     // 2
//    uint32    classMask;                                    // 3
//    uint32    flags;                                        // 4 mask for some thing
//    uint32    reqLevel;                                     // 5
//    uint32    skillTierId;                                  // 6
//    uint32    skillCostID;                                  // 7
//};

//struct SkillTiersEntry{
//    uint32    id;                                           // 0
//    uint32    skillValue[16];                               // 1-17 unknown possibly add value on learn?
//    uint32    maxSkillValue[16];                            // Max value for rank
//};

struct SkillLineEntry
{
    uint32    id;                                           // 0
    uint32    categoryId;                                   // 1 (index from SkillLineCategory.dbc)
    //uint32    skillCostID;                                // 2 not used
    char*     name[16];                                     // 3-18
                                                            // 19 string flags, not used
    //char*     description[16];                            // 20-35, not used
                                                            // 36 string flags, not used
    uint32    spellIcon;                                    // 37
};

struct SkillLineAbilityEntry
{
    uint32    id;                                           // 0, INDEX
    uint32    skillId;                                      // 1
    uint32    spellId;                                      // 2
    uint32    racemask;                                     // 3
    uint32    classmask;                                    // 4
    //uint32    racemaskNot;                                // 5 always 0 in 2.4.2
    //uint32    classmaskNot;                               // 6 always 0 in 2.4.2
    uint32    req_skill_value;                              // 7 for trade skill.not for training.
    uint32    forward_spellid;                              // 8
    uint32    AutolearnType;                                // 9 can be 1 or 2 for spells learned on get skill
    uint32    max_value;                                    // 10
    uint32    min_value;                                    // 11
                                                            // 12-13, unknown, always 0 (may be NumSkillUps + UniqueBit)
    uint32    reqtrainpoints;                               // 14
};

#define MAX_SKILL_STEP 16

struct SkillTiersEntry
{
    uint32 Id;                                              // 0
                                                            //uint32 StepCost[MAX_SKILL_STEP];                      // 1-16
    uint32 MaxSkill[MAX_SKILL_STEP];                        // 17-32
};

//OK BC & LK
struct SkillRaceClassInfoEntry
{
    //uint32 Id;                                            // 0
    uint32 SkillId;                                         // 1
    uint32 RaceMask;                                        // 2
    uint32 ClassMask;                                       // 3
    uint32 Flags;                                           // 4
                                                            //uint32 MinLevel;                                      // 5
    uint32 SkillTier;                                       // 6
                                                            //uint32 SkillCostType;                                 // 7
};

struct SoundEntriesEntry
{
    uint32    Id;                                           // 0        m_ID
    //uint32    Type;                                       // 1        m_soundType
    //char*     InternalName;                               // 2        m_name
    //char*     FileName[10];                               // 3-12     m_File[10]
    //uint32    Freq[10];                                   // 13-22    m_Freq[10]
    //char*     Path;                                       // 23       m_DirectoryBase
                                                            // 24       m_volumeFloat
                                                            // 25       m_flags
                                                            // 26       m_minDistance
                                                            // 27       m_distanceCutoff
                                                            // 28       m_EAXDef
                                                            // 29       new in 3.1
};

#define MAX_SPELL_EFFECTS 3
#define MAX_EFFECT_MASK 7
#define MAX_SPELL_REAGENTS 8

struct SpellEntry
{
    uint32    Id;                                           // 0 normally counted from 0 field (but some tools start counting from 1, check this before tool use for data view!)
    uint32    Category;                                     // 1
    //uint32     castUI                                     // 2 not used
    uint32    Dispel;                                       // 3
    uint32    Mechanic;                                     // 4
    uint32    Attributes;                                   // 5
    uint32    AttributesEx;                                 // 6
    uint32    AttributesEx2;                                // 7
    uint32    AttributesEx3;                                // 8
    uint32    AttributesEx4;                                // 9
    uint32    AttributesEx5;                                // 10
    uint32    AttributesEx6;                                // 11
    uint32    Stances;                                      // 12
    uint32    StancesNot;                                   // 13
    uint32    Targets;                                      // 14
    uint32    TargetCreatureType;                           // 15
    uint32    RequiresSpellFocus;                           // 16
    uint32    FacingCasterFlags;                            // 17
    uint32    CasterAuraState;                              // 18
    uint32    TargetAuraState;                              // 19
    uint32    CasterAuraStateNot;                           // 20
    uint32    TargetAuraStateNot;                           // 21
    uint32    CastingTimeIndex;                             // 22
    uint32    RecoveryTime;                                 // 23
    uint32    CategoryRecoveryTime;                         // 24
    uint32    InterruptFlags;                               // 25
    uint32    AuraInterruptFlags;                           // 26
    uint32    ChannelInterruptFlags;                        // 27
    uint32    ProcFlags;                                    // 28
    uint32    procChance;                                   // 29
    uint32    procCharges;                                  // 30
    uint32    MaxLevel;                                     // 31
    uint32    BaseLevel;                                    // 32
    uint32    SpellLevel;                                   // 33
    uint32    DurationIndex;                                // 34
    uint32    PowerType;                                    // 35
    uint32    ManaCost;                                     // 36
    uint32    ManaCostPerlevel;                             // 37
    uint32    manaPerSecond;                                // 38
    uint32    ManaPerSecondPerLevel;                        // 39
    uint32    rangeIndex;                                   // 40
    float     speed;                                        // 41
    //uint32    modalNextSpell;                             // 42
    uint32    StackAmount;                                  // 43
    uint32    Totem[2];                                     // 44-45
    int32     Reagent[8];                                   // 46-53
    uint32    ReagentCount[8];                              // 54-61
    int32     EquippedItemClass;                            // 62 (value)
    int32     EquippedItemSubClassMask;                     // 63 (mask)
    int32     EquippedItemInventoryTypeMask;                // 64 (mask)
    uint32    Effect[3];                                    // 65-67
    int32     EffectDieSides[3];                            // 68-70
    int32     EffectBaseDice[3];                            // 71-73
    float     EffectDicePerLevel[3];                        // 74-76
    float     EffectRealPointsPerLevel[3];                  // 77-79
    int32     EffectBasePoints[3];                          // 80-82 (don't must be used in spell/auras explicitly, must be used cached Spell::m_currentBasePoints)
    uint32    EffectMechanic[3];                            // 83-85
    uint32    EffectImplicitTargetA[3];                     // 86-88
    uint32    EffectImplicitTargetB[3];                     // 89-91
    uint32    EffectRadiusIndex[3];                         // 92-94 - spellradius.dbc
    uint32    EffectApplyAuraName[3];                       // 95-97
    uint32    EffectAmplitude[3];                           // 98-100
    float     EffectValueMultiplier[3];                       // 101-103
    uint32    EffectChainTarget[3];                         // 104-106
    uint32    EffectItemType[3];                            // 107-109
    int32     EffectMiscValue[3];                           // 110-112
    int32     EffectMiscValueB[3];                          // 113-115
    uint32    EffectTriggerSpell[3];                        // 116-118
    float     EffectPointsPerComboPoint[3];                 // 119-121
    uint32    SpellVisual;                                  // 122
                                                            // 123 not used
    uint32    SpellIconID;                                  // 124
    uint32    activeIconID;                                 // 125
    //uint32    spellPriority;                              // 126
    char*     SpellName[16];                                // 127-142
    //uint32    SpellNameFlag;                              // 143
    char*     Rank[16];                                     // 144-159
    //uint32    RankFlags;                                  // 160
    //char*     Description[16];                            // 161-176 not used
    //uint32    DescriptionFlags;                           // 177     not used
    //char*     ToolTip[16];                                // 178-193 not used
    //uint32    ToolTipFlags;                               // 194 not used
    uint32    ManaCostPercentage;                           // 195
    uint32    StartRecoveryCategory;                        // 196
    uint32    StartRecoveryTime;                            // 197
    uint32    MaxTargetLevel;                               // 198
    uint32    SpellFamilyName;                              // 199
    uint64    SpellFamilyFlags;                             // 200+201
    uint32    MaxAffectedTargets;                           // 202
    uint32    DmgClass;                                     // 203 defenseType
    uint32    PreventionType;                               // 204
    //uint32    StanceBarOrder;                             // 205 not used
    float     EffectDamageMultiplier[3];                    // 206-208
    //uint32    MinFactionId;                               // 209 not used, and 0 in 2.4.2
    //uint32    MinReputation;                              // 210 not used, and 0 in 2.4.2
    //uint32    RequiredAuraVision;                         // 211 not used
    uint32    TotemCategory[2];                             // 212-213
    uint32    AreaId;                                       // 214
    uint32    SchoolMask;                                   // 215 school mask
    uint32    CustomAttributesFlags;                        // not in dbc, custom field for enum SpellCustomAttributes

    public:
        SpellEntry()
        {
            for (int i = 0; i < 16; i++) {
                SpellName[i] = new char[128];
                Rank[i] = new char[128];
            }
        }
        
        ~SpellEntry()
        {
            for (int i = 0; i < 16; i++) {
                delete[] SpellName[i];
                delete[] Rank[i];
            }
        }

    private:
        // prevent creating custom entries (copy data from original in fact)
        SpellEntry(SpellEntry const&);                      // DON'T must have implementation
};

typedef std::set<uint32> SpellCategorySet;
typedef std::map<uint32,SpellCategorySet > SpellCategoryStore;
typedef std::set<uint32> PetFamilySpellsSet;
typedef std::map<uint32,PetFamilySpellsSet > PetFamilySpellsStore;

struct SpellCastTimesEntry
{
    uint32    ID;                                           // 0
    int32     CastTime;                                     // 1
    //float     CastTimePerLevel;                           // 2 unsure / per skill?
    //int32     MinCastTime;                                // 3 unsure
};

struct SpellCategoryEntry
{
    uint32 Id;
    uint32 Flags;
};

struct SpellFocusObjectEntry
{
    uint32    ID;                                           // 0
    //char*     Name[16];                                   // 1-15 unused
                                                            // 16 string flags, unused
};

// stored in SQL table
struct SpellThreatEntry
{
    uint32      spellId;
    int32       flatMod;
    float       pctMod;
};

struct SpellRadiusEntry
{
    uint32    ID;
    float     RadiusMin;
    float     RadiusMax;
};

struct SpellRangeEntry
{
    uint32    ID;
    float     minRange;
    float     maxRange;
    uint32    type;
};

struct SpellShapeshiftEntry
{
    uint32 ID;                                              // 0
    //uint32 buttonPosition;                                // 1 unused
    //char*  Name[16];                                      // 2-17 unused
    //uint32 NameFlags;                                     // 18 unused
    uint32 flags1;                                          // 19
    int32  creatureType;                                    // 20 <=0 humanoid, other normal creature types
    //uint32 AttackIconID;                                  // 21 unused
    uint32 attackSpeed;                                     // 22
    /* In 4.x, the next ones are :
    public int[4] CreatureDisplayID;
    public int[8] PresetSpellID;
    public int MountTypeID;
    public int ExitSoundEntriesID;
    */
    //uint32 modelID;                                       // 23 unused, alliance modelid (where horde case?)
    //uint32 unk2;                                          // 24 unused
    //uint32 unk3;                                          // 25 unused
    //uint32 unk4;                                          // 26 unused
    //uint32 unk5;                                          // 27 unused
    //uint32 unk6;                                          // 28 unused
    //uint32 unk7;                                          // 29 unused
    //uint32 unk8;                                          // 30 unused
    //uint32 unk9;                                          // 31 unused
    //uint32 unk10;                                         // 32 unused
    //uint32 unk11;                                         // 33 unused
    //uint32 unk12;                                         // 34 unused
};

struct SpellDurationEntry
{
    uint32    ID;
    int32     Duration[3];
};

struct SpellItemEnchantmentEntry
{
    uint32      ID;                                         // 0
    uint32      type[3];                                    // 1-3
    uint32      amount[3];                                  // 4-6
    //uint32    amount2[3]                                  // 7-9 always same as similar `amount` value
    uint32      spellid[3];                                 // 10-12
    char*       description[16];                            // 13-29
                                                            // 30 description flags
    uint32      aura_id;                                    // 31
    uint32      slot;                                       // 32
    uint32      GemID;                                      // 33
    uint32      EnchantmentCondition;                       // 34
};

struct SpellItemEnchantmentConditionEntry
{
    uint32  ID;
    uint8   Color[5];
    uint8   Comparator[5];
    uint8   CompareColor[5];
    uint32  Value[5];
};

struct StableSlotPricesEntry
{
    uint32 Slot;
    uint32 Price;
};

#define MAX_TALENT_RANK 5
#define MAX_TALENT_TABS 3

struct TalentEntry
{
    uint32    TalentID;                                     // 0
    uint32    TalentTab;                                    // 1 index in TalentTab.dbc (TalentTabEntry)
    uint32    Row;                                          // 2
    uint32    Col;                                          // 3
    uint32    RankID[5];                                    // 4-8
                                                            // 9-12 not used, always 0, maybe not used high ranks
    uint32    DependsOn;                                    // 13 index in Talent.dbc (TalentEntry)
                                                            // 14-15 not used
    uint32    DependsOnRank;                                // 16
                                                            // 17-19 not used
    uint32    DependsOnSpell;                               // 20 req.spell
};

struct TalentTabEntry
{
    uint32    TalentTabID;                                  // 0
    //char*   name[16];                                     // 1-16, unused
    //uint32  nameFlags;                                    // 17, unused
    //unit32  spellicon;                                    // 18
                                                            // 19 not used
    uint32    ClassMask;                                    // 20
    uint32    tabpage;                                      // 21
    //char*   internalname;                                 // 22
};

struct TaxiNodesEntry
{
    uint32    ID;                                           // 0
    uint32    map_id;                                       // 1
    float     x;                                            // 2
    float     y;                                            // 3
    float     z;                                            // 4
    char*     name[16];                                     // 5-21
                                                            // 22 string flags, unused
    uint32    MountCreatureID[2];                           // 23-24    m_MountCreatureID[2]
};

struct TaxiPathEntry
{
    uint32    ID;
    uint32    from;
    uint32    to;
    uint32    price;
};

struct TaxiPathNodeEntry
{
                                                            // 0        m_ID
    uint32    PathID;                                       // 1        m_PathID
    uint32    NodeIndex;                                    // 2        m_NodeIndex
    uint32    MapID;                                        // 3        m_ContinentID
    float     LocX;                                            // 4        m_LocX
    float     LocY;                                            // 5        m_LocY
    float     LocZ;                                            // 6        m_LocZ
    uint32    actionFlag;                                   // 7        m_flags
    uint32    delay;                                        // 8        m_delay
    uint32    arrivalEventID;                               // 9        m_arrivalEventID
    uint32    departureEventID;                             // 10       m_departureEventID
};

struct TotemCategoryEntry
{
    uint32    ID;                                           // 0
    //char*   name[16];                                     // 1-16
                                                            // 17 string flags, unused
    uint32    categoryType;                                 // 18 (one for specialization)
    uint32    categoryMask;                                 // 19 (compatibility mask for same type: different for totems, compatible from high to low for rods)
};

struct TransportAnimationEntry
{
    //uint32  Id;
    uint32  TransportEntry;
    uint32  TimeSeg;
    float   X;
    float   Y;
    float   Z;
    //uint32  MovementId;
};

struct WMOAreaTableEntry
{
    uint32 Id;                                              // 0 index
    int32 rootId;                                           // 1 used in root WMO
    int32 adtId;                                            // 2 used in adt file
    int32 groupId;                                          // 3 used in group WMO
    //uint32 field4;
    //uint32 field5;
    //uint32 field6;
    //uint32 field7;
    //uint32 field8;
    uint32 Flags;                                           // 9 used for indoor/outdoor determination
    uint32 areaId;                                          // 10 link to AreaTableEntry.ID
    //char *Name[16];
    //uint32 nameFlags;
};

struct WorldMapAreaEntry
{
    //uint32    ID;                                         // 0
    uint32    map_id;                                       // 1
    uint32    zone_id;                                      // 2 index (continent 0 areas ignored)
    //char*   internal_name                                 // 3
    float     y1;                                           // 4
    float     y2;                                           // 5
    float     x1;                                           // 6
    float     x2;                                           // 7
    int32   virtual_map_id;                                 // 8 -1 (map_id have correct map) other: virtual map where zone show (map_id - where zone in fact internally)
};

struct WorldSafeLocsEntry
{
    uint32    ID;                                           // 0
    uint32    map_id;                                       // 1
    float     x;                                            // 2
    float     y;                                            // 3
    float     z;                                            // 4
    //char*   name[16]                                      // 5-20 name, unused
                                                            // 21 name flags, unused
};

#pragma pack(pop)

/*
Structures not used for casting to loaded DBC data and not required then packing
This is supposed to be LK only but we're going to use it for increased compat
hasErrorMessage is not yet used
*/
struct MapDifficulty
{
    MapDifficulty() : resetTime(0), maxPlayers(0), hasErrorMessage(false) { }
    MapDifficulty(uint32 _resetTime, uint32 _maxPlayers, bool _hasErrorMessage) : resetTime(_resetTime), maxPlayers(_maxPlayers), hasErrorMessage(_hasErrorMessage) { }

    uint32 resetTime;
    uint32 maxPlayers;
    bool hasErrorMessage;
};

// Structures not used for casting to loaded DBC data and not required then packing
struct TalentSpellPos
{
    TalentSpellPos() : talent_id(0), rank(0) {}
    TalentSpellPos(uint16 _talent_id, uint8 _rank) : talent_id(_talent_id), rank(_rank) {}

    uint16 talent_id;
    uint8  rank;
};

typedef std::map<uint32,TalentSpellPos> TalentSpellPosMap;

struct TaxiPathBySourceAndDestination
{
    TaxiPathBySourceAndDestination() : ID(0),price(0) {}
    TaxiPathBySourceAndDestination(uint32 _id,uint32 _price) : ID(_id),price(_price) {}

    uint32    ID;
    uint32    price;
};
typedef std::map<uint32,TaxiPathBySourceAndDestination> TaxiPathSetForSource;
typedef std::map<uint32,TaxiPathSetForSource> TaxiPathSetBySource;

typedef std::vector<TaxiPathNodeEntry const*> TaxiPathNodeList;
typedef std::vector<TaxiPathNodeList> TaxiPathNodesByPath;

#define TaxiMaskSize 16
typedef uint32 TaxiMask[TaxiMaskSize];

struct LiquidTypeEntry
{
    uint32 Id;
   //uint32 Unknown; //may be Flags
   //uint32 Unknown; //may be SoundBank
   //uint32 Unknown; //may be SoundID
    uint32 SpellId;
};

struct LightEntry
{
    uint32 Id;
    uint32 MapId;
    float X;
    float Y;
    float Z;
    //float FalloffStart;
    //float FalloffEnd;
    //uint32 SkyAndFog;
    //uint32 WaterSettings;
    //uint32 SunsetParams;
    //uint32 OtherParams;
    //uint32 DeathParams;
#ifdef LICH_KING
    //uint32 Unknown;
    //uint32 Unknown;
    //uint32 Unknown;
#endif
};

#endif