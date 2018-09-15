#include "CommonHelpers.h"
#include "Common.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Player.h"
#include "SharedDefines.h"

enum PlayerSpecializationIconicSpellIds
{
    /* Warrior - Arms */
    SPELL_SWEEPING_STRIKES = 12328,
    SPELL_MORTAL_STRIKE = 12294,
#ifdef LICH_KING
    SPELL_BLADESTORM = 46924,
#endif

    /* Warrior - Fury */
    SPELL_DEATH_WISH = 12292,
    SPELL_BLOODTHIRST = 23881,
#ifdef LICH_KING
    PASSIVE_TITANS_GRIP = 46917,
#endif

    /* Warrior - Protection */
    SPELL_VIGILANCE = 50720,
    SPELL_DEVASTATE = 20243,
#ifdef LICH_KING
    SPELL_SHOCKWAVE = 46968,
#endif

    /* Paladin - Holy*/
    PASSIVE_ILLUMINATION = 20215,
    SPELL_HOLY_SHOCK = 20473,
#ifdef LICH_KING
    SPELL_BEACON_OF_LIGHT = 53563,
#endif

    /* Paladin - Protection */
    SPELL_BLESS_OF_SANC = 20911,
    SPELL_HOLY_SHIELD = 20925,
#ifdef LICH_KING
    SPELL_HAMMER_OF_RIGHTEOUS = 53595,
#endif

    /* Paladin - Retribution */
    SPELL_SEAL_OF_COMMAND = 20375,
    SPELL_CRUSADER_STRIKE = 35395,
#ifdef LICH_KING
    SPELL_DIVINE_STORM = 53385,
#endif

    /* Hunter - Beast Mastery */
    SPELL_BESTIAL_WRATH = 19574,
    PASSIVE_BEAST_WITHIN = 34692,
#ifdef LICH_KING
    PASSIVE_BEAST_MASTERY = 53270,
#endif

    /* Hunter - Marksmanship */
    SPELL_AIMED_SHOT = 19434,
    PASSIVE_TRUESHOT_AURA = 19506,
#ifdef LICH_KING
    SPELL_CHIMERA_SHOT = 53209,
#endif

    /* Hunter - Survival */
    PASSIVE_LOCK_AND_LOAD = 56344,
    SPELL_WYVERN_STING = 19386,
#ifdef LICH_KING
    SPELL_EXPLOSIVE_SHOT = 53301,
#endif

    /* Rogue - Assassination */
    SPELL_COLD_BLOOD = 14177,
    SPELL_MUTILATE = 1329,
#ifdef LICH_KING
    SPELL_HUNGER_FOR_BLOOD = 51662,
#endif

    /* Rogue - Combat */
    SPELL_BLADE_FLURRY = 13877,
    SPELL_ADRENALINE_RUSH = 13750,
#ifdef LICH_KING
    SPELL_KILLING_SPREE = 51690,
#endif

    /* Rogue - Sublety */
    SPELL_HEMORRHAGE = 16511,
    SPELL_PREMEDITATION = 14183,
#ifdef LICH_KING
    SPELL_SHADOW_DANCE = 51713,
#endif

    /* Priest - Discipline */
    PASSIVE_SOUL_WARDING = 63574,
    SPELL_POWER_INFUSION = 10060,
#ifdef LICH_KING
    SPELL_PENANCE = 47540,
#else
    SPELL_PAIN_SUPPRESSION = 33206,
#endif

    /* Priest - Holy */
    PASSIVE_SPIRIT_REDEMPTION = 20711,
    SPELL_DESPERATE_PRAYER = 19236,
#ifdef LICH_KING
    SPELL_GUARDIAN_SPIRIT = 47788,
#endif

    /* Priest - Shadow */
    SPELL_VAMPIRIC_EMBRACE = 15286,
    SPELL_SHADOWFORM = 15473,
    SPELL_VAMPIRIC_TOUCH = 34914,

#ifdef LICH_KING
    /* Death Knight - Blood */
    SPELL_RUNE_TAP = 48982,
    SPELL_HYSTERIA = 49016,
    SPELL_HEART_STRIKE = 55050,

    /* Death Knight - Frost */
    PASSIVE_ICY_TALONS = 50887,
    SPELL_FROST_STRIKE = 49143,
    SPELL_HOWLING_BLAST = 49184,

