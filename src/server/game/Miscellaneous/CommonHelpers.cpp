#include "CommonHelpers.h"
#include "Common.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Player.h"
#include "SharedDefines.h"

enum PlayerSpecializationIconicSpellIds
{
    /* Generic */
    SPELL_AUTO_SHOT         =    75,
    SPELL_SHOOT             =  3018,
    SPELL_THROW             =  2764,
    SPELL_SHOOT_WAND        =  5019,

    /* Warrior - Generic */
    SPELL_BATTLE_STANCE     =  2457,
    SPELL_BERSERKER_STANCE  =  2458,
    SPELL_DEFENSIVE_STANCE  =    71,
    SPELL_CHARGE            = 11578,
    SPELL_INTERCEPT         = 20252,
    SPELL_ENRAGED_REGEN     = 55694,
    SPELL_INTIMIDATING_SHOUT=  5246,
    SPELL_PUMMEL            =  6552,
    SPELL_SHIELD_BASH       =    72,
    SPELL_BLOODRAGE         =  2687,

    /* Warrior - Arms */
    SPELL_SWEEPING_STRIKES  = 12328,
    SPELL_MORTAL_STRIKE     = 12294,
    SPELL_BLADESTORM        = 46924,
    SPELL_REND              = 47465,
    SPELL_RETALIATION       = 20230,
    SPELL_THUNDER_CLAP      = 47502,
#ifdef LICH_KING
    SPELL_SHATTERING_THROW  = 64382,
#endif

    /* Warrior - Fury */
    SPELL_DEATH_WISH        = 12292,
    SPELL_BLOODTHIRST       = 23881,
    SPELL_RECKLESSNESS      =  1719,
    SPELL_PIERCING_HOWL     = 12323,
#ifdef LICH_KING
    SPELL_DEMO_SHOUT        = 47437,
    SPELL_HEROIC_FURY       = 60970,
    SPELL_EXECUTE           = 47471,
    PASSIVE_TITANS_GRIP     = 46917,
#else
    SPELL_DEMO_SHOUT        = 25203,
    SPELL_EXECUTE           = 25236,
#endif

    /* Warrior - Protection */
#ifdef LICH_KING
    SPELL_VIGILANCE         = 50720,
    SPELL_SHOCKWAVE         = 46968,
    SPELL_SHIELD_SLAM       = 47488,
#else
    SPELL_SHIELD_SLAM       = 30356,
#endif
    SPELL_DEVASTATE         = 20243,
    SPELL_CONCUSSION_BLOW   = 12809,
    SPELL_DISARM            =   676,
    SPELL_LAST_STAND        = 12975,
    SPELL_SHIELD_BLOCK      =  2565,
    SPELL_SHIELD_WALL       =   871,
    SPELL_SPELL_REFLECTION  = 23920,

    /* Paladin - Generic */
    SPELL_AURA_MASTERY          = 31821,
#ifdef LICH_KING
    SPELL_LAY_ON_HANDS          = 48788,
    SPELL_BLESSING_OF_MIGHT     = 48932,
#else
    SPELL_LAY_ON_HANDS          = 27154,
    SPELL_BLESSING_OF_MIGHT     = 27140,
#endif
    SPELL_AVENGING_WRATH        = 31884,
    SPELL_DIVINE_PROTECTION     =   498,
    SPELL_DIVINE_SHIELD         =   642,
    SPELL_HAMMER_OF_JUSTICE     = 10308,
    SPELL_HAND_OF_FREEDOM       =  1044,
    SPELL_HAND_OF_PROTECTION    = 10278,
    SPELL_HAND_OF_SACRIFICE     =  6940,

    /* Paladin - Holy*/
    PASSIVE_ILLUMINATION        = 20215,
    SPELL_HOLY_SHOCK            = 20473,
#ifdef LICH_KING
    SPELL_BEACON_OF_LIGHT       = 53563,
    SPELL_FLASH_OF_LIGHT        = 48785,
    SPELL_CONSECRATION          = 48819,
    SPELL_HOLY_LIGHT            = 48782,
#else
    SPELL_FLASH_OF_LIGHT        = 27137,
    SPELL_CONSECRATION          = 27173,
    SPELL_HOLY_LIGHT            = 27136,
#endif
    SPELL_DIVINE_FAVOR          = 20216,
    SPELL_DIVINE_ILLUMINATION   = 31842,

