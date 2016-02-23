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

#ifndef TRINITYCORE_CREATURE_H
#define TRINITYCORE_CREATURE_H

#include "Common.h"
#include "Unit.h"
#include "UpdateMask.h"
#include "ItemPrototype.h"
#include "LootMgr.h"
#include "Database/DatabaseEnv.h"
#include "Cell.h"
#include "CreatureGroups.h"

#include <list>
#include <string>

class SpellInfo;

class CreatureAI;
class CreatureAINew;
class Quest;
class Player;
class WorldSession;
class CreatureGroup;
class TemporarySummon;

enum Gossip_Option
{
    GOSSIP_OPTION_NONE              = 0,                    //UNIT_NPC_FLAG_NONE              = 0,
    GOSSIP_OPTION_GOSSIP            = 1,                    //UNIT_NPC_FLAG_GOSSIP            = 1,
    GOSSIP_OPTION_QUESTGIVER        = 2,                    //UNIT_NPC_FLAG_QUESTGIVER        = 2,
    GOSSIP_OPTION_VENDOR            = 3,                    //UNIT_NPC_FLAG_VENDOR            = 4,
    GOSSIP_OPTION_TAXIVENDOR        = 4,                    //UNIT_NPC_FLAG_TAXIVENDOR        = 8,
    GOSSIP_OPTION_TRAINER           = 5,                    //UNIT_NPC_FLAG_TRAINER           = 16,
    GOSSIP_OPTION_SPIRITHEALER      = 6,                    //UNIT_NPC_FLAG_SPIRITHEALER      = 32,
    GOSSIP_OPTION_SPIRITGUIDE       = 7,                    //UNIT_NPC_FLAG_SPIRITGUIDE       = 64,
    GOSSIP_OPTION_INNKEEPER         = 8,                    //UNIT_NPC_FLAG_INNKEEPER         = 128,
    GOSSIP_OPTION_BANKER            = 9,                    //UNIT_NPC_FLAG_BANKER            = 256,
    GOSSIP_OPTION_PETITIONER        = 10,                   //UNIT_NPC_FLAG_PETITIONER        = 512,
    GOSSIP_OPTION_TABARDDESIGNER    = 11,                   //UNIT_NPC_FLAG_TABARDDESIGNER    = 1024,
    GOSSIP_OPTION_BATTLEFIELD       = 12,                   //UNIT_NPC_FLAG_BATTLEFIELDPERSON = 2048,
    GOSSIP_OPTION_AUCTIONEER        = 13,                   //UNIT_NPC_FLAG_AUCTIONEER        = 4096,
    GOSSIP_OPTION_STABLEPET         = 14,                   //UNIT_NPC_FLAG_STABLE            = 8192,
    GOSSIP_OPTION_ARMORER           = 15,                   //UNIT_NPC_FLAG_ARMORER           = 16384,
    GOSSIP_OPTION_UNLEARNTALENTS    = 16,                   //UNIT_NPC_FLAG_TRAINER (bonus option for GOSSIP_OPTION_TRAINER)
    GOSSIP_OPTION_UNLEARNPETTALENTS = 17,                   //UNIT_NPC_FLAG_TRAINER (bonus option for GOSSIP_OPTION_TRAINER)
    GOSSIP_OPTION_OUTDOORPVP        = 18,                   //added by code (option for outdoor pvp creatures)
    GOSSIP_OPTION_HALLOWS_END       = 19,                    // For Innkeepers during Hallow's End
    GOSSIP_OPTION_MAX
};

enum Gossip_Guard
{
    GOSSIP_GUARD_BANK               = 32,
    GOSSIP_GUARD_RIDE               = 33,
    GOSSIP_GUARD_GUILD              = 34,
    GOSSIP_GUARD_INN                = 35,
    GOSSIP_GUARD_MAIL               = 36,
    GOSSIP_GUARD_AUCTION            = 37,
    GOSSIP_GUARD_WEAPON             = 38,
    GOSSIP_GUARD_STABLE             = 39,
    GOSSIP_GUARD_BATTLE             = 40,
    GOSSIP_GUARD_SPELLTRAINER       = 41,
    GOSSIP_GUARD_SKILLTRAINER       = 42
};

enum Gossip_Guard_Spell
{
    GOSSIP_GUARD_SPELL_WARRIOR      = 64,
    GOSSIP_GUARD_SPELL_PALADIN      = 65,
    GOSSIP_GUARD_SPELL_HUNTER       = 66,
    GOSSIP_GUARD_SPELL_ROGUE        = 67,
    GOSSIP_GUARD_SPELL_PRIEST       = 68,
    GOSSIP_GUARD_SPELL_UNKNOWN1     = 69,
    GOSSIP_GUARD_SPELL_SHAMAN       = 70,
    GOSSIP_GUARD_SPELL_MAGE         = 71,
    GOSSIP_GUARD_SPELL_WARLOCK      = 72,
    GOSSIP_GUARD_SPELL_UNKNOWN2     = 73,
    GOSSIP_GUARD_SPELL_DRUID        = 74
};

