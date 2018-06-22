
#include "../../playerbot.h"
#include "PriestActions.h"
#include "PriestAiObjectContext.h"
#include "PriestNonCombatStrategy.h"
#include "ShadowPriestStrategy.h"
#include "../generic/PullStrategy.h"
#include "PriestTriggers.h"
#include "../NamedObjectContext.h"
#include "HolyPriestStrategy.h"

using namespace ai;


namespace ai
{
    namespace priest
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["nc"] = &priest::StrategyFactoryInternal::nc;
                creators["pull"] = &priest::StrategyFactoryInternal::pull;
                creators["aoe"] = &priest::StrategyFactoryInternal::shadow_aoe;
                creators["shadow aoe"] = &priest::StrategyFactoryInternal::shadow_aoe;
                creators["dps debuff"] = &priest::StrategyFactoryInternal::shadow_debuff;
                creators["shadow debuff"] = &priest::StrategyFactoryInternal::shadow_debuff;
            }

        private:
            static std::shared_ptr<Strategy> nc(PlayerbotAI* ai) { return std::make_shared<PriestNonCombatStrategy>(ai); }
            static std::shared_ptr<Strategy> shadow_aoe(PlayerbotAI* ai) { return std::make_shared<ShadowPriestAoeStrategy>(ai); }
            static std::shared_ptr<Strategy> pull(PlayerbotAI* ai) { return std::make_shared<PullStrategy>(ai, "shoot"); }
            static std::shared_ptr<Strategy> shadow_debuff(PlayerbotAI* ai) { return std::make_shared<ShadowPriestDebuffStrategy>(ai); }
        };

        class CombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["heal"] = &priest::CombatStrategyFactoryInternal::heal;
                creators["shadow"] = &priest::CombatStrategyFactoryInternal::dps;
                creators["dps"] = &priest::CombatStrategyFactoryInternal::dps;
                creators["holy"] = &priest::CombatStrategyFactoryInternal::holy;
            }

        private:
            static std::shared_ptr<Strategy> heal(PlayerbotAI* ai) { return std::make_shared<HealPriestStrategy>(ai); }
            static std::shared_ptr<Strategy> dps(PlayerbotAI* ai) { return std::make_shared<ShadowPriestStrategy>(ai); }
            static std::shared_ptr<Strategy> holy(PlayerbotAI* ai) { return std::make_shared<HolyPriestStrategy>(ai); }
        };
    };
};

