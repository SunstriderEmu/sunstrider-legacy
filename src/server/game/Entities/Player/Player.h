
#ifndef _PLAYER_H
#define _PLAYER_H

#include "ItemTemplate.h"
#include "Unit.h"
#include "Item.h"

#include "QuestDef.h"
#include "Groups/Group.h"
#include "WorldSession.h"
#include "MapReference.h"
#include "Util.h"                                           // for Tokens typedef
#include "SpellMgr.h"
#include "PlayerTaxi.h"

#include<string>
#include<vector>
#include <memory>

struct Mail;
class Channel;
class Creature;
class Pet;
class PlayerMenu;
class MotionTransport;
class UpdateMask;
class PlayerSocial;
class OutdoorPvP;
class SpectatorAddonMsg;
class ArenaTeam;
class Guild;
enum PetType : int;
enum PetSaveMode : int;
struct TrainerSpell;
class SpellCastTargets;
class PlayerAI;
class CinematicMgr;
class TradeData;
class ReputationMgr;
#ifdef PLAYERBOT
// Playerbot mod
class PlayerbotAI;
class PlayerbotTestingAI;
class PlayerbotMgr;
#endif

typedef std::deque<Mail*> PlayerMails;

#define PLAYER_MAX_SKILLS           127
#define PLAYER_MAX_DAILY_QUESTS     25
#define PLAYER_EXPLORED_ZONES_SIZE  128

// Note: SPELLMOD_* values is aura types in fact
enum SpellModType
{
    SPELLMOD_FLAT         = 107,                            // SPELL_AURA_ADD_FLAT_MODIFIER
    SPELLMOD_PCT          = 108                             // SPELL_AURA_ADD_PCT_MODIFIER
};

// 2^n values, Player::m_isunderwater is a bitmask. These are internal values, they are never send to any client
enum PlayerUnderwaterState
{
    UNDERWATER_NONE                     = 0x00,
    UNDERWATER_INWATER                  = 0x01,             // terrain type is water and player is afflicted by it
    UNDERWATER_INLAVA                   = 0x02,             // terrain type is lava and player is afflicted by it
    UNDERWATER_INSLIME                  = 0x04,             // terrain type is lava and player is afflicted by it
    UNDERWATER_INDARKWATER              = 0x08,             // terrain type is dark water and player is afflicted by it

    UNDERWATER_EXIST_TIMERS             = 0x10
};

enum BuyBankSlotResult
{
    ERR_BANKSLOT_FAILED_TOO_MANY    = 0,
    ERR_BANKSLOT_INSUFFICIENT_FUNDS = 1,
    ERR_BANKSLOT_NOTBANKER          = 2,
    ERR_BANKSLOT_OK                 = 3
};

enum PlayerSpellState
{
    PLAYERSPELL_UNCHANGED = 0,
    PLAYERSPELL_CHANGED   = 1,
    PLAYERSPELL_NEW       = 2,
    PLAYERSPELL_REMOVED   = 3,
    //TC PLAYERSPELL_TEMPORARY = 4
};

struct PlayerSpell
{
    uint16 slotId          : 16;
    PlayerSpellState state : 8;
    bool active            : 1;   // show in spellbook
    bool dependent         : 1;   // learned as result another spell learn, skill grow, quest reward, etc
    bool disabled          : 1;   // first rank has been learned in result talent learn but currently talent unlearned, save max learned ranks
};

#define SPELL_WITHOUT_SLOT_ID uint16(-1)

struct SpellModifier
{
    SpellModifier(Aura* _ownerAura) : op(SPELLMOD_DAMAGE), type(SPELLMOD_FLAT), value(0), mask(), spellId(0), ownerAura(_ownerAura) { }

    SpellModOp   op;
    SpellModType type;

    int32 value;
    uint64 mask;
    uint32 spellId;
    Aura* const ownerAura;
};

typedef std::unordered_map<uint16, PlayerSpell*> PlayerSpellMap;
typedef std::unordered_set<SpellModifier*> SpellModContainer;

struct SpellCooldown
{
    time_t end;
    uint16 itemid;
};

enum TrainerSpellState
{
    TRAINER_SPELL_GREEN = 0,
    TRAINER_SPELL_RED   = 1,
    TRAINER_SPELL_GRAY  = 2
};

enum ActionButtonUpdateState
{
    ACTIONBUTTON_UNCHANGED = 0,
    ACTIONBUTTON_CHANGED   = 1,
    ACTIONBUTTON_NEW       = 2,
    ACTIONBUTTON_DELETED   = 3
};

struct ActionButton
{
    ActionButton() : action(0), type(0), misc(0), uState( ACTIONBUTTON_NEW ) {}
    ActionButton(uint16 _action, uint8 _type, uint8 _misc) : action(_action), type(_type), misc(_misc), uState( ACTIONBUTTON_NEW ) {}

    uint16 action;
    uint8 type;
    uint8 misc;
    ActionButtonUpdateState uState;
};

enum ActionButtonType
{
    ACTION_BUTTON_SPELL = 0,
    ACTION_BUTTON_MACRO = 64,
    ACTION_BUTTON_CMACRO= 65,
    ACTION_BUTTON_ITEM  = 128
};

#define  MAX_ACTION_BUTTONS 132                             //checked in 2.3.0

enum ReputationSource
{
    REPUTATION_SOURCE_KILL,
    REPUTATION_SOURCE_QUEST,
    REPUTATION_SOURCE_DAILY_QUEST,
    REPUTATION_SOURCE_WEEKLY_QUEST,
    REPUTATION_SOURCE_MONTHLY_QUEST,
    REPUTATION_SOURCE_REPEATABLE_QUEST,
    REPUTATION_SOURCE_SPELL
};

typedef std::map<uint8,ActionButton> ActionButtonList;

enum RemovePetReason
{
    REMOVE_PET_REASON_OTHER,
    REMOVE_PET_REASON_SCRIPT,
    REMOVE_PET_REASON_PLAYER_DIED,
    REMOVE_PET_REASON_DISMISSED,
};

typedef std::pair<uint16, uint8> CreateSpellPair;

struct PlayerCreateInfoItem
{
    PlayerCreateInfoItem(uint32 id, uint32 amount) : item_id(id), item_amount(amount) {}

    uint32 item_id;
    uint32 item_amount;
};

typedef std::list<PlayerCreateInfoItem> PlayerCreateInfoItems;
struct PlayerCreateInfoSkill
{
    uint16 SkillId;
    uint16 Rank;
};
typedef std::vector<PlayerCreateInfoSkill> PlayerCreateInfoSkills;

struct PlayerClassLevelInfo
{
    PlayerClassLevelInfo() : basehealth(0), basemana(0) {}
    uint16 basehealth;
    uint16 basemana;
};

struct PlayerClassInfo
{
    PlayerClassInfo() : levelInfo(nullptr) { }

    //[level-1] 0..MaxPlayerLevel-1
    std::unique_ptr<PlayerClassLevelInfo[]> levelInfo;
};

struct PlayerLevelInfo
{
    PlayerLevelInfo() { for(unsigned char & stat : stats) stat = 0; }

    uint8 stats[MAX_STATS];
};

struct PlayerInfo
{
                                                            // existence checked by displayId != 0             // existence checked by displayId != 0
    PlayerInfo() : displayId_m(0),displayId_f(0),levelInfo(nullptr)
    {
        positionZ = 0.0f;
        positionX = 0.0f;
        positionY = 0.0f;
        positionO = 0.0f;
        mapId = 0;
        areaId = 0;
    }

    uint32 mapId;
    uint32 areaId;
    float positionX;
    float positionY;
    float positionZ;
    float positionO;
    uint16 displayId_m;
    uint16 displayId_f;
    PlayerCreateInfoItems item;
    std::list<CreateSpellPair> spell;
    PlayerCreateInfoSkills skills;
    std::list<uint16> action[4];

    std::unique_ptr<PlayerLevelInfo[]> levelInfo;
};

struct PvPInfo
{
    PvPInfo() : IsInHostileArea(false), IsHostile(false), IsInNoPvPArea(false), IsInFFAPvPArea(false), endTimer(0) {}

	bool IsHostile;
    bool IsInHostileArea;
    bool IsInNoPvPArea;                 ///> Marks if player is in a sanctuary or friendly capital city
    bool IsInFFAPvPArea;                ///> Marks if player is in an FFAPvP area (such as Gurubashi Arena)
    time_t endTimer;
};

enum DuelState
{
    DUEL_STATE_CHALLENGED,
    DUEL_STATE_COUNTDOWN,
    DUEL_STATE_IN_PROGRESS,
    DUEL_STATE_COMPLETED
};

struct DuelInfo
{
    DuelInfo(Player* opponent, Player* initiator, bool isMounted) : Opponent(opponent), Initiator(initiator), IsMounted(isMounted) {}

    Player* const Opponent;
    Player* const Initiator;
    bool const IsMounted;
    DuelState State = DUEL_STATE_CHALLENGED;
    time_t StartTime = 0;
    time_t OutOfBoundsTime = 0;
};

enum Pack58Step
{
    PACK58_STEP1,
    PACK58_MELEE,
    PACK58_HEAL,
    PACK58_TANK,
    PACK58_MAGIC,
};

enum Pack58Type
{
    PACK58_TYPE_MELEE = 0,
    PACK58_TYPE_HEAL  = 1,
    PACK58_TYPE_TANK  = 2,
    PACK58_TYPE_MAGIC = 3,
};

struct Areas
{
    uint32 areaID;
    uint32 areaFlag;
    float x1;
    float x2;
    float y1;
    float y2;
};

enum LoadData
{
    LOAD_DATA_GUID,
    LOAD_DATA_ACCOUNT,
    LOAD_DATA_NAME,
    LOAD_DATA_RACE,
    LOAD_DATA_CLASS,
    LOAD_DATA_GENDER,
    LOAD_DATA_LEVEL,
    LOAD_DATA_XP,
    LOAD_DATA_MONEY,
    LOAD_DATA_PLAYERBYTES,
    LOAD_DATA_PLAYERBYTES2,
    LOAD_DATA_PLAYERFLAGS,
    LOAD_DATA_POSX,
    LOAD_DATA_POSY,
    LOAD_DATA_POSZ,
    LOAD_DATA_MAP,
    LOAD_DATA_INSTANCE_ID,
    LOAD_DATA_ORIENTATION,
    LOAD_DATA_TAXIMASK,
    LOAD_DATA_ONLINE,
    LOAD_DATA_CINEMATIC,
    LOAD_DATA_TOTALTIME,
    LOAD_DATA_LEVELTIME,
    LOAD_DATA_LOGOUT_TIME,
    LOAD_DATA_IS_LOGOUT_RESTING,
    LOAD_DATA_REST_BONUS,
    LOAD_DATA_RESETTALENTS_COST,
    LOAD_DATA_RESETTALENTS_TIME,
    LOAD_DATA_TRANSX,
    LOAD_DATA_TRANSY,
    LOAD_DATA_TRANSZ,
    LOAD_DATA_TRANSO,
    LOAD_DATA_TRANSGUID,
    LOAD_DATA_EXTRA_FLAGS,
    LOAD_DATA_STABLE_SLOTS,
    LOAD_DATA_AT_LOGIN,
    LOAD_DATA_ZONE,
    LOAD_DATA_DEATH_EXPIRE_TIME,
    LOAD_DATA_TAXI_PATH,
    LOAD_DATA_DUNGEON_DIFF,
    LOAD_DATA_ARENA_PEND_POINTS,
    LOAD_DATA_ARENAPOINTS,
    LOAD_DATA_TOTALHONORPOINTS,
    LOAD_DATA_TODAYHONORPOINTS,
    LOAD_DATA_YESTERDAYHONORPOINTS,
    LOAD_DATA_TOTALKILLS,
    LOAD_DATA_TODAYKILLS,
    LOAD_DATA_YESTERDAYKILLS,
    LOAD_DATA_CHOSEN_TITLE,
    LOAD_DATA_WATCHED_FACTION,
    LOAD_DATA_DRUNK,
    LOAD_DATA_HEALTH,
    LOAD_DATA_POWER1,
    LOAD_DATA_POWER2,
    LOAD_DATA_POWER3,
    LOAD_DATA_POWER4,
    LOAD_DATA_POWER5,
    LOAD_DATA_EXPLOREDZONES,
    LOAD_DATA_EQUIPMENTCACHE,
    LOAD_DATA_AMMOID,
    LOAD_DATA_KNOWNTITLES,
    LOAD_DATA_ACTIONBARS,
    LOAD_DATA_XP_BLOCKED,
    LOAD_DATA_CUSTOM_XP
};

struct EnchantDuration
{
    EnchantDuration() : item(nullptr), slot(MAX_ENCHANTMENT_SLOT), leftduration(0) {};
    EnchantDuration(Item * _item, EnchantmentSlot _slot, uint32 _leftduration) : item(_item), slot(_slot), leftduration(_leftduration) { assert(item); };

    Item * item;
    EnchantmentSlot slot;
    uint32 leftduration;
};

typedef std::list<EnchantDuration> EnchantDurationList;
typedef std::list<Item*> ItemDurationList;

struct LookingForGroupSlot
{
    LookingForGroupSlot() : entry(0), type(0) {}
    bool Empty() const { return !entry && !type; }
    void Clear() { entry = 0; type = 0; }
    void Set(uint32 _entry, uint32 _type ) { entry = _entry; type = _type; }
    bool Is(uint32 _entry, uint32 _type) const { return entry==_entry && type==_type; }
    bool canAutoJoin() const { return entry && (type == 1 || type == 5); }

    uint32 entry;
    uint32 type;
};

#define MAX_LOOKING_FOR_GROUP_SLOT 3

struct LookingForGroup
{
    LookingForGroup() {}
    bool HaveInSlot(LookingForGroupSlot const& slot) const { return HaveInSlot(slot.entry,slot.type); }
    bool HaveInSlot(uint32 _entry, uint32 _type) const
    {
        for(auto slot : slots)
            if(slot.Is(_entry,_type))
                return true;
        return false;
    }

    bool canAutoJoin() const
    {
        for(auto slot : slots)
            if(slot.canAutoJoin())
                return true;
        return false;
    }

    bool Empty() const
    {
        for(auto slot : slots)
            if(!slot.Empty())
                return false;
        return more.Empty();
    }

    LookingForGroupSlot slots[MAX_LOOKING_FOR_GROUP_SLOT];
    LookingForGroupSlot more;
    std::string comment;
};

enum DrunkenState
{
    DRUNKEN_SOBER   = 0,
    DRUNKEN_TIPSY   = 1,
    DRUNKEN_DRUNK   = 2,
    DRUNKEN_SMASHED = 3
};

enum PlayerStateType
{
    /*
        PLAYER_STATE_DANCE
        PLAYER_STATE_SLEEP
        PLAYER_STATE_SIT
        PLAYER_STATE_STAND
        PLAYER_STATE_READYUNARMED
        PLAYER_STATE_WORK
        PLAYER_STATE_POINT(DNR)
        PLAYER_STATE_NONE // not used or just no state, just standing there?
        PLAYER_STATE_STUN
        PLAYER_STATE_DEAD
        PLAYER_STATE_KNEEL
        PLAYER_STATE_USESTANDING
        PLAYER_STATE_STUN_NO_SHEATHE
        PLAYER_STATE_USESTANDING_NO_SHEATHE
        PLAYER_STATE_WORK_NO_SHEATHE
        PLAYER_STATE_SPELLPRECAST
        PLAYER_STATE_READYRIFLE
        PLAYER_STATE_WORK_NO_SHEATHE_MINING
        PLAYER_STATE_WORK_NO_SHEATHE_CHOPWOOD
        PLAYER_STATE_AT_EASE
        PLAYER_STATE_READY1H
        PLAYER_STATE_SPELLKNEELSTART
        PLAYER_STATE_SUBMERGED
    */

    PLAYER_STATE_NONE              = 0,
    PLAYER_STATE_SIT               = 1,
    PLAYER_STATE_SIT_CHAIR         = 2,
    PLAYER_STATE_SLEEP             = 3,
    PLAYER_STATE_SIT_LOW_CHAIR     = 4,
    PLAYER_STATE_SIT_MEDIUM_CHAIR  = 5,
    PLAYER_STATE_SIT_HIGH_CHAIR    = 6,
    PLAYER_STATE_DEAD              = 7,
    PLAYER_STATE_KNEEL             = 8,

    PLAYER_STATE_FORM_ALL          = 0x00FF0000,

    PLAYER_STATE_FLAG_UNTRACKABLE  = 0x04000000,
    PLAYER_STATE_FLAG_ALL          = 0xFF000000,
};