enum Gossip_Guard_Skill
{
    GOSSIP_GUARD_SKILL_ALCHEMY      = 80,
    GOSSIP_GUARD_SKILL_BLACKSMITH   = 81,
    GOSSIP_GUARD_SKILL_COOKING      = 82,
    GOSSIP_GUARD_SKILL_ENCHANT      = 83,
    GOSSIP_GUARD_SKILL_FIRSTAID     = 84,
    GOSSIP_GUARD_SKILL_FISHING      = 85,
    GOSSIP_GUARD_SKILL_HERBALISM    = 86,
    GOSSIP_GUARD_SKILL_LEATHER      = 87,
    GOSSIP_GUARD_SKILL_MINING       = 88,
    GOSSIP_GUARD_SKILL_SKINNING     = 89,
    GOSSIP_GUARD_SKILL_TAILORING    = 90,
    GOSSIP_GUARD_SKILL_ENGINEERING   = 91
};

enum GossipOptionIcon
{
    GOSSIP_ICON_CHAT                = 0,                    //white chat bubble
    GOSSIP_ICON_VENDOR              = 1,                    //brown bag
    GOSSIP_ICON_TAXI                = 2,                    //flight
    GOSSIP_ICON_TRAINER             = 3,                    //book
    GOSSIP_ICON_INTERACT_1          = 4,                    //interaction wheel
    GOSSIP_ICON_INTERACT_2          = 5,                    //interaction wheel
    GOSSIP_ICON_MONEY_BAG           = 6,                    //brown bag with yellow dot
    GOSSIP_ICON_TALK                = 7,                    //white chat bubble with black dots
    GOSSIP_ICON_TABARD              = 8,                    //tabard
    GOSSIP_ICON_BATTLE              = 9,                    //two swords
    GOSSIP_ICON_DOT                 = 10,                   //yellow dot
//next ones may not be BC
    GOSSIP_ICON_CHAT_11             = 11,                   // white chat bubble
    GOSSIP_ICON_CHAT_12             = 12,                   // white chat bubble
    GOSSIP_ICON_CHAT_13             = 13,                   // white chat bubble
    GOSSIP_ICON_UNK_14              = 14,                   // INVALID - DO NOT USE
    GOSSIP_ICON_UNK_15              = 15,                   // INVALID - DO NOT USE
    GOSSIP_ICON_CHAT_16             = 16,                   // white chat bubble
    GOSSIP_ICON_CHAT_17             = 17,                   // white chat bubble
    GOSSIP_ICON_CHAT_18             = 18,                   // white chat bubble
    GOSSIP_ICON_CHAT_19             = 19,                   // white chat bubble
    GOSSIP_ICON_CHAT_20             = 20,                   // white chat bubble
    GOSSIP_ICON_MAX
};

enum CreatureFlagsExtra
{
    CREATURE_FLAG_EXTRA_INSTANCE_BIND        = 0x00000001,       // creature kill bind instance with killer and killer's group
    CREATURE_FLAG_EXTRA_CIVILIAN             = 0x00000002,       // not aggro (ignore faction/reputation hostility)
    CREATURE_FLAG_EXTRA_NO_PARRY             = 0x00000004,       // creature can't parry
    CREATURE_FLAG_EXTRA_NO_PARRY_RUSH        = 0x00000008,       // creature can't parry rush
    CREATURE_FLAG_EXTRA_NO_BLOCK             = 0x00000010,       // creature can't block
    CREATURE_FLAG_EXTRA_NO_CRUSH             = 0x00000020,       // creature can't do crush attacks
    CREATURE_FLAG_EXTRA_NO_XP_AT_KILL        = 0x00000040,       // creature kill not provide XP
    CREATURE_FLAG_EXTRA_TRIGGER              = 0x00000080,       // trigger creature
    CREATURE_FLAG_EXTRA_WORLDEVENT           = 0x00004000,       // custom flag for world event creatures (left room for merging)
    CREATURE_FLAG_EXTRA_NO_SPELL_SLOW        = 0x00008000,       // cannot have spell casting slowed down
    CREATURE_FLAG_EXTRA_NO_TAUNT             = 0x00010000,       // cannot be taunted
    CREATURE_FLAG_EXTRA_NO_CRIT              = 0x00020000,       // creature can't do critical strikes
    CREATURE_FLAG_EXTRA_HOMELESS             = 0x00040000,       // consider current position instead of home position for threat area
    CREATURE_FLAG_EXTRA_ALIVE_INVISIBLE      = 0x00080000,       // not visible for alive players
    CREATURE_FLAG_EXTRA_PERIODIC_RELOC       = 0x00100000,       // periodic on place relocation when ooc (use this for static mobs only)
    CREATURE_FLAG_EXTRA_DUAL_WIELD           = 0x00200000,       // can dual wield
    CREATURE_FLAG_EXTRA_NO_PLAYER_DAMAGE_REQ = 0x00400000,       // creature does not need to take player damage for kill credit
};

enum WeaponSlot
{
    WEAPON_SLOT_MAINHAND = 0,
    WEAPON_SLOT_OFFHAND = 1,
    WEAPON_SLOT_RANGED = 2,
};

#pragma pack(push,1)

