
#include "../../playerbot.h"
#include "WarlockActions.h"
#include "WarlockAiObjectContext.h"
#include "DpsWarlockStrategy.h"
#include "GenericWarlockNonCombatStrategy.h"
#include "TankWarlockStrategy.h"
#include "../generic/PullStrategy.h"
#include "WarlockTriggers.h"
#include "../NamedObjectContext.h"
#include "../actions/UseItemAction.h"

using namespace ai;

namespace ai
{
    namespace warlock
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["nc"] = &warlock::StrategyFactoryInternal::nc;
                creators["pull"] = &warlock::StrategyFactoryInternal::pull;
                creators["aoe"] = &warlock::StrategyFactoryInternal::aoe;
                creators["dps debuff"] = &warlock::StrategyFactoryInternal::dps_debuff;
            }

        private:
            static std::shared_ptr<Strategy> nc(PlayerbotAI* ai) { return std::make_shared<GenericWarlockNonCombatStrategy>(ai); }
            static std::shared_ptr<Strategy> aoe(PlayerbotAI* ai) { return std::make_shared<DpsAoeWarlockStrategy>(ai); }
            static std::shared_ptr<Strategy> dps_debuff(PlayerbotAI* ai) { return std::make_shared<DpsWarlockDebuffStrategy>(ai); }
            static std::shared_ptr<Strategy> pull(PlayerbotAI* ai) { return std::make_shared<PullStrategy>(ai, "shoot"); }
        };

        class CombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["dps"] = &warlock::CombatStrategyFactoryInternal::dps;
                creators["tank"] = &warlock::CombatStrategyFactoryInternal::tank;
            }

        private:
            static std::shared_ptr<Strategy> tank(PlayerbotAI* ai) { return std::make_shared<TankWarlockStrategy>(ai); }
            static std::shared_ptr<Strategy> dps(PlayerbotAI* ai) { return std::make_shared<DpsWarlockStrategy>(ai); }
        };
    };
};

namespace ai
{
    namespace warlock
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["shadow trance"] = &TriggerFactoryInternal::shadow_trance;
                creators["demon armor"] = &TriggerFactoryInternal::demon_armor;
                creators["no healthstone"] = &TriggerFactoryInternal::HasHealthstone;
                creators["no firestone"] = &TriggerFactoryInternal::HasFirestone;
                creators["no spellstone"] = &TriggerFactoryInternal::HasSpellstone;
                creators["corruption"] = &TriggerFactoryInternal::corruption;
                creators["corruption on attacker"] = &TriggerFactoryInternal::corruption_on_attacker;
                creators["curse of agony"] = &TriggerFactoryInternal::curse_of_agony;
                creators["banish"] = &TriggerFactoryInternal::banish;
                creators["spellstone"] = &TriggerFactoryInternal::spellstone;
                creators["backlash"] = &TriggerFactoryInternal::backlash;
                creators["fear"] = &TriggerFactoryInternal::fear;
                creators["immolate"] = &TriggerFactoryInternal::immolate;


            }

        private:
            static std::shared_ptr<Trigger> shadow_trance(PlayerbotAI* ai) { return std::make_shared<ShadowTranceTrigger>(ai); }
            static std::shared_ptr<Trigger> demon_armor(PlayerbotAI* ai) { return std::make_shared<DemonArmorTrigger>(ai); }
            static std::shared_ptr<Trigger> HasHealthstone(PlayerbotAI* ai) { return std::make_shared<HasHealthstoneTrigger>(ai); }
            static std::shared_ptr<Trigger> HasFirestone(PlayerbotAI* ai) { return std::make_shared<HasFirestoneTrigger>(ai); }
            static std::shared_ptr<Trigger> HasSpellstone(PlayerbotAI* ai) { return std::make_shared<HasSpellstoneTrigger>(ai); }
            static std::shared_ptr<Trigger> corruption(PlayerbotAI* ai) { return std::make_shared<CorruptionTrigger>(ai); }
            static std::shared_ptr<Trigger> corruption_on_attacker(PlayerbotAI* ai) { return std::make_shared<CorruptionOnAttackerTrigger>(ai); }
            static std::shared_ptr<Trigger> curse_of_agony(PlayerbotAI* ai) { return std::make_shared<CurseOfAgonyTrigger>(ai); }
            static std::shared_ptr<Trigger> banish(PlayerbotAI* ai) { return std::make_shared<BanishTrigger>(ai); }
            static std::shared_ptr<Trigger> spellstone(PlayerbotAI* ai) { return std::make_shared<SpellstoneTrigger>(ai); }
            static std::shared_ptr<Trigger> backlash(PlayerbotAI* ai) { return std::make_shared<BacklashTrigger>(ai); }
            static std::shared_ptr<Trigger> fear(PlayerbotAI* ai) { return std::make_shared<FearTrigger>(ai); }
            static std::shared_ptr<Trigger> immolate(PlayerbotAI* ai) { return std::make_shared<ImmolateTrigger>(ai); }

        };
    };
};

