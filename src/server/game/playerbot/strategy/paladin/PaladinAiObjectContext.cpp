
#include "../../playerbot.h"
#include "PaladinActions.h"
#include "PaladinTriggers.h"
#include "PaladinAiObjectContext.h"
#include "GenericPaladinNonCombatStrategy.h"
#include "TankPaladinStrategy.h"
#include "DpsPaladinStrategy.h"
#include "PaladinBuffStrategies.h"
#include "../NamedObjectContext.h"
#include "HealPaladinStrategy.h"

using namespace ai;

namespace ai
{
    namespace paladin
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["nc"] = &paladin::StrategyFactoryInternal::nc;
            }

        private:
            static std::shared_ptr<Strategy> nc(PlayerbotAI* ai) { return std::make_shared<GenericPaladinNonCombatStrategy>(ai); }
        };

        class ResistanceStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ResistanceStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["rshadow"] = &paladin::ResistanceStrategyFactoryInternal::rshadow;
                creators["rfrost"] = &paladin::ResistanceStrategyFactoryInternal::rfrost;
                creators["rfire"] = &paladin::ResistanceStrategyFactoryInternal::rfire;
            }

        private:
            static std::shared_ptr<Strategy> rshadow(PlayerbotAI* ai) { return std::make_shared<PaladinShadowResistanceStrategy>(ai); }
            static std::shared_ptr<Strategy> rfrost(PlayerbotAI* ai) { return std::make_shared<PaladinFrostResistanceStrategy>(ai); }
            static std::shared_ptr<Strategy> rfire(PlayerbotAI* ai) { return std::make_shared<PaladinFireResistanceStrategy>(ai); }
        };

        class BuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["bhealth"] = &paladin::BuffStrategyFactoryInternal::bhealth;
                creators["bmana"] = &paladin::BuffStrategyFactoryInternal::bmana;
                creators["bdps"] = &paladin::BuffStrategyFactoryInternal::bdps;
                creators["barmor"] = &paladin::BuffStrategyFactoryInternal::barmor;
                creators["bspeed"] = &paladin::BuffStrategyFactoryInternal::bspeed;
                creators["bthreat"] = &paladin::BuffStrategyFactoryInternal::bthreat;
            }

        private:
            static std::shared_ptr<Strategy> bhealth(PlayerbotAI* ai) { return std::make_shared<PaladinBuffHealthStrategy>(ai); }
            static std::shared_ptr<Strategy> bmana(PlayerbotAI* ai) { return std::make_shared<PaladinBuffManaStrategy>(ai); }
            static std::shared_ptr<Strategy> bdps(PlayerbotAI* ai) { return std::make_shared<PaladinBuffDpsStrategy>(ai); }
            static std::shared_ptr<Strategy> barmor(PlayerbotAI* ai) { return std::make_shared<PaladinBuffArmorStrategy>(ai); }
            static std::shared_ptr<Strategy> bspeed(PlayerbotAI* ai) { return std::make_shared<PaladinBuffSpeedStrategy>(ai); }
            static std::shared_ptr<Strategy> bthreat(PlayerbotAI* ai) { return std::make_shared<PaladinBuffThreatStrategy>(ai); }
        };

        class CombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["tank"] = &paladin::CombatStrategyFactoryInternal::tank;
                creators["dps"] = &paladin::CombatStrategyFactoryInternal::dps;
                creators["heal"] = &paladin::CombatStrategyFactoryInternal::heal;
            }

        private:
            static std::shared_ptr<Strategy> tank(PlayerbotAI* ai) { return std::make_shared<TankPaladinStrategy>(ai); }
            static std::shared_ptr<Strategy> dps(PlayerbotAI* ai) { return std::make_shared<DpsPaladinStrategy>(ai); }
            static std::shared_ptr<Strategy> heal(PlayerbotAI* ai) { return std::make_shared<HealPaladinStrategy>(ai); }
        };
    };
};

