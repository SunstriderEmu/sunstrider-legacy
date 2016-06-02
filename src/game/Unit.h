#ifndef __UNIT_H
#define __UNIT_H

#include "Common.h"
#include "Object.h"
#include "Opcodes.h"
#include "Mthread.h"
#include "SpellAuraDefines.h"
#include "UpdateFields.h"
#include "SharedDefines.h"
#include "ThreatManager.h"
#include "HostileRefManager.h"
#include "Movement/FollowerReference.h"
#include "Movement/FollowerRefManager.h"
#include "EventProcessor.h"
#include "Movement/MotionMaster.h"
#include "DBCStructure.h"
#include "Util.h"
#include <list>
#include "SpellInfo.h"
class UnitAI;
class SpellCastTargets;
class Aura;
class TemporarySummon;

#define WORLD_TRIGGER   12999

//TC compat
typedef Aura AuraApplication;

enum SpellInterruptFlags
{
    SPELL_INTERRUPT_FLAG_MOVEMENT     = 0x01, // why need this for instant?
    SPELL_INTERRUPT_FLAG_PUSH_BACK    = 0x02, // push back
    SPELL_INTERRUPT_FLAG_INTERRUPT    = 0x04, // interrupt
    SPELL_INTERRUPT_FLAG_AUTOATTACK   = 0x08, // no
    SPELL_INTERRUPT_FLAG_DAMAGE       = 0x10  // _complete_ interrupt on direct damage?
};

enum SpellChannelInterruptFlags
{
    CHANNEL_FLAG_DAMAGE      = 0x0002,
    CHANNEL_FLAG_MOVEMENT    = 0x0008,
    CHANNEL_FLAG_TURNING     = 0x0010,
    CHANNEL_FLAG_DAMAGE2     = 0x0080,
    CHANNEL_FLAG_DELAY       = 0x4000
};

enum SpellAuraInterruptFlags
{
    AURA_INTERRUPT_FLAG_HITBYSPELL          = 0x00000001,   // 0    removed when getting hit by a negative spell?
    AURA_INTERRUPT_FLAG_TAKE_DAMAGE              = 0x00000002,   // 1    removed by any damage
    AURA_INTERRUPT_FLAG_CAST                = 0x00000004,   // 2    cast any spells
    AURA_INTERRUPT_FLAG_MOVE                = 0x00000008,   // 3    removed by any movement
    AURA_INTERRUPT_FLAG_TURNING             = 0x00000010,   // 4    removed by any turning
    AURA_INTERRUPT_FLAG_JUMP                = 0x00000020,   // 5    removed by entering combat
    AURA_INTERRUPT_FLAG_NOT_MOUNTED         = 0x00000040,   // 6    removed by unmounting
    AURA_INTERRUPT_FLAG_NOT_ABOVEWATER      = 0x00000080,   // 7    removed by entering water
    AURA_INTERRUPT_FLAG_NOT_UNDERWATER      = 0x00000100,   // 8    removed by leaving water
    AURA_INTERRUPT_FLAG_NOT_SHEATHED        = 0x00000200,   // 9    removed by unsheathing
    AURA_INTERRUPT_FLAG_TALK                = 0x00000400,   // 10   talk to npc / loot? action on creature
    AURA_INTERRUPT_FLAG_USE                 = 0x00000800,   // 11   mine/use/open action on gameobject
    AURA_INTERRUPT_FLAG_MELEE_ATTACK        = 0x00001000,   // 12   removed by attacking
    AURA_INTERRUPT_FLAG_SPELL_ATTACK        = 0x00002000,   // 13   ???
    AURA_INTERRUPT_FLAG_UNK14               = 0x00004000,   // 14
    AURA_INTERRUPT_FLAG_TRANSFORM           = 0x00008000,   // 15   removed by transform?
    AURA_INTERRUPT_FLAG_UNK16               = 0x00010000,   // 16
    AURA_INTERRUPT_FLAG_MOUNT               = 0x00020000,   // 17   misdirect, aspect, swim speed
    AURA_INTERRUPT_FLAG_NOT_SEATED          = 0x00040000,   // 18   removed by standing up
    AURA_INTERRUPT_FLAG_CHANGE_MAP          = 0x00080000,   // 19   leaving map/getting teleported
    AURA_INTERRUPT_FLAG_UNATTACKABLE        = 0x00100000,   // 20   invulnerable or stealth
    AURA_INTERRUPT_FLAG_UNK21               = 0x00200000,   // 21
    AURA_INTERRUPT_FLAG_TELEPORTED          = 0x00400000,   // 22
    AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT    = 0x00800000,   // 23   removed by entering pvp combat
    AURA_INTERRUPT_FLAG_DIRECT_DAMAGE       = 0x01000000    // 24   removed by any direct damage
#ifdef LICH_KING
    AURA_INTERRUPT_FLAG_LANDING             = 0x02000000,   // 25   removed by hitting the ground
    AURA_INTERRUPT_FLAG_LEAVE_COMBAT        = 0x80000000,   // 31   removed by leaving combat
#endif
};

enum SpellModOp
{
    SPELLMOD_DAMAGE                 = 0,
    SPELLMOD_DURATION               = 1,
    SPELLMOD_THREAT                 = 2,
    SPELLMOD_EFFECT1                = 3,
    SPELLMOD_CHARGES                = 4,
    SPELLMOD_RANGE                  = 5,
    SPELLMOD_RADIUS                 = 6,
    SPELLMOD_CRITICAL_CHANCE        = 7,
    SPELLMOD_ALL_EFFECTS            = 8,
    SPELLMOD_NOT_LOSE_CASTING_TIME  = 9,
    SPELLMOD_CASTING_TIME           = 10,
    SPELLMOD_COOLDOWN               = 11,
    SPELLMOD_EFFECT2                = 12,
    // spellmod 13 unused
    SPELLMOD_COST                   = 14,
    SPELLMOD_CRIT_DAMAGE_BONUS      = 15,
    SPELLMOD_RESIST_MISS_CHANCE     = 16,
    SPELLMOD_JUMP_TARGETS           = 17,
    SPELLMOD_CHANCE_OF_SUCCESS      = 18,
    SPELLMOD_ACTIVATION_TIME        = 19,
    SPELLMOD_DAMAGE_MULTIPLIER      = 20,
    SPELLMOD_CASTING_TIME_OLD       = 21,
    SPELLMOD_DOT                    = 22,
    SPELLMOD_EFFECT3                = 23,
    SPELLMOD_BONUS_MULTIPLIER       = 24,
    // spellmod 25, 26 unused
    SPELLMOD_VALUE_MULTIPLIER       = 27,
    SPELLMOD_RESIST_DISPEL_CHANCE   = 28
};

#define MAX_SPELLMOD 32

enum SpellValueMod
{
    SPELLVALUE_BASE_POINT0,
    SPELLVALUE_BASE_POINT1,
    SPELLVALUE_BASE_POINT2,
    SPELLVALUE_MAX_TARGETS,
    SPELLVALUE_MOD_RADIUS,
};

typedef std::pair<SpellValueMod, int32>     CustomSpellValueMod;
class CustomSpellValues : public std::vector<CustomSpellValueMod>
{
    public:
        void AddSpellMod(SpellValueMod mod, int32 value)
        {
            push_back(std::make_pair(mod, value));
        }
};

enum SpellFacingFlags
{
    SPELL_FACING_FLAG_INFRONT = 0x0001
};

#define BASE_MINDAMAGE 1.0f
#define BASE_MAXDAMAGE 2.0f
#define BASE_ATTACK_TIME 2000

#define MAX_AGGRO_RESET_TIME 10 // in seconds

#define DEFAULT_HOVER_HEIGHT 1.0f

// byte value (UNIT_FIELD_BYTES_1,0)
enum UnitStandStateType
{
    UNIT_STAND_STATE_STAND             = 0,
    UNIT_STAND_STATE_SIT               = 1,
    UNIT_STAND_STATE_SIT_CHAIR         = 2,
    UNIT_STAND_STATE_SLEEP             = 3,
    UNIT_STAND_STATE_SIT_LOW_CHAIR     = 4,
    UNIT_STAND_STATE_SIT_MEDIUM_CHAIR  = 5,
    UNIT_STAND_STATE_SIT_HIGH_CHAIR    = 6,
    UNIT_STAND_STATE_DEAD              = 7,
    UNIT_STAND_STATE_KNEEL             = 8,
    UNIT_STAND_STATE_SUBMERGED         = 9
};

// byte flag value (UNIT_FIELD_BYTES_1, 2)
enum UnitStandFlags
{
    UNIT_STAND_FLAGS_UNK1         = 0x01,
    UNIT_STAND_FLAGS_CREEP        = 0x02,
    UNIT_STAND_FLAGS_UNTRACKABLE  = 0x04,
    UNIT_STAND_FLAGS_UNK4         = 0x08,
    UNIT_STAND_FLAGS_UNK5         = 0x10,
    UNIT_STAND_FLAGS_ALL          = 0xFF
};

enum UnitBytes0Offsets
{
    UNIT_BYTES_0_OFFSET_RACE        = 0,
    UNIT_BYTES_0_OFFSET_CLASS       = 1,
    UNIT_BYTES_0_OFFSET_GENDER      = 2,
    UNIT_BYTES_0_OFFSET_POWER_TYPE  = 3,
};

// byte flags value (UNIT_FIELD_BYTES_1, 3)
enum UnitBytes1_Flags
{
    UNIT_BYTE1_FLAG_ALWAYS_STAND    = 0x01,
    UNIT_BYTE1_FLAG_HOVER           = 0x02,
    UNIT_BYTE1_FLAG_UNK_3           = 0x04,
    UNIT_BYTE1_FLAG_ALL             = 0xFF
};

// high byte (3 from 0..3) of UNIT_FIELD_BYTES_2
enum ShapeshiftForm
{
    FORM_NONE               = 0x00,
    FORM_CAT                = 0x01,
    FORM_TREE               = 0x02,
    FORM_TRAVEL             = 0x03,
    FORM_AQUA               = 0x04,
    FORM_BEAR               = 0x05,
    FORM_AMBIENT            = 0x06,
    FORM_GHOUL              = 0x07,
    FORM_DIREBEAR           = 0x08,
    FORM_CREATUREBEAR       = 0x0E,
    FORM_CREATURECAT        = 0x0F,
    FORM_GHOSTWOLF          = 0x10,
    FORM_BATTLESTANCE       = 0x11,
    FORM_DEFENSIVESTANCE    = 0x12,
    FORM_BERSERKERSTANCE    = 0x13,
    FORM_TEST               = 0x14,
    FORM_ZOMBIE             = 0x15,
    FORM_FLIGHT_EPIC        = 0x1B,
    FORM_SHADOW             = 0x1C,
    FORM_FLIGHT             = 0x1D,
    FORM_STEALTH            = 0x1E,
    FORM_MOONKIN            = 0x1F,
    FORM_SPIRITOFREDEMPTION = 0x20
};

// low byte ( 0 from 0..3 ) of UNIT_FIELD_BYTES_2
enum SheathState
{
    SHEATH_STATE_UNARMED  = 0,                              // non prepared weapon
    SHEATH_STATE_MELEE    = 1,                              // prepared melee weapon
    SHEATH_STATE_RANGED   = 2                               // prepared ranged weapon
};

#define MAX_SHEATH_STATE    3

// byte (1 from 0..3) of UNIT_FIELD_BYTES_2
enum UnitBytes2_Flags
{
    UNIT_BYTE2_FLAG_UNK0  = 0x01,
    UNIT_BYTE2_FLAG_UNK1  = 0x02,
    UNIT_BYTE2_FLAG_UNK2  = 0x04,
#ifdef LICH_KING
    UNIT_BYTE2_FLAG_SANCTUARY = 0x08,
#else
    UNIT_BYTE2_FLAG_UNK3  = 0x08, //old UNIT_BYTE2_FLAG_UNK3 but this does not seemed ok
#endif
    UNIT_BYTE2_FLAG_AURAS = 0x10,                           // show possitive auras as positive, and allow its dispel
    UNIT_BYTE2_FLAG_UNK5  = 0x20,
    UNIT_BYTE2_FLAG_UNK6  = 0x40,
    UNIT_BYTE2_FLAG_UNK7  = 0x80
};

// byte (2 from 0..3) of UNIT_FIELD_BYTES_2
enum UnitRename
{
    UNIT_RENAME_NOT_ALLOWED = 0x02,
    UNIT_RENAME_ALLOWED     = 0x03
};

#define CREATURE_MAX_SPELLS     8

enum VictimState
{
    VICTIMSTATE_UNKNOWN1       = 0,
    VICTIMSTATE_NORMAL         = 1,
    VICTIMSTATE_DODGE          = 2,
    VICTIMSTATE_PARRY          = 3,
    VICTIMSTATE_INTERRUPT      = 4,
    VICTIMSTATE_BLOCKS         = 5,
    VICTIMSTATE_EVADES         = 6,
    VICTIMSTATE_IS_IMMUNE      = 7,
    VICTIMSTATE_DEFLECTS       = 8
};

enum HitInfo
{
    HITINFO_NORMALSWING         = 0x00000000,
    HITINFO_UNK1                = 0x00000001,               // req correct packet structure
#ifdef LICH_KING
    HITINFO_AFFECTS_VICTIM      = 0x00000002,
    HITINFO_OFFHAND             = 0x00000004,
    HITINFO_UNK2                = 0x00000008,
    HITINFO_MISS                = 0x00000010,
    HITINFO_FULL_ABSORB         = 0x00000020,
    HITINFO_PARTIAL_ABSORB      = 0x00000040,
    HITINFO_FULL_RESIST         = 0x00000080,
    HITINFO_PARTIAL_RESIST      = 0x00000100,
    HITINFO_CRITICALHIT         = 0x00000200,               // critical hit
    // 0x00000400
    // 0x00000800
    // 0x00001000
    HITINFO_BLOCK               = 0x00002000,               // blocked damage
    // 0x00004000                                           // Hides worldtext for 0 damage
    // 0x00008000                                           // Related to blood visual
    HITINFO_GLANCING            = 0x00010000,
    HITINFO_CRUSHING            = 0x00020000,
    HITINFO_NO_ANIMATION        = 0x00040000,
    // 0x00080000
    // 0x00100000
    HITINFO_SWINGNOHITSOUND     = 0x00200000,               // unused?
    // 0x00400000
    HITINFO_RAGE_GAIN           = 0x00800000,
#else
    HITINFO_NORMALSWING2        = 0x00000002,
    HITINFO_LEFTSWING           = 0x00000004,
    HITINFO_MISS                = 0x00000010,
    HITINFO_ABSORB              = 0x00000020,               // plays absorb sound
    HITINFO_RESIST              = 0x00000040,               // resisted at least some damage
    HITINFO_CRITICALHIT         = 0x00000080,
    HITINFO_UNK2                = 0x00000100,               // 
    HITINFO_UNK3                = 0x00002000,               // 
    HITINFO_GLANCING            = 0x00004000,
    HITINFO_CRUSHING            = 0x00008000,
    HITINFO_NOACTION            = 0x00010000,
    HITINFO_SWINGNOHITSOUND     = 0x00080000,
#endif
};