namespace ai
{
    namespace warlock
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["summon imp"] = &AiObjectContextInternal::summon_imp;
                creators["fel armor"] = &AiObjectContextInternal::fel_armor;
                creators["demon armor"] = &AiObjectContextInternal::demon_armor;
                creators["demon skin"] = &AiObjectContextInternal::demon_skin;
                creators["create healthstone"] = &AiObjectContextInternal::create_healthstone;
                creators["create firestone"] = &AiObjectContextInternal::create_firestone;
                creators["create spellstone"] = &AiObjectContextInternal::create_spellstone;
                creators["spellstone"] = &AiObjectContextInternal::spellstone;
                creators["summon voidwalker"] = &AiObjectContextInternal::summon_voidwalker;
                creators["summon felguard"] = &AiObjectContextInternal::summon_felguard;
                creators["immolate"] = &AiObjectContextInternal::immolate;
                creators["corruption"] = &AiObjectContextInternal::corruption;
                creators["corruption on attacker"] = &AiObjectContextInternal::corruption_on_attacker;
                creators["curse of agony"] = &AiObjectContextInternal::curse_of_agony;
                creators["shadow bolt"] = &AiObjectContextInternal::shadow_bolt;
                creators["drain soul"] = &AiObjectContextInternal::drain_soul;
                creators["drain mana"] = &AiObjectContextInternal::drain_mana;
                creators["drain life"] = &AiObjectContextInternal::drain_life;
                creators["banish"] = &AiObjectContextInternal::banish;
                creators["seed of corruption"] = &AiObjectContextInternal::seed_of_corruption;
                creators["rain of fire"] = &AiObjectContextInternal::rain_of_fire;
                creators["shadowfury"] = &AiObjectContextInternal::shadowfury;
                creators["life tap"] = &AiObjectContextInternal::life_tap;
                creators["fear"] = &AiObjectContextInternal::fear;
                creators["fear on cc"] = &AiObjectContextInternal::fear_on_cc;
                creators["incinirate"] = &AiObjectContextInternal::incinirate;
                creators["conflagrate"] = &AiObjectContextInternal::conflagrate;
            }

        private:
            static std::shared_ptr<Action> conflagrate(PlayerbotAI* ai) { return std::make_shared<CastConflagrateAction>(ai); }
            static std::shared_ptr<Action> incinirate(PlayerbotAI* ai) { return std::make_shared<CastIncinirateAction>(ai); }
            static std::shared_ptr<Action> fear_on_cc(PlayerbotAI* ai) { return std::make_shared<CastFearOnCcAction>(ai); }
            static std::shared_ptr<Action> fear(PlayerbotAI* ai) { return std::make_shared<CastFearAction>(ai); }
            static std::shared_ptr<Action> immolate(PlayerbotAI* ai) { return std::make_shared<CastImmolateAction>(ai); }
            static std::shared_ptr<Action> summon_imp(PlayerbotAI* ai) { return std::make_shared<CastSummonImpAction>(ai); }
            static std::shared_ptr<Action> fel_armor(PlayerbotAI* ai) { return std::make_shared<CastFelArmorAction>(ai); }
            static std::shared_ptr<Action> demon_armor(PlayerbotAI* ai) { return std::make_shared<CastDemonArmorAction>(ai); }
            static std::shared_ptr<Action> demon_skin(PlayerbotAI* ai) { return std::make_shared<CastDemonSkinAction>(ai); }
            static std::shared_ptr<Action> create_healthstone(PlayerbotAI* ai) { return std::make_shared<CastCreateHealthstoneAction>(ai); }
            static std::shared_ptr<Action> create_firestone(PlayerbotAI* ai) { return std::make_shared<CastCreateFirestoneAction>(ai); }
            static std::shared_ptr<Action> create_spellstone(PlayerbotAI* ai) { return std::make_shared<CastCreateSpellstoneAction>(ai); }
            static std::shared_ptr<Action> spellstone(PlayerbotAI* ai) { return std::make_shared<UseSpellItemAction>(ai, "spellstone", true); }
            static std::shared_ptr<Action> summon_voidwalker(PlayerbotAI* ai) { return std::make_shared<CastSummonVoidwalkerAction>(ai); }
            static std::shared_ptr<Action> summon_felguard(PlayerbotAI* ai) { return std::make_shared<CastSummonFelguardAction>(ai); }
            static std::shared_ptr<Action> corruption(PlayerbotAI* ai) { return std::make_shared<CastCorruptionAction>(ai); }
            static std::shared_ptr<Action> corruption_on_attacker(PlayerbotAI* ai) { return std::make_shared<CastCorruptionOnAttackerAction>(ai); }
            static std::shared_ptr<Action> curse_of_agony(PlayerbotAI* ai) { return std::make_shared<CastCurseOfAgonyAction>(ai); }
            static std::shared_ptr<Action> shadow_bolt(PlayerbotAI* ai) { return std::make_shared<CastShadowBoltAction>(ai); }
            static std::shared_ptr<Action> drain_soul(PlayerbotAI* ai) { return std::make_shared<CastDrainSoulAction>(ai); }
            static std::shared_ptr<Action> drain_mana(PlayerbotAI* ai) { return std::make_shared<CastDrainManaAction>(ai); }
            static std::shared_ptr<Action> drain_life(PlayerbotAI* ai) { return std::make_shared<CastDrainLifeAction>(ai); }
            static std::shared_ptr<Action> banish(PlayerbotAI* ai) { return std::make_shared<CastBanishAction>(ai); }
            static std::shared_ptr<Action> seed_of_corruption(PlayerbotAI* ai) { return std::make_shared<CastSeedOfCorruptionAction>(ai); }
            static std::shared_ptr<Action> rain_of_fire(PlayerbotAI* ai) { return std::make_shared<CastRainOfFireAction>(ai); }
            static std::shared_ptr<Action> shadowfury(PlayerbotAI* ai) { return std::make_shared<CastShadowfuryAction>(ai); }
            static std::shared_ptr<Action> life_tap(PlayerbotAI* ai) { return std::make_shared<CastLifeTapAction>(ai); }

        };
    };
};



WarlockAiObjectContext::WarlockAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::warlock::StrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::CombatStrategyFactoryInternal());
    actionContexts.Add(new ai::warlock::AiObjectContextInternal());
    triggerContexts.Add(new ai::warlock::TriggerFactoryInternal());
}