namespace ai
{
    namespace paladin
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["judgement of wisdom"] = &TriggerFactoryInternal::judgement_of_wisdom;
                creators["judgement of light"] = &TriggerFactoryInternal::judgement_of_light;
                creators["blessing"] = &TriggerFactoryInternal::blessing;
                creators["seal"] = &TriggerFactoryInternal::seal;
                creators["art of war"] = &TriggerFactoryInternal::art_of_war;
                creators["blessing of kings on party"] = &TriggerFactoryInternal::blessing_of_kings_on_party;
                creators["crusader aura"] = &TriggerFactoryInternal::crusader_aura;
                creators["retribution aura"] = &TriggerFactoryInternal::retribution_aura;
                creators["devotion aura"] = &TriggerFactoryInternal::devotion_aura;
                creators["shadow resistance aura"] = &TriggerFactoryInternal::shadow_resistance_aura;
                creators["frost resistance aura"] = &TriggerFactoryInternal::frost_resistance_aura;
                creators["fire resistance aura"] = &TriggerFactoryInternal::fire_resistance_aura;
                creators["hammer of justice snare"] = &TriggerFactoryInternal::hammer_of_justice_snare;
                creators["hammer of justice interrupt"] = &TriggerFactoryInternal::hammer_of_justice_interrupt;
                creators["cleanse cure disease"] = &TriggerFactoryInternal::CleanseCureDisease;
                creators["cleanse party member cure disease"] = &TriggerFactoryInternal::CleanseCurePartyMemberDisease;
                creators["cleanse cure poison"] = &TriggerFactoryInternal::CleanseCurePoison;
                creators["cleanse party member cure poison"] = &TriggerFactoryInternal::CleanseCurePartyMemberPoison;
                creators["cleanse cure magic"] = &TriggerFactoryInternal::CleanseCureMagic;
                creators["cleanse party member cure magic"] = &TriggerFactoryInternal::CleanseCurePartyMemberMagic;
                creators["righteous fury"] = &TriggerFactoryInternal::righteous_fury;
                creators["holy shield"] = &TriggerFactoryInternal::holy_shield;
                creators["hammer of justice on enemy healer"] = &TriggerFactoryInternal::hammer_of_justice_on_enemy_target;
            }

        private:
            static std::shared_ptr<Trigger> holy_shield(PlayerbotAI* ai) { return std::make_shared<HolyShieldTrigger>(ai); }
            static std::shared_ptr<Trigger> righteous_fury(PlayerbotAI* ai) { return std::make_shared<RighteousFuryTrigger>(ai); }
            static std::shared_ptr<Trigger> judgement_of_wisdom(PlayerbotAI* ai) { return std::make_shared<JudgementOfWisdomTrigger>(ai); }
            static std::shared_ptr<Trigger> judgement_of_light(PlayerbotAI* ai) { return std::make_shared<JudgementOfLightTrigger>(ai); }
            static std::shared_ptr<Trigger> blessing(PlayerbotAI* ai) { return std::make_shared<BlessingTrigger>(ai); }
            static std::shared_ptr<Trigger> seal(PlayerbotAI* ai) { return std::make_shared<SealTrigger>(ai); }
            static std::shared_ptr<Trigger> art_of_war(PlayerbotAI* ai) { return std::make_shared<ArtOfWarTrigger>(ai); }
            static std::shared_ptr<Trigger> blessing_of_kings_on_party(PlayerbotAI* ai) { return std::make_shared<BlessingOfKingsOnPartyTrigger>(ai); }
            static std::shared_ptr<Trigger> crusader_aura(PlayerbotAI* ai) { return std::make_shared<CrusaderAuraTrigger>(ai); }
            static std::shared_ptr<Trigger> retribution_aura(PlayerbotAI* ai) { return std::make_shared<RetributionAuraTrigger>(ai); }
            static std::shared_ptr<Trigger> devotion_aura(PlayerbotAI* ai) { return std::make_shared<DevotionAuraTrigger>(ai); }
            static std::shared_ptr<Trigger> shadow_resistance_aura(PlayerbotAI* ai) { return std::make_shared<ShadowResistanceAuraTrigger>(ai); }
            static std::shared_ptr<Trigger> frost_resistance_aura(PlayerbotAI* ai) { return std::make_shared<FrostResistanceAuraTrigger>(ai); }
            static std::shared_ptr<Trigger> fire_resistance_aura(PlayerbotAI* ai) { return std::make_shared<FireResistanceAuraTrigger>(ai); }
            static std::shared_ptr<Trigger> hammer_of_justice_snare(PlayerbotAI* ai) { return std::make_shared<HammerOfJusticeSnareTrigger>(ai); }
            static std::shared_ptr<Trigger> hammer_of_justice_interrupt(PlayerbotAI* ai) { return std::make_shared<HammerOfJusticeInterruptSpellTrigger>(ai); }
            static std::shared_ptr<Trigger> CleanseCureDisease(PlayerbotAI* ai) { return std::make_shared<CleanseCureDiseaseTrigger>(ai); }
            static std::shared_ptr<Trigger> CleanseCurePartyMemberDisease(PlayerbotAI* ai) { return std::make_shared<CleanseCurePartyMemberDiseaseTrigger>(ai); }
            static std::shared_ptr<Trigger> CleanseCurePoison(PlayerbotAI* ai) { return std::make_shared<CleanseCurePoisonTrigger>(ai); }
            static std::shared_ptr<Trigger> CleanseCurePartyMemberPoison(PlayerbotAI* ai) { return std::make_shared<CleanseCurePartyMemberPoisonTrigger>(ai); }
            static std::shared_ptr<Trigger> CleanseCureMagic(PlayerbotAI* ai) { return std::make_shared<CleanseCureMagicTrigger>(ai); }
            static std::shared_ptr<Trigger> CleanseCurePartyMemberMagic(PlayerbotAI* ai) { return std::make_shared<CleanseCurePartyMemberMagicTrigger>(ai); }
            static std::shared_ptr<Trigger> hammer_of_justice_on_enemy_target(PlayerbotAI* ai) { return std::make_shared<HammerOfJusticeEnemyHealerTrigger>(ai); }
        };
    };
};

