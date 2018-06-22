//
#include "../../playerbot.h"
#include "DruidActions.h"
#include "DruidAiObjectContext.h"
#include "BearTankDruidStrategy.h"
#include "CatDpsDruidStrategy.h"
#include "CasterDruidStrategy.h"
#include "GenericDruidNonCombatStrategy.h"
#include "../NamedObjectContext.h"
#include "DruidTriggers.h"
#include "HealDruidStrategy.h"

using namespace ai;

namespace ai
{
    namespace druid
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["nc"] = &druid::StrategyFactoryInternal::nc;
                creators["cat aoe"] = &druid::StrategyFactoryInternal::cat_aoe;
                creators["caster aoe"] = &druid::StrategyFactoryInternal::caster_aoe;
                creators["caster debuff"] = &druid::StrategyFactoryInternal::caster_debuff;
                creators["dps debuff"] = &druid::StrategyFactoryInternal::caster_debuff;
            }

        private:
            static std::shared_ptr<Strategy> nc(PlayerbotAI* ai) { return std::make_shared<GenericDruidNonCombatStrategy>(ai); }
            static std::shared_ptr<Strategy> cat_aoe(PlayerbotAI* ai) { return std::make_shared<CatAoeDruidStrategy>(ai); }
            static std::shared_ptr<Strategy> caster_aoe(PlayerbotAI* ai) { return std::make_shared<CasterDruidAoeStrategy>(ai); }
            static std::shared_ptr<Strategy> caster_debuff(PlayerbotAI* ai) { return std::make_shared<CasterDruidDebuffStrategy>(ai); }
        };

        class DruidStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            DruidStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["bear"] = &druid::DruidStrategyFactoryInternal::bear;
                creators["tank"] = &druid::DruidStrategyFactoryInternal::bear;
                creators["cat"] = &druid::DruidStrategyFactoryInternal::cat;
                creators["caster"] = &druid::DruidStrategyFactoryInternal::caster;
                creators["dps"] = &druid::DruidStrategyFactoryInternal::cat;
                creators["heal"] = &druid::DruidStrategyFactoryInternal::heal;
            }

        private:
            static std::shared_ptr<Strategy> bear(PlayerbotAI* ai) { return std::make_shared<BearTankDruidStrategy>(ai); }
            static std::shared_ptr<Strategy> cat(PlayerbotAI* ai) { return std::make_shared<CatDpsDruidStrategy>(ai); }
            static std::shared_ptr<Strategy> caster(PlayerbotAI* ai) { return std::make_shared<CasterDruidStrategy>(ai); }
            static std::shared_ptr<Strategy> heal(PlayerbotAI* ai) { return std::make_shared<HealDruidStrategy>(ai); }
        };
    };
};