//i would like to remove this: (it is defined in item.h
enum InventorySlot
{
    NULL_BAG = 0,
    NULL_SLOT = 255
};

struct FactionTemplateEntry;
class SpellInfo;
struct SpellValue;

class Aura;
class Creature;
class Spell;
class DynamicObject;
class GameObject;
class Item;
class Pet;
class Totem;
class PetAura;
class BigNumber;

//compat TC
typedef Aura AuraEffect;

struct SpellImmune
{
    uint32 type;
    uint32 spellId;
};

typedef std::list<SpellImmune> SpellImmuneList;

enum UnitModifierType
{
    BASE_VALUE = 0,
    BASE_PCT = 1,
    TOTAL_VALUE = 2,
    TOTAL_PCT = 3,
    MODIFIER_TYPE_END = 4
};

enum WeaponDamageRange
{
    MINDAMAGE,
    MAXDAMAGE
};

enum DamageTypeToSchool
{
    RESISTANCE,
    DAMAGE_DEALT,
    DAMAGE_TAKEN
};

enum UnitMods
{
    UNIT_MOD_STAT_STRENGTH,                                 // UNIT_MOD_STAT_STRENGTH..UNIT_MOD_STAT_SPIRIT must be in existed order, it's accessed by index values of Stats enum.
    UNIT_MOD_STAT_AGILITY,
    UNIT_MOD_STAT_STAMINA,
    UNIT_MOD_STAT_INTELLECT,
    UNIT_MOD_STAT_SPIRIT,
    UNIT_MOD_HEALTH,
    UNIT_MOD_MANA,                                          // UNIT_MOD_MANA..UNIT_MOD_HAPPINESS must be in existed order, it's accessed by index values of Powers enum.
    UNIT_MOD_RAGE,
    UNIT_MOD_FOCUS,
    UNIT_MOD_ENERGY,
    UNIT_MOD_HAPPINESS,
    UNIT_MOD_ARMOR,                                         // UNIT_MOD_ARMOR..UNIT_MOD_RESISTANCE_ARCANE must be in existed order, it's accessed by index values of SpellSchools enum.
    UNIT_MOD_RESISTANCE_HOLY,
    UNIT_MOD_RESISTANCE_FIRE,
    UNIT_MOD_RESISTANCE_NATURE,
    UNIT_MOD_RESISTANCE_FROST,
    UNIT_MOD_RESISTANCE_SHADOW,
    UNIT_MOD_RESISTANCE_ARCANE,
    UNIT_MOD_ATTACK_POWER,
    UNIT_MOD_ATTACK_POWER_RANGED,
    UNIT_MOD_DAMAGE_MAINHAND,
    UNIT_MOD_DAMAGE_OFFHAND,
    UNIT_MOD_DAMAGE_RANGED,
    UNIT_MOD_END,
    // synonyms
    UNIT_MOD_STAT_START = UNIT_MOD_STAT_STRENGTH,
    UNIT_MOD_STAT_END = UNIT_MOD_STAT_SPIRIT + 1,
    UNIT_MOD_RESISTANCE_START = UNIT_MOD_ARMOR,
    UNIT_MOD_RESISTANCE_END = UNIT_MOD_RESISTANCE_ARCANE + 1,
    UNIT_MOD_POWER_START = UNIT_MOD_MANA,
    UNIT_MOD_POWER_END = UNIT_MOD_HAPPINESS + 1
};

enum BaseModGroup
{
    CRIT_PERCENTAGE,
    RANGED_CRIT_PERCENTAGE,
    OFFHAND_CRIT_PERCENTAGE,
    SHIELD_BLOCK_VALUE,
    BASEMOD_END
};

enum BaseModType
{
    FLAT_MOD,
    PCT_MOD,
    MOD_END
};

enum DeathState
{
    ALIVE          = 0,
    JUST_DIED      = 1,
    CORPSE         = 2, //dead and corpse still spawned
    DEAD           = 3, //dead and corpse disappeared
    JUST_RESPAWNED = 4
};

enum UnitState
{
    UNIT_STATE_DIED                  = 0x00000001,
    UNIT_STATE_MELEE_ATTACKING       = 0x00000002,                     // player is melee attacking someone
    //UNIT_STATE_MELEE_ATTACK_BY     = 0x00000004,                     // player is melee attack by someone
    UNIT_STATE_STUNNED               = 0x00000008,
    UNIT_STATE_ROAMING               = 0x00000010,
    UNIT_STATE_CHASE                 = 0x00000020,
    //UNIT_STATE_SEARCHING           = 0x00000040,
    UNIT_STATE_FLEEING               = 0x00000080,
    UNIT_STATE_IN_FLIGHT             = 0x00000100,                     // player is in flight mode
    UNIT_STATE_FOLLOW                = 0x00000200,
    UNIT_STATE_ROOT                  = 0x00000400,
    UNIT_STATE_CONFUSED              = 0x00000800,
    UNIT_STATE_DISTRACTED            = 0x00001000,
    UNIT_STATE_ISOLATED              = 0x00002000,                     // area auras do not affect other players
    UNIT_STATE_ATTACK_PLAYER         = 0x00004000,
    UNIT_STATE_CASTING               = 0x00008000,
    UNIT_STATE_POSSESSED             = 0x00010000,
    UNIT_STATE_CHARGING              = 0x00020000,
    UNIT_STATE_JUMPING               = 0x00040000,
    UNIT_STATE_MOVE                  = 0x00100000,
    UNIT_STATE_ROTATING              = 0x00200000,
    UNIT_STATE_EVADE                 = 0x00400000,
    UNIT_STATE_ROAMING_MOVE          = 0x00800000,
    UNIT_STATE_CONFUSED_MOVE         = 0x01000000,
    UNIT_STATE_FLEEING_MOVE          = 0x02000000,
    UNIT_STATE_CHASE_MOVE            = 0x04000000,
    UNIT_STATE_FOLLOW_MOVE           = 0x08000000,
    UNIT_STATE_IGNORE_PATHFINDING    = 0x10000000,                 // do not use pathfinding in any MovementGenerator
    UNIT_STATE_ALL_STATE_SUPPORTED = UNIT_STATE_DIED | UNIT_STATE_MELEE_ATTACKING | UNIT_STATE_STUNNED | UNIT_STATE_ROAMING | UNIT_STATE_CHASE
                                   | UNIT_STATE_FLEEING | UNIT_STATE_IN_FLIGHT | UNIT_STATE_FOLLOW | UNIT_STATE_ROOT | UNIT_STATE_CONFUSED
                                   | UNIT_STATE_DISTRACTED | UNIT_STATE_ISOLATED | UNIT_STATE_ATTACK_PLAYER | UNIT_STATE_CASTING
                                   | UNIT_STATE_POSSESSED | UNIT_STATE_CHARGING | UNIT_STATE_JUMPING | UNIT_STATE_MOVE | UNIT_STATE_ROTATING
                                   | UNIT_STATE_EVADE | UNIT_STATE_ROAMING_MOVE | UNIT_STATE_CONFUSED_MOVE | UNIT_STATE_FLEEING_MOVE
                                   | UNIT_STATE_CHASE_MOVE | UNIT_STATE_FOLLOW_MOVE | UNIT_STATE_IGNORE_PATHFINDING,
    UNIT_STATE_UNATTACKABLE    = UNIT_STATE_IN_FLIGHT,
    // for real move using movegen check and stop (except unstoppable flight)
    UNIT_STATE_MOVING          = UNIT_STATE_ROAMING_MOVE | UNIT_STATE_CONFUSED_MOVE | UNIT_STATE_FLEEING_MOVE | UNIT_STATE_CHASE_MOVE | UNIT_STATE_FOLLOW_MOVE,
    UNIT_STATE_CONTROLLED      = (UNIT_STATE_CONFUSED | UNIT_STATE_STUNNED | UNIT_STATE_FLEEING),
    UNIT_STATE_LOST_CONTROL    = (UNIT_STATE_CONTROLLED | UNIT_STATE_JUMPING | UNIT_STATE_CHARGING),
    UNIT_STATE_SIGHTLESS       = (UNIT_STATE_LOST_CONTROL | UNIT_STATE_EVADE),
    UNIT_STATE_CANNOT_AUTOATTACK     = (UNIT_STATE_LOST_CONTROL | UNIT_STATE_CASTING),
    UNIT_STATE_CANNOT_TURN     = (UNIT_STATE_LOST_CONTROL | UNIT_STATE_ROTATING),
    // stay by different reasons
    UNIT_STATE_NOT_MOVE        = UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DIED | UNIT_STATE_DISTRACTED,
    UNIT_STATE_ALL_STATE       = 0xffffffff                      //(UNIT_STATE_STOPPED | UNIT_STATE_MOVING | UNIT_STATE_IN_COMBAT | UNIT_STATE_IN_FLIGHT)
};

enum UnitMoveType
{
    MOVE_WALK           = 0,
    MOVE_RUN            = 1,
    MOVE_RUN_BACK       = 2,
    MOVE_SWIM           = 3,
    MOVE_SWIM_BACK      = 4,
    MOVE_TURN_RATE      = 5,
    MOVE_FLIGHT         = 6,
    MOVE_FLIGHT_BACK    = 7,
};

#define MAX_MOVE_TYPE 8

extern float baseMoveSpeed[MAX_MOVE_TYPE];
// assume it is 25 yard per 0.6 second
#define SPEED_CHARGE    42.0f

enum WeaponAttackType
{
    BASE_ATTACK   = 0,
    OFF_ATTACK    = 1,
    RANGED_ATTACK = 2,
    MAX_ATTACK    = 3
};

enum CombatRating
{
    CR_WEAPON_SKILL             = 0,
    CR_DEFENSE_SKILL            = 1,
    CR_DODGE                    = 2,
    CR_PARRY                    = 3,
    CR_BLOCK                    = 4,
    CR_HIT_MELEE                = 5,
    CR_HIT_RANGED               = 6,
    CR_HIT_SPELL                = 7,
    CR_CRIT_MELEE               = 8,
    CR_CRIT_RANGED              = 9,
    CR_CRIT_SPELL               = 10,
    CR_HIT_TAKEN_MELEE          = 11,
    CR_HIT_TAKEN_RANGED         = 12,
    CR_HIT_TAKEN_SPELL          = 13,
    CR_CRIT_TAKEN_MELEE         = 14,
    CR_CRIT_TAKEN_RANGED        = 15,
    CR_CRIT_TAKEN_SPELL         = 16,
    CR_HASTE_MELEE              = 17,
    CR_HASTE_RANGED             = 18,
    CR_HASTE_SPELL              = 19,
    CR_WEAPON_SKILL_MAINHAND    = 20,
    CR_WEAPON_SKILL_OFFHAND     = 21,
    CR_WEAPON_SKILL_RANGED      = 22,
    CR_EXPERTISE                = 23,
    MAX_COMBAT_RATING           = 24
};

enum DamageEffectType
{
    DIRECT_DAMAGE           = 0,                            // used for normal weapon damage (not for class abilities or spells)
    SPELL_DIRECT_DAMAGE     = 1,                            // spell/class abilities damage
    DOT                     = 2,
    HEAL                    = 3,
    NODAMAGE                = 4,                            // used also in case when damage applied to health but not applied to spell channelInterruptFlags/etc
    SELF_DAMAGE             = 5
};

enum UnitVisibility
{
    VISIBILITY_OFF                = 0,                      // absolute, not detectable, GM-like, can see all other
    VISIBILITY_ON                 = 1,
    VISIBILITY_GROUP_STEALTH      = 2,                      // detect chance, seen and can see group members
    //VISIBILITY_GROUP_INVISIBILITY = 3,                      // invisibility, can see and can be seen only another invisible unit or invisible detection unit, set only if not stealthed, and in checks not used (mask used instead)
    //VISIBILITY_GROUP_NO_DETECT    = 4,                      // state just at stealth apply for update Grid state. Don't remove, otherwise stealth spells will break
    VISIBILITY_RESPAWN            = 5                       // special totally not detectable visibility for force delete object at respawn command
};

// Value masks for UNIT_FIELD_FLAGS
enum UnitFlags
{
    UNIT_FLAG_SERVER_CONTROLLED     = 0x00000001,                // set only when unit movement is controlled by server - by SPLINE/MONSTER_MOVE packets, together with UNIT_FLAG_STUNNED; only set to units controlled by client; client function CGUnit_C::IsClientControlled returns false when set for owner        
    UNIT_FLAG_NON_ATTACKABLE        = 0x00000002,                // not attackable
    UNIT_FLAG_REMOVE_CLIENT_CONTROL = 0x00000004,
    UNIT_FLAG_PVP_ATTACKABLE        = 0x00000008,                // allow apply pvp rules to attackable state in addition to faction dependent state
    UNIT_FLAG_RENAME                = 0x00000010,
    UNIT_FLAG_PREPARATION           = 0x00000020,                // don't take reagents for spells with SPELL_ATTR5_NO_REAGENT_WHILE_PREP
    UNIT_FLAG_UNK_6                 = 0x00000040,
    UNIT_FLAG_NOT_ATTACKABLE_1      = 0x00000080,                // ?? (UNIT_FLAG_PVP_ATTACKABLE | UNIT_FLAG_NOT_ATTACKABLE_1) is NON_PVP_ATTACKABLE
    UNIT_FLAG_IMMUNE_TO_PC          = 0x00000100,                // disables combat/assistance with PlayerCharacters (PC) - see Unit::_IsValidAttackTarget, Unit::_IsValidAssistTarget
    UNIT_FLAG_IMMUNE_TO_NPC         = 0x00000200,                // disables combat/assistance with NonPlayerCharacters (NPC) - see Unit::_IsValidAttackTarget, Unit::_IsValidAssistTarget
    UNIT_FLAG_LOOTING               = 0x00000400,                // loot animation
    UNIT_FLAG_PET_IN_COMBAT         = 0x00000800,                // in combat?, 2.0.8
    UNIT_FLAG_PVP                   = 0x00001000,                // changed in 3.0.3
    UNIT_FLAG_SILENCED              = 0x00002000,                // silenced, 2.1.1
    UNIT_FLAG_UNK_14                = 0x00004000,                // 2.0.8
    UNIT_FLAG_UNK_15                = 0x00008000,
    UNIT_FLAG_UNK_16                = 0x00010000,
    UNIT_FLAG_PACIFIED              = 0x00020000,
    UNIT_FLAG_STUNNED               = 0x00040000,                // stunned, 2.1.1 old DISABLE_ROTATE
    UNIT_FLAG_IN_COMBAT             = 0x00080000,
    UNIT_FLAG_TAXI_FLIGHT           = 0x00100000,                // disable casting at client side spell not allowed by taxi flight (mounted?), probably used with 0x4 flag
    UNIT_FLAG_DISARMED              = 0x00200000,                // disable melee spells casting..., "Required melee weapon" added to melee spells tooltip.
    UNIT_FLAG_CONFUSED              = 0x00400000,
    UNIT_FLAG_FLEEING               = 0x00800000,
    UNIT_FLAG_PLAYER_CONTROLLED     = 0x01000000,                // used in spell Eyes of the Beast for pet... Let attack by controlled creature
    UNIT_FLAG_NOT_SELECTABLE        = 0x02000000,
    UNIT_FLAG_SKINNABLE             = 0x04000000,
    UNIT_FLAG_MOUNT                 = 0x08000000,
    UNIT_FLAG_UNK_28                = 0x10000000,
    UNIT_FLAG_UNK_29                = 0x20000000,                // used in Feing Death spell
    UNIT_FLAG_SHEATHE               = 0x40000000,
    UNIT_FLAG_UNK_31                = 0x80000000,
    MAX_UNIT_FLAGS                  = 33
};