enum PlayerFlags
{
    PLAYER_FLAGS_GROUP_LEADER       = 0x00000001,
    PLAYER_FLAGS_AFK                = 0x00000002,
    PLAYER_FLAGS_DND                = 0x00000004,
    PLAYER_FLAGS_GM                 = 0x00000008,
    PLAYER_FLAGS_GHOST              = 0x00000010,
    PLAYER_FLAGS_RESTING            = 0x00000020,
    PLAYER_FLAGS_FFA_PVP            = 0x00000080,
    PLAYER_FLAGS_CONTESTED_PVP      = 0x00000100,               // Player has been involved in a PvP combat and will be attacked by contested guards
    PLAYER_FLAGS_IN_PVP             = 0x00000200,
    PLAYER_FLAGS_HIDE_HELM          = 0x00000400,
    PLAYER_FLAGS_HIDE_CLOAK         = 0x00000800,
    PLAYER_FLAGS_PLAYED_LONG_TIME   = 0x00001000,               // played long time
    PLAYER_FLAGS_PLAYED_TOO_LONG    = 0x00002000,               // played too long time
    PLAYER_FLAGS_IS_OUT_OF_BOUNDS   = 0x00004000,
    PLAYER_FLAGS_DEVELOPER          = 0x00008000,               // <Dev> prefix for something?
    PLAYER_FLAGS_SANCTUARY          = 0x00010000,               // player entered sanctuary
    PLAYER_FLAGS_TAXI_BENCHMARK     = 0x00020000,               // taxi benchmark mode (on/off) (2.0.1)
    PLAYER_FLAGS_PVP_COUNTER        = 0x00040000,               // 2.0.8...
    PLAYER_FLAGS_COMMENTATOR        = 0x00080000,
    PLAYER_FLAGS_UNK5               = 0x00100000,
    PLAYER_FLAGS_UNK6               = 0x00200000,
    PLAYER_FLAGS_COMMENTATOR_UBER   = 0x00400000
};

enum PlayerBytesOffsets : uint8
{
    PLAYER_BYTES_OFFSET_SKIN_ID         = 0,
    PLAYER_BYTES_OFFSET_FACE_ID         = 1,
    PLAYER_BYTES_OFFSET_HAIR_STYLE_ID   = 2,
    PLAYER_BYTES_OFFSET_HAIR_COLOR_ID   = 3
};

enum PlayerBytes2Offsets : uint8
{
    PLAYER_BYTES_2_OFFSET_FACIAL_STYLE      = 0,
	//BC OK ?   PLAYER_BYTES_2_OFFSET_PARTY_TYPE        = 1,
	//BC OK ?   PLAYER_BYTES_2_OFFSET_BANK_BAG_SLOTS    = 2,
	//BC OK ?   PLAYER_BYTES_2_OFFSET_REST_STATE        = 3
};
enum PlayerBytes3Offsets : uint8
{
    PLAYER_BYTES_3_OFFSET_GENDER = 0,
    PLAYER_BYTES_3_OFFSET_INEBRIATION = 1,
    //BC OK ? PLAYER_BYTES_3_OFFSET_PVP_TITLE = 2,
    //BC OK ? PLAYER_BYTES_3_OFFSET_ARENA_FACTION = 3
};

enum PlayerFieldBytesOffsets
{
    //BC OK ? PLAYER_FIELD_BYTES_OFFSET_FLAGS                 = 0,
    //BC OK ? PLAYER_FIELD_BYTES_OFFSET_RAF_GRANTABLE_LEVEL   = 1,
    PLAYER_FIELD_BYTES_OFFSET_ACTION_BAR_TOGGLES    = 2,
    //BC OK ? PLAYER_FIELD_BYTES_OFFSET_LIFETIME_MAX_PVP_RANK = 3
};

enum PlayerFieldBytes2Offsets
{
#ifdef LICH_KING
    PLAYER_FIELD_BYTES_2_OFFSET_OVERRIDE_SPELLS_ID                  = 0,    // uint16!
    PLAYER_FIELD_BYTES_2_OFFSET_IGNORE_POWER_REGEN_PREDICTION_MASK  = 2,
    PLAYER_FIELD_BYTES_2_OFFSET_AURA_VISION                         = 3
#else
	//BC OK ? PLAYER_FIELD_BYTES_2_OFFSET_OVERRIDE_SPELLS_ID = 0,    // uint16!
	PLAYER_FIELD_BYTES_2_OFFSET_AURA_VISION = 1,
	//BC OK ? PLAYER_FIELD_BYTES_2_OFFSET_IGNORE_POWER_REGEN_PREDICTION_MASK = 2,
#endif
};

// used in PLAYER_FIELD_BYTES values
enum PlayerFieldByteFlags
{
	PLAYER_FIELD_BYTE_TRACK_STEALTHED	= 0x00000002,
	PLAYER_FIELD_BYTE_RELEASE_TIMER		= 0x00000008,       // Display time till auto release spirit
	PLAYER_FIELD_BYTE_NO_RELEASE_WINDOW = 0x00000010        // Display no "release spirit" window at all
};

// used in PLAYER_FIELD_BYTES2 values (with offsets in PlayerFieldBytes2Offsets)
enum PlayerFieldByte2Flags
{
    PLAYER_FIELD_BYTE2_NONE                 = 0x00,
    PLAYER_FIELD_BYTE2_STEALTH              = 0x20,
    PLAYER_FIELD_BYTE2_INVISIBILITY_GLOW    = 0x40
};


#define PLAYER_TITLE_MASK_ALLIANCE_PVP  \
    (PLAYER_TITLE_PRIVATE | PLAYER_TITLE_CORPORAL |  \
      PLAYER_TITLE_SERGEANT_A | PLAYER_TITLE_MASTER_SERGEANT | \
      PLAYER_TITLE_SERGEANT_MAJOR | PLAYER_TITLE_KNIGHT | \
      PLAYER_TITLE_KNIGHT_LIEUTENANT | PLAYER_TITLE_KNIGHT_CAPTAIN | \
      PLAYER_TITLE_KNIGHT_CHAMPION | PLAYER_TITLE_LIEUTENANT_COMMANDER | \
      PLAYER_TITLE_COMMANDER | PLAYER_TITLE_MARSHAL | \
      PLAYER_TITLE_FIELD_MARSHAL | PLAYER_TITLE_GRAND_MARSHAL)

#define PLAYER_TITLE_MASK_HORDE_PVP  \
    (PLAYER_TITLE_SCOUT | PLAYER_TITLE_GRUNT |  \
      PLAYER_TITLE_SERGEANT_H | PLAYER_TITLE_SENIOR_SERGEANT | \
      PLAYER_TITLE_FIRST_SERGEANT | PLAYER_TITLE_STONE_GUARD | \
      PLAYER_TITLE_BLOOD_GUARD | PLAYER_TITLE_LEGIONNAIRE | \
      PLAYER_TITLE_CENTURION | PLAYER_TITLE_CHAMPION | \
      PLAYER_TITLE_LIEUTENANT_GENERAL | PLAYER_TITLE_GENERAL | \
      PLAYER_TITLE_WARLORD | PLAYER_TITLE_HIGH_WARLORD)

#define PLAYER_TITLE_MASK_ALL_PVP  \
    (PLAYER_TITLE_MASK_ALLIANCE_PVP | PLAYER_TITLE_MASK_HORDE_PVP)

// used for PLAYER__FIELD_KNOWN_TITLES field (uint64), (1<<bit_index) without (-1)
// can't use enum for uint64 values
#define PLAYER_TITLE_DISABLED              0x0000000000000000LL
#define PLAYER_TITLE_NONE                  0x0000000000000001LL
#define PLAYER_TITLE_PRIVATE               0x0000000000000002LL // 1
#define PLAYER_TITLE_CORPORAL              0x0000000000000004LL // 2
#define PLAYER_TITLE_SERGEANT_A            0x0000000000000008LL // 3
#define PLAYER_TITLE_MASTER_SERGEANT       0x0000000000000010LL // 4
#define PLAYER_TITLE_SERGEANT_MAJOR        0x0000000000000020LL // 5
#define PLAYER_TITLE_KNIGHT                0x0000000000000040LL // 6
#define PLAYER_TITLE_KNIGHT_LIEUTENANT     0x0000000000000080LL // 7
#define PLAYER_TITLE_KNIGHT_CAPTAIN        0x0000000000000100LL // 8
#define PLAYER_TITLE_KNIGHT_CHAMPION       0x0000000000000200LL // 9
#define PLAYER_TITLE_LIEUTENANT_COMMANDER  0x0000000000000400LL // 10
#define PLAYER_TITLE_COMMANDER             0x0000000000000800LL // 11
#define PLAYER_TITLE_MARSHAL               0x0000000000001000LL // 12
#define PLAYER_TITLE_FIELD_MARSHAL         0x0000000000002000LL // 13
#define PLAYER_TITLE_GRAND_MARSHAL         0x0000000000004000LL // 14
#define PLAYER_TITLE_SCOUT                 0x0000000000008000LL // 15
#define PLAYER_TITLE_GRUNT                 0x0000000000010000LL // 16
#define PLAYER_TITLE_SERGEANT_H            0x0000000000020000LL // 17
#define PLAYER_TITLE_SENIOR_SERGEANT       0x0000000000040000LL // 18
#define PLAYER_TITLE_FIRST_SERGEANT        0x0000000000080000LL // 19
#define PLAYER_TITLE_STONE_GUARD           0x0000000000100000LL // 20
#define PLAYER_TITLE_BLOOD_GUARD           0x0000000000200000LL // 21
#define PLAYER_TITLE_LEGIONNAIRE           0x0000000000400000LL // 22
#define PLAYER_TITLE_CENTURION             0x0000000000800000LL // 23
#define PLAYER_TITLE_CHAMPION              0x0000000001000000LL // 24
#define PLAYER_TITLE_LIEUTENANT_GENERAL    0x0000000002000000LL // 25
#define PLAYER_TITLE_GENERAL               0x0000000004000000LL // 26
#define PLAYER_TITLE_WARLORD               0x0000000008000000LL // 27
#define PLAYER_TITLE_HIGH_WARLORD          0x0000000010000000LL // 28
#define PLAYER_TITLE_GLADIATOR             0x0000000020000000LL // 29
#define PLAYER_TITLE_DUELIST               0x0000000040000000LL // 30
#define PLAYER_TITLE_RIVAL                 0x0000000080000000LL // 31
#define PLAYER_TITLE_CHALLENGER            0x0000000100000000LL // 32
#define PLAYER_TITLE_SCARAB_LORD           0x0000000200000000LL // 33
#define PLAYER_TITLE_CONQUEROR             0x0000000400000000LL // 34
#define PLAYER_TITLE_JUSTICAR              0x0000000800000000LL // 35
#define PLAYER_TITLE_CHAMPION_OF_THE_NAARU 0x0000001000000000LL // 36
#define PLAYER_TITLE_MERCILESS_GLADIATOR   0x0000002000000000LL // 37
#define PLAYER_TITLE_OF_THE_SHATTERED_SUN  0x0000004000000000LL // 38
#define PLAYER_TITLE_HAND_OF_ADAL          0x0000008000000000LL // 39
#define PLAYER_TITLE_VENGEFUL_GLADIATOR    0x0000010000000000LL // 40

#define KNOWN_TITLES_SIZE   3
#define MAX_TITLE_INDEX     (KNOWN_TITLES_SIZE*64)          // 3 uint64 fields

enum MirrorTimerType
{
    FATIGUE_TIMER      = 0,
    BREATH_TIMER       = 1,
    FIRE_TIMER         = 2
};
#define MAX_TIMERS 3
#define DISABLED_MIRROR_TIMER   -1

// 2^n values
enum PlayerExtraFlags
{
    // gm abilities
    PLAYER_EXTRA_GM_ON              = 0x0001,
    PLAYER_EXTRA_ACCEPT_WHISPERS    = 0x0004,
    PLAYER_EXTRA_TAXICHEAT          = 0x0008,
    PLAYER_EXTRA_GM_INVISIBLE       = 0x0010,
    PLAYER_EXTRA_GM_CHAT            = 0x0020,               // Show GM badge in chat messages

    // other states
    PLAYER_EXTRA_DUEL_AREA          = 0x0040,              // currently in duel area
    PLAYER_EXTRA_PVP_DEATH          = 0x0100               // store PvP death status until corpse creating.
};

// 2^n values
enum AtLoginFlags
{
    AT_LOGIN_NONE          = 0x00,
    AT_LOGIN_RENAME        = 0x01,
    AT_LOGIN_RESET_SPELLS  = 0x02,
    AT_LOGIN_RESET_TALENTS = 0x04,
    AT_LOGIN_SET_DESERTER  = 0x08,
    AT_LOGIN_RESET_FLYS    = 0x10,
    //reuse
    AT_LOGIN_FIRST         = 0x40,
	//0x80
	AT_LOGIN_RESURRECT     = 0x100,
};

typedef std::map<uint32, QuestStatusData> QuestStatusMap;
typedef std::set<uint32> RewardedQuestSet;

enum QuestSaveType
{
    QUEST_DEFAULT_SAVE_TYPE = 0,
    QUEST_DELETE_SAVE_TYPE,
    QUEST_FORCE_DELETE_SAVE_TYPE
};

//               quest
typedef std::map<uint32, QuestSaveType> QuestStatusSaveMap;

enum QuestSlotOffsets
{
    QUEST_ID_OFFSET     = 0,
    QUEST_STATE_OFFSET  = 1,
    QUEST_COUNTS_OFFSET = 2,
    QUEST_TIME_OFFSET   = 3
};

#define MAX_QUEST_OFFSET 4

enum QuestSlotStateMask
{
    QUEST_STATE_NONE     = 0x0000,
    QUEST_STATE_COMPLETE = 0x0001,
    QUEST_STATE_FAIL     = 0x0002,
};

enum SkillUpdateState
{
    SKILL_UNCHANGED     = 0,
    SKILL_CHANGED       = 1,
    SKILL_NEW           = 2,
    SKILL_DELETED       = 3
};

struct SkillStatusData
{
    SkillStatusData(uint8 _pos, SkillUpdateState _uState) : pos(_pos), uState(_uState)
    {
    }
    
    uint8 pos;
    SkillUpdateState uState;
};

typedef std::unordered_map<uint32, SkillStatusData> SkillStatusMap;

class Quest;
class Spell;
class Item;
class WorldSession;

enum PlayerSlots
{
    // first slot for item stored (in any way in player m_items data)
    PLAYER_SLOT_START           = 0,
    // last+1 slot for item stored (in any way in player m_items data)
    PLAYER_SLOT_END             = 118,
    PLAYER_SLOTS_COUNT          = (PLAYER_SLOT_END - PLAYER_SLOT_START)
};

enum EquipmentSlots
{
    EQUIPMENT_SLOT_START        = 0,
    EQUIPMENT_SLOT_HEAD         = 0,
    EQUIPMENT_SLOT_NECK         = 1,
    EQUIPMENT_SLOT_SHOULDERS    = 2,
    EQUIPMENT_SLOT_BODY         = 3,
    EQUIPMENT_SLOT_CHEST        = 4,
    EQUIPMENT_SLOT_WAIST        = 5,
    EQUIPMENT_SLOT_LEGS         = 6,
    EQUIPMENT_SLOT_FEET         = 7,
    EQUIPMENT_SLOT_WRISTS       = 8,
    EQUIPMENT_SLOT_HANDS        = 9,
    EQUIPMENT_SLOT_FINGER1      = 10,
    EQUIPMENT_SLOT_FINGER2      = 11,
    EQUIPMENT_SLOT_TRINKET1     = 12,
    EQUIPMENT_SLOT_TRINKET2     = 13,
    EQUIPMENT_SLOT_BACK         = 14,
    EQUIPMENT_SLOT_MAINHAND     = 15,
    EQUIPMENT_SLOT_OFFHAND      = 16,
    EQUIPMENT_SLOT_RANGED       = 17,
    EQUIPMENT_SLOT_TABARD       = 18,
    EQUIPMENT_SLOT_END          = 19
};

enum InventorySlots
{
    INVENTORY_SLOT_BAG_0        = 255,
    INVENTORY_SLOT_BAG_START    = 19,
    INVENTORY_SLOT_BAG_1        = 19,
    INVENTORY_SLOT_BAG_2        = 20,
    INVENTORY_SLOT_BAG_3        = 21,
    INVENTORY_SLOT_BAG_4        = 22,
    INVENTORY_SLOT_BAG_END      = 23,

    INVENTORY_SLOT_ITEM_START   = 23,
    INVENTORY_SLOT_ITEM_1       = 23,
    INVENTORY_SLOT_ITEM_2       = 24,
    INVENTORY_SLOT_ITEM_3       = 25,
    INVENTORY_SLOT_ITEM_4       = 26,
    INVENTORY_SLOT_ITEM_5       = 27,
    INVENTORY_SLOT_ITEM_6       = 28,
    INVENTORY_SLOT_ITEM_7       = 29,
    INVENTORY_SLOT_ITEM_8       = 30,
    INVENTORY_SLOT_ITEM_9       = 31,
    INVENTORY_SLOT_ITEM_10      = 32,
    INVENTORY_SLOT_ITEM_11      = 33,
    INVENTORY_SLOT_ITEM_12      = 34,
    INVENTORY_SLOT_ITEM_13      = 35,
    INVENTORY_SLOT_ITEM_14      = 36,
    INVENTORY_SLOT_ITEM_15      = 37,
    INVENTORY_SLOT_ITEM_16      = 38,
    INVENTORY_SLOT_ITEM_END     = 39
};