namespace ai
{
    namespace druid
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["thorns"] = &TriggerFactoryInternal::Thorns;
                creators["bash"] = &TriggerFactoryInternal::bash;
                creators["faerie fire (feral)"] = &TriggerFactoryInternal::faerie_fire_feral;
                creators["faerie fire"] = &TriggerFactoryInternal::faerie_fire;
                creators["insect swarm"] = &TriggerFactoryInternal::insect_swarm;
                creators["moonfire"] = &TriggerFactoryInternal::moonfire;
                creators["nature's grasp"] = &TriggerFactoryInternal::natures_grasp;
                creators["tiger's fury"] = &TriggerFactoryInternal::tigers_fury;
                creators["rake"] = &TriggerFactoryInternal::rake;
                creators["mark of the wild"] = &TriggerFactoryInternal::mark_of_the_wild;
                creators["mark of the wild on party"] = &TriggerFactoryInternal::mark_of_the_wild_on_party;
                creators["cure poison"] = &TriggerFactoryInternal::cure_poison;
                creators["party member cure poison"] = &TriggerFactoryInternal::party_member_cure_poison;
                creators["entangling roots"] = &TriggerFactoryInternal::entangling_roots;
                creators["bear form"] = &TriggerFactoryInternal::bear_form;
                creators["cat form"] = &TriggerFactoryInternal::cat_form;
                creators["tree form"] = &TriggerFactoryInternal::tree_form;
                creators["eclipse (solar)"] = &TriggerFactoryInternal::eclipse_solar;
                creators["eclipse (lunar)"] = &TriggerFactoryInternal::eclipse_lunar;
                creators["bash on enemy healer"] = &TriggerFactoryInternal::bash_on_enemy_healer;
            }

        private:
            static std::shared_ptr<Trigger> eclipse_solar(PlayerbotAI* ai) { return std::make_shared<EclipseSolarTrigger>(ai); }
            static std::shared_ptr<Trigger> eclipse_lunar(PlayerbotAI* ai) { return std::make_shared<EclipseLunarTrigger>(ai); }
            static std::shared_ptr<Trigger> Thorns(PlayerbotAI* ai) { return std::make_shared<ThornsTrigger>(ai); }
            static std::shared_ptr<Trigger> bash(PlayerbotAI* ai) { return std::make_shared<BashInterruptSpellTrigger>(ai); }
            static std::shared_ptr<Trigger> faerie_fire_feral(PlayerbotAI* ai) { return std::make_shared<FaerieFireFeralTrigger>(ai); }
            static std::shared_ptr<Trigger> insect_swarm(PlayerbotAI* ai) { return std::make_shared<InsectSwarmTrigger>(ai); }
            static std::shared_ptr<Trigger> moonfire(PlayerbotAI* ai) { return std::make_shared<MoonfireTrigger>(ai); }
            static std::shared_ptr<Trigger> faerie_fire(PlayerbotAI* ai) { return std::make_shared<FaerieFireTrigger>(ai); }
            static std::shared_ptr<Trigger> natures_grasp(PlayerbotAI* ai) { return std::make_shared<NaturesGraspTrigger>(ai); }
            static std::shared_ptr<Trigger> tigers_fury(PlayerbotAI* ai) { return std::make_shared<TigersFuryTrigger>(ai); }
            static std::shared_ptr<Trigger> rake(PlayerbotAI* ai) { return std::make_shared<RakeTrigger>(ai); }
            static std::shared_ptr<Trigger> mark_of_the_wild(PlayerbotAI* ai) { return std::make_shared<MarkOfTheWildTrigger>(ai); }
            static std::shared_ptr<Trigger> mark_of_the_wild_on_party(PlayerbotAI* ai) { return std::make_shared<MarkOfTheWildOnPartyTrigger>(ai); }
            static std::shared_ptr<Trigger> cure_poison(PlayerbotAI* ai) { return std::make_shared<CurePoisonTrigger>(ai); }
            static std::shared_ptr<Trigger> party_member_cure_poison(PlayerbotAI* ai) { return std::make_shared<PartyMemberCurePoisonTrigger>(ai); }
            static std::shared_ptr<Trigger> entangling_roots(PlayerbotAI* ai) { return std::make_shared<EntanglingRootsTrigger>(ai); }
            static std::shared_ptr<Trigger> bear_form(PlayerbotAI* ai) { return std::make_shared<BearFormTrigger>(ai); }
            static std::shared_ptr<Trigger> cat_form(PlayerbotAI* ai) { return std::make_shared<CatFormTrigger>(ai); }
            static std::shared_ptr<Trigger> tree_form(PlayerbotAI* ai) { return std::make_shared<TreeFormTrigger>(ai); }
            static std::shared_ptr<Trigger> bash_on_enemy_healer(PlayerbotAI* ai) { return std::make_shared<BashInterruptEnemyHealerSpellTrigger>(ai); }
        };
    };
};

