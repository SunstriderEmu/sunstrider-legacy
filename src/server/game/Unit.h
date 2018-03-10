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
#include "CombatManager.h"
#include "Movement/FollowerReference.h"
#include "Movement/FollowerRefManager.h"
#include "EventProcessor.h"
#include "Movement/MotionMaster.h"
#include "DBCStructure.h"
#include "Util.h"
#include <list>
#include "SpellDefines.h"
#include "SpellInfo.h"
#include "ItemPrototype.h"
class UnitAI;
class SpellCastTargets;
class Aura;
class AuraEffect;
class TempSummon;
class Guardian;
class Minion;
class TransportBase;
class UnitAura;
class AuraApplication;
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
class Unit;
class SpellHistory;

#define WORLD_TRIGGER   12999

#define BASE_MINDAMAGE 1.0f
#define BASE_MAXDAMAGE 2.0f
#define BASE_ATTACK_TIME 2000

#define MAX_SPELL_CHARM         4
#define MAX_SPELL_VEHICLE       6
#define MAX_SPELL_POSSESS       8
#define MAX_SPELL_CONTROL_BAR   10

#define MAX_AGGRO_RESET_TIME 10 // in seconds
#define MAX_AGGRO_RADIUS 45.0f  // yards

#define DEFAULT_HOVER_HEIGHT 1.0f

// byte value (UNIT_FIELD_BYTES_1,0) // UNIT_BYTES_1_OFFSET_STAND_STATE
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

// byte flag value (UNIT_FIELD_BYTES_1, 2) // UNIT_BYTES_1_OFFSET_VIS_FLAG
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


enum UnitBytes1Offsets
{
    UNIT_BYTES_1_OFFSET_STAND_STATE = 0,
    UNIT_BYTES_1_OFFSET_PET_LOYALTY = 1,
    UNIT_BYTES_1_OFFSET_VIS_FLAG = 2,
    UNIT_BYTES_1_OFFSET_ANIM_TIER = 3,
};

enum UnitBytes2Offsets
{
    UNIT_BYTES_2_OFFSET_SHEATH_STATE = 0,
    UNIT_BYTES_2_OFFSET_1_UNK = 1, //Not sure what this index is for. TC has UNIT_BYTES_2_OFFSET_PVP_FLAG = 1, but this may not be on BC since PvP handling has changed a lot
    UNIT_BYTES_2_OFFSET_PET_FLAGS = 2,
    UNIT_BYTES_2_OFFSET_SHAPESHIFT_FORM = 3,
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

// low byte ( 0 from 0..3 ) of UNIT_FIELD_BYTES_2 // UNIT_BYTES_2_OFFSET_SHEATH_STATE
enum SheathState
{
    SHEATH_STATE_UNARMED  = 0,                              // non prepared weapon
    SHEATH_STATE_MELEE    = 1,                              // prepared melee weapon
    SHEATH_STATE_RANGED   = 2                               // prepared ranged weapon
};

#define MAX_SHEATH_STATE    3

// byte (1 from 0..3) of UNIT_FIELD_BYTES_2 // UNIT_BYTES_2_OFFSET_1_UNK
// All these seem to have changed with LK
enum UnitBytes2_Flags
{
    UNIT_BYTE2_FLAG_UNK0  = 0x01, //TC has UNIT_BYTE2_FLAG_PVP, but this is probably new with Lich King, since we use UNIT_FLAG_PVP instead
    UNIT_BYTE2_FLAG_UNK1  = 0x02, //seems related to PvP mode in LK, not sure this is valid in BC
    UNIT_BYTE2_FLAG_UNK2  = 0x04, //TC has UNIT_BYTE2_FLAG_FFA_PVP, but this is probably new with Lich King, since we use PLAYER_FLAGS_FFA_PVP instead
    UNIT_BYTE2_FLAG_UNK3  = 0x08, //in current logic, always set on player and pet. What is this for? Mangos has UNIT_BYTE2_FLAG_SUPPORTABLE  =  allows for being targeted for healing/bandaging by friendlies
    UNIT_BYTE2_FLAG_AURAS = 0x10, // show positive auras as positive, and allow its dispel (?? Not sure this is okay)
    UNIT_BYTE2_FLAG_UNK5  = 0x20, //Currently always set on player, not sure what this does // mangos has: show negative auras as positive, *not* allowing dispel (at least for pets)
    UNIT_BYTE2_FLAG_UNK6  = 0x40, //currently unused
    UNIT_BYTE2_FLAG_UNK7  = 0x80, //currently unused
};

// byte (2 from 0..3) of UNIT_FIELD_BYTES_2 // UNIT_BYTES_2_OFFSET_PET_FLAGS
// Those has changed with LK
enum UnitRename
{
    UNIT_RENAME_NOT_ALLOWED = 0x02,
    UNIT_RENAME_ALLOWED     = 0x03
};

#define MAX_CREATURE_SPELLS     8

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

//this is sent to client in SMSG_ATTACKERSTATEUPDATE or other packets
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
    HITINFO_OFFHAND             = 0x00000004,
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

struct SpellImmune
{
    uint32 type;
    uint32 spellId;
};

typedef std::list<SpellImmune> SpellImmuneList;

typedef std::list<Unit*> UnitList;

class DispelableAura
{
public:
    DispelableAura(Aura* aura, int32 dispelChance, uint8 dispelCharges);
    ~DispelableAura();

    Aura* GetAura() const { return _aura; }
    bool RollDispel() const;
    uint8 GetDispelCharges() const { return _charges; }

    void IncrementCharges() { ++_charges; }
    bool DecrementCharge()
    {
        if (!_charges)
            return false;

        --_charges;
        return _charges > 0;
    }

private:
    Aura * _aura;
    int32 _chance;
    uint8 _charges;
};
typedef std::vector<DispelableAura> DispelChargesList;

enum UnitModifierFlatType
{
	BASE_VALUE = 0,
	TOTAL_VALUE = 1,
	MODIFIER_TYPE_FLAT_END = 2
};

enum UnitModifierPctType
{
	BASE_PCT = 0,
	TOTAL_PCT = 1,
	MODIFIER_TYPE_PCT_END = 2
};

enum WeaponDamageRange
{
    MINDAMAGE,
    MAXDAMAGE,

    MAX_WEAPON_DAMAGE_RANGE,
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

enum UnitState : uint32
{
    UNIT_STATE_DIED                  = 0x00000001,
    UNIT_STATE_MELEE_ATTACKING       = 0x00000002,                     // player is melee attacking someone
    UNIT_STATE_CHARMED               = 0x00000004,                     // having any kind of charm aura on self
    UNIT_STATE_STUNNED               = 0x00000008,
    UNIT_STATE_ROAMING               = 0x00000010,
    UNIT_STATE_CHASE                 = 0x00000020,
    UNIT_STATE_FOCUSING              = 0x00000040,
    UNIT_STATE_FLEEING               = 0x00000080,
    UNIT_STATE_IN_FLIGHT             = 0x00000100,                     // player is in flight mode
    UNIT_STATE_FOLLOW                = 0x00000200,
    UNIT_STATE_ROOT                  = 0x00000400,
    UNIT_STATE_CONFUSED              = 0x00000800,
    UNIT_STATE_DISTRACTED            = 0x00001000,
    UNIT_STATE_ISOLATED              = 0x00002000,                     // area auras do not affect other players
    UNIT_STATE_ATTACK_PLAYER         = 0x00004000,
    UNIT_STATE_CASTING               = 0x00008000,
    UNIT_STATE_POSSESSED             = 0x00010000,                     // being possessed by another unit
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
    UNIT_STATE_ALL_STATE_SUPPORTED = UNIT_STATE_DIED | UNIT_STATE_MELEE_ATTACKING | UNIT_STATE_CHARMED | UNIT_STATE_STUNNED | UNIT_STATE_ROAMING | UNIT_STATE_CHASE
                                   | UNIT_STATE_FOCUSING | UNIT_STATE_FLEEING | UNIT_STATE_IN_FLIGHT | UNIT_STATE_FOLLOW | UNIT_STATE_ROOT | UNIT_STATE_CONFUSED
                                   | UNIT_STATE_DISTRACTED | UNIT_STATE_ISOLATED | UNIT_STATE_ATTACK_PLAYER | UNIT_STATE_CASTING
                                   | UNIT_STATE_POSSESSED | UNIT_STATE_CHARGING | UNIT_STATE_JUMPING | UNIT_STATE_MOVE | UNIT_STATE_ROTATING
                                   | UNIT_STATE_EVADE | UNIT_STATE_ROAMING_MOVE | UNIT_STATE_CONFUSED_MOVE | UNIT_STATE_FLEEING_MOVE
                                   | UNIT_STATE_CHASE_MOVE | UNIT_STATE_FOLLOW_MOVE | UNIT_STATE_IGNORE_PATHFINDING,
    UNIT_STATE_UNATTACKABLE        = UNIT_STATE_IN_FLIGHT,
    // for real move using movegen check and stop (except unstoppable flight)
    UNIT_STATE_MOVING              = UNIT_STATE_ROAMING_MOVE | UNIT_STATE_CONFUSED_MOVE | UNIT_STATE_FLEEING_MOVE | UNIT_STATE_CHASE_MOVE | UNIT_STATE_FOLLOW_MOVE,
    UNIT_STATE_CONTROLLED          = UNIT_STATE_CONFUSED | UNIT_STATE_STUNNED | UNIT_STATE_FLEEING,
    UNIT_STATE_CANT_CLIENT_CONTROL = UNIT_STATE_CHARMED | UNIT_STATE_FLEEING | UNIT_STATE_CONFUSED | UNIT_STATE_POSSESSED,
    UNIT_STATE_LOST_CONTROL        = UNIT_STATE_CONTROLLED | UNIT_STATE_JUMPING | UNIT_STATE_POSSESSED | UNIT_STATE_CHARGING,
    UNIT_STATE_SIGHTLESS           = UNIT_STATE_LOST_CONTROL | UNIT_STATE_EVADE,
    UNIT_STATE_CANNOT_AUTOATTACK   = UNIT_STATE_CONTROLLED | UNIT_STATE_CHARGING | UNIT_STATE_CASTING,
    UNIT_STATE_CANNOT_TURN         = UNIT_STATE_LOST_CONTROL | UNIT_STATE_ROTATING | UNIT_STATE_FOCUSING,
    // stay by different reasons
    UNIT_STATE_NOT_MOVE            = UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DIED | UNIT_STATE_DISTRACTED,

    UNIT_STATE_ALL_ERASABLE        = UNIT_STATE_ALL_STATE_SUPPORTED & ~(UNIT_STATE_IGNORE_PATHFINDING),
    UNIT_STATE_ALL_STATE           = 0xffffffff                      //(UNIT_STATE_STOPPED | UNIT_STATE_MOVING | UNIT_STATE_IN_COMBAT | UNIT_STATE_IN_FLIGHT)
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

enum WeaponAttackType : unsigned int
{
    BASE_ATTACK   = 0,
    OFF_ATTACK    = 1,
    RANGED_ATTACK = 2,
    MAX_ATTACK    = 3
};

//BC has no spell with more than CR_EXPERTISE
enum CombatRating : unsigned int
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
#ifdef LICH_KING
    CR_ARMOR_PENETRATION        = 24,
#endif
    MAX_COMBAT_RATING           ,
};

enum DamageEffectType : unsigned int
{
    DIRECT_DAMAGE           = 0,                            // used for normal weapon damage (not for class abilities or spells)
    SPELL_DIRECT_DAMAGE     = 1,                            // spell/class abilities damage
    DOT                     = 2,
    HEAL                    = 3,
    NODAMAGE                = 4,                            // used also in case when damage applied to health but not applied to spell channelInterruptFlags/etc
    SELF_DAMAGE             = 5,

