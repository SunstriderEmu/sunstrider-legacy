#ifndef H_CLASS_SPELLS_COEFF
#define H_CLASS_SPELLS_COEFF

namespace ClassSpellsCoeff
{
	namespace Druid
	{
		const float ENTANGLING_ROOTS = 10.0f;
		const float FORCE_OF_NATURE = 8.5f;
		const float HURRICANE = 128.0f;
		const float INSECT_SWARM = 76.0f;
		const float MOONFIRE = 15.0f;
		const float MOONFIRE_DOT = 52.0f;
		const float STARFIRE = 100.0f;
		const float WRATH = 57.1f;

		// Restoration
		const float HEALING_TOUCH = 100.0f;
		const float LIFEBLOOM = 34.29f;
		const float LIFEBLOOM_HOT = 51.8f;
		const float REGROWTH = 30.0f;
		const float REGROWTH_HOT = 70.0f;
		const float REJUVENATION = 80.0f;
		const float SWIFTMEND_REJUVENATION = 100.0f;
		const float SWIFTMEND_REGROWTH = 65.0f;
		const float TRANQUILITY_LVL_70 = 73.0f; // scales with level
	};

	namespace Mage
	{
		// Arcane
		const float ARCANE_BLAST = 71.43f;
		const float ARCANE_EXPLOSION = 21.28f;
		const float ARCANE_MISSILES = 142.86f;
		const float MANA_SHIELD = 50.0f;
			
		// Fire
		const float BLAST_WAVE = 13.57f;
		const float DRAGONS_BREATH = 13.57f;
		const float FIRE_BLAST = 42.86f;
		const float FIREBALL = 100.0f;
		const float FIREBALL_DOT = 0.0f;
		const float FLAMESTRIKE = 17.61f;
		const float FLAMESTRIKE_DOT = 10.96f;
		const float PYROBLAST = 115.0f;
		const float PYROBLAST_DOT = 20.0f;
		const float SCORCH = 42.86f;

		// Frost
		const float BLIZZARD = 76.19f;
		const float CONE_OF_COLD = 13.57f;
		const float FROST_NOVA = 13.57f;
		const float FROSTBOLT = 81.43f;
		const float ICE_BARRIER = 10.0f;
		const float ICE_LANCE = 14.29f;
	};

	namespace Paladin
	{
		// Holy
		const float CONSECRATION = 95.24f;
		const float EXORCISM = 42.86f;
		const float FLASH_OF_LIGHT = 42.86f;
		const float HAMMER_OF_WRATH = 42.86f;
		const float HOLY_LIGHT = 71.43f;
		const float HOLY_SHOCK = 42.86f; // Heal & damage
		const float HOLY_WRATH = 19.05f;
		const float SEAL_OF_RIGHTEOUSNESS_1H = 9.2f; // 1.0 weapon speed
		const float SEAL_OF_RIGHTEOUSNESS_2H = 10.8f; // 1.0 weapon speed
		const float JUDGEMENT_OF_RIGHTEOUSNESS = 71.43f;

		// Protection
		const float AVENGERS_SHIELD = 13.27f;
		const float HOLY_SHIELD = 5.0f;

		// Retribution
		const float JUDGEMENT_OF_BLOOD = 43.0f;
		const float SEAL_OF_COMMAND = 20.0f;
		const float JUDGEMENT_OF_COMMAND = 42.86f;
		const float SEAL_OF_VENGEANCE = 17.0f;
		const float JUDGEMENT_OF_VENGEANCE = 42.86f;
	};

	namespace Priest
	{
		// Discipline
		const float FEEDBACK = 0.0f;
		const float MANA_BURN = 0.0f;
		const float STARSHARDS = 83.5f;

		// Holy
		const float BINDING_HEAL = 42.86f;
		const float CIRCLE_OF_HEALING = 21.4f;
		const float DESPERATE_PRAYER = 42.86f;
		const float FLASH_HEAL = 42.86f;
		const float GREATER_HEAL = 85.71f;
		const float HOLY_FIRE = 85.71f;
		const float HOLY_FIRE_DOT = 16.5f;
		const float HOLY_NOVA = 16.0f; // Damage & Heal
		const float PRAYER_OF_HEALING = 42.85f;
		const float PRAYER_OF_MENDING = 42.86f;
		const float RENEW = 100.0f;
		const float SMITE = 71.43f;

