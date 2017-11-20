#ifndef H_CLASS_SPELLS
#define H_CLASS_SPELLS

namespace Talents
{
    enum Druid
    {
        //Balance
        //Feral Combat
        //Restoration
    };

    enum Hunter
    {
        //Beast Mastery
        //Marksmanship
        //Survival
    };

    enum Mage
    {
        //Arcane
        //Fire
        //Frost
    };

    enum Paladin
    {
        //Holy
        //Protection
        //Retribution
    };

    enum Priest
    {
        //Discipline
        WAND_SPEC_RNK_1 = 14524,
        WAND_SPEC_RNK_2 = 14525,
        WAND_SPEC_RNK_3 = 14526,
        WAND_SPEC_RNK_4 = 14527,
        WAND_SPEC_RNK_5 = 14528,

        //Holy

        //Shadow
    };

    enum Rogue
    {
        //Assassination
        //Combat
        //Subtlety
    };

    enum Shaman
    {
        //Elemental
        //Enhancement
        //Restoration
    };

    enum Warlock
    {
        //Affliction
        //Demonology
        //Destruction
    };

    enum Warrior
    {
        //Arms
        //Fury
        //Protection
    };
};

namespace ClassSpells
{
    enum Druid
    {
        //Balance
        //Feral Combat
        //Restoration
    };

    enum Hunter
    {
        //Beast Mastery
        //Marksmanship
        //Survival
    };

    enum Mage
    {
        //Arcane
        //Fire
        //Frost
    };

    enum Paladin
    {
        //Holy
        //Protection
        //Retribution
    };

    enum Priest
    {
        //Generic
        WAND = 5019,

        //Discipline
        //Holy
        //Shadow
    };

    enum Rogue
    {
        //Assassination
        //Combat
        //Subtlety
    };

    enum Shaman
    {
        //Elemental
        //Enhancement
        //Restoration
    };

    enum Warlock
    {
        //Affliction
        //Demonology
        //Destruction
    };

    enum Warrior
    {
		// Generic
		DUAL_WIELD = 674,
		PARRY = 3127,
		PLATE_MAIL = 750,

		// Arms
		CHARGE_RNK_1 = 100,
		CHARGE_RNK_2 = 6178,
		CHARGE_RNK_3 = 11578,
		HAMSTRING_RNK_1 = 1715,
		HAMSTRING_RNK_2 = 7372,
		HAMSTRING_RNK_3 = 7373,
		HAMSTRING_RNK_4 = 25212,
		HEROIC_STRIKE_RNK_1 = 78,
		HEROIC_STRIKE_RNK_2 = 284,
		HEROIC_STRIKE_RNK_3 = 285,
		HEROIC_STRIKE_RNK_4 = 1608,
		HEROIC_STRIKE_RNK_5 = 11564,
		HEROIC_STRIKE_RNK_6 = 11565,
		HEROIC_STRIKE_RNK_7 = 11566,
		HEROIC_STRIKE_RNK_8 = 11567,
		HEROIC_STRIKE_RNK_9 = 25286,
		HEROIC_STRIKE_RNK_10 = 29707,
		MOCKING_BLOW_RNK_1 = 694,
		MOCKING_BLOW_RNK_2 = 7400,
		MOCKING_BLOW_RNK_3 = 7402,
		MOCKING_BLOW_RNK_4 = 20559,
		MOCKING_BLOW_RNK_5 = 20560,
		MOCKING_BLOW_RNK_6 = 25266,
		MORTAL_STRIKE_RNK_1 = 12294,
		MORTAL_STRIKE_RNK_2 = 21551,
		MORTAL_STRIKE_RNK_3 = 21552,
		MORTAL_STRIKE_RNK_4 = 21553,
		MORTAL_STRIKE_RNK_5 = 25248,
		MORTAL_STRIKE_RNK_6 = 30330,
		OVERPOWER_RNK_1 = 7384,
		OVERPOWER_RNK_2 = 7887,
		OVERPOWER_RNK_3 = 11584,
		OVERPOWER_RNK_4 = 11585,
		REND_RNK_1 = 772,
		REND_RNK_2 = 6546,
		REND_RNK_3 = 6547,
		REND_RNK_4 = 6548,
		REND_RNK_5 = 11572,
		REND_RNK_6 = 11573,
		REND_RNK_7 = 11574,
		REND_RNK_8 = 25208,
		RETALIATION_RNK_1 = 20230,
		SLAM_RNK_1 = 1464,
		SLAM_RNK_2 = 8820,
		SLAM_RNK_3 = 11604,
		SLAM_RNK_4 = 11605,
		SLAM_RNK_5 = 25241,
		SLAM_RNK_6 = 25242,
		THUNDER_CLAP_RNK_1 = 6343,
		THUNDER_CLAP_RNK_2 = 8198,
		THUNDER_CLAP_RNK_3 = 8204,
		THUNDER_CLAP_RNK_4 = 8205,
		THUNDER_CLAP_RNK_5 = 11580,
		THUNDER_CLAP_RNK_6 = 11581,
		THUNDER_CLAP_RNK_7 = 25264,