enum BankSlots
{
    BANK_SLOT_ITEM_START        = 39,
    BANK_SLOT_ITEM_1            = 39,
    BANK_SLOT_ITEM_2            = 40,
    BANK_SLOT_ITEM_3            = 41,
    BANK_SLOT_ITEM_4            = 42,
    BANK_SLOT_ITEM_5            = 43,
    BANK_SLOT_ITEM_6            = 44,
    BANK_SLOT_ITEM_7            = 45,
    BANK_SLOT_ITEM_8            = 46,
    BANK_SLOT_ITEM_9            = 47,
    BANK_SLOT_ITEM_10           = 48,
    BANK_SLOT_ITEM_11           = 49,
    BANK_SLOT_ITEM_12           = 50,
    BANK_SLOT_ITEM_13           = 51,
    BANK_SLOT_ITEM_14           = 52,
    BANK_SLOT_ITEM_15           = 53,
    BANK_SLOT_ITEM_16           = 54,
    BANK_SLOT_ITEM_17           = 55,
    BANK_SLOT_ITEM_18           = 56,
    BANK_SLOT_ITEM_19           = 57,
    BANK_SLOT_ITEM_20           = 58,
    BANK_SLOT_ITEM_21           = 59,
    BANK_SLOT_ITEM_22           = 60,
    BANK_SLOT_ITEM_23           = 61,
    BANK_SLOT_ITEM_24           = 62,
    BANK_SLOT_ITEM_25           = 63,
    BANK_SLOT_ITEM_26           = 64,
    BANK_SLOT_ITEM_27           = 65,
    BANK_SLOT_ITEM_28           = 66,
    BANK_SLOT_ITEM_END          = 67,

    BANK_SLOT_BAG_START         = 67,
    BANK_SLOT_BAG_1             = 67,
    BANK_SLOT_BAG_2             = 68,
    BANK_SLOT_BAG_3             = 69,
    BANK_SLOT_BAG_4             = 70,
    BANK_SLOT_BAG_5             = 71,
    BANK_SLOT_BAG_6             = 72,
    BANK_SLOT_BAG_7             = 73,
    BANK_SLOT_BAG_END           = 74
};

enum BuyBackSlots
{
    // stored in m_buybackitems
    BUYBACK_SLOT_START          = 74,
    BUYBACK_SLOT_1              = 74,
    BUYBACK_SLOT_2              = 75,
    BUYBACK_SLOT_3              = 76,
    BUYBACK_SLOT_4              = 77,
    BUYBACK_SLOT_5              = 78,
    BUYBACK_SLOT_6              = 79,
    BUYBACK_SLOT_7              = 80,
    BUYBACK_SLOT_8              = 81,
    BUYBACK_SLOT_9              = 82,
    BUYBACK_SLOT_10             = 83,
    BUYBACK_SLOT_11             = 84,
    BUYBACK_SLOT_12             = 85,
    BUYBACK_SLOT_END            = 86
};

enum KeyRingSlots
{
    KEYRING_SLOT_START          = 86,
    KEYRING_SLOT_END            = 118
};

struct ItemPosCount
{
    ItemPosCount(uint16 _pos, uint8 _count) : pos(_pos), count(_count) {}
    bool isContainedIn(std::vector<ItemPosCount> const& vec) const;
    uint16 pos;
    uint8 count;
};
typedef std::vector<ItemPosCount> ItemPosCountVec;

enum TransferAbortReason
{
    TRANSFER_ABORT_NONE                     = 0x00,
#ifdef LICH_KING
    TRANSFER_ABORT_ERROR                    = 0x01,
    TRANSFER_ABORT_MAX_PLAYERS              = 0x02,         // Transfer Aborted: instance is full
    TRANSFER_ABORT_NOT_FOUND                = 0x03,         // Transfer Aborted: instance not found
    TRANSFER_ABORT_TOO_MANY_INSTANCES       = 0x04,         // You have entered too many instances recently.
    TRANSFER_ABORT_ZONE_IN_COMBAT           = 0x06,         // Unable to zone in while an encounter is in progress.
    TRANSFER_ABORT_INSUF_EXPAN_LVL          = 0x07,         // You must have <TBC, WotLK> expansion installed to access this area.
    TRANSFER_ABORT_DIFFICULTY               = 0x08,         // <Normal, Heroic, Epic> difficulty mode is not available for %s.
    TRANSFER_ABORT_UNIQUE_MESSAGE           = 0x09,         // Until you've escaped TLK's grasp, you cannot leave this place!
    TRANSFER_ABORT_TOO_MANY_REALM_INSTANCES = 0x0A,         // Additional instances cannot be launched, please try again later.
    TRANSFER_ABORT_NEED_GROUP               = 0x0B,         // 3.1
    TRANSFER_ABORT_NOT_FOUND1               = 0x0C,         // 3.1
    TRANSFER_ABORT_NOT_FOUND2               = 0x0D,         // 3.1
    TRANSFER_ABORT_NOT_FOUND3               = 0x0E,         // 3.2
    TRANSFER_ABORT_REALM_ONLY               = 0x0F,         // All players on party must be from the same realm.
    TRANSFER_ABORT_MAP_NOT_ALLOWED          = 0x10,         // Map can't be entered at this time.
#else
    TRANSFER_ABORT_MAX_PLAYERS              = 0x0001,       // Transfer Aborted: instance is full
    TRANSFER_ABORT_NOT_FOUND                = 0x0002,       // Transfer Aborted: instance not found
    TRANSFER_ABORT_TOO_MANY_INSTANCES       = 0x0003,       // You have entered too many instances recently.
    TRANSFER_ABORT_ZONE_IN_COMBAT           = 0x0005,       // Unable to zone in while an encounter is in progress.
    TRANSFER_ABORT_INSUF_EXPAN_LVL          = 0x0106,       // You must have TBC expansion installed to access this area.
    TRANSFER_ABORT_DIFFICULTY1              = 0x0007,       // Normal difficulty mode is not available for %s.
    TRANSFER_ABORT_DIFFICULTY2              = 0x0107,       // Heroic difficulty mode is not available for %s.
    TRANSFER_ABORT_DIFFICULTY3              = 0x0207,       // Epic difficulty mode is not available for %s.
#endif
};

enum InstanceResetWarningType
{
    RAID_INSTANCE_WARNING_HOURS     = 1,                    // WARNING! %s is scheduled to reset in %d hour(s).
    RAID_INSTANCE_WARNING_MIN       = 2,                    // WARNING! %s is scheduled to reset in %d minute(s)!
    RAID_INSTANCE_WARNING_MIN_SOON  = 3,                    // WARNING! %s is scheduled to reset in %d minute(s). Please exit the zone or you will be returned to your bind location!
    RAID_INSTANCE_WELCOME           = 4                     // Welcome to %s. This raid instance is scheduled to reset in %s.
};

class InstanceSave;

enum RestType
{
    REST_TYPE_NO        = 0,
    REST_TYPE_IN_TAVERN = 1,
    REST_TYPE_IN_CITY   = 2
};

enum TeleportToOptions
{
    TELE_TO_GM_MODE             = 0x01,
    TELE_TO_NOT_LEAVE_TRANSPORT = 0x02,
    TELE_TO_NOT_LEAVE_COMBAT    = 0x04,
    TELE_TO_NOT_UNSUMMON_PET    = 0x08,
    TELE_TO_SPELL               = 0x10,
    TELE_TO_TRANSPORT_TELEPORT  = 0x20,
    TELE_REVIVE_AT_TELEPORT     = 0x40,
    TELE_TO_TEST_MODE           = 0x80,
    TELE_TO_FORCE_RELOAD        = 0x100, // Force leaving and re entering world. Use this when joining another instance when unit is in the same map (mapid)
};

/// Type of environmental damages
enum EnviromentalDamage
{
    DAMAGE_EXHAUSTED = 0,
    DAMAGE_DROWNING  = 1,
    DAMAGE_FALL      = 2,
    DAMAGE_LAVA      = 3,
    DAMAGE_SLIME     = 4,
    DAMAGE_FIRE      = 5,
    DAMAGE_FALL_TO_VOID = 6                                 // custom case for fall without durability loss
};

enum PlayedTimeIndex
{
    PLAYED_TIME_TOTAL = 0,
    PLAYED_TIME_LEVEL = 1
};

#define MAX_PLAYED_TIME_INDEX 2

enum PlayerChatTag
{
    CHAT_TAG_NONE       = 0x00,
    CHAT_TAG_AFK        = 0x01,
    CHAT_TAG_DND        = 0x02,
    CHAT_TAG_GM         = 0x04,
    CHAT_TAG_COM        = 0x08, // Commentator
    CHAT_TAG_DEV        = 0x10
};

// used at player loading query list preparing, and later result selection
enum PlayerLoginQueryIndex
{
    PLAYER_LOGIN_QUERY_LOAD_FROM              = 0,
    PLAYER_LOGIN_QUERY_LOAD_GROUP                ,
    PLAYER_LOGIN_QUERY_LOAD_BOUND_INSTANCES      ,
    PLAYER_LOGIN_QUERY_LOAD_AURAS                ,
    PLAYER_LOGIN_QUERY_LOAD_SPELLS               ,
    PLAYER_LOGIN_QUERY_LOAD_QUEST_STATUS         ,
    PLAYER_LOGIN_QUERY_LOAD_DAILY_QUEST_STATUS   ,
    PLAYER_LOGIN_QUERY_LOAD_REPUTATION           ,
    PLAYER_LOGIN_QUERY_LOAD_INVENTORY            ,
    PLAYER_LOGIN_QUERY_LOAD_ACTIONS              ,
    PLAYER_LOGIN_QUERY_LOAD_MAIL_COUNT           ,
    PLAYER_LOGIN_QUERY_LOAD_MAIL_DATE            ,
    PLAYER_LOGIN_QUERY_LOAD_SOCIAL_LIST          ,
    PLAYER_LOGIN_QUERY_LOAD_HOME_BIND            ,
    PLAYER_LOGIN_QUERY_LOAD_SPELL_COOLDOWNS      ,
    PLAYER_LOGIN_QUERY_LOAD_DECLINED_NAMES       ,
    PLAYER_LOGIN_QUERY_LOAD_GUILD                ,
    PLAYER_LOGIN_QUERY_LOAD_ARENA_INFO           ,
    PLAYER_LOGIN_QUERY_LOAD_SKILLS               ,
    PLAYER_LOGIN_QUERY_LOAD_BG_DATA              ,
    PLAYER_LOGIN_QUERY_LOAD_CORPSE_LOCATION      ,
    PLAYER_LOGIN_QUERY_LOAD_QUEST_STATUS_REW     ,
    PLAYER_LOGIN_QUERY_LOAD_SEASONAL_QUEST_STATUS,
#ifdef LICH_KING
    PLAYER_LOGIN_QUERY_LOAD_RANDOM_BG,
    PLAYER_LOGIN_QUERY_LOAD_WEEKLY_QUEST_STATUS  ,
    PLAYER_LOGIN_QUERY_LOAD_MONTHLY_QUEST_STATUS ,
#endif

    MAX_PLAYER_LOGIN_QUERY
};

enum PlayerDelayedOperations
{
    DELAYED_SAVE_PLAYER         = 0x01,
    DELAYED_RESURRECT_PLAYER    = 0x02,
    DELAYED_SPELL_CAST_DESERTER = 0x04,
    DELAYED_BG_MOUNT_RESTORE    = 0x08,                     ///< Flag to restore mount state after teleport from BG
    DELAYED_BG_TAXI_RESTORE     = 0x10,                     ///< Flag to restore taxi state after teleport from BG
    DELAYED_BG_GROUP_RESTORE    = 0x20,                     ///< Flag to restore group state after teleport from BG
    DELAYED_END
};

// Player summoning auto-decline time (in secs)
#define MAX_PLAYER_SUMMON_DELAY                   (2*MINUTE)
// Maximum money amount : 2^31 - 1
TC_GAME_API extern uint32 const MAX_MONEY_AMOUNT;

struct InstancePlayerBind
{
    InstanceSave *save;
    bool perm;
    /* permanent PlayerInstanceBinds are created in Raid/Heroic instances for players
       that aren't already permanently bound when they are inside when a boss is killed
       or when they enter an instance that the group leader is permanently bound to. */
    
    /*LK
    // extend state listing:
    //EXPIRED  - doesn't affect anything unless manually re-extended by player
    //NORMAL   - standard state
    //EXTENDED - won't be promoted to EXPIRED at next reset period, will instead be promoted to NORMAL
    //BindExtensionState extendState;
    */
    InstancePlayerBind() : save(nullptr), perm(false) {}
};

enum CharDeleteMethod
{
    CHAR_DELETE_REMOVE = 0,                      // Completely remove from the database
    CHAR_DELETE_UNLINK = 1                       // The character gets unlinked from the account,
                                                 // the name gets freed up and appears as deleted ingame
};

enum PlayerCommandStates
{
    CHEAT_NONE      = 0x00,
    CHEAT_GOD       = 0x01,
    CHEAT_CASTTIME  = 0x02,
    CHEAT_COOLDOWN  = 0x04,
    CHEAT_POWER     = 0x08,
    CHEAT_WATERWALK = 0x10
};

struct SpamReport
{
    time_t time;
    ObjectGuid reporterGUID;
    std::string message;
};

typedef std::map<ObjectGuid::LowType, SpamReport> SpamReports;

struct Gladiator {
    Gladiator(ObjectGuid::LowType guid, uint8 r) : playerguid(guid), rank(r) {}
    ObjectGuid::LowType playerguid;
    uint8 rank;
};

class LoginQueryHolder : public SQLQueryHolder
{
private:
    uint32 m_accountId;
    ObjectGuid m_guid;
public:
    LoginQueryHolder(uint32 accountId, ObjectGuid guid)
        : m_accountId(accountId), m_guid(guid) { }
    ObjectGuid GetGuid() const { return m_guid; }
    uint32 GetAccountId() const { return m_accountId; }
    bool Initialize();
};

#define MAX_GLADIATORS_RANK 3

/// Holder for Battleground data
struct BGData
{
    BGData() : bgInstanceID(0), bgTypeID(BATTLEGROUND_TYPE_NONE), bgAfkReportedCount(0), bgAfkReportedTimer(0),
        bgTeam(0), mountSpell(0) {
        ClearTaxiPath();
    }

    uint32 bgInstanceID;                    ///< This variable is set to bg->m_InstanceID,
                                            ///  when player is teleported to BG - (it is battleground's GUID)
    BattlegroundTypeId bgTypeID;

    std::set<uint32>   bgAfkReporter;
    uint8              bgAfkReportedCount;
    time_t             bgAfkReportedTimer;

    uint32 bgTeam;                          ///< What side the player will be added to

    uint32 mountSpell;
    uint32 taxiPath[2];

    WorldLocation joinPos;                  ///< From where player entered BG

    void ClearTaxiPath() { taxiPath[0] = taxiPath[1] = 0; }
    bool HasTaxiPath() const { return taxiPath[0] && taxiPath[1]; }
};

struct TradeStatusInfo
{
    TradeStatusInfo() : Status(TRADE_STATUS_BUSY), TraderGuid(), Result(EQUIP_ERR_OK),
        IsTargetResult(false), ItemLimitCategoryId(0), Slot(0) { }

    TradeStatus Status;
    ObjectGuid TraderGuid;
    InventoryResult Result;
    bool IsTargetResult;
    uint32 ItemLimitCategoryId;
    uint8 Slot;
};

class TC_GAME_API Player : public Unit, public GridObject<Player>
{
    friend class WorldSession;
    friend class Spell;
    friend void Item::AddItemToUpdateQueueOf(Player *player);
    friend void Item::RemoveItemFromUpdateQueueOf(Player *player);
    public:
        explicit Player (WorldSession *session);
        ~Player() override;

        PlayerAI* AI() const { return reinterpret_cast<PlayerAI*>(GetAI()); }

        void CleanupsBeforeDelete(bool finalCleanup = true) override;

        void AddToWorld() override;
        void RemoveFromWorld() override;

		void StopCastingCharm(Aura* except = nullptr);
        void StopCastingBindSight(Aura* except = nullptr);

        void SetTeleportingToTest(uint32 instanceId);
        uint32 GetTeleportingToTest() const { return m_teleportToTestInstanceId; }
        bool TeleportTo(uint32 mapid, float x, float y, float z, float orientation, uint32 options = 0);

        bool TeleportTo(WorldLocation const &loc, uint32 options = 0)
        {
            return TeleportTo(loc.m_mapId, loc.m_positionX, loc.m_positionY, loc.m_positionZ, loc.m_orientation, options);
        }

        void SetSummonPoint(uint32 mapid, float x, float y, float z);
        void SummonIfPossible(bool agree);
        bool IsBeingInvitedForSummon() { return m_invite_summon; }
        void UpdateSummonExpireTime();
        time_t GetSummonExpireTimer() const { return m_summon_expire; }