		// Shadow
		const float DEVOURING_PLAGUE = 80.0f;
		const float MIND_BLAST = 42.86f;
		const float MIND_FLAY = 57.0f; // 19 per tick
		const float MIND_FLAY_TICK = 19.0f;
		const float SHADOW_GUARD = 80.09f;
		const float SHADOW_WORD_DEATH = 42.86f;
		const float SHADOW_WORD_PAIN = 110.0f;
		const float SHADOWFIEND = 65.0f;
		const float TOUCH_OF_WEAKNESS = 10.0f;
		const float VAMPIRIC_TOUCH = 100.0f;
	};

	namespace Shaman
	{
		// Elemental
		const float CHAIN_LIGHTNING = 125.0f;
		const float CHAIN_LIGHTNING_1_JUMP = 71.43f;
		const float CHAIN_LIGHTNING_2_JUMP = 35.71f;
		const float CHAIN_LIGHTNING_3_JUMP = 17.86f;
		const float EARTH_SHOCK = 42.86f;
		const float FIRE_NOVA_TOTEM = 21.4f;
		const float FLAME_SHOCK = 15.0f;
		const float FLAME_SHOCK_DOT = 52.0f;
		const float FROST_SHOCK = 42.86f;
		const float LIGHTNING_BOLT = 79.4f;
		const float MAGMA_TOTEM = 66.7f;
		const float SEARING_TOTEM_TICK = 16.67f;

		// Enhancement
		const float FLAMETONGUE_WEAPON_HIT = 10.0f;
		const float FROSTBRAND_WEAPON = 10.0f;
		const float LIGHTNING_SHIELD = 100.0f;

		// Restoration
		const float CHAIN_HEAL = 125.0f;
		const float CHAIN_HEAL_1_JUMP = 71.43f;
		const float CHAIN_HEAL_2_JUMP = 35.71f;
		const float CHAIN_HEAL_3_JUMP = 17.86f;
		const float EARTH_SHIELD = 285.7f;
		const float HEALING_STREAM_TOTEM_TICK = 4.5f;
		const float HEALING_WAVE = 85.71f;
		const float LESSER_HEALING_WAVE = 42.86f;
	};

	namespace Warlock
	{
		// Affliction
		const float CORRUPTION = 93.6f;
		const float CURSE_OF_AGONY = 120.0f;
		const float CURSE_OF_DOOM = 200.0f;
		const float DARK_PACT = 96.0f;
		const float DRAIN_LIFE = 71.43f;
		const float DRAIN_MANA = 0.0f;
		const float DRAIN_SOUL = 214.3f;
		const float SEED_OF_CORRUPTION = 22.0f;
		const float SEED_OF_CORRUPTION_DOT = 150.0f;
		const float SIPHON_LIFE = 100.0f;
		const float UNSTABLE_AFFLICTION = 180.0f;
		const float UNSTABLE_AFFLICTION_DOT = 120.0f;

		// Demonology
		const float HEALTH_FUNNEL_HEAL = 285.7f;

		// Destruction
		const float CONFLAGRATE = 42.86f;
		const float DEATH_COIL = 22.0f;
		const float HELLFIRE_SELF = 142.86f;
		const float HELLFIRE_ENEMIES = 214.29f;
		const float IMMOLATE = 20.0f;
		const float IMMOLATE_DOT = 65.0f;
		const float INCINERATE = 71.43f;
		const float RAIN_OF_FIRE = 95.2f;
		const float SEARING_PAIN = 42.86f;
		const float SHADOW_BOLT = 85.71f;
		const float SHADOWBURN = 42.86f;
		const float SHADOWFURY = 19.5f;
		const float SHADOW_WARD = 30.0f;
		const float SOUL_FIRE = 115.0f;
	};
};

#endif // H_CLASS_SPELLS_COEFF