// Value masks for UNIT_FIELD_FLAGS_2
enum UnitFlags2
{
    UNIT_FLAG2_FEIGN_DEATH       = 0x00000001,
    UNIT_FLAG2_UNK1              = 0x00000002,   // Hide unit model (show only player equip)
    UNIT_FLAG2_IGNORE_REPUTATION = 0x00000004,
    UNIT_FLAG2_COMPREHEND_LANG   = 0x00000008,
    UNIT_FLAG2_MIRROR_IMAGE      = 0x00000010,
    UNIT_FLAG2_FORCE_MOVE        = 0x00000040,
    //not sure where do the BC ones stop here
#ifdef LICH_KING
    UNIT_FLAG2_DISARM_OFFHAND = 0x00000080,
    UNIT_FLAG2_DISABLE_PRED_STATS = 0x00000100,   // Player has disabled predicted stats (Used by raid frames)
    UNIT_FLAG2_DISARM_RANGED = 0x00000400,   // this does not disable ranged weapon display (maybe additional flag needed?)
    UNIT_FLAG2_REGENERATE_POWER = 0x00000800,
    UNIT_FLAG2_RESTRICT_PARTY_INTERACTION = 0x00001000,   // Restrict interaction to party or raid
    UNIT_FLAG2_PREVENT_SPELL_CLICK = 0x00002000,   // Prevent spellclick
    UNIT_FLAG2_ALLOW_ENEMY_INTERACT = 0x00004000,
    UNIT_FLAG2_DISABLE_TURN = 0x00008000,
    UNIT_FLAG2_UNK2 = 0x00010000,
    UNIT_FLAG2_PLAY_DEATH_ANIM = 0x00020000,   // Plays special death animation upon death
    UNIT_FLAG2_ALLOW_CHEAT_SPELLS = 0x00040000,   // Allows casting spells with AttributesEx7 & SPELL_ATTR7_IS_CHEAT_SPELL
#endif
};

/// Non Player Character flags
enum NPCFlags
{
    UNIT_NPC_FLAG_NONE                  = 0x00000000,
    UNIT_NPC_FLAG_GOSSIP                = 0x00000001,       // 100%
    UNIT_NPC_FLAG_QUESTGIVER            = 0x00000002,       // guessed, probably ok
    UNIT_NPC_FLAG_UNK1                  = 0x00000004,
    UNIT_NPC_FLAG_UNK2                  = 0x00000008,
    UNIT_NPC_FLAG_TRAINER               = 0x00000010,       // 100%
    UNIT_NPC_FLAG_TRAINER_CLASS         = 0x00000020,       // 100%
    UNIT_NPC_FLAG_TRAINER_PROFESSION    = 0x00000040,       // 100%
    UNIT_NPC_FLAG_VENDOR                = 0x00000080,       // 100%
    UNIT_NPC_FLAG_VENDOR_AMMO           = 0x00000100,       // 100%, general goods vendor
    UNIT_NPC_FLAG_VENDOR_FOOD           = 0x00000200,       // 100%
    UNIT_NPC_FLAG_VENDOR_POISON         = 0x00000400,       // guessed
    UNIT_NPC_FLAG_VENDOR_REAGENT        = 0x00000800,       // 100%
    UNIT_NPC_FLAG_REPAIR                = 0x00001000,       // 100%
    UNIT_NPC_FLAG_FLIGHTMASTER          = 0x00002000,       // 100%
    UNIT_NPC_FLAG_SPIRITHEALER          = 0x00004000,       // guessed
    UNIT_NPC_FLAG_SPIRITGUIDE           = 0x00008000,       // guessed
    UNIT_NPC_FLAG_INNKEEPER             = 0x00010000,       // 100%
    UNIT_NPC_FLAG_BANKER                = 0x00020000,       // 100%
    UNIT_NPC_FLAG_PETITIONER            = 0x00040000,       // 100% 0xC0000 = guild petitions, 0x40000 = arena team petitions
    UNIT_NPC_FLAG_TABARDDESIGNER        = 0x00080000,       // 100%
    UNIT_NPC_FLAG_BATTLEMASTER          = 0x00100000,       // 100%
    UNIT_NPC_FLAG_AUCTIONEER            = 0x00200000,       // 100%
    UNIT_NPC_FLAG_STABLEMASTER          = 0x00400000,       // 100%
    UNIT_NPC_FLAG_GUILD_BANKER          = 0x00800000,       // cause client to send 997 opcode
    UNIT_NPC_FLAG_SPELLCLICK            = 0x01000000,       // cause client to send 1015 opcode (spell click)
};

enum MoveFlags
{
    MOVEFLAG_NONE               = 0x00000000,
    MOVEFLAG_SLIDE              = 0x00000002,
    MOVEFLAG_MARCH_ON_SPOT      = 0x00000004,
    MOVEFLAG_JUMP               = 0x00000008,
    MOVEFLAG_WALK               = 0x00000100,
    MOVEFLAG_FLY                = 0x00000200,   //For dragon (+walk = glide)
    MOVEFLAG_ORIENTATION        = 0x00000400,   //Fix orientation
};

enum MovementFlags
{
    MOVEMENTFLAG_NONE           = 0x00000000,
    MOVEMENTFLAG_FORWARD        = 0x00000001,
    MOVEMENTFLAG_BACKWARD       = 0x00000002,
    MOVEMENTFLAG_STRAFE_LEFT    = 0x00000004,
    MOVEMENTFLAG_STRAFE_RIGHT   = 0x00000008,
    MOVEMENTFLAG_LEFT           = 0x00000010,               // Turning left
    MOVEMENTFLAG_RIGHT          = 0x00000020,               // Turning right
    MOVEMENTFLAG_PITCH_UP       = 0x00000040,
    MOVEMENTFLAG_PITCH_DOWN     = 0x00000080,
    MOVEMENTFLAG_WALKING        = 0x00000100,               // Walking
    MOVEMENTFLAG_ONTRANSPORT    = 0x00000200,               // Used for flying on some creatures
    /** Unit appears suspended (swimming in the air) instead of falling */
    MOVEMENTFLAG_DISABLE_GRAVITY= 0x00000400,
    MOVEMENTFLAG_ROOT           = 0x00000800,               // Must not be set along with MOVEMENTFLAG_MASK_MOVING
    MOVEMENTFLAG_JUMPING_OR_FALLING    = 0x00001000,        // TC: MOVEMENTFLAG_FALLING // damage dealt on that type of falling
    MOVEMENTFLAG_FALLING_FAR           = 0x00002000,        // Falling under the map boundaries (clientside the camera will remain at the boundaries and let see the character fall below)
    MOVEMENTFLAG_PENDING_STOP          = 0x00004000,
    MOVEMENTFLAG_PENDING_STRAFE_STOP   = 0x00008000,
    MOVEMENTFLAG_PENDING_FORWARD       = 0x00010000,
    MOVEMENTFLAG_PENDING_BACKWARD      = 0x00020000,
    MOVEMENTFLAG_PENDING_STRAFE_LEFT   = 0x00040000,
    MOVEMENTFLAG_PENDING_STRAFE_RIGHT  = 0x00080000,
    MOVEMENTFLAG_PENDING_ROOT          = 0x00100000,
    MOVEMENTFLAG_SWIMMING              = 0x00200000,               // appears with fly flag also
    MOVEMENTFLAG_ASCENDING             = 0x00400000,               // press "space" when flying
    MOVEMENTFLAG_DESCENDING            = 0x00800000,
    /** Player can fly. Seems to work on some degree on creatures. */
    MOVEMENTFLAG_CAN_FLY               = 0x01000000,               
    /** unit is actually flying. pretty sure this is only used for players. creatures use disable_gravity. This will crash the client in various and horrible ways if set on creatures. */
    MOVEMENTFLAG_PLAYER_FLYING         = 0x02000000,               // 
    MOVEMENTFLAG_SPLINE_ELEVATION      = 0x04000000,               // used for flight paths
    MOVEMENTFLAG_SPLINE_ENABLED        = 0x08000000,               // used for flight paths
    MOVEMENTFLAG_WATERWALKING          = 0x10000000,               // prevent unit from falling through water
    MOVEMENTFLAG_FALLING_SLOW          = 0x20000000,            // active rogue safe fall spell (passive)
    MOVEMENTFLAG_HOVER                 = 0x40000000,               // hover, cannot jump
    
    /// @todo Check if PITCH_UP and PITCH_DOWN really belong here..
    MOVEMENTFLAG_MASK_MOVING =
        MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT |
        MOVEMENTFLAG_PITCH_UP | MOVEMENTFLAG_PITCH_DOWN | MOVEMENTFLAG_JUMPING_OR_FALLING | MOVEMENTFLAG_FALLING_FAR | MOVEMENTFLAG_ASCENDING | MOVEMENTFLAG_DESCENDING |
        MOVEMENTFLAG_SPLINE_ELEVATION,

    MOVEMENTFLAG_MASK_TURNING =
        MOVEMENTFLAG_LEFT | MOVEMENTFLAG_RIGHT,

    MOVEMENTFLAG_MASK_MOVING_FLY =
        MOVEMENTFLAG_PLAYER_FLYING | MOVEMENTFLAG_ASCENDING | MOVEMENTFLAG_DESCENDING,

    /// @todo if needed: add more flags to this masks that are exclusive to players
    MOVEMENTFLAG_MASK_PLAYER_ONLY =
        MOVEMENTFLAG_PLAYER_FLYING,

    /// Movement flags that have change status opcodes associated for players
    MOVEMENTFLAG_MASK_HAS_PLAYER_STATUS_OPCODE = MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_ROOT |
        MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_WATERWALKING | MOVEMENTFLAG_FALLING_SLOW | MOVEMENTFLAG_HOVER
};

#define UNIT_DEFAULT_HOVERHEIGHT 2.5f

enum UnitTypeMask
{
    UNIT_MASK_NONE                  = 0x00000000,
    UNIT_MASK_SUMMON                = 0x00000001,
    UNIT_MASK_MINION                = 0x00000002,
    UNIT_MASK_GUARDIAN              = 0x00000004,
    UNIT_MASK_TOTEM                 = 0x00000008,
    UNIT_MASK_PET                   = 0x00000010,
    UNIT_MASK_VEHICLE               = 0x00000020,
    //NYI
    UNIT_MASK_PUPPET                = 0x00000040,
    UNIT_MASK_HUNTER_PET            = 0x00000080,
    UNIT_MASK_CONTROLABLE_GUARDIAN  = 0x00000100,
};

namespace Movement{
    class MoveSpline;
}

enum DiminishingLevels
{
    DIMINISHING_LEVEL_1             = 0,
    DIMINISHING_LEVEL_2             = 1,
    DIMINISHING_LEVEL_3             = 2,
    DIMINISHING_LEVEL_IMMUNE        = 3
};

struct DiminishingReturn
{
    DiminishingReturn(DiminishingGroup group, uint32 t, uint32 count) : DRGroup(group), hitTime(t), hitCount(count), stack(0) {}

    DiminishingGroup        DRGroup:16;
    uint16                  stack:16;
    uint32                  hitTime;
    uint32                  hitCount;
};

enum MeleeHitOutcome
{
    MELEE_HIT_EVADE,
    MELEE_HIT_MISS,
    MELEE_HIT_DODGE,
    MELEE_HIT_BLOCK,
    MELEE_HIT_PARRY,
    MELEE_HIT_GLANCING,
    MELEE_HIT_CRIT,
    MELEE_HIT_CRUSHING,
    MELEE_HIT_NORMAL,
    MELEE_HIT_BLOCK_CRIT
};

struct CleanDamage
{
    CleanDamage(uint32 _damage, WeaponAttackType _attackType, MeleeHitOutcome _hitOutCome) :
    damage(_damage), attackType(_attackType), hitOutCome(_hitOutCome) {}

    uint32 damage;
    WeaponAttackType attackType;
    MeleeHitOutcome hitOutCome;
};

// Struct for use in Unit::CalculateMeleeDamage
// Need create structure like in SMSG_ATTACKERSTATEUPDATE opcode
struct CalcDamageInfo
{
    Unit  *attacker;             // Attacker
    Unit  *target;               // Target for damage
    uint32 damageSchoolMask;
    uint32 damage;
    uint32 absorb;
    uint32 resist;
    uint32 blocked_amount;
    uint32 HitInfo;
    uint32 TargetState;
// Helper
    WeaponAttackType attackType; //
    uint32 procAttacker;
    uint32 procVictim;
    uint32 procEx;
    uint32 cleanDamage;          // Used only fo rage calcultion
    MeleeHitOutcome hitOutCome;  // TODO: remove this field (need use TargetState)
};