    /* Paladin - Protection */
    SPELL_BLESS_OF_SANC         = 20911,
    SPELL_HOLY_SHIELD           = 20925,
#ifdef LICH_KING
    SPELL_DIVINE_SACRIFICE      = 64205,
    SPELL_HAMMER_OF_RIGHTEOUS   = 53595,
    SPELL_AVENGERS_SHIELD       = 48827,
    SPELL_SHIELD_OF_RIGHTEOUS   = 61411,
    #else
    SPELL_AVENGERS_SHIELD       = 32700,
#endif
    SPELL_RIGHTEOUS_FURY        = 25780,

    /* Paladin - Retribution */
    SPELL_SEAL_OF_COMMAND       = 20375,
    SPELL_CRUSADER_STRIKE       = 35395,
#ifdef LICH_KING
    SPELL_DIVINE_STORM          = 53385,
    SPELL_HAMMER_OF_WRATH       = 48806,
#else
    SPELL_HAMMER_OF_WRATH       = 27180,
#endif
    SPELL_JUDGEMENT_            = 20271,

    /* Hunter - Generic */
    SPELL_DETERRENCE        = 19263,
#ifdef LICH_KING
    SPELL_EXPLOSIVE_TRAP    = 49067,
    SPELL_FREEZING_ARROW    = 60192,
    SPELL_MULTI_SHOT        = 49048,
    SPELL_KILL_SHOT         = 61006,
#else
    SPELL_MULTI_SHOT        = 27021,
    SPELL_EXPLOSIVE_TRAP    = 27025,
#endif
    SPELL_RAPID_FIRE        =  3045,
    SPELL_VIPER_STING       =  3034,

    /* Hunter - Beast Mastery */
    SPELL_BESTIAL_WRATH     = 19574,
    PASSIVE_BEAST_WITHIN    = 34692,
#ifdef LICH_KING
    PASSIVE_BEAST_MASTERY   = 53270,
#endif

    /* Hunter - Marksmanship */
    SPELL_AIMED_SHOT        = 19434,
    PASSIVE_TRUESHOT_AURA   = 19506,
#ifdef LICH_KING
    SPELL_ARCANE_SHOT       = 49045,
    SPELL_STEADY_SHOT       = 49052,
    SPELL_CHIMERA_SHOT      = 53209,
#else
    SPELL_ARCANE_SHOT       = 27019,
    SPELL_STEADY_SHOT       = 34120,
#endif
    SPELL_READINESS         = 23989,
    SPELL_SILENCING_SHOT    = 34490,

    /* Hunter - Survival */
#ifdef LICH_KING
    PASSIVE_LOCK_AND_LOAD   = 56344,
    SPELL_EXPLOSIVE_SHOT    = 53301,
#endif
    SPELL_WYVERN_STING      = 19386,
    SPELL_BLACK_ARROW       =  3674,

    /* Rogue - Generic */
#ifdef LICH_KING
    SPELL_DISMANTLE         = 51722,
#endif
    SPELL_EVASION           = 26669,
    SPELL_KICK              =  1766,
    SPELL_VANISH            = 26889,
    SPELL_BLIND             =  2094,
    SPELL_CLOAK_OF_SHADOWS  = 31224,

    /* Rogue - Assassination */
    SPELL_COLD_BLOOD        = 14177,
    SPELL_MUTILATE          =  1329,
#ifdef LICH_KING
    SPELL_HUNGER_FOR_BLOOD  = 51662,
    SPELL_ENVENOM           = 57993,
#else
    SPELL_ENVENOM           = 32684,
#endif

    /* Rogue - Combat */
#ifdef LICH_KING
    SPELL_SINISTER_STRIKE   = 48637,
    SPELL_EVISCERATE        = 48668,
    SPELL_KILLING_SPREE     = 51690,
#else
    SPELL_EVISCERATE        = 26865,
    SPELL_SINISTER_STRIKE   = 26862,
#endif
    SPELL_BLADE_FLURRY      = 13877,
    SPELL_ADRENALINE_RUSH   = 13750,

    /* Rogue - Sublety */
    SPELL_HEMORRHAGE        = 16511,
    SPELL_PREMEDITATION     = 14183,
#ifdef LICH_KING
    SPELL_SHADOW_DANCE      = 51713,
#endif
    SPELL_PREPARATION       = 14185,
    SPELL_SHADOWSTEP        = 36554,