    TOTAL_DAMAGE_EFFECT_TYPE,
};

/*
enum UnitVisibility : unsigned int
{
    VISIBILITY_OFF                = 0,                      // absolute, not detectable, GM-like, can see all other
    VISIBILITY_ON                 = 1,
    VISIBILITY_GROUP_STEALTH      = 2,                      // detect chance, seen and can see group members
    //VISIBILITY_GROUP_INVISIBILITY = 3,                      // invisibility, can see and can be seen only another invisible unit or invisible detection unit, set only if not stealthed, and in checks not used (mask used instead)
    //VISIBILITY_GROUP_NO_DETECT    = 4,                      // state just at stealth apply for update Grid state. Don't remove, otherwise stealth spells will break
    VISIBILITY_RESPAWN            = 5                       // special totally not detectable visibility for force delete object at respawn command
};
*/

// Value masks for UNIT_FIELD_FLAGS
enum UnitFlags : unsigned int
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
    UNIT_FLAG_PET_IN_COMBAT         = 0x00000800,                // on player pets: whether the pet is chasing a target to attack || on other units: whether any of the unit's minions is in combat
    UNIT_FLAG_PVP                   = 0x00001000,                // changed in 3.0.3
    UNIT_FLAG_SILENCED              = 0x00002000,                // silenced, 2.1.1
    UNIT_FLAG_CANNOT_SWIM           = 0x00004000,                // 2.0.8
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
enum UnitFlags2 : unsigned int
{
    UNIT_FLAG2_FEIGN_DEATH       = 0x00000001,
    UNIT_FLAG2_UNK1              = 0x00000002,   // Hide unit model (show only player equip)
    UNIT_FLAG2_IGNORE_REPUTATION = 0x00000004,
    UNIT_FLAG2_COMPREHEND_LANG   = 0x00000008,
    UNIT_FLAG2_MIRROR_IMAGE      = 0x00000010,
    UNIT_FLAG2_FORCE_MOVE        = 0x00000040,
    //not sure where do the BC ones stop here
#ifdef LICH_KING
    UNIT_FLAG2_DISARM_OFFHAND               = 0x00000080,
    UNIT_FLAG2_DISABLE_PRED_STATS           = 0x00000100,   // Player has disabled predicted stats (Used by raid frames)
    UNIT_FLAG2_DISARM_RANGED                = 0x00000400,   // this does not disable ranged weapon display (maybe additional flag needed?)
    UNIT_FLAG2_REGENERATE_POWER             = 0x00000800,
    UNIT_FLAG2_RESTRICT_PARTY_INTERACTION   = 0x00001000,   // Restrict interaction to party or raid
    UNIT_FLAG2_PREVENT_SPELL_CLICK          = 0x00002000,   // Prevent spellclick
    UNIT_FLAG2_ALLOW_ENEMY_INTERACT         = 0x00004000,
    UNIT_FLAG2_DISABLE_TURN                 = 0x00008000,
    UNIT_FLAG2_UNK2                         = 0x00010000,
    UNIT_FLAG2_PLAY_DEATH_ANIM              = 0x00020000,   // Plays special death animation upon death
    UNIT_FLAG2_ALLOW_CHEAT_SPELLS           = 0x00040000    // Allows casting spells with AttributesEx7 & SPELL_ATTR7_IS_CHEAT_SPELL
#endif
};

/// Non Player Character flags
enum NPCFlags : int
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

enum MoveFlags : int
{
    MOVEFLAG_NONE               = 0x00000000,
    MOVEFLAG_SLIDE              = 0x00000002,
    MOVEFLAG_MARCH_ON_SPOT      = 0x00000004,
    MOVEFLAG_JUMP               = 0x00000008,
    MOVEFLAG_WALK               = 0x00000100,
    MOVEFLAG_FLY                = 0x00000200,   //For dragon (+walk = glide)
    MOVEFLAG_ORIENTATION        = 0x00000400,   //Fix orientation
};

enum MovementFlags : int
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
    
    MOVEMENTFLAG_MASK_MOVING =
        MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT |
        MOVEMENTFLAG_JUMPING_OR_FALLING | MOVEMENTFLAG_FALLING_FAR | MOVEMENTFLAG_ASCENDING | MOVEMENTFLAG_DESCENDING |
        MOVEMENTFLAG_SPLINE_ELEVATION,

    MOVEMENTFLAG_MASK_TURNING =
        MOVEMENTFLAG_LEFT | MOVEMENTFLAG_RIGHT | MOVEMENTFLAG_PITCH_UP | MOVEMENTFLAG_PITCH_DOWN,

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

enum UnitTypeMask : int
{
    UNIT_MASK_NONE                  = 0x00000000,
    UNIT_MASK_SUMMON                = 0x00000001,
    UNIT_MASK_MINION                = 0x00000002,
    UNIT_MASK_GUARDIAN              = 0x00000004,
    UNIT_MASK_TOTEM                 = 0x00000008,
    UNIT_MASK_PET                   = 0x00000010,
    UNIT_MASK_VEHICLE               = 0x00000020,
    UNIT_MASK_PUPPET                = 0x00000040,
    UNIT_MASK_HUNTER_PET            = 0x00000080,
    UNIT_MASK_CONTROLABLE_GUARDIAN  = 0x00000100,
};

namespace Movement{
    class MoveSpline;
}

struct DiminishingReturn
{
    DiminishingReturn() : stack(0), hitTime(0), hitCount(DIMINISHING_LEVEL_1) { }

    void Clear()
    {
        stack = 0;
        hitTime = 0;
        hitCount = DIMINISHING_LEVEL_1;
    }

    uint16                  stack:16;
    uint32                  hitTime;
    uint32                  hitCount;
};

enum MeleeHitOutcome : uint8
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
    MELEE_HIT_BLOCK_CRIT, //not used?

    MELEE_HIT_TOTAL,
};

class DispelInfo
{
public:
    explicit DispelInfo(Unit* dispeller, uint32 dispellerSpellId, uint8 chargesRemoved) :
        _dispellerUnit(dispeller), _dispellerSpell(dispellerSpellId), _chargesRemoved(chargesRemoved) { }

    Unit* GetDispeller() const { return _dispellerUnit; }
    uint32 GetDispellerSpellId() const { return _dispellerSpell; }
    uint8 GetRemovedCharges() const { return _chargesRemoved; }
    void SetRemovedCharges(uint8 amount) { _chargesRemoved = amount; }
private:
    Unit * _dispellerUnit;
    uint32 _dispellerSpell;
    uint8 _chargesRemoved;
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
    Unit* Attacker;
    Unit* Target;

    struct
    {
        uint32 DamageSchoolMask;
        uint32 Damage;
        uint32 Absorb;
        uint32 Resist;
    } Damages[MAX_ITEM_PROTO_DAMAGES];

    uint32 Blocked;
    uint32 HitInfo;
    uint32 TargetState;

    // Helper
    WeaponAttackType AttackType;
    uint32 ProcAttacker;
    uint32 ProcVictim;
    uint32 CleanDamage;          // Used only fo rage calcultion
    MeleeHitOutcome HitOutCome;  // TODO: remove this field (need use TargetState)
};

struct SpellNonMeleeDamage;

class TC_GAME_API DamageInfo
{
private:
    Unit* const m_attacker;
    Unit* const m_victim;
    uint32 m_damage;
    SpellInfo const* const m_spellInfo;
    SpellSchoolMask /*const*/ m_schoolMask;
    DamageEffectType const m_damageType;
    WeaponAttackType m_attackType;
    uint32 m_absorb;
    uint32 m_resist;
    uint32 m_block;
    uint32 m_hitMask;

    // amalgamation constructor (used for proc)
    DamageInfo(std::vector<DamageInfo> const dmgs); //one for each MAX_ITEM_PROTO_DAMAGES

public:
    DamageInfo(Unit* attacker, Unit* victim, uint32 damage, SpellInfo const* spellInfo, SpellSchoolMask schoolMask, DamageEffectType damageType, WeaponAttackType attackType);
    explicit DamageInfo(CalcDamageInfo const& dmgInfo); // amalgamation wrapper
    DamageInfo(CalcDamageInfo const& dmgInfo, uint8 damageIndex);
    DamageInfo(SpellNonMeleeDamage const& spellNonMeleeDamage, DamageEffectType damageType, WeaponAttackType attackType, uint32 hitMask);

    void ModifyDamage(int32 amount);
    void AbsorbDamage(uint32 amount);
    void ResistDamage(uint32 amount);
    void BlockDamage(uint32 amount);

    Unit* GetAttacker() const { return m_attacker; }
    Unit* GetVictim() const { return m_victim; }
    SpellInfo const* GetSpellInfo() const { return m_spellInfo; }
    SpellSchoolMask GetSchoolMask() const { return m_schoolMask; }
    DamageEffectType GetDamageType() const { return m_damageType; }
    WeaponAttackType GetAttackType() const { return m_attackType; }
    uint32 GetDamage() const { return m_damage; }
    uint32 GetAbsorb() const { return m_absorb; }
    uint32 GetResist() const { return m_resist; }
    uint32 GetBlock() const { return m_block; }
    
    uint32 GetHitMask() const;
};

class TC_GAME_API HealInfo
{
private:
    Unit * const _healer;
    Unit* const _target;
    uint32 _heal;
    uint32 _effectiveHeal;
    uint32 _absorb; //always 0 on BC
    SpellInfo const* const _spellInfo;
    SpellSchoolMask const _schoolMask;
    uint32 _hitMask;

public:
    HealInfo(Unit* healer, Unit* target, uint32 heal, SpellInfo const* spellInfo, SpellSchoolMask schoolMask);

    //no effect on BC
    void AbsorbHeal(uint32 amount);
    void SetEffectiveHeal(uint32 amount) { _effectiveHeal = amount; }

    Unit* GetHealer() const { return _healer; }
    Unit* GetTarget() const { return _target; }
    uint32 GetHeal() const { return _heal; }
    uint32 GetEffectiveHeal() const { return _effectiveHeal; }
    uint32 GetAbsorb() const { return _absorb; }
    SpellInfo const* GetSpellInfo() const { return _spellInfo; };
    SpellSchoolMask GetSchoolMask() const { return _schoolMask; };

    uint32 GetHitMask() const;
};

class TC_GAME_API ProcEventInfo
{
    public:
        ProcEventInfo(Unit* actor, Unit* actionTarget, Unit* procTarget, uint32 typeMask,
                      uint32 spellTypeMask, uint32 spellPhaseMask, uint32 hitMask,
                      Spell* spell, DamageInfo* damageInfo, HealInfo* healInfo);

        Unit* GetActor() { return _actor; }
        Unit* GetActionTarget() const { return _actionTarget; }
        Unit* GetProcTarget() const { return _procTarget; }

        uint32 GetTypeMask() const { return _typeMask; }
        uint32 GetSpellTypeMask() const { return _spellTypeMask; }
        uint32 GetSpellPhaseMask() const { return _spellPhaseMask; }
        uint32 GetHitMask() const { return _hitMask; }

        SpellInfo const* GetSpellInfo() const;
        SpellSchoolMask GetSchoolMask() const;

        DamageInfo* GetDamageInfo() const { return _damageInfo; }
        HealInfo* GetHealInfo() const { return _healInfo; }

        Spell const* GetProcSpell() const { return _spell; }

    private:
        Unit* const _actor;
        Unit* const _actionTarget;
        Unit* const _procTarget;
        uint32 _typeMask;
        uint32 _spellTypeMask;
        uint32 _spellPhaseMask;
        uint32 _hitMask;
        Spell* _spell;
        DamageInfo* _damageInfo;
        HealInfo* _healInfo;
};

// Spell damage info structure based on structure sending in SMSG_SPELLNONMELEEDAMAGELOG opcode
struct SpellNonMeleeDamage
{
    SpellNonMeleeDamage(Unit *_attacker, Unit *_target, uint32 _SpellID, uint32 _schoolMask) :
    attacker(_attacker), target(_target), SpellID(_SpellID), damage(0), schoolMask(_schoolMask),
    absorb(0), resist(0), physicalLog(false), unused(false), blocked(0), HitInfo(0), cleanDamage(0),
    fullBlock(false), fullResist(false), fullAbsorb(false) 
    {}

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