// Spell damage info structure based on structure sending in SMSG_SPELLNONMELEEDAMAGELOG opcode
struct SpellNonMeleeDamage{
 SpellNonMeleeDamage(Unit *_attacker, Unit *_target, uint32 _SpellID, uint32 _schoolMask) :
    attacker(_attacker), target(_target), SpellID(_SpellID), damage(0), schoolMask(_schoolMask),
    absorb(0), resist(0), physicalLog(false), unused(false), blocked(0), HitInfo(0), cleanDamage(0) {}
 Unit   *target;
 Unit   *attacker;
 uint32 SpellID;
 uint32 damage;
 uint32 schoolMask;
 uint32 absorb;
 uint32 resist;
 bool   physicalLog;
 bool   unused;
 uint32 blocked;
 uint32 HitInfo;
 // Used for help
 uint32 cleanDamage;
};

struct SpellPeriodicAuraLogInfo
{
    SpellPeriodicAuraLogInfo(Aura const* _aura, uint32 _damage, uint32 _absorb, uint32 _resist, float _multiplier)
        : auraEff(_aura), damage(_damage), absorb(_absorb), resist(_resist), multiplier(_multiplier) { }

    Aura const* auraEff;
    uint32 damage;
    uint32 absorb;
    uint32 resist;
    float  multiplier;
};

uint32 createProcExtendMask(SpellNonMeleeDamage *damageInfo, SpellMissInfo missCondition);

struct UnitActionBarEntry
{
    uint32 Type;
    uint32 SpellOrAction;
};

#define MAX_DECLINED_NAME_CASES 5

struct DeclinedName
{
    std::string name[MAX_DECLINED_NAME_CASES];
};

enum CurrentSpellTypes
{
    CURRENT_MELEE_SPELL = 0,
    CURRENT_FIRST_NON_MELEE_SPELL = 1,                      // just counter
    CURRENT_GENERIC_SPELL = 1,
    CURRENT_AUTOREPEAT_SPELL = 2,
    CURRENT_CHANNELED_SPELL = 3,
    CURRENT_MAX_SPELL = 4                                   // just counter
};

struct GlobalCooldown
{
    explicit GlobalCooldown(uint32 _dur = 0, uint32 _time = 0) : duration(_dur), cast_time(_time) {}

    uint32 duration;
    uint32 cast_time;
};

typedef std::unordered_map<uint32 /*category*/, GlobalCooldown> GlobalCooldownList;

class GlobalCooldownMgr                                     // Shared by Player and CharmInfo
{
    public:
        GlobalCooldownMgr() {}

    public:
        bool HasGlobalCooldown(SpellInfo const* spellInfo) const;
        void AddGlobalCooldown(SpellInfo const* spellInfo, uint32 gcd);
        void CancelGlobalCooldown(SpellInfo const* spellInfo);

    private:
        GlobalCooldownList m_GlobalCooldowns;
};

enum ActiveStates
{
    ACT_ENABLED  = 0xC100,
    ACT_DISABLED = 0x8100,
    ACT_COMMAND  = 0x0700,
    ACT_REACTION = 0x0600,
    ACT_CAST     = 0x0100,
    ACT_PASSIVE  = 0x0000,
    ACT_DECIDE   = 0x0001
};

enum ReactStates
{
    REACT_PASSIVE    = 0,
    REACT_DEFENSIVE  = 1,
    REACT_AGGRESSIVE = 2
};

enum CommandStates
{
    COMMAND_STAY    = 0,
    COMMAND_FOLLOW  = 1,
    COMMAND_ATTACK  = 2,
    COMMAND_ABANDON = 3
};

struct CharmSpellEntry
{
    uint16 spellId;
    uint16 active;
};

enum Rotation
{
    CREATURE_ROTATE_NONE = 0,
    CREATURE_ROTATE_LEFT = 1,
    CREATURE_ROTATE_RIGHT = 2
};

enum SplineType
{
    SPLINETYPE_NORMAL        = 0,
    SPLINETYPE_STOP          = 1,
    SPLINETYPE_FACING_SPOT   = 2,
    SPLINETYPE_FACING_TARGET = 3,
    SPLINETYPE_FACING_ANGLE  = 4
};

typedef std::list<uint64> SharedVisionList;

enum CharmType
{
    CHARM_TYPE_CHARM,
    CHARM_TYPE_POSSESS,
    CHARM_TYPE_VEHICLE,
    CHARM_TYPE_CONVERT
};

struct CharmInfo
{
    public:
        explicit CharmInfo(Unit* unit);
        ~CharmInfo();
        uint32 GetPetNumber() const { return m_petnumber; }
        void SetPetNumber(uint32 petnumber, bool statwindow);

        void SetCommandState(CommandStates st) { m_CommandState = st; }
        CommandStates GetCommandState() { return m_CommandState; }
        bool HasCommandState(CommandStates state) { return (m_CommandState == state); }
        //void SetReactState(ReactStates st) { m_reactState = st; }
        //ReactStates GetReactState() { return m_reactState; }
        //bool HasReactState(ReactStates state) { return (m_reactState == state); }

        void InitPossessCreateSpells();
        void InitCharmCreateSpells();
        void InitPetActionBar();
        void InitEmptyActionBar(bool withAttack = true);
                                                            //return true if successful
        bool AddSpellToAB(uint32 oldid, uint32 newid, uint8 index, ActiveStates newstate = ACT_DECIDE);
        void ToggleCreatureAutocast(uint32 spellid, bool apply);

        UnitActionBarEntry* GetActionBarEntry(uint8 index) { return &(PetActionBar[index]); }
        CharmSpellEntry* GetCharmSpell(uint8 index) { return &(m_charmspells[index]); }
        
        GlobalCooldownMgr& GetGlobalCooldownMgr() { return m_GlobalCooldownMgr; }

    private:
        Unit* m_unit;
        UnitActionBarEntry PetActionBar[10];
        CharmSpellEntry m_charmspells[CREATURE_MAX_SPELLS];
        CommandStates   m_CommandState;
        //ReactStates     m_reactState;
        uint32          m_petnumber;
        bool            m_barInit;

        //for restoration after charmed
        ReactStates     m_oldReactState;

        GlobalCooldownMgr m_GlobalCooldownMgr;
};

// for clearing special attacks
#define REACTIVE_TIMER_START 4000

enum ReactiveType
{
    REACTIVE_DEFENSE      = 1,
    REACTIVE_HUNTER_PARRY = 2,
    REACTIVE_CRIT         = 3,
    REACTIVE_HUNTER_CRIT  = 4,
    REACTIVE_OVERPOWER    = 5,
    MAX_REACTIVE
};

#define MAX_TOTEM 4

// delay time next attack to prevent client attack animation problems
#define ATTACK_DISPLAY_DELAY 200

#define MAX_PLAYER_STEALTH_DETECT_RANGE 45.0f               // max distance for detection targets by player
/* distance in yards at which a creature will be in "warning" mode, look in direction of the player without attacking
Not sure about this but: Creatures should only warn players, and bosses shouldn't warn at all. 
Only on units hostile to players and able to attack him.
*/
#define STEALTH_DETECT_WARNING_RANGE 3.0f   
//if in warning range, we can do the suspicious look.
//time in ms between two warning, counting from warning start (= ignoring duration)
#define SUSPICIOUS_LOOK_COOLDOWN 16000
#define SUSPICIOUS_LOOK_DURATION 5000
// SUSPICIOUS_LOOK_COOLDOWN must always be greater than SUSPICIOUS_LOOK_DURATION

enum StealthDetectedStatus
{
    DETECTED_STATUS_NOT_DETECTED,
    DETECTED_STATUS_WARNING, //almost detected
    DETECTED_STATUS_DETECTED,    
};

enum CanAttackResult
{
    CAN_ATTACK_RESULT_OK,
    CAN_ATTACK_RESULT_DEAD, //target is dead (really dead, not feign death)
    CAN_ATTACK_RESULT_FEIGN_DEATH,
    CAN_ATTACK_RESULT_CIVILIAN, //create is civilian
    CAN_ATTACK_RESULT_NOT_ACCESSIBLE, //isInAccessiblePlaceFor
    CAN_ATTACK_RESULT_TOO_FAR_Z, //target is higher than CREATURE_Z_ATTACK_RANGE
    CAN_ATTACK_RESULT_TOO_FAR, //target is further than attack distance
    CAN_ATTACK_RESULT_NOT_IN_LOS, //not in Line of Sight
    CAN_ATTACK_RESULT_FRIENDLY, //target is friendly
    CAN_ATTACK_RESULT_TARGET_FLAGS, //could not attack because of target flags
    CAN_ATTACK_RESULT_CANNOT_DETECT_INVI, //target cannot be detected because it's invisible to us
    CAN_ATTACK_RESULT_CANNOT_DETECT_STEALTH, //target cannot be detected because it's stealthed from us
    CAN_ATTACK_RESULT_CANNOT_DETECT_STEALTH_WARN_RANGE, //target cannot be detected because it's stealthed from us but is in warn range
    CAN_ATTACK_RESULT_SELF_EVADE, //creature is currently evading
    CAN_ATTACK_RESULT_TARGET_EVADE, //target is a creature in evade mode
    CAN_ATTACK_RESULT_SELF_UNIT_FLAGS, //create cannot attack because of own unit flags
    CAN_ATTACK_RESULT_IMMUNE_TO_TARGET, //Combat disabled with target because of own flags
    CAN_ATTACK_RESULT_OTHERS, //all others reason
};

struct SpellProcEventEntry;                                 // used only privately

class Unit : public WorldObject
{
    public:
        typedef std::set<Unit*> AttackerSet;
        typedef std::pair<uint32, uint8> spellEffectPair;
        typedef std::multimap< spellEffectPair, Aura*> AuraMap;
        typedef std::list<Aura *> AuraList;
        //compat TC
        typedef AuraList AuraEffectList;

        typedef std::multimap<AuraStateType, Aura*> AuraStateAurasMap;
        typedef std::pair<AuraStateAurasMap::const_iterator, AuraStateAurasMap::const_iterator> AuraStateAurasMapBounds;

        typedef std::list<DiminishingReturn> Diminishing;
        typedef std::set<AuraType> AuraTypeSet;
        typedef std::set<uint32> ComboPointHolderSet;

        virtual ~Unit ( );

        void AddToWorld();
        void RemoveFromWorld();

        virtual void CleanupsBeforeDelete(bool finalCleanup = true) override;  // used in ~Creature/~Player (or before mass creature delete to remove cross-references to already deleted units)

        UnitAI* GetAI() { return i_AI; }

        DiminishingLevels GetDiminishing(DiminishingGroup  group);
        void IncrDiminishing(DiminishingGroup group);
        void ApplyDiminishingToDuration(DiminishingGroup  group, int32 &duration,Unit* caster, DiminishingLevels Level);
        void ApplyDiminishingAura(DiminishingGroup  group, bool apply);
        void ClearDiminishings() { m_Diminishing.clear(); }

        virtual void Update( uint32 time );

        void SetAttackTimer(WeaponAttackType type, uint32 time) { m_attackTimer[type] = time; }
        void ResetAttackTimer(WeaponAttackType type = BASE_ATTACK);
        uint32 GetAttackTimer(WeaponAttackType type) const { return m_attackTimer[type]; }
        bool IsAttackReady(WeaponAttackType type = BASE_ATTACK) const { return m_attackTimer[type] == 0; }
        bool HaveOffhandWeapon() const;
        bool CanDualWield() const { return m_canDualWield; }
        void SetCanDualWield(bool value) { m_canDualWield = value; }
        float GetCombatReach() const { return m_floatValues[UNIT_FIELD_COMBATREACH]; }
        float GetMeleeReach() const { float reach = m_floatValues[UNIT_FIELD_COMBATREACH]; return reach > MIN_MELEE_REACH ? reach : MIN_MELEE_REACH; }
        bool IsWithinCombatRange(Unit *obj, float dist2compare) const;
        bool IsWithinMeleeRange(Unit *obj, float dist = MELEE_RANGE) const;
        void GetRandomContactPoint( const Unit* target, float &x, float &y, float &z, float distance2dMin, float distance2dMax ) const;
        uint32 m_extraAttacks;
        bool m_canDualWield;
        
        void StartAutoRotate(uint8 type, uint32 fulltime, double Angle = 0, bool attackVictimOnEnd = true);
        void AutoRotate(uint32 time);
        bool IsUnitRotating() {return IsRotating;}

        void _addAttacker(Unit *pAttacker)                  // must be called only from Unit::Attack(Unit*)
        {
            AttackerSet::iterator itr = m_attackers.find(pAttacker);
            if(itr == m_attackers.end())
                m_attackers.insert(pAttacker);
        }
        void _removeAttacker(Unit *pAttacker)               // must be called only from Unit::AttackStop()
        {
            AttackerSet::iterator itr = m_attackers.find(pAttacker);
            if(itr != m_attackers.end())
                m_attackers.erase(itr);
        }
        Unit* GetAttackerForHelper()                       // If someone wants to help, who to give them
        {
            if (GetVictim() != NULL)
                return GetVictim();

            if (!m_attackers.empty())
                return *(m_attackers.begin());

            return NULL;
        }
        bool Attack(Unit *victim, bool meleeAttack);
        void CastStop(uint32 except_spellid = 0);
        bool AttackStop();
        void RemoveAllAttackers();
        AttackerSet const& GetAttackers() const { return m_attackers; }
        bool IsAttackingPlayer() const;
        Unit* GetVictim() const { return m_attacking; }
        Unit* EnsureVictim() const
        {
            ASSERT(m_attacking);
            return m_attacking;
        }
        
        void CombatStop(bool cast = false);
        void CombatStopWithPets(bool cast = false);
        Unit* SelectNearbyTarget(float dist = NOMINAL_MELEE_RANGE) const;

        void AddUnitState(uint32 f) { m_state |= f; }
        bool HasUnitState(const uint32 f) const { return (m_state & f); }
        void ClearUnitState(uint32 f) { m_state &= ~f; }
        bool CanFreeMove() const;

        uint32 HasUnitTypeMask(uint32 mask) const { return mask & m_unitTypeMask; }
        void AddUnitTypeMask(uint32 mask) { m_unitTypeMask |= mask; }
        bool IsSummon() const { return (m_unitTypeMask & UNIT_MASK_SUMMON) != 0; }
        bool IsGuardian() const { return (m_unitTypeMask & UNIT_MASK_GUARDIAN) != 0; }
        bool IsPet() const { return (m_unitTypeMask & UNIT_MASK_PET) != 0; }
        bool IsHunterPet() const { return (m_unitTypeMask & UNIT_MASK_HUNTER_PET) != 0; }
        bool IsTotem() const { return (m_unitTypeMask & UNIT_MASK_TOTEM) != 0; }
        bool IsVehicle() const { return (m_unitTypeMask & UNIT_MASK_VEHICLE) != 0; }