    /* Priest - Generic */
    SPELL_FEAR_WARD         =  6346,
#ifdef LICH_KING
    SPELL_POWER_WORD_FORT   = 48161,
    SPELL_DIVINE_SPIRIT     = 48073,
    SPELL_SHADOW_PROTECTION = 48169,
    SPELL_DIVINE_HYMN       = 64843,
    SPELL_HYMN_OF_HOPE      = 64901,
    SPELL_SHADOW_WORD_DEATH = 48158,
#else
    SPELL_POWER_WORD_FORT   = 25389,
    SPELL_DIVINE_SPIRIT     = 25312,
    SPELL_SHADOW_PROTECTION = 39374,
    SPELL_SHADOW_WORD_DEATH = 32996,
#endif
    SPELL_PSYCHIC_SCREAM    = 10890,

    /* Priest - Discipline */
#ifdef LICH_KING
    SPELL_PENANCE             = 47540,
    SPELL_POWER_WORD_SHIELD   = 48066,
    PASSIVE_SOUL_WARDING      = 63574,
#else
    SPELL_POWER_WORD_SHIELD   = 25218,
    SPELL_PAIN_SUPPRESSION    = 33206,
#endif
    SPELL_POWER_INFUSION      = 10060,
    SPELL_INNER_FOCUS         = 14751,

    /* Priest - Holy */
    PASSIVE_SPIRIT_REDEMPTION = 20711,
    SPELL_DESPERATE_PRAYER    = 19236,
#ifdef LICH_KING
    SPELL_GUARDIAN_SPIRIT     = 47788,
    SPELL_FLASH_HEAL          = 48071,
    SPELL_RENEW               = 48068,
#else
    SPELL_FLASH_HEAL          = 25235,
    SPELL_RENEW               = 25222,
#endif

    /* Priest - Shadow */
    SPELL_VAMPIRIC_EMBRACE    = 15286,
    SPELL_SHADOWFORM          = 15473,
    SPELL_VAMPIRIC_TOUCH      = 34914,
    SPELL_MIND_FLAY           = 15407,
#ifdef LICH_KING
    SPELL_MIND_BLAST          = 48127,
    SPELL_SHADOW_WORD_PAIN    = 48125,
    SPELL_DEVOURING_PLAGUE    = 48300,
    SPELL_DISPERSION          = 47585,
#else
    SPELL_MIND_BLAST          = 25375,
    SPELL_SHADOW_WORD_PAIN    = 32996,
#endif

    /* Death Knight - Generic */
#ifdef LICH_KING
    SPELL_DEATH_GRIP        = 49576,
    SPELL_STRANGULATE       = 47476,
    SPELL_EMPOWER_RUNE_WEAP = 47568,
    SPELL_ICEBORN_FORTITUDE = 48792,
    SPELL_ANTI_MAGIC_SHELL  = 48707,
    SPELL_DEATH_COIL_DK     = 49895,
    SPELL_MIND_FREEZE       = 47528,
    SPELL_ICY_TOUCH         = 49909,
    AURA_FROST_FEVER        = 55095,
    SPELL_PLAGUE_STRIKE     = 49921,
    AURA_BLOOD_PLAGUE       = 55078,
    SPELL_PESTILENCE        = 50842,

    /* Death Knight - Blood */
    SPELL_RUNE_TAP          = 48982,
    SPELL_HYSTERIA          = 49016,
    SPELL_HEART_STRIKE      = 55050,
    SPELL_DEATH_STRIKE      = 49924,
    SPELL_BLOOD_STRIKE      = 49930,
    SPELL_MARK_OF_BLOOD     = 49005,
    SPELL_VAMPIRIC_BLOOD    = 55233,

    /* Death Knight - Frost */
    PASSIVE_ICY_TALONS      = 50887,
    SPELL_FROST_STRIKE      = 49143,
    SPELL_HOWLING_BLAST     = 49184,
    SPELL_UNBREAKABLE_ARMOR = 51271,
    SPELL_OBLITERATE        = 51425,
    SPELL_DEATHCHILL        = 49796,

    /* Death Knight - Unholy */
    PASSIVE_UNHOLY_BLIGHT   = 49194,
    PASSIVE_MASTER_OF_GHOUL = 52143,
    SPELL_SCOURGE_STRIKE    = 55090,
    SPELL_DEATH_AND_DECAY   = 49938,
    SPELL_ANTI_MAGIC_ZONE   = 51052,
    SPELL_SUMMON_GARGOYLE   = 49206,
#endif