    bool fullBlock;
#ifndef LICH_KING
    bool fullResist;
    bool fullAbsorb;
#endif
};

struct SpellPeriodicAuraLogInfo
{
    SpellPeriodicAuraLogInfo(AuraEffect const* _aura, uint32 _damage, uint32 _absorb, uint32 _resist, float _multiplier)
        : auraEff(_aura), damage(_damage), absorb(_absorb), resist(_resist), multiplier(_multiplier) { }

    AuraEffect const* auraEff;
    uint32 damage;
    uint32 absorb;
    uint32 resist;
    float  multiplier;
};

#define UNIT_ACTION_BUTTON_ACTION(X) (uint32(X) & 0x00FFFFFF)
#define UNIT_ACTION_BUTTON_TYPE(X)   ((uint32(X) & 0xFF000000) >> 24)
#define MAKE_UNIT_ACTION_BUTTON(A, T) (uint32(A) | (uint32(T) << 24))

enum ActiveStates
{
    ACT_PASSIVE  = 0x01,                                    // 0x01 - passive
    ACT_DISABLED = 0x81,                                    // 0x80 - castable
    ACT_ENABLED  = 0xC1,                                    // 0x40 | 0x80 - auto cast + castable
    ACT_COMMAND  = 0x07,                                    // 0x01 | 0x02 | 0x04
    ACT_REACTION = 0x06,                                    // 0x02 | 0x04
    ACT_DECIDE   = 0x00                                     // custom
};

enum ReactStates
{
    REACT_PASSIVE = 0,
    REACT_DEFENSIVE = 1,
    REACT_AGGRESSIVE = 2
};

enum CommandStates
{
    COMMAND_STAY = 0,
    COMMAND_FOLLOW = 1,
    COMMAND_ATTACK = 2,
    COMMAND_ABANDON = 3
};

struct UnitActionBarEntry
{
    UnitActionBarEntry();

    uint32 packedData;

    // helper
    ActiveStates GetType() const;
    uint32 GetAction() const;
    bool IsActionBarForSpell() const;

    void SetActionAndType(uint32 action, ActiveStates type);

    void SetType(ActiveStates type);

    void SetAction(uint32 action);
};

#define MAX_DECLINED_NAME_CASES 5

struct DeclinedName
{
    std::string name[MAX_DECLINED_NAME_CASES];
};

uint32 createProcHitMask(SpellNonMeleeDamage* damageInfo, SpellMissInfo missCondition);

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

typedef UnitActionBarEntry CharmSpellInfo;

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

typedef std::list<Player*> SharedVisionList;

enum CharmType
{
    CHARM_TYPE_CHARM,
    CHARM_TYPE_POSSESS,
    CHARM_TYPE_VEHICLE,
    CHARM_TYPE_CONVERT
};

enum ActionBarIndex
{
	ACTION_BAR_INDEX_START = 0,
	ACTION_BAR_INDEX_PET_SPELL_START = 3,
	ACTION_BAR_INDEX_PET_SPELL_END = 7,
	ACTION_BAR_INDEX_END = 10
};

#define MAX_UNIT_ACTION_BAR_INDEX (ACTION_BAR_INDEX_END-ACTION_BAR_INDEX_START)

class TC_GAME_API CharmInfo
{
    public:
        explicit CharmInfo(Unit* unit);
        ~CharmInfo();
        uint32 GetPetNumber() const { return _petnumber; }
        void SetPetNumber(uint32 petnumber, bool statwindow);

        void SetCommandState(CommandStates st) { _CommandState = st; }
        CommandStates GetCommandState() { return _CommandState; }
        bool HasCommandState(CommandStates state) { return (_CommandState == state); }

        void InitPossessCreateSpells();
        void InitCharmCreateSpells();
        void InitPetActionBar();
        void InitEmptyActionBar(bool withAttack = true);

                                                            //return true if successful
        bool AddSpellToActionBar(SpellInfo const* spellInfo, ActiveStates newstate = ACT_DECIDE, uint8 preferredSlot = 0);
        bool RemoveSpellFromActionBar(uint32 spell_id);
		void LoadPetActionBar(const std::string& data);
        void BuildActionBar(WorldPacket* data);
        void SetSpellAutocast(SpellInfo const* spellInfo, bool state);
        void SetActionBar(uint8 index, uint32 spellOrAction, ActiveStates type)
        {
            PetActionBar[index].SetActionAndType(spellOrAction, type);
        }
        UnitActionBarEntry const* GetActionBarEntry(uint8 index) const { return &(PetActionBar[index]); }
        void ToggleCreatureAutocast(SpellInfo const* spellInfo, bool apply);
        CharmSpellInfo* GetCharmSpell(uint8 index) { return &(_charmspells[index]); }

		void SetIsCommandAttack(bool val);
		bool IsCommandAttack();
		void SetIsCommandFollow(bool val);
		bool IsCommandFollow();
		void SetIsAtStay(bool val);
		bool IsAtStay();
		void SetIsFollowing(bool val);
		bool IsFollowing();
		void SetIsReturning(bool val);
		bool IsReturning();
		void SaveStayPosition();
		void GetStayPosition(float &x, float &y, float &z);

    private:
        Unit* _unit;
        UnitActionBarEntry PetActionBar[MAX_UNIT_ACTION_BAR_INDEX];
        CharmSpellInfo _charmspells[4];
        CommandStates   _CommandState;
        uint32          _petnumber;

        //for restoration after charmed
		ReactStates     _oldReactState;
			
		bool _isCommandAttack;
		bool _isCommandFollow;
		bool _isAtStay;
		bool _isFollowing;
		bool _isReturning;
		float _stayX;
		float _stayY;
		float _stayZ;
};

// for clearing special attacks
#define REACTIVE_TIMER_START 4000

enum ReactiveType
{
    REACTIVE_DEFENSE        = 1,
    REACTIVE_HUNTER_PARRY   = 2,
    REACTIVE_CRIT           = 3,
    REACTIVE_HUNTER_CRIT    = 4,
    REACTIVE_OVERPOWER      = 5,
#ifdef LICH_KING
    REACTIVE_WOLVERINE_BITE = 6,
#endif
    MAX_REACTIVE
};

#define SUMMON_SLOT_PET     0
#define SUMMON_SLOT_TOTEM   1
#define MAX_TOTEM_SLOT      5
#define SUMMON_SLOT_MINIPET 5
#define SUMMON_SLOT_QUEST   6
#define MAX_TOTEM 4
#define MAX_SUMMON_SLOT     7

#define MAX_GAMEOBJECT_SLOT 4

// delay time next attack to prevent client attack animation problems
#define ATTACK_DISPLAY_DELAY 200

#define MAX_PLAYER_STEALTH_DETECT_RANGE 45.0f               // max distance for detection targets by player
/* distance in yards at which a creature will be in "warning" mode, look in direction of the player without attacking
Not sure about this but: Creatures should only warn players, and bosses shouldn't warn at all. 
Only on units hostile to players and able to attack him.
*/
//#define STEALTH_DETECT_WARNING_RANGE 3.0f   
//if in warning range, we can do the suspicious look.
//time in ms between two warning, counting from warning start (= ignoring duration)
#define STEALTH_ALERT_COOLDOWN 16000
#define STEALTH_ALERT_DURATINON 5000
// STEALTH_ALERT_COOLDOWN must always be greater than STEALTH_ALERT_DURATINON

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
	CAN_ATTACK_RESULT_CANNOT_DETECT,
    //CAN_ATTACK_RESULT_CANNOT_DETECT_INVI, //target cannot be detected because it's invisible to us
    //CAN_ATTACK_RESULT_CANNOT_DETECT_STEALTH, //target cannot be detected because it's stealthed from us
    //CAN_ATTACK_RESULT_CANNOT_DETECT_STEALTH_ALERT_RANGE, //target cannot be detected because it's stealthed from us but is in warn range
    CAN_ATTACK_RESULT_SELF_EVADE, //creature is currently evading
    CAN_ATTACK_RESULT_TARGET_EVADE, //target is a creature in evade mode
    CAN_ATTACK_RESULT_SELF_UNIT_FLAGS, //create cannot attack because of own unit flags
    CAN_ATTACK_RESULT_IMMUNE_TO_TARGET, //Combat disabled with target because of own flags
    CAN_ATTACK_RESULT_OTHER_MAP,
    CAN_ATTACK_RESULT_OTHERS, //all others reason
};

enum PlayerTotemType
{
    SUMMON_TYPE_TOTEM_FIRE  = 63,
    SUMMON_TYPE_TOTEM_EARTH = 81,
    SUMMON_TYPE_TOTEM_WATER = 82,
    SUMMON_TYPE_TOTEM_AIR   = 83
};

struct SpellProcEntry;                                 // used only privately

class TC_GAME_API Unit : public WorldObject
{
    public:
        typedef std::set<Unit*> AttackerSet;
        typedef std::vector<Unit*> UnitVector;
		typedef std::set<Unit*> ControlList;

        //typedef std::pair<uint32, uint8 /*effIndex*/> spellEffectPair;
        typedef std::multimap<uint32, Aura*> AuraMap;
        typedef std::pair<AuraMap::const_iterator, AuraMap::const_iterator> AuraMapBounds;
        typedef std::pair<AuraMap::iterator, AuraMap::iterator> AuraMapBoundsNonConst;

        typedef std::multimap<uint32, AuraApplication*> AuraApplicationMap;
        typedef std::pair<AuraApplicationMap::const_iterator, AuraApplicationMap::const_iterator> AuraApplicationMapBounds;
        typedef std::pair<AuraApplicationMap::iterator, AuraApplicationMap::iterator> AuraApplicationMapBoundsNonConst;

        typedef std::multimap<AuraStateType, AuraApplication*> AuraStateAurasMap;
        typedef std::pair<AuraStateAurasMap::const_iterator, AuraStateAurasMap::const_iterator> AuraStateAurasMapBounds;

        typedef std::list<AuraEffect*> AuraEffectList;
        typedef std::list<Aura*> AuraList;
        typedef std::list<AuraApplication *> AuraApplicationList;

        typedef std::vector<std::pair<uint8 /*procEffectMask*/, AuraApplication*>> AuraApplicationProcContainer;

        typedef std::map<uint8, AuraApplication*> VisibleAuraMap;

        typedef std::list<DiminishingReturn> Diminishing;
        typedef std::set<AuraType> AuraTypeSet;
        typedef std::set<uint32> ComboPointHolderSet;

        ~Unit ( ) override;

        void AddToWorld() override;
        void RemoveFromWorld() override;

        void CleanupBeforeRemoveFromMap(bool finalCleanup);
        void CleanupsBeforeDelete(bool finalCleanup = true) override;  // used in ~Creature/~Player (or before mass creature delete to remove cross-references to already deleted units)

        UnitAI* GetAI() { return i_AI; }

        virtual bool IsAffectedByDiminishingReturns() const { return (GetCharmerOrOwnerPlayerOrPlayerItself() != nullptr); }
        DiminishingLevels GetDiminishing(DiminishingGroup group) const;
        void IncrDiminishing(SpellInfo const* auraSpellInfo, bool triggered);
        bool ApplyDiminishingToDuration(SpellInfo const* auraSpellInfo, bool triggered, int32 &duration, Unit* caster, DiminishingLevels previousLevel) const;
        void ApplyDiminishingAura(DiminishingGroup  group, bool apply);
        void ClearDiminishings();

        // target dependent range checks
        float GetSpellMaxRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const;
        float GetSpellMinRangeForTarget(Unit const* target, SpellInfo const* spellInfo) const;

        void Update( uint32 time ) override;

