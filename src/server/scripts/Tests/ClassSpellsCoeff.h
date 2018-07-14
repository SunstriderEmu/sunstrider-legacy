#ifndef H_CLASS_SPELLS_COEFF
#define H_CLASS_SPELLS_COEFF

namespace ClassSpellsCoeff
{
    namespace Druid
    {
        // Balance
        const float ENTANGLING_ROOTS = 0.9f; // DrDamage & WoWWiki
        const float FORCE_OF_NATURE = 0.85f;
        const float HURRICANE = 0.128f;
        const float INSECT_SWARM = 0.76f;
        const float MOONFIRE = 0.15f;
        const float MOONFIRE_DOT = 0.52f;
        const float STARFIRE = 1.0f;
        const float WRATH = 0.571f;

        // Restoration
        const float HEALING_TOUCH = 1.0f;
        const float LIFEBLOOM = 0.3429f; // DrDamage & WoWWiki
        const float LIFEBLOOM_HOT = 0.51983f; // DrDamage
        // Choosing 30% and 70% for Regrowth seems to be the best couple
        // after reviewing WoWWiki talks and DrDamage
        const float REGROWTH = 0.3f;
        const float REGROWTH_HOT = 0.7f;
        const float REJUVENATION = 0.8f;
        const float SWIFTMEND_REJUVENATION = 1.0f;
        const float SWIFTMEND_REGROWTH = 0.65f;
        const float TRANQUILITY_LVL_70 = 0.762f; // DrDamage calculated by castTime / 3.5 / 3
    };

    namespace Mage
    {
        // Arcane
        const float ARCANE_BLAST = 0.7143f;
        const float ARCANE_EXPLOSION = 0.2128f;
        const float ARCANE_MISSILES = 1.4285f;
        const float MANA_SHIELD = 0.5f;
            
        // Fire
        const float BLAST_WAVE = 0.1357f;
        const float DRAGONS_BREATH = 0.1357f;
        const float FIRE_BLAST = 0.4286f;
        const float FIREBALL = 1.0f;
        const float FIREBALL_DOT = 0.f;
        const float FLAMESTRIKE = 0.1761f;
        const float FLAMESTRIKE_DOT = 0.1096f;
        const float PYROBLAST = 0.115f;
        const float PYROBLAST_DOT = 0.2f;
        const float SCORCH = 0.4286f;

        // Frost
        const float BLIZZARD = 0.7619f;
        const float CONE_OF_COLD = 0.1357f;
        const float FROST_NOVA = 0.1357f;
        const float FROSTBOLT = 0.8143f;
        const float ICE_BARRIER = 0.1f;
        const float ICE_LANCE = 0.1429f;
    };

    namespace Paladin
    {
        // Holy
        const float CONSECRATION = 0.9524f;
        const float EXORCISM = 0.4286f;
        const float FLASH_OF_LIGHT = 0.4286f;
        const float HAMMER_OF_WRATH = 0.4286f;
        const float HOLY_LIGHT = 0.7143f;
        const float HOLY_SHOCK = 0.4286f; // Heal & damage
        const float HOLY_WRATH = 0.1905f;
        const float SEAL_OF_RIGHTEOUSNESS_1H = 0.92f; // 1.0 weapon speed
        const float SEAL_OF_RIGHTEOUSNESS_2H = 0.108f; // 1.0 weapon speed
        const float JUDGEMENT_OF_RIGHTEOUSNESS = 0.7143f;

        // Protection
        const float AVENGERS_SHIELD = 0.1327f;
        const float HOLY_SHIELD = 0.5f;

        // Retribution
        const float JUDGEMENT_OF_BLOOD = 0.43f;
        const float SEAL_OF_COMMAND = 0.2f;
        const float JUDGEMENT_OF_COMMAND = 0.4286f;
        const float SEAL_OF_VENGEANCE = 0.17f;
        const float JUDGEMENT_OF_VENGEANCE = 0.4286f;
    };

    namespace Priest
    {
        // Discipline
        const float FEEDBACK = 0.f;
        const float MANA_BURN = 0.f;
        const float POWER_WORD_SHIELD = 0.3f;
        const float STARSHARDS = 0.835f;