    /* Shaman - Generic */
    SPELL_HEROISM           = 32182,
    SPELL_BLOODLUST         =  2825,
    SPELL_GROUNDING_TOTEM   =  8177,

    /* Shaman - Elemental*/
    PASSIVE_ELEMENTAL_FOCUS = 16164,
    SPELL_TOTEM_OF_WRATH    = 30706,
#ifdef LICH_KING
    SPELL_THUNDERSTORM      = 51490,
    SPELL_LIGHTNING_BOLT    = 49238,
    SPELL_EARTH_SHOCK       = 49231,
    SPELL_FLAME_SHOCK       = 49233,
    SPELL_CHAIN_LIGHTNING   = 49271,
    SPELL_LAVA_BURST        = 60043,
#else
    SPELL_LIGHTNING_BOLT    = 25449,
    SPELL_EARTH_SHOCK       = 25454,
    SPELL_CHAIN_LIGHTNING   = 25442,
    SPELL_FLAME_SHOCK       = 29228,
#endif
    SPELL_ELEMENTAL_MASTERY = 16166,

    /* Shaman - Enhancement */
    PASSIVE_SPIRIT_WEAPONS  = 16268,
#ifdef LICH_KING
    SPELL_LAVA_LASH         = 60103,
    SPELL_FERAL_SPIRIT      = 51533,
    AURA_MAELSTROM_WEAPON   = 53817,
#else

#endif
    SPELL_STORMSTRIKE       = 17364,
    SPELL_SHAMANISTIC_RAGE  = 30823,

    /* Shaman - Restoration*/
#ifdef LICH_KING
    SPELL_EARTH_SHIELD      = 49284,
    SPELL_RIPTIDE           = 61295,
    SPELL_HEALING_WAVE      = 49273,
    SPELL_LESSER_HEAL_WAVE  = 49276,
    SPELL_TIDAL_FORCE       = 55198,
    SPELL_MANA_TIDE_TOTEM   =   590,
    SPELL_SHA_NATURE_SWIFT  =   591,
#else
    SPELL_SHA_NATURE_SWIFT  = 16188,
    SPELL_MANA_TIDE_TOTEM   = 16190,
    SPELL_EARTH_SHIELD      = 32594,
    SPELL_HEALING_WAVE      = 25396,
    SPELL_LESSER_HEAL_WAVE  = 25420,
#endif

    /* Mage - Generic */
#ifdef LICH_KING
    SPELL_DAMPEN_MAGIC      = 43015,
    SPELL_MANA_SHIELD       = 43020,
    SPELL_MIRROR_IMAGE      = 55342,
    SPELL_ICE_BLOCK         = 45438,
#else
    SPELL_DAMPEN_MAGIC      = 33944,
    SPELL_MANA_SHIELD       = 27131,
    SPELL_ICE_BLOCK         = 27619,
#endif
    SPELL_EVOCATION         = 12051,
    SPELL_SPELLSTEAL        = 30449,
    SPELL_COUNTERSPELL      =  2139,
        
    /* Mage - Arcane */
#ifdef LICH_KING
    SPELL_FOCUS_MAGIC       = 54646,
    SPELL_ARCANE_BARRAGE    = 44425,
    SPELL_ARCANE_BLAST      = 42897,
    SPELL_ARCANE_MISSILES   = 42846,
#else
    SPELL_ARCANE_BLAST      = 24857,
    SPELL_ARCANE_MISSILES   = 38704,
    AURA_MIND_MASTERY       = 31588,
#endif
    SPELL_ARCANE_POWER      = 12042,
    AURA_ARCANE_BLAST       = 36032,
    SPELL_PRESENCE_OF_MIND  = 12043,

    /* Mage - Fire */
    SPELL_PYROBLAST         = 11366,
    SPELL_COMBUSTION        = 11129,
#ifdef LICH_KING
    SPELL_LIVING_BOMB       = 44457,
    SPELL_FIREBALL          = 42833,
    SPELL_FIRE_BLAST        = 42873,
#else
    SPELL_FIREBALL          = 38692,
    SPELL_FIRE_BLAST        = 27079,
#endif
    SPELL_DRAGONS_BREATH    = 31661,
    SPELL_BLAST_WAVE        = 11113,

