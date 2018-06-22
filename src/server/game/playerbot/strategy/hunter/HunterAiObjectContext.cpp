
#include "../../playerbot.h"
#include "HunterActions.h"
#include "HunterTriggers.h"
#include "HunterAiObjectContext.h"
#include "DpsHunterStrategy.h"
#include "GenericHunterNonCombatStrategy.h"
#include "HunterBuffStrategies.h"
#include "../NamedObjectContext.h"

using namespace ai;


namespace ai
{
    namespace hunter
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["dps"] = &hunter::StrategyFactoryInternal::dps;
                creators["nc"] = &hunter::StrategyFactoryInternal::nc;
                creators["aoe"] = &hunter::StrategyFactoryInternal::aoe;
                creators["dps debuff"] = &hunter::StrategyFactoryInternal::dps_debuff;
            }

        private:
            static std::shared_ptr<Strategy> aoe(PlayerbotAI* ai) { return std::make_shared<DpsAoeHunterStrategy>(ai); }
            static std::shared_ptr<Strategy> dps(PlayerbotAI* ai) { return std::make_shared<DpsHunterStrategy>(ai); }
            static std::shared_ptr<Strategy> nc(PlayerbotAI* ai) { return std::make_shared<GenericHunterNonCombatStrategy>(ai); }
            static std::shared_ptr<Strategy> dps_debuff(PlayerbotAI* ai) { return std::make_shared<DpsHunterDebuffStrategy>(ai); }
        };

        class BuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["bspeed"] = &hunter::BuffStrategyFactoryInternal::bspeed;
                creators["bdps"] = &hunter::BuffStrategyFactoryInternal::bdps;
                creators["bmana"] = &hunter::BuffStrategyFactoryInternal::bmana;
                creators["rnature"] = &hunter::BuffStrategyFactoryInternal::rnature;
            }

        private:
            static std::shared_ptr<Strategy> bspeed(PlayerbotAI* ai) { return std::make_shared<HunterBuffSpeedStrategy>(ai); }
            static std::shared_ptr<Strategy> bdps(PlayerbotAI* ai) { return std::make_shared<HunterBuffDpsStrategy>(ai); }
            static std::shared_ptr<Strategy> bmana(PlayerbotAI* ai) { return std::make_shared<HunterBuffManaStrategy>(ai); }
            static std::shared_ptr<Strategy> rnature(PlayerbotAI* ai) { return std::make_shared<HunterNatureResistanceStrategy>(ai); }
        };
    };
};


namespace ai
{
    namespace hunter
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["aspect of the viper"] = &TriggerFactoryInternal::aspect_of_the_viper;
                creators["black arrow"] = &TriggerFactoryInternal::black_arrow;
                creators["no stings"] = &TriggerFactoryInternal::NoStings;
                creators["hunters pet dead"] = &TriggerFactoryInternal::hunters_pet_dead;
                creators["hunters pet low health"] = &TriggerFactoryInternal::hunters_pet_low_health;
                creators["hunter's mark"] = &TriggerFactoryInternal::hunters_mark;
                creators["freezing trap"] = &TriggerFactoryInternal::freezing_trap;
                creators["aspect of the pack"] = &TriggerFactoryInternal::aspect_of_the_pack;
                creators["rapid fire"] = &TriggerFactoryInternal::rapid_fire;
                creators["aspect of the hawk"] = &TriggerFactoryInternal::aspect_of_the_hawk;
                creators["aspect of the wild"] = &TriggerFactoryInternal::aspect_of_the_wild;
                creators["aspect of the viper"] = &TriggerFactoryInternal::aspect_of_the_viper;
                creators["trueshot aura"] = &TriggerFactoryInternal::trueshot_aura;
                creators["serpent sting on attacker"] = &TriggerFactoryInternal::serpent_sting_on_attacker;
            }

        private:
            static std::shared_ptr<Trigger> serpent_sting_on_attacker(PlayerbotAI* ai) { return std::make_shared<SerpentStingOnAttackerTrigger>(ai); }
            static std::shared_ptr<Trigger> trueshot_aura(PlayerbotAI* ai) { return std::make_shared<TrueshotAuraTrigger>(ai); }
            static std::shared_ptr<Trigger> aspect_of_the_viper(PlayerbotAI* ai) { return std::make_shared<HunterAspectOfTheViperTrigger>(ai); }
            static std::shared_ptr<Trigger> black_arrow(PlayerbotAI* ai) { return std::make_shared<BlackArrowTrigger>(ai); }
            static std::shared_ptr<Trigger> NoStings(PlayerbotAI* ai) { return std::make_shared<HunterNoStingsActiveTrigger>(ai); }
            static std::shared_ptr<Trigger> hunters_pet_dead(PlayerbotAI* ai) { return std::make_shared<HuntersPetDeadTrigger>(ai); }
            static std::shared_ptr<Trigger> hunters_pet_low_health(PlayerbotAI* ai) { return std::make_shared<HuntersPetLowHealthTrigger>(ai); }
            static std::shared_ptr<Trigger> hunters_mark(PlayerbotAI* ai) { return std::make_shared<HuntersMarkTrigger>(ai); }
            static std::shared_ptr<Trigger> freezing_trap(PlayerbotAI* ai) { return std::make_shared<FreezingTrapTrigger>(ai); }
            static std::shared_ptr<Trigger> aspect_of_the_pack(PlayerbotAI* ai) { return std::make_shared<HunterAspectOfThePackTrigger>(ai); }
            static std::shared_ptr<Trigger> rapid_fire(PlayerbotAI* ai) { return std::make_shared<RapidFireTrigger>(ai); }
            static std::shared_ptr<Trigger> aspect_of_the_hawk(PlayerbotAI* ai) { return std::make_shared<HunterAspectOfTheHawkTrigger>(ai); }
            static std::shared_ptr<Trigger> aspect_of_the_wild(PlayerbotAI* ai) { return std::make_shared<HunterAspectOfTheWildTrigger>(ai); }
        };
    };
};