namespace ai
{
    namespace priest
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["devouring plague"] = &TriggerFactoryInternal::devouring_plague;
                creators["shadow word: pain"] = &TriggerFactoryInternal::shadow_word_pain;
                creators["shadow word: pain on attacker"] = &TriggerFactoryInternal::shadow_word_pain_on_attacker;
                creators["dispel magic"] = &TriggerFactoryInternal::dispel_magic;
                creators["dispel magic on party"] = &TriggerFactoryInternal::dispel_magic_party_member;
                creators["cure disease"] = &TriggerFactoryInternal::cure_disease;
                creators["party member cure disease"] = &TriggerFactoryInternal::party_member_cure_disease;
                creators["power word: fortitude"] = &TriggerFactoryInternal::power_word_fortitude;
                creators["power word: fortitude on party"] = &TriggerFactoryInternal::power_word_fortitude_on_party;
                creators["divine spirit"] = &TriggerFactoryInternal::divine_spirit;
                creators["divine spirit on party"] = &TriggerFactoryInternal::divine_spirit_on_party;
                creators["inner fire"] = &TriggerFactoryInternal::inner_fire;
                creators["vampiric touch"] = &TriggerFactoryInternal::vampiric_touch;
                creators["shadowform"] = &TriggerFactoryInternal::shadowform;
                creators["vampiric embrace"] = &TriggerFactoryInternal::vampiric_embrace;

            }

        private:
            static std::shared_ptr<Trigger> vampiric_embrace(PlayerbotAI* ai) { return std::make_shared<VampiricEmbraceTrigger>(ai); }
            static std::shared_ptr<Trigger> shadowform(PlayerbotAI* ai) { return std::make_shared<ShadowformTrigger>(ai); }
            static std::shared_ptr<Trigger> vampiric_touch(PlayerbotAI* ai) { return std::make_shared<VampiricTouchTrigger>(ai); }
            static std::shared_ptr<Trigger> devouring_plague(PlayerbotAI* ai) { return std::make_shared<DevouringPlagueTrigger>(ai); }
            static std::shared_ptr<Trigger> shadow_word_pain(PlayerbotAI* ai) { return std::make_shared<PowerWordPainTrigger>(ai); }
            static std::shared_ptr<Trigger> shadow_word_pain_on_attacker(PlayerbotAI* ai) { return std::make_shared<PowerWordPainOnAttackerTrigger>(ai); }
            static std::shared_ptr<Trigger> dispel_magic(PlayerbotAI* ai) { return std::make_shared<DispelMagicTrigger>(ai); }
            static std::shared_ptr<Trigger> dispel_magic_party_member(PlayerbotAI* ai) { return std::make_shared<DispelMagicPartyMemberTrigger>(ai); }
            static std::shared_ptr<Trigger> cure_disease(PlayerbotAI* ai) { return std::make_shared<CureDiseaseTrigger>(ai); }
            static std::shared_ptr<Trigger> party_member_cure_disease(PlayerbotAI* ai) { return std::make_shared<PartyMemberCureDiseaseTrigger>(ai); }
            static std::shared_ptr<Trigger> power_word_fortitude(PlayerbotAI* ai) { return std::make_shared<PowerWordFortitudeTrigger>(ai); }
            static std::shared_ptr<Trigger> power_word_fortitude_on_party(PlayerbotAI* ai) { return std::make_shared<PowerWordFortitudeOnPartyTrigger>(ai); }
            static std::shared_ptr<Trigger> divine_spirit(PlayerbotAI* ai) { return std::make_shared<DivineSpiritTrigger>(ai); }
            static std::shared_ptr<Trigger> divine_spirit_on_party(PlayerbotAI* ai) { return std::make_shared<DivineSpiritOnPartyTrigger>(ai); }
            static std::shared_ptr<Trigger> inner_fire(PlayerbotAI* ai) { return std::make_shared<InnerFireTrigger>(ai); }
        };
    };
};