    /* Mage - Frost */
    SPELL_ICY_VEINS         = 12472,
    SPELL_ICE_BARRIER       = 11426,
#ifdef LICH_KING
    SPELL_FROST_NOVA        = 42917,
    SPELL_FROSTBOLT         = 42842,
    SPELL_ICE_LANCE         = 42914,
    SPELL_DEEP_FREEZE       = 44572,
#else
    SPELL_FROST_NOVA        = 27088,
    SPELL_FROSTBOLT         = 38697,
    SPELL_ICE_LANCE         = 30455,
#endif
    SPELL_COLD_SNAP         = 11958,

    /* Warlock - Generic */
    SPELL_FEAR                 =  6215,
    SPELL_HOWL_OF_TERROR       = 17928,
#ifdef LICH_KING
    SPELL_CORRUPTION           = 47813, 
    SPELL_DEATH_COIL_W         = 47860,
    SPELL_SHADOW_BOLT          = 47809,
    SPELL_INCINERATE           = 47838,
    SPELL_IMMOLATE             = 47811,
    SPELL_SEED_OF_CORRUPTION   = 47836,
#else
    SPELL_CORRUPTION           = 27216,
    SPELL_DEATH_COIL_W         = 27223,
    SPELL_SHADOW_BOLT          = 27209,
    SPELL_INCINERATE           = 32231,
    SPELL_IMMOLATE             = 27215,
    SPELL_SEED_OF_CORRUPTION   = 27243,
#endif

    /* Warlock - Affliction */
#ifdef LICH_KING
    PASSIVE_SIPHON_LIFE        = 63108,
    SPELL_HAUNT                = 48181,
    SPELL_CURSE_OF_AGONY       = 47864,
    SPELL_DRAIN_SOUL           = 47855,
#else
    SPELL_CURSE_OF_AGONY       = 27218,
    SPELL_DRAIN_SOUL           = 27217,
    AURA_CONTAGION             = 30064,
#endif
    SPELL_UNSTABLE_AFFLICTION  = 30108,

    /* Warlock - Demonology */
    SPELL_SOUL_LINK            = 19028,
#ifdef LICH_KING
    SPELL_METAMORPHOSIS        = 59672,
    SPELL_IMMOLATION_AURA      = 50589,
    SPELL_DEMON_CHARGE         = 54785,
    SPELL_DEMONIC_EMPOWERMENT  = 47193,
    AURA_DECIMATION            = 63167,
    AURA_MOLTEN_CORE           = 71165,
    SPELL_SOUL_FIRE            = 47825,
#else
    SPELL_SOUL_FIRE            = 27211,
    SPELL_SUMMON_FELGUARD      = 30146,
#endif

    /* Warlock - Destruction */
    SPELL_SHADOWBURN           = 17877,
    SPELL_CONFLAGRATE          = 17962,
#ifdef LICH_KING
    SPELL_CHAOS_BOLT           = 50796,
    SPELL_SHADOWFURY           = 47847,
#else
    SPELL_CHAOS_BOLT           = 27209,
    SPELL_SHADOWFURY           = 30414,
#endif

    /* Druid - Generic */
    SPELL_BARKSKIN             = 22812,
    SPELL_INNERVATE            = 29166,

    /* Druid - Balance */
    SPELL_INSECT_SWARM        =  5570,
    SPELL_MOONKIN_FORM        = 24858,
#ifdef LICH_KING
    SPELL_STARFALL            = 48505,
    SPELL_TYPHOON             = 61384,
    SPELL_MOONFIRE            = 48463,
    SPELL_WRATH               = 48461,
    AURA_ECLIPSE_LUNAR        = 48518,
    SPELL_STARFIRE            = 48465,
#else
    SPELL_MOONFIRE            = 26988,
    SPELL_WRATH               = 26985,
    SPELL_STARFIRE            = 26986,
#endif

    /* Druid - Feral */
    SPELL_CAT_FORM            =   768,
#ifdef LICH_KING
    SPELL_SURVIVAL_INSTINCTS  = 61336,
    SPELL_FERAL_CHARGE_CAT    = 49376,
    SPELL_BERSERK             = 50334,
    SPELL_MANGLE_CAT          = 48566,
    SPELL_RAKE                = 48574,
    SPELL_RIP                 = 49800,
    SPELL_SAVAGE_ROAR         = 52610,
    SPELL_TIGER_FURY          = 50213,
    SPELL_CLAW                = 48570,
    SPELL_MAIM                = 49802,
#else
    SPELL_MANGLE_CAT          = 33983,
    SPELL_RAKE                = 27003,
    SPELL_RIP                 = 27008,
    SPELL_TIGER_FURY          = 9846,
    SPELL_CLAW                = 27000,
    SPELL_MAIM                = 22570,
#endif
    SPELL_MANGLE              = 33917,
    SPELL_DASH                = 33357,