        Pet* SummonPet(uint32 entry, float x, float y, float z, float ang, uint32 despwtime);

        uint32 GetLevel() const { return GetUInt32Value(UNIT_FIELD_LEVEL); }
        virtual uint32 GetLevelForTarget(Unit const* /*target*/) const { return GetLevel(); }
        void SetLevel(uint32 lvl);
        uint8 GetRace() const { return GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_RACE); }
        uint32 GetRaceMask() const { return 1 << (GetRace()-1); }
        uint8 GetClass() const { return GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_CLASS); }
        uint32 GetClassMask() const { return 1 << (GetClass()-1); }
        uint8 GetGender() const { return GetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER); }

        float GetStat(Stats stat) const { return float(GetUInt32Value(UNIT_FIELD_STAT0+stat)); }
        void SetStat(Stats stat, int32 val) { SetStatInt32Value(UNIT_FIELD_STAT0+stat, val); }
        uint32 GetArmor() const { return GetResistance(SPELL_SCHOOL_NORMAL) ; }
        void SetArmor(int32 val) { SetResistance(SPELL_SCHOOL_NORMAL, val); }

        uint32 GetResistance(SpellSchools school) const { return GetUInt32Value(UNIT_FIELD_RESISTANCES+school); }
        void SetResistance(SpellSchools school, int32 val) { SetStatInt32Value(UNIT_FIELD_RESISTANCES+school,val); }

        uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
        uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAXHEALTH); }

        bool IsFullHealth() const { return GetHealth() == GetMaxHealth(); }
        bool HealthBelowPct(int32 pct) const { return GetHealth() < CountPctFromMaxHealth(pct); }
        bool HealthBelowPctDamaged(int32 pct, uint32 damage) const { return int64(GetHealth()) - int64(damage) < int64(CountPctFromMaxHealth(pct)); }
        bool HealthAbovePct(int32 pct) const { return GetHealth() > CountPctFromMaxHealth(pct); }
        bool HealthAbovePctHealed(int32 pct, uint32 heal) const { return uint64(GetHealth()) + uint64(heal) > CountPctFromMaxHealth(pct); }
        float GetHealthPct() const { return GetMaxHealth() ? 100.f * GetHealth() / GetMaxHealth() : 0.0f; }
		//get X% heath from max value
        uint32 CountPctFromMaxHealth(int32 pct) const { return CalculatePct(GetMaxHealth(), pct); }
        uint32 CountPctFromCurHealth(int32 pct) const { return CalculatePct(GetHealth(), pct); }

        void SetHealth(   uint32 val);
        void SetMaxHealth(uint32 val);
        inline void SetFullHealth() { SetHealth(GetMaxHealth()); }
        int32 ModifyHealth(int32 val);

        Powers GetPowerType() const { return Powers(GetByteValue(UNIT_FIELD_BYTES_0, 3)); }
        void SetPowerType(Powers power);
        uint32 GetPower(   Powers power) const { return GetUInt32Value(UNIT_FIELD_POWER1   +power); }
        uint32 GetMaxPower(Powers power) const { return GetUInt32Value(UNIT_FIELD_MAXPOWER1+power); }
        void SetPower(   Powers power, uint32 val);
        void SetMaxPower(Powers power, uint32 val);
        int32 ModifyPower(Powers power, int32 val);
        int32 ModifyPowerPct(Powers power, float pct, bool apply = true);
        void ApplyPowerMod(Powers power, uint32 val, bool apply);
        void ApplyMaxPowerMod(Powers power, uint32 val, bool apply);

        uint32 GetAttackTime(WeaponAttackType att) const { return (uint32)(GetFloatValue(UNIT_FIELD_BASEATTACKTIME+att)/m_modAttackSpeedPct[att]); }
        void SetAttackTime(WeaponAttackType att, uint32 val) { SetFloatValue(UNIT_FIELD_BASEATTACKTIME+att,val*m_modAttackSpeedPct[att]); }
        void ApplyAttackTimePercentMod(WeaponAttackType att,float val, bool apply);
        void ApplyCastTimePercentMod(float val, bool apply);
        void HandleParryRush();
        
        SheathState GetSheath() const { return SheathState(GetByteValue(UNIT_FIELD_BYTES_2, 0)); }
        virtual void SetSheath( SheathState sheathed ) { SetByteValue(UNIT_FIELD_BYTES_2, 0, sheathed); }

        // faction template id
        uint32 GetFaction() const { return GetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE); }
        void SetFaction(uint32 faction) { SetUInt32Value(UNIT_FIELD_FACTIONTEMPLATE, faction ); }
        FactionTemplateEntry const* GetFactionTemplateEntry() const;

        //Check if unit is "red" to this unit (not yellow or green)
        bool IsHostileTo(Unit const* unit) const;
        bool IsHostileToPlayers() const;
        //check if unit is "green" to this unit, and can't be attacker. You should use this instead of IsHostileTo for player to check if they can target a given unit, since they can target either red or yellow targets.
        bool IsFriendlyTo(Unit const* unit) const;
        bool IsNeutralToAll() const;

        ReputationRank GetReactionTo(Unit const* target) const;
        ReputationRank GetFactionReactionTo(FactionTemplateEntry const* factionTemplateEntry, Unit const* target) const;

        bool IsInPartyWith(Unit const* unit) const;
        bool IsInRaidWith(Unit const* unit) const;
        void GetPartyMember(std::list<Unit*> &units, float dist);
        void GetRaidMember(std::list<Unit*> &units, float dist);
        bool IsContestedGuard() const
        {
            if(FactionTemplateEntry const* entry = GetFactionTemplateEntry())
                return entry->IsContestedGuardFaction();

            return false;
        }
#ifdef LICH_KING
        virtual bool IsInSanctuary() const { return HasByteFlag(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_UNK3); }
#else
        virtual bool IsInSanctuary() const;
#endif
        bool IsPvP() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); }
        bool IsFFAPvP() const;
        void SetPvP(bool state) { if(state) SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); else RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); }
        uint32 GetCreatureType() const;
        uint32 GetCreatureTypeMask() const
        {
            uint32 creatureType = GetCreatureType();
            return (creatureType >= 1) ? (1 << (creatureType - 1)) : 0;
        }

        uint8 GetStandState() const { return GetByteValue(UNIT_FIELD_BYTES_1, 0); }
        bool IsSitState() const;
        bool IsStandState() const;
        void SetStandState(uint8 state);

        bool IsMounted() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT ); }
        uint32 GetMountID() const { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }
        void Mount(uint32 mount, bool flying = false);
        void Dismount();

        uint16 GetMaxSkillValueForLevel(Unit const* target = NULL) const { return (target ? GetLevelForTarget(target) : GetLevel()) * 5; }
        void RemoveSpellbyDamageTaken(uint32 damage, uint32 spell);
        uint32 DealDamage(Unit *pVictim, uint32 damage, CleanDamage const* cleanDamage = NULL, DamageEffectType damagetype = DIRECT_DAMAGE, SpellSchoolMask damageSchoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellInfo const *spellProto = NULL, bool durabilityLoss = true);
        void Kill(Unit *pVictim, bool durabilityLoss = true);
        void KillSelf(bool durabilityLoss = true) { Kill(this, durabilityLoss); }

        void ProcDamageAndSpell(Unit *pVictim, uint32 procAttacker, uint32 procVictim, uint32 procEx, uint32 amount, WeaponAttackType attType = BASE_ATTACK, SpellInfo const *procSpell = NULL, bool canTrigger = true);
        void ProcDamageAndSpellFor( bool isVictim, Unit * pTarget, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, SpellInfo const * procSpell, uint32 damage );

        void SetEmoteState(uint32 state);
        void HandleEmoteCommand(uint32 anim_id);
        void AttackerStateUpdate (Unit *pVictim, WeaponAttackType attType = BASE_ATTACK, bool extra = false );
        //float MeleeMissChanceCalc(const Unit *pVictim, WeaponAttackType attType) const;

        void CalculateMeleeDamage(Unit *pVictim, uint32 damage, CalcDamageInfo *damageInfo, WeaponAttackType attackType = BASE_ATTACK);
        void DealMeleeDamage(CalcDamageInfo *damageInfo, bool durabilityLoss);

        void CalculateSpellDamageTaken(SpellNonMeleeDamage *damageInfo, int32 damage, SpellInfo const *spellInfo, WeaponAttackType attackType = BASE_ATTACK, bool crit = false);
        void DealSpellDamage(SpellNonMeleeDamage *damageInfo, bool durabilityLoss);

        float MeleeSpellMissChance(const Unit *pVictim, WeaponAttackType attType, int32 skillDiff, uint32 spellId) const;
        SpellMissInfo MeleeSpellHitResult(Unit *pVictim, SpellInfo const *spell);
        SpellMissInfo MagicSpellHitResult(Unit *pVictim, SpellInfo const *spell, Item* castItem = nullptr);
        SpellMissInfo SpellHitResult(Unit *pVictim, SpellInfo const *spell, bool canReflect = false, Item* castItem = nullptr);
        float GetAverageSpellResistance(Unit* caster, SpellSchoolMask damageSchoolMask);

        float GetUnitDodgeChance()    const;
        float GetUnitParryChance()    const;
        float GetUnitBlockChance()    const;
        float GetUnitCriticalChance(WeaponAttackType attackType, const Unit *pVictim) const;
        int32 GetMechanicResistChance(const SpellInfo *spell);
        bool CanUseAttackType(uint8 attacktype) const;

        virtual uint32 GetShieldBlockValue() const =0;
        uint32 GetUnitMeleeSkill(Unit const* target = NULL) const { return (target ? GetLevelForTarget(target) : GetLevel()) * 5; }
        uint32 GetDefenseSkillValue(Unit const* target = NULL) const;
        uint32 GetWeaponSkillValue(WeaponAttackType attType, Unit const* target = NULL) const;
        float GetWeaponProcChance() const;
        float GetPPMProcChance(uint32 WeaponSpeed, float PPM) const;

        MeleeHitOutcome RollMeleeOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType, SpellSchoolMask schoolMask) const;
        MeleeHitOutcome RollMeleeOutcomeAgainst (const Unit *pVictim, WeaponAttackType attType, int32 crit_chance, int32 miss_chance, int32 dodge_chance, int32 parry_chance, int32 block_chance, bool SpellCasted ) const;

        bool IsVendor()       const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR ); }
        bool IsTrainer()      const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER ); }
        bool IsQuestGiver()   const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER ); }
        bool IsGossip()       const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP ); }
        bool IsTaxi()         const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_FLIGHTMASTER ); }
        bool IsGuildMaster()  const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PETITIONER ); }
        bool IsBattleMaster() const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BATTLEMASTER ); }
        bool IsBanker()       const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER ); }
        bool IsInnkeeper()    const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER ); }
        bool IsSpiritHealer() const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER ); }
        bool IsSpiritGuide()  const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITGUIDE ); }
        bool IsTabardDesigner()const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TABARDDESIGNER ); }
        bool IsAuctioner()    const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_AUCTIONEER ); }
        bool IsArmorer()      const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_REPAIR ); }
        bool IsServiceProvider() const
        {
            return HasFlag( UNIT_NPC_FLAGS,
                UNIT_NPC_FLAG_VENDOR | UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_FLIGHTMASTER |
                UNIT_NPC_FLAG_PETITIONER | UNIT_NPC_FLAG_BATTLEMASTER | UNIT_NPC_FLAG_BANKER |
                UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_SPIRITHEALER |
                UNIT_NPC_FLAG_SPIRITGUIDE | UNIT_NPC_FLAG_TABARDDESIGNER | UNIT_NPC_FLAG_AUCTIONEER );
        }
        bool IsSpiritService() const { return HasFlag( UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_SPIRITGUIDE ); }
        bool IsCritter() const { return GetCreatureType() == CREATURE_TYPE_CRITTER; }

        virtual bool IsGuard() const { return false; }

        bool IsInFlight()  const { return HasUnitState(UNIT_STATE_IN_FLIGHT); }

        bool IsInCombat()  const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); }
        bool IsPetInCombat() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_IN_COMBAT); }
        void CombatStart(Unit* target, bool updatePvP = true);
        void SetInCombatState(bool PvP, Unit* enemy = nullptr);
        void SetInCombatWith(Unit* enemy);
        bool IsInCombatWith(Unit const* enemy) const;
        void ClearInCombat();
        uint32 GetCombatTimer() const { return m_CombatTimer; }

        //TC compat
        bool HasAura(uint32 spellId) const {
            return GetAuraCount(spellId) > 0;
        }
        uint32 GetAuraCount(uint32 spellId) const;
        bool HasAuraType(AuraType auraType) const;
        bool HasAuraTypeWithFamilyFlags(AuraType auraType, uint32 familyName,  uint64 familyFlags) const;
        bool HasAuraTypeWithCaster(AuraType auraType, uint64 caster) const;
        bool HasAuraEffect(uint32 spellId, uint32 effIndex = 0) const
            { return m_Auras.find(spellEffectPair(spellId, effIndex)) != m_Auras.end(); }
        bool HasAuraWithMechanic(Mechanics mechanic) const;
        bool HasAuraWithCaster(uint32 spellId, uint32 effIndex, uint64 owner) const;
        bool HasAuraWithCasterNot(uint32 spellId, uint32 effIndex, uint64 owner) const;

        bool virtual HasSpell(uint32 /*spellID*/) const { return false; }
        bool HasBreakableByDamageAuraType(AuraType type, uint32 excludeAura = 0) const;
        bool HasBreakableByDamageCrowdControlAura(Unit* excludeCasterChannel = NULL) const;

        bool HasStealthAura()      const { return HasAuraType(SPELL_AURA_MOD_STEALTH); }
        bool HasInvisibilityAura() const { return HasAuraType(SPELL_AURA_MOD_INVISIBILITY); }
        bool IsFeared()  const { return HasAuraType(SPELL_AURA_MOD_FEAR); }
        bool IsInRoots() const { return HasAuraType(SPELL_AURA_MOD_ROOT); }
        bool IsPolymorphed() const;

        bool IsFrozen() const;
        
        /** Check the following :
            Is Alive
            Has not flag UNIT_FLAG_NON_ATTACKABLE or UNIT_FLAG_NOT_SELECTABLE or UNIT_FLAG_IMMUNE_TO_PC
            Is not gamemaster or spectator or gm not visible
            Is not totem or in evade mode
            Is not in flight
        */
        bool IsAttackableByAOE() const;
        CanAttackResult CanAttack(Unit const* target, bool force = true) const;