namespace ai
{
    namespace priest
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["shadow word: pain"] = &AiObjectContextInternal::shadow_word_pain;
                creators["shadow word: pain on attacker"] = &AiObjectContextInternal::shadow_word_pain_on_attacker;
                creators["devouring plague"] = &AiObjectContextInternal::devouring_plague;
                creators["mind flay"] = &AiObjectContextInternal::mind_flay;
                creators["holy fire"] = &AiObjectContextInternal::holy_fire;
                creators["smite"] = &AiObjectContextInternal::smite;
                creators["mind blast"] = &AiObjectContextInternal::mind_blast;
                creators["shadowform"] = &AiObjectContextInternal::shadowform;
                creators["remove shadowform"] = &AiObjectContextInternal::remove_shadowform;
                creators["holy nova"] = &AiObjectContextInternal::holy_nova;
                creators["power word: fortitude"] = &AiObjectContextInternal::power_word_fortitude;
                creators["power word: fortitude on party"] = &AiObjectContextInternal::power_word_fortitude_on_party;
                creators["divine spirit"] = &AiObjectContextInternal::divine_spirit;
                creators["divine spirit on party"] = &AiObjectContextInternal::divine_spirit_on_party;
                creators["power word: shield"] = &AiObjectContextInternal::power_word_shield;
                creators["power word: shield on party"] = &AiObjectContextInternal::power_word_shield_on_party;
                creators["renew"] = &AiObjectContextInternal::renew;
                creators["renew on party"] = &AiObjectContextInternal::renew_on_party;
                creators["greater heal"] = &AiObjectContextInternal::greater_heal;
                creators["greater heal on party"] = &AiObjectContextInternal::greater_heal_on_party;
                creators["heal"] = &AiObjectContextInternal::heal;
                creators["heal on party"] = &AiObjectContextInternal::heal_on_party;
                creators["lesser heal"] = &AiObjectContextInternal::lesser_heal;
                creators["lesser heal on party"] = &AiObjectContextInternal::lesser_heal_on_party;
                creators["flash heal"] = &AiObjectContextInternal::flash_heal;
                creators["flash heal on party"] = &AiObjectContextInternal::flash_heal_on_party;
                creators["dispel magic"] = &AiObjectContextInternal::dispel_magic;
                creators["dispel magic on party"] = &AiObjectContextInternal::dispel_magic_on_party;
                creators["dispel magic on target"] = &AiObjectContextInternal::dispel_magic_on_target;
                creators["cure disease"] = &AiObjectContextInternal::cure_disease;
                creators["cure disease on party"] = &AiObjectContextInternal::cure_disease_on_party;
                creators["abolish disease"] = &AiObjectContextInternal::abolish_disease;
                creators["abolish disease on party"] = &AiObjectContextInternal::abolish_disease_on_party;
                creators["fade"] = &AiObjectContextInternal::fade;
                creators["inner fire"] = &AiObjectContextInternal::inner_fire;
                creators["resurrection"] = &AiObjectContextInternal::resurrection;
                creators["circle of healing"] = &AiObjectContextInternal::circle_of_healing;
                creators["psychic scream"] = &AiObjectContextInternal::psychic_scream;
                creators["vampiric touch"] = &AiObjectContextInternal::vampiric_touch;
                creators["vampiric embrace"] = &AiObjectContextInternal::vampiric_embrace;
                creators["dispersion"] = &AiObjectContextInternal::dispersion;
            }

        private:
            static std::shared_ptr<Action> dispersion(PlayerbotAI* ai) { return std::make_shared<CastDispersionAction>(ai); }
            static std::shared_ptr<Action> vampiric_embrace(PlayerbotAI* ai) { return std::make_shared<CastVampiricEmbraceAction>(ai); }
            static std::shared_ptr<Action> vampiric_touch(PlayerbotAI* ai) { return std::make_shared<CastVampiricTouchAction>(ai); }
            static std::shared_ptr<Action> psychic_scream(PlayerbotAI* ai) { return std::make_shared<CastPsychicScreamAction>(ai); }
            static std::shared_ptr<Action> circle_of_healing(PlayerbotAI* ai) { return std::make_shared<CastCircleOfHealingAction>(ai); }
            static std::shared_ptr<Action> resurrection(PlayerbotAI* ai) { return std::make_shared<CastResurrectionAction>(ai); }
            static std::shared_ptr<Action> shadow_word_pain(PlayerbotAI* ai) { return std::make_shared<CastPowerWordPainAction>(ai); }
            static std::shared_ptr<Action> shadow_word_pain_on_attacker(PlayerbotAI* ai) { return std::make_shared<CastPowerWordPainOnAttackerAction>(ai); }
            static std::shared_ptr<Action> devouring_plague(PlayerbotAI* ai) { return std::make_shared<CastDevouringPlagueAction>(ai); }
            static std::shared_ptr<Action> mind_flay(PlayerbotAI* ai) { return std::make_shared<CastMindFlayAction>(ai); }
            static std::shared_ptr<Action> holy_fire(PlayerbotAI* ai) { return std::make_shared<CastHolyFireAction>(ai); }
            static std::shared_ptr<Action> smite(PlayerbotAI* ai) { return std::make_shared<CastSmiteAction>(ai); }
            static std::shared_ptr<Action> mind_blast(PlayerbotAI* ai) { return std::make_shared<CastMindBlastAction>(ai); }
            static std::shared_ptr<Action> shadowform(PlayerbotAI* ai) { return std::make_shared<CastShadowformAction>(ai); }
            static std::shared_ptr<Action> remove_shadowform(PlayerbotAI* ai) { return std::make_shared<CastRemoveShadowformAction>(ai); }
            static std::shared_ptr<Action> holy_nova(PlayerbotAI* ai) { return std::make_shared<CastHolyNovaAction>(ai); }
            static std::shared_ptr<Action> power_word_fortitude(PlayerbotAI* ai) { return std::make_shared<CastPowerWordFortitudeAction>(ai); }
            static std::shared_ptr<Action> power_word_fortitude_on_party(PlayerbotAI* ai) { return std::make_shared<CastPowerWordFortitudeOnPartyAction>(ai); }
            static std::shared_ptr<Action> divine_spirit(PlayerbotAI* ai) { return std::make_shared<CastDivineSpiritAction>(ai); }
            static std::shared_ptr<Action> divine_spirit_on_party(PlayerbotAI* ai) { return std::make_shared<CastDivineSpiritOnPartyAction>(ai); }
            static std::shared_ptr<Action> power_word_shield(PlayerbotAI* ai) { return std::make_shared<CastPowerWordShieldAction>(ai); }
            static std::shared_ptr<Action> power_word_shield_on_party(PlayerbotAI* ai) { return std::make_shared<CastPowerWordShieldOnPartyAction>(ai); }
            static std::shared_ptr<Action> renew(PlayerbotAI* ai) { return std::make_shared<CastRenewAction>(ai); }
            static std::shared_ptr<Action> renew_on_party(PlayerbotAI* ai) { return std::make_shared<CastRenewOnPartyAction>(ai); }
            static std::shared_ptr<Action> greater_heal(PlayerbotAI* ai) { return std::make_shared<CastGreaterHealAction>(ai); }
            static std::shared_ptr<Action> greater_heal_on_party(PlayerbotAI* ai) { return std::make_shared<CastGreaterHealOnPartyAction>(ai); }
            static std::shared_ptr<Action> heal(PlayerbotAI* ai) { return std::make_shared<CastHealAction>(ai); }
            static std::shared_ptr<Action> heal_on_party(PlayerbotAI* ai) { return std::make_shared<CastHealOnPartyAction>(ai); }
            static std::shared_ptr<Action> lesser_heal(PlayerbotAI* ai) { return std::make_shared<CastLesserHealAction>(ai); }
            static std::shared_ptr<Action> lesser_heal_on_party(PlayerbotAI* ai) { return std::make_shared<CastLesserHealOnPartyAction>(ai); }
            static std::shared_ptr<Action> flash_heal(PlayerbotAI* ai) { return std::make_shared<CastFlashHealAction>(ai); }
            static std::shared_ptr<Action> flash_heal_on_party(PlayerbotAI* ai) { return std::make_shared<CastFlashHealOnPartyAction>(ai); }
            static std::shared_ptr<Action> dispel_magic(PlayerbotAI* ai) { return std::make_shared<CastDispelMagicAction>(ai); }
            static std::shared_ptr<Action> dispel_magic_on_party(PlayerbotAI* ai) { return std::make_shared<CastDispelMagicOnPartyAction>(ai); }
            static std::shared_ptr<Action> dispel_magic_on_target(PlayerbotAI* ai) { return std::make_shared<CastDispelMagicOnTargetAction>(ai); }
            static std::shared_ptr<Action> cure_disease(PlayerbotAI* ai) { return std::make_shared<CastCureDiseaseAction>(ai); }
            static std::shared_ptr<Action> cure_disease_on_party(PlayerbotAI* ai) { return std::make_shared<CastCureDiseaseOnPartyAction>(ai); }
            static std::shared_ptr<Action> abolish_disease(PlayerbotAI* ai) { return std::make_shared<CastAbolishDiseaseAction>(ai); }
            static std::shared_ptr<Action> abolish_disease_on_party(PlayerbotAI* ai) { return std::make_shared<CastAbolishDiseaseOnPartyAction>(ai); }
            static std::shared_ptr<Action> fade(PlayerbotAI* ai) { return std::make_shared<CastFadeAction>(ai); }
            static std::shared_ptr<Action> inner_fire(PlayerbotAI* ai) { return std::make_shared<CastInnerFireAction>(ai); }
        };
    };
};

PriestAiObjectContext::PriestAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::priest::StrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::CombatStrategyFactoryInternal());
    actionContexts.Add(new ai::priest::AiObjectContextInternal());
    triggerContexts.Add(new ai::priest::TriggerFactoryInternal());
}