    /* Druid - Restoration */
    SPELL_SWIFTMEND           = 18562,
    SPELL_TREE_OF_LIFE        = 33891,
#ifdef LICH_KING
    SPELL_WILD_GROWTH         = 48438,
    SPELL_TRANQUILITY         = 48447,
    SPELL_NOURISH             = 50464,
    SPELL_HEALING_TOUCH       = 48378,
    SPELL_REJUVENATION        = 48441,
    SPELL_REGROWTH            = 48443,
    SPELL_LIFEBLOOM           = 48451,
#else
    SPELL_TRANQUILITY         = 26983,
    SPELL_HEALING_TOUCH       = 26979,
    SPELL_REJUVENATION        = 26982,
    SPELL_REGROWTH            = 26980,
    SPELL_LIFEBLOOM           = 33763,
#endif
    SPELL_NATURE_SWIFTNESS    = 17116,
};

static const uint8 MaximumSpecializationIconicSpells = 3;

// As it turns out, finding out "how many points does the player have in spec X" is actually really expensive to do frequently
// So instead, we just check for a handful of spells that, realistically, no spec is gonna go without (and "has spell" is cheap!)
// Can players deliberately trick this check? Yes.
// Is it worth doing? No.
// Close enough.
static const uint32 PlayerSpecializationIconicSpells[MAX_CLASSES][MAX_TALENT_TREES][MaximumSpecializationIconicSpells] = {
    { // CLASS_NONE
        {0,0,0},
        {0,0,0},
        {0,0,0}
    },
    { // CLASS_WARRIOR
        {SPELL_BLADESTORM, SPELL_MORTAL_STRIKE, SPELL_SWEEPING_STRIKES}, // Arms
#ifdef LICH_KING
        {PASSIVE_TITANS_GRIP, SPELL_BLOODTHIRST, SPELL_DEATH_WISH}, // Fury
        {SPELL_SHOCKWAVE, SPELL_DEVASTATE, SPELL_VIGILANCE} // Protection
#else
        {0, SPELL_BLOODTHIRST, SPELL_DEATH_WISH}, // Fury
        {0, SPELL_DEVASTATE, 0} // Protection
#endif
    },
    { // CLASS_PALADIN
#ifdef LICH_KING
        {SPELL_BEACON_OF_LIGHT, SPELL_HOLY_SHOCK, PASSIVE_ILLUMINATION}, // Holy
        {SPELL_HAMMER_OF_RIGHTEOUS, SPELL_HOLY_SHIELD, SPELL_BLESS_OF_SANC}, // Protection
        {SPELL_DIVINE_STORM, SPELL_CRUSADER_STRIKE, SPELL_SEAL_OF_COMMAND} // Retribution
#else
        {0, SPELL_HOLY_SHOCK, PASSIVE_ILLUMINATION}, // Holy
        {0, SPELL_HOLY_SHIELD, SPELL_BLESS_OF_SANC}, // Protection
        {0, SPELL_CRUSADER_STRIKE, SPELL_SEAL_OF_COMMAND} // Retribution
#endif
    },
    { // CLASS_HUNTER
#ifdef LICH_KING
        {PASSIVE_BEAST_MASTERY, PASSIVE_BEAST_WITHIN, SPELL_BESTIAL_WRATH}, // Beast Mastery
        {SPELL_CHIMERA_SHOT, PASSIVE_TRUESHOT_AURA, SPELL_AIMED_SHOT}, // Marksmanship
        {SPELL_EXPLOSIVE_SHOT, SPELL_WYVERN_STING, PASSIVE_LOCK_AND_LOAD} // Survival
#else
        {0, PASSIVE_BEAST_WITHIN, SPELL_BESTIAL_WRATH}, // Beast Mastery
        {0, PASSIVE_TRUESHOT_AURA, SPELL_AIMED_SHOT}, // Marksmanship
        {0, SPELL_WYVERN_STING, 0} // Survival
#endif
    },
    { // CLASS_ROGUE
#ifdef LICH_KING
        {SPELL_HUNGER_FOR_BLOOD, SPELL_MUTILATE, SPELL_COLD_BLOOD}, // Assassination
        {SPELL_KILLING_SPREE, SPELL_ADRENALINE_RUSH, SPELL_BLADE_FLURRY}, // Combat
        {SPELL_SHADOW_DANCE, SPELL_PREMEDITATION, SPELL_HEMORRHAGE} // Sublety
#else
        {0, SPELL_MUTILATE, SPELL_COLD_BLOOD}, // Assassination
        {0, SPELL_ADRENALINE_RUSH, SPELL_BLADE_FLURRY}, // Combat
        {0, SPELL_PREMEDITATION, SPELL_HEMORRHAGE} // Sublety
#endif
    },
    { // CLASS_PRIEST
#ifdef LICH_KING
        {SPELL_PENANCE, SPELL_POWER_INFUSION, PASSIVE_SOUL_WARDING}, // Discipline
        {SPELL_GUARDIAN_SPIRIT, PASSIVE_SPIRIT_REDEMPTION, SPELL_DESPERATE_PRAYER}, // Holy
        {SPELL_VAMPIRIC_TOUCH, SPELL_SHADOWFORM, SPELL_VAMPIRIC_EMBRACE} // Shadow
#else
        {SPELL_PAIN_SUPPRESSION, SPELL_POWER_INFUSION, 0}, // Discipline
        {0, PASSIVE_SPIRIT_REDEMPTION, SPELL_DESPERATE_PRAYER}, // Holy
        {SPELL_VAMPIRIC_TOUCH, SPELL_SHADOWFORM, SPELL_VAMPIRIC_EMBRACE} // Shadow
#endif
    },
#ifdef LICH_KING
    { // CLASS_DEATH_KNIGHT
        {SPELL_HEART_STRIKE, SPELL_HYSTERIA, SPELL_RUNE_TAP}, // Blood
        {SPELL_HOWLING_BLAST, SPELL_FROST_STRIKE, PASSIVE_ICY_TALONS}, // Frost
        {SPELL_SCOURGE_STRIKE, PASSIVE_MASTER_OF_GHOUL, PASSIVE_UNHOLY_BLIGHT} // Unholy
    },
#else
    { // CLASS_DEATH_KNIGHT
        {0,0,0},
        {0,0,0},
        {0,0,0}
    },
#endif
    { // CLASS_SHAMAN
#ifdef LICH_KING
        {SPELL_THUNDERSTORM, SPELL_TOTEM_OF_WRATH, PASSIVE_ELEMENTAL_FOCUS}, // Elemental
        {SPELL_FERAL_SPIRIT, SPELL_LAVA_LASH, PASSIVE_SPIRIT_WEAPONS}, // Enhancement
        {SPELL_RIPTIDE, SPELL_MANA_TIDE_TOTEM, SPELL_SHA_NATURE_SWIFT} // Restoration
#else
        {0, SPELL_TOTEM_OF_WRATH, PASSIVE_ELEMENTAL_FOCUS}, // Elemental
        {0, SPELL_SHAMANISTIC_RAGE, PASSIVE_SPIRIT_WEAPONS}, // Enhancement
        { SPELL_EARTH_SHIELD, SPELL_MANA_TIDE_TOTEM, SPELL_SHA_NATURE_SWIFT} // Restoration
#endif
    },
    { // CLASS_MAGE
#ifdef LICH_KING
        {SPELL_ARCANE_BARRAGE, SPELL_ARCANE_POWER, SPELL_FOCUS_MAGIC}, // Arcane
        {SPELL_LIVING_BOMB, SPELL_COMBUSTION, SPELL_PYROBLAST}, // Fire
        {SPELL_DEEP_FREEZE, SPELL_ICE_BARRIER, SPELL_ICY_VEINS} // Frost
#else
        {0, SPELL_ARCANE_POWER, 31588}, // Arcane
        {0, SPELL_COMBUSTION, SPELL_PYROBLAST}, // Fire
        {0, SPELL_ICE_BARRIER, SPELL_ICY_VEINS} // Frost
#endif
    },
    { // CLASS_WARLOCK
#ifdef LICH_KING
        {SPELL_HAUNT, SPELL_UNSTABLE_AFFLICTION, PASSIVE_SIPHON_LIFE}, // Affliction
        {SPELL_METAMORPHOSIS, SPELL_DEMONIC_EMPOWERMENT, SPELL_SOUL_LINK}, // Demonology
        {SPELL_CHAOS_BOLT, SPELL_CONFLAGRATE, SPELL_SHADOWBURN} // Destruction
#else
        {0, SPELL_UNSTABLE_AFFLICTION, AURA_CONTAGION}, // Affliction
        {0, SPELL_SUMMON_FELGUARD, SPELL_SOUL_LINK}, // Demonology
        {SPELL_CHAOS_BOLT, SPELL_CONFLAGRATE, SPELL_SHADOWBURN} // Destruction
#endif
    },
    { // CLASS_UNK
        {0,0,0},
        {0,0,0},
        {0,0,0}
    },
    { // CLASS_DRUID
#ifdef LICH_KING
        {SPELL_STARFALL, SPELL_MOONKIN_FORM, SPELL_INSECT_SWARM}, // Balance
        {SPELL_BERSERK, SPELL_MANGLE, SPELL_SURVIVAL_INSTINCTS}, // Feral
        {SPELL_WILD_GROWTH, SPELL_TREE_OF_LIFE, SPELL_SWIFTMEND} // Restoration
#else
        {0, SPELL_MOONKIN_FORM, SPELL_INSECT_SWARM}, // Balance
        {0, SPELL_MANGLE, 0}, // Feral
        {0, SPELL_TREE_OF_LIFE, SPELL_SWIFTMEND} // Restoration
#endif
    }
};