/* TC
        bool isTargetableForAttack(bool checkFakeDeath = true, Unit const* byWho = NULL) const;

        */
        bool IsValidAttackTarget(Unit const* target) const;
        bool _IsValidAttackTarget(Unit const* target, SpellInfo const* bySpell, WorldObject const* obj = NULL) const;

        bool IsValidAssistTarget(Unit const* target) const;
        bool _IsValidAssistTarget(Unit const* target, SpellInfo const* bySpell) const;


        virtual bool IsInWater() const;
        virtual bool IsUnderWater() const;
        virtual void UpdateUnderwaterState(Map* m, float x, float y, float z);
        bool isInAccessiblePlaceFor(Creature const* c) const;
        
        void SetFullTauntImmunity(bool apply);

        void SendHealSpellLog(Unit *pVictim, uint32 SpellID, uint32 Damage, bool critical = false);
        void SendEnergizeSpellLog(Unit *pVictim, uint32 SpellID, uint32 Damage,Powers powertype);
        uint32 SpellNonMeleeDamageLog(Unit *pVictim, uint32 spellID, uint32 damage, bool IsTriggeredSpell = false, bool useSpellDamage = true);
        //returns SpellCastResult
        uint32 CastSpell(Unit* Victim, uint32 spellId, bool triggered = false, Item* castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0 );
        uint32 CastSpell(Unit* Victim,SpellInfo const* spellInfo, bool triggered, Item* castItem= NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0, bool skipHit = false);
        uint32 CastCustomSpell(Unit* Victim, uint32 spellId, int32 const* bp0, int32 const* bp1, int32 const* bp2, bool triggered, Item *castItem= NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        uint32 CastCustomSpell(uint32 spellId, SpellValueMod mod, uint32 value, Unit* Victim = NULL, bool triggered = true, Item *castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        uint32 CastCustomSpell(uint32 spellId, CustomSpellValues const &value, Unit* Victim = NULL, bool triggered = true, Item *castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        uint32 CastSpell(SpellCastTargets const& targets, SpellInfo const* spellInfo, CustomSpellValues const* value, bool triggered = true, Item* castItem = nullptr, Aura* triggeredByAura = nullptr, uint64 originalCaster = 0);
        uint32 CastSpell(float x, float y, float z, uint32 spellId, bool triggered, Item *castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        uint32 CastSpell(GameObject *go, uint32 spellId, bool triggered, Item *castItem = NULL, Aura* triggeredByAura = NULL, uint64 originalCaster = 0);
        void AddAura(uint32 spellId, Unit *target);

        void LogBossDown(Creature* victim);

        bool IsDamageToThreatSpell(SpellInfo const * spellInfo) const;

        void DeMorph();
        void RestoreDisplayId();

        void SendMeleeAttackStart(Unit* pVictim);
        void SendMeleeAttackStop(Unit* victim = nullptr);
        void SendAttackStateUpdate(CalcDamageInfo *damageInfo);
        void SendAttackStateUpdate(uint32 HitInfo, Unit *target, uint8 SwingType, SpellSchoolMask damageSchoolMask, uint32 Damage, uint32 AbsorbDamage, uint32 Resist, VictimState TargetState, uint32 BlockedAmount);
        void SendSpellNonMeleeDamageLog(SpellNonMeleeDamage *log);
        void SendSpellNonMeleeDamageLog(Unit *target,uint32 SpellID,uint32 Damage, SpellSchoolMask damageSchoolMask,uint32 AbsorbedDamage, uint32 Resist,bool PhysicalDamage, uint32 Blocked, bool CriticalHit = false);
        void SendSpellMiss(Unit *target, uint32 spellID, SpellMissInfo missInfo);
        void SendPeriodicAuraLog(SpellPeriodicAuraLogInfo* pInfo);
        void SendSpellDamageResist(Unit* target, uint32 spellId, bool debug = false);
        void SendSpellDamageImmune(Unit* target, uint32 spellId);

        /* (used mainly for blink spell) */
        Position GetLeapPosition(float dist);
        void NearTeleportTo(float x, float y, float z, float orientation, bool casting = false);
        void SendTeleportPacket(Position& pos);
        virtual bool UpdatePosition(float x, float y, float z, float ang, bool teleport = false);
        // returns true if unit's position really changed
        bool UpdatePosition(const Position &pos, bool teleport = false);
        void UpdateOrientation(float orientation);
        void UpdateHeight(float newZ);

        void UpdateSplineMovement(uint32 t_diff);
        void UpdateSplinePosition();
        void DisableSpline();

        void KnockbackFrom(float x, float y, float speedXY, float speedZ);
        void JumpTo(float speedXY, float speedZ, bool forward = true);
        void JumpTo(WorldObject* obj, float speedZ);

        void MonsterMoveWithSpeed(float x, float y, float z, float speed, bool generatePath = false, bool forceDestination = false);
        void SendMovementFlagUpdate();
        void SendMovementFlagUpdate(float dist);

        bool IsAlive() const { return (m_deathState == ALIVE); };
        bool IsDying() const { return (m_deathState == JUST_DIED); };
        bool IsDead() const { return ( m_deathState == DEAD || m_deathState == CORPSE ); };
        DeathState GetDeathState() { return m_deathState; };
        virtual void SetDeathState(DeathState s);           // overwrited in Creature/Player/Pet

        uint64 GetOwnerGUID() const { return  GetUInt64Value(UNIT_FIELD_SUMMONEDBY); }
        void SetOwnerGUID(uint64 owner);
        uint64 GetCreatorGUID() const { return GetUInt64Value(UNIT_FIELD_CREATEDBY); }
        /** Set a "X's minion" text on the creature */
        void SetCreatorGUID(uint64 creator) { SetUInt64Value(UNIT_FIELD_CREATEDBY, creator); }
        uint64 GetMinionGUID() const { return GetUInt64Value(UNIT_FIELD_SUMMON); }
        uint64 GetCharmerGUID() const { return GetUInt64Value(UNIT_FIELD_CHARMEDBY); }
        void SetCharmerGUID(uint64 owner) { SetUInt64Value(UNIT_FIELD_CHARMEDBY, owner); }
        uint64 GetCharmGUID() const { return  GetUInt64Value(UNIT_FIELD_CHARM); }
        //not tc like yet
        void SetPetGUID(uint64 guid) { SetUInt64Value(UNIT_FIELD_SUMMON, guid); }
        //not tc like yet
        uint64 GetPetGUID() const { return GetUInt64Value(UNIT_FIELD_SUMMON); }

        bool IsControlledByPlayer() const { return m_ControlledByPlayer; }
        bool IsCreatedByPlayer() const { return m_CreatedByPlayer; }

        uint64 GetCharmerOrOwnerGUID() const { return GetCharmerGUID() ? GetCharmerGUID() : GetOwnerGUID(); }
        uint64 GetCharmerOrOwnerOrOwnGUID() const
        {
            if(uint64 guid = GetCharmerOrOwnerGUID())
                return guid;
            return GetGUID();
        }
        bool isCharmedOwnedByPlayerOrPlayer() const;

        Player* GetSpellModOwner() const;

        Unit* GetOwner() const;
        Pet* GetPet() const;
        Unit* GetGuardianPet() const;
        Unit* GetCharmer() const;
        Unit* GetCharm() const;
        Unit* GetCharmerOrOwner() const { return GetCharmerGUID() ? GetCharmer() : GetOwner(); }
        Unit* GetCharmerOrOwnerOrSelf() const
        {
            if(Unit *u = GetCharmerOrOwner())
                return u;

            return (Unit*)this;
        }
        Player* GetCharmerOrOwnerPlayerOrPlayerItself() const;
        Player* GetAffectingPlayer() const;

        void SetPet(Pet* pet);
        void SetCharm(Unit* pet);
        void SetCharmedBy(Unit* charmer, bool possess);
        void RemoveCharmedBy(Unit* charmer);
        void RestoreFaction();

        Unit* GetFirstControlled() const;

        bool IsCharmed() const { return GetCharmerGUID() != 0; }
        bool IsPossessed() const { return HasUnitState(UNIT_STATE_POSSESSED); }
        bool IsPossessedByPlayer() const;
        bool IsPossessing() const;
        bool IsPossessing(Unit* u) const;

        CharmInfo* GetCharmInfo() { return m_charmInfo; }
        CharmInfo* InitCharmInfo();
        void       DeleteCharmInfo();
        void UpdateCharmAI();
        //Renamed from TC: m_movedPlayer;
        Player* m_movedByPlayer;
        SharedVisionList const& GetSharedVisionList() { return m_sharedVision; }
        void AddPlayerToVision(Player* plr);
        void RemovePlayerFromVision(Player* plr);
        bool HasSharedVision() const { return !m_sharedVision.empty(); }
        void RemoveBindSightAuras();
        void RemoveCharmAuras();

        Pet* CreateTamedPetFrom(Creature* creatureTarget,uint32 spell_id = 0);

        bool AddAura(Aura *aur);

        void RemoveAura(AuraMap::iterator &i, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(uint32 spellId, uint32 effindex, Aura* except = NULL);
        void RemoveSingleAuraFromStackByDispel(uint32 spellId);
        void RemoveSingleAuraFromStack(uint32 spellId, uint32 effindex);
        void RemoveAurasDueToSpell(uint32 spellId, Aura* except = NULL);
        void RemoveAurasDueToItemSpell(Item* castItem,uint32 spellId);
        void RemoveAurasByCasterSpell(uint32 spellId, uint32 effIndex, uint64 casterGUID);
        void RemoveAurasByCasterSpell(uint32 spellId, uint64 casterGUID);
        void SetAurasDurationByCasterSpell(uint32 spellId, uint64 casterGUID, int32 duration);
        Aura* GetAuraByCasterSpell(uint32 spellId, uint64 casterGUID);
        Aura* GetAuraByCasterSpell(uint32 spellId, uint32 effIndex, uint64 casterGUID);
        void RemoveAurasDueToSpellByDispel(uint32 spellId, uint64 casterGUID, Unit *dispeler);
        void RemoveAurasDueToSpellBySteal(uint32 spellId, uint64 casterGUID, Unit *stealer);
        void RemoveAurasDueToSpellByCancel(uint32 spellId);
        void RemoveAurasAtChanneledTarget(SpellInfo const* spellInfo, Unit * caster);
        void RemoveNotOwnSingleTargetAuras();

        void RemoveAurasByType(AuraType auraType);
        void RemoveAuraTypeByCaster(AuraType auraType, uint64 casterGUID);
        void RemoveRankAurasDueToSpell(uint32 spellId);
        bool RemoveNoStackAurasDueToAura(Aura *Aur);
        void RemoveAurasWithInterruptFlags(uint32 flags, uint32 except = 0, bool withChanneled = false);
        //Attr0 only
        void RemoveAurasWithAttribute(uint32 flags);
        void RemoveAurasWithCustomAttribute(SpellCustomAttributes flags);

        void RemoveAurasWithDispelType( DispelType type );
        bool RemoveAurasWithSpellFamily(uint32 spellFamilyName, uint8 count, bool withPassive);
        void RemoveMovementImpairingAuras();

        void RemoveAllAuras();
        void RemoveAllAurasExcept(uint32 spellId);
        void RemoveArenaAuras(bool onleave = false);
        void RemoveAllAurasOnDeath();
        void DelayAura(uint32 spellId, uint32 effindex, int32 delaytime);

        float GetResistanceBuffMods(SpellSchools school, bool positive) const { return GetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school ); }
        void SetResistanceBuffMods(SpellSchools school, bool positive, float val) { SetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school,val); }
        void ApplyResistanceBuffModsMod(SpellSchools school, bool positive, float val, bool apply) { ApplyModSignedFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school, val, apply); }
        void ApplyResistanceBuffModsPercentMod(SpellSchools school, bool positive, float val, bool apply) { ApplyPercentModFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school, val, apply); }
        void InitStatBuffMods()
        {
            for(int i = STAT_STRENGTH; i < MAX_STATS; ++i) SetFloatValue(UNIT_FIELD_POSSTAT0+i, 0);
            for(int i = STAT_STRENGTH; i < MAX_STATS; ++i) SetFloatValue(UNIT_FIELD_NEGSTAT0+i, 0);
        }
        void ApplyStatBuffMod(Stats stat, float val, bool apply) { ApplyModSignedFloatValue((val > 0 ? UNIT_FIELD_POSSTAT0+stat : UNIT_FIELD_NEGSTAT0+stat), val, apply); }
        void ApplyStatPercentBuffMod(Stats stat, float val, bool apply)
        {
            ApplyPercentModFloatValue(UNIT_FIELD_POSSTAT0+stat, val, apply);
            ApplyPercentModFloatValue(UNIT_FIELD_NEGSTAT0+stat, val, apply);
        }
        void SetCreateStat(Stats stat, float val) { m_createStats[stat] = val; }
        void SetCreateHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_HEALTH, val); }
        uint32 GetCreateHealth() const { return GetUInt32Value(UNIT_FIELD_BASE_HEALTH); }
        void SetCreateMana(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_MANA, val); }
        uint32 GetCreateMana() const { return GetUInt32Value(UNIT_FIELD_BASE_MANA); }
        uint32 GetCreatePowers(Powers power) const;
        float GetPosStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_POSSTAT0+stat); }
        float GetNegStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_NEGSTAT0+stat); }
        float GetCreateStat(Stats stat) const { return m_createStats[stat]; }

        void SetCurrentCastedSpell(Spell * pSpell);
        virtual void ProhibitSpellSchool(SpellSchoolMask /*idSchoolMask*/, uint32 /*unTimeMs*/ ) { }
        void InterruptSpell(uint32 spellType, bool withDelayed = true, bool withInstant = true);

        bool HasDelayedSpell();

        // set withDelayed to true to account delayed spells as casted
        // delayed+channeled spells are always accounted as casted
        // we can skip channeled or delayed checks using flags
        bool IsNonMeleeSpellCast(bool withDelayed, bool skipChanneled = false, bool skipAutorepeat = false) const;

        // set withDelayed to true to interrupt delayed spells too
        // delayed+channeled spells are always interrupted
        void InterruptNonMeleeSpells(bool withDelayed, uint32 spellid = 0, bool withInstant = true);

        Spell* FindCurrentSpellBySpellId(uint32 spell_id) const;
        
        bool IsCombatStationary();
        bool CanReachWithMeleeAttack(Unit* pVictim, float flat_mod = 0.0f) const;
        
        bool IsCCed() const;

        Spell* GetCurrentSpell(CurrentSpellTypes spellType) const { return m_currentSpells[spellType]; }
        Spell* GetCurrentSpell(uint32 spellType) const { return m_currentSpells[spellType]; }

        Spell* m_currentSpells[CURRENT_MAX_SPELL];

        uint32 m_addDmgOnce;
        uint64 m_TotemSlot[MAX_TOTEM];
        uint64 m_TotemSlot254;
        uint64 m_ObjectSlot[4];
        uint32 m_detectInvisibilityMask;
        uint32 m_invisibilityMask;
        uint32 m_ShapeShiftFormSpellId;
        ShapeshiftForm m_form;
        float m_modMeleeHitChance;
        float m_modRangedHitChance;
        float m_modSpellHitChance;
        int32 m_baseSpellCritChance;

        float m_threatModifier[MAX_SPELL_SCHOOL];
        float m_modAttackSpeedPct[3];

        // Event handler. Processed only when creature is alive
        EventProcessor m_Events;

        // stat system
        bool HandleStatModifier(UnitMods unitMod, UnitModifierType modifierType, float amount, bool apply);
        void SetModifierValue(UnitMods unitMod, UnitModifierType modifierType, float value) { m_auraModifiersGroup[unitMod][modifierType] = value; }
        float GetModifierValue(UnitMods unitMod, UnitModifierType modifierType) const;
        float GetTotalStatValue(Stats stat) const;
        float GetTotalAuraModValue(UnitMods unitMod) const;
        SpellSchools GetSpellSchoolByAuraGroup(UnitMods unitMod) const;
        Stats GetStatByAuraGroup(UnitMods unitMod) const;
        Powers GetPowerTypeByAuraGroup(UnitMods unitMod) const;
        bool CanModifyStats() const { return m_canModifyStats; }
        void SetCanModifyStats(bool modifyStats) { m_canModifyStats = modifyStats; }
        virtual bool UpdateStats(Stats stat) = 0;
        virtual bool UpdateAllStats() = 0;
        virtual void UpdateResistances(uint32 school) = 0;
        virtual void UpdateArmor() = 0;
        virtual void UpdateMaxHealth() = 0;
        virtual void UpdateMaxPower(Powers power) = 0;
        virtual void UpdateAttackPowerAndDamage(bool ranged = false) = 0;
        virtual void UpdateDamagePhysical(WeaponAttackType attType);
        float GetTotalAttackPowerValue(WeaponAttackType attType, Unit* victim = nullptr) const;
        float GetAPBonusVersus(WeaponAttackType attType, Unit* victim) const;
        float GetWeaponDamageRange(WeaponAttackType attType ,WeaponDamageRange type) const;
        void SetBaseWeaponDamage(WeaponAttackType attType ,WeaponDamageRange damageRange, float value) { m_weaponDamage[attType][damageRange] = value; }
        uint32 CalculateDamage(WeaponAttackType attType, bool normalized, SpellInfo const* spellProto = NULL, Unit* target = NULL);
        virtual void CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& minDamage, float& maxDamage, Unit* target = nullptr) = 0;
        float GetAPMultiplier(WeaponAttackType attType, bool normalized);

        virtual bool HasMainWeapon() const { return true; }

        bool IsLevitating() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY); }
        bool IsWalking() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING); }
        virtual bool SetWalk(bool enable);
        virtual bool SetDisableGravity(bool disable, bool packetOnly = false);
        virtual bool SetSwim(bool enable);
        virtual bool SetFlying(bool enable, bool packetOnly = false);
        virtual bool SetWaterWalking(bool enable, bool packetOnly = false);
        virtual bool SetFeatherFall(bool enable, bool packetOnly = false);
        virtual bool SetHover(bool enable, bool packetOnly = false);

        void SetInFront(WorldObject const* target);
        void SetInFront(float x, float y);
        void SetFacingTo(float ori);
        void SetFacingToObject(WorldObject const* object);

        uint64 GetTarget() const { return GetUInt64Value(UNIT_FIELD_TARGET); }
        virtual void SetTarget(uint64 /*guid*/) = 0;

        // Movement info
        Movement::MoveSpline * movespline;

        bool m_ControlledByPlayer;
        bool m_CreatedByPlayer;

        bool HandleSpellClick(Unit* clicker, int8 seatId = -1);
