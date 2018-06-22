#include "../../playerbot.h"
#include "ShamanActions.h"
#include "ShamanAiObjectContext.h"
#include "ShamanNonCombatStrategy.h"
#include "HealShamanStrategy.h"
#include "MeleeShamanStrategy.h"
#include "ShamanTriggers.h"
#include "../NamedObjectContext.h"
#include "TotemsShamanStrategy.h"
#include "CasterShamanStrategy.h"

using namespace ai;



namespace ai
{
    namespace shaman
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["nc"] = &shaman::StrategyFactoryInternal::nc;
                creators["totems"] = &shaman::StrategyFactoryInternal::totems;
                creators["melee aoe"] = &shaman::StrategyFactoryInternal::melee_aoe;
                creators["caster aoe"] = &shaman::StrategyFactoryInternal::caster_aoe;
            }

        private:
            static std::shared_ptr<Strategy> nc(PlayerbotAI* ai) { return std::make_shared<ShamanNonCombatStrategy>(ai); }
            static std::shared_ptr<Strategy> totems(PlayerbotAI* ai) { return std::make_shared<TotemsShamanStrategy>(ai); }
            static std::shared_ptr<Strategy> melee_aoe(PlayerbotAI* ai) { return std::make_shared<MeleeAoeShamanStrategy>(ai); }
            static std::shared_ptr<Strategy> caster_aoe(PlayerbotAI* ai) { return std::make_shared<CasterAoeShamanStrategy>(ai); }
        };

        class BuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["bmana"] = &shaman::BuffStrategyFactoryInternal::bmana;
                creators["bdps"] = &shaman::BuffStrategyFactoryInternal::bdps;
            }

        private:
            static std::shared_ptr<Strategy> bmana(PlayerbotAI* ai) { return std::make_shared<ShamanBuffManaStrategy>(ai); }
            static std::shared_ptr<Strategy> bdps(PlayerbotAI* ai) { return std::make_shared<ShamanBuffDpsStrategy>(ai); }
        };

        class CombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["heal"] = &shaman::CombatStrategyFactoryInternal::heal;
                creators["melee"] = &shaman::CombatStrategyFactoryInternal::dps;
                creators["dps"] = &shaman::CombatStrategyFactoryInternal::dps;
                creators["caster"] = &shaman::CombatStrategyFactoryInternal::caster;
            }

        private:
            static std::shared_ptr<Strategy> heal(PlayerbotAI* ai) { return std::make_shared<HealShamanStrategy>(ai); }
            static std::shared_ptr<Strategy> dps(PlayerbotAI* ai) { return std::make_shared<MeleeShamanStrategy>(ai); }
            static std::shared_ptr<Strategy> caster(PlayerbotAI* ai) { return std::make_shared<CasterShamanStrategy>(ai); }
        };
    };
};