// from `creature_template` table
struct CreatureTemplate
{
    uint32  Entry;
    uint32  difficulty_entry_1;
    uint32  Modelid1;
    uint32  Modelid2;
    uint32  Modelid3;
    uint32  Modelid4;
    std::string Name;
    std::string SubName;
    std::string IconName;
    uint32  GossipMenuId;
    uint32  minlevel;
    uint32  maxlevel;
    uint32  expansion;
    uint32  faction;
    uint32  npcflag;
    float   speed;
    float   scale;
    uint32  rank;
    uint32  dmgschool;
    uint32  baseattacktime;
    uint32  rangeattacktime;
    float   BaseVariance;
    float   RangeVariance;
    uint32  unit_class;                                     // enum Classes. Note only 4 classes are known for creatures.
    uint32  unit_flags;                                     // enum UnitFlags mask values
    uint32  dynamicflags;
    uint32  family;                                         // enum CreatureFamily values for type==CREATURE_TYPE_BEAST, or 0 in another cases
    uint32  trainer_type;
    uint32  trainer_spell;
    uint32  trainer_class;
    uint32  trainer_race;
    uint32  type;                                           // enum CreatureType values
    uint32  type_flags;                                     // enum CreatureTypeFlags mask values
    uint32  lootid;
    uint32  pickpocketLootId;
    uint32  SkinLootId;
    int32   resistance[MAX_SPELL_SCHOOL-1];
    uint32  spells[CREATURE_MAX_SPELLS];
    uint32  PetSpellDataId;
    uint32  mingold;
    uint32  maxgold;
    std::string AIName;
    uint32  MovementType;
    uint32  InhabitType;
    bool    RacialLeader;
    float   ModHealth;
    float   ModMana;
    float   ModArmor;
    float   ModDamage;
    float   ModExperience;
    bool    RegenHealth;
    uint32  equipmentId;
    uint32  MechanicImmuneMask;
    uint32  flags_extra;
    uint32  ScriptID;
    uint32  QuestPoolId;
    std::string scriptName; //CreatureAINew
    uint32 GetRandomValidModelId() const;
    uint32 GetFirstValidModelId() const;

    // helpers
    SkillType GetRequiredLootSkill() const
    {
        if(type_flags & CREATURE_TYPEFLAGS_HERBLOOT)
            return SKILL_HERBALISM;
        else if(type_flags & CREATURE_TYPEFLAGS_MININGLOOT)
            return SKILL_MINING;
        else
            return SKILL_SKINNING;                          // normal case
    }

    bool isTameable() const
    {
        return type == CREATURE_TYPE_BEAST && family != 0 && (type_flags & CREATURE_TYPEFLAGS_TAMEABLE);
    }
};

typedef std::unordered_map<uint32, CreatureTemplate> CreatureTemplateContainer;

// Defines base stats for creatures (used to calculate HP/mana/armor/attackpower/rangedattackpower/all damage).
struct CreatureBaseStats
{
    uint32 BaseHealth[MAX_EXPANSIONS];
    uint32 BaseMana;
    uint32 BaseArmor;
    uint32 AttackPower;
    uint32 RangedAttackPower;
    float BaseDamage[MAX_EXPANSIONS];

    // Helpers

    uint32 GenerateHealth(CreatureTemplate const* info) const
    {
        return uint32(ceil(BaseHealth[info->expansion] * info->ModHealth));
    }

    uint32 GenerateMana(CreatureTemplate const* info) const
    {
        // Mana can be 0.
        if (!BaseMana)
            return 0;

        return uint32(ceil(BaseMana * info->ModMana));
    }

    uint32 GenerateArmor(CreatureTemplate const* info) const
    {
        return uint32(ceil(BaseArmor * info->ModArmor));
    }

    float GenerateBaseDamage(CreatureTemplate const* info) const
    {
        return BaseDamage[info->expansion];
    }

    static CreatureBaseStats const* GetBaseStats(uint8 level, uint8 unitClass);
};

typedef std::unordered_map<uint16, CreatureBaseStats> CreatureBaseStatsContainer;

struct CreatureLocale
{
    std::vector<std::string> Name;
    std::vector<std::string> SubName;
};

struct GossipMenuItemsLocale
{
    std::vector<std::string> OptionText;
    std::vector<std::string> BoxText;
};

struct PointOfInterestLocale
{
    std::vector<std::string> IconName;
};

struct EquipmentInfo
{
    uint32  equipmodel[3];
    uint32  equipinfo[3];
    uint32  equipslot[3];
};

typedef std::unordered_map<uint8, EquipmentInfo> EquipmentInfoContainerInternal;
typedef std::unordered_map<uint32, EquipmentInfo> EquipmentInfoContainer;

// from `creature` table
struct CreatureData
{
    uint32 id;                                              // entry in creature_template
    uint16 mapid;
    uint32 displayid;
    int32 equipmentId;
    float posX;
    float posY;
    float posZ;
    float orientation;
    uint32 spawntimesecs;
    float spawndist;
    uint32 currentwaypoint;
    uint32 curhealth;
    uint32 curmana;
    uint8 movementType;
    uint8 spawnMask;
    uint32 poolId;
    uint32 scriptId;
    std::string scriptName;
    uint32 instanceEventId; // If spawned in raid, don't respawn if corresponding instance event is != NOT_STARTED
};