		// Fury
		BATTLE_SHOUT_RNK_1 = 6673,
		BATTLE_SHOUT_RNK_2 = 5242,
		BATTLE_SHOUT_RNK_3 = 6192,
		BATTLE_SHOUT_RNK_4 = 11549,
		BATTLE_SHOUT_RNK_5 = 11550,
		BATTLE_SHOUT_RNK_6 = 11551,
		BATTLE_SHOUT_RNK_7 = 25289,
		BATTLE_SHOUT_RNK_8 = 2048,
		BERSERKER_RAGE_RNK_1 = 18499,
		BERSERKER_STANCE_RNK_1 = 2458,
		BLOODTHIRST_RNK_1 = 23881,
		BLOODTHIRST_RNK_2 = 23892,
		BLOODTHIRST_RNK_3 = 23893,
		BLOODTHIRST_RNK_4 = 23894,
		BLOODTHIRST_RNK_5 = 25251,
		BLOODTHIRST_RNK_6 = 30335,
		CHALLENGING_SHOUT_RNK_1 = 1161,
		CLEAVE_RNK_1 = 845,
		CLEAVE_RNK_2 = 7369,
		CLEAVE_RNK_3 = 11608,
		CLEAVE_RNK_4 = 11609,
		CLEAVE_RNK_5 = 20569,
		CLEAVE_RNK_6 = 25231,
		COMMANDING_SHOUT_RNK_1 = 469,
		DEMORALIZING_SHOUT_RNK_1 = 1160,
		DEMORALIZING_SHOUT_RNK_2 = 6190,
		DEMORALIZING_SHOUT_RNK_3 = 11554,
		DEMORALIZING_SHOUT_RNK_4 = 11555,
		DEMORALIZING_SHOUT_RNK_5 = 11556,
		DEMORALIZING_SHOUT_RNK_6 = 25202,
		DEMORALIZING_SHOUT_RNK_7 = 25203,
		EXECUTE_RNK_1 = 5308,
		EXECUTE_RNK_2 = 20658,
		EXECUTE_RNK_3 = 20660,
		EXECUTE_RNK_4 = 20661,
		EXECUTE_RNK_5 = 20662,
		EXECUTE_RNK_6 = 25234,
		EXECUTE_RNK_7 = 25236,
		INTERCEPT_RNK_1 = 20252,
		INTERCEPT_RNK_2 = 20616,
		INTERCEPT_RNK_3 = 20617,
		INTERCEPT_RNK_4 = 25272,
		INTERCEPT_RNK_5 = 25275,
		INTIMIDATING_SHOUT_RNK_1 = 5246,
		PUMMEL_RNK_1 = 6552,
		PUMMEL_RNK_2 = 6554,
		RAMPAGE_RNK_1 = 29801,
		RAMPAGE_RNK_2 = 30030,
		RAMPAGE_RNK_3 = 30033,
		RECKLESSNESS_RNK_1 = 1719,
		VICTORY_RUSH_RNK_1 = 34428,
		WHIRLWIND_RNK_1 = 1680,

		// Protection
		BLOODRAGE_RNK_1 = 2687,
		DEFENSIVE_STANCE_RNK_1 = 71,
		DEVASTATE_RNK_1 = 20243,
		DEVASTATE_RNK_2 = 30016,
		DEVASTATE_RNK_3 = 30022,
		DISARM_RNK_1 = 676,
		INTERVENE_RNK_1 = 3411,
		REVENGE_RNK_1 = 6572,
		REVENGE_RNK_2 = 6574,
		REVENGE_RNK_3 = 7379,
		REVENGE_RNK_4 = 11600,
		REVENGE_RNK_5 = 11601,
		REVENGE_RNK_6 = 25288,
		REVENGE_RNK_7 = 25269,
		REVENGE_RNK_8 = 30357,
		SHIELD_BASH_RNK_1 = 72,
		SHIELD_BASH_RNK_2 = 1671,
		SHIELD_BASH_RNK_3 = 1672,
		SHIELD_BASH_RNK_4 = 29704,
		SHIELD_BLOCK_RNK_1 = 2565,
		SHIELD_SLAM_RNK_1 = 23922,
		SHIELD_SLAM_RNK_2 = 23923,
		SHIELD_SLAM_RNK_3 = 23924,
		SHIELD_SLAM_RNK_4 = 23925,
		SHIELD_SLAM_RNK_5 = 25258,
		SHIELD_SLAM_RNK_6 = 30356,
		SHIELD_WALL_RNK_1 = 871,
		SPELL_REFLECTION_RNK_1 = 23920,
		STANCE_MASTERY_RNK_1 = 12678,
		SUNDER_ARMOR_RNK_1 = 7386,
		SUNDER_ARMOR_RNK_2 = 7405,
		SUNDER_ARMOR_RNK_3 = 8380,
		SUNDER_ARMOR_RNK_4 = 11596,
		SUNDER_ARMOR_RNK_5 = 11597,
		SUNDER_ARMOR_RNK_6 = 25225,
		TAUNT_RNK_1 = 355,
    };
};

#endif // H_CLASS_SPELLS