namespace ai
{
    namespace shaman
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["windfury totem"] = &TriggerFactoryInternal::windfury_totem;
                creators["mana spring totem"] = &TriggerFactoryInternal::mana_spring_totem;
                creators["flametongue totem"] = &TriggerFactoryInternal::flametongue_totem;
                creators["strength of earth totem"] = &TriggerFactoryInternal::strength_of_earth_totem;
                creators["magma totem"] = &TriggerFactoryInternal::magma_totem;
                creators["searing totem"] = &TriggerFactoryInternal::searing_totem;
                creators["wind shear"] = &TriggerFactoryInternal::wind_shear;
                creators["purge"] = &TriggerFactoryInternal::purge;
                creators["shaman weapon"] = &TriggerFactoryInternal::shaman_weapon;
                creators["water shield"] = &TriggerFactoryInternal::water_shield;
                creators["lightning shield"] = &TriggerFactoryInternal::lightning_shield;
                creators["water breathing"] = &TriggerFactoryInternal::water_breathing;
                creators["water walking"] = &TriggerFactoryInternal::water_walking;
                creators["water breathing on party"] = &TriggerFactoryInternal::water_breathing_on_party;
                creators["water walking on party"] = &TriggerFactoryInternal::water_walking_on_party;
                creators["cleanse spirit poison"] = &TriggerFactoryInternal::cleanse_poison;
                creators["cleanse spirit curse"] = &TriggerFactoryInternal::cleanse_curse;
                creators["cleanse spirit disease"] = &TriggerFactoryInternal::cleanse_disease;
                creators["party member cleanse spirit poison"] = &TriggerFactoryInternal::party_member_cleanse_poison;
                creators["party member cleanse spirit curse"] = &TriggerFactoryInternal::party_member_cleanse_curse;
                creators["party member cleanse spirit disease"] = &TriggerFactoryInternal::party_member_cleanse_disease;
                creators["shock"] = &TriggerFactoryInternal::shock;
                creators["frost shock snare"] = &TriggerFactoryInternal::frost_shock_snare;
                creators["heroism"] = &TriggerFactoryInternal::heroism;
                creators["bloodlust"] = &TriggerFactoryInternal::bloodlust;
                creators["maelstrom weapon"] = &TriggerFactoryInternal::maelstrom_weapon;
                creators["wind shear on enemy healer"] = &TriggerFactoryInternal::wind_shear_on_enemy_healer;
            }

        private:
            static std::shared_ptr<Trigger> maelstrom_weapon(PlayerbotAI* ai) { return std::make_shared<MaelstromWeaponTrigger>(ai); }
            static std::shared_ptr<Trigger> heroism(PlayerbotAI* ai) { return std::make_shared<HeroismTrigger>(ai); }
            static std::shared_ptr<Trigger> bloodlust(PlayerbotAI* ai) { return std::make_shared<BloodlustTrigger>(ai); }
            static std::shared_ptr<Trigger> party_member_cleanse_disease(PlayerbotAI* ai) { return std::make_shared<PartyMemberCleanseSpiritDiseaseTrigger>(ai); }
            static std::shared_ptr<Trigger> party_member_cleanse_curse(PlayerbotAI* ai) { return std::make_shared<PartyMemberCleanseSpiritCurseTrigger>(ai); }
            static std::shared_ptr<Trigger> party_member_cleanse_poison(PlayerbotAI* ai) { return std::make_shared<PartyMemberCleanseSpiritPoisonTrigger>(ai); }
            static std::shared_ptr<Trigger> cleanse_disease(PlayerbotAI* ai) { return std::make_shared<CleanseSpiritDiseaseTrigger>(ai); }
            static std::shared_ptr<Trigger> cleanse_curse(PlayerbotAI* ai) { return std::make_shared<CleanseSpiritCurseTrigger>(ai); }
            static std::shared_ptr<Trigger> cleanse_poison(PlayerbotAI* ai) { return std::make_shared<CleanseSpiritPoisonTrigger>(ai); }
            static std::shared_ptr<Trigger> water_breathing(PlayerbotAI* ai) { return std::make_shared<WaterBreathingTrigger>(ai); }
            static std::shared_ptr<Trigger> water_walking(PlayerbotAI* ai) { return std::make_shared<WaterWalkingTrigger>(ai); }
            static std::shared_ptr<Trigger> water_breathing_on_party(PlayerbotAI* ai) { return std::make_shared<WaterBreathingOnPartyTrigger>(ai); }
            static std::shared_ptr<Trigger> water_walking_on_party(PlayerbotAI* ai) { return std::make_shared<WaterWalkingOnPartyTrigger>(ai); }
            static std::shared_ptr<Trigger> windfury_totem(PlayerbotAI* ai) { return std::make_shared<WindfuryTotemTrigger>(ai); }
            static std::shared_ptr<Trigger> mana_spring_totem(PlayerbotAI* ai) { return std::make_shared<ManaSpringTotemTrigger>(ai); }
            static std::shared_ptr<Trigger> flametongue_totem(PlayerbotAI* ai) { return std::make_shared<FlametongueTotemTrigger>(ai); }
            static std::shared_ptr<Trigger> strength_of_earth_totem(PlayerbotAI* ai) { return std::make_shared<StrengthOfEarthTotemTrigger>(ai); }
            static std::shared_ptr<Trigger> magma_totem(PlayerbotAI* ai) { return std::make_shared<MagmaTotemTrigger>(ai); }
            static std::shared_ptr<Trigger> searing_totem(PlayerbotAI* ai) { return std::make_shared<SearingTotemTrigger>(ai); }
            static std::shared_ptr<Trigger> wind_shear(PlayerbotAI* ai) { return std::make_shared<WindShearInterruptSpellTrigger>(ai); }
            static std::shared_ptr<Trigger> purge(PlayerbotAI* ai) { return std::make_shared<PurgeTrigger>(ai); }
            static std::shared_ptr<Trigger> shaman_weapon(PlayerbotAI* ai) { return std::make_shared<ShamanWeaponTrigger>(ai); }
            static std::shared_ptr<Trigger> water_shield(PlayerbotAI* ai) { return std::make_shared<WaterShieldTrigger>(ai); }
            static std::shared_ptr<Trigger> lightning_shield(PlayerbotAI* ai) { return std::make_shared<LightningShieldTrigger>(ai); }
            static std::shared_ptr<Trigger> shock(PlayerbotAI* ai) { return std::make_shared<ShockTrigger>(ai); }
            static std::shared_ptr<Trigger> frost_shock_snare(PlayerbotAI* ai) { return std::make_shared<FrostShockSnareTrigger>(ai); }
            static std::shared_ptr<Trigger> wind_shear_on_enemy_healer(PlayerbotAI* ai) { return std::make_shared<WindShearInterruptEnemyHealerSpellTrigger>(ai); }
        };
    };
};