uint8 Trinity::Helpers::Entity::GetPlayerSpecialization(Player const* who)
{
    if (!who)
        return 0;
     uint8 playerClass = who->GetClass();
    for (uint8 tier = 0; tier < MaximumSpecializationIconicSpells; ++tier)
    {
        for (uint8 tree = 0; tree < MAX_TALENT_TREES; ++tree)
            if (PlayerSpecializationIconicSpells[playerClass][tree][tier] && who->HasSpell(PlayerSpecializationIconicSpells[playerClass][tree][tier]))
                return tree;
    }
     return 0;
}

 bool Trinity::Helpers::Entity::IsPlayerHealer(Player const* who)
{
    if (!who)
        return false;
     switch (who->GetClass())
    {
        case CLASS_WARRIOR:
        case CLASS_HUNTER:
        case CLASS_ROGUE:
        case CLASS_DEATH_KNIGHT:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
        default:
            return false;
        case CLASS_PALADIN:
            return (Trinity::Helpers::Entity::GetPlayerSpecialization(who) == SPEC_PALADIN_HOLY);
        case CLASS_PRIEST:
            return (Trinity::Helpers::Entity::GetPlayerSpecialization(who) != SPEC_PRIEST_SHADOW);
        case CLASS_SHAMAN:
            return (Trinity::Helpers::Entity::GetPlayerSpecialization(who) == SPEC_SHAMAN_RESTORATION);
        case CLASS_DRUID:
            return (Trinity::Helpers::Entity::GetPlayerSpecialization(who) == SPEC_DRUID_RESTORATION);
    }
}
 bool Trinity::Helpers::Entity::IsPlayerRangedAttacker(Player const* who)
{
    if (!who)
        return false;

     switch (who->GetClass())
    {
        case CLASS_WARRIOR:
        case CLASS_PALADIN:
        case CLASS_ROGUE:
        case CLASS_DEATH_KNIGHT:
        default:
            return false;
        case CLASS_MAGE:
        case CLASS_WARLOCK:
            return true;
        case CLASS_HUNTER:
        {
            // check if we have a ranged weapon equipped
            Item const* rangedSlot = who->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
            if (ItemTemplate const* rangedTemplate = rangedSlot ? rangedSlot->GetTemplate() : nullptr)
            {
                if ((1 << rangedTemplate->SubClass) & ITEM_SUBCLASS_MASK_WEAPON_RANGED)
                    return true;
            }
            return false;
        }
        case CLASS_PRIEST:
            return (Trinity::Helpers::Entity::GetPlayerSpecialization(who) == SPEC_PRIEST_SHADOW);
        case CLASS_SHAMAN:
            return (Trinity::Helpers::Entity::GetPlayerSpecialization(who) == SPEC_SHAMAN_ELEMENTAL);
        case CLASS_DRUID:
            return (Trinity::Helpers::Entity::GetPlayerSpecialization(who) == SPEC_DRUID_BALANCE);
    }
}