// from `creature_addon` table
struct CreatureAddon
{
    uint32 path_id;
    uint32 mount;
    uint32 bytes0;
    uint32 bytes1;
    uint32 bytes2;
    uint32 emote;
    uint32 move_flags;
    std::vector<uint32> auras;
};

typedef std::unordered_map<uint32, CreatureAddon> CreatureAddonContainer;

struct CreatureModelInfo
{
    float bounding_radius;
    float combat_reach;
    uint8 gender;
    uint32 modelid_other_gender;
};

typedef std::unordered_map<uint16, CreatureModelInfo> CreatureModelContainer;

enum InhabitTypeValues
{
    INHABIT_GROUND = 1,
    INHABIT_WATER  = 2,
    INHABIT_AIR    = 4,
    INHABIT_ANYWHERE = INHABIT_GROUND | INHABIT_WATER | INHABIT_AIR
};

// Enums used by StringTextData::Type
enum ChatType
{
    CHAT_TYPE_SAY               = 0,
    CHAT_TYPE_YELL              = 1,
    CHAT_TYPE_TEXT_EMOTE        = 2,
    CHAT_TYPE_BOSS_EMOTE        = 3,
    CHAT_TYPE_WHISPER           = 4,
    CHAT_TYPE_BOSS_WHISPER      = 5,
    CHAT_TYPE_ZONE_YELL         = 6,
    CHAT_TYPE_SERVER_EMOTE      = 7,
    CHAT_TYPE_END               = 255
};

#pragma pack(pop)

// Vendors
struct VendorItem
{
    VendorItem(ItemTemplate const* proto, uint32 _maxcount, uint32 _incrtime, uint32 _ExtendedCost)
        : proto(proto), maxcount(_maxcount), incrtime(_incrtime), ExtendedCost(_ExtendedCost) {}

    ItemTemplate const* proto;
    uint32 maxcount;                                        // 0 for infinity item amount
    uint32 incrtime;                                        // time for restore items amount if maxcount != 0
    uint32 ExtendedCost;
};
typedef std::vector<VendorItem*> VendorItemList;

struct VendorItemData
{
    VendorItemList m_items;

    VendorItem* GetItem(uint32 slot) const
    {
        if(slot>=m_items.size()) return nullptr;
        return m_items[slot];
    }
    bool Empty() const { return m_items.empty(); }
    uint8 GetItemCount() const { return m_items.size(); }
    void AddItem( ItemTemplate const *proto, uint32 maxcount, uint32 ptime, uint32 ExtendedCost)
    {
        m_items.push_back(new VendorItem(proto, maxcount, ptime, ExtendedCost));
    }
    bool RemoveItem( uint32 item_id );
    VendorItem const* FindItem(uint32 item_id) const;
    size_t FindItemSlot(uint32 item_id) const;

    void Clear()
    {
        for (VendorItemList::iterator itr = m_items.begin(); itr != m_items.end(); ++itr)
            delete (*itr);
    }
};

struct VendorItemCount
{
    explicit VendorItemCount(uint32 _item, uint32 _count)
        : itemId(_item), count(_count), lastIncrementTime(time(NULL)) {}

    uint32 itemId;
    uint32 count;
    time_t lastIncrementTime;
};

typedef std::list<VendorItemCount> VendorItemCounts;

struct TrainerSpell
{
    uint32 spell;
    uint32 spellcost;
    uint32 reqskill;
    uint32 reqskillvalue;
    uint32 reqlevel;
};

typedef std::vector<TrainerSpell*> TrainerSpellList;

struct TrainerSpellData
{
    TrainerSpellData() : trainerType(0) {}

    TrainerSpellList spellList;
    uint32 trainerType;                                     // trainer type based at trainer spells, can be different from creature_template value.
                                                            // req. for correct show non-prof. trainers like weaponmaster, allowed values 0 and 2.

    void Clear();
    TrainerSpell const* Find(uint32 spell_id) const;
};

typedef std::map<uint32,time_t> CreatureSpellCooldowns;

// max different by z coordinate for creature aggro reaction
#define CREATURE_Z_ATTACK_RANGE 3
#define CREATURE_MAX_DEATH_WARN_RANGE 60.0f

#ifdef LICH_KING
#define MAX_VENDOR_ITEMS 150                                // Limitation in 3.x.x item count in SMSG_LIST_INVENTORY
#else
#define MAX_VENDOR_ITEMS 255                                // Limitation in item count field size in SMSG_LIST_INVENTORY
#endif

//used for handling non-repeatable random texts
typedef std::vector<uint8> CreatureTextRepeatIds;
typedef std::unordered_map<uint8, CreatureTextRepeatIds> CreatureTextRepeatGroup;

class Creature : public Unit
{
    public:

        explicit Creature();
        virtual ~Creature();

        void AddToWorld();
        void RemoveFromWorld();
        
        void DisappearAndDie();

        bool Create (uint32 guidlow, Map *map, uint32 Entry, const CreatureData *data = nullptr);
        //get data from SQL storage
        void LoadCreatureAddon();
        //reapply creature addon data to creature
        bool InitCreatureAddon(bool reload = false);
        void SelectLevel();
        void LoadEquipment(uint32 equip_entry, bool force=false);
        //Set creature visual weapon (prefer creating values in creature_equip_template in db and loading them with LoadEquipment)
        void SetWeapon(WeaponSlot slot, uint32 displayid, ItemSubclassWeapon subclass, InventoryType inventoryType);