namespace ai
{
    namespace paladin
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["seal of command"] = &AiObjectContextInternal::seal_of_command;
                creators["seal of vengeance"] = &AiObjectContextInternal::seal_of_vengeance;
                creators["blessing of might"] = &AiObjectContextInternal::blessing_of_might;
                creators["divine storm"] = &AiObjectContextInternal::divine_storm;
                creators["blessing of kings on party"] = &AiObjectContextInternal::blessing_of_kings_on_party;
                creators["redemption"] = &AiObjectContextInternal::redemption;
                creators["crusader strike"] = &AiObjectContextInternal::crusader_strike;
                creators["crusader aura"] = &AiObjectContextInternal::crusader_aura;
                creators["seal of light"] = &AiObjectContextInternal::seal_of_light;
                creators["devotion aura"] = &AiObjectContextInternal::devotion_aura;
                creators["holy wrath"] = &AiObjectContextInternal::holy_wrath;
                creators["consecration"] = &AiObjectContextInternal::consecration;
                creators["cleanse disease"] = &AiObjectContextInternal::cleanse_disease;
                creators["cleanse poison"] = &AiObjectContextInternal::cleanse_poison;
                creators["cleanse magic"] = &AiObjectContextInternal::cleanse_magic;
                creators["purify disease"] = &AiObjectContextInternal::purify_disease;
                creators["purify poison"] = &AiObjectContextInternal::purify_poison;
                creators["cleanse poison on party"] = &AiObjectContextInternal::cleanse_poison_on_party;
                creators["cleanse disease on party"] = &AiObjectContextInternal::cleanse_disease_on_party;
                creators["cleanse magic on party"] = &AiObjectContextInternal::cleanse_magic_on_party;
                creators["purify poison on party"] = &AiObjectContextInternal::purify_poison_on_party;
                creators["purify disease on party"] = &AiObjectContextInternal::purify_disease_on_party;
                creators["seal of wisdom"] = &AiObjectContextInternal::seal_of_wisdom;
                creators["seal of justice"] = &AiObjectContextInternal::seal_of_justice;
                creators["seal of righteousness"] = &AiObjectContextInternal::seal_of_righteousness;
                creators["flash of light"] = &AiObjectContextInternal::flash_of_light;
                creators["hand of reckoning"] = &AiObjectContextInternal::hand_of_reckoning;
                creators["avenger's shield"] = &AiObjectContextInternal::avengers_shield;
                creators["exorcism"] = &AiObjectContextInternal::exorcism;
                creators["judgement of light"] = &AiObjectContextInternal::judgement_of_light;
                creators["judgement of wisdom"] = &AiObjectContextInternal::judgement_of_wisdom;
                creators["divine shield"] = &AiObjectContextInternal::divine_shield;
                creators["divine protection"] = &AiObjectContextInternal::divine_protection;
                creators["divine protection on party"] =&AiObjectContextInternal::divine_protection_on_party;
                creators["hammer of justice"] = &AiObjectContextInternal::hammer_of_justice;
                creators["flash of light on party"] = &AiObjectContextInternal::flash_of_light_on_party;
                creators["holy light"] = &AiObjectContextInternal::holy_light;
                creators["holy light on party"] = &AiObjectContextInternal::holy_light_on_party;
                creators["lay on hands"] = &AiObjectContextInternal::lay_on_hands;
                creators["lay on hands on party"] = &AiObjectContextInternal::lay_on_hands_on_party;
                creators["judgement of justice"] = &AiObjectContextInternal::judgement_of_justice;
                creators["hammer of wrath"] = &AiObjectContextInternal::hammer_of_wrath;
                creators["holy shield"] = &AiObjectContextInternal::holy_shield;
                creators["hammer of the righteous"] = &AiObjectContextInternal::hammer_of_the_righteous;
                creators["blessing of kings"] = &AiObjectContextInternal::blessing_of_kings;
                creators["retribution aura"] = &AiObjectContextInternal::retribution_aura;
                creators["shadow resistance aura"] = &AiObjectContextInternal::shadow_resistance_aura;
                creators["frost resistance aura"] = &AiObjectContextInternal::frost_resistance_aura;
                creators["fire resistance aura"] = &AiObjectContextInternal::fire_resistance_aura;
                creators["righteous fury"] = &AiObjectContextInternal::righteous_fury;
                creators["blessing of sanctuary"] = &AiObjectContextInternal::blessing_of_sanctuary;
                creators["hammer of justice on enemy healer"] = &AiObjectContextInternal::hammer_of_justice_on_enemy_healer;
            }

        private:
            static std::shared_ptr<Action> righteous_fury(PlayerbotAI* ai) { return std::make_shared<CastRighteousFuryAction>(ai); }
            static std::shared_ptr<Action> blessing_of_sanctuary(PlayerbotAI* ai) { return std::make_shared<CastBlessingOfSanctuaryAction>(ai); }
            static std::shared_ptr<Action> seal_of_command(PlayerbotAI* ai) { return std::make_shared<CastSealOfCommandAction>(ai); }
            static std::shared_ptr<Action> seal_of_vengeance(PlayerbotAI* ai) { return std::make_shared<CastSealOfVengeanceAction>(ai); }
            static std::shared_ptr<Action> blessing_of_might(PlayerbotAI* ai) { return std::make_shared<CastBlessingOfMightAction>(ai); }
            static std::shared_ptr<Action> divine_storm(PlayerbotAI* ai) { return std::make_shared<CastDivineStormAction>(ai); }
            static std::shared_ptr<Action> blessing_of_kings_on_party(PlayerbotAI* ai) { return std::make_shared<CastBlessingOfKingsOnPartyAction>(ai); }
            static std::shared_ptr<Action> redemption(PlayerbotAI* ai) { return std::make_shared<CastRedemptionAction>(ai); }
            static std::shared_ptr<Action> crusader_strike(PlayerbotAI* ai) { return std::make_shared<CastCrusaderStrikeAction>(ai); }
            static std::shared_ptr<Action> crusader_aura(PlayerbotAI* ai) { return std::make_shared<CastCrusaderAuraAction>(ai); }
            static std::shared_ptr<Action> seal_of_light(PlayerbotAI* ai) { return std::make_shared<CastSealOfLightAction>(ai); }
            static std::shared_ptr<Action> devotion_aura(PlayerbotAI* ai) { return std::make_shared<CastDevotionAuraAction>(ai); }
            static std::shared_ptr<Action> holy_wrath(PlayerbotAI* ai) { return std::make_shared<CastHolyWrathAction>(ai); }
            static std::shared_ptr<Action> consecration(PlayerbotAI* ai) { return std::make_shared<CastConsecrationAction>(ai); }
            static std::shared_ptr<Action> cleanse_poison(PlayerbotAI* ai) { return std::make_shared<CastCleansePoisonAction>(ai); }
            static std::shared_ptr<Action> cleanse_disease(PlayerbotAI* ai) { return std::make_shared<CastCleanseDiseaseAction>(ai); }
            static std::shared_ptr<Action> cleanse_magic(PlayerbotAI* ai) { return std::make_shared<CastCleanseMagicAction>(ai); }
            static std::shared_ptr<Action> purify_poison(PlayerbotAI* ai) { return std::make_shared<CastPurifyPoisonAction>(ai); }
            static std::shared_ptr<Action> purify_disease(PlayerbotAI* ai) { return std::make_shared<CastPurifyDiseaseAction>(ai); }
            static std::shared_ptr<Action> cleanse_poison_on_party(PlayerbotAI* ai) { return std::make_shared<CastCleansePoisonOnPartyAction>(ai); }
            static std::shared_ptr<Action> cleanse_disease_on_party(PlayerbotAI* ai) { return std::make_shared<CastCleanseDiseaseOnPartyAction>(ai); }
            static std::shared_ptr<Action> cleanse_magic_on_party(PlayerbotAI* ai) { return std::make_shared<CastCleanseMagicOnPartyAction>(ai); }
            static std::shared_ptr<Action> purify_poison_on_party(PlayerbotAI* ai) { return std::make_shared<CastPurifyPoisonOnPartyAction>(ai); }
            static std::shared_ptr<Action> purify_disease_on_party(PlayerbotAI* ai) { return std::make_shared<CastPurifyDiseaseOnPartyAction>(ai); }
            static std::shared_ptr<Action> seal_of_wisdom(PlayerbotAI* ai) { return std::make_shared<CastSealOfWisdomAction>(ai); }
            static std::shared_ptr<Action> seal_of_justice(PlayerbotAI* ai) { return std::make_shared<CastSealOfJusticeAction>(ai); }
            static std::shared_ptr<Action> seal_of_righteousness(PlayerbotAI* ai) { return std::make_shared<CastSealOfRighteousnessAction>(ai); }
            static std::shared_ptr<Action> flash_of_light(PlayerbotAI* ai) { return std::make_shared<CastFlashOfLightAction>(ai); }
            static std::shared_ptr<Action> hand_of_reckoning(PlayerbotAI* ai) { return std::make_shared<CastHandOfReckoningAction>(ai); }
            static std::shared_ptr<Action> avengers_shield(PlayerbotAI* ai) { return std::make_shared<CastAvengersShieldAction>(ai); }
            static std::shared_ptr<Action> exorcism(PlayerbotAI* ai) { return std::make_shared<CastExorcismAction>(ai); }
            static std::shared_ptr<Action> judgement_of_light(PlayerbotAI* ai) { return std::make_shared<CastJudgementOfLightAction>(ai); }
            static std::shared_ptr<Action> judgement_of_wisdom(PlayerbotAI* ai) { return std::make_shared<CastJudgementOfWisdomAction>(ai); }
            static std::shared_ptr<Action> divine_shield(PlayerbotAI* ai) { return std::make_shared<CastDivineShieldAction>(ai); }
            static std::shared_ptr<Action> divine_protection(PlayerbotAI* ai) { return std::make_shared<CastDivineProtectionAction>(ai); }
            static std::shared_ptr<Action> divine_protection_on_party(PlayerbotAI* ai) { return std::make_shared<CastDivineProtectionOnPartyAction>(ai); }
            static std::shared_ptr<Action> hammer_of_justice(PlayerbotAI* ai) { return std::make_shared<CastHammerOfJusticeAction>(ai); }
            static std::shared_ptr<Action> flash_of_light_on_party(PlayerbotAI* ai) { return std::make_shared<CastFlashOfLightOnPartyAction>(ai); }
            static std::shared_ptr<Action> holy_light(PlayerbotAI* ai) { return std::make_shared<CastHolyLightAction>(ai); }
            static std::shared_ptr<Action> holy_light_on_party(PlayerbotAI* ai) { return std::make_shared<CastHolyLightOnPartyAction>(ai); }
            static std::shared_ptr<Action> lay_on_hands(PlayerbotAI* ai) { return std::make_shared<CastLayOnHandsAction>(ai); }
            static std::shared_ptr<Action> lay_on_hands_on_party(PlayerbotAI* ai) { return std::make_shared<CastLayOnHandsOnPartyAction>(ai); }
            static std::shared_ptr<Action> judgement_of_justice(PlayerbotAI* ai) { return std::make_shared<CastJudgementOfJusticeAction>(ai); }
            static std::shared_ptr<Action> hammer_of_wrath(PlayerbotAI* ai) { return std::make_shared<CastHammerOfWrathAction>(ai); }
            static std::shared_ptr<Action> holy_shield(PlayerbotAI* ai) { return std::make_shared<CastHolyShieldAction>(ai); }
            static std::shared_ptr<Action> hammer_of_the_righteous(PlayerbotAI* ai) { return std::make_shared<CastHammerOfTheRighteousAction>(ai); }
            static std::shared_ptr<Action> blessing_of_kings(PlayerbotAI* ai) { return std::make_shared<CastBlessingOfKingsAction>(ai); }
            static std::shared_ptr<Action> retribution_aura(PlayerbotAI* ai) { return std::make_shared<CastRetributionAuraAction>(ai); }
            static std::shared_ptr<Action> shadow_resistance_aura(PlayerbotAI* ai) { return std::make_shared<CastShadowResistanceAuraAction>(ai); }
            static std::shared_ptr<Action> frost_resistance_aura(PlayerbotAI* ai) { return std::make_shared<CastFrostResistanceAuraAction>(ai); }
            static std::shared_ptr<Action> fire_resistance_aura(PlayerbotAI* ai) { return std::make_shared<CastFireResistanceAuraAction>(ai); }
            static std::shared_ptr<Action> hammer_of_justice_on_enemy_healer(PlayerbotAI* ai) { return std::make_shared<CastHammerOfJusticeOnEnemyHealerAction>(ai); }
        };
    };
};


PaladinAiObjectContext::PaladinAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::paladin::StrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::CombatStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::BuffStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::ResistanceStrategyFactoryInternal());
    actionContexts.Add(new ai::paladin::AiObjectContextInternal());
    triggerContexts.Add(new ai::paladin::TriggerFactoryInternal());
}