        void SetAttackTimer(WeaponAttackType type, uint32 time) { m_attackTimer[type] = time; }
        void ResetAttackTimer(WeaponAttackType type = BASE_ATTACK);
        uint32 GetAttackTimer(WeaponAttackType type) const { return m_attackTimer[type]; }
        bool IsAttackReady(WeaponAttackType type = BASE_ATTACK) const { return m_attackTimer[type] == 0; }
        bool HaveOffhandWeapon() const;
        bool CanDualWield() const { return m_canDualWield; }
        void SetCanDualWield(bool value) { m_canDualWield = value; }
        float GetCombatReach() const override { return m_floatValues[UNIT_FIELD_COMBATREACH]; }
        bool IsWithinCombatRange(Unit const* obj, float dist2compare) const;
        bool IsWithinMeleeRange(Unit const* obj, float dist = MELEE_RANGE) const;
        float GetMeleeRange(Unit const* target) const;
        virtual SpellSchoolMask GetMeleeDamageSchoolMask(WeaponAttackType attackType = BASE_ATTACK, uint8 damageIndex = 0) const = 0;
        void GetRandomContactPoint( const Unit* target, float &x, float &y, float &z, float distance2dMin, float distance2dMax ) const;
        uint32 m_extraAttacks;
        bool m_canDualWield;
        
        void StartAutoRotate(uint8 type, uint32 fulltime, double Angle = 0, bool attackVictimOnEnd = true);
        void AutoRotate(uint32 time);
        bool IsUnitRotating() {return IsRotating;}

        void _addAttacker(Unit* pAttacker);                  // must be called only from Unit::Attack(Unit*)
        void _removeAttacker(Unit* pAttacker);               // must be called only from Unit::AttackStop()
        Unit* GetAttackerForHelper() const;                 // If someone wants to help, who to give them

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
        
        void ValidateAttackersAndOwnTarget();
        void CombatStop(bool includingCast = false, bool mutualPvP = true);
        void CombatStopWithPets(bool includingCast = false);
        Unit* SelectNearbyTarget(Unit* exclude = nullptr, float dist = NOMINAL_MELEE_RANGE) const;

        void AddUnitState(uint32 f) { m_state |= f; }
        bool HasUnitState(const uint32 f) const { return (m_state & f); }
        void ClearUnitState(uint32 f) { m_state &= ~f; }
        bool CanFreeMove() const;

        uint32 HasUnitTypeMask(uint32 mask) const { return mask & m_unitTypeMask; }
        void AddUnitTypeMask(uint32 mask) { m_unitTypeMask |= mask; }
        bool IsSummon() const   { return (m_unitTypeMask & UNIT_MASK_SUMMON) != 0; }
        bool IsGuardian() const { return (m_unitTypeMask & UNIT_MASK_GUARDIAN) != 0; }
        bool IsPet() const      { return (m_unitTypeMask & UNIT_MASK_PET) != 0; }
        bool IsHunterPet() const{ return (m_unitTypeMask & UNIT_MASK_HUNTER_PET) != 0; }
        bool IsTotem() const    { return (m_unitTypeMask & UNIT_MASK_TOTEM) != 0; }
        bool IsVehicle() const  { return (m_unitTypeMask & UNIT_MASK_VEHICLE) != 0; }

        //Pet* SummonPet(uint32 entry, float x, float y, float z, float ang, uint32 despwtime);

        uint32 GetLevel() const { return GetUInt32Value(UNIT_FIELD_LEVEL); }
        virtual uint8 GetLevelForTarget(WorldObject const* /*target*/) const override { return GetLevel(); }
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
        static float CalculateAverageResistReduction(Unit const* attacker, SpellSchoolMask schoolMask, Unit const* victim, SpellInfo const* spellInfo = nullptr);

        uint32 GetHealth()    const { return GetUInt32Value(UNIT_FIELD_HEALTH); }
        uint32 GetMaxHealth() const { return GetUInt32Value(UNIT_FIELD_MAXHEALTH); }

        bool IsFullHealth() const { return GetHealth() == GetMaxHealth(); }
        bool HealthBelowPct(int32 pct) const { return GetHealth() < CountPctFromMaxHealth(pct); }
        bool HealthBelowPctDamaged(int32 pct, uint32 damage) const { return int64(GetHealth()) - int64(damage) < int64(CountPctFromMaxHealth(pct)); }
        bool HealthAbovePct(int32 pct) const { return GetHealth() > CountPctFromMaxHealth(pct); }
        bool HealthAbovePctHealed(int32 pct, uint32 heal) const { return uint64(GetHealth()) + uint64(heal) > CountPctFromMaxHealth(pct); }
        float GetHealthPct() const { return GetMaxHealth() ? 100.f * GetHealth() / GetMaxHealth() : 0.0f; }
        //get X% heath from max value
        float CountPctFromMaxHealth(float pct) const;
        uint32 CountPctFromMaxHealth(int32 pct) const;
        float CountPctFromCurHealth(float pct) const;
        uint32 CountPctFromCurHealth(int32 pct) const;

        void SetHealth(   uint32 val);
        void SetMaxHealth(uint32 val);
        inline void SetFullHealth() { SetHealth(GetMaxHealth()); }
        int32 ModifyHealth(int32 val);
        int32 GetHealthGain(int32 dVal);

        // Testing
        void DisableRegeneration(bool set) { m_disabledRegen = set; }

        Powers GetPowerType() const { return Powers(GetByteValue(UNIT_FIELD_BYTES_0, 3)); }
        void SetPowerType(Powers power);
        void UpdateDisplayPower();
        uint32 GetPower(   Powers power) const { return GetUInt32Value(UNIT_FIELD_POWER1   +power); }
        uint32 GetMaxPower(Powers power) const { return GetUInt32Value(UNIT_FIELD_MAXPOWER1+power); }
        void SetPower(   Powers power, uint32 val);
        void SetMaxPower(Powers power, uint32 val); 
        inline void SetFullPower(Powers power) { SetPower(power, GetMaxPower(power)); }
        int32 ModifyPower(Powers power, int32 val);
        int32 ModifyPowerPct(Powers power, float pct, bool apply = true);
        void ApplyPowerMod(Powers power, uint32 val, bool apply);
        void ApplyMaxPowerMod(Powers power, uint32 val, bool apply);

        uint32 GetAttackTime(WeaponAttackType att) const { return (uint32)(GetFloatValue(UNIT_FIELD_BASEATTACKTIME+att)/m_modAttackSpeedPct[att]); }
        void SetAttackTime(WeaponAttackType att, uint32 val) { SetFloatValue(UNIT_FIELD_BASEATTACKTIME+att,val*m_modAttackSpeedPct[att]); }
        void ApplyAttackTimePercentMod(WeaponAttackType att,float val, bool apply);
        void ApplyCastTimePercentMod(float val, bool apply);
        void HandleParryRush();
        
        SheathState GetSheath() const { return SheathState(GetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHEATH_STATE)); }
        virtual void SetSheath( SheathState sheathed ) { SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_SHEATH_STATE, sheathed); }

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

        uint8 GetStandState() const { return GetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_STAND_STATE); }
        bool IsSitState() const;
        bool IsStandState() const;
        void SetStandState(uint8 state);

        void  SetStandFlags(uint8 flags) { SetByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, flags); }
        void  RemoveStandFlags(uint8 flags) { RemoveByteFlag(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_VIS_FLAG, flags); }

        bool IsMounted() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT ); }
        uint32 GetMountID() const { return GetUInt32Value(UNIT_FIELD_MOUNTDISPLAYID); }
        void Mount(uint32 mount, bool flying = false);
        void Dismount();

        uint16 GetMaxSkillValueForLevel(Unit const* target = nullptr) const { return (target ? GetLevelForTarget(target) : GetLevel()) * 5; }
        void RemoveSpellbyDamageTaken(uint32 damage, uint32 spell);
        static void DealDamageMods(Unit const* victim, uint32& damage, uint32* absorb);
        static uint32 DealDamage(Unit* attacker, Unit *pVictim, uint32 damage, CleanDamage const* cleanDamage = nullptr, DamageEffectType damagetype = DIRECT_DAMAGE, SpellSchoolMask damageSchoolMask = SPELL_SCHOOL_MASK_NORMAL, SpellInfo const *spellProto = nullptr, bool durabilityLoss = true);
        static void Kill(Unit* attacker, Unit* victim, bool durabilityLoss = true);
        void KillSelf(bool durabilityLoss = true) { Unit::Kill(this, this, durabilityLoss); }
        static void DealHeal(HealInfo& healInfo);

        static void ProcSkillsAndAuras(Unit* actor, Unit* actionTarget, uint32 typeMaskActor, uint32 typeMaskActionTarget,
            uint32 spellTypeMask, uint32 spellPhaseMask, uint32 hitMask, Spell* spell,
            DamageInfo* damageInfo, HealInfo* healInfo);

        void GetProcAurasTriggeredOnEvent(AuraApplicationProcContainer& aurasTriggeringProc, AuraApplicationList* procAuras, ProcEventInfo& eventInfo);
        void TriggerAurasProcOnEvent(Unit* actionTarget, uint32 typeMaskActor, uint32 typeMaskActionTarget,
            uint32 spellTypeMask, uint32 spellPhaseMask, uint32 hitMask, Spell* spell,
            DamageInfo* damageInfo, HealInfo* healInfo);
        void TriggerAurasProcOnEvent(ProcEventInfo& eventInfo, AuraApplicationProcContainer& procAuras);

        void SetEmoteState(uint32 state);
        void HandleEmoteCommand(uint32 anim_id);
        void AttackerStateUpdate (Unit *pVictim, WeaponAttackType attType = BASE_ATTACK, bool extra = false );
        //float MeleeMissChanceCalc(const Unit *pVictim, WeaponAttackType attType) const;

        //used for white damage calculation
        void CalculateMeleeDamage(Unit *pVictim, CalcDamageInfo *damageInfo, WeaponAttackType attackType = BASE_ATTACK);
        void DealMeleeDamage(CalcDamageInfo *damageInfo, bool durabilityLoss);

        void CalculateSpellDamageTaken(SpellNonMeleeDamage *damageInfo, int32 damage, SpellInfo const *spellInfo, WeaponAttackType attackType = BASE_ATTACK, bool crit = false);
        void DealSpellDamage(SpellNonMeleeDamage *damageInfo, bool durabilityLoss);

        float MeleeSpellMissChance(const Unit *pVictim, WeaponAttackType attType, int32 skillDiff, uint32 spellId) const;
        SpellMissInfo MeleeSpellHitResult(Unit *pVictim, SpellInfo const *spell);
        SpellMissInfo MagicSpellHitResult(Unit *pVictim, SpellInfo const *spell, Item* castItem = nullptr);
        SpellMissInfo SpellHitResult(Unit *pVictim, SpellInfo const *spell, bool canReflect = false, Item* castItem = nullptr);
#ifdef TESTS
        SpellMissInfo _forceHitResult = SPELL_MISS_TOTAL;
        void ForceSpellHitResult(SpellMissInfo missInfo) { _forceHitResult = missInfo; }
        void ResetForceSpellHitResult() { _forceHitResult = SpellMissInfo(-1); }

        MeleeHitOutcome _forceMeleeResult = MeleeHitOutcome(-1);
        void ForceMeleeHitResult(MeleeHitOutcome outcome) { _forceMeleeResult = outcome; }
        void ResetForceMeleeHitResult() { _forceMeleeResult = MELEE_HIT_TOTAL; }