        bool Create(ObjectGuid::LowType guidlow, CharacterCreateInfo* createInfo);
        bool Create(ObjectGuid::LowType guidlow, const std::string& name, uint8 race, uint8 class_, uint8 gender, uint8 skin, uint8 face, uint8 hairStyle, uint8 hairColor, uint8 facialHair, uint8 outfitId);
        virtual void SetMapAtCreation(PlayerInfo const* info);

        void Update(uint32 time) override;

        static bool BuildEnumData( PreparedQueryResult  result,  WorldPacket * p_data, WorldSession const* session );

        bool IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index, WorldObject const* caster) const override;

        void SetInWater(bool apply);

        bool IsInWater() const override { return m_isInWater; }
        bool IsUnderWater() const override;

        void SendInitialPacketsBeforeAddToMap();
        void SendInitialPacketsAfterAddToMap();
        void SendSupercededSpell(uint32 oldSpell, uint32 newSpell) const;
        void SendTransferAborted(uint32 mapid, uint16 reason);
        void SendInstanceResetWarning(uint32 mapid, uint32 time);

        bool CanInteractWithQuestGiver(Object* questGiver);
        Creature* GetNPCIfCanInteractWith(ObjectGuid guid, uint32 npcflagmask = UNIT_NPC_FLAG_NONE);
        GameObject* GetGameObjectIfCanInteractWith(ObjectGuid guid) const;
        GameObject* GetGameObjectIfCanInteractWith(ObjectGuid guid, GameobjectTypes type) const;

        void ToggleAFK();
        void ToggleDND();
        bool IsAFK() const { return HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_AFK); };
        bool IsDND() const { return HasFlag(PLAYER_FLAGS,PLAYER_FLAGS_DND); };
        uint8 GetChatTag() const;
        std::string afkMsg;
        std::string dndMsg;

        PlayerSocial *GetSocial() { return m_social; }

        PlayerTaxi m_taxi;
        void InitTaxiNodesForLevel();
        bool ActivateTaxiPathTo(std::vector<uint32> const& nodes, Creature* npc = nullptr, uint32 spellid = 0);
        bool ActivateTaxiPathTo(uint32 taxi_path_id, uint32 spellid = 0);
        void FinishTaxiFlight();
        void CleanupAfterTaxiFlight();
        void ContinueTaxiFlight();
        void SendTaxiNodeStatusMultiple();

        bool IsAcceptWhispers() const { return m_ExtraFlags & PLAYER_EXTRA_ACCEPT_WHISPERS; }
        void SetAcceptWhispers(bool on) { if(on) m_ExtraFlags |= PLAYER_EXTRA_ACCEPT_WHISPERS; else m_ExtraFlags &= ~PLAYER_EXTRA_ACCEPT_WHISPERS; }
        bool IsGameMaster() const { return m_ExtraFlags & PLAYER_EXTRA_GM_ON; }
        void SetGameMaster(bool on);
        bool IsGMChat() const { return GetSession()->GetSecurity() >= SEC_GAMEMASTER1 && (m_ExtraFlags & PLAYER_EXTRA_GM_CHAT); }
        void SetGMChat(bool on) { if(on) m_ExtraFlags |= PLAYER_EXTRA_GM_CHAT; else m_ExtraFlags &= ~PLAYER_EXTRA_GM_CHAT; }
        bool IsTaxiCheater() const { return m_ExtraFlags & PLAYER_EXTRA_TAXICHEAT; }
        void SetTaxiCheater(bool on) { if(on) m_ExtraFlags |= PLAYER_EXTRA_TAXICHEAT; else m_ExtraFlags &= ~PLAYER_EXTRA_TAXICHEAT; }
        bool IsGMVisible() const { return !(m_ExtraFlags & PLAYER_EXTRA_GM_INVISIBLE); }
        void SetGMVisible(bool on);
        void SetPvPDeath(bool on) { if(on) m_ExtraFlags |= PLAYER_EXTRA_PVP_DEATH; else m_ExtraFlags &= ~PLAYER_EXTRA_PVP_DEATH; }
        bool IsInDuelArea() const;
        void SetDuelArea(bool on) { if(on) m_ExtraFlags |= PLAYER_EXTRA_DUEL_AREA; else m_ExtraFlags &= ~PLAYER_EXTRA_DUEL_AREA; }

        void GiveXP(uint32 xp, Unit* victim);
        void GiveLevel(uint32 level);
        void InitStatsForLevel(bool reapplyMods = false);

        // .cheat command related
        bool GetCommandStatus(uint32 command) const { return _activeCheats & command; }
        void SetCommandStatusOn(uint32 command)     { _activeCheats |= command; }
        void SetCommandStatusOff(uint32 command)    { _activeCheats &= ~command; }

        // Played Time Stuff
        time_t m_logintime;
        time_t m_Last_tick;
        uint32 m_Played_time[MAX_PLAYED_TIME_INDEX];
        uint32 GetTotalPlayedTime() { return m_Played_time[PLAYED_TIME_TOTAL]; };
        uint32 GetLevelPlayedTime() { return m_Played_time[PLAYED_TIME_LEVEL]; };
        uint32 GetTotalAccountPlayedTime();

        void SetDeathState(DeathState s) override;                   // overwrite Unit::setDeathState

        void InnEnter (int time,uint32 mapid, float x,float y,float z)
        {
            inn_pos_mapid = mapid;
            inn_pos_x = x;
            inn_pos_y = y;
            inn_pos_z = z;
            time_inn_enter = time;
        };

        float GetRestBonus() const { return m_rest_bonus; };
        void SetRestBonus(float rest_bonus_new);

        RestType GetRestType() const { return rest_type; };
        void SetRestType(RestType n_r_type) { rest_type = n_r_type; };

        uint32 GetInnPosMapId() const { return inn_pos_mapid; };
        float GetInnPosX() const { return inn_pos_x; };
        float GetInnPosY() const { return inn_pos_y; };
        float GetInnPosZ() const { return inn_pos_z; };

        int GetTimeInnEnter() const { return time_inn_enter; };
        void UpdateInnerTime (int time) { time_inn_enter = time; };

        Pet* GetPet() const;
        Pet* SummonPet(uint32 entry, float x, float y, float z, float ang, PetType petType, uint32 despwtime);
        void RemovePet(Pet* pet, PetSaveMode mode, bool returnreagent = false, RemovePetReason reason = REMOVE_PET_REASON_OTHER);

        /// Handles said message in regular chat based on declared language and in config pre-defined Range.
        void Say(std::string const& text, Language language, WorldObject const* = nullptr) override;
        /// Handles yelled message in regular chat based on declared language and in config pre-defined Range.
        void Yell(std::string const& text, Language language, WorldObject const* = nullptr) override;
        /// Outputs an universal text which is supposed to be an action.
        void TextEmote(std::string const& text, WorldObject const* = nullptr, bool = false) override;
        /// Handles whispers from Addons and players based on sender, receiver's guid and language.
        void Whisper(std::string const& text, Language language, Player* receiver, bool = false) override;

        /*********************************************************/
        /***                    STORAGE SYSTEM                 ***/
        /*********************************************************/

        void SetVirtualItemSlot(uint8 i, Item* item);
        void SetSheath(SheathState sheathed) override;
        uint8 FindEquipSlot(ItemTemplate const* proto, uint32 slot, bool swap) const;
        uint32 GetItemCount(uint32 item, bool inBankAlso = false, Item* skipItem = nullptr) const;
        Item* GetFirstItem(uint32 item) const;
        Item* GetItemByGuid(ObjectGuid guid) const;
        Item* GetItemByPos(uint16 pos) const;
        Item* GetItemByPos(uint8 bag, uint8 slot) const;
        Bag*  GetBagByPos(uint8 slot) const;
        //Does additional check for disarmed weapons
        Item* GetUseableItemByPos(uint8 bag, uint8 slot) const;
        Item* GetWeaponForAttack(WeaponAttackType attackType, bool useable = false) const;
        bool HasMainWeapon() const override;
        Item* GetShield(bool useable = false) const;
        static WeaponAttackType GetAttackBySlot(uint8 slot);        // MAX_ATTACK if not weapon slot
        std::vector<Item *> &GetItemUpdateQueue() { return m_itemUpdateQueue; }
        static bool IsInventoryPos(uint16 pos) { return IsInventoryPos(pos >> 8, pos & 255); }
        static bool IsInventoryPos(uint8 bag, uint8 slot);
        static bool IsEquipmentPos(uint16 pos) { return IsEquipmentPos(pos >> 8, pos & 255); }
        static bool IsEquipmentPos(uint8 bag, uint8 slot);
        static bool IsBagPos(uint16 pos);
        static bool IsBankPos(uint16 pos) { return IsBankPos(pos >> 8, pos & 255); }
        static bool IsBankPos(uint8 bag, uint8 slot);
        bool IsValidPos(uint16 pos) { return IsBankPos(pos >> 8, pos & 255); }
        bool IsValidPos(uint8 bag, uint8 slot) const;
        bool HasBankBagSlot(uint8 slot) const;
        bool HasItemCount(uint32 item, uint32 count, bool inBankAlso = false) const;
        uint32 GetEmptyBagSlotsCount() const;
        bool HasItemFitToSpellRequirements(SpellInfo const* spellInfo, Item const* ignoreItem = nullptr) const;
        bool CanNoReagentCast(SpellInfo const* spellInfo) const;
        Item* GetItemOrItemWithGemEquipped(uint32 item) const;
        //itemLimitCategory unused on BC
        InventoryResult CanTakeMoreSimilarItems(Item* pItem, uint32* itemLimitCategory = nullptr) const;
        //itemLimitCategory unused on BC
        InventoryResult CanTakeMoreSimilarItems(uint32 entry, uint32 count, uint32* itemLimitCategory = nullptr) const { return _CanTakeMoreSimilarItems(entry, count, nullptr, nullptr, itemLimitCategory); }
        InventoryResult CanStoreNewItem( uint8 bag, uint8 slot, ItemPosCountVec& dest, uint32 item, uint32 count, uint32* no_space_count = nullptr, ItemTemplate const* proto = nullptr ) const
        {
            return _CanStoreItem(bag, slot, dest, item, count, nullptr, false, no_space_count, proto );
        }
        InventoryResult CanStoreItem( uint8 bag, uint8 slot, ItemPosCountVec& dest, Item *pItem, bool swap = false ) const
        {
            if(!pItem)
                return EQUIP_ERR_ITEM_NOT_FOUND;
            uint32 count = pItem->GetCount();
            return _CanStoreItem( bag, slot, dest, pItem->GetEntry(), count, pItem, swap, nullptr, pItem->GetTemplate());

        }
        InventoryResult CanStoreItems( std::vector<Item*> const& items, uint32 count, uint32* itemLimitCategory) const;
        InventoryResult CanEquipNewItem(uint8 slot, uint16 &dest, uint32 item, bool swap) const;
        InventoryResult CanEquipItem(uint8 slot, uint16 &dest, Item *pItem, bool swap, bool not_loading = true) const;
        InventoryResult CanUnequipItems(uint32 item, uint32 count) const;
        InventoryResult CanUnequipItem(uint16 src, bool swap) const;
        InventoryResult CanBankItem(uint8 bag, uint8 slot, ItemPosCountVec& dest, Item *pItem, bool swap, bool not_loading = true) const;
        InventoryResult CanUseItem(Item *pItem, bool not_loading = true) const;
        bool HasItemTotemCategory(uint32 TotemCategory) const;
        bool CanUseItem(ItemTemplate const *pItem);
        InventoryResult CanUseAmmo(uint32 item) const;
        Item* StoreNewItem(ItemPosCountVec const& pos, uint32 item, bool update, int32 randomPropertyId = 0, GuidSet const& allowedLooters = GuidSet());
        Item* EquipNewItem(uint16 pos, uint32 item, bool update);
        Item* StoreItem(ItemPosCountVec const& pos, Item *pItem, bool update);
        void AutoUnequipOffhandIfNeed();
        bool StoreNewItemInBestSlots(uint32 item_id, uint32 item_count, ItemTemplate const *proto = nullptr);
        void AutoStoreLoot(uint8 bag, uint8 slot, uint32 loot_id, LootStore const& store, bool broadcast = false);
        void AutoStoreLoot(uint32 loot_id, LootStore const& store, bool broadcast = false) { AutoStoreLoot(NULL_BAG, NULL_SLOT, loot_id, store, broadcast); }
        void StoreLootItem(uint8 lootSlot, Loot* loot);
        uint32 GetEquipedItemsLevelSum();
        //Add item count to player, return pointer to item in variable item
        Item* AddItem(uint32 itemId, uint32 count);

        InventoryResult _CanTakeMoreSimilarItems(uint32 entry, uint32 count, Item* pItem, uint32* no_space_count = nullptr, uint32* itemLimitCategory = nullptr) const;
        InventoryResult _CanStoreItem( uint8 bag, uint8 slot, ItemPosCountVec& dest, uint32 entry, uint32 count, Item *pItem = nullptr, bool swap = false, uint32* no_space_count = nullptr, ItemTemplate const* proto = nullptr ) const;

        void ApplyEquipCooldown(Item * pItem);
        void SetAmmo(uint32 item);
        void RemoveAmmo();
        float GetAmmoDPS() const { return m_ammoDPS; }
        bool CheckAmmoCompatibility(const ItemTemplate *ammo_proto) const;
        void QuickEquipItem(uint16 pos, Item *pItem);
        void VisualizeItem(uint8 slot, Item *pItem);
        void SetVisibleItemSlot(uint8 slot, Item *pItem);
        Item* BankItem(ItemPosCountVec const& dest, Item *pItem, bool update)
        {
            return StoreItem( dest, pItem, update);
        }
        void RemoveItem(uint8 bag, uint8 slot, bool update);
        void MoveItemFromInventory(uint8 bag, uint8 slot, bool update);
        // in trade, auction, guild bank, mail....
        void MoveItemToInventory(ItemPosCountVec const& dest, Item* pItem, bool update, bool in_characterInventoryDB = false);
        // in trade, guild bank, mail....
        void RemoveItemDependentAurasAndCasts(Item* pItem);
        void DestroyItem( uint8 bag, uint8 slot, bool update );
        void DestroyItemCount( uint32 item, uint32 count, bool update, bool unequip_check = false, bool inBankAlso = false);
        void DestroyItemCount( Item* item, uint32& count, bool update );
        void SwapItems(uint32 item1, uint32 item2);
        void DestroyConjuredItems( bool update );
        void DestroyZoneLimitedItem( bool update, uint32 new_zone );
        void SplitItem( uint16 src, uint16 dst, uint32 count );
        void SwapItem( uint16 src, uint16 dst );
        void AddItemToBuyBackSlot( Item *pItem );
        Item* GetItemFromBuyBackSlot( uint32 slot );
        void RemoveItemFromBuyBackSlot( uint32 slot, bool del );
        uint32 GetMaxKeyringSize() const { return KEYRING_SLOT_END-KEYRING_SLOT_START; }
        //itemid has no effect on BC
        void SendEquipError(uint8 msg, Item* pItem, Item* pItem2 = nullptr, uint32 itemid = 0) const;
        void SendBuyError(uint8 msg, Creature* pCreature, uint32 item, uint32 param );
        void SendSellError(uint8 msg, Creature* pCreature, ObjectGuid guid, uint32 param );
        void AddWeaponProficiency(uint32 newflag) { m_WeaponProficiency |= newflag; }
        void AddArmorProficiency(uint32 newflag) { m_ArmorProficiency |= newflag; }
        uint32 GetWeaponProficiency() const { return m_WeaponProficiency; }
        uint32 GetArmorProficiency() const { return m_ArmorProficiency; }
        bool IsUseEquipedWeapon(bool mainhand) const
        {
            // disarm applied only to mainhand weapon
            return !IsInFeralForm() && (!mainhand || !HasFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_DISARMED) );
        }
        void SendNewItem( Item *item, uint32 count, bool received, bool created, bool broadcast = false, bool sendChatMessage = true);
        //vendorslot unused on BC
        bool BuyItemFromVendorSlot(ObjectGuid vendorguid, uint32 vendorslot, uint32 item, uint8 count, uint8 bag, uint8 slot);

        float GetReputationPriceDiscount( Creature const* pCreature ) const;
        float GetReputationPriceDiscount(FactionTemplateEntry const* factionTemplate) const;

        Player* GetTrader() const;
        TradeData* GetTradeData() const { return m_trade; }
        void TradeCancel(bool sendback);

        CinematicMgr* GetCinematicMgr() const { return _cinematicMgr; }

        void UpdateEnchantTime(uint32 time);
        void UpdateItemDuration(uint32 time, bool realtimeonly=false);
        void AddEnchantmentDurations(Item *item);
        void RemoveEnchantmentDurations(Item *item);
        void RemoveAllEnchantments(EnchantmentSlot slot, bool arena);
        void RemoveAllCurrentPetAuras();
        void AddEnchantmentDuration(Item *item,EnchantmentSlot slot,uint32 duration);
        void ApplyEnchantment(Item *item,EnchantmentSlot slot,bool apply, bool apply_dur = true, bool ignore_condition = false);
        void ApplyEnchantment(Item *item,bool apply);
        void SendEnchantmentDurations();
        void AddItemDurations(Item *item);
        void RemoveItemDurations(Item *item);
        void SendItemDurations();
		void LoadCorpse(PreparedQueryResult result);
        void LoadPet();

        uint32 m_stableSlots;

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/
        int32 GetQuestLevel(Quest const* quest) const { return quest && (quest->GetQuestLevel() > 0) ? quest->GetQuestLevel() : GetLevel(); }

        void PrepareQuestMenu(ObjectGuid guid);
        void SendPreparedQuest(ObjectGuid guid);
        std::string GetQuestOrTrainerTitleText(Creature* source, QEmote* emote = nullptr, bool allowDefault = true) const; //sun, moved part of SendPreparedQuest in this
        bool IsActiveQuest(uint32 quest_id) const;
        Quest const *GetNextQuest(ObjectGuid guid, Quest const *pQuest);
        bool CanSeeStartQuest(Quest const *pQuest);
        bool CanTakeQuest(Quest const *pQuest, bool msg);
        bool CanAddQuest(Quest const *pQuest, bool msg);
        bool CanCompleteQuest(uint32 quest_id);
        bool CanCompleteRepeatableQuest(Quest const *pQuest);
        bool CanRewardQuest(Quest const *pQuest, bool msg);
        bool CanRewardQuest(Quest const *pQuest, uint32 reward, bool msg);
        void AddQuestAndCheckCompletion(Quest const* quest, Object* questGiver);
        void AddQuest(Quest const *pQuest, Object *questGiver);
        void CompleteQuest(uint32 quest_id);
        void IncompleteQuest(uint32 quest_id);
        void RewardQuest(Quest const *pQuest, uint32 reward, Object* questGiver, bool announce = true);
        void SetRewardedQuest(uint32 quest_id);
        void FailQuest(uint32 quest_id);
        void AbandonQuest(uint32 quest_id);
        bool SatisfyQuestSkill(Quest const* qInfo, bool msg) const;
        bool SatisfyQuestClass(Quest const* qInfo, bool msg) const;
        bool SatisfyQuestLevel(Quest const* qInfo, bool msg);
        bool SatisfyQuestLog(bool msg);
        bool SatisfyQuestDependentQuests(Quest const* qInfo, bool msg) const;
        bool SatisfyQuestPreviousQuest(Quest const* qInfo, bool msg) const;
        bool SatisfyQuestDependentPreviousQuests(Quest const* qInfo, bool msg) const;
        bool SatisfyQuestRace(Quest const* qInfo, bool msg);
        bool SatisfyQuestReputation(Quest const* qInfo, bool msg);
        bool SatisfyQuestStatus(Quest const* qInfo, bool msg);
        bool SatisfyQuestConditions(Quest const* qInfo, bool msg);
        bool SatisfyQuestTimed(Quest const* qInfo, bool msg);
        bool SatisfyQuestExclusiveGroup(Quest const* qInfo, bool msg);
        bool SatisfyQuestDay(Quest const* qInfo, bool msg);