#ifdef LICH_KING
        void EnterVehicle(Unit* base, int8 seatId = -1);
        void EnterVehicleUnattackable(Unit *base, int8 seatId = -1);
        void ExitVehicle(Position const* exitPosition = NULL);
        void ChangeSeat(int8 seatId, bool next = true);

        // Should only be called by AuraEffect::HandleAuraControlVehicle(AuraApplication const* auraApp, uint8 mode, bool apply) const;
        void _ExitVehicle(Position const* exitPosition = NULL);
        void _EnterVehicle(Vehicle* vehicle, int8 seatId, AuraApplication const* aurApp = NULL);
#endif

        void BuildMovementPacket(ByteBuffer *data) const;

        bool isMoving() const   { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_MOVING); }
        bool isTurning() const  { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_TURNING); }
        virtual bool CanFly() const = 0;
        virtual bool CanWalk() const = 0;
        virtual bool CanSwim() const = 0;
        bool IsFlying() const   { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_PLAYER_FLYING | MOVEMENTFLAG_DISABLE_GRAVITY); }
        bool IsHovering() const { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_HOVER); }
        bool IsFalling() const;
        float GetHoverHeight() const { return IsHovering() ? 
#ifdef LICH_KING
            GetFloatValue(UNIT_FIELD_HOVERHEIGHT)
#else
            DEFAULT_HOVER_HEIGHT
#endif
            : 0.0f; 
        }

        // compat TC
        bool IsVisible() const { return m_Visibility != VISIBILITY_OFF; }
        // Visibility system
        UnitVisibility GetVisibility() const { return m_Visibility; }
        void SetVisibility(UnitVisibility x);
        void DestroyForNearbyPlayers();

        // common function for visibility checks for player/creatures with detection code
        virtual bool CanSeeOrDetect(Unit const* u, bool detect, bool inVisibleList = false, bool is3dDistance = true) const;
        bool IsVisibleForOrDetect(Unit const* u, bool detect, bool inVisibleList = false, bool is3dDistance = true) const;
        bool CanDetectInvisibilityOf(Unit const* u) const;
        StealthDetectedStatus CanDetectStealthOf(Unit const* u, float distance) const;

        // virtual functions for all world objects types
        bool IsVisibleForInState(Player const* u, bool inVisibleList) const;
        // function for low level grid visibility checks in player/creature cases
        virtual bool IsVisibleInGridForPlayer(Player const* pl) const = 0;

        AuraList      & GetSingleCastAuras()       { return m_scAuras; }
        AuraList const& GetSingleCastAuras() const { return m_scAuras; }
        SpellImmuneList m_spellImmune[MAX_SPELL_IMMUNITY];

        // Threat related methods
        bool CanHaveThreatList() const;
        float GetThreat(Unit* u) const;
        void AddThreat(Unit* victim, float threat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellInfo const *threatSpell = nullptr);
        void ModifyThreatPct(Unit* victim, int32 percent);
        void ApplyTotalThreatModifier(float& threat, SpellSchoolMask schoolMask = SPELL_SCHOOL_MASK_NORMAL);
        void DeleteThreatList();
        void TauntApply(Unit* pVictim);
        void TauntFadeOut(Unit *taunter);
        ThreatManager& getThreatManager() { return m_ThreatManager; }
        void addHatedBy(HostileReference* pHostileReference) { m_HostileRefManager.insertFirst(pHostileReference); };
        void removeHatedBy(HostileReference* /*pHostileReference*/ ) { /* nothing to do yet */ }
        HostileRefManager& GetHostileRefManager() { return m_HostileRefManager; }
        bool HasInThreatList(uint64 hostileGUID) const;

        //TC compat
        Aura* GetAuraApplication(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint8 reqEffMask = 0, AuraApplication * except = nullptr) const;
        Aura* GetAura(uint32 spellId, uint32 effindex);
        Aura* GetAuraWithCaster(uint32 spellId, uint64 casterGUID);

        AuraApplication* GetAuraApplicationOfRankedSpell(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = 0, uint8 reqEffMask = 0, AuraApplication * except = NULL) const;
        Aura* GetAuraOfRankedSpell(uint32 spellId, uint64 casterGUID = 0, uint64 itemCasterGUID = ObjectGuid::Empty, uint8 reqEffMask = 0) const;

        AuraMap      & GetAuras()       { return m_Auras; }
        AuraMap const& GetAuras() const { return m_Auras; }
        AuraList const& GetAurasByType(AuraType type) const { return m_modAuras[type]; }
        //compat TC
        inline AuraEffectList const& GetAuraEffectsByType(AuraType type) const { return m_modAuras[type]; }
        void ApplyAuraProcTriggerDamage(Aura* aura, bool apply);

        int32 GetTotalAuraModifier(AuraType auratype) const;
        float GetTotalAuraMultiplier(AuraType auratype) const;
        int32 GetMaxPositiveAuraModifier(AuraType auratype) const;
        int32 GetMaxNegativeAuraModifier(AuraType auratype) const;

        int32 GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        float GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;

        int32 GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        float GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;

        Aura* GetDummyAura(uint32 spell_id) const;
        uint32 GetInterruptMask() const { return m_interruptMask; }
        void AddInterruptMask(uint32 mask) { m_interruptMask |= mask; }
        void UpdateInterruptMask();

        uint32 GetDisplayId() const { return GetUInt32Value(UNIT_FIELD_DISPLAYID); }
        void SetDisplayId(uint32 modelId);
        uint32 GetNativeDisplayId() const { return GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID); }
        void SetNativeDisplayId(uint32 modelId) { SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, modelId); }
        ShapeshiftForm GetShapeshiftForm() const { return ShapeshiftForm(GetByteValue(UNIT_FIELD_BYTES_2, 3)); }
        uint32 GetModelForForm (ShapeshiftForm from) const;
        void SetTransForm(uint32 spellid) { m_transform = spellid;}
        uint32 GetTransForm() const { return m_transform;}
        bool IsInDisallowedMountForm() const;

        void AddDynObject(DynamicObject* dynObj);
        void RemoveDynObject(uint32 spellid);
        void RemoveDynObjectWithGUID(uint64 guid) { m_dynObjGUIDs.remove(guid); }
        void RemoveAllDynObjects();
        void AddGameObject(GameObject* gameObj);
        void RemoveGameObject(GameObject* gameObj, bool del);
        void RemoveGameObject(uint32 spellid, bool del);
        void RemoveAllGameObjects();
        DynamicObject *GetDynObject(uint32 spellId, uint32 effIndex);
        DynamicObject *GetDynObject(uint32 spellId);

        /** Change aura state if not already at given value. Be careful not to remove a state if other auras have the same state also active, you can use the m_auraStateAuras to check for those. */
        void ModifyAuraState(AuraStateType flag, bool apply);
        /** Get aura state for given target, needed since some aura state are relative to who casted what*/
        uint32 BuildAuraStateUpdateForTarget(Unit* target) const;
        /* Check if unit has at least one aura of given state; This just checks UNIT_FIELD_AURASTATE if no caster is given, or check in m_auraStateAuras if so (some state being caster dependant) */
        bool HasAuraState(AuraStateType flag, SpellInfo const* spellProto = nullptr, Unit const* Caster = nullptr) const;
        void UnsummonAllTotems();
        Unit* GetMagicHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo);
        Unit* GetMeleeHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo = nullptr);

        uint32 GetCastingTimeForBonus(SpellInfo const* spellProto, DamageEffectType damagetype, uint32 CastingTime) const;
        float CalculateDefaultCoefficient(SpellInfo const *spellInfo, DamageEffectType damagetype) const;

        int32 SpellBaseDamageBonusDone(SpellSchoolMask schoolMask, Unit* pVictim = NULL);
        int32 SpellBaseDamageBonusTaken(SpellSchoolMask schoolMask, bool isDoT = false);
        float SpellPctDamageModsDone(Unit* victim, SpellInfo const *spellProto, DamageEffectType damagetype);

        uint32 SpellDamageBonusDone(Unit *pVictim, SpellInfo const *spellProto, uint32 damage, DamageEffectType damagetype, float TotalMod = 0.0f, uint32 stack = 1);
        uint32 SpellDamageBonusTaken(Unit* caster, SpellInfo const *spellProto, uint32 pdamage, DamageEffectType damagetype, uint32 stack = 1);
        
        // SPELL_AURA_MOD_HEALING_DONE + SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT + SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER + hacks
        int32 SpellBaseHealingBonusDone(SpellSchoolMask schoolMask);
        // SPELL_AURA_MOD_HEALING + hacks
        int32 SpellBaseHealingBonusTaken(SpellSchoolMask schoolMask);
        //Includes SPELL_AURA_MOD_HEALING_DONE_PERCENT + lots of hacks
        float SpellPctHealingModsDone(Unit* victim, SpellInfo const *spellProto, DamageEffectType damagetype);

        /* Alter healamount with healing bonus taken
        This includes : SPELL_AURA_MOD_HEALING_PCT, SPELL_AURA_MOD_HEALING_PCT, SpellBaseHealingBonusTaken SPELLMOD_BONUS_MULTIPLIER, and lots of hacks
        */
        uint32 SpellHealingBonusTaken(Unit* caster, SpellInfo const *spellProto, uint32 healamount, DamageEffectType damagetype, uint32 stack = 1);
        /* Alter healamount with healing bonus done for victim
        This includes : SpellBaseHealingBonusDone, SPELLMOD_BONUS_MULTIPLIER, ApplySpellMod, SpellPctHealingModsDone, and lots of hacks
        */
        uint32 SpellHealingBonusDone(Unit* victim, SpellInfo const *spellProto, uint32 healamount, DamageEffectType damagetype, float TotalMod = 0.0f, uint32 stack = 1);

        void MeleeDamageBonus(Unit *pVictim, uint32 *damage, WeaponAttackType attType, SpellInfo const *spellProto = NULL);
