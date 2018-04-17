#ifndef TRINITYCORE_PET_DEFINES_H
#define TRINITYCORE_PET_DEFINES_H

enum PetType : int
{
    SUMMON_PET              = 0,
    HUNTER_PET              = 1,
    GUARDIAN_PET            = 2,
    MINI_PET                = 3,
    POSSESSED_PET           = 4,
    MAX_PET_TYPE            = 5
};

#ifdef LICH_LING
    #define MAX_PET_STABLES         4
#else
    #define MAX_PET_STABLES         3
#endif

extern char const* petTypeSuffix[MAX_PET_TYPE];

// stored in character_pet.slot
enum PetSaveMode : int
{
    PET_SAVE_AS_DELETED        = -1,                        // not saved in fact
    PET_SAVE_AS_CURRENT        =  0,                        // in current slot (with player)
    PET_SAVE_FIRST_STABLE_SLOT =  1,
    PET_SAVE_LAST_STABLE_SLOT  =  MAX_PET_STABLES,          // last in DB stable slot index (including), all higher have same meaning as PET_SAVE_NOT_IN_SLOT
    PET_SAVE_NOT_IN_SLOT       =  100                       // for avoid conflict with stable size grow will use 100
};

enum HappinessState
{
    UNHAPPY = 1,
    CONTENT = 2,
    HAPPY   = 3
};

enum LoyaltyLevel
{
    REBELLIOUS  = 1,
    UNRULY      = 2,
    SUBMISSIVE  = 3,
    DEPENDABLE  = 4,
    FAITHFUL    = 5,
    BEST_FRIEND = 6
};

enum PetSpellState
{
    PETSPELL_UNCHANGED = 0,
    PETSPELL_CHANGED   = 1,
    PETSPELL_NEW       = 2,
    PETSPELL_REMOVED   = 3
};

enum PetSpellType
{
    PETSPELL_NORMAL = 0,
    PETSPELL_FAMILY = 1,
};

struct PetSpell
{
    uint16 slotId;
    uint16 active;

    PetSpellState state : 16;
    PetSpellType type   : 16;
};

enum ActionFeedback
{
    FEEDBACK_NONE            = 0,
    FEEDBACK_PET_DEAD        = 1,
    FEEDBACK_NOTHING_TO_ATT  = 2,
    FEEDBACK_CANT_ATT_TARGET = 3
};

enum PetTalk
{
    PET_TALK_SPECIAL_SPELL  = 0,
    PET_TALK_ATTACK         = 1
};

enum PetNameInvalidReason : int
{
    // custom, not send
    PET_NAME_SUCCESS                                        = 0,

    PET_NAME_INVALID                                        = 1,
    PET_NAME_NO_NAME                                        = 2,
    PET_NAME_TOO_SHORT                                      = 3,
    PET_NAME_TOO_LONG                                       = 4,
    PET_NAME_MIXED_LANGUAGES                                = 6,
    PET_NAME_PROFANE                                        = 7,
    PET_NAME_RESERVED                                       = 8,
    PET_NAME_THREE_CONSECUTIVE                              = 11,
    PET_NAME_INVALID_SPACE                                  = 12,
    PET_NAME_CONSECUTIVE_SPACES                             = 13,
    PET_NAME_RUSSIAN_CONSECUTIVE_SILENT_CHARACTERS          = 14,
    PET_NAME_RUSSIAN_SILENT_CHARACTER_AT_BEGINNING_OR_END   = 15,
    PET_NAME_DECLENSION_DOESNT_MATCH_BASE_NAME              = 16
};

#define ACTIVE_SPELLS_MAX           4

#define OWNER_MAX_DISTANCE 100

#define PET_FOLLOW_DIST  0.7
#define PET_FOLLOW_ANGLE float(M_PI)

#define PET_XP_FACTOR 0.25f


#define ENTRY_IMP                 416
#define ENTRY_VOIDWALKER          1860
#define ENTRY_SUCCUBUS            1863
#define ENTRY_FELHUNTER           417
#define ENTRY_FELGUARD            17252
#define ENTRY_WATER_ELEMENTAL     510
#define ENTRY_TREANT              1964
#define ENTRY_FIRE_ELEMENTAL      15438
#define ENTRY_SHADOWFIEND         19668
#define ENTRY_SNAKE_TRAP_VENOMOUS 19833
#define ENTRY_SNAKE_TRAP_VIPER    19921
#define ENTRY_EARTH_ELEMENTAL     15352
#define ENTRY_STEAM_TONK          19405

#endif