namespace ai
{
    namespace druid
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["feral charge - bear"] = &AiObjectContextInternal::feral_charge_bear;
                creators["feral charge - cat"] = &AiObjectContextInternal::feral_charge_cat;
                creators["swipe (bear)"] = &AiObjectContextInternal::swipe_bear;
                creators["faerie fire (feral)"] = &AiObjectContextInternal::faerie_fire_feral;
                creators["faerie fire"] = &AiObjectContextInternal::faerie_fire;
                creators["bear form"] = &AiObjectContextInternal::bear_form;
                creators["dire bear form"] = &AiObjectContextInternal::dire_bear_form;
                creators["moonkin form"] = &AiObjectContextInternal::moonkin_form;
                creators["cat form"] = &AiObjectContextInternal::cat_form;
                creators["tree form"] = &AiObjectContextInternal::tree_form;
                creators["caster form"] = &AiObjectContextInternal::caster_form;
                creators["mangle (bear)"] = &AiObjectContextInternal::mangle_bear;
                creators["maul"] = &AiObjectContextInternal::maul;
                creators["bash"] = &AiObjectContextInternal::bash;
                creators["swipe"] = &AiObjectContextInternal::swipe;
                creators["growl"] = &AiObjectContextInternal::growl;
                creators["demoralizing roar"] = &AiObjectContextInternal::demoralizing_roar;
                creators["hibernate"] = &AiObjectContextInternal::hibernate;
                creators["entangling roots"] = &AiObjectContextInternal::entangling_roots;
                creators["entangling roots on cc"] = &AiObjectContextInternal::entangling_roots_on_cc;
                creators["wrath"] = &AiObjectContextInternal::wrath;
                creators["starfall"] = &AiObjectContextInternal::starfall;
                creators["insect swarm"] = &AiObjectContextInternal::insect_swarm;
                creators["moonfire"] = &AiObjectContextInternal::moonfire;
                creators["starfire"] = &AiObjectContextInternal::starfire;
                creators["nature's grasp"] = &AiObjectContextInternal::natures_grasp;
                creators["claw"] = &AiObjectContextInternal::claw;
                creators["mangle (cat)"] = &AiObjectContextInternal::mangle_cat;
                creators["swipe (cat)"] = &AiObjectContextInternal::swipe_cat;
                creators["rake"] = &AiObjectContextInternal::rake;
                creators["ferocious bite"] = &AiObjectContextInternal::ferocious_bite;
                creators["rip"] = &AiObjectContextInternal::rip;
                creators["cower"] = &AiObjectContextInternal::cower;
                creators["survival instincts"] = &AiObjectContextInternal::survival_instincts;
                creators["thorns"] = &AiObjectContextInternal::thorns;
                creators["cure poison"] = &AiObjectContextInternal::cure_poison;
                creators["cure poison on party"] = &AiObjectContextInternal::cure_poison_on_party;
                creators["abolish poison"] = &AiObjectContextInternal::abolish_poison;
                creators["abolish poison on party"] = &AiObjectContextInternal::abolish_poison_on_party;
                creators["berserk"] = &AiObjectContextInternal::berserk;
                creators["tiger's fury"] = &AiObjectContextInternal::tigers_fury;
                creators["mark of the wild"] = &AiObjectContextInternal::mark_of_the_wild;
                creators["mark of the wild on party"] = &AiObjectContextInternal::mark_of_the_wild_on_party;
                creators["regrowth"] = &AiObjectContextInternal::regrowth;
                creators["rejuvenation"] = &AiObjectContextInternal::rejuvenation;
                creators["healing touch"] = &AiObjectContextInternal::healing_touch;
                creators["regrowth on party"] = &AiObjectContextInternal::regrowth_on_party;
                creators["rejuvenation on party"] = &AiObjectContextInternal::rejuvenation_on_party;
                creators["healing touch on party"] = &AiObjectContextInternal::healing_touch_on_party;
                creators["rebirth"] = &AiObjectContextInternal::rebirth;
                creators["revive"] = &AiObjectContextInternal::revive;
                creators["barskin"] = &AiObjectContextInternal::barskin;
                creators["lacerate"] = &AiObjectContextInternal::lacerate;
                creators["hurricane"] = &AiObjectContextInternal::hurricane;
                creators["innervate"] = &AiObjectContextInternal::innervate;
                creators["tranquility"] = &AiObjectContextInternal::tranquility;
                creators["bash on enemy healer"] = &AiObjectContextInternal::bash_on_enemy_healer;
            }

        private:
            static std::shared_ptr<Action> tranquility(PlayerbotAI* ai) { return std::make_shared<CastTranquilityAction>(ai); }
            static std::shared_ptr<Action> feral_charge_bear(PlayerbotAI* ai) { return std::make_shared<CastFeralChargeBearAction>(ai); }
            static std::shared_ptr<Action> feral_charge_cat(PlayerbotAI* ai) { return std::make_shared<CastFeralChargeCatAction>(ai); }
            static std::shared_ptr<Action> swipe_bear(PlayerbotAI* ai) { return std::make_shared<CastSwipeBearAction>(ai); }
            static std::shared_ptr<Action> faerie_fire_feral(PlayerbotAI* ai) { return std::make_shared<CastFaerieFireFeralAction>(ai); }
            static std::shared_ptr<Action> faerie_fire(PlayerbotAI* ai) { return std::make_shared<CastFaerieFireAction>(ai); }
            static std::shared_ptr<Action> bear_form(PlayerbotAI* ai) { return std::make_shared<CastBearFormAction>(ai); }
            static std::shared_ptr<Action> dire_bear_form(PlayerbotAI* ai) { return std::make_shared<CastDireBearFormAction>(ai); }
            static std::shared_ptr<Action> cat_form(PlayerbotAI* ai) { return std::make_shared<CastCatFormAction>(ai); }
            static std::shared_ptr<Action> tree_form(PlayerbotAI* ai) { return std::make_shared<CastTreeFormAction>(ai); }
            static std::shared_ptr<Action> caster_form(PlayerbotAI* ai) { return std::make_shared<CastCasterFormAction>(ai); }
            static std::shared_ptr<Action> mangle_bear(PlayerbotAI* ai) { return std::make_shared<CastMangleBearAction>(ai); }
            static std::shared_ptr<Action> maul(PlayerbotAI* ai) { return std::make_shared<CastMaulAction>(ai); }
            static std::shared_ptr<Action> bash(PlayerbotAI* ai) { return std::make_shared<CastBashAction>(ai); }
            static std::shared_ptr<Action> swipe(PlayerbotAI* ai) { return std::make_shared<CastSwipeAction>(ai); }
            static std::shared_ptr<Action> growl(PlayerbotAI* ai) { return std::make_shared<CastGrowlAction>(ai); }
            static std::shared_ptr<Action> demoralizing_roar(PlayerbotAI* ai) { return std::make_shared<CastDemoralizingRoarAction>(ai); }
            static std::shared_ptr<Action> moonkin_form(PlayerbotAI* ai) { return std::make_shared<CastMoonkinFormAction>(ai); }
            static std::shared_ptr<Action> hibernate(PlayerbotAI* ai) { return std::make_shared<CastHibernateAction>(ai); }
            static std::shared_ptr<Action> entangling_roots(PlayerbotAI* ai) { return std::make_shared<CastEntanglingRootsAction>(ai); }
            static std::shared_ptr<Action> entangling_roots_on_cc(PlayerbotAI* ai) { return std::make_shared<CastEntanglingRootsCcAction>(ai); }
            static std::shared_ptr<Action> wrath(PlayerbotAI* ai) { return std::make_shared<CastWrathAction>(ai); }
            static std::shared_ptr<Action> starfall(PlayerbotAI* ai) { return std::make_shared<CastStarfallAction>(ai); }
            static std::shared_ptr<Action> insect_swarm(PlayerbotAI* ai) { return std::make_shared<CastInsectSwarmAction>(ai); }
            static std::shared_ptr<Action> moonfire(PlayerbotAI* ai) { return std::make_shared<CastMoonfireAction>(ai); }
            static std::shared_ptr<Action> starfire(PlayerbotAI* ai) { return std::make_shared<CastStarfireAction>(ai); }
            static std::shared_ptr<Action> natures_grasp(PlayerbotAI* ai) { return std::make_shared<CastNaturesGraspAction>(ai); }
            static std::shared_ptr<Action> claw(PlayerbotAI* ai) { return std::make_shared<CastClawAction>(ai); }
            static std::shared_ptr<Action> mangle_cat(PlayerbotAI* ai) { return std::make_shared<CastMangleCatAction>(ai); }
            static std::shared_ptr<Action> swipe_cat(PlayerbotAI* ai) { return std::make_shared<CastSwipeCatAction>(ai); }
            static std::shared_ptr<Action> rake(PlayerbotAI* ai) { return std::make_shared<CastRakeAction>(ai); }
            static std::shared_ptr<Action> ferocious_bite(PlayerbotAI* ai) { return std::make_shared<CastFerociousBiteAction>(ai); }
            static std::shared_ptr<Action> rip(PlayerbotAI* ai) { return std::make_shared<CastRipAction>(ai); }
            static std::shared_ptr<Action> cower(PlayerbotAI* ai) { return std::make_shared<CastCowerAction>(ai); }
            static std::shared_ptr<Action> survival_instincts(PlayerbotAI* ai) { return std::make_shared<CastSurvivalInstinctsAction>(ai); }
            static std::shared_ptr<Action> thorns(PlayerbotAI* ai) { return std::make_shared<CastThornsAction>(ai); }
            static std::shared_ptr<Action> cure_poison(PlayerbotAI* ai) { return std::make_shared<CastCurePoisonAction>(ai); }
            static std::shared_ptr<Action> cure_poison_on_party(PlayerbotAI* ai) { return std::make_shared<CastCurePoisonOnPartyAction>(ai); }
            static std::shared_ptr<Action> abolish_poison(PlayerbotAI* ai) { return std::make_shared<CastAbolishPoisonAction>(ai); }
            static std::shared_ptr<Action> abolish_poison_on_party(PlayerbotAI* ai) { return std::make_shared<CastAbolishPoisonOnPartyAction>(ai); }
            static std::shared_ptr<Action> berserk(PlayerbotAI* ai) { return std::make_shared<CastBerserkAction>(ai); }
            static std::shared_ptr<Action> tigers_fury(PlayerbotAI* ai) { return std::make_shared<CastTigersFuryAction>(ai); }
            static std::shared_ptr<Action> mark_of_the_wild(PlayerbotAI* ai) { return std::make_shared<CastMarkOfTheWildAction>(ai); }
            static std::shared_ptr<Action> mark_of_the_wild_on_party(PlayerbotAI* ai) { return std::make_shared<CastMarkOfTheWildOnPartyAction>(ai); }
            static std::shared_ptr<Action> regrowth(PlayerbotAI* ai) { return std::make_shared<CastRegrowthAction>(ai); }
            static std::shared_ptr<Action> rejuvenation(PlayerbotAI* ai) { return std::make_shared<CastRejuvenationAction>(ai); }
            static std::shared_ptr<Action> healing_touch(PlayerbotAI* ai) { return std::make_shared<CastHealingTouchAction>(ai); }
            static std::shared_ptr<Action> regrowth_on_party(PlayerbotAI* ai) { return std::make_shared<CastRegrowthOnPartyAction>(ai); }
            static std::shared_ptr<Action> rejuvenation_on_party(PlayerbotAI* ai) { return std::make_shared<CastRejuvenationOnPartyAction>(ai); }
            static std::shared_ptr<Action> healing_touch_on_party(PlayerbotAI* ai) { return std::make_shared<CastHealingTouchOnPartyAction>(ai); }
            static std::shared_ptr<Action> rebirth(PlayerbotAI* ai) { return std::make_shared<CastRebirthAction>(ai); }
            static std::shared_ptr<Action> revive(PlayerbotAI* ai) { return std::make_shared<CastReviveAction>(ai); }
            static std::shared_ptr<Action> barskin(PlayerbotAI* ai) { return std::make_shared<CastBarskinAction>(ai); }
            static std::shared_ptr<Action> lacerate(PlayerbotAI* ai) { return std::make_shared<CastLacerateAction>(ai); }
            static std::shared_ptr<Action> hurricane(PlayerbotAI* ai) { return std::make_shared<CastHurricaneAction>(ai); }
            static std::shared_ptr<Action> innervate(PlayerbotAI* ai) { return std::make_shared<CastInnervateAction>(ai); }
            static std::shared_ptr<Action> bash_on_enemy_healer(PlayerbotAI* ai) { return std::make_shared<CastBashOnEnemyHealerAction>(ai); }
        };
    };
};

DruidAiObjectContext::DruidAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::druid::StrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::DruidStrategyFactoryInternal());
    actionContexts.Add(new ai::druid::AiObjectContextInternal());
    triggerContexts.Add(new ai::druid::TriggerFactoryInternal());
}