        uint32 GetDBTableGUIDLow() const { return m_DBTableGuid; }
        std::string const& GetSubName() const { return GetCreatureTemplate()->SubName; }

        void Update( uint32 time );                         // overwrited Unit::Update
        void GetRespawnPosition(float &x, float &y, float &z, float* ori = nullptr, float* dist =nullptr) const;
        uint32 GetEquipmentId() const { return m_equipmentId; }

        bool IsPet() const { return m_IsPet; }
        void SetCorpseDelay(uint32 delay) { m_corpseDelay = delay; }
        bool IsTotem() const { return m_isTotem; }
        bool isRacialLeader() const { return GetCreatureTemplate()->RacialLeader; }
        bool IsCivilian() const { return GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_CIVILIAN; }
        bool isTrigger() const { return GetCreatureTemplate()->flags_extra & CREATURE_FLAG_EXTRA_TRIGGER; }
        void SetReactState(ReactStates st) { m_reactState = st; }
        ReactStates GetReactState() { return m_reactState; }
        bool HasReactState(ReactStates state) const { return (m_reactState == state); }
        bool isTrainerFor(Player* player, bool msg) const;
        bool isCanInteractWithBattleMaster(Player* player, bool msg) const;
        bool canResetTalentsOf(Player* pPlayer) const;
        bool IsOutOfThreatArea(Unit* pVictim) const;
        bool IsImmunedToSpell(SpellInfo const* spellInfo, bool useCharges = false) override;
                                                            // redefine Unit::IsImmunedToSpell
        bool IsImmunedToSpellEffect(uint32 effect, uint32 mechanic) const override;
                                                            // redefine Unit::IsImmunedToSpellEffect
        void ProhibitSpellSchool(SpellSchoolMask /*idSchoolMask*/, uint32 /*unTimeMs*/);
        void UpdateProhibitedSchools(uint32 const diff);
        bool IsSpellSchoolMaskProhibited(SpellSchoolMask /*idSchoolMask*/);
        bool isElite() const
        {
            if(IsPet())
                return false;

            uint32 rank = GetCreatureTemplate()->rank;
            return rank != CREATURE_ELITE_NORMAL && rank != CREATURE_ELITE_RARE;
        }

        bool IsWorldBoss() const
        {
            if(IsPet())
                return false;

            return (GetCreatureTemplate()->rank == CREATURE_ELITE_WORLDBOSS) || (GetCreatureTemplate()->type_flags & CREATURE_TYPEFLAGS_BOSS);
        }

        uint32 GetLevelForTarget(Unit const* target) const; // overwrite Unit::GetLevelForTarget for boss level support

        bool isMoving();
        bool IsInEvadeMode() const;

        //remove previous AI, reselect a new one and initilalize it. Also reset movement.
        //@ai assign this AI
        bool AIM_Initialize(CreatureAI* ai = nullptr);
        void Motion_Initialize();
        bool InitCustomScript(uint32 scriptId);

        void WarnDeathToFriendly();

        CreatureAI* AI() { return (CreatureAI*)i_AI; }
        CreatureAINew* getAI() { return m_AI; }

        uint32 GetShieldBlockValue() const                  //dunno mob block value
        {
            return (GetLevel()/2 + uint32(GetStat(STAT_STRENGTH)/20));
        }

        SpellSchoolMask GetMeleeDamageSchoolMask() const { return m_meleeDamageSchoolMask; }
        void SetMeleeDamageSchool(SpellSchools school) { m_meleeDamageSchoolMask = SpellSchoolMask(1 << school); }

        void _AddCreatureSpellCooldown(uint32 spell_id, time_t end_time);
        void _AddCreatureCategoryCooldown(uint32 category, time_t apply_time);
        void AddCreatureSpellCooldown(uint32 spellid);
        bool HasSpellCooldown(uint32 spell_id) const;
        bool HasCategoryCooldown(uint32 spell_id) const;

        bool HasSpell(uint32 spellID) const;

        bool UpdateEntry(uint32 entry, const CreatureData* data = nullptr);
        /* 
        Updates creatures movement flags according to current position
        Remove flying movement flags if creature is not in air.
        Add flying movement flags if creature can fly is in air and not flying.
        Also set creature as swimming if in water
        */
        void UpdateMovementFlags();