namespace ai
{
    namespace shaman
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["water shield"] = &AiObjectContextInternal::water_shield;
                creators["lightning shield"] = &AiObjectContextInternal::lightning_shield;
                creators["strength of earth totem"] = &AiObjectContextInternal::strength_of_earth_totem;
                creators["flametongue totem"] = &AiObjectContextInternal::flametongue_totem;
                creators["searing totem"] = &AiObjectContextInternal::searing_totem;
                creators["magma totem"] = &AiObjectContextInternal::magma_totem;
                creators["windfury totem"] = &AiObjectContextInternal::windfury_totem;
                creators["mana spring totem"] = &AiObjectContextInternal::mana_spring_totem;
                creators["mana tide totem"] = &AiObjectContextInternal::mana_tide_totem;
                creators["healing stream totem"] = &AiObjectContextInternal::healing_stream_totem;
                creators["wind shear"] = &AiObjectContextInternal::wind_shear;
                creators["wind shear on enemy healer"] = &AiObjectContextInternal::wind_shear_on_enemy_healer;
                creators["rockbiter weapon"] = &AiObjectContextInternal::rockbiter_weapon;
                creators["flametongue weapon"] = &AiObjectContextInternal::flametongue_weapon;
                creators["frostbrand weapon"] = &AiObjectContextInternal::frostbrand_weapon;
                creators["windfury weapon"] = &AiObjectContextInternal::windfury_weapon;
                creators["earthliving weapon"] = &AiObjectContextInternal::earthliving_weapon;
                creators["purge"] = &AiObjectContextInternal::purge;
                creators["healing wave"] = &AiObjectContextInternal::healing_wave;
                creators["lesser healing wave"] = &AiObjectContextInternal::lesser_healing_wave;
                creators["healing wave on party"] = &AiObjectContextInternal::healing_wave_on_party;
                creators["lesser healing wave on party"] = &AiObjectContextInternal::lesser_healing_wave_on_party;
                creators["earth shield"] = &AiObjectContextInternal::earth_shield;
                creators["earth shield on party"] = &AiObjectContextInternal::earth_shield_on_party;
                creators["chain heal"] = &AiObjectContextInternal::chain_heal;
                creators["riptide"] = &AiObjectContextInternal::riptide;
                creators["riptide on party"] = &AiObjectContextInternal::riptide_on_party;
                creators["stormstrike"] = &AiObjectContextInternal::stormstrike;
                creators["lava lash"] = &AiObjectContextInternal::lava_lash;
                creators["fire nova"] = &AiObjectContextInternal::fire_nova;
                creators["ancestral spirit"] = &AiObjectContextInternal::ancestral_spirit;
                creators["water walking"] = &AiObjectContextInternal::water_walking;
                creators["water breathing"] = &AiObjectContextInternal::water_breathing;
                creators["water walking on party"] = &AiObjectContextInternal::water_walking_on_party;
                creators["water breathing on party"] = &AiObjectContextInternal::water_breathing_on_party;
                creators["cleanse spirit"] = &AiObjectContextInternal::cleanse_spirit;
                creators["cleanse spirit poison on party"] = &AiObjectContextInternal::cleanse_spirit_poison_on_party;
                creators["cleanse spirit disease on party"] = &AiObjectContextInternal::cleanse_spirit_disease_on_party;
                creators["cleanse spirit curse on party"] = &AiObjectContextInternal::cleanse_spirit_curse_on_party;
                creators["flame shock"] = &AiObjectContextInternal::flame_shock;
                creators["earth shock"] = &AiObjectContextInternal::earth_shock;
                creators["frost shock"] = &AiObjectContextInternal::frost_shock;
                creators["chain lightning"] = &AiObjectContextInternal::chain_lightning;
                creators["lightning bolt"] = &AiObjectContextInternal::lightning_bolt;
                creators["thunderstorm"] = &AiObjectContextInternal::thunderstorm;
                creators["heroism"] = &AiObjectContextInternal::heroism;
                creators["bloodlust"] = &AiObjectContextInternal::bloodlust;
            }

        private:
            static std::shared_ptr<Action> heroism(PlayerbotAI* ai) { return std::make_shared<CastHeroismAction>(ai); }
            static std::shared_ptr<Action> bloodlust(PlayerbotAI* ai) { return std::make_shared<CastBloodlustAction>(ai); }
            static std::shared_ptr<Action> thunderstorm(PlayerbotAI* ai) { return std::make_shared<CastThunderstormAction>(ai); }
            static std::shared_ptr<Action> lightning_bolt(PlayerbotAI* ai) { return std::make_shared<CastLightningBoltAction>(ai); }
            static std::shared_ptr<Action> chain_lightning(PlayerbotAI* ai) { return std::make_shared<CastChainLightningAction>(ai); }
            static std::shared_ptr<Action> frost_shock(PlayerbotAI* ai) { return std::make_shared<CastFrostShockAction>(ai); }
            static std::shared_ptr<Action> earth_shock(PlayerbotAI* ai) { return std::make_shared<CastEarthShockAction>(ai); }
            static std::shared_ptr<Action> flame_shock(PlayerbotAI* ai) { return std::make_shared<CastFlameShockAction>(ai); }
            static std::shared_ptr<Action> cleanse_spirit_poison_on_party(PlayerbotAI* ai) { return std::make_shared<CastCleanseSpiritPoisonOnPartyAction>(ai); }
            static std::shared_ptr<Action> cleanse_spirit_disease_on_party(PlayerbotAI* ai) { return std::make_shared<CastCleanseSpiritDiseaseOnPartyAction>(ai); }
            static std::shared_ptr<Action> cleanse_spirit_curse_on_party(PlayerbotAI* ai) { return std::make_shared<CastCleanseSpiritCurseOnPartyAction>(ai); }
            static std::shared_ptr<Action> cleanse_spirit(PlayerbotAI* ai) { return std::make_shared<CastCleanseSpiritAction>(ai); }
            static std::shared_ptr<Action> water_walking(PlayerbotAI* ai) { return std::make_shared<CastWaterWalkingAction>(ai); }
            static std::shared_ptr<Action> water_breathing(PlayerbotAI* ai) { return std::make_shared<CastWaterBreathingAction>(ai); }
            static std::shared_ptr<Action> water_walking_on_party(PlayerbotAI* ai) { return std::make_shared<CastWaterWalkingOnPartyAction>(ai); }
            static std::shared_ptr<Action> water_breathing_on_party(PlayerbotAI* ai) { return std::make_shared<CastWaterBreathingOnPartyAction>(ai); }
            static std::shared_ptr<Action> water_shield(PlayerbotAI* ai) { return std::make_shared<CastWaterShieldAction>(ai); }
            static std::shared_ptr<Action> lightning_shield(PlayerbotAI* ai) { return std::make_shared<CastLightningShieldAction>(ai); }
            static std::shared_ptr<Action> strength_of_earth_totem(PlayerbotAI* ai) { return std::make_shared<CastStrengthOfEarthTotemAction>(ai); }
            static std::shared_ptr<Action> flametongue_totem(PlayerbotAI* ai) { return std::make_shared<CastFlametongueTotemAction>(ai); }
            static std::shared_ptr<Action> magma_totem(PlayerbotAI* ai) { return std::make_shared<CastMagmaTotemAction>(ai); }
            static std::shared_ptr<Action> searing_totem(PlayerbotAI* ai) { return std::make_shared<CastSearingTotemAction>(ai); }
            static std::shared_ptr<Action> fire_nova(PlayerbotAI* ai) { return std::make_shared<CastFireNovaAction>(ai); }
            static std::shared_ptr<Action> windfury_totem(PlayerbotAI* ai) { return std::make_shared<CastWindfuryTotemAction>(ai); }
            static std::shared_ptr<Action> mana_spring_totem(PlayerbotAI* ai) { return std::make_shared<CastManaSpringTotemAction>(ai); }
            static std::shared_ptr<Action> mana_tide_totem(PlayerbotAI* ai) { return std::make_shared<CastManaTideTotemAction>(ai); }
            static std::shared_ptr<Action> healing_stream_totem(PlayerbotAI* ai) { return std::make_shared<CastHealingStreamTotemAction>(ai); }
            static std::shared_ptr<Action> wind_shear(PlayerbotAI* ai) { return std::make_shared<CastWindShearAction>(ai); }
            static std::shared_ptr<Action> rockbiter_weapon(PlayerbotAI* ai) { return std::make_shared<CastRockbiterWeaponAction>(ai); }
            static std::shared_ptr<Action> flametongue_weapon(PlayerbotAI* ai) { return std::make_shared<CastFlametongueWeaponAction>(ai); }
            static std::shared_ptr<Action> frostbrand_weapon(PlayerbotAI* ai) { return std::make_shared<CastFrostbrandWeaponAction>(ai); }
            static std::shared_ptr<Action> windfury_weapon(PlayerbotAI* ai) { return std::make_shared<CastWindfuryWeaponAction>(ai); }
            static std::shared_ptr<Action> earthliving_weapon(PlayerbotAI* ai) { return std::make_shared<CastEarthlivingWeaponAction>(ai); }
            static std::shared_ptr<Action> purge(PlayerbotAI* ai) { return std::make_shared<CastPurgeAction>(ai); }
            static std::shared_ptr<Action> healing_wave(PlayerbotAI* ai) { return std::make_shared<CastHealingWaveAction>(ai); }
            static std::shared_ptr<Action> lesser_healing_wave(PlayerbotAI* ai) { return std::make_shared<CastLesserHealingWaveAction>(ai); }
            static std::shared_ptr<Action> healing_wave_on_party(PlayerbotAI* ai) { return std::make_shared<CastHealingWaveOnPartyAction>(ai); }
            static std::shared_ptr<Action> lesser_healing_wave_on_party(PlayerbotAI* ai) { return std::make_shared<CastLesserHealingWaveOnPartyAction>(ai); }
            static std::shared_ptr<Action> earth_shield(PlayerbotAI* ai) { return std::make_shared<CastEarthShieldAction>(ai); }
            static std::shared_ptr<Action> earth_shield_on_party(PlayerbotAI* ai) { return std::make_shared<CastEarthShieldOnPartyAction>(ai); }
            static std::shared_ptr<Action> chain_heal(PlayerbotAI* ai) { return std::make_shared<CastChainHealAction>(ai); }
            static std::shared_ptr<Action> riptide(PlayerbotAI* ai) { return std::make_shared<CastRiptideAction>(ai); }
            static std::shared_ptr<Action> riptide_on_party(PlayerbotAI* ai) { return std::make_shared<CastRiptideOnPartyAction>(ai); }
            static std::shared_ptr<Action> stormstrike(PlayerbotAI* ai) { return std::make_shared<CastStormstrikeAction>(ai); }
            static std::shared_ptr<Action> lava_lash(PlayerbotAI* ai) { return std::make_shared<CastLavaLashAction>(ai); }
            static std::shared_ptr<Action> ancestral_spirit(PlayerbotAI* ai) { return std::make_shared<CastAncestralSpiritAction>(ai); }
            static std::shared_ptr<Action> wind_shear_on_enemy_healer(PlayerbotAI* ai) { return std::make_shared<CastWindShearOnEnemyHealerAction>(ai); }
        };
    };
};



ShamanAiObjectContext::ShamanAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::shaman::StrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::CombatStrategyFactoryInternal());
    strategyContexts.Add(new ai::shaman::BuffStrategyFactoryInternal());
    actionContexts.Add(new ai::shaman::AiObjectContextInternal());
    triggerContexts.Add(new ai::shaman::TriggerFactoryInternal());
}