#ifdef LICH_KING
        bool SatisfyQuestWeek(Quest const* qInfo, bool msg) const;
        bool SatisfyQuestMonth(Quest const* qInfo, bool msg) const;
#else
        inline bool SatisfyQuestWeek(Quest const* qInfo, bool msg) const { return true; }
        inline bool SatisfyQuestMonth(Quest const* qInfo, bool msg) const { return true; }
#endif
        bool SatisfyQuestSeasonal(Quest const* qInfo, bool msg) const;
        bool GiveQuestSourceItem(Quest const *pQuest);
        bool TakeQuestSourceItem(uint32 quest_id, bool msg);
        bool GetQuestRewardStatus(uint32 quest_id) const;
        QuestStatus GetQuestStatus(uint32 quest_id) const;
        void SetQuestStatus(uint32 questId, QuestStatus status, bool update = true);
        void RemoveActiveQuest(uint32 questId, bool update = true);
        void RemoveRewardedQuest(uint32 questId, bool update = true);
        void SendQuestUpdate(uint32 questId);
        QuestGiverStatus GetQuestDialogStatus(Object* questGiver);

        void SetDailyQuestStatus(uint32 quest_id);
        void ResetDailyQuestStatus();
        bool IsDailyQuestDone(uint32 quest_id) const;
#ifdef LICH_KING
        void SetWeeklyQuestStatus(uint32 quest_id);
        void ResetWeeklyQuestStatus();
        void SetMonthlyQuestStatus(uint32 quest_id);
        void ResetMonthlyQuestStatus();
#endif
        void ResetSeasonalQuestStatus(uint16 event_id);
        void SetSeasonalQuestStatus(uint32 quest_id);


        uint16 FindQuestSlot(uint32 quest_id) const;
        uint32 GetQuestSlotQuestId(uint16 slot) const { return GetUInt32Value(PLAYER_QUEST_LOG_1_1 + slot * MAX_QUEST_OFFSET + QUEST_ID_OFFSET); }
        uint32 GetQuestSlotState(uint16 slot)   const { return GetUInt32Value(PLAYER_QUEST_LOG_1_1 + slot * MAX_QUEST_OFFSET + QUEST_STATE_OFFSET); }
        uint32 GetQuestSlotCounters(uint16 slot)const { return GetUInt32Value(PLAYER_QUEST_LOG_1_1 + slot * MAX_QUEST_OFFSET + QUEST_COUNTS_OFFSET); }
        uint8 GetQuestSlotCounter(uint16 slot, uint8 counter) const { return GetByteValue(PLAYER_QUEST_LOG_1_1 + slot * MAX_QUEST_OFFSET + QUEST_COUNTS_OFFSET, counter); }
        uint32 GetQuestSlotTime(uint16 slot)    const { return GetUInt32Value(PLAYER_QUEST_LOG_1_1 + slot * MAX_QUEST_OFFSET + QUEST_TIME_OFFSET); }
        void SetQuestSlot(uint16 slot, uint32 quest_id, uint32 timer = 0)
        {
            SetUInt32Value(PLAYER_QUEST_LOG_1_1 + slot*MAX_QUEST_OFFSET + QUEST_ID_OFFSET,quest_id);
            SetUInt32Value(PLAYER_QUEST_LOG_1_1 + slot*MAX_QUEST_OFFSET + QUEST_STATE_OFFSET,0);
            SetUInt32Value(PLAYER_QUEST_LOG_1_1 + slot*MAX_QUEST_OFFSET + QUEST_COUNTS_OFFSET,0);
            SetUInt32Value(PLAYER_QUEST_LOG_1_1 + slot*MAX_QUEST_OFFSET + QUEST_TIME_OFFSET,timer);
        }
        void SetQuestSlotCounter(uint16 slot,uint8 counter,uint8 count) { SetByteValue(PLAYER_QUEST_LOG_1_1 + slot*MAX_QUEST_OFFSET + QUEST_COUNTS_OFFSET,counter,count); }
        void SetQuestSlotState(uint16 slot,uint32 state) { SetFlag(PLAYER_QUEST_LOG_1_1 + slot*MAX_QUEST_OFFSET + QUEST_STATE_OFFSET,state); }
        void RemoveQuestSlotState(uint16 slot,uint32 state) { RemoveFlag(PLAYER_QUEST_LOG_1_1 + slot*MAX_QUEST_OFFSET + QUEST_STATE_OFFSET,state); }
        void SetQuestSlotTimer(uint16 slot,uint32 timer) { SetUInt32Value(PLAYER_QUEST_LOG_1_1 + slot*MAX_QUEST_OFFSET + QUEST_TIME_OFFSET,timer); }
        void SwapQuestSlot(uint16 slot1,uint16 slot2)
        {
            for (int i = 0; i < MAX_QUEST_OFFSET ; ++i )
            {
                uint32 temp1 = GetUInt32Value(PLAYER_QUEST_LOG_1_1 + MAX_QUEST_OFFSET *slot1 + i);
                uint32 temp2 = GetUInt32Value(PLAYER_QUEST_LOG_1_1 + MAX_QUEST_OFFSET *slot2 + i);

                SetUInt32Value(PLAYER_QUEST_LOG_1_1 + MAX_QUEST_OFFSET *slot1 + i, temp2);
                SetUInt32Value(PLAYER_QUEST_LOG_1_1 + MAX_QUEST_OFFSET *slot2 + i, temp1);
            }
        }
        uint32 GetReqKillOrCastCurrentCount(uint32 quest_id, int32 entry);
        void AdjustQuestReqItemCount(Quest const* quest, QuestStatusData& questStatusData);
        void AreaExploredOrEventHappens(uint32 questId);
        void GroupEventHappens(uint32 questId, WorldObject const* pEventObject);
        void ItemAddedQuestCheck(uint32 entry, uint32 count);
        void ItemRemovedQuestCheck(uint32 entry, uint32 count);
        void KilledMonsterCredit(uint32 entry, ObjectGuid guid, uint32 questId = 0);
        void ActivatedGO(uint32 entry, ObjectGuid guid);
        void CastedCreatureOrGO(uint32 entry, ObjectGuid guid, uint32 spell_id);
        void TalkedToCreature(uint32 entry, ObjectGuid guid);
        void MoneyChanged(uint32 value);
        void ReputationChanged(FactionEntry const* factionEntry);
        bool HasQuestForItem(uint32 itemid) const;
        bool HasQuestForGO(int32 GOId);
        void UpdateForQuestWorldObjects();
        bool CanShareQuest(uint32 quest_id) const;

        void SendQuestComplete(uint32 quest_id);
        void SendQuestReward(Quest const *pQuest, uint32 XP);
        void SendQuestFailed(uint32 quest_id) const;
        void SendQuestTimerFailed(uint32 quest_id) const;
        void SendCanTakeQuestResponse(uint32 msg) const;
        void SendQuestConfirmAccept(Quest const* pQuest, Player* pReceiver);
        void SendPushToPartyResponse(Player *pPlayer, uint32 msg);
        void SendQuestUpdateAddItem(Quest const* pQuest, uint32 item_idx, uint32 count);
        void SendQuestUpdateAddCreatureOrGo(Quest const* pQuest, ObjectGuid guid, uint32 creatureOrGO_idx, uint32 old_count, uint32 add_count);
        void SendQuestGiverStatusMultiple();

        uint32 GetSharedQuestID() const { return m_sharedQuestId; }
        ObjectGuid GetPlayerSharingQuest() const { return m_playerSharingQuest; }
        void SetQuestSharingInfo(ObjectGuid guid, uint32 id) { m_playerSharingQuest = guid; m_sharedQuestId = id; }
        void ClearQuestSharingInfo() { m_playerSharingQuest = ObjectGuid::Empty; m_sharedQuestId = 0; }

        uint32 GetInGameTime() { return m_ingametime; };

        void SetInGameTime( uint32 time ) { m_ingametime = time; };

        void AddTimedQuest( uint32 quest_id ) { m_timedquests.insert(quest_id); }
        void RemoveTimedQuest(uint32 questId) { m_timedquests.erase(questId); }

        /*********************************************************/
        /***                   LOAD SYSTEM                     ***/
        /*********************************************************/

        bool LoadFromDB(uint32 guid, SQLQueryHolder *holder);
        static bool   LoadValuesArrayFromDB(Tokens& data,ObjectGuid guid);
        static uint32 GetUInt32ValueFromArray(Tokens const& data, uint16 index);
        static float  GetFloatValueFromArray(Tokens const& data, uint16 index);
        static uint32 GetUInt32ValueFromDB(uint16 index, ObjectGuid guid);
        static float  GetFloatValueFromDB(uint16 index, ObjectGuid guid);
        static uint32 GetZoneIdFromDB(ObjectGuid guid);
        static uint32 GetLevelFromStorage(ObjectGuid guid);
        static bool   LoadPositionFromDB(uint32& mapid, float& x,float& y,float& z,float& o, bool& in_flight, ObjectGuid guid);

        static bool IsValidGender(uint8 Gender) { return Gender <= GENDER_FEMALE; }
        static bool ValidateAppearance(uint8 race, uint8 class_, uint8 gender, uint8 hairID, uint8 hairColor, uint8 faceID, uint8 facialHair, uint8 skinColor, bool create = false);

        /*********************************************************/
        /***                   SAVE SYSTEM                     ***/
        /*********************************************************/

        virtual void SaveToDB(bool create = false);
        virtual void SaveInventoryAndGoldToDB(SQLTransaction trans);                    // fast save function for item/money cheating preventing
        virtual void SaveGoldToDB(SQLTransaction trans);
        virtual void SaveDataFieldToDB();
        static bool SaveValuesArrayInDB(Tokens const& data,ObjectGuid guid);
        static void SetUInt32ValueInArray(Tokens& data,uint16 index, uint32 value);
        static void SetUInt32ValueInDB(uint16 index, uint32 value, ObjectGuid guid);
        static void SetFloatValueInDB(uint16 index, float value, ObjectGuid guid);
        static void SavePositionInDB(uint32 mapid, float x,float y,float z,float o,uint32 zone,ObjectGuid guid);

        bool m_mailsLoaded;
        bool m_mailsUpdated;

        void SetBindPoint(ObjectGuid guid);
        void SendTalentWipeConfirm(ObjectGuid guid);
        void RewardRage( uint32 damage, uint32 weaponSpeedHitFactor, bool attacker );
        void SendPetSkillWipeConfirm();
        //void CalcRage( uint32 damage,bool attacker );
        void RegenerateAll();
        void Regenerate(Powers power);
        void RegenerateHealth();
        void ResetAllPowers();
        void setRegenTimerCount(uint32 time) { m_regenTimerCount = time; }
        void setWeaponChangeTimer(uint32 time) {m_weaponChangeTimer = time;}

        void UpdateWeaponDependentCritAuras(WeaponAttackType attackType);
        void UpdateAllWeaponDependentCritAuras();

        void UpdateWeaponDependentAuras(WeaponAttackType attackType);
        void ApplyItemDependentAuras(Item* item, bool apply);

        bool CheckAttackFitToAuraRequirement(WeaponAttackType attackType, AuraEffect const* aurEff) const override;

        uint32 GetMoney() const;
        bool ModifyMoney(int32 amount, bool sendError = true);
        void SetMoney( uint32 value );
        bool HasEnoughMoney(int32 amount) const;

        RewardedQuestSet const& GetRewardedQuests() const { return m_RewardedQuests; }
        QuestStatusMap& GetQuestStatusMap() { return m_QuestStatus; }

        size_t GetRewardedQuestCount() const { return m_RewardedQuests.size(); }
        bool IsQuestRewarded(uint32 quest_id) const;

        Unit* GetSelectedUnit() const;
        Player* GetSelectedPlayer() const;

        void SetTarget(ObjectGuid /*guid*/) override { } /// Used for serverside target changes, does not apply to players
        void SetSelection(ObjectGuid guid) { SetGuidValue(UNIT_FIELD_TARGET, guid); }

        uint8 GetComboPoints() { return m_comboPoints; }
        uint8 GetComboPoints(Unit const* who = nullptr) const { return (who && m_comboTarget != who->GetGUID()) ? 0 : m_comboPoints; }
        ObjectGuid GetComboTarget() { return m_comboTarget; }

        void AddComboPoints(Unit* target, int8 count, bool forceCurrent = false);
        void ClearComboPoints(uint32 spellId = 0);
        void SendComboPoints();

        void SendMailResult(uint32 mailId, MailResponseType mailAction, MailResponseResult mailError, uint32 equipError = 0, ObjectGuid::LowType item_guid = 0, uint32 item_count = 0);
        void SendNewMail();
        void UpdateNextMailTimeAndUnreads();
        void AddNewMailDeliverTime(time_t deliver_time);
        bool IsMailsLoaded() const { return m_mailsLoaded; }

        void RemoveMail(uint32 id);

        void AddMail(Mail* mail) { m_mail.push_front(mail);}
        uint32 GetMailSize() { return m_mail.size();};
        Mail* GetMail(uint32 id);

        PlayerMails::iterator GetMailBegin() { return m_mail.begin();};
        PlayerMails::iterator GetMailEnd() { return m_mail.end();};

        /*********************************************************/
        /*** MAILED ITEMS SYSTEM ***/
        /*********************************************************/

        uint8 unReadMails;
        time_t m_nextMailDelivereTime;

        typedef std::unordered_map<uint32, Item*> ItemMap;

        ItemMap mMitems;                                    //template defined in objectmgr.cpp

        Item* GetMItem(uint32 id)
        {
            ItemMap::const_iterator itr = mMitems.find(id);
            if (itr != mMitems.end())
                return itr->second;

            return nullptr;
        }

        void AddMItem(Item* it)
        {
            ASSERT( it );
            //assert deleted, because items can be added before loading
            mMitems[it->GetGUID().GetCounter()] = it;
        }

        bool RemoveMItem(uint32 id)
        {
            auto i = mMitems.find(id);
            if (i == mMitems.end())
                return false;

            mMitems.erase(i);
            return true;
        }

        void PetSpellInitialize();
        void CharmSpellInitialize();
        void PossessSpellInitialize();
        void SendRemoveControlBar() const;
        bool HasSpell(uint32 spell) const override;
        bool HasSpellButDisabled(uint32 spell) const;
        TrainerSpellState GetTrainerSpellState(TrainerSpell const* trainer_spell) const;
        bool IsSpellFitByClassAndRace( uint32 spell_id ) const;
        bool HandlePassiveSpellLearn(SpellInfo const* spellInfo);

        void SendProficiency(uint8 pr1, uint32 pr2);
        void SendInitialSpells();
        bool AddSpell(uint32 spell_id, bool active, bool learning = true, bool dependent = false, bool disabled = false, bool loading = false, uint32 fromSkill = 0 );
        void LearnSpell(uint32 spell_id, bool dependent, uint32 fromSkill = 0);
        void RemoveSpell(uint32 spell_id, bool disabled = false);
        void resetSpells();
        void LearnDefaultSkills();
        void LearnDefaultSkill(uint32 skillId, uint16 rank);
        //Old mangos/windrunner logic. Replaced by LearnDefaultSkills except for some rare spells (16 at time of writing). We'll keep this function for now
        void LearnDefaultSpells(bool loading = false); 
        void LearnQuestRewardedSpells();
        void LearnQuestRewardedSpells(Quest const* quest);
        void LearnAllClassSpells();
        void LearnAllClassProficiencies();

        void DoPack58(uint8 step);

        uint32 GetFreeTalentPoints() const { return GetUInt32Value(PLAYER_CHARACTER_POINTS1); }
        void SetFreeTalentPoints(uint32 points);
        bool ResetTalents(bool no_cost = false);
        uint32 ResetTalentsCost() const;
        void InitTalentForLevel();