namespace ai
{
    namespace hunter
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["auto shot"] = &AiObjectContextInternal::auto_shot;
                creators["aimed shot"] = &AiObjectContextInternal::aimed_shot;
                creators["chimera shot"] = &AiObjectContextInternal::chimera_shot;
                creators["explosive shot"] = &AiObjectContextInternal::explosive_shot;
                creators["arcane shot"] = &AiObjectContextInternal::arcane_shot;
                creators["concussive shot"] = &AiObjectContextInternal::concussive_shot;
                creators["distracting shot"] = &AiObjectContextInternal::distracting_shot;
                creators["multi-shot"] = &AiObjectContextInternal::multi_shot;
                creators["volley"] = &AiObjectContextInternal::volley;
                creators["serpent sting"] = &AiObjectContextInternal::serpent_sting;
                creators["serpent sting on attacker"] = &AiObjectContextInternal::serpent_sting_on_attacker;
                creators["wyvern sting"] = &AiObjectContextInternal::wyvern_sting;
                creators["viper sting"] = &AiObjectContextInternal::viper_sting;
                creators["scorpid sting"] = &AiObjectContextInternal::scorpid_sting;
                creators["hunter's mark"] = &AiObjectContextInternal::hunters_mark;
                creators["mend pet"] = &AiObjectContextInternal::mend_pet;
                creators["revive pet"] = &AiObjectContextInternal::revive_pet;
                creators["call pet"] = &AiObjectContextInternal::call_pet;
                creators["black arrow"] = &AiObjectContextInternal::black_arrow;
                creators["freezing trap"] = &AiObjectContextInternal::freezing_trap;
                creators["rapid fire"] = &AiObjectContextInternal::rapid_fire;
                creators["boost"] = &AiObjectContextInternal::rapid_fire;
                creators["readiness"] = &AiObjectContextInternal::readiness;
                creators["aspect of the hawk"] = &AiObjectContextInternal::aspect_of_the_hawk;
                creators["aspect of the wild"] = &AiObjectContextInternal::aspect_of_the_wild;
                creators["aspect of the viper"] = &AiObjectContextInternal::aspect_of_the_viper;
                creators["aspect of the pack"] = &AiObjectContextInternal::aspect_of_the_pack;
                creators["aspect of the cheetah"] = &AiObjectContextInternal::aspect_of_the_cheetah;
                creators["trueshot aura"] = &AiObjectContextInternal::trueshot_aura;
                creators["feign death"] = &AiObjectContextInternal::feign_death;
                creators["wing clip"] = &AiObjectContextInternal::wing_clip;
            }

        private:
            static std::shared_ptr<Action> feign_death(PlayerbotAI* ai) { return std::make_shared<CastFeignDeathAction>(ai); }
            static std::shared_ptr<Action> trueshot_aura(PlayerbotAI* ai) { return std::make_shared<CastTrueshotAuraAction>(ai); }
            static std::shared_ptr<Action> auto_shot(PlayerbotAI* ai) { return std::make_shared<CastAutoShotAction>(ai); }
            static std::shared_ptr<Action> aimed_shot(PlayerbotAI* ai) { return std::make_shared<CastAimedShotAction>(ai); }
            static std::shared_ptr<Action> chimera_shot(PlayerbotAI* ai) { return std::make_shared<CastChimeraShotAction>(ai); }
            static std::shared_ptr<Action> explosive_shot(PlayerbotAI* ai) { return std::make_shared<CastExplosiveShotAction>(ai); }
            static std::shared_ptr<Action> arcane_shot(PlayerbotAI* ai) { return std::make_shared<CastArcaneShotAction>(ai); }
            static std::shared_ptr<Action> concussive_shot(PlayerbotAI* ai) { return std::make_shared<CastConcussiveShotAction>(ai); }
            static std::shared_ptr<Action> distracting_shot(PlayerbotAI* ai) { return std::make_shared<CastDistractingShotAction>(ai); }
            static std::shared_ptr<Action> multi_shot(PlayerbotAI* ai) { return std::make_shared<CastMultiShotAction>(ai); }
            static std::shared_ptr<Action> volley(PlayerbotAI* ai) { return std::make_shared<CastVolleyAction>(ai); }
            static std::shared_ptr<Action> serpent_sting(PlayerbotAI* ai) { return std::make_shared<CastSerpentStingAction>(ai); }
            static std::shared_ptr<Action> serpent_sting_on_attacker(PlayerbotAI* ai) { return std::make_shared<CastSerpentStingOnAttackerAction>(ai); }
            static std::shared_ptr<Action> wyvern_sting(PlayerbotAI* ai) { return std::make_shared<CastWyvernStingAction>(ai); }
            static std::shared_ptr<Action> viper_sting(PlayerbotAI* ai) { return std::make_shared<CastViperStingAction>(ai); }
            static std::shared_ptr<Action> scorpid_sting(PlayerbotAI* ai) { return std::make_shared<CastScorpidStingAction>(ai); }
            static std::shared_ptr<Action> hunters_mark(PlayerbotAI* ai) { return std::make_shared<CastHuntersMarkAction>(ai); }
            static std::shared_ptr<Action> mend_pet(PlayerbotAI* ai) { return std::make_shared<CastMendPetAction>(ai); }
            static std::shared_ptr<Action> revive_pet(PlayerbotAI* ai) { return std::make_shared<CastRevivePetAction>(ai); }
            static std::shared_ptr<Action> call_pet(PlayerbotAI* ai) { return std::make_shared<CastCallPetAction>(ai); }
            static std::shared_ptr<Action> black_arrow(PlayerbotAI* ai) { return std::make_shared<CastBlackArrow>(ai); }
            static std::shared_ptr<Action> freezing_trap(PlayerbotAI* ai) { return std::make_shared<CastFreezingTrap>(ai); }
            static std::shared_ptr<Action> rapid_fire(PlayerbotAI* ai) { return std::make_shared<CastRapidFireAction>(ai); }
            static std::shared_ptr<Action> readiness(PlayerbotAI* ai) { return std::make_shared<CastReadinessAction>(ai); }
            static std::shared_ptr<Action> aspect_of_the_hawk(PlayerbotAI* ai) { return std::make_shared<CastAspectOfTheHawkAction>(ai); }
            static std::shared_ptr<Action> aspect_of_the_wild(PlayerbotAI* ai) { return std::make_shared<CastAspectOfTheWildAction>(ai); }
            static std::shared_ptr<Action> aspect_of_the_viper(PlayerbotAI* ai) { return std::make_shared<CastAspectOfTheViperAction>(ai); }
            static std::shared_ptr<Action> aspect_of_the_pack(PlayerbotAI* ai) { return std::make_shared<CastAspectOfThePackAction>(ai); }
            static std::shared_ptr<Action> aspect_of_the_cheetah(PlayerbotAI* ai) { return std::make_shared<CastAspectOfTheCheetahAction>(ai); }
            static std::shared_ptr<Action> wing_clip(PlayerbotAI* ai) { return std::make_shared<CastWingClipAction>(ai); }
        };
    };
};

HunterAiObjectContext::HunterAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::hunter::StrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::BuffStrategyFactoryInternal());
    actionContexts.Add(new ai::hunter::AiObjectContextInternal());
    triggerContexts.Add(new ai::hunter::TriggerFactoryInternal());
}