    /* Death Knight - Unholy */
    PASSIVE_UNHOLY_BLIGHT = 49194,
    PASSIVE_MASTER_OF_GHOUL = 52143,
    SPELL_SCOURGE_STRIKE = 55090,
#endif

    /* Shaman - Elemental*/
    PASSIVE_ELEMENTAL_FOCUS = 16164,
    SPELL_TOTEM_OF_WRATH = 30706,
#ifdef LICH_KING
    SPELL_THUNDERSTORM = 51490,
#endif

    /* Shaman - Enhancement */
    PASSIVE_SPIRIT_WEAPONS = 16268,
#ifdef LICH_KING
    SPELL_LAVA_LASH = 60103,
    SPELL_FERAL_SPIRIT = 51533,
#else
    SPELL_STORMSTRIKE = 17364,
    SPELL_SHAMANISTIC_RAGE = 30823,
#endif

    /* Shaman - Restoration*/
    SPELL_SHA_NATURE_SWIFT = 591,
    SPELL_MANA_TIDE_TOTEM = 590,
#ifdef LICH_KING
    SPELL_RIPTIDE = 61295,
#else
    SPELL_EARTH_SHIELD = 32594,
#endif

    /* Mage - Arcane */
    SPELL_ARCANE_POWER = 12042,
#ifdef LICH_KING
    SPELL_FOCUS_MAGIC = 54646,
    SPELL_ARCANE_BARRAGE = 44425,
#endif

    /* Mage - Fire */
    SPELL_PYROBLAST = 11366,
    SPELL_COMBUSTION = 11129,
#ifdef LICH_KING
    SPELL_LIVING_BOMB = 44457,
#endif

    /* Mage - Frost */
    SPELL_ICY_VEINS = 12472,
    SPELL_ICE_BARRIER = 11426,
#ifdef LICH_KING
    SPELL_DEEP_FREEZE = 44572,
#endif

    /* Warlock - Affliction */
    PASSIVE_SIPHON_LIFE = 63108,
    SPELL_UNSTABLE_AFFLICTION = 30108,
#ifdef LICH_KING
    SPELL_HAUNT = 48181,
#else
    AURA_CONTAGION = 30064,
#endif

    /* Warlock - Demonology */
    SPELL_SOUL_LINK = 19028,
#ifdef LICH_KING
    SPELL_DEMONIC_EMPOWERMENT = 47193,
    SPELL_METAMORPHOSIS = 59672,
#else
    SPELL_SUMMON_FELGUARD = 30146,
#endif

    /* Warlock - Destruction */
    SPELL_SHADOWBURN = 17877,
    SPELL_CONFLAGRATE = 17962,
#ifdef LICH_KING
    SPELL_CHAOS_BOLT = 50796,
#else
    SPELL_CHAOS_BOLT = 27209,
#endif

    /* Druid - Balance */
    SPELL_INSECT_SWARM = 5570,
    SPELL_MOONKIN_FORM = 24858,
#ifdef LICH_KING
    SPELL_STARFALL = 48505,
#endif

    /* Druid - Feral */
    SPELL_SURVIVAL_INSTINCTS = 61336,
    SPELL_MANGLE = 33917,
#ifdef LICH_KING
    SPELL_BERSERK = 50334,
#endif

    /* Druid - Restoration */
    SPELL_SWIFTMEND = 18562,
    SPELL_TREE_OF_LIFE = 33891,
#ifdef LICH_KING
    SPELL_WILD_GROWTH = 48438,
#endif
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
#ifdef LICH_KING
        {SPELL_BLADESTORM, SPELL_MORTAL_STRIKE, SPELL_SWEEPING_STRIKES}, // Arms
        {PASSIVE_TITANS_GRIP, SPELL_BLOODTHIRST, SPELL_DEATH_WISH}, // Fury
        {SPELL_SHOCKWAVE, SPELL_DEVASTATE, SPELL_VIGILANCE} // Protection
#else
        {0, SPELL_MORTAL_STRIKE, SPELL_SWEEPING_STRIKES}, // Arms
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
#else
        {0, SPELL_UNSTABLE_AFFLICTION, AURA_CONTAGION}, // Affliction
        { SPELL_SUMMON_FELGUARD, 0, SPELL_SOUL_LINK}, // Demonology
#endif
        {SPELL_CHAOS_BOLT, SPELL_CONFLAGRATE, SPELL_SHADOWBURN} // Destruction
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