#ifdef LICH_KING
        void SendTalentsInfoData(bool pet);
#endif
        void LearnTalent(uint32 talentId, uint32 talentRank);

        uint32 GetFreePrimaryProffesionPoints() const { return GetUInt32Value(PLAYER_CHARACTER_POINTS2); }
        void SetFreePrimaryProffesions(uint16 profs) { SetUInt32Value(PLAYER_CHARACTER_POINTS2,profs); }
        void InitPrimaryProffesions();

        PlayerSpellMap const& GetSpellMap() const { return m_spells; }
        PlayerSpellMap      & GetSpellMap()       { return m_spells; }

        void AddSpellMod(SpellModifier*& mod, bool apply);
        bool IsAffectedBySpellmod(SpellInfo const *spellInfo, SpellModifier *mod, Spell const* spell = nullptr);
        template <class T> 
        void ApplySpellMod(uint32 spellId, SpellModOp op, T &basevalue, Spell* spell = nullptr);
        static void ApplyModToSpell(SpellModifier* mod, Spell* spell);
        void SetSpellModTakingSpell(Spell* spell, bool apply);

#ifdef LICH_KING
        static uint32 const ARENA_MAX_COOLDOWN = 10 * MINUTE * IN_MILLISECONDS;
#else
        static uint32 const ARENA_MAX_COOLDOWN = 15 * MINUTE * IN_MILLISECONDS;
#endif
        void RemoveArenaSpellCooldowns(bool removeActivePetCooldowns = false);

        void setResurrectRequestData(ObjectGuid guid, uint32 mapId, float X, float Y, float Z, uint32 health, uint32 mana)
        {
            m_resurrectGUID = guid;
            m_resurrectMap = mapId;
            m_resurrectX = X;
            m_resurrectY = Y;
            m_resurrectZ = Z;
            m_resurrectHealth = health;
            m_resurrectMana = mana;
        };
        void clearResurrectRequestData() { setResurrectRequestData(ObjectGuid::Empty, 0, 0.0f, 0.0f, 0.0f, 0, 0); }
        bool isRessurectRequestedBy(ObjectGuid guid) const { return m_resurrectGUID != 0 && m_resurrectGUID == guid; }
        bool isRessurectRequested() const { return m_resurrectGUID != 0; }
        void RessurectUsingRequestData();

        int getCinematic()
        {
            return m_cinematic;
        }
        void setCinematic(int cine)
        {
            m_cinematic = cine;
        }

        void addActionButton(uint8 button, uint16 action, uint8 type, uint8 misc);
        void removeActionButton(uint8 button);
        void SendInitialActionButtons();

        PvPInfo pvpInfo;
		void UpdatePvPState(bool onlyFFA = false);
        void UpdatePvP(bool state, bool ovrride=false);
        void UpdateZone(uint32 newZone, uint32 newArea);
        void UpdateArea(uint32 newArea);
		void SetNeedsZoneUpdate(bool needsUpdate) { m_needsZoneUpdate = needsUpdate; }
        bool IsInSanctuary() const override { return HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_SANCTUARY); }

        void UpdateZoneDependentAuras( uint32 zone_id );    // zones
        void UpdateAreaDependentAuras( uint32 area_id );    // subzones

        WorldLocation& GetTeleportDest() { return m_teleport_dest; }
        uint32 GetTeleportOptions() const { return m_teleport_options; }
        bool IsBeingTeleported() const { return IsBeingTeleportedNear() || IsBeingTeleportedFar(); }
        bool IsBeingTeleportedNear() const { return mSemaphoreTeleport_Near; }
        bool IsBeingTeleportedFar() const { return mSemaphoreTeleport_Far; }
        void SetSemaphoreTeleportNear(bool semphsetting) { mSemaphoreTeleport_Near = semphsetting; }
        void SetSemaphoreTeleportFar(bool semphsetting) { mSemaphoreTeleport_Far = semphsetting; }
        void ProcessDelayedOperations();

        bool IsCanDelayTeleport() const { return m_bCanDelayTeleport; }
        void SetCanDelayTeleport(bool setting) { m_bCanDelayTeleport = setting; }
        bool IsHasDelayedTeleport() const { return m_bHasDelayedTeleport; }
        void SetDelayedTeleportFlag(bool setting) { m_bHasDelayedTeleport = setting; }
        void ScheduleDelayedOperation(uint32 operation) { if (operation < DELAYED_END) m_DelayedOperations |= operation; }

        void UpdateAfkReport(time_t currTime);
        void UpdatePvPFlag(time_t currTime);
        void SetContestedPvP(Player* attackedPlayer = nullptr);
        void UpdateContestedPvP(uint32 currTime);
        void SetContestedPvPTimer(uint32 newTime) {m_contestedPvPTimer = newTime;}
        void ResetContestedPvP();

        /** todo: -maybe move UpdateDuelFlag+DuelComplete to independent DuelHandler.. **/
        std::unique_ptr<DuelInfo> duel;
        void UpdateDuelFlag(time_t currTime);
        void CheckDuelDistance(time_t currTime);
        void DuelComplete(DuelCompleteType type);
        void SendDuelCountdown(uint32 counter);

        bool IsGroupVisibleFor(Player const* p) const;
        bool IsInSameGroupWith(Player const* p) const;
        bool IsInSameRaidWith(Player const* p) const { return p==this || (GetGroup() != nullptr && GetGroup() == p->GetGroup()); }
        void UninviteFromGroup();
        static void RemoveFromGroup(Group* group, ObjectGuid guid, RemoveMethod method = GROUP_REMOVEMETHOD_DEFAULT, ObjectGuid kicker = ObjectGuid::Empty, char const* reason = nullptr);
        void RemoveFromGroup(RemoveMethod method = GROUP_REMOVEMETHOD_DEFAULT) { RemoveFromGroup(GetGroup(), GetGUID(), method); }
        void SendUpdateToOutOfRangeGroupMembers();

        void SetInGuild(uint32 guildId);
        void SetRank(uint32 rankId);
        void SetGuildIdInvited(uint32 guildId);
        uint32 GetGuildId() const;
        Guild* GetGuild() const;
        uint32 GetRank() const;
        int GetGuildIdInvited() const { return _guildIdInvited; }
        static void RemovePetitionsAndSigns(SQLTransaction trans, ObjectGuid guid, CharterTypes type);
        static uint32 GetGuildIdFromCharacterInfo(ObjectGuid::LowType guid);

        // Arena Team
        void SetInArenaTeam(uint32 ArenaTeamId, uint8 slot, uint8 type);
        static uint32 GetArenaTeamIdFromCharacterInfo(ObjectGuid guid, uint8 slot);
        uint32 GetArenaTeamId(uint8 slot) const { return GetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (slot * 6)); }
        void SetArenaTeamIdInvited(uint32 ArenaTeamId) { _arenaTeamIdInvited = ArenaTeamId; }
        uint32 GetArenaTeamIdInvited() const { return _arenaTeamIdInvited; }
        uint8 GetGladiatorRank() const;
        void UpdateGladiatorTitle(uint8 rank);
        void UpdateArenaTitles();
        void UpdateArenaTitleForRank(uint8 rank, bool add);

        void SetDifficulty(Difficulty dungeon_difficulty, bool sendToPlayer = true, bool asGroup = false);
        void SetDungeonDifficulty(Difficulty dungeon_difficulty) { m_dungeonDifficulty = dungeon_difficulty; }
        //arg has no effect for BC
        Difficulty GetDifficulty(bool isRaid = false) const { return m_dungeonDifficulty; }
        Difficulty GetDungeonDifficulty() const { return m_dungeonDifficulty; } //TC compat
        Difficulty GetRaidDifficulty() const { return REGULAR_DIFFICULTY; } //TC compat

        bool UpdateSkill(uint32 skill_id, uint32 step);
        bool UpdateSkillPro(uint16 SkillId, int32 Chance, uint32 step);

        bool UpdateCraftSkill(uint32 spellid);
        bool UpdateGatherSkill(uint32 SkillId, uint32 SkillValue, uint32 RedLevel, uint32 Multiplicator = 1);
        bool UpdateFishingSkill();

        uint32 GetBaseDefenseSkillValue() const { return GetBaseSkillValue(SKILL_DEFENSE); }
        uint32 GetBaseWeaponSkillValue(WeaponAttackType attType) const;

        float GetHealthBonusFromStamina() const;
        float GetManaBonusFromIntellect() const;

        bool UpdateStats(Stats stat) override;
        bool UpdateAllStats() override;
        void UpdateResistances(uint32 school) override;
        void UpdateArmor() override;
        void UpdateMaxHealth() override;
        void UpdateMaxPower(Powers power) override;
        void UpdateAttackPowerAndDamage(bool ranged = false) override;
        void UpdateShieldBlockValue();
        void UpdateSpellDamageAndHealingBonus();
        void ApplyRatingMod(CombatRating cr, int32 value, bool apply);
        void UpdateRating(CombatRating cr);
        void UpdateAllRatings();    

        void CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& minDamage, float& maxDamage, uint8 damageIndex) const override;
        SpellSchoolMask GetMeleeDamageSchoolMask(WeaponAttackType attackType = BASE_ATTACK, uint8 damageIndex = 0) const override;
        bool HasWand() const;

        void UpdateDefenseBonusesMod();
        float GetMeleeCritFromAgility();
        float GetDodgeFromAgility();
        float GetSpellCritFromIntellect();
        float GetMissPercentageFromDefense() const;
        float OCTRegenHPPerSpirit();
        float OCTRegenMPPerSpirit();
        float GetRatingCoefficient(CombatRating cr) const;
        float GetRatingBonusValue(CombatRating cr) const;
        uint32 GetMeleeCritDamageReduction(uint32 damage) const;
        uint32 GetRangedCritDamageReduction(uint32 damage) const;
        uint32 GetSpellCritDamageReduction(uint32 damage) const;
        uint32 GetDotDamageReduction(uint32 damage) const;

        float GetExpertiseDodgeOrParryReduction(WeaponAttackType attType) const;
        void UpdateBlockPercentage();
        void UpdateCritPercentage(WeaponAttackType attType);
        void UpdateAllCritPercentages();
        void UpdateParryPercentage();
        void UpdateDodgePercentage();
        void UpdateMeleeHitChances();
        void UpdateRangedHitChances();
        void UpdateSpellHitChances();

        void UpdateAllSpellCritChances();
        void UpdateSpellCritChance(uint32 school);
        void UpdateExpertise(WeaponAttackType attType);
        void UpdateManaRegen();

        ObjectGuid GetLootGUID() const { return m_lootGuid; }
        void SetLootGUID(ObjectGuid guid) { m_lootGuid = guid; }

        void RemovedInsignia(Player* looterPlr);

        WorldSession* GetSession() const { return m_session; }
        void SetSession(WorldSession *s) { m_session = s; }

        void BuildCreateUpdateBlockForPlayer(UpdateData *data, Player *target) const override;
        void DestroyForPlayer(Player *target, bool onDeath = false) const override;
        void SendLogXPGain(uint32 GivenXP,Unit* victim,uint32 RestXP);

        //Low Level Packets
        void PlaySound(uint32 Sound, bool OnlySelf);
        //notifiers
        void SendAttackSwingCantAttack();
        void SendAttackSwingCancelAttack();
        void SendAttackSwingDeadTarget();
        void SendAttackSwingNotStanding();
        void SendAttackSwingNotInRange();
        void SendAttackSwingBadFacingAttack();
        void SendAutoRepeatCancel();
        void SendFeignDeathResisted() const;
        void SendExplorationExperience(uint32 Area, uint32 Experience);

        void SendDungeonDifficulty(bool IsInGroup);
        void ResetInstances(uint8 method, bool isRaid = false);
        void SendResetInstanceSuccess(uint32 MapId);
        void SendResetInstanceFailed(uint32 reason, uint32 MapId);
        void SendResetFailedNotify(uint32 mapid);

        bool UpdatePosition(float x, float y, float z, float orientation, bool teleport = false) override;
		bool UpdatePosition(const Position &pos, bool teleport = false) override { return UpdatePosition(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), teleport); }
        void ProcessTerrainStatusUpdate(ZLiquidStatus status, Optional<LiquidData> const& liquidData, bool updateCreatureLiquid ) override;
        void AtExitCombat() override;

        void SendMessageToSet(WorldPacket const* data, bool self) override;
        void SendMessageToSetInRange(WorldPacket const* data, float dist, bool self, bool includeMargin = false, Player const* skipped_rcvr = nullptr) override;
		void SendMessageToSetInRange(WorldPacket const* data, float dist, bool self, bool includeMargin, bool own_team_only, Player const* skipped_rcvr = nullptr);
        void SendMessageToSet(WorldPacket const* data, Player* skipped_rcvr) override;

        /**
        * Deletes a character from the database
        *
        * By default character in just unlinked and not really deleted.
        *
        * @see Player::DeleteOldCharacters
        *
        * @param playerguid       the low-GUID from the player which should be deleted
        * @param accountId        the account id from the player
        * @param updateRealmChars when this flag is set, the amount of characters on that realm will be updated in the realmlist
        * @param deleteFinally    if this flag is set, the character will be permanently removed from the database
        */
        static void DeleteFromDB(ObjectGuid playerguid, uint32 accountId, bool updateRealmChars = true, bool deleteFinally = false);
        static void LeaveAllArenaTeams(ObjectGuid guid);
        static void DeleteOldCharacters();

        Corpse *GetCorpse() const;
        void SpawnCorpseBones(bool triggerSave = true);
		Corpse* CreateCorpse();
        void KillPlayer();
		static void OfflineResurrect(ObjectGuid const& guid, SQLTransaction& trans);
		bool HasCorpse() const { return _corpseLocation.GetMapId() != MAPID_INVALID; }
		WorldLocation GetCorpseLocation() const { return _corpseLocation; }
        uint32 GetResurrectionSpellId();
        void ResurrectPlayer(float restore_percent, bool applySickness = false);
        void BuildPlayerRepop();
        void RepopAtGraveyard();
        void SetIsRepopPending(bool pending) { m_isRepopPending = pending; }
        bool IsRepopPending() const { return m_isRepopPending; }

        void DurabilityLossAll(double percent, bool inventory);
        void DurabilityLoss(Item* item, double percent);
        void DurabilityPointsLossAll(int32 points, bool inventory);
        void DurabilityPointsLoss(Item* item, int32 points);
        void DurabilityPointLossForEquipSlot(EquipmentSlots slot);
        uint32 DurabilityRepairAll(bool cost, float discountMod, bool guildBank);
        uint32 DurabilityRepair(uint16 pos, bool cost, float discountMod, bool guildBank);

        void UpdateMirrorTimers();
        void StopMirrorTimers()
        {
            StopMirrorTimer(FATIGUE_TIMER);
            StopMirrorTimer(BREATH_TIMER);
            StopMirrorTimer(FIRE_TIMER);
        }

        void JoinedChannel(Channel *c);
        void LeftChannel(Channel *c);
        void CleanupChannels();
        void UpdateLocalChannels( uint32 newZone );
        void LeaveLFGChannel();

        void UpdateDefense();
        void UpdateWeaponSkill (WeaponAttackType attType);
        void UpdateCombatSkills(Unit *pVictim, WeaponAttackType attType, bool defence);

        //step = skill tier (I guess)
        void SetSkill(uint32 id, uint16 step, uint16 currVal, uint16 maxVal);
        uint16 GetMaxSkillValue(uint32 skill) const;        // max + perm. bonus
        uint16 GetPureMaxSkillValue(uint32 skill) const;    // max
        uint16 GetSkillValue(uint32 skill) const;           // skill value + perm. bonus + temp bonus
        uint16 GetBaseSkillValue(uint32 skill) const;       // skill value + perm. bonus
        uint16 GetPureSkillValue(uint32 skill) const;       // skill value
        int16 GetSkillPermBonusValue(uint32 skill) const;
        int16 GetSkillTempBonusValue(uint32 skill) const;
        bool HasSkill(uint32 skill) const;
        void LearnSkillRewardedSpells( uint32 skillId, uint32 skillValue);

        void CheckAreaExploreAndOutdoor(void);

        static uint32 TeamForRace(uint8 race);
        uint32 GetTeam() const { return m_team; }
		TeamId GetTeamId() const { return m_team == ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE; }
        void SetFactionForRace(uint8 race);
        
        static bool IsMainFactionForRace(uint32 race, uint32 factionId);
        static uint32 GetMainFactionForRace(uint32 race);
        static uint32 GetNewFactionForRaceChange(uint32 oldRace, uint32 newRace, uint32 factionId);

        bool IsAtGroupRewardDistance(WorldObject const* pRewardSource) const;
        bool RewardPlayerAndGroupAtKill(Unit* pVictim);
        void RewardPlayerAndGroupAtEvent(uint32 creature_id, WorldObject* pRewardSource);
        bool IsHonorOrXPTarget(Unit* victim) const;

        ReputationMgr&       GetReputationMgr() { return *m_reputationMgr; }
        ReputationMgr const& GetReputationMgr() const { return *m_reputationMgr; }
        int32 GetReputation(uint32 faction_id) const;
        ReputationRank GetReputationRank(uint32 faction_id) const;
        void RewardReputation(Unit* victim, float rate);
        void RewardReputation(Quest const* quest);

        int32 CalculateReputationGain(ReputationSource source, uint32 creatureOrQuestLevel, int32 rep, int32 faction, bool noQuestBonus = false);

        void UpdateSkillsForLevel();
        void UpdateSkillsToMaxSkillsForLevel();             // for .levelup
        void ModifySkillBonus(uint32 skillid,int32 val, bool talent);

        /*********************************************************/
        /***                  PVP SYSTEM                       ***/
        /*********************************************************/
        void UpdateArenaFields();
        void UpdateHonorFields();
        bool RewardHonor(Unit *pVictim, uint32 groupsize, float honor = -1, bool pvptoken = false);
        uint32 GetHonorPoints() { return GetUInt32Value(PLAYER_FIELD_HONOR_CURRENCY); }
        uint32 GetArenaPoints() { return GetUInt32Value(PLAYER_FIELD_ARENA_CURRENCY); }
        void ModifyHonorPoints(int32 value, SQLTransaction trans = SQLTransaction(nullptr));      //! If trans is specified, honor save query will be added to trans
        void ModifyArenaPoints(int32 value, SQLTransaction trans = SQLTransaction(nullptr));      //! If trans is specified, arena point save query will be added to trans
        uint32 GetMaxPersonalArenaRatingRequirement();
        void UpdateKnownPvPTitles();
        void SetHonorPoints(uint32 value);
        void SetArenaPoints(uint32 value);

        //End of PvP System

        void SetDrunkValue(uint16 newDrunkValue, uint32 itemid = 0);
        uint8 GetDrunkValue() const { return GetByteValue(PLAYER_BYTES_3, PLAYER_BYTES_3_OFFSET_INEBRIATION); }
        static DrunkenState GetDrunkenstateByValue(uint8 value);

        uint32 GetDeathTimer() const { return m_deathTimer; }
        uint32 GetCorpseReclaimDelay(bool pvp) const;
        void UpdateCorpseReclaimDelay();
		int32 CalculateCorpseReclaimDelay(bool load = false) const;
		void SendCorpseReclaimDelay(uint32 delay);

        uint32 GetShieldBlockValue() const override;                 // overwrite Unit version (virtual)
        bool CanParry() const { return m_canParry; }
        void SetCanParry(bool value);
        bool CanBlock() const { return m_canBlock; }
        void SetCanBlock(bool value);

        void SetRegularAttackTime();

        void HandleBaseModFlatValue(BaseModGroup modGroup, float amount, bool apply);
        void ApplyBaseModPctValue(BaseModGroup modGroup, float pct);

        void SetBaseModFlatValue(BaseModGroup modGroup, float val);
        void SetBaseModPctValue(BaseModGroup modGroup, float val);

        void UpdateDamageDoneMods(WeaponAttackType attackType) override;
        void UpdateBaseModGroup(BaseModGroup modGroup);

        float GetBaseModValue(BaseModGroup modGroup, BaseModType modType) const;
        float GetTotalBaseModValue(BaseModGroup modGroup) const;

        void _ApplyAllStatBonuses();
        void _RemoveAllStatBonuses();

        //return true if any spells was successfully cast
        bool CastItemUseSpell(Item* item, SpellCastTargets const& targets, uint8 cast_count, uint32 glyphIndex = 0);
        void _ApplyItemMods(Item *item,uint8 slot,bool apply, bool updateItemAuras = true);
        void _RemoveAllItemMods();
        void _ApplyAllItemMods();
        void _ApplyItemBonuses(ItemTemplate const *proto,uint8 slot,bool apply);
        void _ApplyWeaponDamage(uint8 slot, ItemTemplate const* proto, /* TC ScalingStatValuesEntry const* ssv, */ bool apply);
        void _ApplyAmmoBonuses();
        bool EnchantmentFitsRequirements(uint32 enchantmentcondition, int8 slot);
        void ToggleMetaGemsActive(uint8 exceptslot, bool apply);
        void CorrectMetaGemEnchants(uint8 slot, bool apply);
        void InitDataForForm(bool reapplyMods = false);

        void ApplyItemEquipSpell(Item *item, bool apply, bool form_change = false);
        void ApplyEquipSpell(SpellInfo const* spellInfo, Item* item, bool apply, bool form_change = false);
        void UpdateEquipSpellsAtFormChange();
        void CastItemCombatSpell(DamageInfo const& damageInfo);
        void CastItemCombatSpell(DamageInfo const& damageInfo, Item* item, ItemTemplate const* proto);

        void SendInitWorldStates(uint32 zoneid, uint32 areaid);
        void SendUpdateWorldState(uint32 Field, uint32 Value);
        void SendDirectMessage(WorldPacket *data) const;

        void SendAuraDurationsForTarget(Unit* target);

        PlayerMenu* PlayerTalkClass;
        std::vector<ItemSetEffect *> ItemSetEff;

        void SendLoot(ObjectGuid guid, LootType loot_type);
        void SendLootError(ObjectGuid guid, LootError error);
        void SendLootRelease( ObjectGuid guid );
        void SendNotifyLootItemRemoved(uint8 lootSlot);
        void SendNotifyLootMoneyRemoved();

        /*********************************************************/
        /***               BATTLEGROUND SYSTEM                 ***/
        /*********************************************************/

        bool InBattleground()       const { return m_bgData.bgInstanceID != 0; }
        uint32 GetBattlegroundId()  const { return m_bgData.bgInstanceID; }
        BattlegroundTypeId GetBattlegroundTypeId() const { return m_bgData.bgTypeID; }
        Battleground* GetBattleground() const;
        bool InArena() const;
          
        bool InBattlegroundQueue(bool ignoreArena = false) const;

        BattlegroundQueueTypeId GetBattlegroundQueueTypeId(uint32 index) const;
        uint32 GetBattlegroundQueueIndex(BattlegroundQueueTypeId bgQueueTypeId) const;
        bool IsInvitedForBattlegroundQueueType(BattlegroundQueueTypeId bgQueueTypeId) const;
        bool InBattlegroundQueueForBattlegroundQueueType(BattlegroundQueueTypeId bgQueueTypeId) const;

        void SetBattlegroundId(uint32 val, BattlegroundTypeId bgTypeId);
        uint32 AddBattlegroundQueueId(BattlegroundQueueTypeId val);
        bool HasFreeBattlegroundQueueId() const;
        void RemoveBattlegroundQueueId(BattlegroundQueueTypeId val);
        void SetInviteForBattlegroundQueueType(BattlegroundQueueTypeId bgQueueTypeId, uint32 instanceId);
        bool IsInvitedForBattlegroundInstance(uint32 instanceId) const;
        WorldLocation const& GetBattlegroundEntryPoint() const { return m_bgData.joinPos; }
        void SetBattlegroundEntryPoint();

		bool TeleportToBGEntryPoint();

        void SetBGTeam(uint32 team) { m_bgTeam = team; }
        uint32 GetBGTeam() const { return m_bgTeam ? m_bgTeam : GetTeam(); }

        void LeaveBattleground(bool teleportToEntryPoint = true);
        bool CanJoinToBattleground(Battleground const* bg) const;
        bool CanReportAfkDueToLimit();
        void ReportedAfkBy(Player* reporter);
        void ClearAfkReports() { m_bgData.bgAfkReporter.clear(); }

        bool GetBGAccessByLevel(BattlegroundTypeId bgTypeId) const;
        bool CanUseBattlegroundObject(GameObject* gameobject);
        bool isAllowedToTakeBattlegroundBase();
        bool IsTotalImmune();