#endif

        float GetUnitDodgeChance(WeaponAttackType attType, Unit const* victim) const;
        float GetUnitParryChance(WeaponAttackType attType, Unit const* victim) const;
        float GetUnitBlockChance(WeaponAttackType attType, Unit const* victim) const;
        float GetUnitMissChance() const;
        float GetUnitCriticalChanceDone(WeaponAttackType attackType) const;
        float GetUnitCriticalChanceTaken(Unit const* attacker, WeaponAttackType attackType, float critDone) const;
        float GetUnitCriticalChanceAgainst(WeaponAttackType attackType, Unit const* victim) const;
        int32 GetMechanicResistChance(const SpellInfo *spell);
        bool CanUseAttackType(uint8 attacktype) const;

        virtual uint32 GetShieldBlockValue() const = 0;
        uint32 GetDefenseSkillValue(Unit const* target = nullptr) const;
        uint32 GetWeaponSkillValue(WeaponAttackType attType, Unit const* target = nullptr) const;
        float GetWeaponProcChance() const;
        float GetPPMProcChance(uint32 WeaponSpeed, float PPM, SpellInfo const* spellProto) const;

        //used to check white attack outcome
        MeleeHitOutcome RollMeleeOutcomeAgainst(const Unit *pVictim, WeaponAttackType attType) const;

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

        /// ====================== THREAT & COMBAT ====================
        bool CanHaveThreatList() const { return m_threatManager.CanHaveThreatList(); }
        // For NPCs with threat list: Whether there are any enemies on our threat list
        // For other units: Whether we're in combat
        // This value is different from IsInCombat when a projectile spell is midair (combat on launch - threat+aggro on impact)
        bool IsEngaged() const { return CanHaveThreatList() ? m_threatManager.IsEngaged() : IsInCombat(); }
        bool IsEngagedBy(Unit const* who) const { return CanHaveThreatList() ? IsThreatenedBy(who) : IsInCombatWith(who); }
        void EngageWithTarget(Unit* who); // Adds target to threat list if applicable, otherwise just sets combat state
                                          // Combat handling
        CombatManager& GetCombatManager() { return m_combatManager; }
        CombatManager const& GetCombatManager() const { return m_combatManager; }
        void AttackedTarget(Unit* target, bool canInitialAggro);

        bool IsImmuneToAll() const { return IsImmuneToPC() && IsImmuneToNPC(); }
        void SetImmuneToAll(bool apply, bool keepCombat);
        virtual void SetImmuneToAll(bool apply) { SetImmuneToAll(apply, false); }
        bool IsImmuneToPC() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC); }
        void SetImmuneToPC(bool apply, bool keepCombat);
        virtual void SetImmuneToPC(bool apply) { SetImmuneToPC(apply, false); }
        bool IsImmuneToNPC() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC); }
        void SetImmuneToNPC(bool apply, bool keepCombat);
        virtual void SetImmuneToNPC(bool apply) { SetImmuneToNPC(apply, false); }

        bool IsInCombat() const { return HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT); }
        bool IsInCombatWith(Unit const* who) const { return who && m_combatManager.IsInCombatWith(who); }
        void SetInCombatWith(Unit* enemy) { if (enemy) m_combatManager.SetInCombatWith(enemy); }
        void ClearInCombat() { m_combatManager.EndAllCombat(); }
        void UpdatePetCombatState();
        // Threat handling
        bool IsThreatened() const;
        bool IsThreatenedBy(Unit const* who) const { return who && m_threatManager.IsThreatenedBy(who, true); }
        // Exposes the threat manager directly - be careful when interfacing with this
        // As a general rule of thumb, any unit pointer MUST be null checked BEFORE passing it to threatmanager methods
        // threatmanager will NOT null check your pointers for you - misuse = crash
        ThreatManager& GetThreatManager() { return m_threatManager; }
        ThreatManager const& GetThreatManager() const { return m_threatManager; }

#ifdef LICH_KING
        void SendClearTarget();
        void SendThreatListUpdate() { m_threatManager.SendThreatListToClients(); }
#endif
        bool HasAuraTypeWithFamilyFlags(AuraType auraType, uint32 familyName, uint64 familyFlags) const;
        bool virtual HasSpell(uint32 /*spellID*/) const { return false; }
        bool HasBreakableByDamageAuraType(AuraType type, uint32 excludeAura = 0) const;
        bool HasBreakableByDamageCrowdControlAura(Unit* excludeCasterChannel = nullptr) const;

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

        bool IsTargetableForAttack(bool checkFakeDeath = true) const;

        bool IsValidAttackTarget(Unit const* target) const;
        bool _IsValidAttackTarget(Unit const* target, SpellInfo const* bySpell, WorldObject const* obj = nullptr) const;

        bool IsValidAssistTarget(Unit const* target) const;
        bool _IsValidAssistTarget(Unit const* target, SpellInfo const* bySpell) const;


        virtual bool IsInWater() const;
        virtual bool IsUnderWater() const;
        bool isInAccessiblePlaceFor(Creature const* c) const;
        
        void SetFullTauntImmunity(bool apply);

        void SendHealSpellLog(HealInfo& healInfo, bool critical = false);
        int32 HealBySpell(HealInfo& healInfo, bool critical = false, SpellMissInfo missInfo = SPELL_MISS_NONE);
        void SendEnergizeSpellLog(Unit *pVictim, uint32 SpellID, uint32 Damage, Powers powertype);
        void EnergizeBySpell(Unit* victim, uint32 spellId, int32 damage, Powers powerType);
        void EnergizeBySpell(Unit* victim, SpellInfo const* spellInfo, int32 damage, Powers powerType, bool sendLog = true);

        // CastSpell's third arg can be a variety of things - check out CastSpellExtraArgs' constructors! Returns SpellCastResult
        uint32 CastSpell(SpellCastTargets const& targets, uint32 spellId, CastSpellExtraArgs const& args = {});
        uint32 CastSpell(WorldObject* target, uint32 spellId, CastSpellExtraArgs const& args = {});
        uint32 CastSpell(Position const& dest, uint32 spellId, CastSpellExtraArgs const& args = {});
        Aura* AddAura(uint32 spellId, Unit* target);
        Aura* AddAura(SpellInfo const* spellInfo, uint8 effMask, Unit* target);
        void SetAuraStack(uint32 spellId, Unit* target, uint32 stack);
        void SendPlaySpellVisual(uint32 id);
#ifdef LICH_KING
        void SendPlaySpellImpact(ObjectGuid guid, uint32 id);
#endif

        void LogBossDown(Creature* victim);

        static bool IsDamageToThreatSpell(SpellInfo const * spellInfo);

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
		void NearTeleportTo(Position const& pos, bool casting = false);
		void NearTeleportTo(float x, float y, float z, float orientation, bool casting = false) { NearTeleportTo(Position(x, y, z, orientation), casting); }
        void SendTeleportPacket(Position const& pos, bool teleportingTransport = false);
        virtual bool UpdatePosition(float x, float y, float z, float ang, bool teleport = false);
        // returns true if unit's position really changed
        virtual bool UpdatePosition(const Position &pos, bool teleport = false);
        void UpdateOrientation(float orientation);
        void UpdateHeight(float newZ);

        void UpdateSplineMovement(uint32 t_diff);
        void UpdateSplinePosition();
        void DisableSpline();

        void ProcessPositionDataChanged(PositionFullTerrainStatus const& data, bool updateCreatureLiquid = false) override;
        virtual void ProcessTerrainStatusUpdate(ZLiquidStatus status, Optional<LiquidData> const& liquidData, bool forceCreature = false);

        virtual void AtEnterCombat() { }
        virtual void AtExitCombat();

		bool IsAlwaysVisibleFor(WorldObject const* seer) const override;
		bool IsAlwaysDetectableFor(WorldObject const* seer) const override;

        void KnockbackFrom(float x, float y, float speedXY, float speedZ);
        void JumpTo(float speedXY, float speedZ, bool forward = true);
        void JumpTo(WorldObject* obj, float speedZ);

        void MonsterMoveWithSpeed(float x, float y, float z, float speed, bool generatePath = false, bool forceDestination = false);
        void SendMovementFlagUpdate();

        bool IsAlive() const { return (m_deathState == ALIVE); };
        bool IsDying() const { return (m_deathState == JUST_DIED); };
        bool IsDead() const { return ( m_deathState == DEAD || m_deathState == CORPSE ); };
        DeathState GetDeathState() { return m_deathState; };
        virtual void SetDeathState(DeathState s);           // overwrited in Creature/Player/Pet

        ObjectGuid GetOwnerGUID() const { return  GetGuidValue(UNIT_FIELD_SUMMONEDBY); }
        void SetOwnerGUID(ObjectGuid owner);
        ObjectGuid GetCreatorGUID() const { return GetGuidValue(UNIT_FIELD_CREATEDBY); }
        /** Set a "X's minion" text on the creature */
        void SetCreatorGUID(ObjectGuid creator) { SetGuidValue(UNIT_FIELD_CREATEDBY, creator); }
        ObjectGuid GetMinionGUID() const { return GetGuidValue(UNIT_FIELD_SUMMON); }
		void SetMinionGUID(ObjectGuid guid) { SetGuidValue(UNIT_FIELD_SUMMON, guid); }
        ObjectGuid GetCharmerGUID() const { return GetGuidValue(UNIT_FIELD_CHARMEDBY); }
        void SetCharmerGUID(ObjectGuid owner) { SetGuidValue(UNIT_FIELD_CHARMEDBY, owner); }
        ObjectGuid GetCharmGUID() const { return  GetGuidValue( UNIT_FIELD_CHARM); }
		void SetPetGUID(ObjectGuid guid) { m_SummonSlot[SUMMON_SLOT_PET] = guid; }
        ObjectGuid GetPetGUID() const { return m_SummonSlot[SUMMON_SLOT_PET]; }