        // Holy
        const float BINDING_HEAL = 0.4286f;
        const float CIRCLE_OF_HEALING = 0.214f; // DrDamage
        const float DESPERATE_PRAYER = 0.4286f;
        const float FLASH_HEAL = 0.4286f;
        const float GREATER_HEAL = 0.8571f;
        const float HEAL = 0.551f;
        const float HOLY_FIRE = 0.8571f;
        const float HOLY_FIRE_DOT = 0.165f;
        const float HOLY_NOVA = 0.163f; // Damage & Heal -- DrDamage
        const float PRAYER_OF_HEALING = 0.4285f;
        const float PRAYER_OF_MENDING = 0.4286f;
        const float RENEW = 1.0f;
        const float SMITE = 0.7143f;

        // Shadow
        const float DEVOURING_PLAGUE = 0.8f;
        const float MIND_BLAST = 0.4286f;
        const float MIND_FLAY = 0.57f; // 19 per tick
        const float MIND_FLAY_TICK = 0.19f;
        const float SHADOW_GUARD = 0.8009f;
        const float SHADOW_WORD_DEATH = 0.4286f;
        const float SHADOW_WORD_PAIN = 1.1f;
        const float SHADOWFIEND = 0.65f;
        const float TOUCH_OF_WEAKNESS = 0.1f;
        const float VAMPIRIC_TOUCH = 1.0f;
    };

    namespace Shaman
    {
        // Elemental
        const float CHAIN_LIGHTNING = 0.125f;
        const float CHAIN_LIGHTNING_1_JUMP = 0.7143f;
        const float CHAIN_LIGHTNING_2_JUMP = 0.3571f;
        const float CHAIN_LIGHTNING_3_JUMP = 0.1786f;
        const float EARTH_SHOCK = 0.4286f;
        const float FIRE_NOVA_TOTEM = 0.214f;
        const float FLAME_SHOCK = 0.15f;
        const float FLAME_SHOCK_DOT = 0.52f;
        const float FROST_SHOCK = 0.4286f;
        const float LIGHTNING_BOLT = 0.794f;
        const float MAGMA_TOTEM = 0.667f;
        const float SEARING_TOTEM_TICK = 0.1667f;

        // Enhancement
        const float FLAMETONGUE_WEAPON_HIT = 0.1f;
        const float FROSTBRAND_WEAPON = 0.1f;
        const float LIGHTNING_SHIELD = 1.0f;

        // Restoration
        const float CHAIN_HEAL = 0.125f;
        const float CHAIN_HEAL_1_JUMP = 0.7143f;
        const float CHAIN_HEAL_2_JUMP = 0.3571f;
        const float CHAIN_HEAL_3_JUMP = 0.1786f;
        const float EARTH_SHIELD = 0.2857f;
        const float HEALING_STREAM_TOTEM_TICK = 0.45f;
        const float HEALING_WAVE = 0.8571f;
        const float LESSER_HEALING_WAVE = 0.4286f;
    };

    namespace Warlock
    {
        // Affliction
        const float CORRUPTION = 0.936f;
        const float CURSE_OF_AGONY = 1.2f;
        const float CURSE_OF_DOOM = 2.0f;
        const float DARK_PACT = 0.96f; // WoWWiki
        const float DRAIN_LIFE = 0.7143f;
        const float DRAIN_MANA = 0.0f;
        const float DRAIN_SOUL = 2.143f;
        const float LIFE_TAP = 0.8f;
        const float SEED_OF_CORRUPTION = 0.22f;
        const float SEED_OF_CORRUPTION_DOT = 1.5f;
        const float SIPHON_LIFE = 1.0f;
        const float UNSTABLE_AFFLICTION = 1.8f;
        const float UNSTABLE_AFFLICTION_DOT = 1.2f;

        // Demonology
        const float HEALTH_FUNNEL_HEAL = 0.2857f;

        // Destruction
        const float CONFLAGRATE = 0.4286f;
        const float DEATH_COIL = 0.22f;
        const float HELLFIRE_SELF = 0.14286f;
        const float HELLFIRE_ENEMIES = 0.21429f;
        const float IMMOLATE = 0.2f;
        const float IMMOLATE_DOT = 0.65f;
        const float INCINERATE = 0.7143f;
        const float RAIN_OF_FIRE = 0.952f;
        const float SEARING_PAIN = 0.4286f;
        const float SHADOW_BOLT = 0.8571f;
        const float SHADOWBURN = 0.4286f;
        const float SHADOWFURY = 0.193f;
        const float SHADOW_WARD = 0.3f;
        const float SOUL_FIRE = 0.115f;
    };
};

#endif // H_CLASS_SPELLS_COEFF