        bool UpdateStats(Stats stat) override;
        bool UpdateAllStats() override;
        void UpdateResistances(uint32 school) override;
        void UpdateArmor() override;
        void UpdateMaxHealth() override;
        void UpdateMaxPower(Powers power) override;
        void UpdateAttackPowerAndDamage(bool ranged = false) override;
        void CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& minDamage, float& maxDamage, Unit* target = nullptr) override;

        uint32 GetCurrentEquipmentId() { return m_equipmentId; }

        VendorItemData const* GetVendorItems() const;
        uint32 GetVendorItemCurrentCount(VendorItem const* vItem);
        uint32 UpdateVendorItemCurrentCount(VendorItem const* vItem, uint32 used_count);

        TrainerSpellData const* GetTrainerSpells() const;

        CreatureTemplate const *GetCreatureTemplate() const { return m_creatureInfo; }
        CreatureAddon const* GetCreatureAddon() const { return m_creatureInfoAddon; }

        std::string GetScriptName();
        uint32 GetScriptId();
        std::string GetAIName() const;
        std::string getScriptName(); // New
        uint32 getInstanceEventId();

        void ResetCreatureEmote();

        // overwrite WorldObject function for proper name localization
        std::string const& GetNameForLocaleIdx(LocaleConstant locale_idx) const override;

        void SetDeathState(DeathState s);                   // overwrite virtual Unit::setDeathState

        bool LoadFromDB(uint32 guid, Map *map);
        void SaveToDB();
                                                            // overwrited in Pet
        virtual void SaveToDB(uint32 mapid, uint8 spawnMask);
        virtual void DeleteFromDB();                        // overwrited in Pet

        Loot loot;
        bool lootForPickPocketed;
        bool lootForBody;
        Player *GetLootRecipient() const;
        Group* GetLootRecipientGroup() const;
        bool hasLootRecipient() const { return m_lootRecipient != 0 || m_lootRecipientGroup; }
        bool isTappedBy(Player const* player) const;                          // return true if the creature is tapped by the player or a member of his party.

        void SetLootRecipient (Unit* unit);
        void AllLootRemovedFromCorpse();

        SpellInfo const* reachWithSpellAttack(Unit *pVictim);
        SpellInfo const* reachWithSpellCure(Unit *pVictim);

        uint32 m_spells[CREATURE_MAX_SPELLS];
        CreatureSpellCooldowns m_CreatureSpellCooldowns;
        CreatureSpellCooldowns m_CreatureCategoryCooldowns;
        uint32 m_GlobalCooldown;

        bool CanSeeOrDetect(Unit const* u, bool detect, bool inVisibleList = false, bool is3dDistance = true) const;
        bool IsWithinSightDist(Unit const* u) const;
        /* Return if creature can aggro and start attacking target, depending on faction, distance, LoS, if target is attackable, ...
        @assistAggro check for assisting instead of standard aggro. This changes the allowed distance only.
        */
        CanAttackResult CanAggro(Unit const* u, bool assistAggro = false) const;
        float GetAttackDistance(Unit const* pl) const;
        
        /** The "suspicious look" is a warning whenever a stealth player is about to be detected by a creature*/
        /* return true if the creature can do a suspicious look on target. This does NOT check for detection range, use CanAggro, CanAttack or CanDetectStealthOf results to ensure this distance is correct. */
        bool CanDoSuspiciousLook(Unit const* target) const;
        //start lookup suspicously at target
        void StartSuspiciousLook(Unit const* target);

        Unit* SelectNearestTarget(float dist = 0, bool playerOnly = false, bool furthest = false) const;
        //select nearest alive player
        Player* SelectNearestPlayer(float distance = 0) const;

        /** Call assistance at short range (chain aggro mechanic) */
        void CallAssistance();
        /** Actively call for help, does NOT check for faction, all friendly are eligibles */
        void CallForHelp(float fRadius);
        void SetNoCallAssistance(bool val) { m_AlreadyCallAssistance = val; }
        bool CanCallAssistance() { return !m_AlreadyCallAssistance; }
        bool CanAssistTo(const Unit* u, const Unit* enemy, bool checkFaction = true) const;
        void DoFleeToGetAssistance(float radius = 50);


        MovementGeneratorType GetDefaultMovementType() const { return m_defaultMovementType; }
        void SetDefaultMovementType(MovementGeneratorType mgt) { m_defaultMovementType = mgt; }

        bool IsVisibleInGridForPlayer(Player const* pl) const;

        void RemoveCorpse(bool setSpawnTime = true);
        
        void ForcedDespawn(uint32 timeMSToDespawn = 0);

        time_t const& GetRespawnTime() const { return m_respawnTime; }
        time_t GetRespawnTimeEx() const;
        void SetRespawnTime(uint32 respawn) { m_respawnTime = respawn ? time(NULL) + respawn : 0; }
        void Respawn();
        void SaveRespawnTime();

        uint32 GetRespawnDelay() const { return m_respawnDelay; }
        void SetRespawnDelay(uint32 delay) { m_respawnDelay = delay; }

        float GetRespawnRadius() const { return m_respawnradius; }
        void SetRespawnRadius(float dist) { m_respawnradius = dist; }

        // Linked Creature Respawning System
        time_t GetLinkedCreatureRespawnTime() const;
        const CreatureData* GetLinkedRespawnCreatureData() const;

        void SendZoneUnderAttackMessage(Player* attacker);

        bool HasQuest(uint32 quest_id) const;
        bool HasInvolvedQuest(uint32 quest_id)  const;

        GridReference<Creature> &GetGridRef() { return m_gridRef; }
        bool isRegeneratingHealth() { return m_regenHealth; }
        void setRegeneratingHealth(bool regenHealth) { m_regenHealth = regenHealth; }
        virtual uint8 GetPetAutoSpellSize() const { return CREATURE_MAX_SPELLS; }
        virtual uint32 GetPetAutoSpellOnPos(uint8 pos) const
        {
            if (pos >= CREATURE_MAX_SPELLS || m_charmInfo->GetCharmSpell(pos)->active != ACT_ENABLED)
                return 0;
            else
                return m_charmInfo->GetCharmSpell(pos)->spellId;
        }

        void SetPosition(float x, float y, float z, float o);
        void SetPosition(const Position &pos) { SetPosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation()); }

        void SetHomePosition(float x, float y, float z, float o) { m_homePosition.Relocate(x, y, z, o); }
        void SetHomePosition(const Position &pos) { m_homePosition.Relocate(pos); }
        void GetHomePosition(float& x, float& y, float& z, float& ori) const { m_homePosition.GetPosition(x, y, z, ori); }
        Position const& GetHomePosition() const { return m_homePosition; }
        float GetDistanceFromHome() const;

        void SetTransportHomePosition(float x, float y, float z, float o) { m_transportHomePosition.Relocate(x, y, z, o); }
        void SetTransportHomePosition(const Position &pos) { m_transportHomePosition.Relocate(pos); }
        void GetTransportHomePosition(float& x, float& y, float& z, float& ori) const { m_transportHomePosition.GetPosition(x, y, z, ori); }
        Position const& GetTransportHomePosition() const { return m_transportHomePosition; }

        uint32 GetGlobalCooldown() const { return m_GlobalCooldown; }

        uint32 GetWaypointPathId(){return m_path_id;}
        void LoadPath(uint32 pathid) { m_path_id = pathid; }

        uint32 GetCurrentWaypointID(){return m_waypointID;}
        void UpdateWaypointID(uint32 wpID){m_waypointID = wpID;}

        //check if creature is present in a formation from CreatureGroupManager, and adds it to it if one is found
        void SearchFormation();
        CreatureGroup *GetFormation(){return m_formation;}
        void SetFormation(CreatureGroup *formation) {m_formation = formation;}

        Unit *SelectVictim(bool evade = true);

        void SetDisableReputationGain(bool disable) { DisableReputationGain = disable; }
        bool IsReputationGainDisabled() const { return DisableReputationGain; }
        bool IsDamageEnoughForLootingAndReward() const { return (m_creatureInfo->flags_extra & CREATURE_FLAG_EXTRA_NO_PLAYER_DAMAGE_REQ) || (m_PlayerDamageReq == 0); }
        void LowerPlayerDamageReq(uint32 unDamage)
        {
            if(m_PlayerDamageReq)
                m_PlayerDamageReq > unDamage ? m_PlayerDamageReq -= unDamage : m_PlayerDamageReq = 0;
        }
        void ResetPlayerDamageReq() { m_PlayerDamageReq = GetHealth() / 2; }
        uint32 m_PlayerDamageReq;
        
        uint32 GetQuestPoolId() const { return m_questPoolId; }
        void SetQuestPoolId(uint32 id) { m_questPoolId = id; }
        uint32 GetCreaturePoolId() const { return m_creaturePoolId; }
        void SetCreaturePoolId(uint32 id) { m_creaturePoolId = id; }
        
        /** This is only filled for world bosses */
        bool HadPlayerInThreatListAtDeath(uint64 guid) const;
        void ConvertThreatListIntoPlayerListAtDeath();
        std::set<uint32> const& GetThreatListAtDeath() const { return m_playerInThreatListAtDeath; }

        // Respawned since less than 5 secs
        bool HasJustRespawned() const { return (m_timeSinceSpawn < 5000); }
        void SetCorpseRemoveTime(uint32 removeTime) { m_corpseRemoveTime = removeTime; }
        uint32 GetCorpseDelay() const { return m_corpseDelay; }
        
        // Scripting tools
        bool IsBelowHPPercent(float percent);
        bool IsAboveHPPercent(float percent);
        bool IsBetweenHPPercent(float minPercent, float maxPercent);
        
        bool IsBeingEscorted() const { return m_isBeingEscorted; }
        void SetEscorted(bool status) { m_isBeingEscorted = status; }

        bool IsSummoned() const { return m_summoned; }
        TemporarySummon* ToTemporarySummon();

        //Play message for current creature when given time is elapsed.
        void AddMessageEvent(uint64 timer, uint32 messageId, uint64 data = 0);

        bool CanWalk() const { return GetCreatureTemplate()->InhabitType & INHABIT_GROUND; }
        bool CanSwim() const { return GetCreatureTemplate()->InhabitType & INHABIT_WATER || IsPet(); }
        bool CanFly() const override { return m_canFly; }

        bool SetWalk(bool enable) override;
        bool SetDisableGravity(bool disable, bool packetOnly = false) override;
        bool SetSwim(bool enable) override;
        // /!\ This is TC SetCanFly
        bool SetFlying(bool enable, bool packetOnly = false) override;
        // /!\ Not TC SetCanFly. This marks creature as able to fly, rather than making it fly. You can then call UpdateMovementFlags() if you want to update fly mode immediately.
        void SetCanFly(bool enable, bool updateMovementFlags = true);
        bool SetWaterWalking(bool enable, bool packetOnly = false) override;
        bool SetFeatherFall(bool enable, bool packetOnly = false) override;
        bool SetHover(bool enable, bool packetOnly = false) override;

        void FarTeleportTo(Map* map, float X, float Y, float Z, float O);

        // Handling caster facing during spellcast
        void SetTarget(uint64 guid);
        void FocusTarget(Spell const* focusSpell, WorldObject const* target);
        void ReleaseFocus(Spell const* focusSpell);

        CreatureTextRepeatIds GetTextRepeatGroup(uint8 textGroup);
        void SetTextRepeatId(uint8 textGroup, uint8 id);
        void ClearTextRepeatGroup(uint8 textGroup);

        void IncreaseUnreachableTargetTime(uint32 timediff) { m_unreachableTargetTime += timediff; }
        void ResetUnreachableTargetTime() { m_unreachableTargetTime = 0; }
        uint32 GetUnreachableTargetTime() { return m_unreachableTargetTime; }
        //enter evade mode if target was unreachable for CONFIG_CREATURE_MAX_UNREACHABLE_TARGET_TIME
        void CheckForUnreachableTarget();

    protected:
        bool CreateFromProto(uint32 guidlow, uint32 Entry, const CreatureData *data = nullptr);
        bool InitEntry(uint32 entry, const CreatureData* data = nullptr);

        // vendor items
        VendorItemCounts m_vendorItemCounts;

        void _RealtimeSetCreatureInfo();

        uint32 m_lootMoney;
        uint64 m_lootRecipient;
        uint32 m_lootRecipientGroup; //group identified by leader
        /* This is only filled for worldbosses with every players with threat > 0  */
        std::set<uint32> m_playerInThreatListAtDeath;

        /// Timers
        uint32 m_corpseRemoveTime;                          // (msecs)timer for death or corpse disappearance
        time_t m_respawnTime;                               // (secs) time of next respawn
        uint32 m_respawnDelay;                              // (secs) delay between corpse disappearance and respawning
        uint32 m_corpseDelay;                               // (secs) delay between death and corpse disappearance
        float m_respawnradius;

        bool m_IsPet;                                       // set only in Pet::Pet
        bool m_isTotem;                                     // set only in Totem::Totem
        ReactStates m_reactState;                           // for AI, not charmInfo
        void RegenerateMana();
        void RegenerateHealth();
        uint32 m_regenTimer;
        uint32 m_areaCombatTimer;
        uint32 m_relocateTimer;
        void AreaCombat();
        MovementGeneratorType m_defaultMovementType;
        uint32 m_DBTableGuid;                               ///< For new or temporary creatures is 0 for saved it is lowguid
        uint32 m_equipmentId;

        bool m_AlreadyCallAssistance;
        bool m_regenHealth;
        bool m_AI_locked;

        SpellSchoolMask m_meleeDamageSchoolMask;
        uint32 m_originalEntry;

        Position m_homePosition;
        Position m_transportHomePosition;

        bool DisableReputationGain;
        
        uint32 m_questPoolId;
        
        uint32 m_creaturePoolId;
        
        //std::vector<uint64> m_allowedToLoot;
        
        uint64 m_timeSinceSpawn;                            // (msecs) elapsed time since (re)spawn
        
        CreatureAINew* m_AI;

        uint32 m_prohibitedSchools[7];
        
        bool m_isBeingEscorted;
        bool m_summoned;

        uint32 m_unreachableTargetTime;
        bool m_canFly; //create is able to fly. Not directly related to the CAN_FLY moveflags. Yes this is all confusing.

        uint32 m_stealthWarningCooldown;

    private:
        //WaypointMovementGenerator vars
        uint32 m_waypointID;
        uint32 m_path_id;

        //Formation var
        CreatureGroup *m_formation;
        bool TriggerJustRespawned;

        GridReference<Creature> m_gridRef;
        CreatureTemplate const* m_creatureInfo;                 // in heroic mode can different from ObjectMgr::GetCreatureTemplate(GetEntry())
        CreatureAddon const* m_creatureInfoAddon;

        Spell const* _focusSpell;   ///> Locks the target during spell cast for proper facing

        CreatureTextRepeatGroup m_textRepeat;
};

class AssistDelayEvent : public BasicEvent
{
    public:
        AssistDelayEvent(const uint64& victim, Unit& owner) : BasicEvent(), m_victim(victim), m_owner(owner) { }

        bool Execute(uint64 e_time, uint32 p_time);
        void AddAssistant(const uint64& guid) { m_assistants.push_back(guid); }
    private:
        AssistDelayEvent();

        uint64            m_victim;
        std::list<uint64> m_assistants;
        Unit&             m_owner;
};

class ForcedDespawnDelayEvent : public BasicEvent
{
    public:
        ForcedDespawnDelayEvent(Creature& owner) : BasicEvent(), m_owner(owner) { }
        bool Execute(uint64 e_time, uint32 p_time);

    private:
        Creature& m_owner;
};

class AIMessageEvent : public BasicEvent
{
public:
    AIMessageEvent(Creature& owner, uint32 id, uint64 data = 0) : 
        owner(owner),
        id(id),
        data(data)
    {}

    bool Execute(uint64 /*e_time*/, uint32 /*p_time*/);

private:
    Creature& owner;
    uint32 id;
    uint64 data;
};

#endif