#ifdef LICH_KING
		void SetCritterGUID(ObjectGuid guid) { SetGuidValue(UNIT_FIELD_CRITTER, guid); }
        ObjectGuid GetCritterGUID() const { return GetGuidValue(UNIT_FIELD_CRITTER);
#else
		void SetCritterGUID(ObjectGuid guid) { m_miniPet = guid; }
        ObjectGuid GetCritterGUID() const { return m_miniPet; }
#endif

        bool IsControlledByPlayer() const { return m_ControlledByPlayer; }

        ObjectGuid GetCharmerOrOwnerGUID() const { return GetCharmerGUID() ? GetCharmerGUID() : GetOwnerGUID(); }
        ObjectGuid GetCharmerOrOwnerOrOwnGUID() const
        {
            if(ObjectGuid guid = GetCharmerOrOwnerGUID())
                return guid;
            return GetGUID();
        }
        bool isCharmedOwnedByPlayerOrPlayer() const;

        Player* GetSpellModOwner() const;

        Unit* GetOwner() const;
        Pet* GetPet() const;
		Guardian* GetGuardianPet() const;
		Minion* GetFirstMinion() const;
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
        bool IsCharmedOwnedByPlayerOrPlayer() const;

		void SetMinion(Minion *minion, bool apply);
		void GetAllMinionsByEntry(std::list<Creature*>& Minions, uint32 entry);
		void RemoveAllMinionsByEntry(uint32 entry);
        //void SetPet(Pet* pet);
		void SetCharm(Unit* target, bool apply);
		bool SetCharmedBy(Unit* charmer, CharmType type, AuraApplication const* aurApp = NULL);
        void RemoveCharmedBy(Unit* charmer);
        void RestoreFaction();

        Unit* GetNextRandomRaidMemberOrPet(float radius);

		ControlList m_Controlled;
		Unit* GetFirstControlled() const;
		void RemoveAllControlled();
        ObjectGuid m_miniPet;

        bool IsCharmed() const { return GetCharmerGUID() != 0; }
        bool IsPossessed() const { return HasUnitState(UNIT_STATE_POSSESSED); }
        bool IsPossessedByPlayer() const;
        bool IsPossessing() const;
        bool IsPossessing(Unit* u) const;

        CharmInfo* GetCharmInfo() { return m_charmInfo; }
        CharmInfo* InitCharmInfo();
        void       DeleteCharmInfo();
        void UpdateCharmAI();
        // returns the unit that this player IS CONTROLLING
        Unit* GetUnitBeingMoved() const;
        // returns the player that this player IS CONTROLLING
        Player* GetPlayerBeingMoved() const;
        // returns the player that this unit is BEING CONTROLLED BY
        Player* GetPlayerMovingMe() const { return m_playerMovingMe; }
        // only set for direct client control (possess effects, vehicles and similar)
        Player* m_playerMovingMe;
        SharedVisionList const& GetSharedVisionList() { return m_sharedVision; }
        void AddPlayerToVision(Player* plr);
        void RemovePlayerFromVision(Player* plr);
        bool HasSharedVision() const { return !m_sharedVision.empty(); }
        void RemoveBindSightAuras();
        void RemoveCharmAuras(Aura* except = nullptr);

        Pet* CreateTamedPetFrom(Creature* creatureTarget, uint32 spell_id = 0);
        Pet* CreateTamedPetFrom(uint32 creatureEntry, uint32 spell_id = 0);
        bool InitTamedPet(Pet* pet, uint8 level, uint32 spell_id);

        /*
        bool AddAura(Aura *aur);
        */

        // aura apply/remove helpers - you should better not use these
        Aura* _TryStackingOrRefreshingExistingAura(SpellInfo const* newAura, uint8 effMask, Unit* caster, int32* baseAmount = nullptr, Item* castItem = nullptr, ObjectGuid casterGUID = ObjectGuid::Empty, bool resetPeriodicTimer = true);
        void _AddAura(UnitAura* aura, Unit* caster);
        AuraApplication * _CreateAuraApplication(Aura* aura, uint8 effMask);
        void _ApplyAuraEffect(Aura* aura, uint8 effIndex);
        void _ApplyAura(AuraApplication * aurApp, uint8 effMask);
        void _UnapplyAura(AuraApplicationMap::iterator &i, AuraRemoveMode removeMode);
        void _UnapplyAura(AuraApplication * aurApp, AuraRemoveMode removeMode);
        void _RemoveNoStackAurasDueToAura(Aura* aura, bool checkStrongerAura = false);
        void _RegisterAuraEffect(AuraEffect* aurEff, bool apply);

        // m_ownedAuras container management
        AuraMap      & GetOwnedAuras() { return m_ownedAuras; }
        AuraMap const& GetOwnedAuras() const { return m_ownedAuras; }

        void RemoveOwnedAura(AuraMap::iterator &i, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveOwnedAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, uint8 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveOwnedAura(Aura* aura, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);

        Aura* GetOwnedAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint8 reqEffMask = 0, Aura* except = nullptr) const;

        // m_appliedAuras container management
        AuraApplicationMap      & GetAppliedAuras() { return m_appliedAuras; }
        AuraApplicationMap const& GetAppliedAuras() const { return m_appliedAuras; }

        void RemoveAura(AuraApplicationMap::iterator &i, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, uint8 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(AuraApplication * aurApp, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAura(Aura* aur, AuraRemoveMode mode = AURA_REMOVE_BY_DEFAULT);
        void RemoveSingleAuraFromStack(uint32 spellId);

        // Convenience methods removing auras by predicate
        void RemoveAppliedAuras(std::function<bool(AuraApplication const*)> const& check);
        void RemoveOwnedAuras(std::function<bool(Aura const*)> const& check);

        // Optimized overloads taking advantage of map key
        void RemoveAppliedAuras(uint32 spellId, std::function<bool(AuraApplication const*)> const& check);
        void RemoveOwnedAuras(uint32 spellId, std::function<bool(Aura const*)> const& check);

        void RemoveAurasByType(AuraType auraType, std::function<bool(AuraApplication const*)> const& check);

        void RemoveAurasDueToSpell(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, uint8 reqEffMask = 0, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAuraFromStack(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, AuraRemoveMode removeMode = AURA_REMOVE_BY_DEFAULT);
        void RemoveAurasDueToSpellByDispel(uint32 spellId, uint32 dispellerSpellId, ObjectGuid casterGUID, Unit* dispeller, uint8 chargesRemoved = 1);
        void RemoveAurasDueToSpellBySteal(uint32 spellId, ObjectGuid casterGUID, Unit* stealer);
        void RemoveAurasDueToItemSpell(uint32 spellId, ObjectGuid castItemGuid);
        void RemoveAurasByType(AuraType auraType, ObjectGuid casterGUID = ObjectGuid::Empty, Aura* except = nullptr, bool negative = true, bool positive = true);
        void RemoveNotOwnSingleTargetAuras(uint32 newPhase = 0x0);
        void RemoveAurasWithInterruptFlags(uint32 flag, uint32 except = 0, bool withChanneled = true);
        void RemoveAurasWithAttribute(uint32 flags); //only Attr0
        void RemoveAurasWithCustomAttribute(SpellCustomAttributes flags);
#ifdef LICH_KING
        void RemoveAurasWithFamily(SpellFamilyNames family, uint32 familyFlag1, uint32 familyFlag2, uint32 familyFlag3, ObjectGuid casterGUID);
#else
        void RemoveAurasWithFamily(SpellFamilyNames family, uint64 familyFlags, ObjectGuid casterGUID);
#endif
        void RemoveAurasWithMechanic(uint32 mechanic_mask, AuraRemoveMode removemode = AURA_REMOVE_BY_DEFAULT, uint32 except = 0);
        void RemoveMovementImpairingAuras(bool withRoot);
        void RemoveAurasByShapeShift();

        void RemoveAreaAurasDueToLeaveWorld();
        void RemoveAllAuras();
        void RemoveArenaAuras(bool onLeave);
        void RemoveAurasOnEvade();
        void RemoveAllAurasOnDeath();
        void RemoveAllAurasRequiringDeadTarget();
        void RemoveAllAurasExceptType(AuraType type);
        void RemoveAllAurasExceptType(AuraType type1, AuraType type2); /// @todo: once we support variadic templates use them here
        void RemoveAllGroupBuffsFromCaster(ObjectGuid casterGUID);
        void DelayOwnedAuras(uint32 spellId, ObjectGuid caster, int32 delaytime);

        void _RemoveAllAuraStatMods();
        void _ApplyAllAuraStatMods();

        AuraEffectList const& GetAuraEffectsByType(AuraType type) const { return m_modAuras[type]; }
        AuraList      & GetSingleCastAuras() { return m_scAuras; }
        AuraList const& GetSingleCastAuras() const { return m_scAuras; }

        AuraEffect* GetAuraEffect(uint32 spellId, uint8 effIndex, ObjectGuid casterGUID = ObjectGuid::Empty) const;
        AuraEffect* GetAuraEffectOfRankedSpell(uint32 spellId, uint8 effIndex, ObjectGuid casterGUID = ObjectGuid::Empty) const;
        AuraEffect* GetAuraEffect(AuraType type, SpellFamilyNames name, uint32 iconId, uint8 effIndex) const; // spell mustn't have familyflags
#ifdef LICH_KING
        AuraEffect* GetAuraEffect(AuraType type, SpellFamilyNames family, uint32 familyFlag1, uint32 familyFlag2, uint32 familyFlag3, ObjectGuid casterGUID = ObjectGuid::Empty) const;
#else
        AuraEffect* GetAuraEffect(AuraType type, SpellFamilyNames family, uint64 familyFlags, ObjectGuid casterGUID = ObjectGuid::Empty) const;
#endif
        AuraEffect* GetDummyAuraEffect(SpellFamilyNames name, uint32 iconId, uint8 effIndex) const;
        AuraEffect* GetDummyAura(uint32 spell_id) const;

        AuraApplication * GetAuraApplication(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint8 reqEffMask = 0, AuraApplication * except = nullptr) const;
        Aura* GetAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint8 reqEffMask = 0) const;

        AuraApplication * GetAuraApplicationOfRankedSpell(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint8 reqEffMask = 0, AuraApplication * except = nullptr) const;
        Aura* GetAuraOfRankedSpell(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint8 reqEffMask = 0) const;

        void GetDispellableAuraList(Unit* caster, uint32 dispelMask, DispelChargesList& dispelList, bool isReflect = false) const;

        bool HasAuraEffect(uint32 spellId, uint8 effIndex, ObjectGuid caster = ObjectGuid::Empty) const;
        uint32 GetAuraCount(uint32 spellId) const;
        bool HasAura(uint32 spellId, ObjectGuid casterGUID = ObjectGuid::Empty, ObjectGuid itemCasterGUID = ObjectGuid::Empty, uint8 reqEffMask = 0) const;
        bool HasAuraType(AuraType auraType) const;
        bool HasAuraTypeWithCaster(AuraType auratype, ObjectGuid caster) const;
        bool HasAuraTypeWithMiscvalue(AuraType auratype, int32 miscvalue) const;
        bool HasAuraTypeWithAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        bool HasAuraTypeWithValue(AuraType auratype, int32 value) const;
        bool HasNegativeAuraWithInterruptFlag(uint32 flag, ObjectGuid guid = ObjectGuid::Empty) const;
        bool HasAuraWithMechanic(Mechanics mechanicMask) const;
        bool HasStrongerAuraWithDR(SpellInfo const* auraSpellInfo, Unit* caster, bool triggered) const;

        AuraEffect* IsScriptOverriden(SpellInfo const* spell, int32 script) const;
#ifdef LICH_KING
        uint32 GetDiseasesByCaster(ObjectGuid casterGUID, bool remove = false);
        uint32 GetDoTsByCaster(ObjectGuid casterGUID) const;
#endif

        float GetResistanceBuffMods(SpellSchools school, bool positive) const { return GetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school ); }
        void SetResistanceBuffMods(SpellSchools school, bool positive, float val) { SetFloatValue(positive ? UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE+school : UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE+school,val); }

		void UpdateResistanceBuffModsMod(SpellSchools school);
		void InitStatBuffMods();
		void UpdateStatBuffMod(Stats stat);

        void SetCreateStat(Stats stat, float val) { m_createStats[stat] = val; }
        void SetCreateHealth(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_HEALTH, val); }
        uint32 GetCreateHealth() const { return GetUInt32Value(UNIT_FIELD_BASE_HEALTH); }
        void SetCreateMana(uint32 val) { SetUInt32Value(UNIT_FIELD_BASE_MANA, val); }
        uint32 GetCreateMana() const { return GetUInt32Value(UNIT_FIELD_BASE_MANA); }
        uint32 GetCreatePowers(Powers power) const;
        float GetPosStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_POSSTAT0+stat); }
        float GetNegStat(Stats stat) const { return GetFloatValue(UNIT_FIELD_NEGSTAT0+stat); }
        float GetCreateStat(Stats stat) const { return m_createStats[stat]; }

        ObjectGuid GetChannelObjectGuid() const { return GetGuidValue(UNIT_FIELD_CHANNEL_OBJECT); }
        void SetChannelObjectGuid(ObjectGuid guid) { SetGuidValue(UNIT_FIELD_CHANNEL_OBJECT, guid); }

        void SetCurrentCastedSpell(Spell * pSpell);
        void InterruptSpell(uint32 spellType, bool withDelayed = true, bool withInstant = true);
        void FinishSpell(CurrentSpellTypes spellType, bool ok = true);

        bool HasDelayedSpell();

        // set withDelayed to true to account delayed spells as casted
        // delayed+channeled spells are always accounted as casted
        // we can skip channeled or delayed checks using flags
        bool IsNonMeleeSpellCast(bool withDelayed, bool skipChanneled = false, bool skipAutorepeat = false, bool isAutoshoot = false) const;

        // set withDelayed to true to interrupt delayed spells too
        // delayed+channeled spells are always interrupted
        void InterruptNonMeleeSpells(bool withDelayed, uint32 spellid = 0, bool withInstant = true);

        Spell* FindCurrentSpellBySpellId(uint32 spell_id) const;
        
        bool IsCombatStationary();
        bool CanReachWithMeleeAttack(Unit* pVictim, float flat_mod = 0.0f) const;

        float GetCombatDistance() const { return m_CombatDistance; };
        void SetCombatDistance(float dist);

        bool IsCCed() const;

        inline Spell* GetCurrentSpell(CurrentSpellTypes spellType) const { return m_currentSpells[spellType]; }
        inline Spell* GetCurrentSpell(uint32 spellType) const { return m_currentSpells[spellType]; }

        // Check if our current channel spell has attribute SPELL_ATTR5_CAN_CHANNEL_WHEN_MOVING
        virtual bool IsMovementPreventedByCasting() const;
        virtual bool IsFocusing(Spell const* /*focusSpell*/ = nullptr, bool /*withDelay*/ = false) { return false; }

        Spell* m_currentSpells[CURRENT_MAX_SPELL];

        SpellHistory* GetSpellHistory() { return m_spellHistory; }
        SpellHistory const* GetSpellHistory() const { return m_spellHistory; }

        uint32 m_addDmgOnce;
        ObjectGuid m_SummonSlot[MAX_SUMMON_SLOT];
        ObjectGuid m_ObjectSlot[MAX_GAMEOBJECT_SLOT];
        uint32 m_ShapeShiftFormSpellId;
        ShapeshiftForm m_form;
        float m_modMeleeHitChance;
        float m_modRangedHitChance;
        float m_modSpellHitChance;
        int32 m_baseSpellCritChance;

        float m_modAttackSpeedPct[3];

        // Event handler. Processed only when creature is alive
        EventProcessor m_Events;

        // stat system
		void HandleStatFlatModifier(UnitMods unitMod, UnitModifierFlatType modifierType, float amount, bool apply);
		void ApplyStatPctModifier(UnitMods unitMod, UnitModifierPctType modifierType, float amount);


		void SetStatFlatModifier(UnitMods unitMod, UnitModifierFlatType modifierType, float val);
		void SetStatPctModifier(UnitMods unitMod, UnitModifierPctType modifierType, float val);

		float GetFlatModifierValue(UnitMods unitMod, UnitModifierFlatType modifierType) const;
		float GetPctModifierValue(UnitMods unitMod, UnitModifierPctType modifierType) const;

		void UpdateUnitMod(UnitMods unitMod);

        // only players have item requirements
        virtual bool CheckAttackFitToAuraRequirement(WeaponAttackType /*attackType*/, AuraEffect const* /*aurEff*/) const { return true; }

        virtual void UpdateDamageDoneMods(WeaponAttackType attackType);
        void UpdateAllDamageDoneMods();

        void UpdateDamagePctDoneMods(WeaponAttackType attackType);
        void UpdateAllDamagePctDoneMods();

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
		virtual void UpdateAllResistances();
        virtual void UpdateArmor() = 0;
        virtual void UpdateMaxHealth() = 0;
        virtual void UpdateMaxPower(Powers power) = 0;
        virtual void UpdateAttackPowerAndDamage(bool ranged = false) = 0;
        virtual void UpdateDamagePhysical(WeaponAttackType attType);
        float GetTotalAttackPowerValue(WeaponAttackType attType, Unit* victim = nullptr) const;
        float GetAPBonusVersus(WeaponAttackType attType, Unit* victim) const;
        float GetWeaponDamageRange(WeaponAttackType attType ,WeaponDamageRange type, uint8 damageIndex = 0) const;
        void SetBaseWeaponDamage(WeaponAttackType attType ,WeaponDamageRange damageRange, float value, uint8 damageIndex = 0) { m_weaponDamage[attType][damageRange][damageIndex] = value; }
        uint32 CalculateDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, uint8 itemDamagesMask = 0) const;
        virtual void CalculateMinMaxDamage(WeaponAttackType attType, bool normalized, bool addTotalPct, float& minDamage, float& maxDamage, uint8 damageIndex) const = 0;
        float GetAPMultiplier(WeaponAttackType attType, bool normalized) const;

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
        void SetFacingTo(float ori, bool force = true);
        void SetFacingToObject(WorldObject const* object, bool force = true);

        ObjectGuid GetTarget() const { return GetGuidValue(UNIT_FIELD_TARGET); }
        virtual void SetTarget(ObjectGuid /*guid*/) = 0;

        // Movement info
        Movement::MoveSpline * movespline;

        int32 GetHighestExclusiveSameEffectSpellGroupValue(AuraEffect const* aurEff, AuraType auraType, bool checkMiscValue = false, int32 miscValue = 0) const;
        bool IsHighestExclusiveAura(Aura const* aura, bool removeOtherAuraApplications = false);

        bool m_ControlledByPlayer;

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
        /// Returns the transport this unit is on directly (if on vehicle and transport, return vehicle)
        TransportBase* GetDirectTransport() const;

        void BuildMovementPacket(ByteBuffer *data) const;
		static void BuildMovementPacket(Position const& pos, Position const& transportPos, MovementInfo const& movementInfo, ByteBuffer* data);

        bool isMoving() const   { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_MOVING); }
        bool isTurning() const  { return m_movementInfo.HasMovementFlag(MOVEMENTFLAG_MASK_TURNING); }
        virtual bool CanFly() const = 0;
        virtual bool CanWalk() const = 0;
        virtual bool CanSwim() const;
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

        virtual float GetFollowAngle() const { return static_cast<float>(M_PI/2); }

		// Visibility system
		bool IsVisible() const;
		void SetVisible(bool x);

		void SetPhaseMask(uint32 newPhaseMask, bool update) override;// overwrite WorldObject::SetPhaseMask
		void UpdateObjectVisibility(bool forced = true) override;

        SpellImmuneList m_spellImmune[MAX_SPELL_IMMUNITY];

        int32 GetTotalAuraModifier(AuraType auratype) const;
        float GetTotalAuraMultiplier(AuraType auratype) const;
        int32 GetMaxPositiveAuraModifier(AuraType auratype) const;
        int32 GetMaxNegativeAuraModifier(AuraType auratype) const;

        int32 GetTotalAuraModifier(AuraType auratype, std::function<bool(AuraEffect const*)> const& predicate) const;
        float GetTotalAuraMultiplier(AuraType auratype, std::function<bool(AuraEffect const*)> const& predicate) const;
        int32 GetMaxPositiveAuraModifier(AuraType auratype, std::function<bool(AuraEffect const*)> const& predicate) const;
        int32 GetMaxNegativeAuraModifier(AuraType auratype, std::function<bool(AuraEffect const*)> const& predicate) const;

        int32 GetTotalAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        float GetTotalAuraMultiplierByMiscMask(AuraType auratype, uint32 misc_mask) const;
        int32 GetMaxPositiveAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask, AuraEffect const* except = nullptr) const;
        int32 GetMaxNegativeAuraModifierByMiscMask(AuraType auratype, uint32 misc_mask) const;

        int32 GetTotalAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        float GetTotalAuraMultiplierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxPositiveAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;
        int32 GetMaxNegativeAuraModifierByMiscValue(AuraType auratype, int32 misc_value) const;

        int32 GetTotalAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        float GetTotalAuraMultiplierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        int32 GetMaxPositiveAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;
        int32 GetMaxNegativeAuraModifierByAffectMask(AuraType auratype, SpellInfo const* affectedSpell) const;

        VisibleAuraMap const* GetVisibleAuras() { return &m_visibleAuras; }
        AuraApplication * GetVisibleAura(uint8 slot) const;
        void SetVisibleAura(uint8 slot, AuraApplication * aur);
        void RemoveVisibleAura(uint8 slot);

        uint32 GetInterruptMask() const { return m_interruptMask; }
        void AddInterruptMask(uint32 mask) { m_interruptMask |= mask; }
        void UpdateInterruptMask();

        uint32 GetDisplayId() const { return GetUInt32Value(UNIT_FIELD_DISPLAYID); }
        virtual void SetDisplayId(uint32 modelId);
        uint32 GetNativeDisplayId() const { return GetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID); }
        void SetNativeDisplayId(uint32 modelId) { SetUInt32Value(UNIT_FIELD_NATIVEDISPLAYID, modelId); }
        ShapeshiftForm GetShapeshiftForm() const { return ShapeshiftForm(GetByteValue(UNIT_FIELD_BYTES_2, 3)); }
        void SetShapeshiftForm(ShapeshiftForm form);
        uint32 GetModelForTotem(PlayerTotemType totemType);
        uint32 GetModelForForm (ShapeshiftForm from) const;
        void SetTransForm(uint32 spellid) { m_transform = spellid;}
        uint32 GetTransForm() const { return m_transform;}
        bool IsInDisallowedMountForm() const;

        void AddGameObject(GameObject* gameObj);
        void RemoveGameObject(GameObject* gameObj, bool del);
        void RemoveGameObject(uint32 spellid, bool del);
        void RemoveAllGameObjects();

        // DynamicObject management
        void _RegisterDynObject(DynamicObject* dynObj);
        void _UnregisterDynObject(DynamicObject* dynObj);
        DynamicObject* GetDynObject(uint32 spellId);
        void RemoveDynObject(uint32 spellId);
        void RemoveAllDynObjects();

        /** Change aura state if not already at given value. Be careful not to remove a state if other auras have the same state also active, you can use the m_auraStateAuras to check for those. */
        void ModifyAuraState(AuraStateType flag, bool apply);
        /** Get aura state for given target, needed since some aura state are relative to who casted what*/
        uint32 BuildAuraStateUpdateForTarget(Unit* target) const;
        /* Check if unit has at least one aura of given state; This just checks UNIT_FIELD_AURASTATE if no caster is given, or check in m_auraStateAuras if so (some state being caster dependant) */
        bool HasAuraState(AuraStateType flag, SpellInfo const* spellProto = nullptr, Unit const* Caster = nullptr) const;
        void UnsummonAllTotems();
        //This triggers a KillMagnetEvent
        bool IsMagnet() const;
        Unit* GetMagicHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo);
        Unit* GetMeleeHitRedirectTarget(Unit* victim, SpellInfo const* spellInfo = nullptr);

        uint32 GetCastingTimeForBonus(SpellInfo const* spellProto, DamageEffectType damagetype, uint32 CastingTime) const;
        float CalculateDefaultCoefficient(SpellInfo const *spellInfo, DamageEffectType damagetype) const;

        uint32 GetRemainingPeriodicAmount(ObjectGuid caster, uint32 spellId, AuraType auraType, uint8 effectIndex = 0) const;

        int32 SpellBaseDamageBonusDone(SpellSchoolMask schoolMask, Unit* pVictim = nullptr);
        int32 SpellBaseDamageBonusTaken(SpellInfo const* spellInfo, bool isDoT = false);
        float SpellDamagePctDone(Unit* victim, SpellInfo const *spellProto, DamageEffectType damagetype);

        uint32 SpellDamageBonusDone(Unit *pVictim, SpellInfo const *spellProto, uint32 damage, DamageEffectType damagetype, uint8 effIndex, Optional<float> const& donePctTotal = {}, uint32 stack = 1);
        uint32 SpellDamageBonusTaken(Unit* caster, SpellInfo const *spellProto, uint32 pdamage, DamageEffectType damagetype);
        
        // SPELL_AURA_MOD_HEALING_DONE + SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT + SPELL_AURA_MOD_SPELL_HEALING_OF_ATTACK_POWER + hacks
        int32 SpellBaseHealingBonusDone(SpellSchoolMask schoolMask);
        // SPELL_AURA_MOD_HEALING + hacks
        int32 SpellBaseHealingBonusTaken(SpellInfo const* spellProto);
        //Includes SPELL_AURA_MOD_HEALING_DONE_PERCENT + lots of hacks
        float SpellHealingPctDone(Unit* victim, SpellInfo const *spellProto);

        /* Alter healamount with healing bonus taken
        This includes : SPELL_AURA_MOD_HEALING_PCT, SPELL_AURA_MOD_HEALING_PCT, SPELLMOD_BONUS_MULTIPLIER, and lots of hacks
        */
        uint32 SpellHealingBonusTaken(Unit* caster, SpellInfo const *spellProto, uint32 healamount, DamageEffectType damagetype);
        /* Alter healamount with healing bonus done for victim
        This includes : SpellBaseHealingBonusDone, SPELLMOD_BONUS_MULTIPLIER, ApplySpellMod, SpellHealingPctDone, and lots of hacks
        */
        uint32 SpellHealingBonusDone(Unit* victim, SpellInfo const *spellProto, uint32 healamount, DamageEffectType damagetype, uint8 effIndex, Optional<float> const& donePctTotal = {}, uint32 stack = 1);

        uint32 MeleeDamageBonusDone(Unit* pVictim, uint32 damage, WeaponAttackType attType, SpellInfo const* spellProto = nullptr, SpellSchoolMask damageSchoolMask = SPELL_SCHOOL_MASK_NORMAL);
        uint32 MeleeDamageBonusTaken(Unit* attacker, uint32 pdamage, WeaponAttackType attType, SpellInfo const* spellProto = nullptr, SpellSchoolMask damageSchoolMask = SPELL_SCHOOL_MASK_NORMAL);

        bool   IsSpellBlocked(Unit *pVictim, SpellInfo const *spellProto, WeaponAttackType attackType = BASE_ATTACK);
        float  SpellCritChanceDone(SpellInfo const* spellInfo, SpellSchoolMask schoolMask, WeaponAttackType attackType = BASE_ATTACK) const;
        float  SpellCritChanceTaken(Unit const* caster, SpellInfo const* spellInfo, SpellSchoolMask schoolMask, float doneChance, WeaponAttackType attackType = BASE_ATTACK) const;
        static uint32 SpellCriticalDamageBonus(Unit const* caster, SpellInfo const* spellProto, uint32 damage, Unit* victim);
        static uint32 SpellCriticalHealingBonus(Unit const* caster, SpellInfo const* spellProto, uint32 damage, Unit* victim);
        //Spells disabled in spell_disabled table
        bool IsSpellDisabled(uint32 const spellId);

        void SetLastManaUse(uint32 spellCastTime) { m_lastManaUse = spellCastTime; }
        bool IsUnderLastManaUseEffect() const;

        void ApplySpellImmune(uint32 spellId, uint32 op, uint32 type, bool apply);
        virtual bool IsImmunedToSpell(SpellInfo const* spellInfo, Unit* caster);

		bool IsImmunedToDamage(SpellInfo const* spellInfo) const;
		bool IsImmunedToDamage(SpellSchoolMask meleeSchoolMask) const;
        virtual bool IsImmunedToSpellEffect(SpellInfo const* spellInfo, uint32 index, Unit* caster) const;
        uint32 GetSchoolImmunityMask() const;
        uint32 GetDamageImmunityMask() const;
        uint32 GetMechanicImmunityMask() const;

        static bool IsDamageReducedByArmor(SpellSchoolMask damageSchoolMask, SpellInfo const* spellInfo = nullptr, int8 effIndex = -1);
        static uint32 CalcArmorReducedDamage(Unit const* attacker, Unit* pVictim, const uint32 damage, SpellInfo const* spellInfo, WeaponAttackType attackType);
        static uint32 CalcSpellResistedDamage(Unit const* attacker, Unit* victim, uint32 damage, SpellSchoolMask schoolMask, SpellInfo const* spellInfo);
        static void CalcAbsorbResist(DamageInfo& damageInfo);

        void  UpdateSpeed(UnitMoveType mtype);
        float GetSpeed( UnitMoveType mtype ) const;
        float GetSpeedRate( UnitMoveType mtype ) const { return m_speed_rate[mtype]; }
        void SetSpeedRate(UnitMoveType mtype, float rate, bool sendUpdate = true);

        float ApplyEffectModifiers(SpellInfo const* spellProto, uint8 effect_index, float value) const;
        int32 CalculateSpellDamage(Unit const* target, SpellInfo const* spellProto, uint8 effect_index, int32 const* basePoints = nullptr);
        int32 CalcSpellDuration(SpellInfo const* spellProto);
        int32 ModSpellDuration(SpellInfo const* spellProto, Unit const* target, int32 duration, bool positive, uint32 effectMask);
        float CalculateSpellpowerCoefficientLevelPenalty(SpellInfo const* spellProto) const;
        void ModSpellDurationTime(SpellInfo const* spellProto, int32& castTime, Spell* spell = nullptr);
        int32 CalculateAOEAvoidance(int32 damage, uint32 schoolMask, ObjectGuid const& casterGuid) const;

        void AddFollower(FollowerReference* pRef) { m_FollowingRefManager.insertFirst(pRef); }
        void RemoveFollower(FollowerReference* /*pRef*/) { /* nothing to do yet */ }

        MotionMaster* GetMotionMaster() { return i_motionMaster; }
        const MotionMaster* GetMotionMaster() const { return i_motionMaster; }

        //returns wheter unit his currently stopped (= has any move related UNIT_STATE)
        bool IsStopped() const { return !(HasUnitState(UNIT_STATE_MOVING)); }
        void StopMoving();
        void StopMovingOnCurrentPos();
        void PauseMovement(uint32 timer = 0, uint8 slot = 0, bool forced = true); // timer in ms
        void ResumeMovement(uint32 timer = 0, uint8 slot = 0); // timer in ms

        void AddUnitMovementFlag(uint32 f) { m_movementInfo.flags |= f; }
        void RemoveUnitMovementFlag(uint32 f) { m_movementInfo.flags &= ~f; }
        bool HasUnitMovementFlag(uint32 f) const { return (m_movementInfo.flags & f) == f; }
        uint32 GetUnitMovementFlags() const { return m_movementInfo.flags; }
        void SetUnitMovementFlags(uint32 f) { m_movementInfo.flags = f; }

        float GetPositionZMinusOffset() const;

        void SetControlled(bool apply, UnitState state);
        void ApplyControlStatesIfNeeded();
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

        // proc trigger system
        bool CanProc() const { return !m_procDeep; }
        void SetCantProc(bool apply);

        // pet auras
        typedef std::set<PetAura const*> PetAuraSet;
        PetAuraSet m_petAuras;
        void AddPetAura(PetAura const* petSpell);
        void RemovePetAura(PetAura const* petSpell);

        // relocation notification
        //void SetToNotify();
        //bool m_Notified, m_IsInNotifyList;
        //float oldX, oldY, oldZ;

        bool IsAIEnabled, NeedChangeAI;

        void OutDebugInfo() const;
        virtual bool IsLoading() const { return false; }
		bool IsDuringRemoveFromWorld() const { return m_duringRemoveFromWorld; }

        Pet* ToPet() { if(IsPet()) return reinterpret_cast<Pet*>(this); else return nullptr; } 
        Pet const* ToPet() const { if (IsPet()) return reinterpret_cast<Pet const*>(this); else return nullptr; }

        Totem* ToTotem(){ if(IsTotem()) return reinterpret_cast<Totem*>(this); else return nullptr; } 
        Totem const* ToTotem() const { if (IsTotem()) return reinterpret_cast<Totem const*>(this); else return nullptr; }

        TempSummon* ToTempSummon() { if (IsSummon()) return reinterpret_cast<TempSummon*>(this); else return nullptr; }
        TempSummon const* ToTempSummon() const { if (IsSummon()) return reinterpret_cast<TempSummon const*>(this); else return nullptr; }

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
        void old_Talk(uint32 textId, ChatMsg msgType, float textRange, ObjectGuid targetGUID, uint32 language);
        void old_Say(int32 textId, uint32 language, ObjectGuid TargetGuid);
        void old_Yell(int32 textId, uint32 language, ObjectGuid TargetGuid);
        void old_TextEmote(int32 textId, ObjectGuid TargetGuid, bool IsBossEmote = false);
        void old_Whisper(int32 textId, ObjectGuid receiver, bool IsBossWhisper = false);

        float GetCollisionHeight() const override;

        void FeignDeathDetected(Unit const* by);
        bool IsFeighDeathDetected(Unit const* by) const;
        void ResetFeignDeathDetected();

    protected:
        explicit Unit (bool isWorldObject);

        void BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, Player* target) const override;

        UnitAI* i_AI;
        UnitAI* i_disabledAI;

        bool _last_in_water_status;
        Position _lastInWaterCheckPosition;
        bool _last_isunderwater_status;

        void _UpdateSpells(uint32 time);
        void _DeleteRemovedAuras();

        void _UpdateAutoRepeatSpell();
        bool m_AutoRepeatFirstCast;

        uint32 m_attackTimer[MAX_ATTACK];

        float m_createStats[MAX_STATS];

        AttackerSet m_attackers;
        Unit* m_attacking;

        DeathState m_deathState;

        int32 m_procDeep;

        typedef std::list<DynamicObject*> DynObjectList;
        DynObjectList m_dynObj;

        typedef std::list<GameObject*> GameObjectList;
        GameObjectList m_gameObj;

        bool m_isSorted;
        uint32 m_transform;

        AuraMap m_ownedAuras; //all auras owned by this unit, not necessarily on this unit
        AuraApplicationMap m_appliedAuras; //all auras present on this unit
        AuraList m_removedAuras; //auras marked for remove
        AuraMap::iterator m_auraUpdateIterator; //m_ownedAuras iterator
        uint32 m_removedAurasCount; //count how much auras were removed (does not reset at each update)

        AuraEffectList m_modAuras[TOTAL_AURAS]; //all aura effects applied on this unit
        AuraList m_scAuras;                     // casted singlecast auras. List auras casted on other units with the flag SPELL_ATTR5_SINGLE_TARGET_SPELL, such as polymorph
        AuraApplicationList m_interruptableAuras;          // auras on this unit with an AuraInterruptFlags
        AuraApplicationList m_ccAuras; //crowd control aura with a chance of being interrupted by damage
        AuraStateAurasMap m_auraStateAuras;        // List of all auras affecting aura states, casted by who, Used for improve performance of aura state checks on aura apply/remove
        uint32 m_interruptMask;

		float m_auraFlatModifiersGroup[UNIT_MOD_END][MODIFIER_TYPE_FLAT_END];
		float m_auraPctModifiersGroup[UNIT_MOD_END][MODIFIER_TYPE_PCT_END];
        float m_weaponDamage[MAX_ATTACK][MAX_WEAPON_DAMAGE_RANGE][MAX_ITEM_PROTO_DAMAGES];
        bool m_canModifyStats;
        VisibleAuraMap m_visibleAuras;

        float m_speed_rate[MAX_MOVE_TYPE];

        CharmInfo* m_charmInfo;
        SharedVisionList m_sharedVision; //Players sharing this unit vision

        MotionMaster* i_motionMaster;

        uint32 m_reactiveTimer[MAX_REACTIVE];
        uint32 m_regenTimer; //used differently in Creature and Player... Creature has it counting down, players has it up (TC logic)

        uint32 m_unitTypeMask;
        LiquidTypeEntry const* _lastLiquid;
        
        uint8 m_justCCed; // Set to 2 when getting CC aura, decremented (if > 0) every update - used to stop pet combat on target

        std::map<ObjectGuid, uint16> m_detectedUnit; // <guid,timer> for stealth detection, a spotted unit is kept visible for a while 
        std::set<ObjectGuid> m_detectedByUnit; //we need to keep track of who detected us to be able to reset it when needed
        
        // Testing
        bool m_disabledRegen;

        bool m_cleanupDone; // lock made to not add stuff after cleanup before delete
        bool m_duringRemoveFromWorld; // lock made to not add stuff after begining removing from world

        float m_CombatDistance;

        SpellHistory* m_spellHistory;

        GuidSet m_feignDeathDetectedBy;
    private:
        void ProcSkillsAndReactives(bool isVictim, Unit* procTarget, uint32 typeMask, uint32 hitMask, WeaponAttackType attType);

        uint32 m_state;                                     // Even derived shouldn't modify
        uint32 m_lastManaUse;                               // msecs
        TimeTrackerSmall m_movesplineTimer;

        DiminishingReturn m_Diminishing[DIMINISHING_MAX];
        // Manage all Units that are threatened by us
        friend class CombatManager;
        CombatManager m_combatManager; 
        friend class ThreatManager;
        ThreatManager m_threatManager;

        FollowerRefManager m_FollowingRefManager;

        ComboPointHolderSet m_ComboPointHolders;
        
        uint8 IsRotating;//0none 1left 2right
        uint32 RotateTimer;
        uint32 RotateTimerFull;
        double RotateAngle;
        ObjectGuid LastTargetGUID;
        bool m_attackVictimOnEnd;

        bool _targetLocked; // locks the target during spell cast for proper facing
        time_t _lastDamagedTime; // Part of Evade mechanic
};

// drop a charge for the first aura found of type SPELL_AURA_SPELL_MAGNET
class RedirectSpellEvent : public BasicEvent
{
public:
    RedirectSpellEvent(Unit& self, ObjectGuid auraOwnerGUID, AuraEffect* auraEffect) : _self(self), _auraOwnerGUID(auraOwnerGUID), _auraEffect(auraEffect) { }
    bool Execute(uint64 e_time, uint32 p_time) override;

protected:
    Unit & _self;
    ObjectGuid _auraOwnerGUID;
    AuraEffect* _auraEffect;
};

#endif