#ifdef LICH_KING
        bool GetRandomWinner() const { return m_IsBGRandomWinner; }
        void SetRandomWinner(bool isWinner);
#endif

        /*********************************************************/
        /***               OUTDOOR PVP SYSTEM                  ***/
        /*********************************************************/

        OutdoorPvP * GetOutdoorPvP() const;
        // returns true if the player is in active state for outdoor pvp objective capturing, false otherwise
        bool IsOutdoorPvPActive();

        /*********************************************************/
        /***                    REST SYSTEM                    ***/
        /*********************************************************/

        bool isRested() const { return GetRestTime() >= 10000; }
        uint32 GetXPRestBonus(uint32 xp);
        uint32 GetRestTime() const { return m_restTime;};
        void SetRestTime(uint32 v) { m_restTime = v;};

        /*********************************************************/
        /***              ENVIROMENTAL SYSTEM                  ***/
        /*********************************************************/

        bool IsImmuneToEnvironmentalDamage() const;
        uint32 EnvironmentalDamage(EnviromentalDamage type, uint32 damage);

        /*********************************************************/
        /***               FLOOD FILTER SYSTEM                 ***/
        /*********************************************************/

        void UpdateSpeakTime();
        bool CanSpeak() const;

        /*********************************************************/
        /***                 VARIOUS SYSTEMS                   ***/
        /*********************************************************/
        
        void UpdateFallInformationIfNeed(MovementInfo const& minfo, uint16 opcode);
        WorldObject* m_seer;
        void SetFallInformation(uint32 time, float z);
        void HandleFall(MovementInfo const& movementInfo);

        //only for TC compat
        bool CanFlyInZone(uint32 mapid, uint32 zone, SpellInfo const* bySpell) const { return true; }

        void SetFlying(bool apply) override;

        bool CanFly() const override { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_CAN_FLY); }
        bool CanWalk() const override { return true; }
        bool CanSwim() const override { return true; }

        void HandleDrowning(uint32 time_diff);

        //relocate without teleporting
        void RelocateToArenaZone(bool secondary = false);
        void TeleportToArenaZone(bool secondary = false);
        void RelocateToBetaZone();
        void TeleportToBetaZone();
        bool ShouldGoToSecondaryArenaZone();
        void GetArenaZoneCoord(bool secondary, uint32& map, float& x, float& y, float& z, float& o);
        void GetBetaZoneCoord(uint32& map, float& x, float& y, float& z, float& o);
        
        std::string GetDebugInfo() const override;

    public:
		void SetSeer(WorldObject* target) { m_seer = target; }
		void SetViewpoint(WorldObject* target, bool apply);
		WorldObject* GetViewpoint() const;

        uint32 GetSaveTimer() const { return m_nextSave; }
        void   SetSaveTimer(uint32 timer) { m_nextSave = timer; }

        #ifdef PLAYERBOT
        // A Player can either have a playerbotMgr (to manage its bots), or have playerbotAI (if it is a bot), or
        // neither. Code that enables bots must create the playerbotMgr and set it using SetPlayerbotMgr.
        // EquipmentSets& GetEquipmentSets() { return m_EquipmentSets; } //revmoed, not existing on BC
        void SetPlayerbotAI(PlayerbotAI* ai) { m_playerbotAI = ai; }
        PlayerbotAI* GetPlayerbotAI() { return m_playerbotAI; }
        void SetPlayerbotMgr(PlayerbotMgr* mgr) { m_playerbotMgr=mgr; }
        PlayerbotMgr* GetPlayerbotMgr() { return m_playerbotMgr; }
        void SetBotDeathTimer() { m_deathTimer = 0; }
        //PlayerTalentMap& GetTalentMap(uint8 spec) { return *m_talents[spec]; }
        #endif
        #ifdef TESTS
        PlayerbotTestingAI* GetTestingPlayerbotAI() const;
        #endif
        bool IsTestingBot() const;

        // Recall position
        WorldLocation m_recall_location;
        void SaveRecallPosition();
        bool Recall() { return TeleportTo(m_recall_location); }
        
        uint32 m_ConditionErrorMsgId;

        // Homebind coordinates
        uint32 m_homebindMapId;
        uint16 m_homebindAreaId;
        float m_homebindX;
        float m_homebindY;
        float m_homebindZ;
        void SetHomebind(WorldLocation const& loc, uint32 area_id);

        // currently visible objects at player client
		GuidUnorderedSet m_clientGUIDs;

        bool HaveAtClient(WorldObject const* u) const { return u==this || m_clientGUIDs.find(u->GetGUID())!=m_clientGUIDs.end(); }

		bool IsNeverVisible() const override;
        bool IsVisibleGloballyFor(Player* pl) const;

        void SendInitialVisiblePackets(Unit* target);
		void UpdateObjectVisibility(bool forced = true) override;
		void UpdateVisibilityForPlayer();
		void UpdateVisibilityOf(WorldObject* target);
		void UpdateTriggerVisibility();

        template<class T>
            void UpdateVisibilityOf(T* target, UpdateData& data, std::set<Unit*>& visibleNow);

        uint8 m_forced_speed_changes[MAX_MOVE_TYPE];

        bool HasAtLoginFlag(AtLoginFlags f) const { return m_atLoginFlags & f; }
        void SetAtLoginFlag(AtLoginFlags f) { m_atLoginFlags |= f; }
        void RemoveAtLoginFlag(AtLoginFlags f) { m_atLoginFlags = m_atLoginFlags & ~f; }

        LookingForGroup m_lookingForGroup;

        // Temporarily removed pet cache
        uint32 GetTemporaryUnsummonedPetNumber() const { return m_temporaryUnsummonedPetNumber; }
        void SetTemporaryUnsummonedPetNumber(uint32 petnumber) { m_temporaryUnsummonedPetNumber = petnumber; }
        void UnsummonPetTemporaryIfAny();
        void ResummonPetTemporaryUnSummonedIfAny();
        bool IsPetNeedBeTemporaryUnsummoned() const;
        uint32 GetOldPetSpell() const { return m_oldpetspell; }
        void SetOldPetSpell(uint32 petspell) { m_oldpetspell = petspell; }
        
        // Experience Blocking
        bool IsXpBlocked() const { return m_isXpBlocked; }
        void SetXpBlocked(bool blocked) { m_isXpBlocked = blocked; }

        void SendCinematicStart(uint32 CinematicSequenceId) const;
        void SendMovieStart(uint32 MovieId) const;

        // Anti Undermap
        void SaveSafePosition(Position pos);
        bool UndermapRecall();

        Optional<Position> _lastSafePosition;

        /*********************************************************/
        /***                 INSTANCE SYSTEM                   ***/
        /*********************************************************/

        typedef std::unordered_map< uint32 /*mapId*/, InstancePlayerBind > BoundInstancesMap;

        void UpdateHomebindTime(uint32 time);

        uint32 m_HomebindTimer;
        bool m_InstanceValid;
        // permanent binds and solo binds by difficulty
        BoundInstancesMap m_boundInstances[MAX_DIFFICULTY];
        InstancePlayerBind* GetBoundInstance(uint32 mapid, Difficulty difficulty);
        BoundInstancesMap& GetBoundInstances(Difficulty difficulty) { return m_boundInstances[difficulty]; }
        //raid is unused on BC
        InstanceSave * GetInstanceSave(uint32 mapid, bool raid = false);
        void UnbindInstance(uint32 mapid, Difficulty difficulty, bool unload = false);
        void UnbindInstance(BoundInstancesMap::iterator &itr, Difficulty difficulty, bool unload = false);
        InstancePlayerBind* BindToInstance(InstanceSave *save, bool permanent, bool load = false);
        void SendRaidInfo();
        void SendSavedInstances();
        bool Satisfy(AccessRequirement const*, uint32 target_map, bool report = false);
        bool CheckInstanceValidity(bool /*isLogin*/);

        /*********************************************************/
        /***                   GROUP SYSTEM                    ***/
        /*********************************************************/

        Group * GetGroupInvite() { return m_groupInvite; }
        void SetGroupInvite(Group *group) { m_groupInvite = group; }
        Group* GetGroup() { return m_group.getTarget(); }
        const Group* GetGroup() const { return (const Group*)m_group.getTarget(); }
        GroupReference& GetGroupRef() { return m_group; }
        void SetGroup(Group *group, int8 subgroup = -1);
        uint8 GetSubGroup() const { return m_group.getSubGroup(); }
        uint32 GetGroupUpdateFlag() const { return m_groupUpdateMask; }
        void SetGroupUpdateFlag(uint32 flag) { m_groupUpdateMask |= flag; }
        uint64 GetAuraUpdateMask() const { return m_auraUpdateMask; }
        void SetAuraUpdateMask(uint8 slot) { m_auraUpdateMask |= (uint64(1) << slot); }
        void UnsetAuraUpdateMask(uint8 slot) { m_auraUpdateMask &= ~(uint64(1) << slot); }
        Player* GetNextRandomRaidMember(float radius) const;
        PartyResult CanUninviteFromGroup() const;
        // Teleporter NPC: Check level requirements (in Config)
        bool HasLevelInRangeForTeleport() const;
        
        // Battleground Group System
        inline void SetBattlegroundOrBattlefieldRaid(Group* group, int8 subgroup = -1) { SetBattlegroundRaid(group, subgroup); } //Compat tc
        void SetBattlegroundRaid(Group* group, int8 subgroup = -1);
        void RemoveFromBattlegroundRaid();
        inline void RemoveFromBattlegroundOrBattlefieldRaid() { RemoveFromBattlegroundRaid(); }; //TC COMPAT
        Group* GetOriginalGroup() { return m_originalGroup.getTarget(); }
        GroupReference& GetOriginalGroupRef() { return m_originalGroup; }
        uint8 GetOriginalSubGroup() const { return m_originalGroup.getSubGroup(); }
        void SetOriginalGroup(Group* group, int8 subgroup = -1);
        
        void SetPassOnGroupLoot(bool bPassOnGroupLoot) { m_bPassOnGroupLoot = bPassOnGroupLoot; }
        bool GetPassOnGroupLoot() const { return m_bPassOnGroupLoot; }

        MapReference &GetMapRef() { return m_mapRef; }

		// Set map to player and add reference
		void SetMap(Map* map) override;
		void ResetMap() override;

        bool IsAllowedToLoot(Creature const* creature) const;

        DeclinedName const* GetDeclinedNames() const { return m_declinedname; }
        bool HasTitle(uint32 bitIndex) const;
        bool HasTitle(CharTitlesEntry const* title) const { return HasTitle(title->bit_index); }
        void SetTitle(CharTitlesEntry const* title, bool notify = false, bool setCurrentTitle = false);
        void RemoveTitle(CharTitlesEntry const* title, bool notify = true);

        Spell* m_spellModTakingSpell;

        uint8 GetRace() const { return m_race; }
        uint8 GetGender() const { return m_gender; }
        void SetGender(uint8 gender) { m_gender = gender; }
        void SetRace(uint8 newrace) { m_race = newrace; } // Race/Faction change
        
        void SetSpiritRedeptionKiller(ObjectGuid killerGUID) { m_spiritRedemptionKillerGUID = killerGUID; }
        ObjectGuid GetSpiritRedemptionKiller() { return m_spiritRedemptionKillerGUID; }

        bool hasCustomXpRate() const { return m_customXp != 0.0f; }
        float getCustomXpRate() const { return m_customXp; }

        bool HaveSpectators() const;
        void SendSpectatorAddonMsgToBG(SpectatorAddonMsg msg);
        bool isSpectateCanceled() { return spectateCanceled; }
        void CancelSpectate()     { spectateCanceled = true; }
        Player* getSpectateFrom()   { return spectateFrom; }
        bool isSpectator() const  { return spectatorFlag; }
        void SetSpectate(bool on);
        void SendDataForSpectator();

        void setCommentator(bool on);

        void addSpamReport(ObjectGuid reporterGUID, std::string message);
        time_t lastLagReport;
        
        std::vector<Item*> m_itemUpdateQueue;
        bool m_itemUpdateQueueBlocked;
        
        /*********************************************************/
        /***                    GOSSIP SYSTEM                  ***/
        /*********************************************************/

        void PrepareGossipMenu(WorldObject* source, uint32 menuId = 0, bool showQuests = false);
        void SendPreparedGossip(WorldObject* source);
        void OnGossipSelect(WorldObject* source, uint32 gossipListId, uint32 menuId);

        uint32 GetGossipTextId(uint32 menuId, WorldObject* source) const;
        uint32 GetGossipTextId(WorldObject* source) const;
        static uint32 GetDefaultGossipMenuForSource(WorldObject* source);

        void SetHasMovedInUpdate(bool moved) { m_hasMovedInUpdate = moved; }
        bool GetHasMovedInUpdate() const { return m_hasMovedInUpdate; }

    protected:

        /*********************************************************/
        /***               BATTLEGROUND SYSTEM                 ***/
        /*********************************************************/

        /*
        this is an array of BG queues (BgTypeIDs) in which is player
        */
        struct BgBattlegroundQueueID_Rec
        {
            BattlegroundQueueTypeId bgQueueTypeId;
            uint32 invitedToInstance;
        };
        BgBattlegroundQueueID_Rec m_bgBattlegroundQueueID[PLAYER_MAX_BATTLEGROUND_QUEUES];
        BGData                    m_bgData;