//TODO        uint32 MeleeDamageBonusTaken(Unit* attacker, uint32 pdamage, WeaponAttackType attType, SpellInfo const *spellProto = NULL);

        bool   IsSpellBlocked(Unit *pVictim, SpellInfo const *spellProto, WeaponAttackType attackType = BASE_ATTACK);
        bool   IsSpellCrit(Unit *pVictim, SpellInfo const *spellProto, SpellSchoolMask schoolMask, WeaponAttackType attackType = BASE_ATTACK);
        uint32 SpellCriticalBonus(SpellInfo const *spellProto, uint32 damage, Unit *pVictim);
        //Spells disabled in spell_disabled table
        bool IsSpellDisabled(uint32 const spellId);

        void SetLastManaUse(uint32 spellCastTime) { m_lastManaUse = spellCastTime; }
        bool IsUnderLastManaUseEffect() const;

        void SetContestedPvP(Player *attackedPlayer = NULL);

        void ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply);
        void ApplySpellDispelImmunity(const SpellInfo * spellProto, DispelType type, bool apply);
        virtual bool IsImmunedToSpell(SpellInfo const* spellInfo, bool useCharges = false);
                                                            // redefined in Creature
        bool IsImmunedToDamage(SpellSchoolMask meleeSchoolMask, bool useCharges = false);
        virtual bool IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index) const;
                                                            // redefined in Creature

        uint32 CalcArmorReducedDamage(Unit* pVictim, const uint32 damage);
        void CalcAbsorbResist(Unit *pVictim, SpellSchoolMask schoolMask, DamageEffectType damagetype, const uint32 damage, uint32 *absorb, uint32 *resist, uint32 spellId);

        void  UpdateSpeed(UnitMoveType mtype);
        float GetSpeed( UnitMoveType mtype ) const;
        float GetSpeedRate( UnitMoveType mtype ) const { return m_speed_rate[mtype]; }
        void SetSpeedRate(UnitMoveType mtype, float rate, bool sendUpdate = true);

        void _RemoveAllAuraMods();
        void _ApplyAllAuraMods();

        int32 CalculateSpellDamage(SpellInfo const* spellProto, uint8 effect_index, int32 basePoints, Unit const* target);
        int32 CalculateSpellDuration(SpellInfo const* spellProto, uint8 effect_index, Unit const* target);
        float CalculateLevelPenalty(SpellInfo const* spellProto) const;
        void ModSpellCastTime(SpellInfo const* spellProto, int32 & castTime, Spell * spell);
        int32 CalculateAOEDamageReduction(int32 damage, uint32 schoolMask, Unit* caster) const;

        void AddFollower(FollowerReference* pRef) { m_FollowingRefManager.insertFirst(pRef); }
        void RemoveFollower(FollowerReference* /*pRef*/) { /* nothing to do yet */ }

        MotionMaster* GetMotionMaster() { return i_motionMaster; }
        const MotionMaster* GetMotionMaster() const { return i_motionMaster; }

        //returns wheter unit his currently stopped (= has any move related UNIT_STATE)
        bool IsStopped() const { return !(HasUnitState(UNIT_STATE_MOVING)); }
        void StopMoving();
        void StopMovingOnCurrentPos();

        void AddUnitMovementFlag(uint32 f) { m_movementInfo.flags |= f; }
        void RemoveUnitMovementFlag(uint32 f) { m_movementInfo.flags &= ~f; }
        bool HasUnitMovementFlag(uint32 f) const { return (m_movementInfo.flags & f) == f; }
        uint32 GetUnitMovementFlags() const { return m_movementInfo.flags; }
        void SetUnitMovementFlags(uint32 f) { m_movementInfo.flags = f; }

        float GetPositionZMinusOffset() const;

        void SetControlled(bool apply, UnitState state);
        void SetFeared(bool apply);
        void SetConfused(bool apply);
        void SetStunned(bool apply);
        /** Set as rooted, updates unit flags, stop spline movement and send update to players around */
        void SetRooted(bool apply);

        uint32 m_rootTimes;

        void AddComboPointHolder(uint32 lowguid) { m_ComboPointHolders.insert(lowguid); }
        void RemoveComboPointHolder(uint32 lowguid) { m_ComboPointHolders.erase(lowguid); }
        void ClearComboPointHolders();

        ///----------Pet responses methods-----------------
        void SendPetCastFail(uint32 spellid, uint8 msg);
        void SendPetActionFeedback (uint8 msg);
        void SendPetTalk (uint32 pettalk);
        void SendPetSpellCooldown (uint32 spellid, time_t cooltime);
        //send AI_REACTION_HOSTILE (aggro sound) to owner if owner exists and is a player
        void SendPetAIReaction();
        ///----------End of Pet responses methods----------

        void PropagateSpeedChange() { GetMotionMaster()->PropagateSpeedChange(); }

        /* Send AI reaction to target, or to all players around if none given
         (this for example used to do the aggro sound) */
        void SendAIReaction(AIReaction reaction, Player* target = nullptr);

        // reactive attacks
        void ClearAllReactives();
        void StartReactiveTimer( ReactiveType reactive ) { m_reactiveTimer[reactive] = REACTIVE_TIMER_START;}
        void UpdateReactives(uint32 p_time);
        bool HasReactiveTimerActive(ReactiveType reactive) const;

        // group updates
        void UpdateAuraForGroup(uint8 slot);

        // pet auras
        typedef std::set<PetAura const*> PetAuraSet;
        PetAuraSet m_petAuras;
        void AddPetAura(PetAura const* petSpell);
        void RemovePetAura(PetAura const* petSpell);

        // relocation notification
        void SetToNotify();
        bool m_Notified, m_IsInNotifyList;
        float oldX, oldY, oldZ;

        void SetReducedThreatPercent(uint32 pct, uint64 guid)
        {
            m_redirectThreatPercent = pct;
            m_misdirectionTargetGUID = guid;
        }

        void SetLastMisdirectionTargetGUID(uint64 guid)
        {
            m_misdirectionLastTargetGUID = guid;
        }

        uint32 GetRedirectThreatPercent() { return m_redirectThreatPercent; }
        Unit *GetRedirectThreatTarget();
        Unit *GetLastRedirectTarget();

        bool IsAIEnabled, NeedChangeAI;
             
        //TODO: these should be removed in favor of WorldObject::FindNearestCreature and FindNearestGameObject
        Creature* FindCreatureInGrid(uint32 entry, float range, bool isAlive);
        GameObject* FindGOInGrid(uint32 entry, float range);
        
        Pet* ToPet(){ if(IsPet()) return reinterpret_cast<Pet*>(this); else return NULL; } 
        Pet const* ToPet() const { if (IsPet()) return reinterpret_cast<Pet const*>(this); else return NULL; }

        Totem* ToTotem(){ if(IsTotem()) return reinterpret_cast<Totem*>(this); else return NULL; } 
        Totem const* ToTotem() const { if (IsTotem()) return reinterpret_cast<Totem const*>(this); else return NULL; }

        TemporarySummon* ToTemporarySummon() { if (IsSummon()) return reinterpret_cast<TemporarySummon*>(this); else return NULL; }
        TemporarySummon const* ToTemporarySummon() const { if (IsSummon()) return reinterpret_cast<TemporarySummon const*>(this); else return NULL; }

        void SetSummoner(Unit* summoner) { m_summoner = summoner->GetGUID(); }
        virtual Unit* GetSummoner() const;
        uint64 GetSummonerGUID() { return m_summoner; }
        
        bool IsJustCCed() { return (m_justCCed > 0); }
        
        // Part of Evade mechanics
        time_t GetLastDamagedTime() const { return _lastDamagedTime; }
        void SetLastDamagedTime(time_t val) { _lastDamagedTime = val; }

        virtual void Talk(std::string const& text, ChatMsg msgType, Language language, float textRange, WorldObject const* target);
        virtual void Say(std::string const& text, Language language = LANG_UNIVERSAL, WorldObject const* target = nullptr);
        virtual void Yell(std::string const& text, Language language = LANG_UNIVERSAL, WorldObject const* target = nullptr);
        virtual void TextEmote(std::string const& text, WorldObject const* target = nullptr, bool isBossEmote = false);
        virtual void Whisper(std::string const& text, Language language, Player* target, bool isBossWhisper = false);
        virtual void ServerEmote(std::string const& text, bool isBossEmote = false);
        virtual void YellToMap(std::string const& text, Language language = LANG_UNIVERSAL);
        // textId: Id from broadcast_text
        void Talk(uint32 textId, ChatMsg msgType, float textRange, WorldObject const* target);
        // textId: Id from broadcast_text
        void Say(uint32 textId, WorldObject const* target = nullptr);
        // textId: Id from broadcast_text
        void Yell(uint32 textId, WorldObject const* target = nullptr);
        // textId: Id from broadcast_text
        void TextEmote(uint32 textId, WorldObject const* target = nullptr, bool isBossEmote = false);
        // textId: Id from broadcast_text
        void Whisper(uint32 textId, Player* target, bool isBossWhisper = false);

        //retrocompatibility function, do not use. TextId's here are from script_texts or trinity_string
        void old_Talk(uint32 textId, ChatMsg msgType, float textRange, uint64 targetGUID, uint32 language);
        void old_Say(int32 textId, uint32 language, uint64 TargetGuid);
        void old_Yell(int32 textId, uint32 language, uint64 TargetGuid);
        void old_TextEmote(int32 textId, uint64 TargetGuid, bool IsBossEmote = false);
        void old_Whisper(int32 textId, uint64 receiver, bool IsBossWhisper = false);

        // update m_last_isunderwater_status 
        void UpdateEnvironmentIfNeeded(const uint8 option);

        Position m_last_environment_position;
        bool m_last_isinwater_status;
        bool m_last_isunderwater_status;
        bool m_is_updating_environment;
    protected:
        explicit Unit ();

        void BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const override;

        UnitAI* i_AI;
        UnitAI* i_disabledAI;

        void _UpdateSpells(uint32 time);
        void _DeleteAuras();

        void _UpdateAutoRepeatSpell();
        bool m_AutoRepeatFirstCast;

        uint32 m_attackTimer[MAX_ATTACK];

        float m_createStats[MAX_STATS];

        AttackerSet m_attackers;
        Unit* m_attacking;

        DeathState m_deathState;

        AuraMap m_Auras;
        AuraMap::iterator m_AurasUpdateIterator;
        uint32 m_removedAurasCount;

        typedef std::list<uint64> DynObjectGUIDs;
        DynObjectGUIDs m_dynObjGUIDs;

        std::list<GameObject*> m_gameObj;
        bool m_isSorted;
        uint32 m_transform;
        AuraList m_removedAuras;

        AuraList m_modAuras[TOTAL_AURAS];
        std::mutex m_GiantLock;
        AuraList m_scAuras;                        // casted singlecast auras
        AuraList m_interruptableAuras;
        AuraList m_ccAuras;
        AuraStateAurasMap m_auraStateAuras;        // List of all auras affecting aura states, casted by who, Used for improve performance of aura state checks on aura apply/remove
        uint32 m_interruptMask;

        float m_auraModifiersGroup[UNIT_MOD_END][MODIFIER_TYPE_END];
        float m_weaponDamage[MAX_ATTACK][2];
        bool m_canModifyStats;
        //std::list< spellEffectPair > AuraSpells[TOTAL_AURAS];  // TODO: use this if ok for mem

        float m_speed_rate[MAX_MOVE_TYPE];

        CharmInfo *m_charmInfo;
        SharedVisionList m_sharedVision;

        virtual SpellSchoolMask GetMeleeDamageSchoolMask() const;

        MotionMaster* i_motionMaster;

        uint32 m_reactiveTimer[MAX_REACTIVE];

        ThreatManager m_ThreatManager;
        
        uint32 m_unitTypeMask;
        LiquidTypeEntry const* _lastLiquid;
        
        uint64 m_summoner;
        
        uint8 m_justCCed; // Set to 2 when getting CC aura, decremented (if > 0) every update - used to stop pet combat on target

        std::map<uint64,uint16> m_detectedUnit; // <guid,timer> for stealth detection, a spotted unit is kept visible for a while 
        std::set<uint64> m_detectedByUnit; //we need to keep track of who detected us to be able to reset it when needed

    private:
        bool IsTriggeredAtSpellProcEvent( Aura* aura, SpellInfo const* procSpell, uint32 procFlag, uint32 procExtra, WeaponAttackType attType, bool isVictim, bool active, SpellProcEventEntry const*& spellProcEvent );
        bool HandleDummyAuraProc(   Unit *pVictim, uint32 damage, Aura* triggredByAura, SpellInfo const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        bool HandleHasteAuraProc(   Unit *pVictim, uint32 damage, Aura* triggredByAura, SpellInfo const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        bool HandleProcTriggerSpell(Unit *pVictim, uint32 damage, Aura* triggredByAura, SpellInfo const *procSpell, uint32 procFlag, uint32 procEx, uint32 cooldown);
        bool HandleOverrideClassScriptAuraProc(Unit *pVictim, Aura* triggredByAura, SpellInfo const *procSpell, uint32 cooldown);
        bool HandleMendingAuraProc(Aura* triggeredByAura);

        uint32 m_state;                                     // Even derived shouldn't modify
        uint32 m_CombatTimer;
        uint32 m_lastManaUse;                               // msecs
        TimeTrackerSmall m_movesplineTimer;

        UnitVisibility m_Visibility;

        Diminishing m_Diminishing;
        // Manage all Units threatening us
//        ThreatManager m_ThreatManager;
        // Manage all Units that are threatened by us
        HostileRefManager m_HostileRefManager;

        FollowerRefManager m_FollowingRefManager;

        ComboPointHolderSet m_ComboPointHolders;

        uint32 m_redirectThreatPercent;
        uint64 m_misdirectionTargetGUID;
        uint64 m_misdirectionLastTargetGUID;
        
        uint8 IsRotating;//0none 1left 2right
        uint32 RotateTimer;
        uint32 RotateTimerFull;
        double RotateAngle;
        uint64 LastTargetGUID;
        bool m_attackVictimOnEnd;

        uint32 m_procDeep;
        
        bool _targetLocked; // locks the target during spell cast for proper facing
        time_t _lastDamagedTime; // Part of Evade mechanic
};

// drop a charge for the first aura found of type SPELL_AURA_SPELL_MAGNET
class RedirectSpellEvent : public BasicEvent
{
public:
    RedirectSpellEvent(Unit& self, uint64 auraOwnerGUID, AuraEffect* auraEffect) : _self(self), _auraOwnerGUID(auraOwnerGUID), _auraEffect(auraEffect) { }
    bool Execute(uint64 e_time, uint32 p_time);

protected:
    Unit& _self;
    uint64 _auraOwnerGUID;
    AuraEffect* _auraEffect;
};

#endif