#ifdef LICH_KING
        bool m_IsBGRandomWinner; //NYI
#endif

        uint8 m_bgAfkReportedCount;
        time_t m_bgAfkReportedTimer;
        uint32 m_regenTimerCount;
        uint32 m_contestedPvPTimer;

        uint32 m_bgTeam;    // what side the player will be added to

        /*********************************************************/
        /***                    QUEST SYSTEM                   ***/
        /*********************************************************/

        typedef std::set<uint32> QuestSet;
        typedef std::set<uint32> SeasonalQuestSet;
        typedef std::unordered_map<uint32, SeasonalQuestSet> SeasonalEventQuestMap;

        QuestSet m_timedquests;
#ifdef LICH_KING
        QuestSet m_weeklyquests;
        QuestSet m_monthlyquests;
#endif
        SeasonalEventQuestMap m_seasonalquests;

        ObjectGuid m_playerSharingQuest;
        uint32 m_sharedQuestId;
        uint32 m_ingametime;

        /*********************************************************/
        /***                   LOAD SYSTEM                     ***/
        /*********************************************************/

        void _LoadActions(PreparedQueryResult result);
        void _LoadAuras(PreparedQueryResult result, uint32 timediff);
        void _LoadBoundInstances(PreparedQueryResult result);
        void _LoadInventory(PreparedQueryResult result, uint32 timediff);
        void _LoadMailInit(PreparedQueryResult resultUnread, PreparedQueryResult resultDelivery);
        void _LoadMail();
        void _LoadMailedItems(Mail *mail);
        void _LoadQuestStatus(PreparedQueryResult result);
        void _LoadQuestStatusRewarded(PreparedQueryResult result);
        void _LoadDailyQuestStatus(PreparedQueryResult result);
#ifdef LICH_KING
        void _LoadWeeklyQuestStatus(PreparedQueryResult result);
        void _LoadMonthlyQuestStatus(PreparedQueryResult result);
        void _LoadRandomBGStatus(PreparedQueryResult result);
#endif
        void _LoadSeasonalQuestStatus(PreparedQueryResult result);
        void _LoadGroup(PreparedQueryResult result);
        void _LoadSkills(PreparedQueryResult result);
        void _LoadSpells(PreparedQueryResult result);
        bool _LoadHomeBind(PreparedQueryResult result);
        void _LoadDeclinedNames(PreparedQueryResult result);
        void _LoadArenaTeamInfo(PreparedQueryResult result);
        void _LoadBGData(PreparedQueryResult result);

        /*********************************************************/
        /***                   SAVE SYSTEM                     ***/
        /*********************************************************/

        void _SaveActions(SQLTransaction trans);
        void _SaveAuras(SQLTransaction trans);
        void _SaveInventory(SQLTransaction trans);
        void _SaveMail(SQLTransaction trans);
        void _SaveQuestStatus(SQLTransaction& trans);
        void _SaveDailyQuestStatus(SQLTransaction trans);
#ifdef LICH_KING
        void _SaveWeeklyQuestStatus(SQLTransaction& trans);
        void _SaveMonthlyQuestStatus(SQLTransaction& trans);
#endif
        void _SaveSeasonalQuestStatus(SQLTransaction& trans);
        void _SaveSpells(SQLTransaction trans);
        void _SaveSkills(SQLTransaction trans);
        void _SaveBGData(SQLTransaction& trans);

        /*********************************************************/
        /***              ENVIRONMENTAL SYSTEM                 ***/
        /*********************************************************/
        void HandleSobering();
        void SendMirrorTimer(MirrorTimerType Type, uint32 MaxValue, uint32 CurrentValue, int32 Regen);
        void StopMirrorTimer(MirrorTimerType Type);
        int32 getMaxTimer(MirrorTimerType timer);
        bool m_isInWater;

         // Current teleport data
        WorldLocation m_teleport_dest;
        uint32 m_teleport_options;
        bool mSemaphoreTeleport_Near;
        bool mSemaphoreTeleport_Far;

        uint32 m_DelayedOperations;
        bool m_bCanDelayTeleport;
        bool m_bHasDelayedTeleport;

        // Used to resurect a dead player into a ghost by 2 non consecutive steps
        bool m_isRepopPending;

        /*********************************************************/
        /***                  HONOR SYSTEM                     ***/
        /*********************************************************/
        time_t m_lastHonorUpdateTime;

        bool _removeSpell(uint16 spell_id);
        ObjectGuid m_lootGuid;

        uint8 m_race;
        uint8 m_class;
        uint8 m_gender;
        uint32 m_team;
        uint32 m_nextSave;
        time_t m_speakTime;
        uint32 m_speakCount;
        Difficulty m_dungeonDifficulty;

        uint32 m_atLoginFlags;

        Item* m_items[PLAYER_SLOTS_COUNT];
        uint32 m_currentBuybackSlot;

        uint32 m_ExtraFlags;

        ObjectGuid m_comboTarget;
        int8 m_comboPoints;

        QuestStatusMap m_QuestStatus;
        QuestStatusSaveMap m_QuestStatusSave;

        RewardedQuestSet m_RewardedQuests;
        QuestStatusSaveMap m_RewardedQuestsSave;

        SkillStatusMap mSkillStatus;

        uint32 _guildIdInvited;
        uint32 _arenaTeamIdInvited;

        PlayerMails m_mail;
        PlayerSpellMap m_spells;
        ReputationMgr*  m_reputationMgr;
#ifdef LICH_KING
        PlayerTalentMap* m_talents[MAX_TALENT_SPECS];
#endif

        ActionButtonList m_actionButtons;

        float m_auraBaseFlatMod[BASEMOD_END];
        float m_auraBasePctMod[BASEMOD_END];
        int16 m_baseRatingValue[MAX_COMBAT_RATING];
        SpellModContainer m_spellMods[MAX_SPELLMOD];
        EnchantDurationList m_enchantDuration;
        ItemDurationList m_itemDuration;

        ObjectGuid m_resurrectGUID;
        uint32 m_resurrectMap;
        float m_resurrectX, m_resurrectY, m_resurrectZ;
        uint32 m_resurrectHealth, m_resurrectMana;

        WorldSession *m_session;

        typedef std::list<Channel*> JoinedChannelsList;
        JoinedChannelsList m_channels;

        int m_cinematic;

        TradeData* m_trade;

        bool   m_DailyQuestChanged;
#ifdef LICH_KING
        bool   m_WeeklyQuestChanged;
        bool   m_MonthlyQuestChanged;
#endif
        bool   m_SeasonalQuestChanged;
        time_t m_lastDailyQuestTime;

        uint32 m_hostileReferenceCheckTimer;
        uint32 m_drunkTimer;
        uint16 m_drunk;
        uint32 m_weaponChangeTimer;

        uint32 m_zoneUpdateId;
        uint32 m_zoneUpdateTimer;
        uint32 m_areaUpdateId;

        uint32 m_deathTimer; // time left before forced releasing
        time_t m_deathExpireTime; // added delay expiration time

        uint32 m_restTime;

        uint32 m_teleportToTestInstanceId; //bypass most teleport checks...

        uint32 m_WeaponProficiency;
        uint32 m_ArmorProficiency;
        bool m_canParry;
        bool m_canBlock;
        uint8 m_swingErrorMsg;
        float m_ammoDPS;
        ////////////////////Rest System/////////////////////
        int time_inn_enter;
        uint32 inn_pos_mapid;
        float  inn_pos_x;
        float  inn_pos_y;
        float  inn_pos_z;
        float m_rest_bonus;
        RestType rest_type;
        ////////////////////Rest System/////////////////////

        uint32 m_resetTalentsCost;
        time_t m_resetTalentsTime;
        uint32 m_usedTalentCount;

        // Social
        PlayerSocial *m_social;

        // Groups
        GroupReference m_group;
        GroupReference m_originalGroup;
        Group *m_groupInvite;
        uint32 m_groupUpdateMask;
        uint64 m_auraUpdateMask;

        // Temporarily removed pet cache
        uint32 m_temporaryUnsummonedPetNumber;
        uint32 m_oldpetspell;

        uint32 _activeCheats; //mask from PlayerCommandStates

		bool CanAlwaysSee(WorldObject const* obj) const override;
		bool IsAlwaysDetectableFor(WorldObject const* seer) const override;
        bool CanNeverSee(WorldObject const* obj) const override;

		bool m_needsZoneUpdate;

#ifdef PLAYERBOT
        PlayerbotAI* m_playerbotAI;
        PlayerbotMgr* m_playerbotMgr;
#endif

        // Player summoning
        time_t m_summon_expire;
        uint32 m_summon_mapid;
        float  m_summon_x;
        float  m_summon_y;
        float  m_summon_z;
        bool   m_invite_summon;

        DeclinedName *m_declinedname;
        
        // Experience Blocking
        bool m_isXpBlocked;
        
        // Spirit of Redemption
        ObjectGuid m_spiritRedemptionKillerGUID;
        
        time_t _attackersCheckTime;
        
        bool m_bPassOnGroupLoot;
        
        // spectator system
        bool spectatorFlag;
        bool spectateCanceled;
        Player *spectateFrom;
       
        // true if player has moved in this update. In the previous system, in a player moved and stopped in the same update you had no way to know it. (this is used to fix spell not always properly interrupted)
        bool m_hasMovedInUpdate;

        //sun: keep EquipItem in private, they're really use to missuse
        Item* EquipItem(uint16 pos, Item *pItem, bool update, bool interruptSpells = true);

    private:
        // internal common parts for CanStore/StoreItem functions
        InventoryResult _CanStoreItem_InSpecificSlot( uint8 bag, uint8 slot, ItemPosCountVec& dest, ItemTemplate const *pProto, uint32& count, bool swap, Item *pSrcItem ) const;
        InventoryResult _CanStoreItem_InBag( uint8 bag, ItemPosCountVec& dest, ItemTemplate const *pProto, uint32& count, bool merge, bool non_specialized, Item *pSrcItem, uint8 skip_bag, uint8 skip_slot ) const;
        InventoryResult _CanStoreItem_InInventorySlots( uint8 slot_begin, uint8 slot_end, ItemPosCountVec& dest, ItemTemplate const *pProto, uint32& count, bool merge, Item *pSrcItem, uint8 skip_bag, uint8 skip_slot ) const;
        Item* _StoreItem( uint16 pos, Item *pItem, uint32 count, bool clone, bool update );
        Item* _LoadItem(SQLTransaction& trans, uint32 zoneId, uint32 timeDiff, Field* fields);

        CinematicMgr* _cinematicMgr;

        int32 m_MirrorTimer[MAX_TIMERS];
        uint8 m_MirrorTimerFlags;
        uint8 m_MirrorTimerFlagsLast;

        MapReference m_mapRef;

        UnitAI *i_AI;
        
        uint32 m_lastFallTime;
        float  m_lastFallZ;

        uint32 m_lastOpenLockKey;
        
        float m_customXp;
        
        SpamReports _spamReports;
        time_t _lastSpamAlert; // When was the last time we reported this ugly spammer to the staff?

		WorldLocation _corpseLocation;

    public:
        bool m_kickatnextupdate;
        uint32 m_swdBackfireDmg;
};

void AddItemsSetItem(Player*player,Item *item);
void RemoveItemsSetItem(Player*player,ItemTemplate const *proto);

#endif
